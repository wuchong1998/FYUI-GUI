#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	void InvalidateD2DBitmapCacheInternal(HBITMAP hBitmap);
	bool TryDrawBitmapRectWithDirect2DInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rcDest, const RECT& rcPaint, const RECT& rcSource, bool useAlpha, UINT uFade, UINT uRotate);
	bool TryDrawImageWithDirect2DInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade, bool hole, bool xtiled, bool ytiled);
}
