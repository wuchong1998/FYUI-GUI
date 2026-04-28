#include "pch.h"
#include "UITabLayout.h"
#include "UILayoutContentUtil.h"

namespace FYUI
{
	namespace
	{
		SIZE ClampTabItemSize(CControlUI* pControl, SIZE szAvailable)
		{
			SIZE sz = pControl->EstimateSize(szAvailable);
			if( sz.cx == 0 ) sz.cx = MAX(0, szAvailable.cx);
			if( sz.cy == 0 ) sz.cy = MAX(0, szAvailable.cy);
			if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
			if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
			if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
			if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
			return sz;
		}
	}

	IMPLEMENT_DUICONTROL(CTabLayoutUI)
		CTabLayoutUI::CTabLayoutUI() : m_iCurSel(-1)
	{
	}

	std::wstring_view CTabLayoutUI::GetClass() const
	{
		return _T("TabLayoutUI");
	}

	LPVOID CTabLayoutUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, DUI_CTR_TABLAYOUT) == 0 ) return static_cast<CTabLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	int CTabLayoutUI::ResolveTabSelectionAfterAdd(CControlUI* pControl, int iIndex) const
	{
		if( pControl != NULL && m_iCurSel == -1 && pControl->IsVisible() ) {
			return GetItemIndex(pControl);
		}
		if( m_iCurSel != -1 && iIndex <= m_iCurSel ) {
			return m_iCurSel + 1;
		}
		return m_iCurSel;
	}

	int CTabLayoutUI::ResolveTabSelectionAfterRemoval(int iRemovedIndex) const
	{
		if( GetCount() <= 0 ) return -1;
		if( m_iCurSel < 0 ) return -1;
		if( m_iCurSel < iRemovedIndex ) return m_iCurSel;
		return min(iRemovedIndex, GetCount() - 1);
	}

	bool CTabLayoutUI::Add(CControlUI* pControl)
	{
		bool ret = CContainerUI::Add(pControl);
		if( !ret ) return ret;

		m_iCurSel = ResolveTabSelectionAfterAdd(pControl, GetItemIndex(pControl));
		UpdateSelectionVisibility(m_iCurSel, false);

		return ret;
	}

	bool CTabLayoutUI::AddAt(CControlUI* pControl, int iIndex)
	{
		bool ret = CContainerUI::AddAt(pControl, iIndex);
		if( !ret ) return ret;

		m_iCurSel = ResolveTabSelectionAfterAdd(pControl, iIndex);
		UpdateSelectionVisibility(m_iCurSel, false);

		return ret;
	}

		bool CTabLayoutUI::Remove(CControlUI* pControl, bool bChildDelayed)
		{
			if( pControl == NULL) return false;

			int index = GetItemIndex(pControl);
			bool ret = CContainerUI::Remove(pControl, bChildDelayed);
			if( !ret ) return false;

			if( m_iCurSel == index ) {
				m_iCurSel = ResolveTabSelectionAfterRemoval(index);
			UpdateSelectionVisibility(m_iCurSel, false);
			NeedParentUpdate();
		}
		else if( m_iCurSel > index ) {
			m_iCurSel -= 1;
		}

			return ret;
		}

		bool CTabLayoutUI::RemoveAt(int iIndex, bool bChildDelayed)
		{
			if( iIndex < 0 || iIndex >= GetCount() ) return false;

			bool ret = CContainerUI::RemoveAt(iIndex, bChildDelayed);
			if( !ret ) return false;

			if( m_iCurSel == iIndex ) {
				m_iCurSel = ResolveTabSelectionAfterRemoval(iIndex);
				UpdateSelectionVisibility(m_iCurSel, false);
				NeedParentUpdate();
			}
			else if( m_iCurSel > iIndex ) {
				m_iCurSel -= 1;
			}

			return true;
		}

		void CTabLayoutUI::RemoveAll(bool bChildDelayed)
		{
			m_iCurSel = -1;
			CContainerUI::RemoveAll(bChildDelayed);
			NeedParentUpdate();
		}

	int CTabLayoutUI::GetCurSel() const
	{
		return m_iCurSel;
	}

	bool CTabLayoutUI::SelectItem(int iIndex,bool bIsFocus)
	{
		if( iIndex < 0 || iIndex >= m_items.GetSize() ) return false;
		if( iIndex == m_iCurSel ) return true;

		int iOldSel = m_iCurSel;
		m_iCurSel = iIndex;
		UpdateSelectionVisibility(m_iCurSel, bIsFocus);
		SetPos(m_rcItem);
		NeedParentUpdate();

		if( m_pManager != NULL && bIsFocus) {
			m_pManager->SetNextTabControl();
			m_pManager->SendNotify(this, DUI_MSGTYPE_TABSELECT, m_iCurSel, iOldSel);
		}
		return true;
	}

	bool CTabLayoutUI::SelectItem( CControlUI* pControl ,bool bIsFocus)
	{

		int iIndex=GetItemIndex(pControl);
		if (iIndex==-1)
			return false;
		else
			return SelectItem(iIndex,bIsFocus);
	}

	void CTabLayoutUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		const std::wstring valueText(pstrValue);
		if( StringUtil::CompareNoCase(pstrName, _T("selectedid")) == 0 ) SelectItem(_ttoi(valueText.c_str()));
		return CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	CTabLayoutUI* CTabLayoutUI::Clone()
	{
		CTabLayoutUI* pClone = new CTabLayoutUI();
		pClone->CopyData(this);

		return pClone;
	}

	void CTabLayoutUI::CopyData(CTabLayoutUI* pControl)
	{
		std::wstring strName = pControl->GetName();
		m_iCurSel = pControl->m_iCurSel;
		__super::CopyData(pControl);
	}

	CControlUI* CTabLayoutUI::GetSelectedItemControl() const
	{
		if( m_iCurSel < 0 || m_iCurSel >= m_items.GetSize() ) return NULL;
		return static_cast<CControlUI*>(m_items[m_iCurSel]);
	}

	void CTabLayoutUI::UpdateSelectionVisibility(int iSelected, bool bSetFocus)
	{
		for( int it = 0; it < m_items.GetSize(); ++it )
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			const bool bSelected = (it == iSelected);
			pControl->SetVisible(bSelected);
			if( bSelected && bSetFocus ) {
				pControl->SetFocus();
			}
		}
	}


	void CTabLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		/*std::wstring strName = GetName();
		if (strName == L"pdf_attribute_content_float")
		{
		int A = 0;
		}*/
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = ApplyLayoutInsetRect(m_rcItem, GetInset());

		for( int it = 0; it < m_items.GetSize(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( pControl->IsFloat() ) {
				SetFloatPos(it);
			}
		}

		CControlUI* pSelectedControl = GetSelectedItemControl();
		if( pSelectedControl == NULL ) return;
		if( !pSelectedControl->IsVisible() ) pSelectedControl->SetVisible(true);

		RECT rcCtrl = rc;
		RECT rcPadding = pSelectedControl->GetPadding();
		rcCtrl.left += rcPadding.left;
		rcCtrl.top += rcPadding.top;
		rcCtrl.right -= rcPadding.right;
		rcCtrl.bottom -= rcPadding.bottom;

		SIZE szAvailable = { rcCtrl.right - rcCtrl.left, rcCtrl.bottom - rcCtrl.top };
		SIZE sz = ClampTabItemSize(pSelectedControl, szAvailable);
		pSelectedControl->SetPos({ rcCtrl.left, rcCtrl.top, rcCtrl.left + sz.cx, rcCtrl.top + sz.cy });
	}
}

