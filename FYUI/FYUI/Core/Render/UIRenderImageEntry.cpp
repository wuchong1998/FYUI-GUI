#include "pch.h"
#include "../UIRender.h"
#include "UIRenderImageDispatchInternal.h"
#include "UIRenderImageLegacyInternal.h"
#include "UIRenderImageRuntimeInternal.h"

namespace FYUI
{
	namespace
	{
#ifdef SVG
	void FreeSvgBitmap(HBITMAP& hBitmap)
	{
		if (hBitmap == NULL || g_SDK == nullptr) {
			return;
		}

		FreeSVGBitmap FreeSVGBitmapFunc = (FreeSVGBitmap)g_SDK->GetFunction("FreeSVGBitmap");
		InvalidateD2DBitmapCacheInternal(hBitmap);
		if (FreeSVGBitmapFunc != nullptr) {
			FreeSVGBitmapFunc(hBitmap);
		}
		else {
			::DeleteObject(hBitmap);
		}
		hBitmap = NULL;
	}

	bool RefreshSvgBitmapForDest(TDrawInfo* pDrawInfo, const RECT& rcDest)
	{
		if (pDrawInfo == NULL || pDrawInfo->pImageInfo == NULL || pDrawInfo->pImageInfo->pHandle == nullptr || g_SDK == nullptr) {
			return false;
		}

		const int nWidth = rcDest.right - rcDest.left;
		const int nHeight = rcDest.bottom - rcDest.top;
		if (nWidth <= 0 || nHeight <= 0) {
			return false;
		}

		if (pDrawInfo->pImageInfo->hBitmap != NULL &&
			pDrawInfo->pImageInfo->nDestWidth == nWidth &&
			pDrawInfo->pImageInfo->nDestHeight == nHeight) {
			return true;
		}

		GetSvgHbitmapFunc GetSvgHbitmap = (GetSvgHbitmapFunc)g_SDK->GetFunction("GetSvgHbitmap");
		if (GetSvgHbitmap == nullptr) {
			return pDrawInfo->pImageInfo->hBitmap != NULL;
		}

		bool bAlphaChannel = false;
		HBITMAP hBitmap = NULL;
		GetSvgHbitmap(pDrawInfo->pImageInfo->pHandle, nWidth, nHeight, bAlphaChannel, hBitmap);
		if (hBitmap == NULL) {
			return pDrawInfo->pImageInfo->hBitmap != NULL;
		}

		FreeSvgBitmap(pDrawInfo->pImageInfo->hBitmap);
		pDrawInfo->pImageInfo->hBitmap = hBitmap;
		pDrawInfo->pImageInfo->nX = nWidth;
		pDrawInfo->pImageInfo->nY = nHeight;
		pDrawInfo->pImageInfo->nDestWidth = nWidth;
		pDrawInfo->pImageInfo->nDestHeight = nHeight;
		pDrawInfo->pImageInfo->bAlpha = bAlphaChannel;
		pDrawInfo->fPresent = 1.0f;
		return true;
	}
#endif

	bool DrawImageEntry(CPaintRenderContext& renderContext, const RECT& rc, const std::wstring& sImageName, const std::wstring& sImageResType, RECT rcItem, RECT rcBmpPart, RECT rcCorner, DWORD dwMask, UINT uFade, UINT uRotate, bool bGdiplus, bool bHole, bool bTiledX, bool bTiledY, TDrawInfo* pDrawInfoCache = NULL, HINSTANCE instance = NULL)
	{
		if (!CanUseImageRenderContextInternal(renderContext)) {
			return false;
		}

		CPaintManagerUI* pManager = renderContext.GetManager();
		const RECT& rcPaint = renderContext.GetPaintRect();

		if (sImageName.empty()) {
			return false;
		}
		const TImageInfo* data = NULL;
		data = pManager->GetImageEx(sImageName, sImageResType, dwMask, false, bGdiplus, instance);
		if (!data) return false;

		NormalizeDrawImageBitmapPart(data, rcBmpPart);
		const bool bHasPaintRect = !IsDrawImagePaintRectEmpty(rcPaint);
		if (bHasPaintRect && ShouldSkipDrawImageByClip(rc, rcPaint, rcItem)) {
			return true;
		}

		if (bGdiplus) {
			return DrawResolvedGdiplusBitmapDirect2DPath(
				renderContext,
				data,
				rcItem,
				rcBmpPart,
				uFade,
				uRotate,
				bHasPaintRect);
		}

		DrawResolvedBitmapImagePath(
			renderContext,
			data,
			rcItem,
			rcBmpPart,
			rcCorner,
			uFade,
			bHole,
			bTiledX,
			bTiledY,
			pDrawInfoCache,
			bHasPaintRect);
		return true;
	}
	}

