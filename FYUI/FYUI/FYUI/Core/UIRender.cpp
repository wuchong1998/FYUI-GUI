#include "pch.h"
#include "UIRender.h"
#include "UIRenderContext.h"
#include "UIRenderD2DCacheTypesInternal.h"
#include "UIRenderD2DFrameMetricsInternal.h"
#include "UIRenderD2DResourceInternal.h"
#include "UIRenderD2DSharedInternal.h"
#include "UIRenderDirectWriteTextInternal.h"
#include "UIRenderHtmlMetricsInternal.h"
#include "UIRenderHtmlParseInternal.h"
#include "UIRenderImageD2DInternal.h"
#include "UIRenderImageRuntimeInternal.h"
#include "UIRenderPrimitiveInternal.h"
#include "UIRenderTextSharedInternal.h"
#include "UIRenderSurface.h"

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <algorithm>
#include <cmath>
#include <cwctype>
#include <memory>
#include <new>
#include <utility>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#include <unordered_map>

#include <vector>
#include <string>
#pragma warning(push)
#pragma warning(disable:4244)
///////////////////////////////////////////////////////////////////////////////////////
namespace FYUI
{
	static int g_iFontID = MAX_FONT_ID;
	int GetRenderHelperFontId()
	{
		return g_iFontID;
	}
	using Microsoft::WRL::ComPtr;
	using namespace RenderD2DFrameMetricsInternal;
	using namespace RenderD2DResourceInternal;
	using namespace RenderD2DInternal;
	using namespace RenderHtmlMetricsInternal;
	using namespace RenderHtmlParseInternal;
	using namespace RenderImageD2DInternal;

	namespace
	{
		constexpr float kDirect2DPixelCoordinateDpi = 96.0f;

		enum class D2DClipType
		{
			AxisAlignedRect,
			RoundRect,
		};

		struct D2DClipState
		{
			D2DClipType type = D2DClipType::AxisAlignedRect;
			RECT rc = {};
			int width = 0;
			int height = 0;
		};

		std::vector<D2DClipState>& ActiveD2DClipStack()
		{
			static thread_local std::vector<D2DClipState> clipStack;
			return clipStack;
		}

		struct HtmlParseCacheValue;

		struct D2DRenderState
		{
			// Backend preferences and factories
			RenderBackendType preferredBackend = RenderBackendAuto;
			Direct2DRenderMode preferredRenderMode = Direct2DRenderModeAuto;
			Direct2DRenderMode activeRenderMode = Direct2DRenderModeAuto;
			ComPtr<ID2D1Factory> factory;
			ComPtr<ID2D1DCRenderTarget> dcRenderTarget;
			ComPtr<IDWriteFactory> writeFactory;
			ComPtr<IWICImagingFactory> wicFactory;

			// Batched drawing state
			HDC batchNativeDC = NULL;
			RECT batchRect = { 0, 0, 0, 0 };
			UINT batchDepth = 0;
			bool batchDrawing = false;
			D2D1_MATRIX_3X2_F dcBindTransform = D2D1::Matrix3x2F::Identity();

			// Image and primitive caches
			std::unordered_map<D2DBitmapCacheKey, ComPtr<ID2D1Bitmap>, D2DBitmapCacheKeyHasher> bitmapCache;
			std::unordered_map<DWORD, ComPtr<ID2D1SolidColorBrush>> brushCache;
			std::unordered_map<int, ComPtr<ID2D1StrokeStyle>> strokeStyleCache;
			std::unordered_map<unsigned long long, ComPtr<ID2D1GradientStopCollection>> gradientStopCollectionCache;

			// Plain text caches
			std::unordered_map<D2DTextFormatCacheKey, ComPtr<IDWriteTextFormat>, D2DTextFormatCacheKeyHasher> textFormatCache;
			std::unordered_map<D2DTextFormatCacheKey, ComPtr<IDWriteInlineObject>, D2DTextFormatCacheKeyHasher> textEllipsisCache;
			std::unordered_map<D2DTextLayoutCacheKey, CachedTextLayoutValue, D2DTextLayoutCacheKeyHasher> textLayoutCache;
			std::unordered_map<PlainTextNormalizationCacheKey, PlainTextNormalizationCacheValue, PlainTextNormalizationCacheKeyHasher> plainTextNormalizationCache;

			// Html text caches
			std::unordered_map<HtmlParseCacheKey, std::shared_ptr<HtmlParseCacheValue>, HtmlParseCacheKeyHasher> htmlParseCache;
			std::unordered_map<HtmlTextLayoutCacheKey, CachedTextLayoutValue, HtmlTextLayoutCacheKeyHasher> htmlTextLayoutCache;
			std::unordered_map<HtmlHitTestMetricsCacheKey, std::vector<RECT>, HtmlHitTestMetricsCacheKeyHasher> htmlHitTestMetricsCache;
			std::unordered_map<UINT_PTR, HtmlDrawingEffectState> htmlDrawingEffectStates;

			Direct2DFrameMetricsInternal frameMetrics;
		};

		D2DRenderState& GetD2DRenderState()
		{
			static D2DRenderState state;
			return state;
		}

		void ClearD2DResourceCaches(D2DRenderState& state)
		{
			state.bitmapCache.clear();
			state.brushCache.clear();
			state.strokeStyleCache.clear();
			state.gradientStopCollectionCache.clear();
		}

		void ClearHtmlRuntimeCaches(D2DRenderState& state);

		template<typename TCache, typename TKey, typename TObject>
		bool TryReturnCachedComObject(const TCache& cache, const TKey& key, TObject** ppObject)
		{
			auto it = cache.find(key);
			if (it == cache.end() || !it->second) {
				return false;
			}

			*ppObject = it->second.Get();
			(*ppObject)->AddRef();
			return true;
		}

		template<typename TCache, typename TKey, typename TObject>
		void StoreCachedComObject(TCache& cache, const TKey& key, ComPtr<TObject>& object, size_t cacheLimit, TObject** ppObject)
		{
			if (cache.size() >= cacheLimit) {
				cache.clear();
			}

			cache[key] = object;
			*ppObject = object.Detach();
		}

		template<typename TCache, typename TKey, typename TValue>
		void StoreCachedValue(TCache& cache, const TKey& key, TValue&& value, size_t cacheLimit)
		{
			if (cache.size() >= cacheLimit) {
				cache.clear();
			}

			cache[key] = std::forward<TValue>(value);
		}

		void StoreCachedDirectWriteTextLayout(
			D2DRenderState& state,
			const D2DTextLayoutCacheKey& layoutKey,
			IDWriteTextLayout* pTextLayout,
			const DWRITE_TEXT_METRICS& metrics,
			bool hasMetrics)
		{
			++state.frameMetrics.frameTextLayoutCacheRefreshCount;
			if (hasMetrics) {
				++state.frameMetrics.frameTextMetricsCacheRefreshCount;
			}

			StoreCachedValue(
				state.textLayoutCache,
				layoutKey,
				BuildCachedDirectWriteTextLayoutValueInternal(pTextLayout, metrics, hasMetrics),
				GetDirectWriteTextLayoutCacheLimitInternal());
		}

		template<typename TCache, typename TKey>
		HRESULT TryReturnCachedDirectWriteTextLayout(
			const TCache& cache,
			const TKey& layoutKey,
			UINT& layoutHitCounter,
			UINT& metricsHitCounter,
			ComPtr<IDWriteTextLayout>& textLayout,
			DWRITE_TEXT_METRICS* pTextMetrics,
			bool& usedCachedLayout)
		{
			usedCachedLayout = false;

			auto it = cache.find(layoutKey);
			if (it == cache.end() || !it->second.layout) {
				return S_OK;
			}

			++layoutHitCounter;
			textLayout = it->second.layout;

			bool usedCachedMetrics = false;
			const HRESULT hr = ResolveCachedDirectWriteTextMetricsInternal(it->second, pTextMetrics, usedCachedMetrics);
			if (FAILED(hr)) {
				return hr;
			}
			if (usedCachedMetrics) {
				++metricsHitCounter;
			}

			usedCachedLayout = true;
			return S_OK;
		}

		void StoreCachedPlainTextNormalization(
			D2DRenderState& state,
			const PlainTextNormalizationCacheKey& cacheKey,
			const std::wstring& normalizedText,
			const std::vector<DWRITE_TEXT_RANGE>& hotkeyRanges)
		{
			PlainTextNormalizationCacheValue cacheValue;
			cacheValue.normalizedText = normalizedText;
			cacheValue.hotkeyRanges = hotkeyRanges;

			++state.frameMetrics.frameTextNormalizationCacheRefreshCount;
			StoreCachedValue(
				state.plainTextNormalizationCache,
				cacheKey,
				std::move(cacheValue),
				GetDirectWritePlainTextNormalizationCacheLimitInternal());
		}

		void StoreCachedDirectWriteHtmlTextLayout(
			D2DRenderState& state,
			const HtmlTextLayoutCacheKey& layoutKey,
			IDWriteTextLayout* pTextLayout,
			const DWRITE_TEXT_METRICS& metrics,
			bool hasMetrics)
		{
			if (state.htmlTextLayoutCache.size() >= GetDirectWriteHtmlTextLayoutCacheLimitInternal()) {
				ClearHtmlRuntimeCaches(state);
			}

			++state.frameMetrics.frameHtmlLayoutCacheRefreshCount;
			if (hasMetrics) {
				++state.frameMetrics.frameHtmlMetricsCacheRefreshCount;
			}

			state.htmlTextLayoutCache[layoutKey] = BuildCachedDirectWriteTextLayoutValueInternal(pTextLayout, metrics, hasMetrics);
			state.htmlDrawingEffectStates[reinterpret_cast<UINT_PTR>(pTextLayout)] = HtmlDrawingEffectState();
		}

		void StoreCachedHtmlHitTestMetrics(
			D2DRenderState& state,
			const HtmlHitTestMetricsCacheKey& cacheKey,
			const std::vector<RECT>& rects)
		{
			++state.frameMetrics.frameHtmlHitTestCacheRefreshCount;
			StoreCachedValue(
				state.htmlHitTestMetricsCache,
				cacheKey,
				rects,
				GetDirectWriteHtmlHitTestMetricsCacheLimitInternal());
		}

		RECT ResolveDCRenderTargetBindRect(HDC hNativeDC, const RECT& fallbackRect)
		{
			RECT rcBind = fallbackRect;
			RECT rcClip = { 0 };
			const int nClipType = ::GetClipBox(hNativeDC, &rcClip);
			if (nClipType != ERROR && IsRectValid(rcClip)) {
				if (IsRectValid(rcBind)) {
					RECT rcIntersect = { 0 };
					if (::IntersectRect(&rcIntersect, &rcBind, &rcClip)) {
						rcBind = rcIntersect;
					}
					else {
						rcBind = rcClip;
					}
				}
				else {
					rcBind = rcClip;
				}
			}

			if (!IsRectValid(rcBind)) {
				rcBind.left = 0;
				rcBind.top = 0;
				rcBind.right = 1;
				rcBind.bottom = 1;
			}
			return rcBind;
		}

		D2D1_MATRIX_3X2_F BuildDCRenderTargetBindTransform(const RECT& rcTarget)
		{
			return D2D1::Matrix3x2F::Translation(
				-static_cast<float>(rcTarget.left),
				-static_cast<float>(rcTarget.top));
		}

		HRESULT CreateDCRenderTargetForMode(
			D2DRenderState& state,
			Direct2DRenderMode requestedMode,
			Direct2DRenderMode resolvedMode,
			Direct2DRenderMode* pActiveMode)
		{
			const D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties(
				ToD2DRenderTargetType(resolvedMode),
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
				0.0f,
				0.0f,
				D2D1_RENDER_TARGET_USAGE_NONE,
				D2D1_FEATURE_LEVEL_DEFAULT);

			HRESULT hr = state.factory->CreateDCRenderTarget(&properties, state.dcRenderTarget.ReleaseAndGetAddressOf());
			if (SUCCEEDED(hr)) {
				state.activeRenderMode = requestedMode;
				if (pActiveMode != nullptr) {
					*pActiveMode = resolvedMode;
				}
			}
			return hr;
		}

		HRESULT EnsureD2DFactory()
		{
			D2DRenderState& state = GetD2DRenderState();
			if (state.factory) {
				return S_OK;
			}
			return ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, state.factory.ReleaseAndGetAddressOf());
		}

		HRESULT EnsureDCRenderTarget()
		{
			D2DRenderState& state = GetD2DRenderState();
			HRESULT hr = EnsureD2DFactory();
			if (FAILED(hr)) {
				return hr;
			}
			if (state.dcRenderTarget) {
				return S_OK;
			}

			Direct2DRenderMode activeMode = Direct2DRenderModeAuto;
			hr = CreateDCRenderTargetForMode(state, state.preferredRenderMode, state.preferredRenderMode, &activeMode);
			if (SUCCEEDED(hr)) {
				state.activeRenderMode = activeMode;
				return hr;
			}

			if (state.preferredRenderMode != Direct2DRenderModeSoftware) {
				hr = CreateDCRenderTargetForMode(state, state.preferredRenderMode, Direct2DRenderModeSoftware, &activeMode);
				if (SUCCEEDED(hr)) {
					state.activeRenderMode = activeMode;
					return hr;
				}
			}

			state.activeRenderMode = Direct2DRenderModeAuto;
			return hr;
		}

