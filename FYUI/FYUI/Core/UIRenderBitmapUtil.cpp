#include "pch.h"
#include "UIRender.h"
#include "UIRenderBatchInternal.h"
#include "UIRenderContext.h"
#include "UIRenderPrimitiveInternal.h"
#include "UIRenderSurface.h"

namespace FYUI
{
	namespace
	{
		static const float OneThird = 1.0f / 3;

		void RGBtoHSL(DWORD ARGB, float* H, float* S, float* L)
		{
			const float
				R = (float)GetRValue(ARGB),
				G = (float)GetGValue(ARGB),
				B = (float)GetBValue(ARGB),
				nR = (R < 0 ? 0 : (R > 255 ? 255 : R)) / 255,
				nG = (G < 0 ? 0 : (G > 255 ? 255 : G)) / 255,
				nB = (B < 0 ? 0 : (B > 255 ? 255 : B)) / 255,
				m = min(min(nR, nG), nB),
				M = max(max(nR, nG), nB);
			*L = (m + M) / 2;
			if (M == m) {
				*H = *S = 0;
			}
			else {
				const float
					f = (nR == m) ? (nG - nB) : ((nG == m) ? (nB - nR) : (nR - nG)),
					i = (nR == m) ? 3.0f : ((nG == m) ? 5.0f : 1.0f);
				*H = (i - f / (M - m));
				if (*H >= 6) {
					*H -= 6;
				}
				*H *= 60;
				*S = (2 * (*L) <= 1) ? ((M - m) / (M + m)) : ((M - m) / (2 - M - m));
			}
		}

		void HSLtoRGB(DWORD* ARGB, float H, float S, float L)
		{
			const float
				q = 2 * L < 1 ? L * (1 + S) : (L + S - L * S),
				p = 2 * L - q,
				h = H / 360,
				tr = h + OneThird,
				tg = h,
				tb = h - OneThird,
				ntr = tr < 0 ? tr + 1 : (tr > 1 ? tr - 1 : tr),
				ntg = tg < 0 ? tg + 1 : (tg > 1 ? tg - 1 : tg),
				ntb = tb < 0 ? tb + 1 : (tb > 1 ? tb - 1 : tb),
				B = 255 * (6 * ntr < 1 ? p + (q - p) * 6 * ntr : (2 * ntr < 1 ? q : (3 * ntr < 2 ? p + (q - p) * 6 * (2.0f * OneThird - ntr) : p))),
				G = 255 * (6 * ntg < 1 ? p + (q - p) * 6 * ntg : (2 * ntg < 1 ? q : (3 * ntg < 2 ? p + (q - p) * 6 * (2.0f * OneThird - ntg) : p))),
				R = 255 * (6 * ntb < 1 ? p + (q - p) * 6 * ntb : (2 * ntb < 1 ? q : (3 * ntb < 2 ? p + (q - p) * 6 * (2.0f * OneThird - ntb) : p)));
			*ARGB &= 0xFF000000;
			*ARGB |= RGB((BYTE)(R < 0 ? 0 : (R > 255 ? 255 : R)), (BYTE)(G < 0 ? 0 : (G > 255 ? 255 : G)), (BYTE)(B < 0 ? 0 : (B > 255 ? 255 : B)));
		}

		void NormalizeGeneratedBitmapAlpha(BYTE* pBits, int cx, int cy)
		{
			if (pBits == nullptr || cx <= 0 || cy <= 0) {
				return;
			}

			DWORD* pPixel = reinterpret_cast<DWORD*>(pBits);
			const size_t pixelCount = static_cast<size_t>(cx) * static_cast<size_t>(cy);
			for (size_t i = 0; i < pixelCount; ++i, ++pPixel) {
				if (((*pPixel & 0xFF000000) == 0) && ((*pPixel & 0x00FFFFFF) != 0)) {
					*pPixel |= 0xFF000000;
				}
			}
		}
	}

	HBITMAP CRenderEngine::GenerateBitmap(CPaintManagerUI* pManager, RECT rc, CControlUI* pStopControl, DWORD dwFilterColor)
	{
		if (pManager == NULL) return NULL;
		int cx = rc.right - rc.left;
		int cy = rc.bottom - rc.top;
		if (cx <= 0 || cy <= 0) return NULL;

		CPaintRenderSurface paintSurface;
		RECT rcSurface = { 0, 0, cx, cy };
		CPaintRenderContext measureContext = pManager->CreateMeasureRenderContext(rcSurface);
		if (!paintSurface.Ensure(measureContext, cx, cy)) {
			return NULL;
		}

		BYTE* pBits = paintSurface.GetBits();
		if (pBits != NULL) {
			::ZeroMemory(pBits, static_cast<size_t>(cx) * static_cast<size_t>(cy) * 4);
		}

		CPaintRenderContext renderContext = paintSurface.CreateRenderContext(
			pManager,
			rc,
			pManager->GetActiveRenderBackend(),
			pManager->GetActiveDirect2DRenderMode());
		HDC hPaintNativeDC = renderContext.GetDC();

		int iSaveDC = ::SaveDC(hPaintNativeDC);
		::SetWindowOrgEx(hPaintNativeDC, rc.left, rc.top, NULL);
		CScopedDirect2DBatchInternal batchScope(renderContext);
		CControlUI* pRoot = pManager->GetRoot();
		if (pRoot != NULL) {
			pRoot->Paint(renderContext, pStopControl);
		}
		::RestoreDC(hPaintNativeDC, iSaveDC);

		RECT rcClone = { 0, 0, cx, cy };
		if (dwFilterColor > 0x00FFFFFF) {
			DrawColorInternal(renderContext, rcClone, dwFilterColor);
		}
		NormalizeGeneratedBitmapAlpha(pBits, cx, cy);

		::GdiFlush();

		return paintSurface.DetachBitmap();
	}

