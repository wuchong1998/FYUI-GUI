#include "pch.h"
#include "UIHorizontalLayout.h"
#include "UILayoutLinearUtil.h"

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

		if( bHasVerticalScroll ) 
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( bHasHorizontalScroll ) 
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

			info.sz.cy = info.iControlMaxHeight;
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
			CRenderEngine::DrawColor(renderContext, rcSeparator, 0xAA000000);
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
		else if( StringUtil::CompareNoCase(pstrName, _T("sepimmbordercolor")) == 0 ) 
		{
			DWORD clrColor = 0;
			if (StringUtil::TryParseColor(pstrValue, clrColor)) {
				SetSepImmBorderColor(clrColor);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("sepimmleavebordercolor")) == 0 ) 
		{
			DWORD clrColor = 0;
			if (StringUtil::TryParseColor(pstrValue, clrColor)) {
				SetSepImmLeaveBorderColor(clrColor);
			}
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CHorizontalLayoutUI::DoEvent(TEventUI& event)
	{

		if( m_iSepWidth != 0 ) {
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
					m_rcItem = m_rcNewPos;
					if( !m_bImmMode && m_pManager ) m_pManager->RemovePostPaint(this);
					if (m_bImmMode)
					{
						m_pManager->SendNotify(this, DUI_MSGTYPE_SPLITMOVE_UP);
					}
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if (m_bImmMode)
				{
					RECT rcSeparator = GetThumbRect(false);
					if (::PtInRect(&rcSeparator, event.ptMouse))
					{
						::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
						if (GetSepImmBorderColor() != 0)
						{
							SetBorderColor(GetSepImmBorderColor());
						}
					}
				}
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG cx = event.ptMouse.x - ptLastMouse.x;
					ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					if( m_iSepWidth >= 0 ) {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.right - m_iSepWidth ) return;
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
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.left - m_iSepWidth ) return;
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
				if (m_bImmMode)
				{
					if (GetSepImmLeaveBorderColor() != 0)
					{
						SetBorderColor(GetSepImmLeaveBorderColor());
					}
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
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) {
			if( m_iSepWidth >= 0 ) return CDuiRect(m_rcNewPos.right - m_iSepWidth, m_rcNewPos.top, m_rcNewPos.right, m_rcNewPos.bottom);
			else return CDuiRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.left - m_iSepWidth, m_rcNewPos.bottom);
		}
		else {
			if( m_iSepWidth >= 0 ) 
				return CDuiRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
			else 
				return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
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
		__super::CopyData(pControl);
	}




}
