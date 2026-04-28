#include "pch.h"
#include "UIRender.h"
#include "UIRenderImageDispatchInternal.h"
#include "UIRenderImageLegacyInternal.h"

namespace FYUI
{
	namespace
	{
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

		bool bSave = false;
		std::wstring sImageName = (pDrawInfo)->sImageName;

		RECT rcDest = rcItem;
		if (pDrawInfo->rcDest.left != 0 || pDrawInfo->rcDest.top != 0 ||
			pDrawInfo->rcDest.right != 0 || pDrawInfo->rcDest.bottom != 0) {
			rcDest.left = rcItem.left + pDrawInfo->rcDest.left;
			rcDest.top = rcItem.top + pDrawInfo->rcDest.top;
			rcDest.right = rcItem.left + pDrawInfo->rcDest.right;
			if (rcDest.right > rcItem.right) rcDest.right = rcItem.right;
			rcDest.bottom = rcItem.top + pDrawInfo->rcDest.bottom;
			if (rcDest.bottom > rcItem.bottom) rcDest.bottom = rcItem.bottom;
		}

		sImageName = StringUtil::Right(sImageName, 3);
		StringUtil::MakeLower(sImageName);
		if (pDrawInfo->pImageInfo == NULL && sImageName == L"svg")
		{
			if (pDrawInfo->bLoaded) return false;
			pDrawInfo->bLoaded = true;
			if (pDrawInfo->sDrawString.empty()) return false;

			bool bUseRes = false;
			bool bUseHSL = false;
			DWORD dwMask = 0;
			std::wstring sImageResType;
			GetDrawInfo(&pDrawInfo, bUseRes, bUseHSL, dwMask, sImageResType, pManager);

			TImageInfo* data = NULL;
			if (bUseRes == false)
			{
				data = const_cast<TImageInfo*>(pManager->GetImageEx(pDrawInfo->sImageName.c_str(), NULL, dwMask, bUseHSL));
			}
			else
			{
				data = const_cast<TImageInfo*>(pManager->GetImageEx(pDrawInfo->sImageName.c_str(), sImageResType.c_str(), dwMask, bUseHSL));
			}
			if (!data) return false;
			pDrawInfo->fPresent = static_cast<float>((rcDest.right - rcDest.left) * 1.0 / data->nX);
			pDrawInfo->pImageInfo = data;
		}
#ifdef SVG
		if (pDrawInfo->bSvg)
		{
			if (pDrawInfo->pImageInfo->hBitmap == NULL)
			{
				bool bUseRes = false;
				bool bUseHSL = false;
				DWORD dwMask = 0;
				std::wstring sImageResType;
				GetDrawInfo(&pDrawInfo, bUseRes, bUseHSL, dwMask, sImageResType, pManager);

				int nWidth = rcDest.right - rcDest.left;
				int nHeight = rcDest.bottom - rcDest.top;
				if (pDrawInfo->pImageInfo->nX > 0 && pDrawInfo->pImageInfo->nY > 0)
				{
				}
				else
				{
					GetSvgWidthHeightFunc GetSvgWidthHeight = (GetSvgWidthHeightFunc)g_SDK->GetFunction("GetSvgWidthHeight");
					if (GetSvgWidthHeight != nullptr)
					{
						GetSvgWidthHeight(pDrawInfo->pImageInfo->pHandle, pDrawInfo->pImageInfo->nX, pDrawInfo->pImageInfo->nY);
					}
				}

				bool bAlphaChannel = false;
				HBITMAP hBitmap = nullptr;
				GetSvgHbitmapFunc GetSvgHbitmap = (GetSvgHbitmapFunc)g_SDK->GetFunction("GetSvgHbitmap");
				if (GetSvgHbitmap != nullptr)
					GetSvgHbitmap(pDrawInfo->pImageInfo->pHandle, nWidth, nHeight, bAlphaChannel, hBitmap);

				if (pDrawInfo->pImageInfo->hBitmap)
				{
					CRenderEngine::FreeBitmap(pDrawInfo->pImageInfo->hBitmap);
					pDrawInfo->pImageInfo->hBitmap = nullptr;
				}

				pDrawInfo->pImageInfo->hBitmap = hBitmap;

				pDrawInfo->fPresent = static_cast<float>((rcDest.right - rcDest.left) * 1.0 / nWidth);
				pDrawInfo->pImageInfo->bAlpha = bAlphaChannel;
			}
			else
			{
				RECT rcCorner = { 0,0,0,0 }, rcDest2 = { 0,0,0,0 };
				GetDrawInfo2(pDrawInfo->sDrawString, rcCorner, rcDest2, pManager);
				int nDestWidth = rcDest.right - rcDest.left;
				int nDestHeight = rcDest.bottom - rcDest.top;
				float fPresent = static_cast<float>((rcDest.right - rcDest.left) * 1.0 / pDrawInfo->pImageInfo->nX);

				if (rcCorner.left > 0 || rcCorner.top > 0 || rcCorner.right > 0 || rcCorner.bottom > 0)
				{
					int nScale = pManager->ScaleValue(100);
					float fScale = nScale * 1.0 / 100.0;
					if (pDrawInfo->pImageInfo->fPresent != fScale)
					{
						HBITMAP hBitmap = nullptr;
						bool bAlphaChannel = true;

						if (pDrawInfo->pImageInfo->pHandle == nullptr)
						{
							TImageInfo* data = LoadImage(pDrawInfo->sImageName.c_str(), nScale);
							if (pDrawInfo->pImageInfo->hBitmap)
							{
								FreeSVGBitmap FreeSVGBitmapFunc = (FreeSVGBitmap)g_SDK->GetFunction("FreeSVGBitmap");
								if (FreeSVGBitmapFunc != nullptr)
									FreeSVGBitmapFunc(pDrawInfo->pImageInfo->hBitmap);
								pDrawInfo->pImageInfo->hBitmap = nullptr;
							}
							pDrawInfo->pImageInfo = data;
						}
						int nSvgWidth = 0, nSvgHeight = 0;
						GetSvgWidthHeightFunc GetSvgWidthHeight = (GetSvgWidthHeightFunc)g_SDK->GetFunction("GetSvgWidthHeight");
						if (GetSvgWidthHeight != nullptr)
						{
							GetSvgWidthHeight(pDrawInfo->pImageInfo->pHandle, nSvgWidth, nSvgHeight);
						}

						int nWidth = round(fScale * nSvgWidth);
						int nHeight = round(fScale * nSvgHeight);

						pDrawInfo->pImageInfo->nX = nWidth;
						pDrawInfo->pImageInfo->nY = nHeight;

						GetSvgHbitmapFunc GetSvgHbitmap = (GetSvgHbitmapFunc)g_SDK->GetFunction("GetSvgHbitmap");
						if (GetSvgHbitmap != nullptr)
							GetSvgHbitmap(pDrawInfo->pImageInfo->pHandle, nWidth, nHeight, bAlphaChannel, hBitmap);

						pDrawInfo->pImageInfo->bAlpha = bAlphaChannel;
						if (pDrawInfo->pImageInfo->hBitmap)
						{
							FreeSVGBitmap FreeSVGBitmapFunc = (FreeSVGBitmap)g_SDK->GetFunction("FreeSVGBitmap");
							if (FreeSVGBitmapFunc != nullptr)
								FreeSVGBitmapFunc(pDrawInfo->pImageInfo->hBitmap);
						}
						pDrawInfo->pImageInfo->hBitmap = hBitmap;
						pDrawInfo->pImageInfo->fPresent = fScale;
					}
				}
				else
				{
					if ((rcCorner.left == pDrawInfo->rcCorner.left && rcCorner.right == pDrawInfo->rcCorner.right &&
						rcCorner.top == pDrawInfo->rcCorner.top && rcCorner.bottom == pDrawInfo->rcCorner.bottom)
						&& rcDest2.left == pDrawInfo->rcDest.left && rcDest2.right == pDrawInfo->rcDest.right &&
						rcDest2.top == pDrawInfo->rcDest.top && rcDest2.bottom == pDrawInfo->rcDest.bottom
						&& pDrawInfo->pImageInfo->nDestWidth == nDestWidth && pDrawInfo->pImageInfo->nDestHeight == nDestHeight)
					{
					}
					else
					{
						bool bUseRes = false;
						bool bUseHSL = false;
						DWORD dwMask = 0;
						std::wstring sImageResType;
						GetDrawInfo(&pDrawInfo, bUseRes, bUseHSL, dwMask, sImageResType, pManager);

						int nWidth = rcDest.right - rcDest.left;
						int nHeight = rcDest.bottom - rcDest.top;
						if (pDrawInfo->pImageInfo->nX > 0 && pDrawInfo->pImageInfo->nY > 0)
						{
							pDrawInfo->pImageInfo->nX = nWidth;
							pDrawInfo->pImageInfo->nY = nHeight;
						}
						else
						{
							GetSvgWidthHeightFunc GetSvgWidthHeight = (GetSvgWidthHeightFunc)g_SDK->GetFunction("GetSvgWidthHeight");
							if (GetSvgWidthHeight != nullptr)
							{
								GetSvgWidthHeight(pDrawInfo->pImageInfo->pHandle, pDrawInfo->pImageInfo->nX, pDrawInfo->pImageInfo->nY);
							}
						}

						bool bAlphaChannel = false;
						HBITMAP hBitmap = nullptr;

						if (pDrawInfo->pImageInfo->pHandle == nullptr)
						{
							int nScale = pManager->ScaleValue(100);
							TImageInfo* data = LoadImage(pDrawInfo->sImageName.c_str(), nScale);
							if (pDrawInfo->pImageInfo->hBitmap)
							{
								FreeSVGBitmap FreeSVGBitmapFunc = (FreeSVGBitmap)g_SDK->GetFunction("FreeSVGBitmap");
								if (FreeSVGBitmapFunc != nullptr)
									FreeSVGBitmapFunc(pDrawInfo->pImageInfo->hBitmap);
								pDrawInfo->pImageInfo->hBitmap = nullptr;
							}
							pDrawInfo->pImageInfo = data;
						}

						GetSvgHbitmapFunc GetSvgHbitmap = (GetSvgHbitmapFunc)g_SDK->GetFunction("GetSvgHbitmap");
						if (GetSvgHbitmap != nullptr)
							GetSvgHbitmap(pDrawInfo->pImageInfo->pHandle, nWidth, nHeight, bAlphaChannel, hBitmap);

						pDrawInfo->pImageInfo->nDestWidth = nWidth;
						pDrawInfo->pImageInfo->nDestHeight = nHeight;
						if (pDrawInfo->pImageInfo->hBitmap != nullptr)
						{
							FreeSVGBitmap FreeSVGBitmapFunc = (FreeSVGBitmap)g_SDK->GetFunction("FreeSVGBitmap");
							if (FreeSVGBitmapFunc != nullptr)
								FreeSVGBitmapFunc(pDrawInfo->pImageInfo->hBitmap);
						}
						pDrawInfo->pImageInfo->hBitmap = hBitmap;
						pDrawInfo->pImageInfo->bAlpha = bAlphaChannel;
					}
				}
			}
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