	HBITMAP CRenderEngine::GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc, DWORD dwFilterColor)
	{
		if (pManager == NULL || pControl == NULL) return NULL;
		int cx = rc.right - rc.left;
		int cy = rc.bottom - rc.top;
		if (cx <= 0 || cy <= 0) return NULL;

		CPaintRenderSurface paintSurface;
		RECT rcSurface = { 0, 0, cx, cy };
		CPaintRenderContext measureContext = pManager->CreateMeasureRenderContext(rcSurface);
		if (!paintSurface.Ensure(measureContext, cx, cy)) {
			return NULL;
		}

		BYTE* pBits = paintSurface.GetBits();
		if (pBits != NULL) {
			::ZeroMemory(pBits, static_cast<size_t>(cx) * static_cast<size_t>(cy) * 4);
		}

		CPaintRenderContext renderContext = paintSurface.CreateRenderContext(
			pManager,
			rc,
			pManager->GetActiveRenderBackend(),
			pManager->GetActiveDirect2DRenderMode());
		HDC hPaintNativeDC = renderContext.GetDC();

		int iSaveDC = ::SaveDC(hPaintNativeDC);
		::SetWindowOrgEx(hPaintNativeDC, rc.left, rc.top, NULL);
		CScopedDirect2DBatchInternal batchScope(renderContext);
		pControl->Paint(renderContext, NULL);
		::RestoreDC(hPaintNativeDC, iSaveDC);

		RECT rcClone = { 0, 0, cx, cy };
		if (dwFilterColor > 0x00FFFFFF) {
			DrawColorInternal(renderContext, rcClone, dwFilterColor);
		}
		NormalizeGeneratedBitmapAlpha(pBits, cx, cy);

		::GdiFlush();

		return paintSurface.DetachBitmap();
	}

	void CRenderEngine::CheckAlphaColor(DWORD& dwColor)
	{
		if ((0x00FFFFFF & dwColor) == 0)
		{
			dwColor += 1;
		}
	}

	DWORD CRenderEngine::AdjustColor(DWORD dwColor, short H, short S, short L)
	{
		if (H == 180 && S == 100 && L == 100) return dwColor;
		float fH, fS, fL;
		float S1 = S / 100.0f;
		float L1 = L / 100.0f;
		RGBtoHSL(dwColor, &fH, &fS, &fL);
		fH += (H - 180);
		fH = fH > 0 ? fH : fH + 360;
		fS *= S1;
		fL *= L1;
		HSLtoRGB(&dwColor, fH, fS, fL);
		return dwColor;
	}

	HBITMAP CreateARGB32BitmapInternal(CPaintRenderContext& renderContext, int cx, int cy, BYTE** pBits)
	{
		HDC hNativeDC = renderContext.GetDC();
		if (hNativeDC == NULL || cx <= 0 || cy <= 0 || pBits == nullptr) {
			return NULL;
		}

		BITMAPINFO bitmapInfo = {};
		bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapInfo.bmiHeader.biWidth = cx;
		bitmapInfo.bmiHeader.biHeight = cy;
		bitmapInfo.bmiHeader.biPlanes = 1;
		bitmapInfo.bmiHeader.biBitCount = 32;
		bitmapInfo.bmiHeader.biCompression = BI_RGB;
		bitmapInfo.bmiHeader.biSizeImage = static_cast<DWORD>(cx) * static_cast<DWORD>(cy) * 4;

		return CreateDIBSection(hNativeDC, &bitmapInfo, DIB_RGB_COLORS, (void**)pBits, NULL, NULL);
	}

	void CRenderEngine::AdjustImage(bool bUseHSL, TImageInfo* imageInfo, short H, short S, short L)
	{
		if (imageInfo == NULL || imageInfo->bUseHSL == false || imageInfo->hBitmap == NULL ||
			imageInfo->pBits == NULL || imageInfo->pSrcBits == NULL)
			return;
		if (bUseHSL == false || (H == 180 && S == 100 && L == 100)) {
			::CopyMemory(imageInfo->pBits, imageInfo->pSrcBits, imageInfo->nX * imageInfo->nY * 4);
			return;
		}

		float fH, fS, fL;
		float S1 = S / 100.0f;
		float L1 = L / 100.0f;
		for (int i = 0; i < imageInfo->nX * imageInfo->nY; i++) {
			RGBtoHSL(*(DWORD*)(imageInfo->pSrcBits + i * 4), &fH, &fS, &fL);
			fH += (H - 180);
			fH = fH > 0 ? fH : fH + 360;
			fS *= S1;
			fL *= L1;
			HSLtoRGB((DWORD*)(imageInfo->pBits + i * 4), fH, fS, fL);
		}
	}

} // namespace DuiLib
