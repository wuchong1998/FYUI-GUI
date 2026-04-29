#pragma once

#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>

namespace FYUI
{
	namespace RenderImageD2DInternal
	{
		bool DrawBitmapSegmentWithDirect2DInternal(
			ID2D1RenderTarget* pRenderTarget,
			ID2D1Bitmap* pBitmap,
			const RECT& rcDest,
			const RECT& rcSource,
			const RECT& rcPaint,
			float opacity);

		void DrawTiledBitmapSegmentWithDirect2DInternal(
			ID2D1RenderTarget* pRenderTarget,
			ID2D1Bitmap* pBitmap,
			const RECT& rcDest,
			const RECT& rcSource,
			const RECT& rcPaint,
			float opacity,
			bool tileX,
			bool tileY);

		RECT GetRotatedBoundsInternal(const RECT& rc, float angleDegrees);

		HRESULT CreateD2DBitmapFromHBITMAPInternal(
			ID2D1RenderTarget* pRenderTarget,
			IWICImagingFactory* pWicFactory,
			HBITMAP hBitmap,
			bool useAlpha,
			ID2D1Bitmap** ppBitmap);
	}
}
