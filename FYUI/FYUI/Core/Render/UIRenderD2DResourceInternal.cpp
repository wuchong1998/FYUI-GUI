#include "pch.h"
#include "UIRenderD2DResourceInternal.h"
#include "UIRenderD2DSharedInternal.h"

#include <d2d1helper.h>

namespace FYUI
{
	namespace RenderD2DResourceInternal
	{
		HRESULT CreateSolidColorBrushInternal(ID2D1RenderTarget* pRenderTarget, DWORD color, ID2D1SolidColorBrush** ppBrush)
		{
			if (pRenderTarget == nullptr || ppBrush == nullptr) {
				return E_INVALIDARG;
			}
			return pRenderTarget->CreateSolidColorBrush(RenderD2DInternal::ToD2DColor(color), ppBrush);
		}

		HRESULT CreateStrokeStyleInternal(ID2D1Factory* pFactory, int nStyle, ID2D1StrokeStyle** ppStrokeStyle)
		{
			if (pFactory == nullptr || ppStrokeStyle == nullptr) {
				return E_INVALIDARG;
			}

			D2D1_STROKE_STYLE_PROPERTIES strokeProperties = D2D1::StrokeStyleProperties();
			strokeProperties.dashStyle = RenderD2DInternal::ToD2DDashStyle(nStyle);
			return pFactory->CreateStrokeStyle(strokeProperties, nullptr, 0, ppStrokeStyle);
		}

		HRESULT CreateGradientStopCollectionInternal(
			ID2D1RenderTarget* pRenderTarget,
			DWORD dwFirst,
			DWORD dwSecond,
			ID2D1GradientStopCollection** ppStopCollection)
		{
			if (pRenderTarget == nullptr || ppStopCollection == nullptr) {
				return E_INVALIDARG;
			}

			const D2D1_GRADIENT_STOP stops[2] =
			{
				{ 0.0f, RenderD2DInternal::ToD2DColor(dwFirst) },
				{ 1.0f, RenderD2DInternal::ToD2DColor(dwSecond) }
			};
			return pRenderTarget->CreateGradientStopCollection(stops, 2, ppStopCollection);
		}

		int BuildStrokeStyleCacheKeyInternal(int nStyle)
		{
			return nStyle & PS_STYLE_MASK;
		}

		bool ShouldUseD2DStrokeStyleInternal(int nStyle)
		{
			return BuildStrokeStyleCacheKeyInternal(nStyle) != PS_SOLID;
		}

		unsigned long long BuildGradientStopCollectionCacheKeyInternal(DWORD dwFirst, DWORD dwSecond)
		{
			return (static_cast<unsigned long long>(dwFirst) << 32) |
				static_cast<unsigned long long>(dwSecond);
		}

		size_t GetD2DBitmapResourceCacheLimitInternal()
		{
			return 256;
		}

		size_t GetD2DSolidColorBrushCacheLimitInternal()
		{
			return 128;
		}

		size_t GetD2DStrokeStyleCacheLimitInternal()
		{
			return 16;
		}

		size_t GetD2DGradientStopCollectionCacheLimitInternal()
		{
			return 64;
		}
	}
}