		HRESULT EnsureDirectWriteFactory()
		{
			D2DRenderState& state = GetD2DRenderState();
			if (state.writeFactory) {
				return S_OK;
			}

			return ::DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(state.writeFactory.ReleaseAndGetAddressOf()));
		}

		HRESULT EnsureWICFactory()
		{
			D2DRenderState& state = GetD2DRenderState();
			if (state.wicFactory) {
				return S_OK;
			}

			return ::CoCreateInstance(
				CLSID_WICImagingFactory,
				nullptr,
				CLSCTX_INPROC_SERVER,
				__uuidof(IWICImagingFactory),
				reinterpret_cast<void**>(state.wicFactory.ReleaseAndGetAddressOf()));
		}

		bool CanUseDirect2D();

		void ResetDirect2DBatchState(D2DRenderState& state)
		{
			state.batchNativeDC = NULL;
			::SetRectEmpty(&state.batchRect);
			state.batchDepth = 0;
			state.batchDrawing = false;
		}

		void ResetD2DRenderTargetResources(D2DRenderState& state)
		{
			state.dcRenderTarget.Reset();
			state.activeRenderMode = Direct2DRenderModeAuto;
			ClearD2DResourceCaches(state);
		}

		void ClearHtmlRuntimeCaches(D2DRenderState& state)
		{
			state.htmlTextLayoutCache.clear();
			state.htmlHitTestMetricsCache.clear();
			state.htmlDrawingEffectStates.clear();
		}

		HRESULT FinishDirect2DDraw(D2DRenderState& state)
		{
			if (!state.batchDrawing || !state.dcRenderTarget) {
				return S_FALSE;
			}

			const HRESULT hr = state.dcRenderTarget->EndDraw();
			state.batchDrawing = false;
			if (hr == D2DERR_RECREATE_TARGET) {
				ResetD2DRenderTargetResources(state);
			}
			return hr;
		}

		bool BindDCRenderTargetToNativeDC(HDC hNativeDC, const RECT& rcBind, ID2D1DCRenderTarget** ppRenderTarget)
		{
			if (ppRenderTarget == nullptr) {
				return false;
			}
			*ppRenderTarget = nullptr;

			if (hNativeDC == NULL || !CanUseDirect2D()) {
				return false;
			}

			D2DRenderState& state = GetD2DRenderState();
			const HRESULT ensureHr = EnsureDCRenderTarget();
			if (FAILED(ensureHr) || !state.dcRenderTarget) {
				return false;
			}

			const RECT rcTarget = ResolveDCRenderTargetBindRect(hNativeDC, rcBind);
			const HRESULT bindHr = state.dcRenderTarget->BindDC(hNativeDC, &rcTarget);
			if (FAILED(bindHr)) {
				ResetD2DRenderTargetResources(state);
				return false;
			}

			state.dcBindTransform = BuildDCRenderTargetBindTransform(rcTarget);
			state.dcRenderTarget->SetTransform(state.dcBindTransform);
			state.dcRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			state.dcRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
			// FYUI layouts and manager scaling are pixel based; keep D2D from applying a second DPI scale.
			state.dcRenderTarget->SetDpi(kDirect2DPixelCoordinateDpi, kDirect2DPixelCoordinateDpi);
			*ppRenderTarget = state.dcRenderTarget.Get();
			return true;
		}

		bool ResumeDirect2DBatch(D2DRenderState& state)
		{
			if (state.batchDepth == 0 || state.batchNativeDC == NULL) {
				return false;
			}
			if (state.batchDrawing) {
				return state.dcRenderTarget != nullptr;
			}

			ID2D1DCRenderTarget* renderTarget = nullptr;
			if (!BindDCRenderTargetToNativeDC(state.batchNativeDC, state.batchRect, &renderTarget) || renderTarget == nullptr) {
				return false;
			}

			renderTarget->BeginDraw();
			state.batchDrawing = true;
			return true;
		}

		void FlushDirect2DBatchForNativeDC(HDC hNativeDC)
		{
			D2DRenderState& state = GetD2DRenderState();
			if (state.batchDepth == 0 || !state.batchDrawing) {
				return;
			}
			if (hNativeDC != NULL && state.batchNativeDC != hNativeDC) {
				return;
			}
			++state.frameMetrics.frameBatchFlushCount;
			FinishDirect2DDraw(state);
		}

		void FlushActiveDirect2DBatch()
		{
			FlushDirect2DBatchForNativeDC(NULL);
		}

		bool CanUseDirect2D()
		{
			D2DRenderState& state = GetD2DRenderState();
			if (state.preferredBackend == RenderBackendGDI) {
				return false;
			}
			return SUCCEEDED(EnsureD2DFactory());
		}

		bool CanUseDirect2DRenderContext(const CPaintRenderContext& renderContext)
		{
			return renderContext.GetDC() != NULL && CanUseDirect2D();
		}

		class D2DDrawScope
		{
		public:
			D2DDrawScope(CPaintRenderContext& renderContext, const RECT& rcBind)
			{
				if (!CanUseDirect2DRenderContext(renderContext)) {
					return;
				}
				HDC hNativeDC = renderContext.GetDC();
				D2DRenderState& state = GetD2DRenderState();
				if (state.batchDepth > 0 && state.batchNativeDC == hNativeDC) {
					if (ResumeDirect2DBatch(state)) {
						m_renderTarget = state.dcRenderTarget.Get();
						ApplyActiveClips();
					}
					return;
				}

				m_resumeBatch = state.batchDepth > 0 && state.batchNativeDC != NULL && state.batchDrawing;
				if (m_resumeBatch) {
					FinishDirect2DDraw(state);
				}

				ID2D1DCRenderTarget* renderTarget = nullptr;
				if (!BindDCRenderTargetToNativeDC(hNativeDC, rcBind, &renderTarget) || renderTarget == nullptr) {
					if (m_resumeBatch) {
						ResumeDirect2DBatch(state);
					}
					m_resumeBatch = false;
					return;
				}

				renderTarget->BeginDraw();
				m_renderTarget = renderTarget;
				m_ownsDraw = true;
				++state.frameMetrics.frameStandaloneDrawCount;
				ApplyActiveClips();
			}

			~D2DDrawScope()
			{
				PopActiveClips();

				if (m_ownsDraw && m_renderTarget != nullptr) {
					D2DRenderState& state = GetD2DRenderState();
					const HRESULT hr = m_renderTarget->EndDraw();
					if (hr == D2DERR_RECREATE_TARGET) {
						state.dcRenderTarget.Reset();
						state.activeRenderMode = Direct2DRenderModeAuto;
						ClearD2DResourceCaches(state);
					}
				}

				if (m_resumeBatch) {
					ResumeDirect2DBatch(GetD2DRenderState());
				}
			}

			ID2D1DCRenderTarget* Get() const
			{
				return m_renderTarget;
			}

			explicit operator bool() const
			{
				return m_renderTarget != nullptr;
			}

		private:
			void ApplyActiveClips()
			{
				if (m_renderTarget == nullptr) {
					return;
				}

				const std::vector<D2DClipState>& clipStack = ActiveD2DClipStack();
				if (clipStack.empty()) {
					return;
				}

				ComPtr<ID2D1Factory> factory;
				m_renderTarget->GetFactory(factory.GetAddressOf());
				if (!factory) {
					return;
				}

				for (const D2DClipState& clip : clipStack) {
					if (!IsRectValid(clip.rc)) {
						continue;
					}

					if (clip.type == D2DClipType::AxisAlignedRect) {
						m_renderTarget->PushAxisAlignedClip(ToD2DRectF(clip.rc), D2D1_ANTIALIAS_MODE_ALIASED);
						m_clipOperations.push_back(D2DClipType::AxisAlignedRect);
						continue;
					}

					ComPtr<ID2D1RoundedRectangleGeometry> geometry;
					const D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
						ToD2DRectF(clip.rc),
						static_cast<float>((std::max)(1, clip.width)) * 0.5f,
						static_cast<float>((std::max)(1, clip.height)) * 0.5f);
					if (FAILED(factory->CreateRoundedRectangleGeometry(roundedRect, geometry.GetAddressOf()))) {
						continue;
					}

					ComPtr<ID2D1Layer> layer;
					if (FAILED(m_renderTarget->CreateLayer(nullptr, layer.GetAddressOf()))) {
						continue;
					}

					m_renderTarget->PushLayer(
						D2D1::LayerParameters(
							D2D1::InfiniteRect(),
							geometry.Get(),
							D2D1_ANTIALIAS_MODE_PER_PRIMITIVE),
						layer.Get());
					m_clipLayers.push_back(layer);
					m_clipGeometries.push_back(geometry);
					m_clipOperations.push_back(D2DClipType::RoundRect);
				}
			}

			void PopActiveClips()
			{
				if (m_renderTarget == nullptr) {
					return;
				}

				for (auto it = m_clipOperations.rbegin(); it != m_clipOperations.rend(); ++it) {
					if (*it == D2DClipType::RoundRect) {
						m_renderTarget->PopLayer();
					}
					else {
						m_renderTarget->PopAxisAlignedClip();
					}
				}
				m_clipOperations.clear();
				m_clipLayers.clear();
				m_clipGeometries.clear();
			}

			ID2D1DCRenderTarget* m_renderTarget = nullptr;
			std::vector<D2DClipType> m_clipOperations;
			std::vector<ComPtr<ID2D1Layer>> m_clipLayers;
			std::vector<ComPtr<ID2D1Geometry>> m_clipGeometries;
			bool m_ownsDraw = false;
			bool m_resumeBatch = false;
		};

		HRESULT CreateD2DBitmapFromHBITMAP(ID2D1RenderTarget* pRenderTarget, HBITMAP hBitmap, bool useAlpha, ID2D1Bitmap** ppBitmap)
		{
			if (pRenderTarget == nullptr || ppBitmap == nullptr || hBitmap == nullptr) {
				return E_INVALIDARG;
			}

			HRESULT hr = EnsureWICFactory();
			if (FAILED(hr)) {
				return hr;
			}

			return CreateD2DBitmapFromHBITMAPInternal(pRenderTarget, GetD2DRenderState().wicFactory.Get(), hBitmap, useAlpha, ppBitmap);
		}

		HRESULT GetCachedD2DBitmapFromHBITMAP(ID2D1RenderTarget* pRenderTarget, HBITMAP hBitmap, bool useAlpha, ID2D1Bitmap** ppBitmap)
		{
			if (ppBitmap == nullptr) {
				return E_INVALIDARG;
			}
			*ppBitmap = nullptr;

			if (pRenderTarget == nullptr || hBitmap == nullptr) {
				return E_INVALIDARG;
			}

			D2DRenderState& state = GetD2DRenderState();
			if (pRenderTarget != state.dcRenderTarget.Get()) {
				return CreateD2DBitmapFromHBITMAP(pRenderTarget, hBitmap, useAlpha, ppBitmap);
			}

			const D2DBitmapCacheKey key = { hBitmap, useAlpha };
			if (TryReturnCachedComObject(state.bitmapCache, key, ppBitmap)) {
				return S_OK;
			}

			ComPtr<ID2D1Bitmap> bitmap;
			HRESULT hr = CreateD2DBitmapFromHBITMAP(pRenderTarget, hBitmap, useAlpha, bitmap.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			StoreCachedComObject(state.bitmapCache, key, bitmap, GetD2DBitmapResourceCacheLimitInternal(), ppBitmap);
			return S_OK;
		}

		void GetCachedPlainTextNormalization(
			const std::wstring& sourceText,
			UINT uStyle,
			std::wstring& normalizedText,
			std::vector<DWRITE_TEXT_RANGE>& hotkeyRanges);

		HRESULT GetCachedTextFormat(const D2DTextFormatCacheKey& key, IDWriteTextFormat** ppTextFormat)
		{
			if (ppTextFormat == nullptr) {
				return E_INVALIDARG;
			}
			*ppTextFormat = nullptr;

			HRESULT hr = EnsureDirectWriteFactory();
			if (FAILED(hr)) {
				return hr;
			}

			D2DRenderState& state = GetD2DRenderState();
			if (TryReturnCachedComObject(state.textFormatCache, key, ppTextFormat)) {
				return S_OK;
			}

			ComPtr<IDWriteTextFormat> textFormat;
			hr = state.writeFactory->CreateTextFormat(
				key.fontFamily.c_str(),
				nullptr,
				static_cast<DWRITE_FONT_WEIGHT>(key.fontWeight),
				static_cast<DWRITE_FONT_STYLE>(key.fontStyle),
				DWRITE_FONT_STRETCH_NORMAL,
				static_cast<float>(key.fontSize100) / 100.0f,
				L"",
				textFormat.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			textFormat->SetTextAlignment(static_cast<DWRITE_TEXT_ALIGNMENT>(key.textAlignment));
			textFormat->SetParagraphAlignment(static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(key.paragraphAlignment));
			textFormat->SetWordWrapping(static_cast<DWRITE_WORD_WRAPPING>(key.wordWrapping));
			if (key.lineSpacing100 > 0 && key.baseline100 > 0) {
				textFormat->SetLineSpacing(
					DWRITE_LINE_SPACING_METHOD_UNIFORM,
					static_cast<float>(key.lineSpacing100) / 100.0f,
					static_cast<float>(key.baseline100) / 100.0f);
			}

			StoreCachedComObject(state.textFormatCache, key, textFormat, GetDirectWriteTextFormatCacheLimitInternal(), ppTextFormat);
			return S_OK;
		}

		HRESULT GetCachedEllipsisInlineObject(
			const D2DTextFormatCacheKey& key,
			IDWriteTextFormat* pTextFormat,
			IDWriteInlineObject** ppEllipsis)
		{
			if (ppEllipsis == nullptr || pTextFormat == nullptr) {
				return E_INVALIDARG;
			}
			*ppEllipsis = nullptr;

			HRESULT hr = EnsureDirectWriteFactory();
			if (FAILED(hr)) {
				return hr;
			}

			D2DRenderState& state = GetD2DRenderState();
			if (TryReturnCachedComObject(state.textEllipsisCache, key, ppEllipsis)) {
				return S_OK;
			}

			ComPtr<IDWriteInlineObject> ellipsis;
			hr = state.writeFactory->CreateEllipsisTrimmingSign(pTextFormat, ellipsis.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			StoreCachedComObject(state.textEllipsisCache, key, ellipsis, GetDirectWriteTextFormatCacheLimitInternal(), ppEllipsis);
			return S_OK;
		}

		void ApplyTextLayoutEndEllipsis(
			const D2DTextFormatCacheKey& formatKey,
			IDWriteTextFormat* pTextFormat,
			IDWriteTextLayout* pTextLayout,
			UINT uStyle)
		{
			if ((uStyle & DT_END_ELLIPSIS) == 0 || pTextFormat == nullptr || pTextLayout == nullptr) {
				return;
			}

			ComPtr<IDWriteInlineObject> ellipsis;
			if (SUCCEEDED(GetCachedEllipsisInlineObject(formatKey, pTextFormat, ellipsis.GetAddressOf()))) {
				DWRITE_TRIMMING trimming = {};
				trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
				pTextLayout->SetTrimming(&trimming, ellipsis.Get());
			}
		}

		HRESULT GetCachedSolidColorBrush(ID2D1RenderTarget* pRenderTarget, DWORD color, ID2D1SolidColorBrush** ppBrush)
		{
			if (ppBrush == nullptr) {
				return E_INVALIDARG;
			}
			*ppBrush = nullptr;

			if (pRenderTarget == nullptr) {
				return E_INVALIDARG;
			}

			D2DRenderState& state = GetD2DRenderState();
			if (pRenderTarget != state.dcRenderTarget.Get()) {
				return CreateSolidColorBrushInternal(pRenderTarget, color, ppBrush);
			}

			if (TryReturnCachedComObject(state.brushCache, color, ppBrush)) {
				return S_OK;
			}

			ComPtr<ID2D1SolidColorBrush> brush;
			HRESULT hr = CreateSolidColorBrushInternal(pRenderTarget, color, brush.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			StoreCachedComObject(state.brushCache, color, brush, GetD2DSolidColorBrushCacheLimitInternal(), ppBrush);
			return S_OK;
		}

		HRESULT GetCachedStrokeStyle(int nStyle, ID2D1StrokeStyle** ppStrokeStyle)
		{
			if (ppStrokeStyle == nullptr) {
				return E_INVALIDARG;
			}
			*ppStrokeStyle = nullptr;

			const int styleKey = BuildStrokeStyleCacheKeyInternal(nStyle);
			if (!ShouldUseD2DStrokeStyleInternal(styleKey)) {
				return S_FALSE;
			}

			HRESULT hr = EnsureD2DFactory();
			if (FAILED(hr)) {
				return hr;
			}

			D2DRenderState& state = GetD2DRenderState();
			if (TryReturnCachedComObject(state.strokeStyleCache, styleKey, ppStrokeStyle)) {
				return S_OK;
			}

			ComPtr<ID2D1StrokeStyle> strokeStyle;
			hr = CreateStrokeStyleInternal(state.factory.Get(), styleKey, strokeStyle.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			StoreCachedComObject(state.strokeStyleCache, styleKey, strokeStyle, GetD2DStrokeStyleCacheLimitInternal(), ppStrokeStyle);
			return S_OK;
		}

		HRESULT GetCachedGradientStopCollection(
			ID2D1RenderTarget* pRenderTarget,
			DWORD dwFirst,
			DWORD dwSecond,
			ID2D1GradientStopCollection** ppStopCollection)
		{
			if (ppStopCollection == nullptr) {
				return E_INVALIDARG;
			}
			*ppStopCollection = nullptr;

			if (pRenderTarget == nullptr) {
				return E_INVALIDARG;
			}

			D2DRenderState& state = GetD2DRenderState();
			if (pRenderTarget != state.dcRenderTarget.Get()) {
				return CreateGradientStopCollectionInternal(pRenderTarget, dwFirst, dwSecond, ppStopCollection);
			}

			const unsigned long long key = BuildGradientStopCollectionCacheKeyInternal(dwFirst, dwSecond);
			if (TryReturnCachedComObject(state.gradientStopCollectionCache, key, ppStopCollection)) {
				return S_OK;
			}

			ComPtr<ID2D1GradientStopCollection> stopCollection;
			HRESULT hr = CreateGradientStopCollectionInternal(pRenderTarget, dwFirst, dwSecond, stopCollection.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			StoreCachedComObject(state.gradientStopCollectionCache, key, stopCollection, GetD2DGradientStopCollectionCacheLimitInternal(), ppStopCollection);
			return S_OK;
		}

		HRESULT CreateTextLayout(
			CPaintManagerUI* pManager,
			std::wstring_view text,
			int iFont,
			UINT uStyle,
			const RECT& rc,
			std::wstring& wideText,
			ComPtr<IDWriteTextFormat>& textFormat,
			ComPtr<IDWriteTextLayout>& textLayout,
			DWRITE_TEXT_METRICS* pTextMetrics = nullptr)
		{
			if (pManager == nullptr) {
				return E_INVALIDARG;
			}

			HRESULT hr = EnsureDirectWriteFactory();
			if (FAILED(hr)) {
				return hr;
			}

			TFontInfo* pFontInfo = ResolveTextFallbackFontInfoInternal(pManager, iFont);
			if (pFontInfo == nullptr) {
				return E_FAIL;
			}

			const std::wstring sourceText = ToWideText(text);
			const std::wstring fontFamily = ResolveTextFallbackFontFamilyInternal(pFontInfo);
			const float fontSize = static_cast<float>(pManager->ScaleValue(pFontInfo->iSize));
			const D2DTextFormatCacheKey formatKey = BuildDirectWriteTextFormatCacheKeyInternal(
				fontFamily,
				fontSize,
				pFontInfo->bBold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
				pFontInfo->bItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
				uStyle);
			hr = GetCachedTextFormat(formatKey, textFormat.ReleaseAndGetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			const DirectWriteTextLayoutBoundsInternal layoutBounds = ResolveDirectWriteTextLayoutBoundsInternal(rc, uStyle, fontSize);

			D2DTextLayoutCacheKey layoutKey;
			const bool cacheLayout = CanCacheDirectWriteTextLayoutInternal(sourceText);
			if (cacheLayout) {
				layoutKey = BuildDirectWriteTextLayoutCacheKeyInternal(
					formatKey,
					sourceText,
					layoutBounds,
					uStyle,
					pFontInfo->bUnderline,
					pFontInfo->bStrikeout);

				D2DRenderState& state = GetD2DRenderState();
				bool usedCachedLayout = false;
				hr = TryReturnCachedDirectWriteTextLayout(
					state.textLayoutCache,
					layoutKey,
					state.frameMetrics.frameTextLayoutCacheHitCount,
					state.frameMetrics.frameTextMetricsCacheHitCount,
					textLayout,
					pTextMetrics,
					usedCachedLayout);
				if (FAILED(hr)) {
					return hr;
				}
				if (usedCachedLayout) {
					wideText.clear();
					return S_OK;
				}
			}

			std::vector<DWRITE_TEXT_RANGE> hotkeyRanges;
			GetCachedPlainTextNormalization(sourceText, uStyle, wideText, hotkeyRanges);
			hr = GetD2DRenderState().writeFactory->CreateTextLayout(
				wideText.c_str(),
				static_cast<UINT32>(wideText.length()),
				textFormat.Get(),
				layoutBounds.maxWidth,
				layoutBounds.maxHeight,
				textLayout.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			ApplyTextLayoutEndEllipsis(formatKey, textFormat.Get(), textLayout.Get(), uStyle);

			if (!wideText.empty()) {
				const DWRITE_TEXT_RANGE textRange = { 0, static_cast<UINT32>(wideText.length()) };
				if (pFontInfo->bUnderline) {
					textLayout->SetUnderline(TRUE, textRange);
				}
				if (pFontInfo->bStrikeout) {
					textLayout->SetStrikethrough(TRUE, textRange);
				}
			}
			for (const DWRITE_TEXT_RANGE& range : hotkeyRanges) {
				textLayout->SetUnderline(TRUE, range);
			}

			DWRITE_TEXT_METRICS metrics = {};
			bool hasMetrics = false;
			if (cacheLayout || pTextMetrics != nullptr) {
				hr = textLayout->GetMetrics(&metrics);
				if (FAILED(hr)) {
					return hr;
				}
				hasMetrics = true;
				if (pTextMetrics != nullptr) {
					*pTextMetrics = metrics;
				}
			}

			if (cacheLayout) {
				D2DRenderState& state = GetD2DRenderState();
				StoreCachedDirectWriteTextLayout(state, layoutKey, textLayout.Get(), metrics, hasMetrics);
			}

			return S_OK;
		}

		void GetCachedPlainTextNormalization(
			const std::wstring& sourceText,
			UINT uStyle,
			std::wstring& normalizedText,
			std::vector<DWRITE_TEXT_RANGE>& hotkeyRanges)
		{
			const bool cacheNormalization = CanCachePlainTextNormalizationInternal(sourceText);
			PlainTextNormalizationCacheKey cacheKey;
			if (cacheNormalization) {
				cacheKey = BuildPlainTextNormalizationCacheKeyInternal(sourceText, uStyle);

				D2DRenderState& state = GetD2DRenderState();
				auto it = state.plainTextNormalizationCache.find(cacheKey);
				if (it != state.plainTextNormalizationCache.end()) {
					++state.frameMetrics.frameTextNormalizationCacheHitCount;
					normalizedText = it->second.normalizedText;
					hotkeyRanges = it->second.hotkeyRanges;
					return;
				}
			}

			NormalizePlainTextForDirectWriteInternal(sourceText, uStyle, normalizedText, hotkeyRanges);

			if (cacheNormalization) {
				D2DRenderState& state = GetD2DRenderState();
				StoreCachedPlainTextNormalization(state, cacheKey, normalizedText, hotkeyRanges);
			}
		}

		struct HtmlTextStyle
		{
			std::wstring fontFamily;
			float fontSize = 12.0f;
			DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL;
			DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL;
			bool underline = false;
			bool strikeout = false;
			float lineHeight = 0.0f;
			float baseline = 0.0f;
			bool hasBackground = false;
			DWORD backgroundColor = 0;
			DWORD color = 0xFF000000;
		};

		struct HtmlStyleRange
		{
			UINT32 start = 0;
			UINT32 length = 0;
			HtmlTextStyle style;
		};

		struct HtmlLinkRange
		{
			std::wstring target;
			UINT32 start = 0;
			UINT32 length = 0;
		};

		struct HtmlColorRange
		{
			DWORD color = 0;
			UINT32 start = 0;
			UINT32 length = 0;
		};

		struct HtmlBackgroundRange
		{
			DWORD color = 0;
			UINT32 start = 0;
			UINT32 length = 0;
		};

		struct HtmlInlineImage
		{
			const TImageInfo* imageInfo = nullptr;
			RECT sourceRect = { 0, 0, 0, 0 };
			UINT32 position = 0;
			float width = 0.0f;
			float height = 0.0f;
			float baseline = 0.0f;
			bool useAlpha = false;
		};

		struct HtmlParsedText
		{
			std::wstring text;
			std::vector<HtmlStyleRange> ranges;
			std::vector<HtmlLinkRange> links;
			std::vector<HtmlColorRange> colorRanges;
			std::vector<HtmlColorRange> foregroundRanges;
			std::vector<HtmlBackgroundRange> backgroundRanges;
			std::vector<HtmlInlineImage> inlineImages;
			float uniformLineHeight = 0.0f;
			float uniformBaseline = 0.0f;
			unsigned long long drawingEffectSignature = 0;
			unsigned long long layoutStructureSignature = 0;
		};

		size_t GetHtmlSignatureHashSeed()
		{
			return sizeof(size_t) >= 8
				? static_cast<size_t>(1469598103934665603ULL)
				: static_cast<size_t>(2166136261UL);
		}

		unsigned long long BuildHtmlTextLayoutStructureSignature(const HtmlParsedText& parsedText)
		{
			size_t hash = GetHtmlSignatureHashSeed();
			hash = HashCombine(hash, std::hash<size_t>()(parsedText.ranges.size()));
			for (const HtmlStyleRange& range : parsedText.ranges) {
				hash = HashCombine(hash, std::hash<UINT32>()(range.start));
				hash = HashCombine(hash, std::hash<UINT32>()(range.length));
				hash = HashCombine(hash, std::hash<std::wstring>()(ResolveTextFallbackFontFamilyInternal(range.style.fontFamily)));
				hash = HashCombine(hash, std::hash<LONG>()(ToDirectWriteCacheScale100Internal(ResolveTextFallbackFontSizeInternal(range.style.fontSize))));
				hash = HashCombine(hash, std::hash<UINT>()(static_cast<UINT>(range.style.fontWeight)));
				hash = HashCombine(hash, std::hash<UINT>()(static_cast<UINT>(range.style.fontStyle)));
				hash = HashCombine(hash, std::hash<bool>()(range.style.underline));
				hash = HashCombine(hash, std::hash<bool>()(range.style.strikeout));
			}
			return static_cast<unsigned long long>(hash);
		}

		unsigned long long BuildHtmlDrawingEffectSignature(const HtmlParsedText& parsedText)
		{
			size_t hash = GetHtmlSignatureHashSeed();
			hash = HashCombine(hash, std::hash<size_t>()(parsedText.foregroundRanges.size()));
			for (const HtmlColorRange& range : parsedText.foregroundRanges) {
				hash = HashCombine(hash, std::hash<DWORD>()(range.color));
				hash = HashCombine(hash, std::hash<UINT32>()(range.start));
				hash = HashCombine(hash, std::hash<UINT32>()(range.length));
			}
			return static_cast<unsigned long long>(hash);
		}

		struct HtmlParseCacheValue
		{
			HtmlParsedText parsedText;
			HtmlTextStyle baseStyle;
		};

		void StoreCachedHtmlParse(
			D2DRenderState& state,
			const HtmlParseCacheKey& cacheKey,
			const HtmlParsedText& parsedText,
			const HtmlTextStyle& baseStyle)
		{
			std::shared_ptr<HtmlParseCacheValue> cachedValue = std::make_shared<HtmlParseCacheValue>();
			cachedValue->parsedText = parsedText;
			cachedValue->baseStyle = baseStyle;

			++state.frameMetrics.frameHtmlParseCacheRefreshCount;
			StoreCachedValue(
				state.htmlParseCache,
				cacheKey,
				std::move(cachedValue),
				GetDirectWriteHtmlParseCacheLimitInternal());
		}

		struct HtmlLocalHitTestRangeKey
		{
			UINT32 start = 0;
			UINT32 length = 0;

			bool operator==(const HtmlLocalHitTestRangeKey& other) const
			{
				return start == other.start && length == other.length;
			}
		};

		struct HtmlLocalHitTestRangeKeyHasher
		{
			size_t operator()(const HtmlLocalHitTestRangeKey& key) const
			{
				size_t hash = std::hash<UINT32>()(key.start);
				hash = HashCombine(hash, std::hash<UINT32>()(key.length));
				return hash;
			}
		};

		struct HtmlBrushRange
		{
			DWORD color = 0;
			ComPtr<ID2D1SolidColorBrush> brush;
		};

		class HtmlBrushCache
		{
		public:
			HtmlBrushCache(
				ID2D1RenderTarget* pRenderTarget,
				DWORD defaultColor,
				ID2D1SolidColorBrush* pDefaultBrush,
				size_t reserveCapacity)
				: m_pRenderTarget(pRenderTarget),
				m_defaultColor(defaultColor),
				m_pDefaultBrush(pDefaultBrush)
			{
				if (reserveCapacity > 0) {
					m_brushes.reserve(reserveCapacity);
					m_lookup.reserve(reserveCapacity);
				}
			}

			ID2D1SolidColorBrush* EnsureBrush(DWORD color)
			{
				if (color == m_defaultColor) {
					return m_pDefaultBrush;
				}

				auto lookupIt = m_lookup.find(color);
				if (lookupIt != m_lookup.end()) {
					return m_brushes[lookupIt->second].brush.Get();
				}

				HtmlBrushRange item;
				item.color = color;
				if (FAILED(GetCachedSolidColorBrush(m_pRenderTarget, color, item.brush.GetAddressOf()))) {
					return nullptr;
				}

				m_brushes.push_back(item);
				const size_t index = m_brushes.size() - 1;
				m_lookup[color] = index;
				return m_brushes[index].brush.Get();
			}

		private:
			ID2D1RenderTarget* m_pRenderTarget = nullptr;
			DWORD m_defaultColor = 0;
			ID2D1SolidColorBrush* m_pDefaultBrush = nullptr;
			std::vector<HtmlBrushRange> m_brushes;
			std::unordered_map<DWORD, size_t> m_lookup;
		};

		class HtmlHitTestMetricsProvider
		{
		public:
			HtmlHitTestMetricsProvider(IDWriteTextLayout* pTextLayout, bool enablePersistentCache)
				: m_pTextLayout(pTextLayout), m_enablePersistentCache(enablePersistentCache)
			{
			}

			const std::vector<RECT>* Resolve(UINT32 start, UINT32 length)
			{
				if (m_pTextLayout == nullptr || length == 0) {
					return nullptr;
				}

				const HtmlLocalHitTestRangeKey localKey = { start, length };
				auto localIt = m_localCache.find(localKey);
				if (localIt != m_localCache.end()) {
					return &localIt->second;
				}

				D2DRenderState& state = GetD2DRenderState();
				const HtmlHitTestMetricsCacheKey cacheKey = {
					reinterpret_cast<UINT_PTR>(m_pTextLayout),
					start,
					length
				};
				if (m_enablePersistentCache) {
					auto cacheIt = state.htmlHitTestMetricsCache.find(cacheKey);
					if (cacheIt != state.htmlHitTestMetricsCache.end()) {
						++state.frameMetrics.frameHtmlHitTestCacheHitCount;
						auto inserted = m_localCache.emplace(localKey, cacheIt->second);
						return &inserted.first->second;
					}
				}

				std::vector<RECT> rects = BuildMergedHtmlMetricRectsInternal(m_pTextLayout, start, length);
				auto inserted = m_localCache.emplace(localKey, std::move(rects));
				if (m_enablePersistentCache) {
					StoreCachedHtmlHitTestMetrics(state, cacheKey, inserted.first->second);
				}
				return &inserted.first->second;
			}

		private:
			IDWriteTextLayout* m_pTextLayout = nullptr;
			bool m_enablePersistentCache = false;
			std::unordered_map<HtmlLocalHitTestRangeKey, std::vector<RECT>, HtmlLocalHitTestRangeKeyHasher> m_localCache;
		};

		struct HtmlStyleFrame
		{
			HtmlTagType type = HtmlTagType::Bold;
			HtmlTextStyle previousStyle;
			UINT32 linkStart = 0;
			std::wstring linkTarget;
		};

		bool ApplyFontInfoToHtmlStyle(CPaintManagerUI* pManager, TFontInfo* pFontInfo, HtmlTextStyle& style);

		HtmlTextStyle CreateHtmlBaseStyle(CPaintManagerUI* pManager, int iFont, DWORD dwTextColor)
		{
			HtmlTextStyle style;
			ApplyFontInfoToHtmlStyle(pManager, ResolveTextFallbackFontInfoInternal(pManager, iFont), style);

			style.fontFamily = ResolveTextFallbackFontFamilyInternal(style.fontFamily);
			style.fontSize = ResolveTextFallbackFontSizeInternal(style.fontSize);
			if (style.lineHeight <= 0.0f) {
				style.lineHeight = (std::max)(style.fontSize * 1.35f, style.fontSize + 2.0f);
			}
			if (style.baseline <= 0.0f) {
				style.baseline = (std::min)(style.lineHeight - 1.0f, (std::max)(style.fontSize, style.lineHeight * 0.8f));
			}
			style.color = ResolveTextColorInternal(pManager, dwTextColor);
			return style;
		}

		bool ApplyFontInfoToHtmlStyle(CPaintManagerUI* pManager, TFontInfo* pFontInfo, HtmlTextStyle& style)
		{
			if (pManager == nullptr || pFontInfo == nullptr) {
				return false;
			}

			style.fontFamily = ResolveTextFallbackFontFamilyInternal(pFontInfo);
			style.fontSize = static_cast<float>(pManager->ScaleValue(pFontInfo->iSize));
			style.fontWeight = pFontInfo->bBold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
			style.fontStyle = pFontInfo->bItalic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
			style.underline = pFontInfo->bUnderline;
			style.strikeout = pFontInfo->bStrikeout;
			style.lineHeight = static_cast<float>(pFontInfo->tm.tmHeight + pFontInfo->tm.tmExternalLeading);
			style.baseline = static_cast<float>(pFontInfo->tm.tmAscent);
			return true;
		}

		bool ApplyFontSpecToHtmlStyle(CPaintManagerUI* pManager, const std::wstring& spec, HtmlTextStyle& style)
		{
			if (pManager == nullptr) {
				return false;
			}

			int fontId = 0;
			if (TryParseInteger(spec, fontId)) {
				TFontInfo* pFontInfo = ResolveTextFallbackFontInfoInternal(pManager, fontId);
				return ApplyFontInfoToHtmlStyle(pManager, pFontInfo, style);
			}

			std::wstring trimmed = TrimWideString(spec);
			if (trimmed.empty()) {
				return false;
			}

			size_t split = trimmed.find_first_of(L" \t");
			std::wstring fontFamily = split == std::wstring::npos ? trimmed : trimmed.substr(0, split);
			std::wstring rest = split == std::wstring::npos ? std::wstring() : TrimWideString(trimmed.substr(split + 1));
			if (fontFamily.empty()) {
				return false;
			}

			int fontSize = 10;
			size_t attrStart = 0;
			while (attrStart < rest.length() && std::iswspace(rest[attrStart]) != 0) {
				++attrStart;
			}
			if (attrStart < rest.length() && (std::iswdigit(rest[attrStart]) != 0 || rest[attrStart] == L'-' || rest[attrStart] == L'+')) {
				size_t sizeEnd = attrStart + 1;
				while (sizeEnd < rest.length() && std::iswdigit(rest[sizeEnd]) != 0) {
					++sizeEnd;
				}
				StringUtil::TryParseInt(rest.substr(attrStart, sizeEnd - attrStart), fontSize);
				attrStart = sizeEnd;
			}

			std::wstring attrText = TrimWideString(rest.substr(attrStart));
			std::wstring attrLower = attrText;
			std::transform(attrLower.begin(), attrLower.end(), attrLower.begin(), [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });

			style.fontFamily = fontFamily;
			style.fontSize = static_cast<float>(pManager->ScaleValue(fontSize > 0 ? fontSize : 10));
			style.fontWeight = attrLower.find(L"bold") != std::wstring::npos ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
			style.fontStyle = attrLower.find(L"italic") != std::wstring::npos ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
			style.underline = attrLower.find(L"underline") != std::wstring::npos;
			style.strikeout = attrLower.find(L"strikeout") != std::wstring::npos;
			return true;
		}

		bool TryParseHtmlInlineImage(CPaintManagerUI* pManager, const std::wstring& body, HtmlInlineImage& inlineImage)
		{
			if (pManager == nullptr) {
				return false;
			}

			const std::wstring trimmed = TrimWideString(body);
			if (trimmed.empty()) {
				return false;
			}

			std::wstring imageName;
			std::wstring imageResType;
			int imageListCount = 1;
			int imageListIndex = 0;

			if (trimmed.find(L"file='") != std::wstring::npos || trimmed.find(L"res='") != std::wstring::npos ||
				trimmed.find(L"file=\"") != std::wstring::npos || trimmed.find(L"res=\"") != std::wstring::npos) {
				std::unordered_map<std::wstring, std::wstring> attributes;
				if (!ParseHtmlAttributeMap(trimmed, attributes)) {
					return false;
				}
				auto fileIt = attributes.find(L"file");
				auto resIt = attributes.find(L"res");
				auto restypeIt = attributes.find(L"restype");
				if (fileIt != attributes.end()) {
					imageName = fileIt->second;
				}
				else if (resIt != attributes.end()) {
					imageName = resIt->second;
				}
				if (restypeIt != attributes.end()) {
					imageResType = restypeIt->second;
				}
			}
			else {
				std::vector<std::wstring> tokens;
				size_t index = 0;
				while (index < trimmed.length()) {
					while (index < trimmed.length() && std::iswspace(trimmed[index]) != 0) {
						++index;
					}
					if (index >= trimmed.length()) {
						break;
					}
					const size_t start = index;
					while (index < trimmed.length() && std::iswspace(trimmed[index]) == 0) {
						++index;
					}
					tokens.push_back(trimmed.substr(start, index - start));
				}
				if (tokens.empty()) {
					return false;
				}

				imageName = tokens[0];
				if (tokens.size() > 1) {
					TryParseInteger(tokens[1], imageListCount);
				}
				if (tokens.size() > 2) {
					TryParseInteger(tokens[2], imageListIndex);
				}
			}

			if (imageName.empty()) {
				return false;
			}

			const TImageInfo* pImageInfo = imageResType.empty()
				? pManager->GetImageEx(imageName.c_str())
				: pManager->GetImageEx(imageName.c_str(), imageResType.c_str());
			if (pImageInfo == nullptr || pImageInfo->hBitmap == nullptr || pImageInfo->nX <= 0 || pImageInfo->nY <= 0) {
				return false;
			}

			if (imageListCount <= 0) {
				imageListCount = 1;
			}
			if (imageListIndex < 0 || imageListIndex >= imageListCount) {
				imageListIndex = 0;
			}

			int imageWidth = pImageInfo->nX;
			if (imageListCount > 1) {
				imageWidth /= imageListCount;
			}
			if (imageWidth <= 0) {
				return false;
			}

			inlineImage.imageInfo = pImageInfo;
			inlineImage.useAlpha = pImageInfo->bAlpha;
			inlineImage.width = static_cast<float>(imageWidth);
			inlineImage.height = static_cast<float>(pImageInfo->nY);
			inlineImage.baseline = inlineImage.height;
			inlineImage.sourceRect.left = imageWidth * imageListIndex;
			inlineImage.sourceRect.top = 0;
			inlineImage.sourceRect.right = imageWidth * (imageListIndex + 1);
			inlineImage.sourceRect.bottom = pImageInfo->nY;
			return true;
		}

		void FlushHtmlStyleRange(HtmlParsedText& parsedText, UINT32& runStart, const HtmlTextStyle& style)
		{
			const UINT32 textLength = static_cast<UINT32>(parsedText.text.length());
			if (textLength > runStart) {
				HtmlStyleRange range;
				range.start = runStart;
				range.length = textLength - runStart;
				range.style = style;
				parsedText.ranges.push_back(range);

				HtmlColorRange colorRange;
				colorRange.color = style.color;
				colorRange.start = range.start;
				colorRange.length = range.length;
				if (!parsedText.colorRanges.empty()) {
					HtmlColorRange& previous = parsedText.colorRanges.back();
					if (previous.color == colorRange.color &&
						previous.start + previous.length == colorRange.start) {
						previous.length += colorRange.length;
					}
					else {
						parsedText.colorRanges.push_back(colorRange);
					}
				}
				else {
					parsedText.colorRanges.push_back(colorRange);
				}

				if (style.hasBackground) {
					HtmlBackgroundRange backgroundRange;
					backgroundRange.color = style.backgroundColor;
					backgroundRange.start = range.start;
					backgroundRange.length = range.length;
					if (!parsedText.backgroundRanges.empty()) {
						HtmlBackgroundRange& previous = parsedText.backgroundRanges.back();
						if (previous.color == backgroundRange.color &&
							previous.start + previous.length == backgroundRange.start) {
							previous.length += backgroundRange.length;
						}
						else {
							parsedText.backgroundRanges.push_back(backgroundRange);
						}
					}
					else {
						parsedText.backgroundRanges.push_back(backgroundRange);
					}
				}
			}
			runStart = textLength;
		}

		void UpdateHtmlUniformLineMetrics(HtmlParsedText& parsedText, const HtmlTextStyle& style, int paragraphExtra, float inlineHeight = 0.0f)
		{
			const float lineHeight = (std::max)(style.lineHeight, inlineHeight);
			const float baseline = style.baseline > 0.0f ? style.baseline : (std::min)(lineHeight - 1.0f, (std::max)(style.fontSize, lineHeight * 0.8f));
			parsedText.uniformLineHeight = (std::max)(parsedText.uniformLineHeight, lineHeight + static_cast<float>(paragraphExtra));
			parsedText.uniformBaseline = (std::max)(parsedText.uniformBaseline, baseline + static_cast<float>(paragraphExtra));
		}

		void FinalizeHtmlForegroundRanges(HtmlParsedText& parsedText, DWORD baseColor)
		{
			parsedText.foregroundRanges.clear();
			for (const HtmlColorRange& range : parsedText.colorRanges) {
				if (range.length == 0 || range.color == baseColor) {
					continue;
				}

				if (!parsedText.foregroundRanges.empty()) {
					HtmlColorRange& previous = parsedText.foregroundRanges.back();
					if (previous.color == range.color &&
						previous.start + previous.length == range.start) {
						previous.length += range.length;
						continue;
					}
				}

				parsedText.foregroundRanges.push_back(range);
			}
		}

		void AppendHtmlParagraphBreak(HtmlParsedText& parsedText, UINT32& runStart, const HtmlTextStyle& style)
		{
			if (!parsedText.text.empty() && parsedText.text.back() == L'\n') {
				runStart = static_cast<UINT32>(parsedText.text.length());
				return;
			}

			FlushHtmlStyleRange(parsedText, runStart, style);
			parsedText.text.push_back(L'\n');
			runStart = static_cast<UINT32>(parsedText.text.length());
		}

		struct HtmlDirectWriteParseState
		{
			HtmlTextStyle currentStyle;
			UINT32 runStart = 0;
			std::vector<HtmlStyleFrame> frames;
			std::vector<int> paragraphExtraStack = std::vector<int>(1, 0);
			bool inRaw = false;
		};

		int GetCurrentHtmlParagraphExtra(const HtmlDirectWriteParseState& state)
		{
			return state.paragraphExtraStack.empty() ? 0 : state.paragraphExtraStack.back();
		}

		bool TryHandleHtmlClosingTagForDirectWrite(
			HtmlTagType tagType,
			UINT uStyle,
			HtmlParsedText& parsedText,
			HtmlDirectWriteParseState& state)
		{
			if (tagType == HtmlTagType::Paragraph) {
				if (state.paragraphExtraStack.size() <= 1) {
					return false;
				}
				if ((uStyle & DT_SINGLELINE) == 0 && !parsedText.text.empty()) {
					AppendHtmlParagraphBreak(parsedText, state.runStart, state.currentStyle);
				}
				state.paragraphExtraStack.pop_back();
				return true;
			}

			if (state.frames.empty() || state.frames.back().type != tagType) {
				return false;
			}

			FlushHtmlStyleRange(parsedText, state.runStart, state.currentStyle);
			const HtmlStyleFrame frame = state.frames.back();
			state.frames.pop_back();
			if (tagType == HtmlTagType::Link && static_cast<UINT32>(parsedText.text.length()) > frame.linkStart) {
				HtmlLinkRange linkRange;
				linkRange.target = frame.linkTarget;
				linkRange.start = frame.linkStart;
				linkRange.length = static_cast<UINT32>(parsedText.text.length()) - frame.linkStart;
				parsedText.links.push_back(linkRange);
			}

			state.currentStyle = frame.previousStyle;
			state.runStart = static_cast<UINT32>(parsedText.text.length());
			return true;
		}

		bool TryHandleHtmlInlineImageTagForDirectWrite(
			CPaintManagerUI* pManager,
			const std::wstring& body,
			HtmlParsedText& parsedText,
			HtmlDirectWriteParseState& state)
		{
			if (body.empty()) {
				return false;
			}

			FlushHtmlStyleRange(parsedText, state.runStart, state.currentStyle);

			HtmlInlineImage inlineImage;
			if (!TryParseHtmlInlineImage(pManager, body, inlineImage)) {
				return false;
			}

			inlineImage.position = static_cast<UINT32>(parsedText.text.length());
			parsedText.inlineImages.push_back(inlineImage);
			parsedText.text.push_back(L'\xFFFC');
			UpdateHtmlUniformLineMetrics(parsedText, state.currentStyle, GetCurrentHtmlParagraphExtra(state), inlineImage.height);
			state.runStart = static_cast<UINT32>(parsedText.text.length());
			return true;
		}

		bool TryHandleHtmlOpeningTagForDirectWrite(
			CPaintManagerUI* pManager,
			HtmlTagType tagType,
			const std::wstring& body,
			DWORD dwTextColor,
			UINT uStyle,
			const std::wstring& hoverLink,
			const HtmlTextStyle& baseStyle,
			HtmlParsedText& parsedText,
			HtmlDirectWriteParseState& state)
		{
			if (tagType == HtmlTagType::Italic && !body.empty()) {
				return TryHandleHtmlInlineImageTagForDirectWrite(pManager, body, parsedText, state);
			}

			if (tagType == HtmlTagType::Paragraph) {
				int paragraphExtra = 0;
				const std::wstring trimmedBody = TrimWideString(body);
				if (!trimmedBody.empty() && !TryParseInteger(trimmedBody, paragraphExtra)) {
					return false;
				}

				paragraphExtra = (std::max)(0, paragraphExtra);
				if ((uStyle & DT_SINGLELINE) == 0 && !parsedText.text.empty()) {
					AppendHtmlParagraphBreak(parsedText, state.runStart, state.currentStyle);
				}
				state.paragraphExtraStack.push_back(paragraphExtra);
				UpdateHtmlUniformLineMetrics(parsedText, state.currentStyle, GetCurrentHtmlParagraphExtra(state));
				return true;
			}

			FlushHtmlStyleRange(parsedText, state.runStart, state.currentStyle);

			HtmlStyleFrame frame;
			frame.type = tagType;
			frame.previousStyle = state.currentStyle;
			if (tagType == HtmlTagType::Link) {
				frame.linkStart = static_cast<UINT32>(parsedText.text.length());
				frame.linkTarget = body;
			}
			state.frames.push_back(frame);

			switch (tagType)
			{
			case HtmlTagType::Link:
			{
				DWORD linkColor = baseStyle.color;
				if (dwTextColor == 0) {
					linkColor = ResolveTextColorInternal(pManager, pManager->GetDefaultLinkFontColor());
				}
				if (!hoverLink.empty() || !body.empty()) {
					if (hoverLink == body) {
						linkColor = ResolveTextColorInternal(pManager, pManager->GetDefaultLinkHoverFontColor());
					}
				}
				state.currentStyle.color = linkColor;
				state.currentStyle.underline = true;
			}
			break;
			case HtmlTagType::Bold:
				state.currentStyle.fontWeight = DWRITE_FONT_WEIGHT_BOLD;
				break;
			case HtmlTagType::Color:
			{
				DWORD color = 0;
				if (!TryParseHexColor(body, color)) {
					return false;
				}
				state.currentStyle.color = ResolveTextColorInternal(pManager, color);
			}
			break;
			case HtmlTagType::Font:
				if (!ApplyFontSpecToHtmlStyle(pManager, body, state.currentStyle)) {
					return false;
				}
				UpdateHtmlUniformLineMetrics(parsedText, state.currentStyle, GetCurrentHtmlParagraphExtra(state));
				break;
			case HtmlTagType::Italic:
				state.currentStyle.fontStyle = DWRITE_FONT_STYLE_ITALIC;
				break;
			case HtmlTagType::Selected:
				state.currentStyle.hasBackground = true;
				state.currentStyle.backgroundColor = ResolveTextColorInternal(pManager, pManager->GetDefaultSelectedBkColor());
				break;
			case HtmlTagType::Underline:
				state.currentStyle.underline = true;
				break;
			case HtmlTagType::Raw:
				state.inRaw = true;
				break;
			default:
				break;
			}

			state.runStart = static_cast<UINT32>(parsedText.text.length());
			return true;
		}

		bool AppendHtmlInlineSpacer(
			const std::wstring& body,
			bool vertical,
			const HtmlTextStyle& currentStyle,
			HtmlParsedText& parsedText,
			UINT32& runStart)
		{
			if (TrimWideString(body).empty()) {
				return true;
			}

			int value = 0;
			if (!TryParseInteger(body, value)) {
				return false;
			}
			value = (std::max)(0, value);
			if (value == 0) {
				return true;
			}

			FlushHtmlStyleRange(parsedText, runStart, currentStyle);

			HtmlInlineImage spacer;
			spacer.position = static_cast<UINT32>(parsedText.text.length());
			if (vertical) {
				spacer.width = 0.0f;
				spacer.height = static_cast<float>(value);
				spacer.baseline = spacer.height;
			}
			else {
				const float inlineHeight = currentStyle.fontSize > 1.0f ? currentStyle.fontSize : 1.0f;
				spacer.width = static_cast<float>(value);
				spacer.height = inlineHeight;
				spacer.baseline = inlineHeight;
			}

			parsedText.inlineImages.push_back(spacer);
			parsedText.text.push_back(L'\xFFFC');
			runStart = static_cast<UINT32>(parsedText.text.length());
			return true;
		}

		bool TryParseHtmlTextForDirectWrite(
			CPaintManagerUI* pManager,
			std::wstring_view text,
			DWORD dwTextColor,
			int iFont,
			UINT uStyle,
			const std::wstring& hoverLink,
			HtmlParsedText& parsedText,
			HtmlTextStyle& baseStyle)
		{
			if (pManager == nullptr) {
				return false;
			}

			parsedText = HtmlParsedText();
			baseStyle = CreateHtmlBaseStyle(pManager, iFont, dwTextColor);
			HtmlDirectWriteParseState state;
			state.currentStyle = baseStyle;
			parsedText.uniformLineHeight = baseStyle.lineHeight;
			parsedText.uniformBaseline = baseStyle.baseline;
			std::wstring source = ToWideText(text);

			for (size_t index = 0; index < source.length();) {
				if (state.inRaw) {
					size_t rawIndex = index;
					bool isClosing = false;
					HtmlTagType tagType = HtmlTagType::Raw;
					std::wstring body;
					if (TryReadHtmlTag(source, rawIndex, isClosing, tagType, body) && isClosing && tagType == HtmlTagType::Raw) {
						if (state.frames.empty() || state.frames.back().type != HtmlTagType::Raw) {
							return false;
						}
						state.frames.pop_back();
						state.inRaw = false;
						index = rawIndex;
						continue;
					}

					parsedText.text.push_back(source[index++]);
					continue;
				}

				if (index + 2 < source.length()) {
					if (source[index] == L'<' && source[index + 2] == L'>' && (source[index + 1] == L'{' || source[index + 1] == L'}')) {
						parsedText.text.push_back(source[index + 1]);
						index += 3;
						continue;
					}
					if (source[index] == L'{' && source[index + 2] == L'}' && (source[index + 1] == L'<' || source[index + 1] == L'>')) {
						parsedText.text.push_back(source[index + 1]);
						index += 3;
						continue;
					}
				}

				if (source[index] == L'\r') {
					++index;
					continue;
				}

				if (source[index] == L'\n') {
					if ((uStyle & DT_SINGLELINE) != 0) {
						break;
					}
					parsedText.text.push_back(L'\n');
					++index;
					continue;
				}

				bool isClosing = false;
				HtmlTagType tagType = HtmlTagType::Bold;
				std::wstring body;
				size_t tagIndex = index;
				if (TryReadHtmlTag(source, tagIndex, isClosing, tagType, body)) {
					if (isClosing) {
						if (!TryHandleHtmlClosingTagForDirectWrite(tagType, uStyle, parsedText, state)) {
							return false;
						}
						index = tagIndex;
						continue;
					}

					if (!TryHandleHtmlOpeningTagForDirectWrite(
						pManager,
						tagType,
						body,
						dwTextColor,
						uStyle,
						hoverLink,
						baseStyle,
						parsedText,
						state)) {
						return false;
					}
					index = tagIndex;
					continue;
				}

				if (source[index] == L'<' || source[index] == L'{') {
					if (index + 1 < source.length()) {
						const wchar_t tagName = static_cast<wchar_t>(std::towlower(source[index + 1]));
						if (tagName == L'n') {
							if ((uStyle & DT_SINGLELINE) == 0) {
								const wchar_t closeDelim = source[index] == L'<' ? L'>' : L'}';
								if (index + 2 < source.length() && source[index + 2] == closeDelim) {
									parsedText.text.push_back(L'\n');
									index += 3;
									continue;
								}
							}
							else {
								const wchar_t closeDelim = source[index] == L'<' ? L'>' : L'}';
								if (index + 2 < source.length() && source[index + 2] == closeDelim) {
									index += 3;
									continue;
								}
							}
						}
						if (tagName == L'x' || tagName == L'y') {
							std::wstring spacerBody;
							size_t nextIndex = index;
							if (!TryReadHtmlInlineTagBody(source, index, tagName, spacerBody, nextIndex)) {
								return false;
							}
							if (!AppendHtmlInlineSpacer(spacerBody, tagName == L'y', state.currentStyle, parsedText, state.runStart)) {
								return false;
							}
							index = nextIndex;
							continue;
						}
					}
				}

				parsedText.text.push_back(source[index++]);
			}

			if (state.inRaw || !state.frames.empty() || state.paragraphExtraStack.size() != 1) {
				return false;
			}

			FlushHtmlStyleRange(parsedText, state.runStart, state.currentStyle);
			FinalizeHtmlForegroundRanges(parsedText, baseStyle.color);
			parsedText.drawingEffectSignature = BuildHtmlDrawingEffectSignature(parsedText);
			parsedText.layoutStructureSignature = BuildHtmlTextLayoutStructureSignature(parsedText);
			return true;
		}

		HRESULT CreateHtmlTextLayout(
			CPaintManagerUI* pManager,
			const HtmlParsedText& parsedText,
			const HtmlTextStyle& baseStyle,
			UINT uStyle,
			const RECT& rc,
			ID2D1RenderTarget* pRenderTarget,
			ComPtr<IDWriteTextFormat>& textFormat,
			ComPtr<IDWriteTextLayout>& textLayout,
			DWRITE_TEXT_METRICS* pTextMetrics = nullptr)
		{
			if (pManager == nullptr) {
				return E_INVALIDARG;
			}

			HRESULT hr = EnsureDirectWriteFactory();
			if (FAILED(hr)) {
				return hr;
			}

			const std::wstring fontFamily = ResolveTextFallbackFontFamilyInternal(baseStyle.fontFamily);
			const float fontSize = ResolveTextFallbackFontSizeInternal(baseStyle.fontSize);
			const bool useCustomLineSpacing =
				parsedText.uniformLineHeight > baseStyle.lineHeight + 0.01f ||
				parsedText.uniformBaseline > baseStyle.baseline + 0.01f;
			const D2DTextFormatCacheKey formatKey = BuildDirectWriteTextFormatCacheKeyInternal(
				fontFamily,
				fontSize,
				baseStyle.fontWeight,
				baseStyle.fontStyle,
				uStyle,
				useCustomLineSpacing ? parsedText.uniformLineHeight : 0.0f,
				useCustomLineSpacing ? parsedText.uniformBaseline : 0.0f);
			hr = GetCachedTextFormat(formatKey, textFormat.ReleaseAndGetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			const DirectWriteTextLayoutBoundsInternal layoutBounds = ResolveDirectWriteTextLayoutBoundsInternal(rc, uStyle, fontSize, 32.0f);

			D2DRenderState& state = GetD2DRenderState();
			const bool cacheLayout = CanCacheDirectWriteHtmlTextLayoutInternal(parsedText.text, !parsedText.inlineImages.empty());
			HtmlTextLayoutCacheKey layoutKey;
			if (cacheLayout) {
				const unsigned long long layoutStructureSignature = parsedText.layoutStructureSignature != 0
					? parsedText.layoutStructureSignature
					: BuildHtmlTextLayoutStructureSignature(parsedText);
				layoutKey = BuildDirectWriteHtmlTextLayoutCacheKeyInternal(
					formatKey,
					parsedText.text,
					layoutStructureSignature,
					layoutBounds,
					uStyle);

				bool usedCachedLayout = false;
				hr = TryReturnCachedDirectWriteTextLayout(
					state.htmlTextLayoutCache,
					layoutKey,
					state.frameMetrics.frameHtmlLayoutCacheHitCount,
					state.frameMetrics.frameHtmlMetricsCacheHitCount,
					textLayout,
					pTextMetrics,
					usedCachedLayout);
				if (FAILED(hr)) {
					return hr;
				}
				if (usedCachedLayout) {
					return S_OK;
				}
			}

			hr = state.writeFactory->CreateTextLayout(
				parsedText.text.c_str(),
				static_cast<UINT32>(parsedText.text.length()),
				textFormat.Get(),
				layoutBounds.maxWidth,
				layoutBounds.maxHeight,
				textLayout.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			ApplyTextLayoutEndEllipsis(formatKey, textFormat.Get(), textLayout.Get(), uStyle);

			for (const HtmlStyleRange& range : parsedText.ranges) {
				if (range.length == 0) {
					continue;
				}

				const DWRITE_TEXT_RANGE textRange = { range.start, range.length };
				const std::wstring rangeFontFamily = ResolveTextFallbackFontFamilyInternal(range.style.fontFamily);
				textLayout->SetFontFamilyName(rangeFontFamily.c_str(), textRange);
				textLayout->SetFontSize(ResolveTextFallbackFontSizeInternal(range.style.fontSize), textRange);
				textLayout->SetFontWeight(range.style.fontWeight, textRange);
				textLayout->SetFontStyle(range.style.fontStyle, textRange);
				textLayout->SetUnderline(range.style.underline ? TRUE : FALSE, textRange);
				textLayout->SetStrikethrough(range.style.strikeout ? TRUE : FALSE, textRange);
			}

			class DWriteInlineImage final : public IDWriteInlineObject
			{
			public:
				DWriteInlineImage(ID2D1RenderTarget* pTarget, const HtmlInlineImage& inlineImage)
					: m_refCount(1), m_renderTarget(pTarget), m_sourceRect(inlineImage.sourceRect)
				{
					m_metrics.width = inlineImage.width;
					m_metrics.height = inlineImage.height;
					m_metrics.baseline = inlineImage.baseline > 0.0f ? inlineImage.baseline : inlineImage.height;
					m_metrics.supportsSideways = FALSE;

					m_overhangs.left = 0;
					m_overhangs.top = 0;
					m_overhangs.right = 0;
					m_overhangs.bottom = 0;

					if (m_renderTarget != nullptr && inlineImage.imageInfo != nullptr && inlineImage.imageInfo->hBitmap != nullptr) {
						GetCachedD2DBitmapFromHBITMAP(m_renderTarget.Get(), inlineImage.imageInfo->hBitmap, inlineImage.useAlpha, m_bitmap.GetAddressOf());
					}
				}

				IFACEMETHOD(QueryInterface)(REFIID iid, void** ppvObject) override
				{
					if (ppvObject == nullptr) {
						return E_INVALIDARG;
					}
					*ppvObject = nullptr;
					if (iid == __uuidof(IUnknown) || iid == __uuidof(IDWriteInlineObject)) {
						*ppvObject = static_cast<IDWriteInlineObject*>(this);
						AddRef();
						return S_OK;
					}
					return E_NOINTERFACE;
				}

				IFACEMETHOD_(ULONG, AddRef)() override
				{
					return ++m_refCount;
				}

				IFACEMETHOD_(ULONG, Release)() override
				{
					const ULONG count = --m_refCount;
					if (count == 0) {
						delete this;
					}
					return count;
				}

				IFACEMETHOD(Draw)(
					void*,
					IDWriteTextRenderer*,
					FLOAT originX,
					FLOAT originY,
					BOOL,
					BOOL,
					IUnknown*) override
				{
					if (m_renderTarget == nullptr || m_bitmap == nullptr) {
						return S_OK;
					}

					const D2D1_RECT_F destRect = D2D1::RectF(
						originX,
						originY,
						originX + m_metrics.width,
						originY + m_metrics.height);
					const D2D1_RECT_F sourceRect = ToD2DRectF(m_sourceRect);
					m_renderTarget->DrawBitmap(m_bitmap.Get(), destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &sourceRect);
					return S_OK;
				}

				IFACEMETHOD(GetMetrics)(DWRITE_INLINE_OBJECT_METRICS* metrics) override
				{
					if (metrics == nullptr) {
						return E_INVALIDARG;
					}
					*metrics = m_metrics;
					return S_OK;
				}

				IFACEMETHOD(GetOverhangMetrics)(DWRITE_OVERHANG_METRICS* overhangs) override
				{
					if (overhangs == nullptr) {
						return E_INVALIDARG;
					}
					*overhangs = m_overhangs;
					return S_OK;
				}

				IFACEMETHOD(GetBreakConditions)(DWRITE_BREAK_CONDITION* breakConditionBefore, DWRITE_BREAK_CONDITION* breakConditionAfter) override
				{
					if (breakConditionBefore == nullptr || breakConditionAfter == nullptr) {
						return E_INVALIDARG;
					}
					*breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
					*breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;
					return S_OK;
				}

			private:
				ULONG m_refCount;
				ComPtr<ID2D1RenderTarget> m_renderTarget;
				ComPtr<ID2D1Bitmap> m_bitmap;
				DWRITE_INLINE_OBJECT_METRICS m_metrics = {};
				DWRITE_OVERHANG_METRICS m_overhangs = {};
				RECT m_sourceRect = { 0, 0, 0, 0 };
			};

			for (const HtmlInlineImage& inlineImage : parsedText.inlineImages) {
				DWriteInlineImage* pInlineImage = new (std::nothrow) DWriteInlineImage(pRenderTarget, inlineImage);
				if (pInlineImage == nullptr) {
					return E_OUTOFMEMORY;
				}

				const DWRITE_TEXT_RANGE textRange = { inlineImage.position, 1 };
				hr = textLayout->SetInlineObject(pInlineImage, textRange);
				pInlineImage->Release();
				if (FAILED(hr)) {
					return hr;
				}
			}

			DWRITE_TEXT_METRICS metrics = {};
			bool hasMetrics = false;
			if (cacheLayout || pTextMetrics != nullptr) {
				hr = textLayout->GetMetrics(&metrics);
				if (FAILED(hr)) {
					return hr;
				}
				hasMetrics = true;
				if (pTextMetrics != nullptr) {
					*pTextMetrics = metrics;
				}
			}

			if (cacheLayout) {
				StoreCachedDirectWriteHtmlTextLayout(state, layoutKey, textLayout.Get(), metrics, hasMetrics);
			}

			return S_OK;
		}

		void PopulateHtmlLinkRects(
			IDWriteTextLayout* pTextLayout,
			const HtmlParsedText& parsedText,
			const RECT& rc,
			RECT* prcLinks,
			std::wstring* sLinks,
			int linkCapacity,
			int& nLinkRects,
			HtmlHitTestMetricsProvider& hitTestMetrics)
		{
			nLinkRects = 0;
			if (pTextLayout == nullptr || prcLinks == nullptr || sLinks == nullptr || linkCapacity <= 0) {
				return;
			}

			for (const HtmlLinkRange& link : parsedText.links) {
				if (link.length == 0 || nLinkRects >= linkCapacity) {
					continue;
				}

				const std::vector<RECT>* pRects = hitTestMetrics.Resolve(link.start, link.length);
				if (pRects == nullptr || pRects->empty()) {
					continue;
				}

				const wchar_t* pLinkTarget = link.target.c_str();
				for (const RECT& rcMetric : *pRects) {
					if (!IsRectValid(rcMetric) || nLinkRects >= linkCapacity) {
						continue;
					}

					prcLinks[nLinkRects] = OffsetHtmlMetricRectInternal(rcMetric, rc);
					sLinks[nLinkRects] = pLinkTarget;
					++nLinkRects;
				}
			}
		}

		void FillHtmlBackgroundRanges(
			ID2D1RenderTarget* pRenderTarget,
			IDWriteTextLayout* pTextLayout,
			const HtmlParsedText& parsedText,
			const RECT& rc,
			HtmlBrushCache& brushCache,
			HtmlHitTestMetricsProvider& hitTestMetrics)
		{
			if (pRenderTarget == nullptr || pTextLayout == nullptr) {
				return;
			}

			for (const HtmlBackgroundRange& range : parsedText.backgroundRanges) {
				if (range.length == 0) {
					continue;
				}

				ID2D1SolidColorBrush* pBrush = brushCache.EnsureBrush(range.color);
				if (pBrush == nullptr) {
					continue;
				}

				const std::vector<RECT>* pRects = hitTestMetrics.Resolve(range.start, range.length);
				if (pRects == nullptr || pRects->empty()) {
					continue;
				}

				for (const RECT& rcMetric : *pRects) {
					if (!IsRectValid(rcMetric)) {
						continue;
					}
					pRenderTarget->FillRectangle(ToD2DRectF(OffsetHtmlMetricRectInternal(rcMetric, rc)), pBrush);
				}
			}
		}

		struct HtmlDrawingEffectPlan
		{
			HtmlDrawingEffectState* pEffectState = nullptr;
			bool applyDrawingEffects = false;
			bool clearDrawingEffects = false;
		};

		template<typename TEffectStates>
		HtmlDrawingEffectPlan ResolveHtmlDrawingEffectPlan(
			TEffectStates& effectStates,
			UINT& cacheHitCounter,
			UINT_PTR textLayoutId,
			bool cacheHtmlLayout,
			bool hasCustomForeground,
			unsigned long long drawingEffectSignature)
		{
			HtmlDrawingEffectPlan plan;
			plan.applyDrawingEffects = hasCustomForeground;

			if (!cacheHtmlLayout) {
				return plan;
			}

			auto effectStateIt = effectStates.find(textLayoutId);
			if (effectStateIt == effectStates.end()) {
				return plan;
			}

			plan.pEffectState = &effectStateIt->second;
			if (plan.pEffectState->initialized &&
				plan.pEffectState->signature == drawingEffectSignature &&
				plan.pEffectState->hasCustomForeground == hasCustomForeground) {
				++cacheHitCounter;
				plan.applyDrawingEffects = false;
			}
			else if (plan.pEffectState->initialized && plan.pEffectState->hasCustomForeground) {
				plan.clearDrawingEffects = true;
			}

			return plan;
		}

		void RecordHtmlDrawingEffectState(
			HtmlDrawingEffectState& effectState,
			unsigned long long drawingEffectSignature,
			bool hasCustomForeground)
		{
			effectState.signature = drawingEffectSignature;
			effectState.initialized = true;
			effectState.hasCustomForeground = hasCustomForeground;
		}

		bool ApplyHtmlDrawingEffects(
			D2DRenderState& state,
			IDWriteTextLayout* pTextLayout,
			const HtmlParsedText& parsedText,
			HtmlBrushCache& brushCache,
			const HtmlDrawingEffectPlan& plan,
			bool cacheHtmlLayout,
			unsigned long long drawingEffectSignature,
			bool hasCustomForeground)
		{
			if (plan.applyDrawingEffects || plan.clearDrawingEffects) {
				if (plan.clearDrawingEffects && !parsedText.text.empty()) {
					const DWRITE_TEXT_RANGE fullRange = { 0, static_cast<UINT32>(parsedText.text.length()) };
					pTextLayout->SetDrawingEffect(nullptr, fullRange);
				}

				if (plan.applyDrawingEffects) {
					for (const HtmlColorRange& range : parsedText.foregroundRanges) {
						if (range.length == 0) {
							continue;
						}

						ID2D1SolidColorBrush* pBrush = brushCache.EnsureBrush(range.color);
						if (pBrush == nullptr) {
							++state.frameMetrics.frameHtmlDirectWriteRenderFailureCount;
							return false;
						}

						const DWRITE_TEXT_RANGE textRange = { range.start, range.length };
						pTextLayout->SetDrawingEffect(pBrush, textRange);
					}
				}

				if (cacheHtmlLayout && plan.pEffectState != nullptr) {
					++state.frameMetrics.frameHtmlDrawingEffectCacheRefreshCount;
					RecordHtmlDrawingEffectState(*plan.pEffectState, drawingEffectSignature, hasCustomForeground);
				}
			}
			else if (cacheHtmlLayout && plan.pEffectState != nullptr && !plan.pEffectState->initialized) {
				RecordHtmlDrawingEffectState(*plan.pEffectState, drawingEffectSignature, hasCustomForeground);
			}

			return true;
		}

		bool DrawHtmlTextInternal(
			CPaintRenderContext& renderContext,
			RECT& rc,
			std::wstring_view text,
			DWORD dwTextColor,
			RECT* prcLinks,
			std::wstring* sLinks,
			int& nLinkRects,
			int iFont,
			UINT uStyle)
		{
			CPaintManagerUI* pManager = renderContext.GetManager();
			if (!CanUseDirect2DRenderContext(renderContext) || pManager == nullptr || ::IsRectEmpty(&rc)) {
				return false;
			}

			const int linkCapacity = nLinkRects;
			std::wstring hoverLink;
			if (prcLinks != nullptr && sLinks != nullptr && linkCapacity > 0) {
				const POINT ptMouse = pManager->GetMousePos();
				for (int i = 0; i < linkCapacity; ++i) {
					if (::PtInRect(prcLinks + i, ptMouse)) {
						hoverLink = *(sLinks + i);
						break;
					}
				}
			}

			HtmlParsedText parsedText;
			HtmlTextStyle baseStyle;
			const std::wstring htmlSourceText = ToWideText(text);
			const HtmlParseCacheKey parseCacheKey = BuildDirectWriteHtmlParseCacheKeyInternal(
				reinterpret_cast<UINT_PTR>(pManager),
				htmlSourceText,
				hoverLink,
				dwTextColor,
				iFont,
				uStyle);

			D2DRenderState& state = GetD2DRenderState();
			auto parseCacheIt = state.htmlParseCache.find(parseCacheKey);
			if (parseCacheIt != state.htmlParseCache.end() && parseCacheIt->second) {
				++state.frameMetrics.frameHtmlParseCacheHitCount;
				parsedText = parseCacheIt->second->parsedText;
				baseStyle = parseCacheIt->second->baseStyle;
			}
			else {
				if (!TryParseHtmlTextForDirectWrite(pManager, text, dwTextColor, iFont, uStyle, hoverLink, parsedText, baseStyle)) {
					++state.frameMetrics.frameHtmlDirectWriteParseFailureCount;
					return false;
				}

				if (CanCacheDirectWriteHtmlParseInternal(htmlSourceText, !parsedText.inlineImages.empty())) {
					StoreCachedHtmlParse(state, parseCacheKey, parsedText, baseStyle);
				}
			}

			ComPtr<IDWriteTextFormat> textFormat;
			ComPtr<IDWriteTextLayout> textLayout;
			DWRITE_TEXT_METRICS metrics = {};
			if ((uStyle & DT_CALCRECT) != 0) {
				if (FAILED(CreateHtmlTextLayout(pManager, parsedText, baseStyle, uStyle, rc, nullptr, textFormat, textLayout, &metrics))) {
					++state.frameMetrics.frameHtmlDirectWriteLayoutFailureCount;
					return false;
				}
				ApplyDirectWriteTextMetricsToRectInternal(metrics, rc);
				nLinkRects = 0;
				return true;
			}

			D2DDrawScope drawScope(renderContext, rc);
			if (!drawScope) {
				++state.frameMetrics.frameHtmlDirectWriteRenderFailureCount;
				return false;
			}
			if (FAILED(CreateHtmlTextLayout(pManager, parsedText, baseStyle, uStyle, rc, drawScope.Get(), textFormat, textLayout))) {
				++state.frameMetrics.frameHtmlDirectWriteLayoutFailureCount;
				return false;
			}

			const bool cacheHtmlLayout = CanCacheDirectWriteHtmlTextLayoutInternal(parsedText.text, !parsedText.inlineImages.empty());
			const bool hasCustomForeground = !parsedText.foregroundRanges.empty();
			const bool hasHtmlBackgroundWork = !parsedText.backgroundRanges.empty();
			const bool hasHtmlLinkWork = prcLinks != nullptr && sLinks != nullptr && linkCapacity > 0 && !parsedText.links.empty();
			const bool hasHtmlHitTestWork = hasHtmlBackgroundWork || hasHtmlLinkWork;

			ComPtr<ID2D1SolidColorBrush> defaultBrush;
			if (FAILED(GetCachedSolidColorBrush(drawScope.Get(), baseStyle.color, defaultBrush.GetAddressOf()))) {
				++state.frameMetrics.frameHtmlDirectWriteRenderFailureCount;
				return false;
			}
			ID2D1SolidColorBrush* pDefaultBrush = defaultBrush.Get();
			if (pDefaultBrush == nullptr) {
				++state.frameMetrics.frameHtmlDirectWriteRenderFailureCount;
				return false;
			}

			const size_t htmlBrushCapacity = parsedText.foregroundRanges.size() + parsedText.backgroundRanges.size();
			HtmlBrushCache htmlBrushCache(drawScope.Get(), baseStyle.color, defaultBrush.Get(), htmlBrushCapacity);

			const unsigned long long drawingEffectSignature = hasCustomForeground ? parsedText.drawingEffectSignature : 0ULL;
			const UINT_PTR textLayoutId = reinterpret_cast<UINT_PTR>(textLayout.Get());
			const HtmlDrawingEffectPlan effectPlan = ResolveHtmlDrawingEffectPlan(
				state.htmlDrawingEffectStates,
				state.frameMetrics.frameHtmlDrawingEffectCacheHitCount,
				textLayoutId,
				cacheHtmlLayout,
				hasCustomForeground,
				drawingEffectSignature);
			if (!ApplyHtmlDrawingEffects(
				state,
				textLayout.Get(),
				parsedText,
				htmlBrushCache,
				effectPlan,
				cacheHtmlLayout,
				drawingEffectSignature,
				hasCustomForeground)) {
				return false;
			}

			if (!hasCustomForeground && !hasHtmlHitTestWork) {
				nLinkRects = 0;
				D2D1_DRAW_TEXT_OPTIONS drawOptions = D2D1_DRAW_TEXT_OPTIONS_NONE;
				if ((uStyle & DT_NOCLIP) == 0) {
					drawOptions = static_cast<D2D1_DRAW_TEXT_OPTIONS>(drawOptions | D2D1_DRAW_TEXT_OPTIONS_CLIP);
				}

				drawScope.Get()->DrawTextLayout(
					D2D1::Point2F(static_cast<float>(rc.left), static_cast<float>(rc.top)),
					textLayout.Get(),
					pDefaultBrush,
					drawOptions);
				return true;
			}

			const bool enableHtmlHitTestCache = hasHtmlHitTestWork && cacheHtmlLayout;
			HtmlHitTestMetricsProvider hitTestMetrics(textLayout.Get(), enableHtmlHitTestCache);
			HtmlHitTestMetricsProvider* pHitTestMetrics = hasHtmlHitTestWork ? &hitTestMetrics : nullptr;
			if (hasHtmlHitTestWork) {
				// Stack allocation avoids a tiny heap allocation on every html draw with hit-testing.
			}

			if (hasHtmlBackgroundWork && pHitTestMetrics != nullptr) {
				FillHtmlBackgroundRanges(drawScope.Get(), textLayout.Get(), parsedText, rc, htmlBrushCache, *pHitTestMetrics);
			}
			if (hasHtmlLinkWork && pHitTestMetrics != nullptr) {
				PopulateHtmlLinkRects(textLayout.Get(), parsedText, rc, prcLinks, sLinks, linkCapacity, nLinkRects, *pHitTestMetrics);
			}
			else {
				nLinkRects = 0;
			}

			D2D1_DRAW_TEXT_OPTIONS drawOptions = D2D1_DRAW_TEXT_OPTIONS_NONE;
			if ((uStyle & DT_NOCLIP) == 0) {
				drawOptions = static_cast<D2D1_DRAW_TEXT_OPTIONS>(drawOptions | D2D1_DRAW_TEXT_OPTIONS_CLIP);
			}

			drawScope.Get()->DrawTextLayout(
				D2D1::Point2F(static_cast<float>(rc.left), static_cast<float>(rc.top)),
				textLayout.Get(),
				pDefaultBrush,
				drawOptions);
			return true;
		}

		bool DrawTextInternal(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, int iFont, UINT uStyle)
		{
			CPaintManagerUI* pManager = renderContext.GetManager();
			if (!CanUseDirect2DRenderContext(renderContext) || pManager == nullptr) {
				return false;
			}

			std::wstring wideText;
			ComPtr<IDWriteTextFormat> textFormat;
			ComPtr<IDWriteTextLayout> textLayout;
			DWRITE_TEXT_METRICS metrics = {};
			DWRITE_TEXT_METRICS* pMetrics = ((uStyle & DT_CALCRECT) != 0) ? &metrics : nullptr;
			if (FAILED(CreateTextLayout(pManager, text, iFont, uStyle, rc, wideText, textFormat, textLayout, pMetrics))) {
				return false;
			}

			if ((uStyle & DT_CALCRECT) != 0) {
				ApplyDirectWriteTextMetricsToRectInternal(metrics, rc);
				return true;
			}

			D2DDrawScope drawScope(renderContext, rc);
			if (!drawScope) {
				return false;
			}

			ComPtr<ID2D1SolidColorBrush> brush;
			if (FAILED(GetCachedSolidColorBrush(drawScope.Get(), ResolveTextColorInternal(pManager, dwTextColor), brush.GetAddressOf()))) {
				return false;
			}

			D2D1_DRAW_TEXT_OPTIONS drawOptions = D2D1_DRAW_TEXT_OPTIONS_NONE;
			if ((uStyle & DT_NOCLIP) == 0) {
				drawOptions = static_cast<D2D1_DRAW_TEXT_OPTIONS>(drawOptions | D2D1_DRAW_TEXT_OPTIONS_CLIP);
			}

			drawScope.Get()->DrawTextLayout(
				D2D1::Point2F(static_cast<float>(rc.left), static_cast<float>(rc.top)),
				textLayout.Get(),
				brush.Get(),
				drawOptions);
			return true;
		}

		SIZE GetTextSizeInternal(CPaintRenderContext& renderContext, std::wstring_view text, int iFont, UINT uStyle)
		{
			SIZE size = { 0, 0 };
			CPaintManagerUI* pManager = renderContext.GetManager();
			if (!CanUseDirect2D() || pManager == nullptr) {
				return size;
			}

			RECT rcMeasure = { 0, 0, 4096, 4096 };
			std::wstring wideText;
			ComPtr<IDWriteTextFormat> textFormat;
			ComPtr<IDWriteTextLayout> textLayout;
			DWRITE_TEXT_METRICS metrics = {};
			if (FAILED(CreateTextLayout(pManager, text, iFont, uStyle | DT_CALCRECT, rcMeasure, wideText, textFormat, textLayout, &metrics))) {
				return size;
			}

			size = ToDirectWriteTextMetricsSizeInternal(metrics);
			return size;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	void InvalidateD2DBitmapCacheInternal(HBITMAP hBitmap)
	{
		if (hBitmap == NULL) {
			return;
		}

		D2DRenderState& state = GetD2DRenderState();
		state.bitmapCache.erase(D2DBitmapCacheKey{ hBitmap, false });
		state.bitmapCache.erase(D2DBitmapCacheKey{ hBitmap, true });
	}

	bool TryDrawBitmapRectWithDirect2DInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rcDest, const RECT& rcPaint, const RECT& rcSource, bool useAlpha, UINT uFade, UINT uRotate)
	{
		if (!CanUseDirect2DRenderContext(renderContext) || hBitmap == nullptr || !IsRectValid(rcDest) || !IsRectValid(rcSource)) {
			return false;
		}

		const RECT rcBounds = uRotate == 0 ? rcDest : GetRotatedBoundsInternal(rcDest, static_cast<float>(uRotate));
		if (IsRectValid(rcPaint) && !HasVisibleIntersection(rcPaint, rcBounds)) {
			return true;
		}

		const RECT rcBind = IsRectValid(rcPaint) ? rcPaint : rcBounds;
		D2DDrawScope drawScope(renderContext, rcBind);
		if (!drawScope) {
			return false;
		}

		ComPtr<ID2D1Bitmap> bitmap;
		if (FAILED(GetCachedD2DBitmapFromHBITMAP(drawScope.Get(), hBitmap, useAlpha, bitmap.GetAddressOf()))) {
			return false;
		}

		const float opacity = static_cast<float>(uFade) / 255.0f;
		if (uRotate == 0) {
			DrawBitmapSegmentWithDirect2DInternal(drawScope.Get(), bitmap.Get(), rcDest, rcSource, rcBind, opacity);
			return true;
		}

		if (IsRectValid(rcPaint)) {
			drawScope.Get()->PushAxisAlignedClip(ToD2DRectF(rcPaint), D2D1_ANTIALIAS_MODE_ALIASED);
		}

		const float destWidth = static_cast<float>(rcDest.right - rcDest.left);
		const float destHeight = static_cast<float>(rcDest.bottom - rcDest.top);
		const float sourceWidth = static_cast<float>(rcSource.right - rcSource.left);
		const float sourceHeight = static_cast<float>(rcSource.bottom - rcSource.top);
		const bool isIntegerScaled =
			std::fabs(destWidth - sourceWidth) < 0.001f &&
			std::fabs(destHeight - sourceHeight) < 0.001f;

		const D2D1_POINT_2F rotationCenter = D2D1::Point2F(
			(static_cast<float>(rcDest.left) + static_cast<float>(rcDest.right)) * 0.5f,
			(static_cast<float>(rcDest.top) + static_cast<float>(rcDest.bottom)) * 0.5f);
		const D2D1_MATRIX_3X2_F bindTransform = GetD2DRenderState().dcBindTransform;
		drawScope.Get()->SetTransform(
			D2D1::Matrix3x2F::Rotation(static_cast<float>(uRotate), rotationCenter) * bindTransform);

		const D2D1_RECT_F sourceRect = ToD2DRectF(rcSource);
		drawScope.Get()->DrawBitmap(
			bitmap.Get(),
			ToD2DRectF(rcDest),
			opacity,
			isIntegerScaled ? D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR : D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			&sourceRect);

		drawScope.Get()->SetTransform(bindTransform);
		if (IsRectValid(rcPaint)) {
			drawScope.Get()->PopAxisAlignedClip();
		}
		return true;
	}

	bool TryDrawImageWithDirect2DInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade, bool hole, bool xtiled, bool ytiled)
	{
		if (!CanUseDirect2DRenderContext(renderContext) || hBitmap == nullptr) {
			return false;
		}
		if (!IsRectValid(rc) || !HasVisibleIntersection(rcPaint, rc)) {
			return true;
		}

		D2DDrawScope drawScope(renderContext, rcPaint);
		if (!drawScope) {
			return false;
		}

		ComPtr<ID2D1Bitmap> bitmap;
		if (FAILED(GetCachedD2DBitmapFromHBITMAP(drawScope.Get(), hBitmap, bAlpha, bitmap.GetAddressOf()))) {
			return false;
		}

		const float opacity = static_cast<float>(uFade) / 255.0f;
		if (!hole) {
			RECT rcMiddleDest = { rc.left + rcCorners.left, rc.top + rcCorners.top, rc.right - rcCorners.right, rc.bottom - rcCorners.bottom };
			RECT rcMiddleSource = { rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top, rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom };
			if (IsRectValid(rcMiddleDest) && IsRectValid(rcMiddleSource)) {
				if (xtiled || ytiled) {
					DrawTiledBitmapSegmentWithDirect2DInternal(drawScope.Get(), bitmap.Get(), rcMiddleDest, rcMiddleSource, rcPaint, opacity, xtiled, ytiled);
				}
				else {
					DrawBitmapSegmentWithDirect2DInternal(drawScope.Get(), bitmap.Get(), rcMiddleDest, rcMiddleSource, rcPaint, opacity);
				}
			}
		}

		const RECT rcPiecesDest[8] = {
			{ rc.left, rc.top, rc.left + rcCorners.left, rc.top + rcCorners.top },
			{ rc.left + rcCorners.left, rc.top, rc.right - rcCorners.right, rc.top + rcCorners.top },
			{ rc.right - rcCorners.right, rc.top, rc.right, rc.top + rcCorners.top },
			{ rc.left, rc.top + rcCorners.top, rc.left + rcCorners.left, rc.bottom - rcCorners.bottom },
			{ rc.right - rcCorners.right, rc.top + rcCorners.top, rc.right, rc.bottom - rcCorners.bottom },
			{ rc.left, rc.bottom - rcCorners.bottom, rc.left + rcCorners.left, rc.bottom },
			{ rc.left + rcCorners.left, rc.bottom - rcCorners.bottom, rc.right - rcCorners.right, rc.bottom },
			{ rc.right - rcCorners.right, rc.bottom - rcCorners.bottom, rc.right, rc.bottom }
		};

		const RECT rcPiecesSource[8] = {
			{ rcBmpPart.left, rcBmpPart.top, rcBmpPart.left + rcCorners.left, rcBmpPart.top + rcCorners.top },
			{ rcBmpPart.left + rcCorners.left, rcBmpPart.top, rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top },
			{ rcBmpPart.right - rcCorners.right, rcBmpPart.top, rcBmpPart.right, rcBmpPart.top + rcCorners.top },
			{ rcBmpPart.left, rcBmpPart.top + rcCorners.top, rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom },
			{ rcBmpPart.right - rcCorners.right, rcBmpPart.top + rcCorners.top, rcBmpPart.right, rcBmpPart.bottom - rcCorners.bottom },
			{ rcBmpPart.left, rcBmpPart.bottom - rcCorners.bottom, rcBmpPart.left + rcCorners.left, rcBmpPart.bottom },
			{ rcBmpPart.left + rcCorners.left, rcBmpPart.bottom - rcCorners.bottom, rcBmpPart.right - rcCorners.right, rcBmpPart.bottom },
			{ rcBmpPart.right - rcCorners.right, rcBmpPart.bottom - rcCorners.bottom, rcBmpPart.right, rcBmpPart.bottom }
		};

		for (int i = 0; i < 8; ++i) {
			if (IsRectValid(rcPiecesDest[i]) && IsRectValid(rcPiecesSource[i])) {
				DrawBitmapSegmentWithDirect2DInternal(drawScope.Get(), bitmap.Get(), rcPiecesDest[i], rcPiecesSource[i], rcPaint, opacity);
			}
		}
		return true;
	}


	void CRenderEngine::SetPreferredRenderBackend(RenderBackendType backend)
	{
		D2DRenderState& state = GetD2DRenderState();
		state.preferredBackend = backend;
		if (backend == RenderBackendGDI) {
			FlushActiveDirect2DBatch();
			ResetD2DRenderTargetResources(state);
			ClearHtmlRuntimeCaches(state);
			ResetDirect2DBatchState(state);
		}
	}

	RenderBackendType CRenderEngine::GetPreferredRenderBackend()
	{
		return GetD2DRenderState().preferredBackend;
	}

	RenderBackendType CRenderEngine::GetActiveRenderBackend()
	{
		return CanUseDirect2D() ? RenderBackendDirect2D : RenderBackendGDI;
	}

	void CRenderEngine::SetPreferredDirect2DRenderMode(Direct2DRenderMode mode)
	{
		D2DRenderState& state = GetD2DRenderState();
		if (state.preferredRenderMode == mode) {
			return;
		}

		state.preferredRenderMode = mode;
		FlushActiveDirect2DBatch();
		ResetD2DRenderTargetResources(state);
		ClearHtmlRuntimeCaches(state);
		ResetDirect2DBatchState(state);
	}

	Direct2DRenderMode CRenderEngine::GetPreferredDirect2DRenderMode()
	{
		return GetD2DRenderState().preferredRenderMode;
	}

	Direct2DRenderMode CRenderEngine::GetActiveDirect2DRenderMode()
	{
		D2DRenderState& state = GetD2DRenderState();
		if (state.preferredBackend == RenderBackendGDI) {
			return Direct2DRenderModeAuto;
		}
		if (!state.dcRenderTarget && CanUseDirect2D()) {
			EnsureDCRenderTarget();
		}
		return state.activeRenderMode;
	}

	bool CRenderEngine::IsDirect2DAvailable()
	{
		return CanUseDirect2D();
	}

	void BeginDirect2DBatchInternal(CPaintRenderContext& renderContext)
	{
		if (!CanUseDirect2DRenderContext(renderContext)) {
			return;
		}
		HDC hNativeDC = renderContext.GetDC();
		D2DRenderState& state = GetD2DRenderState();
		if (state.batchDepth > 0) {
			if (state.batchNativeDC == hNativeDC) {
				++state.batchDepth;
			}
			return;
		}

		state.batchNativeDC = hNativeDC;
		state.batchRect = ResolveDCRenderTargetBindRect(hNativeDC, renderContext.GetPaintRect());
		state.batchDepth = 1;
		state.batchDrawing = false;
		ResumeDirect2DBatch(state);
	}

	void EndDirect2DBatchInternal(CPaintRenderContext& renderContext)
	{
		HDC hNativeDC = renderContext.GetDC();
		D2DRenderState& state = GetD2DRenderState();
		if (state.batchDepth == 0) {
			return;
		}
		if (hNativeDC != NULL && state.batchNativeDC != hNativeDC) {
			return;
		}

		if (state.batchDepth > 1) {
			--state.batchDepth;
			return;
		}

		// Keep the final depth active until FlushDirect2DBatchForNativeDC commits EndDraw.
		FlushDirect2DBatchForNativeDC(state.batchNativeDC);
		ResetDirect2DBatchState(state);
	}

	void FlushDirect2DBatchForContextInternal(CPaintRenderContext& renderContext)
	{
		FlushDirect2DBatchForNativeDC(renderContext.GetDC());
	}

	void CRenderEngine::ResetFrameMetrics()
	{
		D2DRenderState& state = GetD2DRenderState();
		ResetDirect2DFrameMetricsInternal(state.frameMetrics);
	}

	void CRenderEngine::ConsumeFrameMetrics(
		UINT& nDirect2DBatchFlushes,
		UINT& nDirect2DStandaloneDraws,
		UINT& nTextLayoutCacheHits,
		UINT& nTextLayoutCacheRefreshes,
		UINT& nTextMetricsCacheHits,
		UINT& nTextMetricsCacheRefreshes,
		UINT& nTextNormalizationCacheHits,
		UINT& nTextNormalizationCacheRefreshes,
		UINT& nHtmlParseCacheHits,
		UINT& nHtmlParseCacheRefreshes,
		UINT& nHtmlLayoutCacheHits,
		UINT& nHtmlLayoutCacheRefreshes,
		UINT& nHtmlMetricsCacheHits,
		UINT& nHtmlMetricsCacheRefreshes,
		UINT& nHtmlHitTestCacheHits,
		UINT& nHtmlHitTestCacheRefreshes,
		UINT& nHtmlDrawingEffectCacheHits,
		UINT& nHtmlDrawingEffectCacheRefreshes,
		UINT& nHtmlDirectWriteParseFailures,
		UINT& nHtmlDirectWriteLayoutFailures,
		UINT& nHtmlDirectWriteRenderFailures)
	{
		D2DRenderState& state = GetD2DRenderState();
		ConsumeDirect2DFrameMetricsInternal(
			state.frameMetrics,
			nDirect2DBatchFlushes,
			nDirect2DStandaloneDraws,
			nTextLayoutCacheHits,
			nTextLayoutCacheRefreshes,
			nTextMetricsCacheHits,
			nTextMetricsCacheRefreshes,
			nTextNormalizationCacheHits,
			nTextNormalizationCacheRefreshes,
			nHtmlParseCacheHits,
			nHtmlParseCacheRefreshes,
			nHtmlLayoutCacheHits,
			nHtmlLayoutCacheRefreshes,
			nHtmlMetricsCacheHits,
			nHtmlMetricsCacheRefreshes,
			nHtmlHitTestCacheHits,
			nHtmlHitTestCacheRefreshes,
			nHtmlDrawingEffectCacheHits,
			nHtmlDrawingEffectCacheRefreshes,
			nHtmlDirectWriteParseFailures,
			nHtmlDirectWriteLayoutFailures,
			nHtmlDirectWriteRenderFailures);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	void PushD2DRectClipInternal(const RECT& rc)
	{
		if (!IsRectValid(rc)) {
			return;
		}

		ActiveD2DClipStack().push_back(D2DClipState{ D2DClipType::AxisAlignedRect, rc, 0, 0 });
	}

	void PushD2DRoundClipInternal(const RECT& rc, int width, int height)
	{
		if (!IsRectValid(rc) || width <= 0 || height <= 0) {
			return;
		}

		ActiveD2DClipStack().push_back(D2DClipState{ D2DClipType::RoundRect, rc, width, height });
	}

	void PopD2DClipInternal()
	{
		std::vector<D2DClipState>& clipStack = ActiveD2DClipStack();
		if (!clipStack.empty()) {
			clipStack.pop_back();
		}
	}

	void DrawColorInternal(CPaintRenderContext& renderContext, const RECT& rc, DWORD color)
	{
		if (!CanUseDirect2DRenderContext(renderContext) || !IsRectValid(rc) || color <= 0x00FFFFFF) {
			return;
		}

		D2DDrawScope drawScope(renderContext, rc);
		if (!drawScope) {
			return;
		}

		ComPtr<ID2D1SolidColorBrush> brush;
		if (FAILED(GetCachedSolidColorBrush(drawScope.Get(), color, brush.GetAddressOf()))) {
			return;
		}

		drawScope.Get()->FillRectangle(ToD2DRectF(rc), brush.Get());
	}

	void DrawRoundColorInternal(CPaintRenderContext& renderContext, const RECT& rc, int width, int height, DWORD color)
	{
		if (!CanUseDirect2DRenderContext(renderContext) || !IsRectValid(rc) || color <= 0x00FFFFFF) {
			return;
		}

		D2DDrawScope drawScope(renderContext, rc);
		if (!drawScope) {
			return;
		}

		ComPtr<ID2D1SolidColorBrush> brush;
		if (FAILED(GetCachedSolidColorBrush(drawScope.Get(), color, brush.GetAddressOf()))) {
			return;
		}

		const D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			ToD2DRectF(rc),
			static_cast<float>((std::max)(1, width)) * 0.5f,
			static_cast<float>((std::max)(1, height)) * 0.5f);
		drawScope.Get()->FillRoundedRectangle(roundedRect, brush.Get());
	}

	void DrawGradientInternal(CPaintRenderContext& renderContext, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps)
	{
		(void)nSteps;
		if (!CanUseDirect2DRenderContext(renderContext) || !IsRectValid(rc)) {
			return;
		}

		D2DDrawScope drawScope(renderContext, rc);
		if (!drawScope) {
			return;
		}

		ComPtr<ID2D1GradientStopCollection> stopCollection;
		if (FAILED(GetCachedGradientStopCollection(drawScope.Get(), dwFirst, dwSecond, stopCollection.GetAddressOf()))) {
			return;
		}

		const D2D1_POINT_2F startPoint = D2D1::Point2F(static_cast<float>(rc.left), static_cast<float>(rc.top));
		const D2D1_POINT_2F endPoint = bVertical
			? D2D1::Point2F(static_cast<float>(rc.left), static_cast<float>(rc.bottom))
			: D2D1::Point2F(static_cast<float>(rc.right), static_cast<float>(rc.top));

		ComPtr<ID2D1LinearGradientBrush> brush;
		if (FAILED(drawScope.Get()->CreateLinearGradientBrush(
			D2D1::LinearGradientBrushProperties(startPoint, endPoint),
			stopCollection.Get(),
			brush.GetAddressOf()))) {
			return;
		}

		drawScope.Get()->FillRectangle(ToD2DRectF(rc), brush.Get());
	}

	void DrawLineInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle /*= PS_SOLID*/)
	{
		if (!CanUseDirect2DRenderContext(renderContext) || nSize <= 0) {
			return;
		}

		RECT rcBind = rc;
		if (!IsRectValid(rcBind)) {
			rcBind.right = rcBind.left + (rcBind.right == rcBind.left ? 1 : 0);
			rcBind.bottom = rcBind.top + (rcBind.bottom == rcBind.top ? 1 : 0);
		}

		D2DDrawScope drawScope(renderContext, rcBind);
		if (!drawScope) {
			return;
		}

		ComPtr<ID2D1SolidColorBrush> brush;
		if (FAILED(GetCachedSolidColorBrush(drawScope.Get(), NormalizeRenderableColor(dwPenColor), brush.GetAddressOf()))) {
			return;
		}

		ComPtr<ID2D1StrokeStyle> strokeStyle;
		if (ShouldUseD2DStrokeStyleInternal(nStyle)) {
			GetCachedStrokeStyle(nStyle, strokeStyle.GetAddressOf());
		}

		drawScope.Get()->DrawLine(
			D2D1::Point2F(static_cast<float>(rc.left), static_cast<float>(rc.top)),
			D2D1::Point2F(static_cast<float>(rc.right), static_cast<float>(rc.bottom)),
			brush.Get(),
			static_cast<float>(nSize),
			strokeStyle.Get());
	}

	void DrawRectInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle /*= PS_SOLID*/)
	{
		if (!CanUseDirect2DRenderContext(renderContext) || !IsRectValid(rc) || nSize <= 0) {
			return;
		}

		D2DDrawScope drawScope(renderContext, rc);
		if (!drawScope) {
			return;
		}

		ComPtr<ID2D1SolidColorBrush> brush;
		if (FAILED(GetCachedSolidColorBrush(drawScope.Get(), NormalizeRenderableColor(dwPenColor), brush.GetAddressOf()))) {
			return;
		}

		ComPtr<ID2D1StrokeStyle> strokeStyle;
		if (ShouldUseD2DStrokeStyleInternal(nStyle)) {
			GetCachedStrokeStyle(nStyle, strokeStyle.GetAddressOf());
		}

		drawScope.Get()->DrawRectangle(ToInsetD2DRectF(rc, static_cast<float>(nSize)), brush.Get(), static_cast<float>(nSize), strokeStyle.Get());
	}

	void DrawRoundRectInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle /*= PS_SOLID*/)
	{
		if (!CanUseDirect2DRenderContext(renderContext) || !IsRectValid(rc) || nSize <= 0) {
			return;
		}

		D2DDrawScope drawScope(renderContext, rc);
		if (!drawScope) {
			return;
		}

		ComPtr<ID2D1SolidColorBrush> brush;
		if (FAILED(GetCachedSolidColorBrush(drawScope.Get(), NormalizeRenderableColor(dwPenColor), brush.GetAddressOf()))) {
			return;
		}

		ComPtr<ID2D1StrokeStyle> strokeStyle;
		if (ShouldUseD2DStrokeStyleInternal(nStyle)) {
			GetCachedStrokeStyle(nStyle, strokeStyle.GetAddressOf());
		}

		const D2D1_ROUNDED_RECT roundedRect = D2D1::RoundedRect(
			ToInsetD2DRectF(rc, static_cast<float>(nSize)),
			static_cast<float>(width) * 0.5f,
			static_cast<float>(height) * 0.5f);
		drawScope.Get()->DrawRoundedRectangle(roundedRect, brush.Get(), static_cast<float>(nSize), strokeStyle.Get());
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	void CRenderEngine::DrawText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, int iFont, UINT uStyle, DWORD dwTextBKColor)
	{
		DrawColorInternal(renderContext, rc, dwTextBKColor);
		DrawTextInternal(renderContext, rc, text, dwTextColor, iFont, uStyle);
	}

	void CRenderEngine::DrawText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, int iFont, UINT uStyle)
	{
		DrawTextInternal(renderContext, rc, text, dwTextColor, iFont, uStyle);
	}

	void CRenderEngine::DrawText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, int iFont, UINT uStyle, bool bGDIPlusDrawText)
	{
		(void)bGDIPlusDrawText;
		DrawTextInternal(renderContext, rc, text, dwTextColor, iFont, uStyle);
	}

	void CRenderEngine::DrawHtmlText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, RECT* prcLinks, std::wstring* sLinks, int& nLinkRects, int iFont, UINT uStyle)
	{
		// text 的 FYUI 轻量富文本语法见 UIRender.h 中 DrawHtmlText 的接口注释。
		DrawHtmlTextInternal(renderContext, rc, text, dwTextColor, prcLinks, sLinks, nLinkRects, iFont, uStyle);
	}

	SIZE CRenderEngine::GetTextSize(CPaintRenderContext& renderContext, std::wstring_view text, int iFont, UINT uStyle)
	{
		return GetTextSizeInternal(renderContext, text, iFont, uStyle);
	}

} // namespace DuiLib
#pragma warning(pop)


