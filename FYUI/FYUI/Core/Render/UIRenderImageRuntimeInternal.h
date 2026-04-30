#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	/**
	 * @brief 执行 InvalidateD2DBitmapCacheInternal 操作
	 * @details 用于执行 InvalidateD2DBitmapCacheInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param hBitmap [in] h位图参数
	 */
	void InvalidateD2DBitmapCacheInternal(HBITMAP hBitmap);
	/**
	 * @brief 执行 TryDrawBitmapRectWithDirect2DInternal 操作
	 * @details 用于执行 TryDrawBitmapRectWithDirect2DInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param hBitmap [in] h位图参数
	 * @param rcDest [in] Dest矩形区域
	 * @param rcPaint [in] 重绘区域
	 * @param rcSource [in] 来源矩形区域
	 * @param useAlpha [in] useAlpha参数
	 * @param uFade [in] Fade标志
	 * @param uRotate [in] Rotate标志
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool TryDrawBitmapRectWithDirect2DInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rcDest, const RECT& rcPaint, const RECT& rcSource, bool useAlpha, UINT uFade, UINT uRotate);
	/**
	 * @brief 执行 TryDrawImageWithDirect2DInternal 操作
	 * @details 用于执行 TryDrawImageWithDirect2DInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param hBitmap [in] h位图参数
	 * @param rc [in] 矩形区域
	 * @param rcPaint [in] 重绘区域
	 * @param rcBmpPart [in] BmpPart矩形区域
	 * @param rcCorners [in] Corners矩形区域
	 * @param bAlpha [in] 是否Alpha
	 * @param uFade [in] Fade标志
	 * @param hole [in] hole参数
	 * @param xtiled [in] xtiled参数
	 * @param ytiled [in] ytiled参数
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool TryDrawImageWithDirect2DInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade, bool hole, bool xtiled, bool ytiled);
}
