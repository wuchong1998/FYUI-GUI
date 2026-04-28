#include "pch.h"
#include "UIRender.h"
#include "UIRenderImageLegacyInternal.h"

namespace FYUI
{
	void smt(HBITMAP src_hbmp, HBITMAP des_hbmp)
	{
		BITMAP Bitmap;
		GetObject(src_hbmp, sizeof(Bitmap), (LPSTR)&Bitmap);
		BITMAP Bitmap1;
		GetObject(des_hbmp, sizeof(Bitmap1), (LPSTR)&Bitmap1);
		WORD wBitCount = Bitmap.bmBitsPixel;

		LPBYTE pBuf = (LPBYTE)Bitmap.bmBits;
		LPBYTE pBuf1 = (LPBYTE)Bitmap1.bmBits;

		int sw = Bitmap.bmWidth - 1, sh = Bitmap.bmHeight - 1, dw = Bitmap1.bmWidth - 1, dh = Bitmap1.bmHeight - 1;
		int B, N, x, y;
		int nPixelSize = wBitCount / 8;
		BYTE* pLinePrev, * pLineNext;
		BYTE* pA, * pB, * pC, * pD;
		BYTE* pDest;
		for (int i = 0; i <= dh; ++i)
		{
			pDest = (BYTE*)(pBuf1 + Bitmap1.bmWidth * i * nPixelSize);
			y = i * sh / dh;
			N = dh - i * sh % dh;
			pLinePrev = (BYTE*)(pBuf + Bitmap.bmWidth * y * nPixelSize);
			y++;
			pLineNext = (N == dh) ? pLinePrev : (BYTE*)(pBuf + Bitmap.bmWidth * y * nPixelSize);
			for (int j = 0; j <= dw; ++j)
			{
				x = j * sw / dw * nPixelSize;
				B = dw - j * sw % dw;
				pA = pLinePrev + x;
				pB = pA + nPixelSize;
				pC = pLineNext + x;
				pD = pC + nPixelSize;
				if (B == dw)
				{
					pB = pA;
					pD = pC;
				}
				for (int k = 0; k < nPixelSize; ++k)
					*pDest++ = (BYTE)(int)(
						(B * N * (*pA++ - *pB - *pC + *pD) + dw * N * *pB++
							+ dh * B * *pC++ + (dw * dh - dh * B - dw * N) * *pD++
							+ dw * dh / 2) / (double)(dw * dh)
						);
			}
		}
	}

	void smt2(HBITMAP src_hbmp, HBITMAP des_hbmp)
	{
		BITMAP src = {};
		BITMAP dst = {};
		GetObject(src_hbmp, sizeof(src), (LPSTR)&src);
		GetObject(des_hbmp, sizeof(dst), (LPSTR)&dst);

		if (!src.bmBits || !dst.bmBits)
			return;

		const int srcW = src.bmWidth;
		const int srcH = src.bmHeight;
		const int dstW = dst.bmWidth;
		const int dstH = dst.bmHeight;

		const int srcBpp = src.bmBitsPixel / 8;
		const int dstBpp = dst.bmBitsPixel / 8;
		if (srcBpp < 3 || dstBpp < 3)
			return;

		const int srcStride = src.bmWidthBytes;
		const int dstStride = dst.bmWidthBytes;

		const float scaleX = (float)srcW / (float)dstW;
		const float scaleY = (float)srcH / (float)dstH;

		const int a = 3;
		const float pi = 3.14159265358979323846f;

		auto sinc = [&](float x) -> float
			{
				if (x == 0.0f)
					return 1.0f;
				x *= pi;
				return sinf(x) / x;
			};

		auto lanczos = [&](float x) -> float
			{
				x = fabsf(x);
				if (x >= (float)a)
					return 0.0f;
				return sinc(x) * sinc(x / (float)a);
			};

		for (int y = 0; y < dstH; ++y)
		{
			float srcY = (y + 0.5f) * scaleY - 0.5f;
			int yBase = (int)floorf(srcY);

			for (int x = 0; x < dstW; ++x)
			{
				float srcX = (x + 0.5f) * scaleX - 0.5f;
				int xBase = (int)floorf(srcX);

				float sumW = 0.0f;
				float sum[4] = { 0 };

				for (int iy = yBase - a + 1; iy <= yBase + a; ++iy)
				{
					int sy = iy;
					if (sy < 0) sy = 0;
					if (sy >= srcH) sy = srcH - 1;

					float wy = lanczos(srcY - (float)iy);
					if (wy == 0.0f)
						continue;

					const BYTE* pSrcRow = (const BYTE*)src.bmBits + sy * srcStride;

					for (int ix = xBase - a + 1; ix <= xBase + a; ++ix)
					{
						int sx = ix;
						if (sx < 0) sx = 0;
						if (sx >= srcW) sx = srcW - 1;

						float wx = lanczos(srcX - (float)ix);
						float w = wx * wy;
						if (w == 0.0f)
							continue;

						const BYTE* pSrc = pSrcRow + sx * srcBpp;
						sum[0] += pSrc[0] * w;
						sum[1] += pSrc[1] * w;
						sum[2] += pSrc[2] * w;
						if (srcBpp == 4)
							sum[3] += pSrc[3] * w;

						sumW += w;
					}
				}

				if (sumW <= 0.0f)
					sumW = 1.0f;

				BYTE* pDst = (BYTE*)dst.bmBits + y * dstStride + x * dstBpp;
				for (int c = 0; c < 3; ++c)
				{
					float v = sum[c] / sumW;
					if (v < 0.0f) v = 0.0f;
					if (v > 255.0f) v = 255.0f;
					pDst[c] = (BYTE)(v + 0.5f);
				}

				if (dstBpp == 4)
				{
					float v = (srcBpp == 4) ? (sum[3] / sumW) : 255.0f;
					if (v < 0.0f) v = 0.0f;
					if (v > 255.0f) v = 255.0f;
					pDst[3] = (BYTE)(v + 0.5f);
				}
			}
		}
	}

