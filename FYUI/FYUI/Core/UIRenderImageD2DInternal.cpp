#include "pch.h"
#include "UIRenderImageD2DInternal.h"
#include "UIRenderD2DSharedInternal.h"

#include <algorithm>
#include <cmath>
#include <wrl/client.h>

namespace FYUI
{
	namespace RenderImageD2DInternal
	{
		using Microsoft::WRL::ComPtr;

		bool DrawBitmapSegmentWithDirect2DInternal(
			ID2D1RenderTarget* pRenderTarget,
			ID2D1Bitmap* pBitmap,
			const RECT& rcDest,
			const RECT& rcSource,
			const RECT& rcPaint,
			float opacity)
		{
			if (pRenderTarget == nullptr || pBitmap == nullptr || !RenderD2DInternal::IsRectValid(rcDest) || !RenderD2DInternal::IsRectValid(rcSource)) {
				return false;
			}

			RECT rcVisible = {};
			if (!::IntersectRect(&rcVisible, &rcDest, &rcPaint)) {
				return true;
			}

			const float destWidth = static_cast<float>(rcDest.right - rcDest.left);
			const float destHeight = static_cast<float>(rcDest.bottom - rcDest.top);
			if (destWidth <= 0.0f || destHeight <= 0.0f) {
				return false;
			}

			const float sourceWidth = static_cast<float>(rcSource.right - rcSource.left);
			const float sourceHeight = static_cast<float>(rcSource.bottom - rcSource.top);

			D2D1_RECT_F sourceRect = {};
			sourceRect.left = static_cast<float>(rcSource.left) + (static_cast<float>(rcVisible.left - rcDest.left) * sourceWidth / destWidth);
			sourceRect.top = static_cast<float>(rcSource.top) + (static_cast<float>(rcVisible.top - rcDest.top) * sourceHeight / destHeight);
			sourceRect.right = static_cast<float>(rcSource.left) + (static_cast<float>(rcVisible.right - rcDest.left) * sourceWidth / destWidth);
			sourceRect.bottom = static_cast<float>(rcSource.top) + (static_cast<float>(rcVisible.bottom - rcDest.top) * sourceHeight / destHeight);

			const bool isIntegerScaled =
				(rcVisible.right - rcVisible.left) == (rcSource.right - rcSource.left) &&
				(rcVisible.bottom - rcVisible.top) == (rcSource.bottom - rcSource.top);
			pRenderTarget->DrawBitmap(
				pBitmap,
				RenderD2DInternal::ToD2DRectF(rcVisible),
				opacity,
				isIntegerScaled ? D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR : D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				&sourceRect);
			return true;
		}

		void DrawTiledBitmapSegmentWithDirect2DInternal(
			ID2D1RenderTarget* pRenderTarget,
			ID2D1Bitmap* pBitmap,
			const RECT& rcDest,
			const RECT& rcSource,
			const RECT& rcPaint,
			float opacity,
			bool tileX,
			bool tileY)
		{
			if (pRenderTarget == nullptr || pBitmap == nullptr || !RenderD2DInternal::IsRectValid(rcDest) || !RenderD2DInternal::IsRectValid(rcSource)) {
				return;
			}

			const LONG tileWidth = rcSource.right - rcSource.left;
			const LONG tileHeight = rcSource.bottom - rcSource.top;
			if (tileWidth <= 0 || tileHeight <= 0) {
				return;
			}

			for (LONG top = rcDest.top; top < rcDest.bottom; top += tileY ? tileHeight : (rcDest.bottom - rcDest.top)) {
				const LONG drawHeight = tileY ? std::min<LONG>(tileHeight, rcDest.bottom - top) : (rcDest.bottom - rcDest.top);
				for (LONG left = rcDest.left; left < rcDest.right; left += tileX ? tileWidth : (rcDest.right - rcDest.left)) {
					const LONG drawWidth = tileX ? std::min<LONG>(tileWidth, rcDest.right - left) : (rcDest.right - rcDest.left);
					RECT rcPieceDest = { left, top, left + drawWidth, top + drawHeight };
					RECT rcPieceSource = {
						rcSource.left,
						rcSource.top,
						tileX ? (rcSource.left + drawWidth) : rcSource.right,
						tileY ? (rcSource.top + drawHeight) : rcSource.bottom
					};
					DrawBitmapSegmentWithDirect2DInternal(pRenderTarget, pBitmap, rcPieceDest, rcPieceSource, rcPaint, opacity);
					if (!tileX) {
						break;
					}
				}
				if (!tileY) {
					break;
				}
			}
		}

