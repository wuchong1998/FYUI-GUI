#include "pch.h"
#include "UIRender.h"
#include "UIRenderImageLegacyInternal.h"

namespace FYUI
{
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
		if (pDrawInfoCache != NULL && pDrawInfoCache->hCachedScaledBitmap != NULL) {
			pDrawInfoCache->ClearCachedBitmap();
		}

		DrawImageInternal(renderContext, data->hBitmap, rcItem, rcDrawPaint, rcBmpPart, rcCorner, useAlpha, uFade, bHole, bTiledX, bTiledY);
	}

} // namespace DuiLib
