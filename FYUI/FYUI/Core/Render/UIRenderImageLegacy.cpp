#include "pch.h"
#include "UIRenderImageDispatchInternal.h"
#include "UIRenderImageLegacyInternal.h"
#include "UIRenderImageRuntimeInternal.h"

namespace FYUI
{
	bool DrawResolvedGdiplusBitmapDirect2DPath(
		CPaintRenderContext& renderContext,
		const TImageInfo* data,
		const RECT& rcItem,
		const RECT& rcBmpPart,
		UINT uFade,
		UINT uRotate,
		bool bHasPaintRect)
	{
		CPaintManagerUI* pManager = renderContext.GetManager();
		const RECT& rcPaint = renderContext.GetPaintRect();
		if (!CanUseImageRenderContextInternal(renderContext) || data == nullptr) {
			return false;
		}

		const bool useAlpha = pManager->IsLayered() ? true : data->bAlpha;
		const RECT rcDirect2DPaint = bHasPaintRect ? rcPaint : RECT{ 0, 0, 0, 0 };
		if (TryDrawBitmapRectWithDirect2DInternal(renderContext, data->hBitmap, rcItem, rcDirect2DPaint, rcBmpPart, useAlpha, uFade, uRotate)) {
			return true;
		}
		return false;
	}
}
