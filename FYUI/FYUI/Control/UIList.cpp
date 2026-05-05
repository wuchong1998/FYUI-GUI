#include "pch.h"
#include "UIList.h"
#include "../Core/Render/UIRenderContext.h"

namespace FYUI 
{
	namespace
	{
		LPVOID ListIndexToPtr(int value)
		{
			return reinterpret_cast<LPVOID>(static_cast<INT_PTR>(value));
		}

		int PtrToListIndex(LPVOID value)
		{
			return static_cast<int>(reinterpret_cast<INT_PTR>(value));
		}

		LPVOID ListColorToPtr(DWORD value)
		{
			return reinterpret_cast<LPVOID>(static_cast<UINT_PTR>(value));
		}

		DWORD PtrToListColor(LPVOID value)
		{
			return static_cast<DWORD>(reinterpret_cast<UINT_PTR>(value));
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListUI)

		CListUI::CListUI()
		: m_pCallback(NULL)
		, m_bScrollSelect(false)
		, m_iCurSel(-1)
		, m_iExpandedItem(-1)
		, m_bMultiSel(false)
		, m_iFirstSel(-1)
	{
		m_pList = new CListBodyUI(this);
		m_pHeader = new CListHeaderUI;
		Add(m_pHeader);
		Add(m_pList);

		// 閸掓銆冮柊宥囩枂
		m_ListInfo.nColumns = 0;
		m_ListInfo.nFont = -1;
		m_ListInfo.uTextStyle = DT_VCENTER | DT_SINGLELINE;
		m_ListInfo.dwTextColor = 0xFF000000;
		m_ListInfo.dwBkColor = 0;
		m_ListInfo.bAlternateBk = false;
		m_ListInfo.dwSelectedTextColor = 0xFF000000;
		m_ListInfo.dwSelectedBkColor = 0xFFC1E3FF;
		m_ListInfo.dwHotTextColor = 0xFF000000;
		m_ListInfo.dwHotBkColor = 0xFFE9F5FF;
		m_ListInfo.dwDisabledTextColor = 0xFFCCCCCC;
		m_ListInfo.dwDisabledBkColor = 0xFFFFFFFF;
		m_ListInfo.dwLineColor = 0;
		m_ListInfo.bShowRowLine = false;
		m_ListInfo.bShowColumnLine = false;
		m_ListInfo.bShowHtml = false;
		m_ListInfo.bMultiExpandable = false;
		m_ListInfo.bRSelected = false;
		::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
		::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));

	}

	std::wstring_view CListUI::GetClass() const
	{
		return _T("ListUI");
	}

	UINT CListUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP;
	}

	LPVOID CListUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LIST) == 0) return static_cast<CListUI*>(this);
		if (StringUtil::CompareNoCase(pstrName, _T("IList")) == 0) return static_cast<IListUI*>(this);
		if (StringUtil::CompareNoCase(pstrName, _T("IListOwner")) == 0) return static_cast<IListOwnerUI*>(this);
		return CVerticalLayoutUI::GetInterface(pstrName);
	}

	CControlUI* CListUI::GetItemAt(int iIndex) const
	{
		return m_pList->GetItemAt(iIndex);
	}

	int CListUI::GetItemIndex(CControlUI* pControl) const
	{
		if (pControl->GetInterface(_T("ListHeader")) != NULL) return CVerticalLayoutUI::GetItemIndex(pControl);
		// We also need to recognize header sub-items
		if (StringUtil::Find(pControl->GetClass(), _T("ListHeaderItemUI")) != -1) return m_pHeader->GetItemIndex(pControl);

		return m_pList->GetItemIndex(pControl);
	}

	bool CListUI::SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate)
	{
		if (pControl->GetInterface(_T("ListHeader")) != NULL) return CVerticalLayoutUI::SetItemIndex(pControl, iIndex);
		// We also need to recognize header sub-items
		if (StringUtil::Find(pControl->GetClass(), _T("ListHeaderItemUI")) != -1) return m_pHeader->SetItemIndex(pControl, iIndex);

		int iOrginIndex = m_pList->GetItemIndex(pControl);
		if (iOrginIndex == -1) return false;
		if (iOrginIndex == iIndex) return true;

		IListItemUI* pSelectedListItem = NULL;
		if (m_iCurSel >= 0) pSelectedListItem =
			static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
		if (!m_pList->SetItemIndex(pControl, iIndex, bUpdate)) return false;
		int iMinIndex = min(iOrginIndex, iIndex);
		int iMaxIndex = max(iOrginIndex, iIndex);
		for (int i = iMinIndex; i < iMaxIndex + 1; ++i) {
			CControlUI* p = m_pList->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if (pListItem != NULL) {
				pListItem->SetIndex(i);
			}
		}
		if (m_iCurSel >= 0 && pSelectedListItem != NULL) m_iCurSel = pSelectedListItem->GetIndex();
		return true;
	}

	int CListUI::GetCount() const
	{
		return m_pList->GetCount();
	}

	bool CListUI::Add(CControlUI* pControl)
	{

		// Override the Add() method so we can add items specifically to
		// the intended widgets. Headers are assumed to be
		// answer the correct interface so we can add multiple list headers.
		if (pControl->GetInterface(_T("ListHeader")) != NULL) {
			if (m_pHeader != pControl && m_pHeader->GetCount() == 0) {
				CVerticalLayoutUI::Remove(m_pHeader);
				m_pHeader = static_cast<CListHeaderUI*>(pControl);
			}
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return CVerticalLayoutUI::AddAt(pControl, 0);
		}
		// We also need to recognize header sub-items
		if (StringUtil::Find(pControl->GetClass(), _T("ListHeaderItemUI")) != -1) {
			bool ret = m_pHeader->Add(pControl);
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return ret;
		}
		// The list items should know about us
		std::wstring strName = GetName();
		std::wstring strName2 =pControl->GetName();
		std::wstring strClass(GetClass());

		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem == nullptr)
		{
			CVerticalLayoutUI* pVerLayout = dynamic_cast<CVerticalLayoutUI*>(pControl);
			if (pVerLayout != nullptr)
			{
				bool bLabelElementFound = false;
				for (int i = 0; i < pVerLayout->GetCount(); i++)
				{
					CListContainerElementUI* pContainerElement = dynamic_cast<CListContainerElementUI*>(pVerLayout->GetItemAt(i));
					if (pContainerElement != nullptr)
					{
						pListItem = static_cast<IListItemUI*>(pContainerElement->GetInterface(_T("ListItem")));
						pListItem->SetOwner(this);
						pListItem->SetIndex(GetCount());
					}
					else
					{
						CListLabelElementUI* pLabelElement = dynamic_cast<CListLabelElementUI*>(pVerLayout->GetItemAt(i));
						if (pLabelElement != nullptr)
						{
							std::wstring strName =pLabelElement->GetName();
							pListItem = static_cast<IListItemUI*>(pLabelElement->GetInterface(_T("ListLabelElement")));
							pListItem->SetOwner(this);
							pLabelElement->SetOwner(this);
							bLabelElementFound =true;
						}
					}
				}
				if (bLabelElementFound)
				{
					return m_pList->Add(pControl);
				}
			}

		}
		if (pListItem != NULL) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(GetCount());
			return m_pList->Add(pControl);
		}
		return CVerticalLayoutUI::Add(pControl);
	}

	bool CListUI::AddAt(CControlUI* pControl, int iIndex)
	{
		// Override the AddAt() method so we can add items specifically to
		// the intended widgets. Headers and are assumed to be
		// answer the correct interface so we can add multiple list headers.
		if (pControl->GetInterface(_T("ListHeader")) != NULL) {
			if (m_pHeader != pControl && m_pHeader->GetCount() == 0) {
				CVerticalLayoutUI::Remove(m_pHeader);
				m_pHeader = static_cast<CListHeaderUI*>(pControl);
			}
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return CVerticalLayoutUI::AddAt(pControl, 0);
		}
		// We also need to recognize header sub-items
		if (StringUtil::Find(pControl->GetClass(), _T("ListHeaderItemUI")) != -1) {
			bool ret = m_pHeader->AddAt(pControl, iIndex);
			m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
			return ret;
		}
		if (!m_pList->AddAt(pControl, iIndex)) return false;

		// The list items should know about us
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if (pListItem != NULL) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(iIndex);
		}

			ReindexListItems(iIndex + 1);
			ShiftSelectionIndices(iIndex, 1);
			ShiftTrackedItemIndices(iIndex, 1);
			if (m_iCurSel >= iIndex) m_iCurSel += 1;
			if (m_iFirstSel >= iIndex) m_iFirstSel += 1;
			return true;
		}

	bool CListUI::Remove(CControlUI* pControl)
	{
		if (pControl->GetInterface(_T("ListHeader")) != NULL) return CVerticalLayoutUI::Remove(pControl);
		// We also need to recognize header sub-items
		if (StringUtil::Find(pControl->GetClass(), _T("ListHeaderItemUI")) != -1) return m_pHeader->Remove(pControl);

		int iIndex = m_pList->GetItemIndex(pControl);
		if (iIndex == -1) return false;

			if (!m_pList->RemoveAt(iIndex)) return false;
	
			ReindexListItems(iIndex);
			RestoreTrackedStateAfterRemoval(iIndex);
			RestoreSelectionAfterRemoval(iIndex);
			return true;
		}

	bool CListUI::RemoveAt(int iIndex)
	{
			if (!m_pList->RemoveAt(iIndex)) return false;
	
			ReindexListItems(iIndex);
			RestoreTrackedStateAfterRemoval(iIndex);
			RestoreSelectionAfterRemoval(iIndex);
			return true;
		}

		void CListUI::RemoveAll()
		{
			m_iCurSel = -1;
			m_iFirstSel = -1;
			m_iCurSelActivate = -1;
			m_iExpandedItem = -1;
			m_aSelItems.Empty();
			m_pList->RemoveAll();
		}

	void CListUI::SetPos(RECT rc, bool bNeedInvalidate)
	{

		CVerticalLayoutUI::SetPos(rc, bNeedInvalidate);

		if (m_pHeader == NULL) return;
		// Determine general list information and the size of header columns
		m_ListInfo.nColumns = MIN(m_pHeader->GetCount(), UILIST_MAX_COLUMNS);
		// The header/columns may or may not be visible at runtime. In either case
		// we should determine the correct dimensions...

		if (!m_pHeader->IsVisible()) {
			for (int it = 0; it < m_pHeader->GetCount(); it++) {
				static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(true);
			}
			m_pHeader->SetPos(CDuiRect(rc.left, 0, rc.right, 0), bNeedInvalidate);
		}

		for (int i = 0; i < m_ListInfo.nColumns; i++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_pHeader->GetItemAt(i));
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;
			RECT rcPos = pControl->GetPos();
			m_ListInfo.rcColumn[i] = pControl->GetPos();
		}
		if (!m_pHeader->IsVisible()) {
			for (int it = 0; it < m_pHeader->GetCount(); it++) {
				static_cast<CControlUI*>(m_pHeader->GetItemAt(it))->SetInternVisible(false);
			}
		}
		m_pList->SetPos(m_pList->GetPos(), bNeedInvalidate);
	}

	void CListUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CVerticalLayoutUI::Move(szOffset, bNeedInvalidate);
		if (!m_pHeader->IsVisible()) m_pHeader->Move(szOffset, false);
	}

	CListUI* CListUI::Clone()
	{
		CListUI* pClone = new CListUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CListUI::CopyData(CListUI* pControl)
	{

		m_bScrollSelect = pControl->m_bScrollSelect;
		m_bMultiSel = pControl->m_bMultiSel;
		m_iCurSel = pControl->m_iCurSel;
		m_iFirstSel = pControl->m_iFirstSel;

		for (int i = 0; i < pControl->m_aSelItems.GetSize(); ++i)
		{
			int index = PtrToListIndex(pControl->m_aSelItems.GetAt(i));
			m_aSelItems.Add(ListIndexToPtr(index));
		}

		m_iCurSelActivate = pControl->m_iCurSelActivate;  // 閸欏苯鍤惃鍕灙
		m_iExpandedItem = pControl->m_iExpandedItem;
		m_ListInfo= pControl->m_ListInfo;	
		__super::CopyData(pControl);
	}

	bool CListUI::ResolveListItemByIndex(int iIndex, CControlUI*& pControl, IListItemUI*& pListItem, bool bRequireEnabled, bool bRequireVisible) const
	{
		pControl = NULL;
		pListItem = NULL;
		if( iIndex < 0 || iIndex >= GetCount() ) return false;

		pControl = GetItemAt(iIndex);
		if( pControl == NULL ) return false;
		if( bRequireVisible && !pControl->IsVisible() ) return false;
		if( bRequireEnabled && !pControl->IsEnabled() ) return false;

		pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		return pListItem != NULL;
	}

	void CListUI::ReindexListItems(int iStart)
	{
		for( int i = max(iStart, 0); i < m_pList->GetCount(); ++i ) {
			CControlUI* pControl = m_pList->GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) {
				pListItem->SetIndex(i);
			}
		}
	}

	void CListUI::ShiftSelectionIndices(int iStartIndex, int iDelta)
	{
		if( iDelta == 0 ) return;

		for( int i = 0; i < m_aSelItems.GetSize(); ++i ) {
			int iSelIndex = PtrToListIndex(m_aSelItems.GetAt(i));
			if( iSelIndex < iStartIndex ) continue;

			iSelIndex += iDelta;
			if( iSelIndex < 0 ) {
				m_aSelItems.Remove(i);
				--i;
				continue;
			}
			m_aSelItems.SetAt(i, ListIndexToPtr(iSelIndex));
		}
	}

		void CListUI::RestoreSelectionAfterRemoval(int iRemovedIndex)
		{
			RemoveSelectionIndex(iRemovedIndex);
			ShiftSelectionIndices(iRemovedIndex + 1, -1);
			if( m_iFirstSel == iRemovedIndex ) m_iFirstSel = -1;
		else if( m_iFirstSel > iRemovedIndex ) m_iFirstSel -= 1;

		if( iRemovedIndex == m_iCurSel && m_iCurSel >= 0 ) {
			const int iSel = m_iCurSel;
			m_iCurSel = -1;
			SelectItem(FindSelectable(iSel, false));
		}
			else if( iRemovedIndex < m_iCurSel ) {
				m_iCurSel -= 1;
			}
		}

		void CListUI::ShiftTrackedItemIndices(int iStartIndex, int iDelta)
		{
			if( iDelta == 0 ) return;

			if( m_iCurSelActivate >= iStartIndex ) {
				m_iCurSelActivate += iDelta;
			}
			if( m_iExpandedItem >= iStartIndex ) {
				m_iExpandedItem += iDelta;
			}
		}

		void CListUI::RestoreTrackedStateAfterRemoval(int iRemovedIndex)
		{
			if( m_iCurSelActivate == iRemovedIndex ) m_iCurSelActivate = -1;
			else if( m_iCurSelActivate > iRemovedIndex ) m_iCurSelActivate -= 1;

			if( m_iExpandedItem == iRemovedIndex ) m_iExpandedItem = -1;
			else if( m_iExpandedItem > iRemovedIndex ) m_iExpandedItem -= 1;
		}

	void CListUI::AddSelectionIndex(int iIndex)
	{
		if( m_aSelItems.Find(ListIndexToPtr(iIndex)) == -1 ) {
			m_aSelItems.Add(ListIndexToPtr(iIndex));
		}
	}

	void CListUI::RemoveSelectionIndex(int iIndex)
	{
		int aIndex = m_aSelItems.Find(ListIndexToPtr(iIndex));
		if( aIndex >= 0 ) {
			m_aSelItems.Remove(aIndex);
		}
	}

	void CListUI::NotifySelectionChanged(int iIndex)
	{
		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, iIndex);
		}
	}

	void CListUI::NotifySelectionChanged(int iIndex, int iAnchorIndex)
	{
		if( m_pManager != NULL ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, iIndex, iAnchorIndex);
		}
	}

	void CListUI::NotifySelectionCleared()
	{
		if( IsMultiSelect() && m_pManager != NULL ) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, -1, -1);
		}
	}

	void CListUI::ApplySelectionFocusAndScroll(CControlUI* pControl, int iIndex, bool bTakeFocus, bool bSetScrollPos)
	{
		if( bSetScrollPos ) {
			EnsureVisible(iIndex);
		}
		if( bTakeFocus && pControl != NULL ) {
			pControl->SetFocus();
		}
	}

	void CListUI::SelectItemRange(int iStart, int iEnd)
	{
		if( iStart > iEnd ) {
			std::swap(iStart, iEnd);
		}

		for( int iIndex = iStart; iIndex <= iEnd; ++iIndex ) {
			CControlUI* pControl = NULL;
			IListItemUI* pListItem = NULL;
			if( !ResolveListItemByIndex(iIndex, pControl, pListItem, true, false) ) continue;
			if( !pListItem->SelectMulti(true) ) continue;
			AddSelectionIndex(iIndex);
		}
	}

	RECT CListUI::GetListViewportRect() const
	{
		RECT rcList = m_pList->GetPos();
		RECT rcListInset = m_pList->GetInset();

		rcList.left += rcListInset.left;
		rcList.top += rcListInset.top;
		rcList.right -= rcListInset.right;
		rcList.bottom -= rcListInset.bottom;

		CScrollBarUI* pHorizontalScrollBar = m_pList->GetHorizontalScrollBar();
		if( pHorizontalScrollBar != NULL && pHorizontalScrollBar->IsVisible() ) {
			rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();
		}
		return rcList;
	}



	int CListUI::GetMinSelItemIndex()
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;
		int min = PtrToListIndex(m_aSelItems.GetAt(0));
		int index;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			index = PtrToListIndex(m_aSelItems.GetAt(i));
			if (min > index)
				min = index;
		}
		return min;
	}

	int CListUI::GetMaxSelItemIndex()
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;
		int max = PtrToListIndex(m_aSelItems.GetAt(0));
		int index;
		for (int i = 0; i < m_aSelItems.GetSize(); ++i)
		{
			index = PtrToListIndex(m_aSelItems.GetAt(i));
			if (max < index)
				max = index;
		}
		return max;
	}

	void CListUI::UpdateListFocusState(bool bFocused)
	{
		m_bFocused = bFocused;
	}

	bool CListUI::HandleListKeyDownEvent(const TEventUI& event)
	{
		if( event.Type != UIEVENT_KEYDOWN ) return false;

		switch( event.chKey ) {
		case VK_UP:
			if( m_aSelItems.GetSize() > 0 ) {
				int index = GetMinSelItemIndex() - 1;
				UnSelectAllItems();
				index > 0 ? SelectItem(index, true) : SelectItem(0, true);
			}
			return true;
		case VK_DOWN:
			if( m_aSelItems.GetSize() > 0 ) {
				int index = GetMaxSelItemIndex() + 1;
				UnSelectAllItems();
				index + 1 > m_pList->GetCount() ? SelectItem(GetCount() - 1, true) : SelectItem(index, true);
			}
			return true;
		case VK_PRIOR:
			PageUp();
			return true;
		case VK_NEXT:
			PageDown();
			return true;
		case VK_HOME:
			SelectItem(FindSelectable(0, false), true);
			return true;
		case VK_END:
			SelectItem(FindSelectable(GetCount() - 1, true), true);
			return true;
		case VK_RETURN:
			if( m_iCurSel != -1 ) GetItemAt(m_iCurSel)->Activate();
			return true;
		case 0x41:
			if( IsMultiSelect() && (GetKeyState(VK_CONTROL) & 0x8000) ) {
				UnSelectAllItems();
				SelectAllItems();
			}
			return true;
		}
		return false;
	}

	bool CListUI::HandleListScrollWheelEvent(const TEventUI& event)
	{
		if( event.Type != UIEVENT_SCROLLWHEEL ) return false;

		switch( LOWORD(event.wParam) ) {
		case SB_LINEUP:
			if( m_bScrollSelect && !IsMultiSelect() ) {
				const int next = FindSelectable(m_iCurSel - 1, false);
				if( next < 0 || next == m_iCurSel ) return false;
				SelectItem(next, true);
			}
			else {
				const SIZE scrollPos = GetScrollPos();
				if( scrollPos.cy <= 0 ) return false;
				LineUp();
			}
			return true;
		case SB_LINEDOWN:
			if( m_bScrollSelect && !IsMultiSelect() ) {
				const int next = FindSelectable(m_iCurSel + 1, true);
				if( next < 0 || next == m_iCurSel ) return false;
				SelectItem(next, true);
			}
			else {
				const SIZE scrollPos = GetScrollPos();
				const SIZE scrollRange = GetScrollRange();
				if( scrollPos.cy >= scrollRange.cy ) return false;
				LineDown();
			}
			return true;
		}
		return false;
	}

	void CListUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pParent != NULL) m_pParent->DoEvent(event);
			else CVerticalLayoutUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS)
		{
			UpdateListFocusState(true);
			return;
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			UpdateListFocusState(false);
			return;
		}

		if( HandleListKeyDownEvent(event) ) return;
		if( HandleListScrollWheelEvent(event) ) return;
		CVerticalLayoutUI::DoEvent(event);
	}

	CListHeaderUI* CListUI::GetHeader() const
	{
		return m_pHeader;
	}

	CContainerUI* CListUI::GetList() const
	{
		return m_pList;
	}

	bool CListUI::GetScrollSelect()
	{
		return m_bScrollSelect;
	}

	void CListUI::SetScrollSelect(bool bScrollSelect)
	{
		m_bScrollSelect = bScrollSelect;
	}

	int CListUI::GetCurSelActivate() const
	{
		return m_iCurSelActivate;
	}

	bool CListUI::SelectItemActivate(int iIndex)
	{
		if (!SelectItem(iIndex, true)) {
			return false;
		}

		m_iCurSelActivate = iIndex;
		return true;
	}

	int CListUI::GetCurSel() const
	{
		return m_iCurSel;
	}

	bool CListUI::SelectItem(int iIndex, bool bTakeFocus, bool bIsClick, bool bSetScrollPos)
	{
		// 闁插秶鐤嗘径姘垛偓澶庢崳婵绨崣?
		m_iFirstSel = -1;
		const int iLastSel = m_iCurSel;
		// 閸欐牗绉烽崗璺虹暊闁瀚ㄦい?
		const int nSelCountBefore = m_aSelItems.GetSize();
		UnSelectItem(iIndex, true);
		// 閸掋倖鏌囬弰顖氭儊閸氬牊纭堕崚妤勩€冩い?
		if (iIndex < 0) {
			if( nSelCountBefore > 0 && m_aSelItems.GetSize() == 0 ) {
				NotifySelectionCleared();
			}
			return false;
		}
		// 闁瀚ㄨぐ鎾冲閸掓銆冩い?
		CControlUI* pControl = NULL;
		IListItemUI* pListItem = NULL;
		if (!ResolveListItemByIndex(iIndex, pControl, pListItem, false, false)) return false;
		// 瀹歌尙绮￠柅澶嬪
		if (m_aSelItems.Find(ListIndexToPtr(iIndex)) != -1) {
			m_iCurSel = iIndex;
			ApplySelectionFocusAndScroll(pControl, iIndex, bTakeFocus, bSetScrollPos);
			if (nSelCountBefore > 1 || iLastSel != m_iCurSel) {
				NotifySelectionChanged(iIndex);
			}
			return true;
		}
		if (!pListItem->Select(true, bIsClick)) {
			return false;
		}
		m_iCurSel = iIndex;
		AddSelectionIndex(iIndex);

		ApplySelectionFocusAndScroll(pControl, iIndex, bTakeFocus, bSetScrollPos);
		if (m_pManager != NULL && iLastSel != m_iCurSel)
		{
			NotifySelectionChanged(iIndex);
		}

		return true;
	}

	bool CListUI::SelectMultiItem(int iIndex, bool bTakeFocus)
	{
		// 閺堫亜绱戦崥顖氼樋闁?
		if (!IsMultiSelect()) return SelectItem(iIndex, bTakeFocus);
		// 閸忋劑鍎撮崣鏍ㄧХ
		if (iIndex < 0) {
			const bool bHadSelection = m_aSelItems.GetSize() > 0;
			UnSelectAllItems();
			if( bHadSelection ) {
				NotifySelectionCleared();
			}
			return true;
		}

		// 婢舵岸鈧鎹ｆ慨瀣碍閸?
		if (m_iFirstSel == -1) {
			if (m_iCurSel != -1) {
				m_iFirstSel = m_iCurSel;
			}
			else {
				m_iFirstSel = iIndex;
			}
		}

		CControlUI* pControl = NULL;
		IListItemUI* pListItem = NULL;
		if (!ResolveListItemByIndex(iIndex, pControl, pListItem, true, false)) return false;

		// 婢舵岸鈧鍨介弬?
		if ((GetKeyState(VK_CONTROL) & 0x8000)) {
			int aIndex = m_aSelItems.Find(ListIndexToPtr(iIndex));
			if (aIndex != -1) {
				if (!pListItem->SelectMulti(false)) return false;
				m_aSelItems.Remove(aIndex);
				if (m_iCurSel == iIndex) {
					m_iCurSel = (m_aSelItems.GetSize() > 0) ? PtrToListIndex(m_aSelItems.GetAt(m_aSelItems.GetSize() - 1)) : -1;
				}
				if (m_iFirstSel == iIndex) {
					m_iFirstSel = m_iCurSel;
				}
				NotifySelectionChanged(m_iCurSel);
			}
			else {
				if (!pListItem->SelectMulti(true)) return false;

				m_iCurSel = iIndex;
				AddSelectionIndex(iIndex);
				ApplySelectionFocusAndScroll(pControl, iIndex, bTakeFocus, true);
				NotifySelectionChanged(iIndex);
			}
		}
		else if ((GetKeyState(VK_SHIFT) & 0x8000)) {
			const int iRangeAnchor = m_iFirstSel;
			UnSelectAllItems();
			m_iFirstSel = iRangeAnchor;
			SelectItemRange(iRangeAnchor, iIndex);
			m_iCurSel = iIndex;
			ApplySelectionFocusAndScroll(pControl, iIndex, bTakeFocus, true);
			NotifySelectionChanged(iIndex, iRangeAnchor);
		}
		else {
			UnSelectAllItems();
			if (!pListItem->SelectMulti(true)) return false;

			m_iCurSel = iIndex;
			m_iFirstSel = iIndex;
			AddSelectionIndex(iIndex);
			ApplySelectionFocusAndScroll(pControl, iIndex, bTakeFocus, true);
			NotifySelectionChanged(iIndex);
		}
		return true;
	}

	void CListUI::SetMultiSelect(bool bMultiSel)
	{
		if( m_bMultiSel == bMultiSel ) return;

		int iKeepSel = m_iCurSel;
		if( !bMultiSel && iKeepSel < 0 && m_aSelItems.GetSize() > 0 ) {
			iKeepSel = PtrToListIndex(m_aSelItems.GetAt(0));
		}

		m_bMultiSel = bMultiSel;
		if( !bMultiSel ) {
			UnSelectAllItems();
			if( iKeepSel >= 0 ) {
				SelectItem(iKeepSel, false, false, false);
			}
		}
	}

	bool CListUI::IsMultiSelect() const
	{
		return m_bMultiSel;
	}

	bool CListUI::UnSelectItem(int iIndex, bool bOthers)
	{
		if (bOthers) {
			for (int i = m_aSelItems.GetSize() - 1; i >= 0; --i) {
				int iSelIndex = PtrToListIndex(m_aSelItems.GetAt(i));
				if (iSelIndex == iIndex) continue;
				CControlUI* pControl = NULL;
				IListItemUI* pSelListItem = NULL;
				if (!ResolveListItemByIndex(iSelIndex, pControl, pSelListItem, false, false)) continue;
				if (!pSelListItem->SelectMulti(false)) continue;
				if (m_iCurSel == iSelIndex) m_iCurSel = -1;
				m_aSelItems.Remove(i);
			}
		}
		else {
			CControlUI* pControl = NULL;
			IListItemUI* pListItem = NULL;
			if (!ResolveListItemByIndex(iIndex, pControl, pListItem, false, false)) return false;
			int aIndex = m_aSelItems.Find(ListIndexToPtr(iIndex));
			if (aIndex < 0) return false;
			if (!pListItem->SelectMulti(false)) return false;
			m_aSelItems.Remove(aIndex);
			if (m_iCurSel == iIndex) {
				m_iCurSel = (m_aSelItems.GetSize() > 0) ? PtrToListIndex(m_aSelItems.GetAt(m_aSelItems.GetSize() - 1)) : -1;
			}
			if (m_iFirstSel == iIndex) {
				m_iFirstSel = m_iCurSel;
			}
		}
		return true;
	}

	void CListUI::SelectAllItems()
	{
		m_iFirstSel = -1;
		for (int i = 0; i < GetCount(); ++i) {
			CControlUI* pControl = NULL;
			IListItemUI* pListItem = NULL;
			if (!ResolveListItemByIndex(i, pControl, pListItem, true, true)) continue;
			if (!pListItem->SelectMulti(true)) continue;
			AddSelectionIndex(i);
			if( m_iFirstSel == -1 ) m_iFirstSel = i;
			m_iCurSel = i;
		}

		if( m_iCurSel >= 0 ) {
			NotifySelectionChanged(m_iCurSel, m_iFirstSel);
		}
	}

	void CListUI::UnSelectAllItems()
	{
		for (int i = 0; i < m_aSelItems.GetSize(); ++i) {
			int iSelIndex = PtrToListIndex(m_aSelItems.GetAt(i));
			CControlUI* pControl = NULL;
			IListItemUI* pListItem = NULL;
			if (!ResolveListItemByIndex(iSelIndex, pControl, pListItem, false, false)) continue;
			if (!pListItem->SelectMulti(false)) continue;
		}
		m_aSelItems.Empty();
		m_iCurSel = -1;
		m_iFirstSel = -1;
	}

	int CListUI::GetSelectItemCount() const
	{
		return m_aSelItems.GetSize();
	}

	int CListUI::GetNextSelItem(int nItem) const
	{
		if (m_aSelItems.GetSize() <= 0)
			return -1;

		if (nItem < 0) {
			return PtrToListIndex(m_aSelItems.GetAt(0));
		}
		int aIndex = m_aSelItems.Find(ListIndexToPtr(nItem));
		if (aIndex < 0) return -1;
		if (aIndex + 1 > m_aSelItems.GetSize() - 1)
			return -1;
		return PtrToListIndex(m_aSelItems.GetAt(aIndex + 1));
	}

	UINT CListUI::GetListType()
	{
		return LT_LIST;
	}

	TListInfoUI* CListUI::GetListInfo()
	{
		return &m_ListInfo;
	}

	bool CListUI::IsDelayedDestroy() const
	{
		return m_pList->IsDelayedDestroy();
	}

	void CListUI::SetDelayedDestroy(bool bDelayed)
	{
		m_pList->SetDelayedDestroy(bDelayed);
	}

	int CListUI::GetChildPadding() const
	{
		return m_pList->GetChildPadding();
	}

	void CListUI::SetChildPadding(int iPadding)
	{
		m_pList->SetChildPadding(iPadding);
	}

	void CListUI::SetItemFont(int index)
	{
		m_ListInfo.nFont = index;
		NeedUpdate();
	}

	void CListUI::SetItemTextStyle(UINT uStyle)
	{
		m_ListInfo.uTextStyle = uStyle;
		NeedUpdate();
	}

	void CListUI::SetItemTextPadding(RECT rc)
	{
		m_ListInfo.rcTextPadding = rc;
		NeedUpdate();
	}

	RECT CListUI::GetItemTextPadding() const
	{
		RECT rect = m_ListInfo.rcTextPadding;
		GetManager()->ScaleRect(&rect);
		return rect;
	}

	void CListUI::SetItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwTextColor = dwTextColor;
		Invalidate();
	}

	void CListUI::SetItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwBkColor = dwBkColor;
		Invalidate();
	}

	void CListUI::SetItemBkImage(std::wstring_view pStrImage)
	{
		m_ListInfo.sBkImage = pStrImage;
		Invalidate();
	}

	void CListUI::SetAlternateBk(bool bAlternateBk)
	{
		m_ListInfo.bAlternateBk = bAlternateBk;
		Invalidate();
	}

	DWORD CListUI::GetItemTextColor() const
	{
		return m_ListInfo.dwTextColor;
	}

	DWORD CListUI::GetItemBkColor() const
	{
		return m_ListInfo.dwBkColor;
	}

	std::wstring_view CListUI::GetItemBkImage() const
	{
		return m_ListInfo.sBkImage;
	}

	bool CListUI::IsAlternateBk() const
	{
		return m_ListInfo.bAlternateBk;
	}

	void CListUI::SetSelectedItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwSelectedTextColor = dwTextColor;
		Invalidate();
	}

	void CListUI::SetSelectedItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwSelectedBkColor = dwBkColor;
		Invalidate();
	}

	void CListUI::SetSelectedItemImage(std::wstring_view pStrImage)
	{
		m_ListInfo.sSelectedImage = pStrImage;
		Invalidate();
	}

	DWORD CListUI::GetSelectedItemTextColor() const
	{
		return m_ListInfo.dwSelectedTextColor;
	}

	DWORD CListUI::GetSelectedItemBkColor() const
	{
		return m_ListInfo.dwSelectedBkColor;
	}

	std::wstring_view CListUI::GetSelectedItemImage() const
	{
		return m_ListInfo.sSelectedImage;
	}

	void CListUI::SetHotItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwHotTextColor = dwTextColor;
		Invalidate();
	}

	void CListUI::SetHotItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwHotBkColor = dwBkColor;
		Invalidate();
	}

	void CListUI::SetHotItemImage(std::wstring_view pStrImage)
	{
		m_ListInfo.sHotImage = pStrImage;
		Invalidate();
	}

	DWORD CListUI::GetHotItemTextColor() const
	{
		return m_ListInfo.dwHotTextColor;
	}
	DWORD CListUI::GetHotItemBkColor() const
	{
		return m_ListInfo.dwHotBkColor;
	}

	std::wstring_view CListUI::GetHotItemImage() const
	{
		return m_ListInfo.sHotImage;
	}

	void CListUI::SetDisabledItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	void CListUI::SetDisabledItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwDisabledBkColor = dwBkColor;
		Invalidate();
	}

	void CListUI::SetDisabledItemImage(std::wstring_view pStrImage)
	{
		m_ListInfo.sDisabledImage = pStrImage;
		Invalidate();
	}

	DWORD CListUI::GetDisabledItemTextColor() const
	{
		return m_ListInfo.dwDisabledTextColor;
	}

	DWORD CListUI::GetDisabledItemBkColor() const
	{
		return m_ListInfo.dwDisabledBkColor;
	}

	std::wstring_view CListUI::GetDisabledItemImage() const
	{
		return m_ListInfo.sDisabledImage;
	}

	DWORD CListUI::GetItemLineColor() const
	{
		return m_ListInfo.dwLineColor;
	}

	void CListUI::SetItemLineColor(DWORD dwLineColor)
	{
		m_ListInfo.dwLineColor = dwLineColor;
		Invalidate();
	}
	void CListUI::SetItemShowRowLine(bool bShowLine)
	{
		m_ListInfo.bShowRowLine = bShowLine;
		Invalidate();
	}
	void CListUI::SetItemShowColumnLine(bool bShowLine)
	{
		m_ListInfo.bShowColumnLine = bShowLine;
		Invalidate();
	}
	bool CListUI::IsItemShowHtml()
	{
		return m_ListInfo.bShowHtml;
	}

	void CListUI::SetItemShowHtml(bool bShowHtml)
	{
		if (m_ListInfo.bShowHtml == bShowHtml) return;

		m_ListInfo.bShowHtml = bShowHtml;
		NeedUpdate();
	}

	bool CListUI::IsItemRSelected()
	{
		return m_ListInfo.bRSelected;
	}

	void CListUI::SetItemRSelected(bool bSelected)
	{
		if (m_ListInfo.bRSelected == bSelected) return;

		m_ListInfo.bRSelected = bSelected;
		NeedUpdate();
	}

	void CListUI::SetMultiExpanding(bool bMultiExpandable)
	{
		m_ListInfo.bMultiExpandable = bMultiExpandable;
	}

	bool CListUI::ExpandItem(int iIndex, bool bExpand /*= true*/)
	{
		if (m_iExpandedItem >= 0 && !m_ListInfo.bMultiExpandable) {
			CControlUI* pControl = GetItemAt(m_iExpandedItem);
			if (pControl != NULL) {
				IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
				if (pItem != NULL) pItem->Expand(false);
			}
			m_iExpandedItem = -1;
		}
		if (bExpand) {
			CControlUI* pControl = GetItemAt(iIndex);
			if (pControl == NULL) return false;
			if (!pControl->IsVisible()) return false;
			IListItemUI* pItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if (pItem == NULL) return false;
			m_iExpandedItem = iIndex;
			if (!pItem->Expand(true)) {
				m_iExpandedItem = -1;
				return false;
			}
		}
		NeedUpdate();
		return true;
	}

	int CListUI::GetExpandedItem() const
	{
		return m_iExpandedItem;
	}

	void CListUI::EnsureVisible(int iIndex)
	{
		if( iIndex < 0 ) return;
		CControlUI* pControl = GetItemAt(iIndex);
		if( pControl == NULL ) return;
		RECT rcItem = pControl->GetPos();
		RECT rcList = GetListViewportRect();

		if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
		int dx = 0;
		if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
		if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
		Scroll(0, dx);
	}

	void CListUI::Scroll(int dx, int dy)
	{
		if (dx == 0 && dy == 0) return;
		SIZE sz = m_pList->GetScrollPos();
		m_pList->SetScrollPos(CDuiSize(sz.cx + dx, sz.cy + dy));
	}

	    void CListUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
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

        if (StringUtil::EqualsNoCase(name, L"header")) GetHeader()->SetVisible(!StringUtil::EqualsNoCase(pstrValueView, L"hidden"));
        else if (StringUtil::EqualsNoCase(name, L"headerbkimage")) GetHeader()->SetBkImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"scrollselect")) SetScrollSelect(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"multiexpanding")) SetMultiExpanding(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"itemfont")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) m_ListInfo.nFont = value;
        }
        else if (StringUtil::EqualsNoCase(name, L"itemalign")) {
            if (contains(L"left")) {
                m_ListInfo.uTextStyle &= ~(DT_CENTER | DT_RIGHT);
                m_ListInfo.uTextStyle |= DT_LEFT;
            }
            if (contains(L"center")) {
                m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_RIGHT);
                m_ListInfo.uTextStyle |= DT_CENTER;
            }
            if (contains(L"right")) {
                m_ListInfo.uTextStyle &= ~(DT_LEFT | DT_CENTER);
                m_ListInfo.uTextStyle |= DT_RIGHT;
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"itemvalign")) {
            if (contains(L"top")) {
                m_ListInfo.uTextStyle &= ~(DT_VCENTER | DT_BOTTOM);
                m_ListInfo.uTextStyle |= DT_TOP;
            }
            if (contains(L"vcenter")) {
                m_ListInfo.uTextStyle &= ~(DT_TOP | DT_BOTTOM | DT_WORDBREAK);
                m_ListInfo.uTextStyle |= DT_VCENTER | DT_SINGLELINE;
            }
            if (contains(L"bottom")) {
                m_ListInfo.uTextStyle &= ~(DT_TOP | DT_VCENTER);
                m_ListInfo.uTextStyle |= DT_BOTTOM;
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"itemendellipsis")) {
            if (StringUtil::ParseBool(pstrValueView)) m_ListInfo.uTextStyle |= DT_END_ELLIPSIS;
            else m_ListInfo.uTextStyle &= ~DT_END_ELLIPSIS;
        }
        else if (StringUtil::EqualsNoCase(name, L"itemtextpadding")) {
            RECT rect = { 0 };
            if (StringUtil::TryParseRect(pstrValueView, rect)) SetItemTextPadding(rect);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemtextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetItemTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itembkcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetItemBkColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itembkimage")) SetItemBkImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"itemaltbk")) SetAlternateBk(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"itemforeimage")) { m_ListInfo.sForeImage = pstrValueView; Invalidate(); }
        else if (StringUtil::EqualsNoCase(name, L"itemhotforeimage")) { m_ListInfo.sHotForeImage = pstrValueView; Invalidate(); }
        else if (StringUtil::EqualsNoCase(name, L"itemselectedforeimage")) { m_ListInfo.sSelectedForeImage = pstrValueView; Invalidate(); }
        else if (StringUtil::EqualsNoCase(name, L"itemselectedtextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetSelectedItemTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemselectedbkcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetSelectedItemBkColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemselectedimage")) SetSelectedItemImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"itemhottextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetHotItemTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemhotbkcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetHotItemBkColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemhotimage")) SetHotItemImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"itemdisabledtextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetDisabledItemTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemdisabledbkcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetDisabledItemBkColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemdisabledimage")) SetDisabledItemImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"itemlinecolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetItemLineColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemshowrowline")) SetItemShowRowLine(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"itemshowcolumnline")) SetItemShowColumnLine(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"itemshowhtml")) SetItemShowHtml(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"multiselect")) SetMultiSelect(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"itemrselected")) SetItemRSelected(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"quick_scroll")) {
            const bool quickScrolling = StringUtil::ParseBool(pstrValueView);
            SetQuickScrolling(quickScrolling);
            m_pList->SetQuickScrolling(quickScrolling);
        }
        else CVerticalLayoutUI::SetAttribute(pstrNameView, pstrValueView);
    }

	IListCallbackUI* CListUI::GetTextCallback() const
	{
		return m_pCallback;
	}

	void CListUI::SetTextCallback(IListCallbackUI* pCallback)
	{
		m_pCallback = pCallback;
	}

	SIZE CListUI::GetScrollPos() const
	{
		return m_pList->GetScrollPos();
	}

	SIZE CListUI::GetScrollRange() const
	{
		return m_pList->GetScrollRange();
	}

	void CListUI::SetScrollPos(SIZE szPos, bool bMsg)
	{
		m_pList->SetScrollPos(szPos, bMsg);
	}

	void CListUI::LineUp()
	{
		m_pList->LineUp();
	}

	void CListUI::LineDown()
	{
		m_pList->LineDown();
	}

	void CListUI::PageUp()
	{
		m_pList->PageUp();
	}

	void CListUI::PageDown()
	{
		m_pList->PageDown();
	}

	void CListUI::HomeUp()
	{
		m_pList->HomeUp();
	}

	void CListUI::EndDown()
	{
		m_pList->EndDown();
	}

	void CListUI::LineLeft()
	{
		m_pList->LineLeft();
	}

	void CListUI::LineRight()
	{
		m_pList->LineRight();
	}

	void CListUI::PageLeft()
	{
		m_pList->PageLeft();
	}

	void CListUI::PageRight()
	{
		m_pList->PageRight();
	}

	void CListUI::HomeLeft()
	{
		m_pList->HomeLeft();
	}

	void CListUI::EndRight()
	{
		m_pList->EndRight();
	}

	void CListUI::EnableScrollBar(bool bEnableVertical, bool bEnableHorizontal)
	{
		m_pList->EnableScrollBar(bEnableVertical, bEnableHorizontal);
	}

	CScrollBarUI* CListUI::GetVerticalScrollBar() const
	{
		return m_pList->GetVerticalScrollBar();
	}

	CScrollBarUI* CListUI::GetHorizontalScrollBar() const
	{
		return m_pList->GetHorizontalScrollBar();
	}

	BOOL CListUI::SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData)
	{
		if (!m_pList)
			return FALSE;
		return m_pList->SortItems(pfnCompare, dwData);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CListBodyUI::CListBodyUI(CListUI* pOwner) : m_pOwner(pOwner)
	{
		ASSERT(m_pOwner);
	}

	BOOL CListBodyUI::SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData)
	{
		if (!pfnCompare)
			return FALSE;
		m_pCompareFunc = pfnCompare;
		m_compareData = dwData;
		CControlUI **pData = (CControlUI **)m_items.c_str();
		qsort_s(m_items.c_str(), m_items.GetSize(), sizeof(CControlUI*), CListBodyUI::ItemComareFunc, this);
		IListItemUI *pItem = NULL;
		for (int i = 0; i < m_items.GetSize(); ++i)
		{
			pItem = (IListItemUI*)(static_cast<CControlUI*>(m_items[i])->GetInterface(TEXT("ListItem")));
			if (pItem)
			{
				pItem->SetIndex(i);
				pItem->Select(false);
			}
		}
		m_pOwner->SelectItem(-1);
		if (m_pManager)
		{
			SetPos(GetPos());
			Invalidate();
		}

		return TRUE;
	}


	void CListBodyUI::CopyData(CListBodyUI* pControl)
	{

		__super::CopyData(pControl);
	}

	int __cdecl CListBodyUI::ItemComareFunc(void *pvlocale, const void *item1, const void *item2)
	{
		CListBodyUI *pThis = (CListBodyUI*)pvlocale;
		if (!pThis || !item1 || !item2)
			return 0;
		return pThis->ItemComareFunc(item1, item2);
	}

	int __cdecl CListBodyUI::ItemComareFunc(const void *item1, const void *item2)
	{
		CControlUI *pControl1 = *(CControlUI**)item1;
		CControlUI *pControl2 = *(CControlUI**)item2;
		return m_pCompareFunc((UINT_PTR)pControl1, (UINT_PTR)pControl2, m_compareData);
	}

	int CListBodyUI::GetScrollStepSize() const
	{
		if (m_pOwner != NULL) return m_pOwner->GetScrollStepSize();

		return CVerticalLayoutUI::GetScrollStepSize();
	}

	void CListBodyUI::SetScrollPos(SIZE szPos, bool bMsg)
	{
		int cx = 0;
		int cy = 0;
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
			int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
			cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
		}

		RECT rcPos;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;

			rcPos = pControl->GetPos();
			rcPos.left -= cx;
			rcPos.right -= cx;
			rcPos.top -= cy;
			rcPos.bottom -= cy;
			pControl->SetPos(rcPos, true);
		}

		Invalidate();
		if (m_pOwner) {
			CListHeaderUI* pHeader = m_pOwner->GetHeader();
			if (pHeader == NULL) return;
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			pInfo->nColumns = MIN(pHeader->GetCount(), UILIST_MAX_COLUMNS);

			if (!pHeader->IsVisible()) {
				for (int it = 0; it < pHeader->GetCount(); it++) {
					static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(true);
				}
			}
			for (int i = 0; i < pInfo->nColumns; i++) {
				CControlUI* pControl = static_cast<CControlUI*>(pHeader->GetItemAt(i));
				if (!pControl->IsVisible()) continue;
				if (pControl->IsFloat()) continue;

				RECT rcPos = pControl->GetPos();
				rcPos.left -= cx;
				rcPos.right -= cx;
				pControl->SetPos(rcPos);
				pInfo->rcColumn[i] = pControl->GetPos();
			}
			if (!pHeader->IsVisible()) {
				for (int it = 0; it < pHeader->GetCount(); it++) {
					static_cast<CControlUI*>(pHeader->GetItemAt(it))->SetInternVisible(false);
				}
			}
		}
	}

	void CListBodyUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);

		// Adjust for inset
		int iChildPadding = GetChildPadding();
		RECT rcInset = GetInset();
		// Adjust for inset
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;

		if (m_pOwner->IsFixedScrollbar() && m_pVerticalScrollBar) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		else if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
			szAvailable.cx += m_pHorizontalScrollBar->GetScrollRange();

		int cxNeeded = 0;
		int nAdjustables = 0;
		int cyFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if (sz.cy == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
			}
			cyFixed += sz.cy + pControl->GetPadding().top + pControl->GetPadding().bottom;

			RECT rcPadding = pControl->GetPadding();
			sz.cx = MAX(sz.cx, 0);
			if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
			if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			cxNeeded = MAX(cxNeeded, sz.cx);
			nEstimateNum++;
		}
		cyFixed += (nEstimateNum - 1) * iChildPadding;

		if (m_pOwner) {
			CListHeaderUI* pHeader = m_pOwner->GetHeader();
			if (pHeader != NULL && pHeader->GetCount() > 0) {
				cxNeeded = MAX(0, pHeader->EstimateSize(CDuiSize(rc.right - rc.left, rc.bottom - rc.top)).cx);
				if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
				{
					int nOffset = m_pHorizontalScrollBar->GetScrollPos();
					RECT rcHeader = pHeader->GetPos();
					rcHeader.left = rc.left - nOffset;
					pHeader->SetPos(rcHeader);
				}
			}
		}

		// Place elements
		int cyNeeded = 0;
		int cyExpand = 0;
		if (nAdjustables > 0) cyExpand = MAX(0, (szAvailable.cy - cyFixed) / nAdjustables);
		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosY = rc.top;
		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
			iPosY -= m_pVerticalScrollBar->GetScrollPos();
		}
		int iPosX = rc.left;
		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		int iAdjustable = 0;
		int cyFixedRemaining = cyFixed;
		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);

			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it2);
				continue;
			}

			RECT rcPadding = pControl->GetPadding();
			szRemaining.cy -= rcPadding.top;
			SIZE sz = pControl->EstimateSize(szRemaining);
			if (sz.cy == 0) {
				iAdjustable++;
				sz.cy = cyExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cy = MAX(0, szRemaining.cy - rcPadding.bottom - cyFixedRemaining);
				}
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
			}
			else {
				if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
				if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();
				cyFixedRemaining -= sz.cy;
			}

			sz.cx = MAX(cxNeeded, szAvailable.cx - rcPadding.left - rcPadding.right);

			if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
			if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

			RECT rcCtrl = { iPosX + rcPadding.left, iPosY + rcPadding.top, iPosX + rcPadding.left + sz.cx, iPosY + sz.cy + rcPadding.top + rcPadding.bottom };
			pControl->SetPos(rcCtrl);

			iPosY += sz.cy + iChildPadding + rcPadding.top + rcPadding.bottom;
			cyNeeded += sz.cy + rcPadding.top + rcPadding.bottom;
			szRemaining.cy -= sz.cy + iChildPadding + rcPadding.bottom;
		}
		cyNeeded += (nEstimateNum - 1) * iChildPadding;

		if (m_pHorizontalScrollBar != NULL) {
			if (cxNeeded > rc.right - rc.left) {
				if (m_pHorizontalScrollBar->IsVisible()) {
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
				}
				else {
					m_pHorizontalScrollBar->SetVisible(true);
					m_pHorizontalScrollBar->SetScrollRange(cxNeeded - (rc.right - rc.left));
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
			else {
				if (m_pHorizontalScrollBar->IsVisible()) {
					m_pHorizontalScrollBar->SetVisible(false);
					m_pHorizontalScrollBar->SetScrollRange(0);
					m_pHorizontalScrollBar->SetScrollPos(0);
					rc.bottom += m_pHorizontalScrollBar->GetFixedHeight();
				}
			}
		}
		UINT uListType = m_pOwner->GetListType();
		if (uListType == LT_LIST) {
			// 鐠侊紕鐣诲Ο顏勬倻鐏忓搫顕?
			int nItemCount = m_items.GetSize();
			if (nItemCount > 0)
			{
				CControlUI* pControl = static_cast<CControlUI*>(m_items[0]);
				int nFixedWidth = pControl->GetFixedWidth();
				if (nFixedWidth > 0)
				{
					int nRank = (rc.right - rc.left) / nFixedWidth;
					if (nRank > 0)
					{
						cyNeeded = ((nItemCount - 1) / nRank + 1) * pControl->GetFixedHeight();
					}
				}
			}
		}
		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}

	void CListBodyUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CVerticalLayoutUI::DoEvent(event);
			return;
		}

		CVerticalLayoutUI::DoEvent(event);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListHeaderUI)

		CListHeaderUI::CListHeaderUI() :
		m_bIsScaleHeader(false)
	{
	}

	CListHeaderUI::~CListHeaderUI()
	{
	}

	std::wstring_view CListHeaderUI::GetClass() const
	{
		return _T("ListHeaderUI");
	}

	LPVOID CListHeaderUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LISTHEADER) == 0) return this;
		return CHorizontalLayoutUI::GetInterface(pstrName);
	}

	SIZE CListHeaderUI::EstimateSize(SIZE szAvailable)
	{
		SIZE cXY = { 0, GetFixedHeight() };
		if (cXY.cy == 0 && m_pManager != NULL) {
			for (int it = 0; it < m_items.GetSize(); it++) {
				cXY.cy = MAX(cXY.cy, static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cy);
			}
			int nMin = m_pManager->GetDefaultFontInfo()->tm.tmHeight + ScaleValue(6);
			cXY.cy = MAX(cXY.cy, nMin);
		}

		for (int it = 0; it < m_items.GetSize(); it++) {
			cXY.cx += static_cast<CControlUI*>(m_items[it])->EstimateSize(szAvailable).cx;
		}
		if (cXY.cx < szAvailable.cx) cXY.cx = szAvailable.cx;
		return cXY;
	}

	void CListHeaderUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		RECT rcInset = GetInset();
		// Adjust for inset
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;

		if (m_items.GetSize() == 0) {
			return;
		}

		int iChildPadding = GetChildPadding();
		// Determine the width of elements that are sizeable
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;
		for (int it1 = 0; it1 < m_items.GetSize(); it1++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			if (sz.cx == 0) {
				nAdjustables++;
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			cxFixed += sz.cx + pControl->GetPadding().left + pControl->GetPadding().right;
			nEstimateNum++;
		}
		cxFixed += (nEstimateNum - 1) * iChildPadding;

		int cxExpand = 0;
		int cxNeeded = 0;
		if (nAdjustables > 0) cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);

		int nHeaderWidth = rc.right - rc.left;
		CListUI *pList = static_cast<CListUI*>(GetParent());
		if (pList != NULL) {
			CScrollBarUI* pVScroll = pList->GetVerticalScrollBar();
			if (pVScroll != NULL) {
				nHeaderWidth -= pVScroll->GetWidth();
				szAvailable.cx = nHeaderWidth;
			}
		}

		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left;
		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;

		for (int it2 = 0; it2 < m_items.GetSize(); it2++) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
			if (!pControl->IsVisible()) continue;
			if (pControl->IsFloat()) {
				SetFloatPos(it2);
				continue;
			}
			RECT rcPadding = pControl->GetPadding();
			szRemaining.cx -= rcPadding.left;

			SIZE sz = { 0,0 };
			if (m_bIsScaleHeader) {
				CListHeaderItemUI* pHeaderItem = static_cast<CListHeaderItemUI*>(pControl);
				sz.cx = int(nHeaderWidth * (float)pHeaderItem->GetScale() / 100);
			}
			else {
				sz = pControl->EstimateSize(szRemaining);
			}

			if (sz.cx == 0) {
				iAdjustable++;
				sz.cx = cxExpand;
				// Distribute remaining to last element (usually round-off left-overs)
				if (iAdjustable == nAdjustables) {
					sz.cx = MAX(0, szRemaining.cx - rcPadding.right - cxFixedRemaining);
				}
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
			}
			else {
				if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
				if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();

				cxFixedRemaining -= sz.cx;
			}

			sz.cy = pControl->GetFixedHeight();
			if (sz.cy == 0) sz.cy = rc.bottom - rc.top - rcPadding.top - rcPadding.bottom;
			if (sz.cy < 0) sz.cy = 0;
			if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
			if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

			RECT rcCtrl = { iPosX + rcPadding.left, rc.top + rcPadding.top, iPosX + sz.cx + rcPadding.left + rcPadding.right, rc.top + rcPadding.top + sz.cy };
			pControl->SetPos(rcCtrl);
			iPosX += sz.cx + iChildPadding + rcPadding.left + rcPadding.right;
			cxNeeded += sz.cx + rcPadding.left + rcPadding.right;
			szRemaining.cx -= sz.cx + iChildPadding + rcPadding.right;
		}
		cxNeeded += (nEstimateNum - 1) * iChildPadding;
	}

	    void CListHeaderUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
    {
        if (StringUtil::EqualsNoCase(StringUtil::TrimView(pstrNameView), L"scaleheader")) SetScaleHeader(StringUtil::ParseBool(pstrValueView));
        else CHorizontalLayoutUI::SetAttribute(pstrNameView, pstrValueView);
    }

	void CListHeaderUI::SetScaleHeader(bool bIsScale)
	{
		m_bIsScaleHeader = bIsScale;
	}

	bool CListHeaderUI::IsScaleHeader() const
	{
		return m_bIsScaleHeader;
	}

	void CListHeaderUI::DoInit()
	{

	}
	void CListHeaderUI::DoPostPaint(CPaintRenderContext& /*renderContext*/) 
	{

	}

	CListHeaderUI* CListHeaderUI::Clone()
	{
		CListHeaderUI* pClone = new CListHeaderUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CListHeaderUI::CopyData(CListHeaderUI* pControl)
	{
		m_bIsScaleHeader= pControl->m_bIsScaleHeader;
		__super::CopyData(pControl);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListHeaderItemUI)

		CListHeaderItemUI::CListHeaderItemUI() : m_bDragable(true), m_uButtonState(0), m_iSepWidth(4),
		m_uTextStyle(DT_VCENTER | DT_CENTER | DT_SINGLELINE), m_dwTextColor(0), m_iFont(-1), m_bShowHtml(false), m_nScale(0)
	{
		SetTextPadding(CDuiRect(2, 0, 2, 0));
		ptLastMouse.x = ptLastMouse.y = 0;
		SetMinWidth(16);
	}

	std::wstring_view CListHeaderItemUI::GetClass() const
	{
		return _T("ListHeaderItemUI");
	}

	LPVOID CListHeaderItemUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LISTHEADERITEM) == 0) return this;
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CListHeaderItemUI::GetControlFlags() const
	{
		if (IsEnabled() && m_iSepWidth != 0) return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CListHeaderItemUI::SetEnabled(bool bEnable)
	{
		CContainerUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState = 0;
		}
	}

	bool CListHeaderItemUI::IsDragable() const
	{
		return m_bDragable;
	}

	void CListHeaderItemUI::SetDragable(bool bDragable)
	{
		m_bDragable = bDragable;
		if (!m_bDragable) m_uButtonState &= ~UISTATE_CAPTURED;
	}

	DWORD CListHeaderItemUI::GetSepWidth() const
	{
		if(m_pManager != NULL) return m_pManager->ScaleValue(m_iSepWidth);
		return m_iSepWidth;
	}

	void CListHeaderItemUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	DWORD CListHeaderItemUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	void CListHeaderItemUI::SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}

	DWORD CListHeaderItemUI::GetTextColor() const
	{
		return m_dwTextColor;
	}


	void CListHeaderItemUI::SetTextColor(DWORD dwTextColor)
	{
		m_dwTextColor = dwTextColor;
	}

	RECT CListHeaderItemUI::GetTextPadding() const
	{
		RECT rcTextPadding = m_rcTextPadding;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcTextPadding);
		return rcTextPadding;
	}

	void CListHeaderItemUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		Invalidate();
	}

	void CListHeaderItemUI::SetFont(int index)
	{
		m_iFont = index;
	}

	bool CListHeaderItemUI::IsShowHtml()
	{
		return m_bShowHtml;
	}

	void CListHeaderItemUI::SetShowHtml(bool bShowHtml)
	{
		if (m_bShowHtml == bShowHtml) return;

		m_bShowHtml = bShowHtml;
		Invalidate();
	}

	std::wstring_view CListHeaderItemUI::GetNormalImage() const
	{
		return m_sNormalImage;
	}

	void CListHeaderItemUI::SetNormalImage(std::wstring_view pStrImage)
	{
		m_sNormalImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CListHeaderItemUI::GetHotImage() const
	{
		return m_sHotImage;
	}

	void CListHeaderItemUI::SetHotImage(std::wstring_view pStrImage)
	{
		m_sHotImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CListHeaderItemUI::GetPushedImage() const
	{
		return m_sPushedImage;
	}

	void CListHeaderItemUI::SetPushedImage(std::wstring_view pStrImage)
	{
		m_sPushedImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CListHeaderItemUI::GetFocusedImage() const
	{
		return m_sFocusedImage;
	}

	void CListHeaderItemUI::SetFocusedImage(std::wstring_view pStrImage)
	{
		m_sFocusedImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CListHeaderItemUI::GetSepImage() const
	{
		return m_sSepImage;
	}

	void CListHeaderItemUI::SetSepImage(std::wstring_view pStrImage)
	{
		m_sSepImage = pStrImage;
		Invalidate();
	}

	void CListHeaderItemUI::SetScale(int nScale)
	{
		m_nScale = nScale;
	}

	int CListHeaderItemUI::GetScale() const
	{
		return m_nScale;
	}

	    void CListHeaderItemUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
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
        else if (StringUtil::EqualsNoCase(name, L"scale")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetScale(value);
        }
        else CHorizontalLayoutUI::SetAttribute(pstrNameView, pstrValueView);
    }

	void CListHeaderItemUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pParent != NULL) m_pParent->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			Invalidate();
		}
		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if (!IsEnabled()) return;
			RECT rcSeparator = GetThumbRect();
			const int separatorExpand = ScaleValue(4);
			if (m_iSepWidth >= 0)
				rcSeparator.left -= separatorExpand;
			else
				rcSeparator.right += separatorExpand;
			if (::PtInRect(&rcSeparator, event.ptMouse)) {
				if (m_bDragable) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
				}
			}
			else {
				m_uButtonState |= UISTATE_PUSHED;
				m_pManager->SendNotify(this, DUI_MSGTYPE_HEADERCLICK);
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				m_uButtonState &= ~UISTATE_CAPTURED;
				if (GetParent())
					GetParent()->NeedParentUpdate();
			}
			else if ((m_uButtonState & UISTATE_PUSHED) != 0) {
				m_uButtonState &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0) {
				RECT rcPadding = GetPadding();
				RECT rc = m_rcItem;
				if (m_iSepWidth >= 0) {
					rc.right -= ptLastMouse.x - event.ptMouse.x;
				}
				else {
					rc.left -= ptLastMouse.x - event.ptMouse.x;
				}

				if (rc.right - rc.left - rcPadding.right > GetMinWidth()) {
					SetFixedWidthFromPixels(rc.right - rc.left - rcPadding.right, false);
					ptLastMouse = event.ptMouse;
					if (GetParent())
						GetParent()->NeedParentUpdate();
				}
			}
			return;
		}
		if (event.Type == UIEVENT_SETCURSOR)
		{
			RECT rcSeparator = GetThumbRect();
			const int separatorExpand = ScaleValue(4);
			if (m_iSepWidth >= 0)
				rcSeparator.left -= separatorExpand;
			else
				rcSeparator.right += separatorExpand;
			if (IsEnabled() && m_bDragable && ::PtInRect(&rcSeparator, event.ptMouse)) {
				::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
				return;
			}
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				if ((m_uButtonState & UISTATE_HOT) == 0) {
					m_uButtonState |= UISTATE_HOT;
					Invalidate();
				}
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if (IsEnabled()) {
				if ((m_uButtonState & UISTATE_HOT) != 0) {
					m_uButtonState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
			return;
		}
		CHorizontalLayoutUI::DoEvent(event);
	}

	SIZE CListHeaderItemUI::EstimateSize(SIZE szAvailable)
	{
		const SIZE fixedSize = GetFixedSize();
		if (fixedSize.cy == 0) return CDuiSize(fixedSize.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + ScaleValue(14));
		return CHorizontalLayoutUI::EstimateSize(szAvailable);
	}

	RECT CListHeaderItemUI::GetThumbRect() const
	{
		const int sepWidth = ScaleValue(m_iSepWidth);
		if (m_iSepWidth >= 0) return CDuiRect(m_rcItem.right - sepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
		else return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - sepWidth, m_rcItem.bottom);
	}

	void CListHeaderItemUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		if (IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~UISTATE_FOCUSED;

		if ((m_uButtonState & UISTATE_PUSHED) != 0) {
			if (m_sPushedImage.empty() && !m_sNormalImage.empty()) DrawImage(renderContext, m_sNormalImage);
			if (!DrawImage(renderContext, m_sPushedImage)) {}
		}
		else if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (m_sHotImage.empty() && !m_sNormalImage.empty()) DrawImage(renderContext, m_sNormalImage);
			if (!DrawImage(renderContext, m_sHotImage)) {}
		}
		else if ((m_uButtonState & UISTATE_FOCUSED) != 0) {
			if (m_sFocusedImage.empty() && !m_sNormalImage.empty()) DrawImage(renderContext, m_sNormalImage);
			if (!DrawImage(renderContext, m_sFocusedImage)) {}
		}
		else {
			if (!m_sNormalImage.empty()) {
				if (!DrawImage(renderContext, m_sNormalImage)) {}
			}
		}

		if (!m_sSepImage.empty()) {
			RECT rcThumb = GetThumbRect();
			rcThumb.left -= m_rcItem.left;
			rcThumb.top -= m_rcItem.top;
			rcThumb.right -= m_rcItem.left;
			rcThumb.bottom -= m_rcItem.top;

			m_sSepImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
			if (!DrawImage(renderContext, m_sSepImage, m_sSepImageModify)) {}
		}
	}

	void CListHeaderItemUI::PaintText(CPaintRenderContext& renderContext)
	{
		if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();

		RECT rcText = m_rcItem;
		const RECT rcTextPadding = GetTextPadding();
		rcText.left += rcTextPadding.left;
		rcText.top += rcTextPadding.top;
		rcText.right -= rcTextPadding.right;
		rcText.bottom -= rcTextPadding.bottom;

		std::wstring sText = GetText();
		if (sText.empty()) return;
		int nLinks = 0;
		if (m_bShowHtml)
			CRenderEngine::DrawHtmlText(renderContext, rcText, sText, m_dwTextColor, NULL, NULL, nLinks, m_iFont, m_uTextStyle);
		else
		CRenderEngine::DrawText(renderContext, rcText, sText, m_dwTextColor, m_iFont, m_uTextStyle);
	}

	CListHeaderItemUI* CListHeaderItemUI::Clone()
	{
		CListHeaderItemUI* pClone = new CListHeaderItemUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CListHeaderItemUI::CopyData(CListHeaderItemUI* pControl)
	{
		SetDragable(pControl->IsDragable());
		SetSepWidth(pControl->m_iSepWidth);
		SetTextStyle(pControl->GetTextStyle());
		SetTextColor(pControl->GetTextColor());
		SetTextPadding(pControl->m_rcTextPadding);
		SetFont(pControl->m_iFont);
		SetShowHtml(pControl->IsShowHtml());
		SetNormalImage(pControl->GetNormalImage());
		SetHotImage(pControl->GetHotImage());
		SetPushedImage(pControl->GetPushedImage());
		SetFocusedImage(pControl->GetFocusedImage());
		SetSepImage(pControl->GetSepImage());
		SetScale(pControl->GetScale());

		m_uButtonState = pControl->m_uButtonState;
		ptLastMouse = pControl->ptLastMouse;
		m_sSepImageModify = pControl->m_sSepImageModify;

		__super::CopyData(pControl);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	CListElementUI::CListElementUI() : m_iIndex(-1),
		m_pOwner(NULL),
		m_bSelected(false),
		m_uButtonState(0)
	{
	}

	std::wstring_view CListElementUI::GetClass() const
	{
		return _T("ListElementUI");
	}

	UINT CListElementUI::GetControlFlags() const
	{
		return UIFLAG_WANTRETURN;
	}

	LPVOID CListElementUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LISTITEM) == 0) return static_cast<IListItemUI*>(this);
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LISTELEMENT) == 0) return static_cast<CListElementUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	IListOwnerUI* CListElementUI::GetOwner()
	{
		return m_pOwner;
	}

	void CListElementUI::SetOwner(CControlUI* pOwner)
	{
		m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface(_T("IListOwner")));
	}

	void CListElementUI::SetVisible(bool bVisible)
	{
		CControlUI::SetVisible(bVisible);
		if (!IsVisible() && m_bSelected)
		{
			m_bSelected = false;
			if (m_pOwner != NULL) m_pOwner->UnSelectItem(m_iIndex, false);
		}
	}

	void CListElementUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState = 0;
			if (m_bSelected && m_pOwner != NULL) {
				m_bSelected = false;
				m_pOwner->UnSelectItem(m_iIndex, false);
			}
		}
	}

	int CListElementUI::GetIndex() const
	{
		return m_iIndex;
	}

	void CListElementUI::SetIndex(int iIndex)
	{
		m_iIndex = iIndex;
	}

	void CListElementUI::Invalidate()
	{
		if (!IsVisible()) return;

		if (GetParent()) {
			CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
			if (pParentContainer) {
				RECT rc = pParentContainer->GetPos();
				RECT rcInset = pParentContainer->GetInset();
				rc.left += rcInset.left;
				rc.top += rcInset.top;
				rc.right -= rcInset.right;
				rc.bottom -= rcInset.bottom;
				CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
				if (pVerticalScrollBar && pVerticalScrollBar->IsVisible()) rc.right -= pVerticalScrollBar->GetFixedWidth();
				CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
				if (pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

				RECT invalidateRc = m_rcItem;
				if (!::IntersectRect(&invalidateRc, &m_rcItem, &rc))
				{
					return;
				}

				CControlUI* pParent = GetParent();
				RECT rcTemp;
				RECT rcParent;
				while (pParent = pParent->GetParent())
				{
					rcTemp = invalidateRc;
					rcParent = pParent->GetPos();
					if (!::IntersectRect(&invalidateRc, &rcTemp, &rcParent))
					{
						return;
					}
				}

				if (m_pManager != NULL) m_pManager->Invalidate(invalidateRc);
			}
			else {
				CControlUI::Invalidate();
			}
		}
		else {
			CControlUI::Invalidate();
		}
	}

	bool CListElementUI::Activate()
	{
		if (!CControlUI::Activate()) return false;
		if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE);
		return true;
	}

	bool CListElementUI::IsSelected() const
	{
		return m_bSelected;
	}

	bool CListElementUI::Select(bool bSelect, bool bIsClick)
	{
		if (!IsEnabled()) return false;
		// 閸欐牗绉烽崗璺虹暊閸掓銆冩い瑙勬殶閹?
		if (m_pOwner) {
			m_pOwner->UnSelectItem(m_iIndex, true);
		}
		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;
		if (m_pOwner) {
			if (bSelect)
			{
				m_pOwner->SelectItem(m_iIndex, false,bIsClick);
			}
			else m_pOwner->UnSelectItem(m_iIndex);
		}
		Invalidate();

		return true;
	}

	bool CListElementUI::SelectMulti(bool bSelect)
	{
		if (!IsEnabled()) return false;

		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;
		Invalidate();

		return true;
	}

	bool CListElementUI::IsExpanded() const
	{
		return false;
	}

	bool CListElementUI::Expand(bool /*bExpand = true*/)
	{
		return false;
	}

	void CListElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_DBLCLICK)
		{
			if (IsEnabled()) {
				Activate();
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN && IsEnabled())
		{
			if (event.chKey == VK_RETURN) {
				Activate();
				Invalidate();
				return;
			}
		}
		// An important twist: The list-item will send the event not to its immediate
		// parent but to the "attached" list. A list may actually embed several components
		// in its path to the item, but key-presses etc. needs to go to the actual list.
		if (m_pOwner != NULL) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	    void CListElementUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
    {
        if (StringUtil::EqualsNoCase(StringUtil::TrimView(pstrNameView), L"selected")) Select();
        else CControlUI::SetAttribute(pstrNameView, pstrValueView);
    }

	void CListElementUI::DrawItemBk(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		ASSERT(m_pOwner);
		if (m_pOwner == NULL) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iBackColor = 0;
		if (!pInfo->bAlternateBk || m_iIndex % 2 == 0) iBackColor = pInfo->dwBkColor;
		if ((m_uButtonState & UISTATE_HOT) != 0 && pInfo->dwHotBkColor > 0) {
			iBackColor = pInfo->dwHotBkColor;
		}
		if (IsSelected() && pInfo->dwSelectedBkColor > 0) {
			iBackColor = pInfo->dwSelectedBkColor;
		}
		if (!IsEnabled() && pInfo->dwDisabledBkColor > 0) {
			iBackColor = pInfo->dwDisabledBkColor;
		}

		if (iBackColor != 0) {
			CRenderEngine::DrawColor(renderContext, m_rcItem, GetAdjustColor(iBackColor));
		}

		if (!IsEnabled()) {
			if (!pInfo->sDisabledImage.empty()) {
				if (!DrawImage(renderContext, pInfo->sDisabledImage)) {}
				else return;
			}
		}
		if (IsSelected()) {
			if (!pInfo->sSelectedImage.empty()) {
				if (!DrawImage(renderContext, pInfo->sSelectedImage)) {}
				else return;
			}
		}
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!pInfo->sHotImage.empty()) {
				if (!DrawImage(renderContext, pInfo->sHotImage)) {}
				else return;
			}
		}

		if (!m_sBkImage.empty()) {
			if (!pInfo->bAlternateBk || m_iIndex % 2 == 0) {
				if (!DrawImage(renderContext, m_sBkImage)) {}
			}
		}

		if (m_sBkImage.empty()) {
			if (!pInfo->sBkImage.empty()) {
				if (!DrawImage(renderContext, pInfo->sBkImage)) {}
				else return;
			}
		}

		if (pInfo->dwLineColor != 0) {
			if (pInfo->bShowRowLine) {
				RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
				CRenderEngine::DrawLine(renderContext, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
			}
			if (pInfo->bShowColumnLine) {
				for (int i = 0; i < pInfo->nColumns; i++) {
					RECT rcLine = { pInfo->rcColumn[i].right - 1, m_rcItem.top, pInfo->rcColumn[i].right - 1, m_rcItem.bottom };
					CRenderEngine::DrawLine(renderContext, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
				}
			}
		}
	}

	void CListElementUI::DrawItemText(CPaintRenderContext& /*renderContext*/, const RECT& /*rcItem*/)
	{
	}



	CListElementUI* CListElementUI::Clone()
	{
		CListElementUI* pClone = new CListElementUI();
		pClone->CopyData(this);
		return pClone;
	}


	void CListElementUI::CopyData(CListElementUI* pControl)
	{
		m_iIndex = pControl->m_iIndex;
		m_bSelected = pControl->m_bSelected;
		m_uButtonState = pControl->m_uButtonState;
		__super::CopyData(pControl);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListLabelElementUI)

		CListLabelElementUI::CListLabelElementUI()
	{
	}

	std::wstring_view CListLabelElementUI::GetClass() const
	{
		return _T("ListLabelElementUI");
	}

	LPVOID CListLabelElementUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LISTLABELELEMENT) == 0) return static_cast<CListLabelElementUI*>(this);
		return CListElementUI::GetInterface(pstrName);
	}

	void CListLabelElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CListElementUI::DoEvent(event);
			return;
		}

		// 閸欐娊鏁柅澶嬪
		if (m_pOwner != NULL)
		{
			if (m_pOwner->GetListInfo()->bRSelected && event.Type == UIEVENT_RBUTTONDOWN)
			{
				if (IsEnabled()) {
					// 婢舵岸鈧?
					if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_SHIFT) & 0x8000)) {
						if (m_pOwner != NULL) m_pOwner->SelectMultiItem(m_iIndex);
					}
					else {
						Select(true);
					}
				}
				return;
			}
		}

		if (event.Type == UIEVENT_BUTTONDOWN)
		{
			if (IsEnabled()) {
				if(IsRichEvent()) 
				{
					m_pManager->SendNotify(this, DUI_MSGTYPE_BUTTONDOWN);
				}

				// 婢舵岸鈧?
				if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_SHIFT) & 0x8000)) {
					if (m_pOwner != NULL) m_pOwner->SelectMultiItem(m_iIndex);
				}
				else 
				{
					if (IsRichEvent())
					{
						Select(true,true);
					}
					else
					{
						Select(true);
					}

				}
			}
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			if (IsEnabled()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				if ((m_uButtonState & UISTATE_HOT) == 0) {
					m_uButtonState |= UISTATE_HOT;
					Invalidate();
					if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER);
				}
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if ((m_uButtonState & UISTATE_HOT) != 0) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE);
			}
			return;
		}
		CListElementUI::DoEvent(event);
	}


	SIZE CListLabelElementUI::EstimateSize(SIZE szAvailable)
	{
		if (m_pOwner == NULL) return CDuiSize(0, 0);
		std::wstring sText = GetText();

		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		SIZE cXY = GetFixedSize();
		if (cXY.cy == 0 && m_pManager != NULL && pInfo != NULL) {
			const RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
			cXY.cy = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight + ScaleValue(8);
			cXY.cy += rcTextPadding.top + rcTextPadding.bottom;
		}

		if (cXY.cx == 0) {
			cXY.cx = szAvailable.cx;
		}

		return cXY;
	}

	bool CListLabelElementUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		DrawItemBk(renderContext, m_rcItem);
		DrawItemText(renderContext, m_rcItem);
		return true;
	}

	void CListLabelElementUI::DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		std::wstring sText = GetText();
		if (sText.empty()) return;

		if (m_pOwner == NULL) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iTextColor = pInfo->dwTextColor;
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			iTextColor = pInfo->dwHotTextColor;
		}
		if (IsSelected()) {
			iTextColor = pInfo->dwSelectedTextColor;
		}
		if (!IsEnabled()) {
			iTextColor = pInfo->dwDisabledTextColor;
		}
		int nLinks = 0;
		RECT rcText = rcItem;
		RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
		rcText.left += rcTextPadding.left;
		rcText.right -= rcTextPadding.right;
		rcText.top += rcTextPadding.top;
		rcText.bottom -= rcTextPadding.bottom;

		if (pInfo->bShowHtml)
			CRenderEngine::DrawHtmlText(renderContext, rcText, sText, iTextColor, NULL, NULL, nLinks, pInfo->nFont, pInfo->uTextStyle);
		else
		CRenderEngine::DrawText(renderContext, rcText, sText, iTextColor, pInfo->nFont, pInfo->uTextStyle);
	}

	CListLabelElementUI* CListLabelElementUI::Clone()
	{
		CListLabelElementUI* pClone = new CListLabelElementUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CListLabelElementUI::CopyData(CListLabelElementUI* pControl)
	{
		__super::CopyData(pControl);
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListTextElementUI)

		CListTextElementUI::CListTextElementUI() : m_nLinks(0), m_nHoverLink(-1), m_pOwner(NULL)
	{
		::ZeroMemory(&m_rcLinks, sizeof(m_rcLinks));
	}

	CListTextElementUI::~CListTextElementUI()
	{
		std::wstring* pText;
		for (int it = 0; it < m_aTexts.GetSize(); it++) {
			pText = static_cast<std::wstring*>(m_aTexts[it]);
			if (pText) delete pText;
		}
		m_aTexts.Empty();
	}

	std::wstring_view CListTextElementUI::GetClass() const
	{
		return _T("ListTextElementUI");
	}

	LPVOID CListTextElementUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LISTTEXTELEMENT) == 0) return static_cast<CListTextElementUI*>(this);
		return CListLabelElementUI::GetInterface(pstrName);
	}

	UINT CListTextElementUI::GetControlFlags() const
	{
		return UIFLAG_WANTRETURN | ((IsEnabled() && m_nLinks > 0) ? UIFLAG_SETCURSOR : 0);
	}

	std::wstring_view CListTextElementUI::GetText(int iIndex) const
	{
		std::wstring* pText = static_cast<std::wstring*>(m_aTexts.GetAt(iIndex));
		if (pText) {
			if (!IsResourceText())
				return *pText;
			m_sTextResourceCache = CResourceManager::GetInstance()->GetText(*pText);
			return m_sTextResourceCache;
		}
		return {};
	}

	void CListTextElementUI::SetText(int iIndex, std::wstring_view pstrText)
	{
		if (m_pOwner == NULL) return;

		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if (iIndex < 0 || iIndex >= pInfo->nColumns) return;
		while (m_aTexts.GetSize() < pInfo->nColumns) { m_aTexts.Add(NULL); }

		std::wstring* pText = static_cast<std::wstring*>(m_aTexts[iIndex]);
		if (pText && *pText == pstrText) return;
		if (pText == NULL && pstrText.empty()) return;

		if (pText) { delete pText; pText = NULL; }
		m_aTexts.SetAt(iIndex, new std::wstring(pstrText));

		Invalidate();
	}

	DWORD CListTextElementUI::GetTextColor(int iIndex) const
	{
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if( iIndex < 0 || iIndex >= pInfo->nColumns || m_aTextColors.GetSize() <= 0 ) return pInfo->dwTextColor;

		DWORD dwColor = PtrToListColor(m_aTextColors.GetAt(iIndex));
		return dwColor;
	}

	void CListTextElementUI::SetTextColor(int iIndex, DWORD dwTextColor)
	{
		if( m_pOwner == NULL ) return;

		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		if( iIndex < 0 || iIndex >= pInfo->nColumns ) return;
		while( m_aTextColors.GetSize() < pInfo->nColumns ) { m_aTextColors.Add(ListColorToPtr(pInfo->dwTextColor)); }
		m_aTextColors.SetAt(iIndex, ListColorToPtr(dwTextColor));

		Invalidate();
	}

	void CListTextElementUI::SetOwner(CControlUI* pOwner)
	{
		CListElementUI::SetOwner(pOwner);
		m_pOwner = static_cast<IListUI*>(pOwner->GetInterface(_T("IList")));
	}

	std::wstring* CListTextElementUI::GetLinkContent(int iIndex)
	{
		if (iIndex >= 0 && iIndex < m_nLinks) return &m_sLinks[iIndex];
		return NULL;
	}

	void CListTextElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CListLabelElementUI::DoEvent(event);
			return;
		}

		// When you hover over a link
		if (event.Type == UIEVENT_SETCURSOR) {
			for (int i = 0; i < m_nLinks; i++) {
				if (::PtInRect(&m_rcLinks[i], event.ptMouse)) {
					::SetCursor(::LoadCursor(NULL, IDC_HAND));
					return;
				}
			}
		}
		if (event.Type == UIEVENT_BUTTONUP && IsEnabled()) {
			for (int i = 0; i < m_nLinks; i++) {
				if (::PtInRect(&m_rcLinks[i], event.ptMouse)) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_LINK, i);
					return;
				}
			}
		}
		if (m_nLinks > 0 && event.Type == UIEVENT_MOUSEMOVE) {
			int nHoverLink = -1;
			for (int i = 0; i < m_nLinks; i++) {
				if (::PtInRect(&m_rcLinks[i], event.ptMouse)) {
					nHoverLink = i;
					break;
				}
			}

			if (m_nHoverLink != nHoverLink) {
				Invalidate();
				m_nHoverLink = nHoverLink;
			}
		}
		if (m_nLinks > 0 && event.Type == UIEVENT_MOUSELEAVE) {
			if (m_nHoverLink != -1) {
				Invalidate();
				m_nHoverLink = -1;
			}
		}
		CListLabelElementUI::DoEvent(event);
	}

	SIZE CListTextElementUI::EstimateSize(SIZE szAvailable)
	{
		TListInfoUI* pInfo = NULL;
		if (m_pOwner) pInfo = m_pOwner->GetListInfo();

		SIZE cXY = GetFixedSize();
		if (cXY.cy == 0 && m_pManager != NULL) {
			cXY.cy = 0;
			if (pInfo != NULL) {
				const RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
				cXY.cy = m_pManager->GetFontInfo(pInfo->nFont)->tm.tmHeight + ScaleValue(8);
				cXY.cy += rcTextPadding.top + rcTextPadding.bottom;
			}
		}

		return cXY;
	}

	void CListTextElementUI::DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		if( m_pOwner == NULL ) return;

		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		IListCallbackUI* pCallback = m_pOwner->GetTextCallback();
		m_nLinks = 0;
		int nLinks = lengthof(m_rcLinks);
		const std::wstring rowText = CControlUI::GetText();
		const std::vector<std::wstring_view> rowTextParts = StringUtil::SplitView(rowText, L'|', false);
		const RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
		for( int i = 0; i < pInfo->nColumns; i++ )
		{
			RECT rcItem = { pInfo->rcColumn[i].left, m_rcItem.top, pInfo->rcColumn[i].right, m_rcItem.bottom };
			rcItem.left += rcTextPadding.left;
			rcItem.right -= rcTextPadding.right;
			rcItem.top += rcTextPadding.top;
			rcItem.bottom -= rcTextPadding.bottom;

			DWORD iTextColor = pInfo->dwTextColor;
			std::wstring strText;
			if( pCallback ) {
				strText = pCallback->GetItemText(this, m_iIndex, i);
				int iState = 0;
				if( (m_uButtonState & UISTATE_HOT) != 0 ) {
					iState = 1;
				}
				if( IsSelected() ) {
					iState = 2;
				}
				if( !IsEnabled() ) {
					iState = 3;
				}
				iTextColor = pCallback->GetItemTextColor(this, m_iIndex, i, iState);
			}
			else {
				if( i < m_aTexts.GetSize() && m_aTexts.GetAt(i) != NULL ) {
					strText.assign(GetText(i));
				}
				else if( i < static_cast<int>(rowTextParts.size()) ) {
					strText.assign(rowTextParts[i]);
				}

				iTextColor = GetTextColor(i);
				if( (m_uButtonState & UISTATE_HOT) != 0 ) {
					iTextColor = pInfo->dwHotTextColor;
				}
				if( IsSelected() ) {
					iTextColor = pInfo->dwSelectedTextColor;
				}
				if( !IsEnabled() ) {
					iTextColor = pInfo->dwDisabledTextColor;
				}

			}
			if (pInfo->bShowHtml)
				CRenderEngine::DrawHtmlText(renderContext, rcItem, strText, iTextColor,
					&m_rcLinks[m_nLinks], &m_sLinks[m_nLinks], nLinks, pInfo->nFont, pInfo->uTextStyle);
			else
		CRenderEngine::DrawText(renderContext, rcItem, strText, iTextColor,
			pInfo->nFont, pInfo->uTextStyle);

			m_nLinks += nLinks;
			nLinks = lengthof(m_rcLinks) - m_nLinks; 
		}
		for( int i = m_nLinks; i < lengthof(m_rcLinks); i++ ) {
			::ZeroMemory(m_rcLinks + i, sizeof(RECT));
			m_sLinks[i].clear();
		}
	}

	CListTextElementUI* CListTextElementUI::Clone()
	{
		CListTextElementUI* pClone = new CListTextElementUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CListTextElementUI::CopyData(CListTextElementUI* pControl)
	{
		m_nLinks = pControl->m_nLinks;
		m_nHoverLink =pControl->m_nHoverLink;

		for (int it = 0; it < pControl->m_aTexts.GetSize(); it++)
		{
			std::wstring* pText = static_cast<std::wstring*>(pControl->m_aTexts[it]);
			m_aTexts.Add(pText);
		}

		for (int it = 0; it < pControl->m_aTextColors.GetSize(); it++)
		{
			std::wstring* pText = static_cast<std::wstring*>(pControl->m_aTextColors[it]);
			m_aTextColors.Add(pText);
		}
		__super::CopyData(pControl);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CListContainerElementUI)

		CListContainerElementUI::CListContainerElementUI() :
		m_iIndex(-1),
		m_pOwner(NULL),
		m_bSelected(false),
		m_uButtonState(0)
	{
	}

	std::wstring_view CListContainerElementUI::GetClass() const
	{
		return _T("ListContainerElementUI");
	}

	UINT CListContainerElementUI::GetControlFlags() const
	{
		if (IsEnabled() && !GetText().empty()) {
			return UIFLAG_SETCURSOR;
		}
		else return UIFLAG_WANTRETURN;
	}

	LPVOID CListContainerElementUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LISTITEM) == 0) return static_cast<IListItemUI*>(this);
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_LISTCONTAINERELEMENT) == 0) return static_cast<CListContainerElementUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	IListOwnerUI* CListContainerElementUI::GetOwner()
	{
		return m_pOwner;
	}

	void CListContainerElementUI::SetOwner(CControlUI* pOwner)
	{
		m_pOwner = static_cast<IListOwnerUI*>(pOwner->GetInterface(_T("IListOwner")));
	}

	void CListContainerElementUI::SetVisible(bool bVisible)
	{
		CContainerUI::SetVisible(bVisible);
		if (!IsVisible() && m_bSelected)
		{
			m_bSelected = false;
			if (m_pOwner != NULL) m_pOwner->UnSelectItem(m_iIndex, false);
		}
	}

	void CListContainerElementUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState = 0;
			if (m_bSelected && m_pOwner != NULL) {
				m_bSelected = false;
				m_pOwner->UnSelectItem(m_iIndex, false);
			}
		}
	}

	int CListContainerElementUI::GetIndex() const
	{
		return m_iIndex;
	}

	void CListContainerElementUI::SetIndex(int iIndex)
	{
		m_iIndex = iIndex;
	}

	void CListContainerElementUI::Invalidate()
	{
		if (!IsVisible()) return;

		if (GetParent()) {
			CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
			if (pParentContainer) {
				RECT rc = pParentContainer->GetPos();
				RECT rcInset = pParentContainer->GetInset();
				rc.left += rcInset.left;
				rc.top += rcInset.top;
				rc.right -= rcInset.right;
				rc.bottom -= rcInset.bottom;
				CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
				if (pVerticalScrollBar && pVerticalScrollBar->IsVisible()) rc.right -= pVerticalScrollBar->GetFixedWidth();
				CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
				if (pHorizontalScrollBar && pHorizontalScrollBar->IsVisible()) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

				RECT invalidateRc = m_rcItem;
				if (!::IntersectRect(&invalidateRc, &m_rcItem, &rc))
				{
					return;
				}

				CControlUI* pParent = GetParent();
				RECT rcTemp;
				RECT rcParent;
				while (pParent = pParent->GetParent())
				{
					rcTemp = invalidateRc;
					rcParent = pParent->GetPos();
					if (!::IntersectRect(&invalidateRc, &rcTemp, &rcParent))
					{
						return;
					}
				}

				if (m_pManager != NULL) m_pManager->Invalidate(invalidateRc);
			}
			else {
				CContainerUI::Invalidate();
			}
		}
		else {
			CContainerUI::Invalidate();
		}
	}

	bool CListContainerElementUI::Activate()
	{
		if (!CContainerUI::Activate()) return false;
		if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMACTIVATE);
		return true;
	}

	bool CListContainerElementUI::IsSelected() const
	{
		return m_bSelected;
	}

	bool CListContainerElementUI::Select(bool bSelect, bool bIsClick )
	{
		if (!IsEnabled()) return false;
		// 閸欐牗绉烽崗璺虹暊閸掓銆冩い瑙勬殶閹?
		if (m_pOwner) {
			m_pOwner->UnSelectItem(m_iIndex, true);
		}
		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;
		if (m_pOwner) {
			if (bSelect) 
			{
				m_pOwner->SelectItem(m_iIndex, false, bIsClick);
			}
			else m_pOwner->UnSelectItem(m_iIndex);
		}
		Invalidate();

		return true;
	}

	bool CListContainerElementUI::SelectMulti(bool bSelect)
	{
		if (!IsEnabled()) return false;

		if (bSelect == m_bSelected) return true;
		m_bSelected = bSelect;

		Invalidate();
		return true;
	}

	bool CListContainerElementUI::IsExpanded() const
	{
		return false;
	}

	bool CListContainerElementUI::Expand(bool /*bExpand = true*/)
	{
		return false;
	}

	void CListContainerElementUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETCURSOR) {
			::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(GetCursor())));
			return;
		}
		if (event.Type == UIEVENT_DBLCLICK)
		{
			if (IsEnabled()) {
				Activate();
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN && IsEnabled())
		{
			if (event.chKey == VK_RETURN) {
				Activate();
				Invalidate();
				return;
			}
		}
		if (event.Type == UIEVENT_BUTTONDOWN)
		{
			if (IsEnabled()) {
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_BUTTONDOWN);

				// 婢舵岸鈧?
				if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_SHIFT) & 0x8000)) {
					if (m_pOwner != NULL) m_pOwner->SelectMultiItem(m_iIndex);
				}
				else {
					Select(true);
				}
			}
			return;
		}
		// 閸欐娊鏁柅澶嬪
		if (m_pOwner != NULL)
		{
			if (m_pOwner->GetListInfo()->bRSelected && event.Type == UIEVENT_RBUTTONDOWN)
			{
				if (IsEnabled()) {
					// 婢舵岸鈧?
					if ((GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_SHIFT) & 0x8000)) {
						if (m_pOwner != NULL) m_pOwner->SelectMultiItem(m_iIndex);
					}
					else {
						Select(true);
					}
				}
				return;
			}
		}

		if (event.Type == UIEVENT_BUTTONUP)
		{
			if (IsEnabled()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMCLICK);
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				if ((m_uButtonState & UISTATE_HOT) == 0) {
					m_uButtonState |= UISTATE_HOT;
					Invalidate();
					if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER);
				}
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if ((m_uButtonState & UISTATE_HOT) != 0) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
				if(IsRichEvent()) m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE);
			}
			return;
		}
		if (event.Type == UIEVENT_TIMER)
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_TIMER, event.wParam, event.lParam);
			return;
		}

		if (event.Type == UIEVENT_CONTEXTMENU)
		{
			if (IsContextMenuUsed()) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
				return;
			}
		}
		// An important twist: The list-item will send the event not to its immediate
		// parent but to the "attached" list. A list may actually embed several components
		// in its path to the item, but key-presses etc. needs to go to the actual list.
		if (m_pOwner != NULL) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}


	    void CListContainerElementUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
    {
        if (StringUtil::EqualsNoCase(StringUtil::TrimView(pstrNameView), L"selected")) Select();
        else CContainerUI::SetAttribute(pstrNameView, pstrValueView);
    }

	bool CListContainerElementUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		bool bResult = false;
		const RECT& rcPaint = renderContext.GetPaintRect();
		if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) {
			return bResult;
		}

		RECT rcTemp = { 0 };
		if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) {
			return bResult;
		}

		CRenderClip clip;
		CRenderClip::GenerateClip(renderContext, rcTemp, clip);
		bResult = CControlUI::DoPaint(renderContext, pStopControl);

		DrawItemBk(renderContext, m_rcItem);
		DrawItemText(renderContext, m_rcItem);

		if (m_items.GetSize() > 0) {
			RECT rc = m_rcItem;
			rc.left += m_rcInset.left;
			rc.top += m_rcInset.top;
			rc.right -= m_rcInset.right;
			rc.bottom -= m_rcInset.bottom;
			if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
				rc.right -= m_pVerticalScrollBar->GetFixedWidth();
			}
			if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
				rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
			}

			if (!::IntersectRect(&rcTemp, &rcPaint, &rc)) {
				for (int it = 0; it < m_items.GetSize(); it++) {
					CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
					if (!pControl->IsVisible()) {
						continue;
					}
					if (!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos())) {
						continue;
					}
					if (pControl->IsFloat()) {
						if (!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos())) {
							continue;
						}
						bResult = pControl->DoPaint(renderContext, pStopControl);
					}
				}
			}
			else {
				CRenderClip childClip;
				CRenderClip::GenerateClip(renderContext, rcTemp, childClip);
				for (int it = 0; it < m_items.GetSize(); it++) {
					CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
					if (!pControl->IsVisible()) {
						continue;
					}
					if (!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos())) {
						continue;
					}
					if (pControl->IsFloat()) {
						if (!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos())) {
							continue;
						}
						CRenderClip::UseOldClipBegin(renderContext, childClip);
						bResult = pControl->DoPaint(renderContext, pStopControl);
						CRenderClip::UseOldClipEnd(renderContext, childClip);
					}
					else {
						if (!::IntersectRect(&rcTemp, &rc, &pControl->GetPos())) {
							continue;
						}
						bResult = pControl->DoPaint(renderContext, pStopControl);
					}
				}
			}
		}

		if (m_pVerticalScrollBar != NULL &&
			m_pVerticalScrollBar->IsVisible()) {
			if (::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos())) {
				bResult = m_pVerticalScrollBar->DoPaint(renderContext, pStopControl);
			}
		}

		if (m_pHorizontalScrollBar != NULL &&
			m_pHorizontalScrollBar->IsVisible()) {
			if (::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos())) {
				bResult = m_pHorizontalScrollBar->DoPaint(renderContext, pStopControl);
			}
		}

		return bResult;
	}

	void CListContainerElementUI::DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		std::wstring sText = GetText();
		if (sText.empty()) return;

		if (m_pOwner == NULL) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iTextColor = pInfo->dwTextColor;
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			iTextColor = pInfo->dwHotTextColor;
		}
		if (IsSelected()) {
			iTextColor = pInfo->dwSelectedTextColor;
		}
		if (!IsEnabled()) {
			iTextColor = pInfo->dwDisabledTextColor;
		}
		int nLinks = 0;
		RECT rcText = rcItem;
		RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
		rcText.left += rcTextPadding.left;
		rcText.right -= rcTextPadding.right;
		rcText.top += rcTextPadding.top;
		rcText.bottom -= rcTextPadding.bottom;

		if (pInfo->bShowHtml)
			CRenderEngine::DrawHtmlText(renderContext, rcText, sText, iTextColor, NULL, NULL, nLinks, pInfo->nFont, pInfo->uTextStyle);
		else
		CRenderEngine::DrawText(renderContext, rcText, sText, iTextColor, pInfo->nFont, pInfo->uTextStyle);
	}

	void CListContainerElementUI::DrawItemBk(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		ASSERT(m_pOwner);
		if (m_pOwner == NULL) return;
		TListInfoUI* pInfo = m_pOwner->GetListInfo();
		DWORD iBackColor = 0;
		if (!pInfo->bAlternateBk || m_iIndex % 2 == 0) iBackColor = pInfo->dwBkColor;

		std::wstring sForeImage = pInfo->sForeImage;
		if ((m_uButtonState & UISTATE_HOT) != 0 && pInfo->dwHotBkColor > 0) {
			iBackColor = pInfo->dwHotBkColor;
			if(!pInfo->sHotForeImage.empty()) {
				sForeImage = pInfo->sHotForeImage;
			}
		}
		if (IsSelected() && pInfo->dwSelectedBkColor > 0) {
			iBackColor = pInfo->dwSelectedBkColor;

		}
		if (!IsEnabled() && pInfo->dwDisabledBkColor > 0) {
			iBackColor = pInfo->dwDisabledBkColor;
		}
		if (iBackColor != 0) {
			CRenderEngine::DrawColor(renderContext, m_rcItem, GetAdjustColor(iBackColor));
		}

		if (!IsEnabled()) {
			if (!pInfo->sDisabledImage.empty()) {
				if (!DrawImage(renderContext, pInfo->sDisabledImage)) {}
				else return;
			}
		}
		if (IsSelected()) {
			if (!pInfo->sSelectedImage.empty()) {
				bool bDrawOk = DrawImage(renderContext, pInfo->sSelectedImage);
				if(!pInfo->sSelectedForeImage.empty()) {
					DrawImage(renderContext, pInfo->sSelectedForeImage);
				}
				if(bDrawOk) return;
			}
		}
		if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (!pInfo->sHotImage.empty()) {
				bool bDrawOk = DrawImage(renderContext, pInfo->sHotImage);
				if(!pInfo->sHotForeImage.empty()) {
					DrawImage(renderContext, pInfo->sHotForeImage);
				}
				if(bDrawOk) return;
			}
		}

		if (!m_sBkImage.empty()) {
			if (!pInfo->bAlternateBk || m_iIndex % 2 == 0) {
				if (!DrawImage(renderContext, m_sBkImage)) {}

				if(!pInfo->sForeImage.empty()) {
					DrawImage(renderContext, pInfo->sForeImage);
				}
			}
		}

		if (m_sBkImage.empty()) {
			if (!pInfo->sBkImage.empty()) {
				bool bDrawOk = DrawImage(renderContext, pInfo->sBkImage);
				if(!pInfo->sForeImage.empty()) {
					DrawImage(renderContext, pInfo->sForeImage);
				}
			}
		}

		if (pInfo->dwLineColor != 0) {
			if (pInfo->bShowRowLine) {
				RECT rcLine = { m_rcItem.left, m_rcItem.bottom - 1, m_rcItem.right, m_rcItem.bottom - 1 };
				CRenderEngine::DrawLine(renderContext, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
			}
			if (pInfo->bShowColumnLine) {
				for (int i = 0; i < pInfo->nColumns; i++) {
					RECT rcLine = { pInfo->rcColumn[i].right - 1, m_rcItem.top, pInfo->rcColumn[i].right - 1, m_rcItem.bottom };
					CRenderEngine::DrawLine(renderContext, rcLine, 1, GetAdjustColor(pInfo->dwLineColor));
				}
			}
		}
	}

	void CListContainerElementUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		if (m_pOwner == NULL) return;

		UINT uListType = m_pOwner->GetListType();
		if (uListType == LT_LIST) {
			int nFixedWidth = GetFixedWidth();
			if (nFixedWidth > 0)
			{
				int nRank = (rc.right - rc.left) / nFixedWidth;
				if (nRank > 0)
				{
					int nIndex = GetIndex();
					int nfloor = nIndex / nRank;
					int nHeight = rc.bottom - rc.top;

					rc.top = rc.top - nHeight * (nIndex - nfloor);
					rc.left = rc.left + nFixedWidth * (nIndex % nRank);
					rc.right = rc.left + nFixedWidth;
					rc.bottom = nHeight + rc.top;
				}
			}
		}
		CHorizontalLayoutUI::SetPos(rc, bNeedInvalidate);

		if (uListType != LT_LIST && uListType != LT_TREE) return;
		CListUI* pList = static_cast<CListUI*>(m_pOwner);
		if (uListType == LT_TREE)
		{
			pList = (CListUI*)pList->CControlUI::GetInterface(_T("List"));
			if (pList == NULL) return;
		}

		CListHeaderUI *pHeader = pList->GetHeader();
		if (pHeader == NULL || !pHeader->IsVisible()) return;
		int nCount = m_items.GetSize();
		for (int i = 0; i < nCount; i++)
		{
			CControlUI *pListItem = static_cast<CControlUI*>(m_items[i]);
			CControlUI *pHeaderItem = pHeader->GetItemAt(i);
			if (pHeaderItem == NULL) return;
			RECT rcHeaderItem = pHeaderItem->GetPos();
			if (pListItem != NULL && !(rcHeaderItem.left == 0 && rcHeaderItem.right == 0))
			{
				RECT rt = pListItem->GetPos();
				rt.left = rcHeaderItem.left;
				rt.right = rcHeaderItem.right;
				pListItem->SetPos(rt);
			}
		}
	}

	CListContainerElementUI* CListContainerElementUI::Clone()
	{
		CListContainerElementUI* pClone = new CListContainerElementUI();
		pClone->CopyData(this);
		return pClone;
	}
	void CListContainerElementUI::CopyData(CListContainerElementUI* pControl)
	{
		m_iIndex = pControl->m_iIndex;
		m_bSelected = pControl->m_bSelected;
		m_uButtonState = pControl->m_uButtonState;
		__super::CopyData(pControl);
	}

} 



