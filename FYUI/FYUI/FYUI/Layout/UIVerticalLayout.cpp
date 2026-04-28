#include "pch.h"
#include "UIVerticalLayout.h"
#include "UILayoutLinearUtil.h"

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CVerticalLayoutUI)
		CVerticalLayoutUI::CVerticalLayoutUI() : m_iSepHeight(0), m_uButtonState(0), m_bImmMode(false)
	{
		ptLastMouse.x = ptLastMouse.y = 0;
		::ZeroMemory(&m_rcNewPos, sizeof(m_rcNewPos));
	}

	std::wstring_view CVerticalLayoutUI::GetClass() const
	{
		return _T("VerticalLayoutUI");
	}

	LPVOID CVerticalLayoutUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_VERTICALLAYOUT) == 0) return static_cast<CVerticalLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CVerticalLayoutUI::GetControlFlags() const
	{

		if( (IsEnabled() && m_iSepHeight != 0) || m_bCursorMouse ==false ) 
			return UIFLAG_SETCURSOR;
		else 
			return 0;
	}


	void CVerticalLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{

		CControlUI::SetPos(rc, bNeedInvalidate);
		/*std::wstring strName = GetName ();
		if ( strName == L"attchment_verlayout" )
		{
			int A =0;
		}*/

		CContainerUI* pParent = (CContainerUI*)GetParent();
		if ( pParent )
		{
			RECT rcParentPos = pParent->GetPos();
			if (rc.top > rcParentPos.bottom || rc.bottom < rcParentPos.top)
				return ;
		}

		rc = m_rcItem;

		// Adjust for inset
		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;

		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) 
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		const int nItemCount = m_items.GetSize();
		if( nItemCount == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}


		const int iChildPadding = GetChildPadding();
		const UINT iChildAlign = GetChildAlign();
		const UINT iChildVAlign = GetChildVAlign();
		const bool bHasVerticalScroll = m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible();
		const bool bHasHorizontalScroll = m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible();

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if( bHasHorizontalScroll ) 
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();
		if( bHasVerticalScroll ) 
			szAvailable.cy += m_pVerticalScrollBar->GetScrollRange();


		std::vector<LinearLayoutInfo> layoutInfos;
		layoutInfos.reserve(nItemCount);  // 棰勫垎閰嶉伩鍏嶅姩鎬佹墿瀹?

		int cxNeeded = 0;
		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimateNum = 0;


		for( int it1 = 0; it1 < nItemCount; it1++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);

			LinearLayoutInfo info;
			CollectLinearLayoutInfo(pControl, szAvailable, LinearLayoutAxis::Vertical, info, cyFixed, cxNeeded, nAdjustables, nEstimateNum);
			layoutInfos.emplace_back(info);
		}

		cyFixed += (nEstimateNum - 1) * iChildPadding;

		// Place elements
		int cyNeeded = 0;
		int cyExpand = 0;
		if( nAdjustables > 0 ) 
			cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);

		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosY = rc.top;

		if( bHasVerticalScroll )
		{
			iPosY -= m_pVerticalScrollBar->GetScrollPos();
		}
		else
		{
			// 瀛愭帶浠跺瀭鐩村榻愭柟寮?
			if(nAdjustables <= 0) {
				if (iChildVAlign == DT_VCENTER) {
					iPosY += (szAvailable.cy - cyFixed) / 2;
				}
				else if (iChildVAlign == DT_BOTTOM) {
					iPosY += (szAvailable.cy - cyFixed);
				}
			}
		}

		// 
		int iScrollPosX = 0;
		int iScrollRangeX = 0;
		if( bHasHorizontalScroll ) {
			iScrollPosX = m_pHorizontalScrollBar->GetScrollPos();
			iScrollRangeX = m_pHorizontalScrollBar->GetScrollRange();
		}

		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;


		for( int it2 = 0; it2 < nItemCount; it2++ ) 
		{
			LinearLayoutInfo& info = layoutInfos[it2];

			if( !info.bVisible ) continue;

			if( info.bFloat ) {
				SetFloatPos(it2);
				continue;
			}

			szRemaining.cy -= info.rcPadding.top;

			SIZE szControlAvailable = szRemaining;
			szControlAvailable.cx -= info.rcPadding.left + info.rcPadding.right;

			if (szControlAvailable.cx > info.iControlMaxWidth) 
				szControlAvailable.cx = info.iControlMaxWidth;
			if (szControlAvailable.cy > info.iControlMaxHeight) 
				szControlAvailable.cy = info.iControlMaxHeight;

			cyFixedRemaining -= (info.rcPadding.top + info.rcPadding.bottom);
			if (it2 > 0) cyFixedRemaining -= iChildPadding;

			// 
			if( info.sz.cy == 0 ) {
				iAdjustable++;
				info.sz.cy = cyExpand;

				if( iAdjustable == nAdjustables ) {
					info.sz.cy = MAX(0, szRemaining.cy - info.rcPadding.bottom - cyFixedRemaining);
				}

				const int minHeight = info.pControl->GetMinHeight();
				const int maxHeight = info.pControl->GetMaxHeight();
				if( info.sz.cy < minHeight ) info.sz.cy = minHeight;
				if( info.sz.cy > maxHeight ) info.sz.cy = maxHeight;
			}
			else {
				cyFixedRemaining -= info.sz.cy;
			}

			info.sz.cx = MAX(info.sz.cx, 0);
			if( info.sz.cx == 0 ) 
				info.sz.cx = szAvailable.cx - info.rcPadding.left - info.rcPadding.right;
			if( info.sz.cx > szControlAvailable.cx ) 
				info.sz.cx = szControlAvailable.cx;

			const int minWidth = info.pControl->GetMinWidth();
			if( info.sz.cx < minWidth ) info.sz.cx = minWidth;

			// 
			RECT rcCtrl;
			switch(iChildAlign)
			{
			case DT_CENTER: {
				int iPosX = (rc.right + rc.left) / 2;
				if( bHasHorizontalScroll ) {
					iPosX += iScrollRangeX / 2;
					iPosX -= iScrollPosX;
				}
				rcCtrl = { 
					iPosX - info.sz.cx/2, 
					iPosY + info.rcPadding.top, 
					iPosX + info.sz.cx - info.sz.cx/2, 
					iPosY + info.sz.cy + info.rcPadding.top 
				};
				break;
			}
			case DT_RIGHT: {
				int iPosX = rc.right;
				if( bHasHorizontalScroll ) {
					iPosX += iScrollRangeX;
					iPosX -= iScrollPosX;
				}
				rcCtrl = { 
					iPosX - info.rcPadding.right - info.sz.cx, 
					iPosY + info.rcPadding.top, 
					iPosX - info.rcPadding.right, 
					iPosY + info.sz.cy + info.rcPadding.top 
				};
				break;
			}
			default: { // DT_LEFT
				int iPosX = rc.left;
				if( bHasHorizontalScroll ) {
					iPosX -= iScrollPosX;
				}
				rcCtrl = { 
					iPosX + info.rcPadding.left, 
					iPosY + info.rcPadding.top, 
					iPosX + info.rcPadding.left + info.sz.cx, 
					iPosY + info.sz.cy + info.rcPadding.top 
				};
				break;
			}
			}

			info.pControl->SetPos(rcCtrl, false);

			iPosY += info.sz.cy + iChildPadding + info.rcPadding.top + info.rcPadding.bottom;
			cyNeeded += info.sz.cy + info.rcPadding.top + info.rcPadding.bottom;
			szRemaining.cy -= info.sz.cy + iChildPadding + info.rcPadding.bottom;
		}

		cyNeeded += (nEstimateNum - 1) * iChildPadding;

		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}



	void CVerticalLayoutUI::DoPostPaint(CPaintRenderContext& renderContext)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode ) {
			RECT rcSeparator = GetThumbRect(true);
			CRenderEngine::DrawColor(renderContext, rcSeparator, 0xAA000000);
		}
	}


	void CVerticalLayoutUI::SetSepHeight(int iHeight)
	{
		m_iSepHeight = iHeight;
	}

	int CVerticalLayoutUI::GetSepHeight() const
	{
		if(m_pManager != NULL) return m_pManager->ScaleValue(m_iSepHeight);
		return m_iSepHeight;
	}

	void CVerticalLayoutUI::SetSepImmMode(bool bImmediately)
	{
		if( m_bImmMode == bImmediately ) return;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode && m_pManager != NULL ) {
			m_pManager->RemovePostPaint(this);
		}

		m_bImmMode = bImmediately;
	}

	bool CVerticalLayoutUI::IsSepImmMode() const
	{
		return m_bImmMode;
	}

	void CVerticalLayoutUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		const std::wstring valueText(pstrValue);
		const wchar_t* value = valueText.c_str();
		if( StringUtil::CompareNoCase(pstrName, _T("sepheight")) == 0 ) SetSepHeight(_ttoi(value));
		else if( StringUtil::CompareNoCase(pstrName, _T("sepimm")) == 0 ) SetSepImmMode(StringUtil::CompareNoCase(pstrValue, _T("true")) == 0);
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CVerticalLayoutUI::DoEvent(TEventUI& event)
	{
		if( m_iSepHeight != 0 ) {
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
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG cy = event.ptMouse.y - ptLastMouse.y;
					ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					if( m_iSepHeight >= 0 ) {
						if( cy > 0 && event.ptMouse.y < m_rcNewPos.bottom + m_iSepHeight ) return;
						if( cy < 0 && event.ptMouse.y > m_rcNewPos.bottom ) return;
						rc.bottom += cy;
						if( rc.bottom - rc.top <= GetMinHeight() ) {
							if( m_rcNewPos.bottom - m_rcNewPos.top <= GetMinHeight() ) return;
							rc.bottom = rc.top + GetMinHeight();
						}
						if( rc.bottom - rc.top >= GetMaxHeight() ) {
							if( m_rcNewPos.bottom - m_rcNewPos.top >= GetMaxHeight() ) return;
							rc.bottom = rc.top + GetMaxHeight();
						}
					}
					else {
						if( cy > 0 && event.ptMouse.y < m_rcNewPos.top ) return;
						if( cy < 0 && event.ptMouse.y > m_rcNewPos.top + m_iSepHeight ) return;
						rc.top += cy;
						if( rc.bottom - rc.top <= GetMinHeight() ) {
							if( m_rcNewPos.bottom - m_rcNewPos.top <= GetMinHeight() ) return;
							rc.top = rc.bottom - GetMinHeight();
						}
						if( rc.bottom - rc.top >= GetMaxHeight() ) {
							if( m_rcNewPos.bottom - m_rcNewPos.top >= GetMaxHeight() ) return;
							rc.top = rc.bottom - GetMaxHeight();
						}
					}

					CDuiRect rcInvalidate = GetThumbRect(true);
					m_rcNewPos = rc;
					SetFixedHeightFromPixels(m_rcNewPos.bottom - m_rcNewPos.top);

					if( m_bImmMode ) {
						m_rcItem = m_rcNewPos;
						NeedParentUpdate();
					}
					else {
						rcInvalidate.Join(GetThumbRect(true));
						rcInvalidate.Join(GetThumbRect(false));
						if( m_pManager ) m_pManager->Invalidate(rcInvalidate);
					}
					return;
				}
			}
			if( event.Type == UIEVENT_SETCURSOR )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
					return;
				}
			}
		}

		if (event.Type == UIEVENT_SETCURSOR)
		{
			if(m_bCursorMouse ==false)
				return;
		}
		CContainerUI::DoEvent(event);
	}

	RECT CVerticalLayoutUI::GetThumbRect(bool bUseNew) const
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) {
			if( m_iSepHeight >= 0 ) 
				return CDuiRect(m_rcNewPos.left, MAX(m_rcNewPos.bottom - m_iSepHeight, m_rcNewPos.top), 
					m_rcNewPos.right, m_rcNewPos.bottom);
			else 
				return CDuiRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.right, 
					MIN(m_rcNewPos.top - m_iSepHeight, m_rcNewPos.bottom));
		}
		else {
			if( m_iSepHeight >= 0 ) 
				return CDuiRect(m_rcItem.left, MAX(m_rcItem.bottom - m_iSepHeight, m_rcItem.top), m_rcItem.right, 
					m_rcItem.bottom);
			else 
				return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.right, 
					MIN(m_rcItem.top - m_iSepHeight, m_rcItem.bottom));

		}
	}


	CVerticalLayoutUI* CVerticalLayoutUI::Clone()
	{
		CVerticalLayoutUI* pClone = new CVerticalLayoutUI();
		pClone->CopyData(this);

		return pClone;
	}
	void CVerticalLayoutUI::CopyData(CVerticalLayoutUI* pControl)
	{
		m_iSepHeight = pControl->m_iSepHeight;
		m_uButtonState = 0;
		ptLastMouse = pControl->ptLastMouse;
		m_rcNewPos = pControl->m_rcNewPos;
		m_bImmMode = pControl->m_bImmMode;
		__super::CopyData(pControl);

	}


}