		RECT GetRotatedBoundsInternal(const RECT& rc, float angleDegrees)
		{
			if (!RenderD2DInternal::IsRectValid(rc)) {
				return rc;
			}

			const float normalizedAngle = std::fmod(angleDegrees, 360.0f);
			if (std::fabs(normalizedAngle) < 0.001f) {
				return rc;
			}

			const float radians = normalizedAngle * 3.14159265358979323846f / 180.0f;
			const float cosTheta = std::cos(radians);
			const float sinTheta = std::sin(radians);
			const float centerX = (static_cast<float>(rc.left) + static_cast<float>(rc.right)) * 0.5f;
			const float centerY = (static_cast<float>(rc.top) + static_cast<float>(rc.bottom)) * 0.5f;

			struct FloatPoint
			{
				float x;
				float y;
			};

			const FloatPoint corners[4] = {
				{ static_cast<float>(rc.left), static_cast<float>(rc.top) },
				{ static_cast<float>(rc.right), static_cast<float>(rc.top) },
				{ static_cast<float>(rc.left), static_cast<float>(rc.bottom) },
				{ static_cast<float>(rc.right), static_cast<float>(rc.bottom) }
			};

			float minX = 0.0f;
			float minY = 0.0f;
			float maxX = 0.0f;
			float maxY = 0.0f;
			for (int i = 0; i < 4; ++i) {
				const float offsetX = corners[i].x - centerX;
				const float offsetY = corners[i].y - centerY;
				const float rotatedX = centerX + (offsetX * cosTheta) - (offsetY * sinTheta);
				const float rotatedY = centerY + (offsetX * sinTheta) + (offsetY * cosTheta);
				if (i == 0) {
					minX = maxX = rotatedX;
					minY = maxY = rotatedY;
				}
				else {
					minX = (std::min)(minX, rotatedX);
					minY = (std::min)(minY, rotatedY);
					maxX = (std::max)(maxX, rotatedX);
					maxY = (std::max)(maxY, rotatedY);
				}
			}

			RECT rcBounds = {};
			rcBounds.left = static_cast<LONG>(std::floor(minX));
			rcBounds.top = static_cast<LONG>(std::floor(minY));
			rcBounds.right = static_cast<LONG>(std::ceil(maxX));
			rcBounds.bottom = static_cast<LONG>(std::ceil(maxY));
			return rcBounds;
		}

		HRESULT CreateD2DBitmapFromHBITMAPInternal(
			ID2D1RenderTarget* pRenderTarget,
			IWICImagingFactory* pWicFactory,
			HBITMAP hBitmap,
			bool useAlpha,
			ID2D1Bitmap** ppBitmap)
		{
			if (pRenderTarget == nullptr || pWicFactory == nullptr || ppBitmap == nullptr || hBitmap == nullptr) {
				return E_INVALIDARG;
			}

			ComPtr<IWICBitmap> wicBitmap;
			const WICBitmapAlphaChannelOption alphaMode = useAlpha ? WICBitmapUseAlpha : WICBitmapIgnoreAlpha;
			HRESULT hr = pWicFactory->CreateBitmapFromHBITMAP(hBitmap, nullptr, alphaMode, wicBitmap.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			ComPtr<IWICFormatConverter> converter;
			hr = pWicFactory->CreateFormatConverter(converter.GetAddressOf());
			if (FAILED(hr)) {
				return hr;
			}

			hr = converter->Initialize(
				wicBitmap.Get(),
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				nullptr,
				0.0f,
				WICBitmapPaletteTypeMedianCut);
			if (FAILED(hr)) {
				return hr;
			}

			return pRenderTarget->CreateBitmapFromWicBitmap(converter.Get(), nullptr, ppBitmap);
		}
	}
}
