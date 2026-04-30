#pragma once

namespace FYUI
{
	/**
	 * @brief 应用布局Inset矩形
	 * @details 用于应用布局Inset矩形。具体行为由当前对象状态以及传入参数共同决定。
	 * @param rc [in] 矩形区域
	 * @param rcInset [in] Inset矩形区域
	 * @return 返回对应的几何结果
	 */
	inline RECT ApplyLayoutInsetRect(RECT rc, const RECT& rcInset)
	{
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;
		return rc;
	}

	/**
	 * @brief 调整布局矩形For可见状态滚动Bars
	 * @details 用于调整布局矩形For可见状态滚动Bars。具体行为由当前对象状态以及传入参数共同决定。
	 * @param rc [in] 矩形区域
	 * @param pVerticalScrollBar [in] 垂直滚动Bar对象
	 * @param pHorizontalScrollBar [in] 水平滚动Bar对象
	 * @return 返回对应的几何结果
	 */
	inline RECT AdjustLayoutRectForVisibleScrollBars(RECT rc, CScrollBarUI* pVerticalScrollBar, CScrollBarUI* pHorizontalScrollBar)
	{
		if( pVerticalScrollBar && pVerticalScrollBar->IsVisible() ) {
			rc.right -= pVerticalScrollBar->GetFixedWidth();
		}
		if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) {
			rc.bottom -= pHorizontalScrollBar->GetFixedHeight();
		}
		return rc;
	}
}
