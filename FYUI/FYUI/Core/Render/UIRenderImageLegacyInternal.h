#pragma once

#include "UIRenderContext.h"

#include <string_view>

namespace Gdiplus
{
	class Image;
}

namespace FYUI
{
	class CPaintRenderContext;
	struct tagTDrawInfo;
	typedef tagTDrawInfo TDrawInfo;

	void DrawImageInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false);
	void DrawRotateImageInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, bool bAlpha, UINT uFade = 255, UINT uRotate = 0);
	bool DrawImageInfoInternal(CPaintRenderContext& renderContext, const RECT& rcItem, TDrawInfo* pDrawInfo, HINSTANCE instance = NULL);
	bool DrawImageStringInternal(CPaintRenderContext& renderContext, const RECT& rcItem, std::wstring_view pStrImage, std::wstring_view pStrModify = {}, HINSTANCE instance = NULL);
}