	HBITMAP CreateARGB32ImageScaleBitmapInternal(CPaintRenderContext& renderContext, int cx, int cy, BYTE** pBits, bool bPositive)
	{
		HDC hScaleNativeDC = renderContext.GetDC();
		if (hScaleNativeDC == NULL) {
			return NULL;
		}

		BITMAPINFO bmi = {};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = cx;
		bmi.bmiHeader.biHeight = bPositive ? -cy : cy;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;
		return ::CreateDIBSection(hScaleNativeDC, &bmi, DIB_RGB_COLORS, reinterpret_cast<void**>(pBits), NULL, 0);
	}

	bool CanUseImageRenderContextInternal(const CPaintRenderContext& renderContext)
	{
		return renderContext.GetManager() != nullptr && renderContext.GetDC() != NULL;
	}

	bool IsDrawImagePaintRectEmpty(const RECT& rcPaint)
	{
		return rcPaint.left == 0 && rcPaint.right == 0 && rcPaint.top == 0 && rcPaint.bottom == 0;
	}

	void NormalizeDrawImageBitmapPart(const TImageInfo* data, RECT& rcBmpPart)
	{
		if (data == NULL) {
			return;
		}

		if (rcBmpPart.left == 0 && rcBmpPart.right == 0 && rcBmpPart.top == 0 && rcBmpPart.bottom == 0) {
			rcBmpPart.right = data->nX;
			rcBmpPart.bottom = data->nY;
		}
		if (rcBmpPart.right > data->nX) rcBmpPart.right = data->nX;
		if (rcBmpPart.bottom > data->nY) rcBmpPart.bottom = data->nY;
	}

	bool ShouldSkipDrawImageByClip(const RECT& rc, const RECT& rcPaint, const RECT& rcItem)
	{
		RECT rcTemp = { 0 };
		if (!::IntersectRect(&rcTemp, &rcItem, &rc)) return true;
		if (!::IntersectRect(&rcTemp, &rcItem, &rcPaint)) return true;
		return false;
	}

	bool HasEmptyDrawImageCorners(const RECT& rcCorner)
	{
		return rcCorner.left == 0 && rcCorner.right == 0 && rcCorner.top == 0 && rcCorner.bottom == 0;
	}

	bool IsLegacyScaledBitmapCacheValid(const TDrawInfo* pDrawInfoCache, HBITMAP hBitmapSource, const RECT& rcBmpPart, int paint_width, int paint_height)
	{
		return pDrawInfoCache != NULL
			&& pDrawInfoCache->hCachedScaledBitmap != NULL
			&& pDrawInfoCache->hCachedScaledBitmapSource == hBitmapSource
			&& pDrawInfoCache->szCachedScaledBitmap.cx == paint_width
			&& pDrawInfoCache->szCachedScaledBitmap.cy == paint_height
			&& ::EqualRect(&pDrawInfoCache->rcCachedScaledSource, &rcBmpPart);
	}

	void PruneLegacyScaledBitmapCache(
		TDrawInfo* pDrawInfoCache,
		bool bUseLegacyScaledBitmap,
		bool bHasEmptyCorners,
		HBITMAP hBitmapSource,
		const RECT& rcBmpPart,
		int paint_width,
		int paint_height)
	{
		if (pDrawInfoCache == NULL || pDrawInfoCache->hCachedScaledBitmap == NULL) {
			return;
		}

		if (!bUseLegacyScaledBitmap
			|| !bHasEmptyCorners
			|| !IsLegacyScaledBitmapCacheValid(pDrawInfoCache, hBitmapSource, rcBmpPart, paint_width, paint_height)) {
			pDrawInfoCache->ClearCachedBitmap();
		}
	}

