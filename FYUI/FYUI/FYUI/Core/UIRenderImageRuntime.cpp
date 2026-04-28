#include "pch.h"
#include "UIRender.h"
#include "UIRenderImageLegacyInternal.h"
#include "UIRenderImageRuntimeInternal.h"

namespace FYUI
{
	namespace
	{
		void DeleteBitmapObject(HBITMAP hBitmap)
		{
			if (hBitmap != NULL) {
				::DeleteObject(hBitmap);
			}
		}

		void FreeImageResources(TImageInfo* pImageInfo, bool bDelete)
		{
			if (pImageInfo == NULL) {
				return;
			}

			if (pImageInfo->pImage) {
				delete pImageInfo->pImage;
			}
			pImageInfo->pImage = NULL;

			if (pImageInfo->hBitmap && pImageInfo->pHandle)
			{
#ifdef SVG
				FreeSVGBitmap FreeSVGBitmapFunc = (FreeSVGBitmap)g_SDK->GetFunction("FreeSVGBitmap");
				if (FreeSVGBitmapFunc != nullptr) {
					FreeSVGBitmapFunc(pImageInfo->hBitmap);
				}
#endif
			}
			else if (pImageInfo->hBitmap) {
				::DeleteObject(pImageInfo->hBitmap);
			}
			pImageInfo->hBitmap = NULL;

			if (pImageInfo->pBits) {
				delete[] pImageInfo->pBits;
			}
			pImageInfo->pBits = NULL;

			if (pImageInfo->pSrcBits) {
				delete[] pImageInfo->pSrcBits;
			}
			pImageInfo->pSrcBits = NULL;

			if (pImageInfo->pHandle)
			{
#ifdef SVG
				FreeSVGHandle FreeSVGHandleFunc = (FreeSVGHandle)g_SDK->GetFunction("FreeSVGHandle");
				FreeSVGHandleFunc(pImageInfo->pHandle);
#endif
			}
			pImageInfo->pHandle = NULL;

			if (bDelete) {
				delete pImageInfo;
			}
		}
	}

	void CRenderEngine::FreeImage(TImageInfo* pImageInfo, bool bDelete)
	{
		if (pImageInfo == NULL) {
			return;
		}

		InvalidateD2DBitmapCacheInternal(pImageInfo->hBitmap);
		FreeImageResources(pImageInfo, bDelete);
	}

	void CRenderEngine::FreeBitmap(HBITMAP hBitmap)
	{
		if (hBitmap == NULL) {
			return;
		}

		InvalidateD2DBitmapCacheInternal(hBitmap);
		DeleteBitmapObject(hBitmap);
	}

	void DrawImageInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade, bool hole, bool xtiled, bool ytiled)
	{
		if (hBitmap == NULL) {
			return;
		}
		TryDrawImageWithDirect2DInternal(renderContext, hBitmap, rc, rcPaint, rcBmpPart, rcCorners, bAlpha, uFade, hole, xtiled, ytiled);
	}

	void CRenderEngine::DrawImage(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade, bool hole, bool xtiled, bool ytiled)
	{
		DrawImageInternal(renderContext, hBitmap, rc, renderContext.GetPaintRect(), rcBmpPart, rcCorners, bAlpha, uFade, hole, xtiled, ytiled);
	}

	void DrawRotateImageInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, bool bAlpha, UINT uFade, UINT uRotate)
	{
		if (hBitmap == NULL) {
			return;
		}
		TryDrawBitmapRectWithDirect2DInternal(renderContext, hBitmap, rc, rcPaint, rcBmpPart, bAlpha, uFade, uRotate);
	}

	void CRenderEngine::DrawRotateImage(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcBmpPart, bool bAlpha, UINT uFade, UINT uRotate)
	{
		DrawRotateImageInternal(renderContext, hBitmap, rc, renderContext.GetPaintRect(), rcBmpPart, bAlpha, uFade, uRotate);
	}
}
