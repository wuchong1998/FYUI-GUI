#include "pch.h"
#include "UIChildLayout.h"


namespace FYUI
{
	IMPLEMENT_DUICONTROL(CChildLayoutUI)

	CChildLayoutUI::CChildLayoutUI()
	{

	}

	bool CChildLayoutUI::HasChildLayoutSource() const
	{
		return !m_pstrXMLFile.empty();
	}

	bool CChildLayoutUI::HasLoadedChildLayout() const
	{
		return GetCount() > 0;
	}

	CContainerUI* CChildLayoutUI::CreateChildLayoutContainer() const
	{
		if( !HasChildLayoutSource() ) return NULL;

		CDialogBuilder builder;
	return static_cast<CContainerUI*>(builder.Create(STRINGorID(m_pstrXMLFile.c_str()), {}, NULL, m_pManager));
	}

	void CChildLayoutUI::ClearChildLayoutContainer()
	{
		if( HasLoadedChildLayout() ) {
			RemoveAll();
		}
	}

	void CChildLayoutUI::ApplyChildLayoutContainer(CContainerUI* pChildWindow)
	{
		ClearChildLayoutContainer();
		if( pChildWindow ) {
			Add(pChildWindow);
		}
	}

		bool CChildLayoutUI::ReloadChildLayout()
		{
			const bool bHadLoadedChild = HasLoadedChildLayout();
			if( !HasChildLayoutSource() ) {
				ClearChildLayoutContainer();
				if( bHadLoadedChild ) {
					NeedParentUpdate();
				}
				return bHadLoadedChild;
			}
			CContainerUI* pChildWindow = CreateChildLayoutContainer();
			if( pChildWindow == NULL ) return false;
			ApplyChildLayoutContainer(pChildWindow);
			if( m_rcItem.right > m_rcItem.left && m_rcItem.bottom > m_rcItem.top ) {
				SetPos(m_rcItem, false);
			}
			NeedParentUpdate();
			return true;
		}

	void CChildLayoutUI::Init()
	{
		if( !HasChildLayoutSource() ) return;
		ReloadChildLayout();
	}

	void CChildLayoutUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("xmlfile")) == 0 )
			SetChildLayoutXML(pstrValue);
		else
			CContainerUI::SetAttribute(pstrName,pstrValue);
	}

		void CChildLayoutUI::SetChildLayoutXML(std::wstring_view pXML)
		{
			if( m_pstrXMLFile == pXML ) return;
			m_pstrXMLFile = pXML;
			if( m_pManager != NULL || HasLoadedChildLayout() ) {
				ReloadChildLayout();
			}
		}

	std::wstring CChildLayoutUI::GetChildLayoutXML()
	{
		return m_pstrXMLFile;
	}

	LPVOID CChildLayoutUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_CHILDLAYOUT) == 0) return static_cast<CChildLayoutUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	std::wstring_view CChildLayoutUI::GetClass() const
	{
		return _T("ChildLayoutUI");
	}

	CChildLayoutUI* CChildLayoutUI::Clone()
	{
		CChildLayoutUI* pClone = new CChildLayoutUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CChildLayoutUI::CopyData(CChildLayoutUI* pControl)
	{
		m_pstrXMLFile = pControl->m_pstrXMLFile;
		__super::CopyData(pControl);
	}
} // namespace DuiLib