	HBITMAP ResolveLegacyScaledBitmap(
		CPaintRenderContext& renderContext,
		CPaintManagerUI* pManager,
		const TImageInfo* data,
		const RECT& rcBmpPart,
		int bitmap_width,
		int bitmap_height,
		int paint_width,
		int paint_height,
		TDrawInfo* pDrawInfoCache,
		bool& bReleaseScaledBitmap)
	{
		bReleaseScaledBitmap = false;
		if (IsLegacyScaledBitmapCacheValid(pDrawInfoCache, data->hBitmap, rcBmpPart, paint_width, paint_height)) {
			if (pManager != NULL) {
				pManager->NotifyImageScaleCacheHit();
			}
			return pDrawInfoCache->hCachedScaledBitmap;
		}

		BYTE* pScaledBits = NULL;
		HBITMAP hNewScaledBitmap = CreateARGB32ImageScaleBitmapInternal(renderContext, paint_width, paint_height, &pScaledBits, true);
		if (hNewScaledBitmap == NULL) {
			return NULL;
		}

		if (bitmap_width > 200 || bitmap_height > 200)
			smt(data->hBitmap, hNewScaledBitmap);
		else
			smt2(data->hBitmap, hNewScaledBitmap);

		if (pDrawInfoCache != NULL) {
			pDrawInfoCache->ClearCachedBitmap();
			pDrawInfoCache->hCachedScaledBitmap = hNewScaledBitmap;
			pDrawInfoCache->hCachedScaledBitmapSource = data->hBitmap;
			pDrawInfoCache->rcCachedScaledSource = rcBmpPart;
			pDrawInfoCache->szCachedScaledBitmap.cx = paint_width;
			pDrawInfoCache->szCachedScaledBitmap.cy = paint_height;
			if (pManager != NULL) {
				pManager->NotifyImageScaleCacheRefresh();
			}
			return hNewScaledBitmap;
		}

		bReleaseScaledBitmap = true;
		return hNewScaledBitmap;
	}

	void DrawResolvedBitmapImagePath(
		CPaintRenderContext& renderContext,
		const TImageInfo* data,
		const RECT& rcItem,
		const RECT& rcBmpPart,
		const RECT& rcCorner,
		UINT uFade,
		bool bHole,
		bool bTiledX,
		bool bTiledY,
		TDrawInfo* pDrawInfoCache,
		bool bHasPaintRect)
	{
		CPaintManagerUI* pManager = renderContext.GetManager();
		const RECT& rcPaint = renderContext.GetPaintRect();
		if (!CanUseImageRenderContextInternal(renderContext) || data == nullptr) {
			return;
		}

		const bool useAlpha = pManager->IsLayered() ? true : data->bAlpha;
		const RECT rcDrawPaint = bHasPaintRect ? rcPaint : rcItem;
		const int bitmap_width = rcBmpPart.right - rcBmpPart.left;
		const int bitmap_height = rcBmpPart.bottom - rcBmpPart.top;
		const int paint_width = rcItem.right - rcItem.left;
		const int paint_height = rcItem.bottom - rcItem.top;
		const bool bHasEmptyCorners = HasEmptyDrawImageCorners(rcCorner);
		const bool bUseLegacyScaledBitmap = (CRenderEngine::GetActiveRenderBackend() != RenderBackendDirect2D);

		PruneLegacyScaledBitmapCache(
			pDrawInfoCache,
			bUseLegacyScaledBitmap,
			bHasEmptyCorners,
			data->hBitmap,
			rcBmpPart,
			paint_width,
			paint_height);

		if (bUseLegacyScaledBitmap
			&& (paint_width != bitmap_width || paint_height != bitmap_height)
			&& bHasEmptyCorners) {
			bool bReleaseScaledBitmap = false;
			HBITMAP hScaledBitmap = ResolveLegacyScaledBitmap(
				renderContext,
				pManager,
				data,
				rcBmpPart,
				bitmap_width,
				bitmap_height,
				paint_width,
				paint_height,
				pDrawInfoCache,
				bReleaseScaledBitmap);

			if (hScaledBitmap != NULL) {
				const RECT rcScaledBitmapPart = { 0, 0, paint_width, paint_height };
				DrawImageInternal(renderContext, hScaledBitmap, rcItem, rcDrawPaint, rcScaledBitmapPart, rcCorner, useAlpha, uFade, bHole, bTiledX, bTiledY);
				if (bReleaseScaledBitmap) {
					CRenderEngine::FreeBitmap(hScaledBitmap);
				}
				return;
			}
		}

		DrawImageInternal(renderContext, data->hBitmap, rcItem, rcDrawPaint, rcBmpPart, rcCorner, useAlpha, uFade, bHole, bTiledX, bTiledY);
	}

} // namespace DuiLib
