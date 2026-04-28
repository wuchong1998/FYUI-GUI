#pragma once

namespace FYUI
{
	inline RECT ApplyLayoutInsetRect(RECT rc, const RECT& rcInset)
	{
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;
		return rc;
	}

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