	bool DrawImageInfoInternal(CPaintRenderContext& renderContext, const RECT& rcItem, TDrawInfo* pDrawInfo, HINSTANCE instance)
	{
		if (!CanUseImageRenderContextInternal(renderContext) || pDrawInfo == NULL) return false;

		CPaintManagerUI* pManager = renderContext.GetManager();
		const RECT& rcPaint = renderContext.GetPaintRect();

		std::wstring sImageName = (pDrawInfo)->sImageName;

		RECT rcDest = rcItem;
		if (pDrawInfo->szImage.cx > 0 && pDrawInfo->szImage.cy > 0) {
			MakeImageDest(rcItem, pDrawInfo->szImage, pDrawInfo->sAlign, pDrawInfo->rcPadding, rcDest);
		}
		if (pDrawInfo->rcDest.left != 0 || pDrawInfo->rcDest.top != 0 ||
			pDrawInfo->rcDest.right != 0 || pDrawInfo->rcDest.bottom != 0) {
			rcDest.left = rcItem.left + pDrawInfo->rcDest.left;
			rcDest.top = rcItem.top + pDrawInfo->rcDest.top;
			rcDest.right = rcItem.left + pDrawInfo->rcDest.right;
			if (rcDest.right > rcItem.right) rcDest.right = rcItem.right;
			rcDest.bottom = rcItem.top + pDrawInfo->rcDest.bottom;
			if (rcDest.bottom > rcItem.bottom) rcDest.bottom = rcItem.bottom;
		}

		const bool bIsSvg = pDrawInfo->bSvg;
		if (pDrawInfo->pImageInfo == NULL && bIsSvg)
		{
			if (pDrawInfo->bLoaded) return false;
			pDrawInfo->bLoaded = true;
			TImageInfo* data = const_cast<TImageInfo*>(pManager->GetImageEx(
				pDrawInfo->sImageName,
				pDrawInfo->sResType,
				pDrawInfo->dwMask,
				pDrawInfo->bHSL,
				pDrawInfo->bGdiplus,
				instance));
			if (!data) return false;
			if (data->nX > 0) {
				pDrawInfo->fPresent = static_cast<float>((rcDest.right - rcDest.left) * 1.0 / data->nX);
			}
			pDrawInfo->pImageInfo = data;
		}
#ifdef SVG
		if (bIsSvg && !RefreshSvgBitmapForDest(pDrawInfo, rcDest)) {
			return false;
		}
#endif

		bool bRet = DrawImageEntry(renderContext, rcItem, pDrawInfo->sImageName, pDrawInfo->sResType, rcDest, \
			pDrawInfo->rcSource, pDrawInfo->rcCorner, pDrawInfo->dwMask, pDrawInfo->uFade, pDrawInfo->uRotate, pDrawInfo->bGdiplus, pDrawInfo->bHole, pDrawInfo->bTiledX, pDrawInfo->bTiledY, pDrawInfo, instance);

		return bRet;
	}

	bool CRenderEngine::DrawImageInfo(CPaintRenderContext& renderContext, const RECT& rcItem, TDrawInfo* pDrawInfo, HINSTANCE instance)
	{
		return DrawImageInfoInternal(renderContext, rcItem, pDrawInfo, instance);
	}

	bool DrawImageStringInternal(CPaintRenderContext& renderContext, const RECT& rcItem, std::wstring_view pStrImage, std::wstring_view pStrModify, HINSTANCE instance)
	{
		if (!CanUseImageRenderContextInternal(renderContext)) return false;

		CPaintManagerUI* pManager = renderContext.GetManager();
		TDrawInfo* pDrawInfo = pManager->GetDrawInfo(pStrImage, pStrModify);
		return DrawImageInfoInternal(renderContext, rcItem, pDrawInfo, instance);
	}

	bool CRenderEngine::DrawImageString(CPaintRenderContext& renderContext, const RECT& rcItem, std::wstring_view pStrImage, std::wstring_view pStrModify, HINSTANCE instance)
	{
		return DrawImageStringInternal(renderContext, rcItem, pStrImage, pStrModify, instance);
	}

} // namespace DuiLib


