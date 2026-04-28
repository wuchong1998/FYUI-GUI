#include "pch.h"
#include "UIListEx.h"
#include "../Core/UIRenderContext.h"

namespace FYUI {

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListExUI)

		CListExUI::CListExUI() : m_pComboBoxUI(NULL), m_bAddMessageFilter(FALSE),m_nRow(-1),m_nColum(-1),m_pXCallback(NULL)
	{
	}

	std::wstring_view CListExUI::GetClass() const
	{
		return _T("XListUI");
	}

	UINT CListExUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

		LPVOID CListExUI::GetInterface(std::wstring_view pstrName)
		{
			if( StringUtil::CompareNoCase(pstrName, _T("ListEx")) == 0 ) return static_cast<IListOwnerUI*>(this);
			return CListUI::GetInterface(pstrName);
		}

		bool CListExUI::Remove(CControlUI* pControl)
		{
			if( pControl == NULL ) return false;
			const int nRow = GetItemIndex(pControl);
			if( nRow >= 0 ) {
				SyncInlineEditorAfterRemoval(nRow);
			}
			return CListUI::Remove(pControl);
		}

		bool CListExUI::RemoveAt(int iIndex)
		{
			SyncInlineEditorAfterRemoval(iIndex);
			return CListUI::RemoveAt(iIndex);
		}

		void CListExUI::RemoveAll()
		{
			HideEditAndComboCtrl();
			CListUI::RemoveAll();
		}

		CListContainerHeaderItemUI* CListExUI::GetHeaderItemAt(int nColum) const
		{
			return static_cast<CListContainerHeaderItemUI*>(m_pHeader->GetItemAt(nColum));
		}

	CListTextExtElementUI* CListExUI::GetTextExtItemAt(int nIndex) const
	{
		CControlUI* pControl = GetItemAt(nIndex);
		return pControl ? static_cast<CListTextExtElementUI*>(pControl->GetInterface(_T("ListTextExElement"))) : NULL;
	}

	void CListExUI::HandleHeaderCheckedNotify(const TNotifyUI& msg)
	{
		const BOOL bCheck = (BOOL)msg.lParam;
		CListHeaderUI* pHeader = GetHeader();
		for( int i = 0; i < pHeader->GetCount(); ++i ) {
			if( pHeader->GetItemAt(i) != msg.pSender ) continue;

			for( int j = 0; j < GetCount(); ++j ) {
				CListTextExtElementUI* pLItem = GetTextExtItemAt(j);
				if( pLItem != NULL ) {
					pLItem->SetCheck(bCheck);
				}
			}
			break;
		}
	}

	void CListExUI::HandleItemCheckedNotify(const TNotifyUI& msg)
	{
		for( int i = 0; i < GetCount(); ++i ) {
			CListTextExtElementUI* pLItem = GetTextExtItemAt(i);
			if( pLItem != NULL && pLItem == msg.pSender ) {
				OnListItemChecked(LOWORD(msg.wParam), HIWORD(msg.wParam), msg.lParam != 0 ? TRUE : FALSE);
				break;
			}
		}
	}

	bool CListExUI::HandleInlineComboNotify(const TNotifyUI& msg)
	{
		if( !m_pComboBoxUI || m_nRow < 0 || m_nColum < 0 ) return false;
		if (!StringUtil::EqualsNoCase(msg.sType, DUI_MSGTYPE_LISTITEMSELECT)) return false;
		if( static_cast<int>(msg.lParam) < 0 ) return false;

		CommitInlineComboSelection();
		return true;
	}

	bool CListExUI::HandleInlineScrollNotify(const TNotifyUI& msg)
	{
		if (!StringUtil::EqualsNoCase(msg.sType, L"scroll")) return false;
		if( m_pComboBoxUI == NULL || m_nRow < 0 || m_nColum < 0 ) return false;

		HideEditAndComboCtrl();
		return true;
	}

	BOOL CListExUI::CheckColumEditable(int nColum)
	{
		(void)nColum;
		return FALSE;
	}
	void CListExUI::InitListCtrl()
	{
		if (!m_bAddMessageFilter)
		{
			GetManager()->AddNotifier(this);
			m_bAddMessageFilter = TRUE;
		}
	}
	BOOL CListExUI::CheckColumComboBoxable(int nColum)
	{
		CListContainerHeaderItemUI* pHItem = GetHeaderItemAt(nColum);
		return pHItem != NULL? pHItem->GetColumeComboable() : FALSE;
	}

	CComboBoxUI* CListExUI::GetComboBoxUI()
	{
		if (m_pComboBoxUI == NULL)
		{
			m_pComboBoxUI = new CComboBoxUI;
			m_pComboBoxUI->SetName(_T("ListEx_Combo"));
			const std::wstring_view pDefaultAttributes = GetManager()->GetDefaultAttributeList(L"Combo");
			if( !pDefaultAttributes.empty() ) {
				m_pComboBoxUI->ApplyAttributeList(pDefaultAttributes);
			}

			Add(m_pComboBoxUI);
		}
		return m_pComboBoxUI;
	}

	BOOL CListExUI::CheckColumCheckBoxable(int nColum)
	{
		CListContainerHeaderItemUI* pHItem = GetHeaderItemAt(nColum);
		return pHItem != NULL? pHItem->GetColumeCheckable() : FALSE;
	}

	void CListExUI::Notify(TNotifyUI& msg)
	{	
		std::wstring strName = msg.pSender->GetName();

		if (StringUtil::EqualsNoCase(msg.sType, L"listheaditemchecked"))
		{
			HandleHeaderCheckedNotify(msg);
		}
		else if (StringUtil::EqualsNoCase(msg.sType, DUI_MSGTYPE_LISTITEMCHECKED))
		{
			HandleItemCheckedNotify(msg);
		}

		if (StringUtil::EqualsNoCase(strName, L"ListEx_Combo") && m_pComboBoxUI && m_nRow >= 0 && m_nColum >= 0)
		{
			HandleInlineComboNotify(msg);
		}
		else
		{
			HandleInlineScrollNotify(msg);
		}
	}

		void CListExUI::HideInlineComboControl()
		{
			if(!m_pComboBoxUI) return;

			RECT rc = {0,0,0,0};
			m_pComboBoxUI->SetPos(rc);
			m_pComboBoxUI->SetVisible(false);
		}

		void CListExUI::HideEditAndComboCtrl()
		{
			SetEditRowAndColum(-1, -1);
			HideInlineComboControl();
		}

		void CListExUI::CommitInlineComboSelection()
		{
			CListTextExtElementUI* pRowCtrl = GetTextExtItemAt(m_nRow);
		if (pRowCtrl && m_pComboBoxUI)
			{
				pRowCtrl->SetText(m_nColum, m_pComboBoxUI->GetText());
			}
			HideEditAndComboCtrl();
		}

	void CListExUI::ShowInlineCombo(int nIndex, int nColum, const RECT& rcColumn, std::wstring_view lpstrText)
	{
		if (!GetComboBoxUI()) return;

		m_pComboBoxUI->RemoveAll();
		SetEditRowAndColum(nIndex, nColum);
		m_pComboBoxUI->SetText(lpstrText);
		if (m_pXCallback)
		{
			m_pXCallback->GetItemComboTextArray(m_pComboBoxUI, nIndex, nColum);
		}
		m_pComboBoxUI->SetPos(rcColumn);
		m_pComboBoxUI->SetVisible(TRUE);
	}

	void CListExUI::ActivateInlineCellEditor(int nIndex, int nColum, const RECT& rcColumn, std::wstring_view lpstrText)
	{
		if( nColum < 0 ) {
			HideEditAndComboCtrl();
		}
		else if( CheckColumComboBoxable(nColum) ) {
			ShowInlineCombo(nIndex, nColum, rcColumn, lpstrText);
		}
		else {
			HideEditAndComboCtrl();
		}
	}

	void CListExUI::SyncHeaderCheckState(int nColum, BOOL bChecked)
	{
		CListContainerHeaderItemUI* pHItem = GetHeaderItemAt(nColum);
		if( pHItem == NULL ) {
			return;
		}
		if( !bChecked ) {
			pHItem->SetCheck(FALSE);
			return;
		}

		BOOL bCheckAll = TRUE;
		for( int i = 0; i < GetCount(); ++i ) {
			CListTextExtElementUI* pLItem = GetTextExtItemAt(i);
			if( pLItem != NULL && !pLItem->GetCheck() ) {
				bCheckAll = FALSE;
				break;
			}
		}
		pHItem->SetCheck(bCheckAll ? TRUE : FALSE);
	}

	IListComboCallbackUI* CListExUI::GetTextArrayCallback() const
	{
		return m_pXCallback;
	}

	void CListExUI::SetTextArrayCallback(IListComboCallbackUI* pCallback)
	{
		m_pXCallback = pCallback;
	}
	void CListExUI::OnListItemClicked(int nIndex, int nColum, RECT* lpRCColum, std::wstring_view lpstrText)
	{
		ActivateInlineCellEditor(nIndex, nColum, *lpRCColum, lpstrText);
	}
		void CListExUI::OnListItemChecked(int nIndex, int nColum, BOOL bChecked)
		{
			(void)nIndex;
			SyncHeaderCheckState(nColum, bChecked);
		}

		void CListExUI::DismissInlineEditorsForRow(int nRow)
		{
			if( m_nRow != nRow ) return;
			HideEditAndComboCtrl();
		}

		void CListExUI::SyncInlineEditorAfterRemoval(int nRow)
		{
			if( m_nRow < 0 || m_nColum < 0 ) return;
			if( nRow < 0 ) return;

			if( m_nRow == nRow ) {
				HideEditAndComboCtrl();
			}
			else if( m_nRow > nRow ) {
				m_nRow -= 1;
			}
		}

		void CListExUI::DoEvent(TEventUI& event)
		{
			if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_SCROLLWHEEL)
			{
				HideEditAndComboCtrl();
		}

		CListUI::DoEvent(event);
	}
	void CListExUI::SetColumItemColor(int nIndex, int nColum, DWORD iBKColor)
	{
		CListTextExtElementUI* pLItem = GetTextExtItemAt(nIndex);
		if( pLItem != NULL) 
		{
			DWORD iTextBkColor = iBKColor;
			pLItem->SetColumItemColor(nColum, iTextBkColor);
		}
	}

	BOOL CListExUI::GetColumItemColor(int nIndex, int nColum, DWORD& iBKColor)
	{
		CListTextExtElementUI* pLItem = GetTextExtItemAt(nIndex);
		if( pLItem == NULL) 
		{
			return FALSE;
		}
		pLItem->GetColumItemColor(nColum, iBKColor);
		return TRUE;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListContainerHeaderItemUI)

		CListContainerHeaderItemUI::CListContainerHeaderItemUI() : m_bDragable(TRUE), m_uButtonState(0), m_iSepWidth(4),
		m_uTextStyle(DT_VCENTER | DT_CENTER | DT_SINGLELINE), m_dwTextColor(0), m_iFont(-1), m_bShowHtml(FALSE),
		m_bEditable(FALSE),m_bComboable(FALSE),m_bCheckBoxable(FALSE),m_uCheckBoxState(0),m_bChecked(FALSE),m_pOwner(NULL)
	{
		SetTextPadding(CDuiRect(2, 0, 2, 0));
		ptLastMouse.x = ptLastMouse.y = 0;
		SetMinWidth(16);
	}

	std::wstring_view CListContainerHeaderItemUI::GetClass() const
	{
		return _T("ListContainerHeaderItemUI");
	}

	LPVOID CListContainerHeaderItemUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("ListContainerHeaderItem")) == 0 ) return this;
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CListContainerHeaderItemUI::GetControlFlags() const
	{
		if( IsEnabled() && m_iSepWidth != 0 ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CListContainerHeaderItemUI::SetEnabled(BOOL bEnable)
	{
		CContainerUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	BOOL CListContainerHeaderItemUI::IsDragable() const
	{
		return m_bDragable;
	}

	void CListContainerHeaderItemUI::SetDragable(BOOL bDragable)
	{
		m_bDragable = bDragable;
		if ( !m_bDragable ) m_uButtonState &= ~UISTATE_CAPTURED;
	}

	DWORD CListContainerHeaderItemUI::GetSepWidth() const
	{
		if(m_pManager != NULL) return m_pManager->ScaleValue(m_iSepWidth);
		return m_iSepWidth;
	}

	void CListContainerHeaderItemUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	DWORD CListContainerHeaderItemUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	void CListContainerHeaderItemUI::SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}

	DWORD CListContainerHeaderItemUI::GetTextColor() const
	{
		return m_dwTextColor;
	}


	void CListContainerHeaderItemUI::SetTextColor(DWORD dwTextColor)
	{
		m_dwTextColor = dwTextColor;
	}

	RECT CListContainerHeaderItemUI::GetTextPadding() const
	{
		RECT rcTextPadding = m_rcTextPadding;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcTextPadding);
		return rcTextPadding;
	}

	void CListContainerHeaderItemUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		Invalidate();
	}

	void CListContainerHeaderItemUI::SetFont(int index)
	{
		m_iFont = index;
	}

	BOOL CListContainerHeaderItemUI::IsShowHtml()
	{
		return m_bShowHtml;
	}

	void CListContainerHeaderItemUI::SetShowHtml(BOOL bShowHtml)
	{
		if( m_bShowHtml == bShowHtml ) return;

		m_bShowHtml = bShowHtml;
		Invalidate();
	}

	std::wstring_view CListContainerHeaderItemUI::GetNormalImage() const
	{
		return m_sNormalImage;
	}

	void CListContainerHeaderItemUI::SetNormalImage(std::wstring_view pStrImage)
	{
		m_sNormalImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CListContainerHeaderItemUI::GetHotImage() const
	{
		return m_sHotImage;
	}

	void CListContainerHeaderItemUI::SetHotImage(std::wstring_view pStrImage)
	{
		m_sHotImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CListContainerHeaderItemUI::GetPushedImage() const
	{
		return m_sPushedImage;
	}

	void CListContainerHeaderItemUI::SetPushedImage(std::wstring_view pStrImage)
	{
		m_sPushedImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CListContainerHeaderItemUI::GetFocusedImage() const
	{
		return m_sFocusedImage;
	}

	void CListContainerHeaderItemUI::SetFocusedImage(std::wstring_view pStrImage)
	{
		m_sFocusedImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CListContainerHeaderItemUI::GetSepImage() const
	{
		return m_sSepImage;
	}

	void CListContainerHeaderItemUI::SetSepImage(std::wstring_view pStrImage)
	{
		m_sSepImage = pStrImage;
		Invalidate();
	}

	    void CListContainerHeaderItemUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
    {
        const std::wstring_view name = StringUtil::TrimView(pstrNameView);
        if (name.empty()) {
            return;
        }

        std::wstring lowerValue(pstrValueView);
        StringUtil::MakeLower(lowerValue);
        auto contains = [&lowerValue](std::wstring_view token) {
            return lowerValue.find(token) != std::wstring::npos;
        };

        if (StringUtil::EqualsNoCase(name, L"dragable")) SetDragable(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"sepwidth")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetSepWidth(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"align")) {
            if (contains(L"left")) {
                m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
                m_uTextStyle |= DT_LEFT;
            }
            if (contains(L"center")) {
                m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
                m_uTextStyle |= DT_CENTER;
            }
            if (contains(L"right")) {
                m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
                m_uTextStyle |= DT_RIGHT;
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"endellipsis")) {
            if (StringUtil::ParseBool(pstrValueView)) m_uTextStyle |= DT_END_ELLIPSIS;
            else m_uTextStyle &= ~DT_END_ELLIPSIS;
        }
        else if (StringUtil::EqualsNoCase(name, L"font")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetFont(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"textcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"textpadding")) {
            RECT rect = { 0 };
            if (StringUtil::TryParseRect(pstrValueView, rect)) SetTextPadding(rect);
        }
        else if (StringUtil::EqualsNoCase(name, L"showhtml")) SetShowHtml(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"normalimage")) SetNormalImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"hotimage")) SetHotImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"pushedimage")) SetPushedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"focusedimage")) SetFocusedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"sepimage")) SetSepImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"editable")) SetColumeEditable(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"comboable")) SetColumeComboable(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"checkable")) SetColumeCheckable(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"checkboxwidth")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetCheckBoxWidth(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"checkboxheight")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetCheckBoxHeight(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"checkboxnormalimage")) SetCheckBoxNormalImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxhotimage")) SetCheckBoxHotImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxpushedimage")) SetCheckBoxPushedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxfocusedimage")) SetCheckBoxFocusedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxdisabledimage")) SetCheckBoxDisabledImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxselectedimage")) SetCheckBoxSelectedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxforeimage")) SetCheckBoxForeImage(pstrValueView);
        else CContainerUI::SetAttribute(pstrNameView, pstrValueView);
    }

	void CListContainerHeaderItemUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		//CheckBoxAble
		if (m_bCheckBoxable)
		{
			RECT rcCheckBox;
			GetCheckBoxRect(rcCheckBox);

			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
			{
				if( ::PtInRect(&rcCheckBox, event.ptMouse)) 
				{
					m_uCheckBoxState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uCheckBoxState & UISTATE_CAPTURED) != 0 ) 
				{
					if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
						m_uCheckBoxState |= UISTATE_PUSHED;
					else 
						m_uCheckBoxState &= ~UISTATE_PUSHED;
					Invalidate();
				}
				else if (::PtInRect(&rcCheckBox, event.ptMouse))
				{
					m_uCheckBoxState |= UISTATE_HOT;
					Invalidate();
				}
				else
				{
					m_uCheckBoxState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uCheckBoxState & UISTATE_CAPTURED) != 0 )
				{
					if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
					{
						SetCheck(!GetCheck());
						CContainerUI* pOwner = (CContainerUI*)m_pParent;
						if (pOwner)
						{
							m_pManager->SendNotify(this, DUI_MSGTYPE_LISTHEADITEMCHECKED, pOwner->GetItemIndex(this), m_bChecked);
						}

					}
					m_uCheckBoxState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
					Invalidate();
				}
				else if (::PtInRect(&rcCheckBox, event.ptMouse))
				{

				}
			}
			else if( event.Type == UIEVENT_MOUSEENTER )
			{
				if( ::PtInRect(&rcCheckBox, event.ptMouse) ) 
				{
					m_uCheckBoxState |= UISTATE_HOT;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSELEAVE )
			{
				m_uCheckBoxState &= ~UISTATE_HOT;
				Invalidate();
			}
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
		{
			if( !IsEnabled() ) return;
			RECT rcSeparator = GetThumbRect();
			if (m_iSepWidth>=0)
				rcSeparator.left-=4;
			else
				rcSeparator.right+=4;
			if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
				if( m_bDragable ) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
				}
			}
			else {
				m_uButtonState |= UISTATE_PUSHED;
				m_pManager->SendNotify(this, DUI_MSGTYPE_LISTHEADERCLICK);
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~UISTATE_CAPTURED;
				if( GetParent() ) 
					GetParent()->NeedParentUpdate();
			}
			else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
				m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				RECT rc = m_rcItem;
				if( m_iSepWidth >= 0 ) {
					rc.right -= ptLastMouse.x - event.ptMouse.x;
				}
				else {
					rc.left -= ptLastMouse.x - event.ptMouse.x;
				}

				if( rc.right - rc.left > GetMinWidth() ) {
					m_cxyFixed.cx = rc.right - rc.left;
					ptLastMouse = event.ptMouse;
					if( GetParent() ) 
						GetParent()->NeedParentUpdate();
				}
			}
			return;
		}
		if( event.Type == UIEVENT_SETCURSOR )
		{
			RECT rcSeparator = GetThumbRect();
			if (m_iSepWidth>=0)
				rcSeparator.left-=4;
			else
				rcSeparator.right+=4;
			if( IsEnabled() && m_bDragable && ::PtInRect(&rcSeparator, event.ptMouse) ) {
				::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
				return;
			}
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CContainerUI::DoEvent(event);
	}

	SIZE CListContainerHeaderItemUI::EstimateSize(SIZE szAvailable)
	{
		if( m_cxyFixed.cy == 0 ) return CDuiSize(m_cxyFixed.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + 14);
		return CContainerUI::EstimateSize(szAvailable);
	}

	RECT CListContainerHeaderItemUI::GetThumbRect() const
	{
		if( m_iSepWidth >= 0 ) return CDuiRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
		else return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
	}

	void CListContainerHeaderItemUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		//HeadItem Bkgnd
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;

		if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( m_sPushedImage.empty() && !m_sNormalImage.empty() ) DrawImage(renderContext, m_sNormalImage);
			if( !DrawImage(renderContext, m_sPushedImage) ) {}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( m_sHotImage.empty() && !m_sNormalImage.empty() ) DrawImage(renderContext, m_sNormalImage);
			if( !DrawImage(renderContext, m_sHotImage) ) {}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( m_sFocusedImage.empty() && !m_sNormalImage.empty() ) DrawImage(renderContext, m_sNormalImage);
			if( !DrawImage(renderContext, m_sFocusedImage) ) {}
		}
		else {
			if( !m_sNormalImage.empty() ) {
				if( !DrawImage(renderContext, m_sNormalImage) ) {}
			}
		}

		if( !m_sSepImage.empty() ) {
			RECT rcThumb = GetThumbRect();
			rcThumb.left -= m_rcItem.left;
			rcThumb.top -= m_rcItem.top;
			rcThumb.right -= m_rcItem.left;
			rcThumb.bottom -= m_rcItem.top;

			m_sSepImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
			if( !DrawImage(renderContext, m_sSepImage, m_sSepImageModify) ) {}
		}

		if(m_bCheckBoxable)
		{
			m_uCheckBoxState &= ~UISTATE_PUSHED;

			if( (m_uCheckBoxState & UISTATE_SELECTED) != 0 ) {
				if( !m_sCheckBoxSelectedImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxSelectedImage) ) {}
					else goto Label_ForeImage;
				}
			}

			if( IsFocused() ) m_uCheckBoxState |= UISTATE_FOCUSED;
			else m_uCheckBoxState &= ~ UISTATE_FOCUSED;
			if( !IsEnabled() ) m_uCheckBoxState |= UISTATE_DISABLED;
			else m_uCheckBoxState &= ~ UISTATE_DISABLED;

			if( (m_uCheckBoxState & UISTATE_DISABLED) != 0 ) {
				if( !m_sCheckBoxDisabledImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxDisabledImage) ) {}
					else return;
				}
			}
			else if( (m_uCheckBoxState & UISTATE_PUSHED) != 0 ) {
				if( !m_sCheckBoxPushedImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxPushedImage) ) {}
					else return;
				}
			}
			else if( (m_uCheckBoxState & UISTATE_HOT) != 0 ) {
				if( !m_sCheckBoxHotImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxHotImage) ) {}
					else return;
				}
			}
			else if( (m_uCheckBoxState & UISTATE_FOCUSED) != 0 ) {
				if( !m_sCheckBoxFocusedImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxFocusedImage) ) {}
					else return;
				}
			}

			if( !m_sCheckBoxNormalImage.empty() ) {
				if( !DrawCheckBoxImage(renderContext, m_sCheckBoxNormalImage) ) {}
				else return;
			}

		Label_ForeImage:
			if( !m_sCheckBoxForeImage.empty() ) {
				if( !DrawCheckBoxImage(renderContext, m_sCheckBoxForeImage) ) {}
			}
		}
	}

	void CListContainerHeaderItemUI::PaintText(CPaintRenderContext& renderContext)
	{
		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();

		RECT rcText = m_rcItem;
		rcText.left += m_rcTextPadding.left;
		rcText.top += m_rcTextPadding.top;
		rcText.right -= m_rcTextPadding.right;
		rcText.bottom -= m_rcTextPadding.bottom;
		if (m_bCheckBoxable) {
			RECT rcCheck;
			GetCheckBoxRect(rcCheck);
			rcText.left += (rcCheck.right - rcCheck.left);
		}

		std::wstring sText = GetText();
		if( sText.empty() ) return;

		int nLinks = 0;
		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(renderContext, rcText, sText, m_dwTextColor, \
				NULL, NULL, nLinks, m_iFont, DT_SINGLELINE | m_uTextStyle);
		else
		CRenderEngine::DrawText(renderContext, rcText, sText, m_dwTextColor, \
			m_iFont, DT_SINGLELINE | m_uTextStyle);
	}

	BOOL CListContainerHeaderItemUI::GetColumeEditable()
	{
		return m_bEditable;
	}

	void CListContainerHeaderItemUI::SetColumeEditable(BOOL bEnable)
	{
		m_bEditable = bEnable;
	}

	BOOL CListContainerHeaderItemUI::GetColumeComboable()
	{
		return m_bComboable;
	}

	void CListContainerHeaderItemUI::SetColumeComboable(BOOL bEnable)
	{
		m_bComboable = bEnable;
	}

	BOOL CListContainerHeaderItemUI::GetColumeCheckable()
	{
		return m_bCheckBoxable;
	}
	void CListContainerHeaderItemUI::SetColumeCheckable(BOOL bEnable)
	{
		m_bCheckBoxable = bEnable;
	}
	void CListContainerHeaderItemUI::SetCheck(BOOL bCheck)
	{
		if( m_bChecked == bCheck ) return;
		m_bChecked = bCheck;
		if( m_bChecked ) m_uCheckBoxState |= UISTATE_SELECTED;
		else m_uCheckBoxState &= ~UISTATE_SELECTED;
		Invalidate();
	}

	BOOL CListContainerHeaderItemUI::GetCheck()
	{
		return m_bChecked;
	}
	BOOL CListContainerHeaderItemUI::DrawCheckBoxImage(CPaintRenderContext& renderContext, const std::wstring& pStrImage, const std::wstring& pStrModify)
	{
		RECT rcCheckBox;
		GetCheckBoxRect(rcCheckBox);
		return CRenderEngine::DrawImageString(renderContext, rcCheckBox, pStrImage, pStrModify);
	}

	std::wstring_view CListContainerHeaderItemUI::GetCheckBoxNormalImage()
	{
		return m_sCheckBoxNormalImage;
	}

	void CListContainerHeaderItemUI::SetCheckBoxNormalImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxNormalImage = pStrImage;
	}

	std::wstring_view CListContainerHeaderItemUI::GetCheckBoxHotImage()
	{
		return m_sCheckBoxHotImage;
	}

	void CListContainerHeaderItemUI::SetCheckBoxHotImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxHotImage = pStrImage;
	}

	std::wstring_view CListContainerHeaderItemUI::GetCheckBoxPushedImage()
	{
		return m_sCheckBoxPushedImage;
	}

	void CListContainerHeaderItemUI::SetCheckBoxPushedImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxPushedImage = pStrImage;
	}

	std::wstring_view CListContainerHeaderItemUI::GetCheckBoxFocusedImage()
	{
		return m_sCheckBoxFocusedImage;
	}

	void CListContainerHeaderItemUI::SetCheckBoxFocusedImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxFocusedImage = pStrImage;
	}

	std::wstring_view CListContainerHeaderItemUI::GetCheckBoxDisabledImage()
	{
		return m_sCheckBoxDisabledImage;
	}

	void CListContainerHeaderItemUI::SetCheckBoxDisabledImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxDisabledImage = pStrImage;
	}
	std::wstring_view CListContainerHeaderItemUI::GetCheckBoxSelectedImage()
	{
		return m_sCheckBoxSelectedImage;
	}

	void CListContainerHeaderItemUI::SetCheckBoxSelectedImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxSelectedImage = pStrImage;
	}
	std::wstring_view CListContainerHeaderItemUI::GetCheckBoxForeImage()
	{
		return m_sCheckBoxForeImage;
	}

	void CListContainerHeaderItemUI::SetCheckBoxForeImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxForeImage = pStrImage;
	}
	int CListContainerHeaderItemUI::GetCheckBoxWidth() const
	{
		if(m_pManager) return m_pManager->ScaleValue(m_cxyCheckBox.cx);
		return m_cxyCheckBox.cx;
	}

	void CListContainerHeaderItemUI::SetCheckBoxWidth(int cx)
	{
		if( cx < 0 ) return; 
		m_cxyCheckBox.cx = cx;
	}

	int CListContainerHeaderItemUI::GetCheckBoxHeight()  const 
	{
		if(m_pManager) return m_pManager->ScaleValue(m_cxyCheckBox.cy);
		return m_cxyCheckBox.cy;
	}

	void CListContainerHeaderItemUI::SetCheckBoxHeight(int cy)
	{
		if( cy < 0 ) return; 
		m_cxyCheckBox.cy = cy;
	}
	void CListContainerHeaderItemUI::GetCheckBoxRect(RECT &rc)
	{
		memset(&rc, 0x00, sizeof(rc)); 
		int nItemHeight = m_rcItem.bottom - m_rcItem.top;
		rc.left = m_rcItem.left + 6;
		rc.top = m_rcItem.top + (nItemHeight - GetCheckBoxHeight()) / 2;
		rc.right = rc.left + GetCheckBoxWidth();
		rc.bottom = rc.top + GetCheckBoxHeight();
	}

	void CListContainerHeaderItemUI::SetOwner(CContainerUI* pOwner)
	{
		m_pOwner = pOwner;
	}
	CContainerUI* CListContainerHeaderItemUI::GetOwner()
	{
		return m_pOwner;
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListTextExtElementUI)

		CListTextExtElementUI::CListTextExtElementUI() : 
		m_nLinks(0), m_nHoverLink(-1), m_pOwner(NULL),m_uCheckBoxState(0),m_bChecked(FALSE)
	{
		::ZeroMemory(&m_rcLinks, sizeof(m_rcLinks));
		m_cxyCheckBox.cx = m_cxyCheckBox.cy = 0;

		::ZeroMemory(&ColumCorlorArray, sizeof(ColumCorlorArray));
	}

		CListTextExtElementUI::~CListTextExtElementUI()
		{
			if( m_pOwner != NULL ) {
				CListExUI* pListCtrl = static_cast<CListExUI*>(m_pOwner->GetInterface(_T("ListEx")));
				if( pListCtrl != NULL ) {
					pListCtrl->DismissInlineEditorsForRow(GetIndex());
				}
			}

			std::wstring* pText;
			for( int it = 0; it < m_aTexts.GetSize(); it++ ) {
				pText = static_cast<std::wstring*>(m_aTexts[it]);
				if( pText ) delete pText;
		}
		m_aTexts.Empty();
	}

	std::wstring_view CListTextExtElementUI::GetClass() const
	{
		return _T("ListTextExElementUI");
	}

	LPVOID CListTextExtElementUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("ListTextExElement")) == 0 ) return static_cast<CListTextExtElementUI*>(this);
		return CListLabelElementUI::GetInterface(pstrName);
	}

	UINT CListTextExtElementUI::GetControlFlags() const
	{
		return UIFLAG_WANTRETURN | ( (IsEnabled() && m_nLinks > 0) ? UIFLAG_SETCURSOR : 0);
	}

	std::wstring_view CListTextExtElementUI::GetText(int iIndex) const
	{
		std::wstring* pText = static_cast<std::wstring*>(m_aTexts.GetAt(iIndex));
		if( pText ) return *pText;
		return {};
	}

	void CListTextExtElementUI::SetText(int iIndex, std::wstring_view pstrText)
	{
		if( m_pOwner == NULL ) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if( iIndex < 0 || iIndex >= pInfo->nColumns ) return;
		while( m_aTexts.GetSize() < pInfo->nColumns ) { m_aTexts.Add(NULL); }

		std::wstring* pText = static_cast<std::wstring*>(m_aTexts[iIndex]);
		if( pText && *pText == pstrText ) return;
		if( pText == NULL && pstrText.empty() ) return;

		if ( pText )
			*pText = pstrText;
		else
			m_aTexts.SetAt(iIndex, new std::wstring(pstrText));
		Invalidate();
	}

		void CListTextExtElementUI::SetOwner(CControlUI* pOwner)
		{
			CListElementUI::SetOwner(pOwner);
			m_pOwner = static_cast<CListUI*>(pOwner->GetInterface(_T("List")));
		}

		void CListTextExtElementUI::SetVisible(bool bVisible)
		{
			if( !bVisible && m_pOwner != NULL ) {
				CListExUI* pListCtrl = static_cast<CListExUI*>(m_pOwner->GetInterface(_T("ListEx")));
				if( pListCtrl != NULL ) {
					pListCtrl->DismissInlineEditorsForRow(GetIndex());
				}
			}
			CListElementUI::SetVisible(bVisible);
		}

		void CListTextExtElementUI::SetEnabled(bool bEnable)
		{
			if( !bEnable && m_pOwner != NULL ) {
				CListExUI* pListCtrl = static_cast<CListExUI*>(m_pOwner->GetInterface(_T("ListEx")));
				if( pListCtrl != NULL ) {
					pListCtrl->DismissInlineEditorsForRow(GetIndex());
				}
			}
			CListElementUI::SetEnabled(bEnable);
		}

	std::wstring* CListTextExtElementUI::GetLinkContent(int iIndex)
	{
		if( iIndex >= 0 && iIndex < m_nLinks ) return &m_sLinks[iIndex];
		return NULL;
	}

	bool CListTextExtElementUI::HandleLinkCursorEvent(const TEventUI& event)
	{
		if( event.Type != UIEVENT_SETCURSOR ) return false;
		for( int i = 0; i < m_nLinks; ++i ) {
			if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return true;
			}
		}
		return false;
	}

	bool CListTextExtElementUI::HandleLinkClickEvent(const TEventUI& event)
	{
		if( event.Type != UIEVENT_BUTTONUP || !IsEnabled() ) return false;
		for( int i = 0; i < m_nLinks; ++i ) {
			if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_LINK, i);
				return true;
			}
		}
		return false;
	}

	void CListTextExtElementUI::UpdateLinkHoverState(const TEventUI& event)
	{
		if( m_nLinks <= 0 ) return;

		if( event.Type == UIEVENT_MOUSEMOVE ) {
			int nHoverLink = -1;
			for( int i = 0; i < m_nLinks; ++i ) {
				if( ::PtInRect(&m_rcLinks[i], event.ptMouse) ) {
					nHoverLink = i;
					break;
				}
			}
			if( m_nHoverLink != nHoverLink ) {
				Invalidate();
				m_nHoverLink = nHoverLink;
			}
		}
		else if( event.Type == UIEVENT_MOUSELEAVE && m_nHoverLink != -1 ) {
			Invalidate();
			m_nHoverLink = -1;
		}
	}

	void CListTextExtElementUI::NotifyInlineCellClick(const TEventUI& event, CListExUI* pListCtrl)
	{
		if( event.Type != UIEVENT_BUTTONUP || !m_pOwner->IsFocused() ) return;

		const int nColum = HitTestColum(event.ptMouse);
		RECT rc = { 0, 0, 0, 0 };
		if( nColum >= 0 ) {
			GetColumRect(nColum, rc);
		}
		pListCtrl->OnListItemClicked(GetIndex(), nColum, &rc, GetText(nColum));
	}

	void CListTextExtElementUI::HandleColumnCheckBoxEvent(const TEventUI& event, CListExUI* pListCtrl)
	{
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		for( int i = 0; i < pInfo->nColumns; ++i )
		{
			if( !pListCtrl->CheckColumCheckBoxable(i) ) continue;

			RECT rcCheckBox;
			GetCheckBoxRect(i, rcCheckBox);

			if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK )
			{
				if( ::PtInRect(&rcCheckBox, event.ptMouse) )
				{
					m_uCheckBoxState |= UISTATE_PUSHED | UISTATE_CAPTURED;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if( (m_uCheckBoxState & UISTATE_CAPTURED) != 0 )
				{
					if( ::PtInRect(&rcCheckBox, event.ptMouse) )
						m_uCheckBoxState |= UISTATE_PUSHED;
					else
						m_uCheckBoxState &= ~UISTATE_PUSHED;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uCheckBoxState & UISTATE_CAPTURED) != 0 )
				{
					if( ::PtInRect(&rcCheckBox, event.ptMouse) )
					{
						SetCheck(!GetCheck());
						if( m_pManager )
						{
							m_pManager->SendNotify(this, DUI_MSGTYPE_LISTITEMCHECKED, MAKEWPARAM(GetIndex(), 0), m_bChecked);
						}
					}
					m_uCheckBoxState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSEENTER )
			{
				if( ::PtInRect(&rcCheckBox, event.ptMouse) )
				{
					m_uCheckBoxState |= UISTATE_HOT;
					Invalidate();
				}
			}
			else if( event.Type == UIEVENT_MOUSELEAVE )
			{
				m_uCheckBoxState &= ~UISTATE_HOT;
				Invalidate();
			}
		}
	}

	void CListTextExtElementUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
			else CListLabelElementUI::DoEvent(event);
			return;
		}

		if( HandleLinkCursorEvent(event) ) return;
		if( HandleLinkClickEvent(event) ) return;
		UpdateLinkHoverState(event);

		CListExUI * pListCtrl = (CListExUI *)m_pOwner;
		NotifyInlineCellClick(event, pListCtrl);
		HandleColumnCheckBoxEvent(event, pListCtrl);

		CListLabelElementUI::DoEvent(event);
	}

	SIZE CListTextExtElementUI::EstimateSize(SIZE szAvailable)
	{
		TListInfoUI* pInfo = NULL;
		if( m_pOwner ) pInfo = m_pOwner->GetListInfo();

		SIZE cXY = m_cxyFixed;
		if( cXY.cy == 0 && m_pManager != NULL && pInfo != NULL) {
			cXY.cy = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight + 8;
			cXY.cy += pInfo->rcTextPadding.top + pInfo->rcTextPadding.bottom;
		}

		return cXY;
	}

	void CListTextExtElementUI::DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		if( m_pOwner == NULL ) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iTextColor = pInfo->dwTextColor;

		if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			iTextColor = pInfo->dwHotTextColor;
		}
		if( IsSelected() ) {
			iTextColor = pInfo->dwSelectedTextColor;
		}
		if( !IsEnabled() ) {
			iTextColor = pInfo->dwDisabledTextColor;
		}
		IListCallbackUI* pCallback = m_pOwner->GetTextCallback();
		//DUIASSERT(pCallback);
		//if( pCallback == NULL ) return;

		CListExUI * pListCtrl = (CListExUI *)m_pOwner;
		m_nLinks = 0;
		int nLinks = lengthof(m_rcLinks);
		const std::wstring rowText = CControlUI::GetText();
		const std::vector<std::wstring_view> rowTextParts = StringUtil::SplitView(rowText, L'|', false);
		const RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };

			DWORD iTextBkColor = 0;
			if (GetColumItemColor(i, iTextBkColor))
			{	
				CRenderEngine::DrawColor(renderContext, rcItem, iTextBkColor);
			}

			rcItem.left += rcTextPadding.left;
			rcItem.right -= rcTextPadding.right;
			rcItem.top += rcTextPadding.top;
			rcItem.bottom -= rcTextPadding.bottom;

			//婵☆偀鍋撻柡灞诲劜濡叉悂宕ラ敃鍌涗粯閻熸洑鐒﹀Ο澶岀矆缁″ゾeckBox
			if (pListCtrl->CheckColumCheckBoxable(i))
			{
				RECT rcCheckBox;
				GetCheckBoxRect(i, rcCheckBox);
				rcItem.left += (rcCheckBox.right - rcCheckBox.left);
			}

			std::wstring strText;//濞戞挸绉虫繛鍥偨閳┊CTSTR闁挎稑鑻幆渚€宕氬▎鎾搭€欓柛鎺曟硾閵囧﹥寰?by cddjr 2011/10/20
			if( pCallback ) strText = pCallback->GetItemText(this, m_iIndex, i);
			else if( i < m_aTexts.GetSize() && m_aTexts.GetAt(i) != NULL ) strText.assign(GetText(i));
			else if( i < static_cast<int>(rowTextParts.size()) ) strText.assign(rowTextParts[i]);
			if( pInfo->bShowHtml )
				CRenderEngine::DrawHtmlText(renderContext, rcItem, strText.c_str(), iTextColor, \
					&m_rcLinks[m_nLinks], &m_sLinks[m_nLinks], nLinks, pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
			else
		CRenderEngine::DrawText(renderContext, rcItem, strText.c_str(), iTextColor, \
			pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);

			m_nLinks += nLinks;
			nLinks = lengthof(m_rcLinks) - m_nLinks; 
		}
		for( int i = m_nLinks; i < lengthof(m_rcLinks); i++ ) {
			::ZeroMemory(m_rcLinks + i, sizeof(RECT));
			m_sLinks[i].clear();
		}
	}

	void CListTextExtElementUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		CListExUI * pListCtrl = (CListExUI *)m_pOwner;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			if (pListCtrl->CheckColumCheckBoxable(i))
			{
				RECT rcCheckBox;
				GetCheckBoxRect(i, rcCheckBox);

				m_uCheckBoxState &= ~UISTATE_PUSHED;

				if( (m_uCheckBoxState & UISTATE_SELECTED) != 0 ) {
					if( !m_sCheckBoxSelectedImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxSelectedImage, std::wstring(), rcCheckBox) ) {}
						else goto Label_ForeImage;
					}
				}

				if( IsFocused() ) m_uCheckBoxState |= UISTATE_FOCUSED;
				else m_uCheckBoxState &= ~ UISTATE_FOCUSED;
				if( !IsEnabled() ) m_uCheckBoxState |= UISTATE_DISABLED;
				else m_uCheckBoxState &= ~ UISTATE_DISABLED;

				if( (m_uCheckBoxState & UISTATE_DISABLED) != 0 ) {
					if( !m_sCheckBoxDisabledImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxDisabledImage, std::wstring(), rcCheckBox) ) {}
						else return;
					}
				}
				else if( (m_uCheckBoxState & UISTATE_PUSHED) != 0 ) {
					if( !m_sCheckBoxPushedImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxPushedImage, std::wstring(), rcCheckBox) ) {}
						else return;
					}
				}
				else if( (m_uCheckBoxState & UISTATE_HOT) != 0 ) {
					if( !m_sCheckBoxHotImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxHotImage, std::wstring(), rcCheckBox) ) {}
						else return;
					}
				}
				else if( (m_uCheckBoxState & UISTATE_FOCUSED) != 0 ) {
					if( !m_sCheckBoxFocusedImage.empty() ) {
					if( !DrawCheckBoxImage(renderContext, m_sCheckBoxFocusedImage, std::wstring(), rcCheckBox) ) {}
						else return;
					}
				}

				if( !m_sCheckBoxNormalImage.empty() ) {
				if( !DrawCheckBoxImage(renderContext, m_sCheckBoxNormalImage, std::wstring(), rcCheckBox) ) {}
					else return;
				}

			Label_ForeImage:
				if( !m_sCheckBoxForeImage.empty() ) {
				if( !DrawCheckBoxImage(renderContext, m_sCheckBoxForeImage, std::wstring(), rcCheckBox) ) {}
				}
			}
		}
	}

	BOOL CListTextExtElementUI::DrawCheckBoxImage(CPaintRenderContext& renderContext, const std::wstring& pStrImage, const std::wstring& pStrModify, RECT& rcCheckBox)
	{
		return CRenderEngine::DrawImageString(renderContext, rcCheckBox, pStrImage, pStrModify);
	}

	    void CListTextExtElementUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
    {
        const std::wstring_view name = StringUtil::TrimView(pstrNameView);
        if (name.empty()) {
            return;
        }

        if (StringUtil::EqualsNoCase(name, L"checkboxwidth")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetCheckBoxWidth(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"checkboxheight")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetCheckBoxHeight(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"checkboxnormalimage")) SetCheckBoxNormalImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxhotimage")) SetCheckBoxHotImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxpushedimage")) SetCheckBoxPushedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxfocusedimage")) SetCheckBoxFocusedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxdisabledimage")) SetCheckBoxDisabledImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxselectedimage")) SetCheckBoxSelectedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxforeimage")) SetCheckBoxForeImage(pstrValueView);
        else CListLabelElementUI::SetAttribute(pstrNameView, pstrValueView);
    }
	std::wstring_view CListTextExtElementUI::GetCheckBoxNormalImage()
	{
		return m_sCheckBoxNormalImage;
	}

	void CListTextExtElementUI::SetCheckBoxNormalImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxNormalImage = pStrImage;
	}

	std::wstring_view CListTextExtElementUI::GetCheckBoxHotImage()
	{
		return m_sCheckBoxHotImage;
	}

	void CListTextExtElementUI::SetCheckBoxHotImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxHotImage = pStrImage;
	}

	std::wstring_view CListTextExtElementUI::GetCheckBoxPushedImage()
	{
		return m_sCheckBoxPushedImage;
	}

	void CListTextExtElementUI::SetCheckBoxPushedImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxPushedImage = pStrImage;
	}

	std::wstring_view CListTextExtElementUI::GetCheckBoxFocusedImage()
	{
		return m_sCheckBoxFocusedImage;
	}

	void CListTextExtElementUI::SetCheckBoxFocusedImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxFocusedImage = pStrImage;
	}

	std::wstring_view CListTextExtElementUI::GetCheckBoxDisabledImage()
	{
		return m_sCheckBoxDisabledImage;
	}

	void CListTextExtElementUI::SetCheckBoxDisabledImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxDisabledImage = pStrImage;
	}
	std::wstring_view CListTextExtElementUI::GetCheckBoxSelectedImage()
	{
		return m_sCheckBoxSelectedImage;
	}

	void CListTextExtElementUI::SetCheckBoxSelectedImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxSelectedImage = pStrImage;
	}
	std::wstring_view CListTextExtElementUI::GetCheckBoxForeImage()
	{
		return m_sCheckBoxForeImage;
	}

	void CListTextExtElementUI::SetCheckBoxForeImage(std::wstring_view pStrImage)
	{
		m_sCheckBoxForeImage = pStrImage;
	}

	bool CListTextExtElementUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		(void)pStopControl;
		if( !::IntersectRect(&m_rcPaint, &renderContext.GetPaintRect(), &m_rcItem) ) return true;
		DrawItemBk(renderContext, m_rcItem);
		PaintStatusImage(renderContext);
		DrawItemText(renderContext, m_rcItem);
		return true;
	}

	void CListTextExtElementUI::GetCheckBoxRect(int nIndex, RECT &rc)
	{
		memset(&rc, 0x00, sizeof(rc));
		int nItemHeight = m_rcItem.bottom - m_rcItem.top;
		rc.left = m_rcItem.left + 6;
		rc.top = m_rcItem.top + (nItemHeight - GetCheckBoxHeight()) / 2;
		rc.right = rc.left + GetCheckBoxWidth();
		rc.bottom = rc.top + GetCheckBoxHeight();
	}
	int CListTextExtElementUI::GetCheckBoxWidth() const
	{
		return m_cxyCheckBox.cx;
	}

	void CListTextExtElementUI::SetCheckBoxWidth(int cx)
	{
		if( cx < 0 ) return; 
		m_cxyCheckBox.cx = cx;
	}

	int CListTextExtElementUI::GetCheckBoxHeight()  const 
	{
		return m_cxyCheckBox.cy;
	}

	void CListTextExtElementUI::SetCheckBoxHeight(int cy)
	{
		if( cy < 0 ) return; 
		m_cxyCheckBox.cy = cy;
	}

	void CListTextExtElementUI::SetCheck(BOOL bCheck)
	{
		if( m_bChecked == bCheck ) return;
		m_bChecked = bCheck;
		if( m_bChecked ) m_uCheckBoxState |= UISTATE_SELECTED;
		else m_uCheckBoxState &= ~UISTATE_SELECTED;
		Invalidate();
	}

	BOOL  CListTextExtElementUI::GetCheck() const
	{
		return m_bChecked;
	}

	int CListTextExtElementUI::HitTestColum(POINT ptMouse)
	{
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
			rcItem.left += pInfo->rcTextPadding.left;
			rcItem.right -= pInfo->rcTextPadding.right;
			rcItem.top += pInfo->rcTextPadding.top;
			rcItem.bottom -= pInfo->rcTextPadding.bottom;

			if( ::PtInRect(&rcItem, ptMouse)) 
			{
				return i;
			}
		}
		return -1;
	}

	BOOL CListTextExtElementUI::CheckColumEditable(int nColum)
	{
		return m_pOwner->CheckColumEditable(nColum);
	}
	void CListTextExtElementUI::GetColumRect(int nColum, RECT &rc)
	{
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		RECT rcOwnerPos = m_pOwner->GetPos();

		rc.left = pInfo->rcColumn[nColum].left + 1;
		rc.top  = 1;
		rc.right = pInfo->rcColumn[nColum].right - 1;
		rc.bottom = m_rcItem.bottom - m_rcItem.top - 1;
		OffsetRect(&rc, -rcOwnerPos.left, m_rcItem.top - rcOwnerPos.top);
	}

	void CListTextExtElementUI::SetColumItemColor(int nColum, DWORD iBKColor)
	{
		ColumCorlorArray[nColum].bEnable = TRUE;
		ColumCorlorArray[nColum].iBKColor = iBKColor;
		Invalidate();
	}
	BOOL CListTextExtElementUI::GetColumItemColor(int nColum, DWORD& iBKColor)
	{
		if (!ColumCorlorArray[nColum].bEnable)
		{
			return FALSE;
		}
		iBKColor = ColumCorlorArray[nColum].iBKColor;
		return TRUE;
	}

} // namespace DuiLib



