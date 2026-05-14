#include "pch.h"
#include "UIHorizontalLayout.h"
#include "UILayoutLinearUtil.h"
#include <cmath>

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CHorizontalLayoutUI)
		CHorizontalLayoutUI::CHorizontalLayoutUI() : m_iSepWidth(0), m_uButtonState(0), m_bImmMode(false)
	{
		ptLastMouse.x = ptLastMouse.y = 0;
		::ZeroMemory(&m_rcNewPos, sizeof(m_rcNewPos));
	}

	std::wstring_view CHorizontalLayoutUI::GetClass() const
	{
		return _T("HorizontalLayoutUI");
	}

	LPVOID CHorizontalLayoutUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_HORIZONTALLAYOUT) == 0) return static_cast<CHorizontalLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CHorizontalLayoutUI::GetControlFlags() const
	{
		if( (IsEnabled() && m_iSepWidth != 0) || m_bCursorMouse ==false) 
			return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CHorizontalLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{

		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		// Adjust for inset
		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;


		const bool bHasVerticalScroll = m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible();
		const bool bHasHorizontalScroll = m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible();

		if( !m_bScrollFloat && bHasVerticalScroll )
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( !m_bScrollFloat && bHasHorizontalScroll )
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		const int nItemCount = m_items.GetSize();
		if( nItemCount == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}


		const int iChildPadding = GetChildPadding();
		const UINT iChildAlign = GetChildAlign();
		const UINT iChildVAlign = GetChildVAlign();

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if( bHasVerticalScroll ) 
			szAvailable.cy += m_pVerticalScrollBar->GetScrollRange();


		std::vector<LinearLayoutInfo> layoutInfos;
		layoutInfos.reserve(nItemCount);  // ✅ 预分配避免动态扩容

		int cyNeeded = 0;
		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;


		for( int it1 = 0; it1 < nItemCount; it1++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);

			LinearLayoutInfo info;
			CollectLinearLayoutInfo(pControl, szAvailable, LinearLayoutAxis::Horizontal, info, cxFixed, cyNeeded, nAdjustables, nEstimateNum);
			layoutInfos.push_back(info);
		}

		cxFixed += (nEstimateNum - 1) * iChildPadding;

		// Place elements
		int cxNeeded = 0;
		int cxExpand = 0;
		if( nAdjustables > 0 ) 
			cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);

		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left;


		if( bHasHorizontalScroll ) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		else {
			// 子控件横向对齐方式
			if(nAdjustables <= 0) {
				if (iChildAlign == DT_CENTER) {
					iPosX += (szAvailable.cx - cxFixed) / 2;
				}
				else if (iChildAlign == DT_RIGHT) {
					iPosX += (szAvailable.cx - cxFixed);
				}
			}
		}


		int iScrollPosY = 0;
		int iScrollRangeY = 0;
		if( bHasVerticalScroll ) {
			iScrollPosY = m_pVerticalScrollBar->GetScrollPos();
			iScrollRangeY = m_pVerticalScrollBar->GetScrollRange();
		}

		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;


		for( int it2 = 0; it2 < nItemCount; it2++ ) 
		{
			LinearLayoutInfo& info = layoutInfos[it2];

			if( !info.bVisible ) continue;

			if( info.bFloat ) {
				SetFloatPos(it2);
				continue;
			}

			szRemaining.cx -= info.rcPadding.left;

			SIZE szControlAvailable = szRemaining;
			szControlAvailable.cy -= info.rcPadding.top + info.rcPadding.bottom;

			if (szControlAvailable.cx > info.iControlMaxWidth) 
				szControlAvailable.cx = info.iControlMaxWidth;
			if (szControlAvailable.cy > info.iControlMaxHeight) 
				szControlAvailable.cy = info.iControlMaxHeight;

			cxFixedRemaining -= (info.rcPadding.left + info.rcPadding.right);
			if (it2 > 0) cxFixedRemaining -= iChildPadding;


			if( info.sz.cx == 0 ) {
				iAdjustable++;
				info.sz.cx = cxExpand;

				if( iAdjustable == nAdjustables ) {
					info.sz.cx = MAX(0, szRemaining.cx - info.rcPadding.right - cxFixedRemaining);
				}

				const int minWidth = info.pControl->GetMinWidth();
				const int maxWidth = info.pControl->GetMaxWidth();
				if( info.sz.cx < minWidth ) info.sz.cx = minWidth;
				if( info.sz.cx > maxWidth ) info.sz.cx = maxWidth;
			}
			else {
				cxFixedRemaining -= info.sz.cx;
			}

			info.sz.cy = MAX(info.sz.cy, 0);
			if( info.sz.cy == 0 )
				info.sz.cy = szAvailable.cy - info.rcPadding.top - info.rcPadding.bottom;
			if( info.sz.cy < 0 ) info.sz.cy = 0;
			if( info.sz.cy > szControlAvailable.cy )
				info.sz.cy = szControlAvailable.cy;

			const int minHeight = info.pControl->GetMinHeight();
			if( info.sz.cy < minHeight ) info.sz.cy = minHeight;


			RECT rcCtrl;
			switch(iChildVAlign)
			{
			case DT_VCENTER: {
				int iPosY = (rc.bottom + rc.top) / 2;
				if( bHasVerticalScroll ) {
					iPosY += iScrollRangeY / 2;
					iPosY -= iScrollPosY;
				}
				rcCtrl = { 
					iPosX + info.rcPadding.left, 
					iPosY - info.sz.cy/2, 
					iPosX + info.sz.cx + info.rcPadding.left, 
					iPosY + info.sz.cy - info.sz.cy/2 
				};
				break;
			}
			case DT_BOTTOM: {
				int iPosY = rc.bottom;
				if( bHasVerticalScroll ) {
					iPosY += iScrollRangeY;
					iPosY -= iScrollPosY;
				}
				rcCtrl = { 
					iPosX + info.rcPadding.left, 
					iPosY - info.rcPadding.bottom - info.sz.cy, 
					iPosX + info.sz.cx + info.rcPadding.left, 
					iPosY - info.rcPadding.bottom 
				};
				break;
			}
			default: { // DT_TOP
				int iPosY = rc.top;
				if( bHasVerticalScroll ) {
					iPosY -= iScrollPosY;
				}
				rcCtrl = { 
					iPosX + info.rcPadding.left, 
					iPosY + info.rcPadding.top, 
					iPosX + info.sz.cx + info.rcPadding.left, 
					iPosY + info.sz.cy + info.rcPadding.top 
				};
				break;
			}
			}

			info.pControl->SetPos(rcCtrl, false);

			iPosX += info.sz.cx + iChildPadding + info.rcPadding.left + info.rcPadding.right;
			cxNeeded += info.sz.cx + info.rcPadding.left + info.rcPadding.right;
			szRemaining.cx -= info.sz.cx + iChildPadding + info.rcPadding.right;
		}

		cxNeeded += (nEstimateNum - 1) * iChildPadding;

		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}



	void CHorizontalLayoutUI::DoPostPaint(CPaintRenderContext& renderContext)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode ) {
			RECT rcSeparator = GetThumbRect(true);
			// 自定义虚影厚度：维持虚影跟随分隔条的方向对齐（正/负 sepwidth）
			if (m_iSepGhostSize > 0 && m_pManager != NULL) {
				const int nGhostPixels = m_pManager->ScaleValue(m_iSepGhostSize);
				if (m_iSepWidth >= 0) {
					rcSeparator.left = rcSeparator.right - nGhostPixels;
				}
				else {
					rcSeparator.right = rcSeparator.left + nGhostPixels;
				}
			}
			CRenderEngine::DrawColor(renderContext, rcSeparator, m_dwSepGhostColor);
		}
	}

	void CHorizontalLayoutUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	int CHorizontalLayoutUI::GetSepWidth() const
	{
		return m_iSepWidth;
	}

	void CHorizontalLayoutUI::SetSepImmMode(bool bImmediately)
	{
		if( m_bImmMode == bImmediately ) return;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode && m_pManager != NULL ) {
			m_pManager->RemovePostPaint(this);
		}

		m_bImmMode = bImmediately;
	}

	bool CHorizontalLayoutUI::IsSepImmMode() const
	{
		return m_bImmMode;
	}

	void CHorizontalLayoutUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("sepwidth")) == 0 ) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetSepWidth(value);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("sepimm")) == 0 ) SetSepImmMode(StringUtil::ParseBool(pstrValue));
		else if( StringUtil::CompareNoCase(pstrName, _T("sepghostcolor")) == 0 ) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValue, color)) SetSepGhostColor(color);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("sepghostsize")) == 0 ) {
			int iValue = 0;
			if (StringUtil::TryParseInt(pstrValue, iValue)) SetSepGhostSize(iValue);
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CHorizontalLayoutUI::SetSepGhostColor(DWORD dwColor)
	{
		if (m_dwSepGhostColor == dwColor) return;
		m_dwSepGhostColor = dwColor;
		Invalidate();
	}

	DWORD CHorizontalLayoutUI::GetSepGhostColor() const
	{
		return m_dwSepGhostColor;
	}

	void CHorizontalLayoutUI::SetSepGhostSize(int iSize)
	{
		if (m_iSepGhostSize == iSize) return;
		m_iSepGhostSize = iSize;
		Invalidate();
	}

	int CHorizontalLayoutUI::GetSepGhostSize() const
	{
		return m_iSepGhostSize;
	}

	bool CHorizontalLayoutUI::IsHot() const
	{
		// 鼠标悬停于分隔条 → Hot；拖拽期间由 IsPushed 接管，避免同时为真
		return m_bSepHover && (m_uButtonState & UISTATE_CAPTURED) == 0;
	}

	bool CHorizontalLayoutUI::IsPushed() const
	{
		return (m_uButtonState & UISTATE_CAPTURED) != 0;
	}

	void CHorizontalLayoutUI::DoEvent(TEventUI& event)
	{

		if( m_iSepWidth != 0 ) {
			// DPI 兼容：m_iSepWidth 存的是逻辑像素，命中/边界比较前先换算为设备像素（ScaleValue 保留负号）
			const int nSepPixels = m_pManager != NULL ? m_pManager->ScaleValue(m_iSepWidth) : m_iSepWidth;

			if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
					m_rcNewPos = m_rcItem;
					if( !m_bImmMode && m_pManager ) m_pManager->AddPostPaint(this);
					return;
				}
			}
			if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
					// 不再手动赋值 m_rcItem = m_rcNewPos；
					// MOUSEMOVE 中已通过 SetFixedWidthFromPixels 更新了 m_cxyFixed，
					// NeedParentUpdate 触发的下一帧父 SetPos 会用新尺寸重排 自身 + 兄弟控件。
					// 提前覆盖 m_rcItem 会让本控件"抢先"突破父布局区，引起兄弟（如 Label）视觉上未跟随的脏区缺失问题。
					if( !m_bImmMode && m_pManager ) m_pManager->RemovePostPaint(this);
					if (m_pManager) {
						m_pManager->SendNotify(this, DUI_MSGTYPE_SPLITMOVE_UP);
					}
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				// 即时模式下：进入分隔条时给光标反馈，并通过 m_bSepHover 触发基类 PaintBorder 选用 HotBorderColor
				if (m_bImmMode && (m_uButtonState & UISTATE_CAPTURED) == 0)
				{
					RECT rcSeparator = GetThumbRect(false);
					const bool bOnSep = (::PtInRect(&rcSeparator, event.ptMouse) != FALSE);
					if (bOnSep) {
						::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
					}
					if (bOnSep != m_bSepHover) {
						m_bSepHover = bOnSep;
						Invalidate();
					}
				}
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG cx = event.ptMouse.x - ptLastMouse.x;
					ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					if( nSepPixels >= 0 ) {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.right - nSepPixels ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.right ) return;
						rc.right += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.right = rc.left + GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.right = rc.left + GetMaxWidth();
						}
					}
					else {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.left ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.left - nSepPixels ) return;
						rc.left += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.left = rc.right - GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.left = rc.right - GetMaxWidth();
						}
					}

					CDuiRect rcInvalidate = GetThumbRect(true);
					m_rcNewPos = rc;
					SetFixedWidthFromPixels(m_rcNewPos.right - m_rcNewPos.left, false);
					//不可以直接设置m_cxyFixed.cx，因为会导致Control 获取 m_cxyFixedScaled不准确 需要SetFixedWidth
					//m_cxyFixed.cx = m_pManager->UnscaleValue(m_rcNewPos.right - m_rcNewPos.left);

					if( m_bImmMode ) {
						m_rcItem = m_rcNewPos;
						NeedParentUpdate();
						m_pManager->SendNotify(this, DUI_MSGTYPE_SPLITMOVE);
					}
					else {
						rcInvalidate.Join(GetThumbRect(true));
						rcInvalidate.Join(GetThumbRect(false));
						if( m_pManager ) m_pManager->Invalidate(rcInvalidate);
					}
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSELEAVE )
			{
				// 离开 layout：清除分隔条悬停态，让基类 PaintBorder 回退到普通 BorderColor
				if (m_bSepHover) {
					m_bSepHover = false;
					Invalidate();
				}
			}
			if( event.Type == UIEVENT_SETCURSOR )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
					return;
				}
			}
		}

		CContainerUI::DoEvent(event);
	}

	RECT CHorizontalLayoutUI::GetThumbRect(bool bUseNew) const
	{
		// DPI 兼容：m_iSepWidth 为逻辑像素，命中矩形需以设备像素构造
		const int nSepPixels = m_pManager != NULL ? m_pManager->ScaleValue(m_iSepWidth) : m_iSepWidth;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) {
			if( nSepPixels >= 0 ) return CDuiRect(m_rcNewPos.right - nSepPixels, m_rcNewPos.top, m_rcNewPos.right, m_rcNewPos.bottom);
			else return CDuiRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.left - nSepPixels, m_rcNewPos.bottom);
		}
		else {
			if( nSepPixels >= 0 ) 
				return CDuiRect(m_rcItem.right - nSepPixels, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
			else 
				return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - nSepPixels, m_rcItem.bottom);
		}
	}

	CHorizontalLayoutUI* CHorizontalLayoutUI::Clone()
	{
		CHorizontalLayoutUI* pClone = new CHorizontalLayoutUI();
		// 复制基本属性
		pClone->CopyData(this);

		return pClone;
	}

	void CHorizontalLayoutUI::CopyData(CHorizontalLayoutUI* pControl)
	{

		m_iSepWidth = pControl->m_iSepWidth;
		m_uButtonState = 0;
		ptLastMouse = pControl->ptLastMouse;
		m_rcNewPos = pControl->m_rcNewPos;
		m_bImmMode = pControl->m_bImmMode;
		m_dwSepGhostColor = pControl->m_dwSepGhostColor;
		m_iSepGhostSize = pControl->m_iSepGhostSize;
		__super::CopyData(pControl);
	}





}