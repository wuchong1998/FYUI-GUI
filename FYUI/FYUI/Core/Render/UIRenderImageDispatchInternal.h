#pragma once

#include "UIRenderContext.h"

#include <string>

namespace FYUI
{
	class CPaintManagerUI;
	struct tagTDrawInfo;
	typedef tagTDrawInfo TDrawInfo;
	struct tagTImageInfo;
	typedef tagTImageInfo TImageInfo;
}

namespace FYUI
{
	bool CanUseImageRenderContextInternal(const CPaintRenderContext& renderContext);
	bool DrawResolvedGdiplusBitmapDirect2DPath(CPaintRenderContext& renderContext, const TImageInfo* data, const RECT& rcItem, const RECT& rcBmpPart, UINT uFade, UINT uRotate, bool bHasPaintRect);
	bool MakeImageDest(const RECT& rcControl, const CDuiSize& szImage, const std::wstring& sAlign, const RECT& rcPadding, RECT& rcDest);
	void GetDrawInfo(TDrawInfo** pDrawInfo, bool bUseRes, bool bUseHSL, DWORD& dwMask, std::wstring& sImageResType, CPaintManagerUI* pManager);
	void GetDrawInfo2(std::wstring& sDrawString, RECT& rcCorner, RECT& rcDest, CPaintManagerUI* pManager);
	bool IsDrawImagePaintRectEmpty(const RECT& rcPaint);
	void NormalizeDrawImageBitmapPart(const TImageInfo* data, RECT& rcBmpPart);
	bool ShouldSkipDrawImageByClip(const RECT& rc, const RECT& rcPaint, const RECT& rcItem);
	void DrawResolvedBitmapImagePath(CPaintRenderContext& renderContext, const TImageInfo* data, const RECT& rcItem, const RECT& rcBmpPart, const RECT& rcCorner, UINT uFade, bool bHole, bool bTiledX, bool bTiledY, TDrawInfo* pDrawInfoCache, bool bHasPaintRect);
}
