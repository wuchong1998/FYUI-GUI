#include "pch.h"
#include "UITreeView.h"

#pragma warning( disable: 4251 )
namespace FYUI
{
	IMPLEMENT_DUICONTROL(CTreeNodeUI)

		//************************************
		// 鍑芥暟鍚嶇О: CTreeNodeUI
		// 杩斿洖绫诲瀷: 
		// 鍙傛暟淇℃伅: CTreeNodeUI * _ParentNode
		// 鍑芥暟璇存槑: 
		//************************************
		CTreeNodeUI::CTreeNodeUI( CTreeNodeUI* _ParentNode /*= NULL*/ )
	{
		m_dwItemTextColor = 0x00000000;
		m_dwItemHotTextColor = 0;
		m_dwSelItemTextColor = 0;
		m_dwSelItemHotTextColor	= 0;

		pTreeView = NULL;
		m_iTreeLavel = 0;
		m_bIsVisable = TRUE;
		m_bIsCheckBox = FALSE;
		pParentTreeNode	= NULL;

		pHoriz = new CHorizontalLayoutUI();
		pFolderButton = new CCheckBoxUI();
		pDottedLine = new CLabelUI();
		pCheckBox = new CCheckBoxUI();
		pItemButton = new COptionUI();

		this->SetFixedHeight(18);
		this->SetFixedWidth(250);
		pFolderButton->SetFixedWidth(GetFixedHeight());
		pDottedLine->SetFixedWidth(2);
		pCheckBox->SetFixedWidth(GetFixedHeight());
		pItemButton->SetAttribute(_T("align"),_T("left"));
		pDottedLine->SetVisible(FALSE);
		pCheckBox->SetVisible(FALSE);
		pItemButton->SetMouseEnabled(FALSE);

		if(_ParentNode) {
			if (StringUtil::CompareNoCase(_ParentNode->GetClass(), _T("TreeNodeUI")) != 0) return;
			pDottedLine->SetVisible(_ParentNode->IsVisible());
			pDottedLine->SetFixedWidth(_ParentNode->GetDottedLine()->GetFixedWidth()+16);
			this->SetParentNode(_ParentNode);
		}
		pHoriz->SetChildVAlign(DT_VCENTER);
		pHoriz->Add(pDottedLine);
		pHoriz->Add(pFolderButton);
		pHoriz->Add(pCheckBox);
		pHoriz->Add(pItemButton);
		Add(pHoriz);
	}

	//************************************
	// 鍑芥暟鍚嶇О: ~CTreeNodeUI
	// 杩斿洖绫诲瀷: 
	// 鍙傛暟淇℃伅: void
	// 鍑芥暟璇存槑: 
	//************************************
	CTreeNodeUI::~CTreeNodeUI( void )
	{

	}

	//************************************
	// 鍑芥暟鍚嶇О: GetClass
	// 杩斿洖绫诲瀷: std::wstring_view
	// 鍑芥暟璇存槑: 
	//************************************
	std::wstring_view CTreeNodeUI::GetClass() const
	{
		return _T("TreeNodeUI");
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetInterface
	// 杩斿洖绫诲瀷: LPVOID
	// 鍙傛暟淇℃伅: std::wstring_view pstrName
	// 鍑芥暟璇存槑: 
	//************************************
	LPVOID CTreeNodeUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("TreeNode")) == 0)
			return static_cast<CTreeNodeUI*>(this);
		return CListContainerElementUI::GetInterface(pstrName);
	}

	//************************************
	// 鍑芥暟鍚嶇О: DoEvent
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: TEventUI & event
	// 鍑芥暟璇存槑:
	//************************************
	void CTreeNodeUI::DoEvent( TEventUI& event )
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pOwner != NULL ) m_pOwner->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}
		CListContainerElementUI::DoEvent(event);
		if( event.Type == UIEVENT_DBLCLICK ) {
			if( IsEnabled() ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_TREEITEMDBCLICK);
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER ) {
			if( IsEnabled()) {
				if(m_bSelected && GetSelItemHotTextColor())
					pItemButton->SetTextColor(GetSelItemHotTextColor());
				else
					pItemButton->SetTextColor(GetItemHotTextColor());
			}
			else 
				pItemButton->SetTextColor(pItemButton->GetDisabledTextColor());

			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE ) {
			if( IsEnabled()) {
				if(m_bSelected && GetSelItemTextColor())
					pItemButton->SetTextColor(GetSelItemTextColor());
				else if(!m_bSelected)
					pItemButton->SetTextColor(GetItemTextColor());
			}
			else 
				pItemButton->SetTextColor(pItemButton->GetDisabledTextColor());

			return;
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: Invalidate
	// 杩斿洖绫诲瀷: void
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::Invalidate()
	{
		if( !IsVisible() )
			return;

		if( GetParent() ) {
			CContainerUI* pParentContainer = static_cast<CContainerUI*>(GetParent()->GetInterface(_T("Container")));
			if( pParentContainer ) {
				RECT rc = pParentContainer->GetPos();
				RECT rcInset = pParentContainer->GetInset();
				rc.left += rcInset.left;
				rc.top += rcInset.top;
				rc.right -= rcInset.right;
				rc.bottom -= rcInset.bottom;
				CScrollBarUI* pVerticalScrollBar = pParentContainer->GetVerticalScrollBar();
				if( pVerticalScrollBar && pVerticalScrollBar->IsVisible() ) rc.right -= pVerticalScrollBar->GetFixedWidth();
				CScrollBarUI* pHorizontalScrollBar = pParentContainer->GetHorizontalScrollBar();
				if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rc.bottom -= pHorizontalScrollBar->GetFixedHeight();

				RECT invalidateRc = m_rcItem;
				if( !::IntersectRect(&invalidateRc, &m_rcItem, &rc) ) 
					return;

				CControlUI* pParent = GetParent();
				RECT rcTemp;
				RECT rcParent;
				while( pParent = pParent->GetParent() ) {
					rcTemp = invalidateRc;
					rcParent = pParent->GetPos();
					if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
						return;
				}

				if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
			}
			else {
				CContainerUI::Invalidate();
			}
		}
		else {
			CContainerUI::Invalidate();
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: Select
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: bool bSelect
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::Select( bool bSelect /*= true*/ )
	{
		bool nRet = CListContainerElementUI::Select(bSelect);
		if(m_bSelected)
			pItemButton->SetTextColor(GetSelItemTextColor());
		else 
			pItemButton->SetTextColor(GetItemTextColor());

		return nRet;
	}

	bool CTreeNodeUI::SelectMulti(bool bSelect)
	{
		bool nRet = CListContainerElementUI::SelectMulti(bSelect);
		if(m_bSelected)
			pItemButton->SetTextColor(GetSelItemTextColor());
		else 
			pItemButton->SetTextColor(GetItemTextColor());

		return nRet;
	}
	//************************************
	// 鍑芥暟鍚嶇О: Add
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: CControlUI * _pTreeNodeUI
	// 鍑芥暟璇存槑: 閫氳繃鑺傜偣瀵硅薄娣诲姞鑺傜偣
	//************************************
	bool CTreeNodeUI::Add( CControlUI* _pTreeNodeUI )
	{
		if (NULL != static_cast<CTreeNodeUI*>(_pTreeNodeUI->GetInterface(_T("TreeNode"))))
			return AddChildNode((CTreeNodeUI*)_pTreeNodeUI);

		return CListContainerElementUI::Add(_pTreeNodeUI);
	}

	//************************************
	// 鍑芥暟鍚嶇О: AddAt
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: CControlUI * pControl
	// 鍙傛暟淇℃伅: int iIndex				璇ュ弬鏁颁粎閽堝褰撳墠鑺傜偣涓嬬殑鍏勫紵绱㈠紩锛屽苟闈炲垪琛ㄨ鍥剧储寮?
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::AddAt( CControlUI* pControl, int iIndex )
	{
		if(NULL == static_cast<CTreeNodeUI*>(pControl->GetInterface(_T("TreeNode"))))
			return FALSE;

		CTreeNodeUI* pIndexNode = static_cast<CTreeNodeUI*>(mTreeNodes.GetAt(iIndex));
		if(!pIndexNode){
			if(!mTreeNodes.Add(pControl))
				return FALSE;
		}
		else if(pIndexNode && !mTreeNodes.InsertAt(iIndex,pControl))
			return FALSE;

		if(!pIndexNode && pTreeView && pTreeView->GetItemAt(GetTreeIndex()+1))
			pIndexNode = static_cast<CTreeNodeUI*>(pTreeView->GetItemAt(GetTreeIndex()+1)->GetInterface(_T("TreeNode")));

		pControl = CalLocation((CTreeNodeUI*)pControl);

		if(pTreeView && pIndexNode)
			return pTreeView->AddAt((CTreeNodeUI*)pControl,pIndexNode);
		else 
			return pTreeView->Add((CTreeNodeUI*)pControl);

		return TRUE;
	}

	//************************************
	// 鍑芥暟鍚嶇О: Remove
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: CControlUI * pControl
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::Remove( CControlUI* pControl )
	{
		return RemoveAt((CTreeNodeUI*)pControl);
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetVisibleTag
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: bool _IsVisible
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetVisibleTag( bool _IsVisible )
	{
		m_bIsVisable = _IsVisible;
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetVisibleTag
	// 杩斿洖绫诲瀷: bool
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::GetVisibleTag()
	{
		return m_bIsVisable;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetItemText
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: std::wstring_view pstrValue
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetItemText(std::wstring_view pstrValue)
	{
		pItemButton->SetText(pstrValue);
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetItemText
	// 杩斿洖绫诲瀷: DuiLib::std::wstring
	// 鍑芥暟璇存槑: 
	//************************************
	std::wstring CTreeNodeUI::GetItemText()
	{
		return pItemButton->GetText();
	}

	//************************************
	// 鍑芥暟鍚嶇О: CheckBoxSelected
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: bool _Selected
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::CheckBoxSelected( bool _Selected )
	{
		pCheckBox->Selected(_Selected);
	}

	//************************************
	// 鍑芥暟鍚嶇О: IsCheckBoxSelected
	// 杩斿洖绫诲瀷: bool
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::IsCheckBoxSelected() const
	{
		return pCheckBox->IsSelected();
	}

	//************************************
	// 鍑芥暟鍚嶇О: IsHasChild
	// 杩斿洖绫诲瀷: bool
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::IsHasChild() const
	{
		return !mTreeNodes.empty();
	}


	long CTreeNodeUI::GetTreeLevel()
	{
		long level = 0;
		CTreeNodeUI* pParentNode = GetParentNode();
		while(pParentNode != NULL) {
			level++;
			pParentNode = pParentNode->GetParentNode();
		}
		return level;
	}
	//************************************
	// 鍑芥暟鍚嶇О: AddChildNode
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: CTreeNodeUI * _pTreeNodeUI
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::AddChildNode( CTreeNodeUI* _pTreeNodeUI )
	{
		if (!_pTreeNodeUI)
			return FALSE;

		if (NULL == static_cast<CTreeNodeUI*>(_pTreeNodeUI->GetInterface(_T("TreeNode"))))
			return FALSE;

		_pTreeNodeUI = CalLocation(_pTreeNodeUI);

		bool nRet = TRUE;

		if(pTreeView){
			CTreeNodeUI* pNode = static_cast<CTreeNodeUI*>(mTreeNodes.GetAt(mTreeNodes.GetSize()-1));
			if(!pNode || !pNode->GetLastNode())
				nRet = pTreeView->AddAt(_pTreeNodeUI,GetTreeIndex()+1) >= 0;
			else nRet = pTreeView->AddAt(_pTreeNodeUI,pNode->GetLastNode()->GetTreeIndex()+1) >= 0;
		}

		if(nRet)
			mTreeNodes.Add(_pTreeNodeUI);

		return nRet;
	}

	//************************************
	// 鍑芥暟鍚嶇О: RemoveAt
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: CTreeNodeUI * _pTreeNodeUI
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::RemoveAt( CTreeNodeUI* _pTreeNodeUI )
	{
		int nIndex = mTreeNodes.Find(_pTreeNodeUI);
		CTreeNodeUI* pNode = static_cast<CTreeNodeUI*>(mTreeNodes.GetAt(nIndex));
		if(pNode && pNode == _pTreeNodeUI)
		{
			while(pNode->IsHasChild())
				pNode->RemoveAt(static_cast<CTreeNodeUI*>(pNode->mTreeNodes.GetAt(0)));

			mTreeNodes.Remove(nIndex);

			if(pTreeView)
				pTreeView->Remove(_pTreeNodeUI);

			return TRUE;
		}
		return FALSE;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetParentNode
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: CTreeNodeUI * _pParentTreeNode
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetParentNode( CTreeNodeUI* _pParentTreeNode )
	{
		pParentTreeNode = _pParentTreeNode;
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetParentNode
	// 杩斿洖绫诲瀷: CTreeNodeUI*
	// 鍑芥暟璇存槑: 
	//************************************
	CTreeNodeUI* CTreeNodeUI::GetParentNode()
	{
		return pParentTreeNode;
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetCountChild
	// 杩斿洖绫诲瀷: long
	// 鍑芥暟璇存槑: 
	//************************************
	long CTreeNodeUI::GetCountChild()
	{
		return mTreeNodes.GetSize();
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetTreeView
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: CTreeViewUI * _CTreeViewUI
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetTreeView( CTreeViewUI* _CTreeViewUI )
	{
		pTreeView = _CTreeViewUI;
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetTreeView
	// 杩斿洖绫诲瀷: CTreeViewUI*
	// 鍑芥暟璇存槑: 
	//************************************
	CTreeViewUI* CTreeNodeUI::GetTreeView()
	{
		return pTreeView;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetAttribute
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: std::wstring_view pstrName
	// 鍙傛暟淇℃伅: std::wstring_view pstrValue
	// 鍑芥暟璇存槑: 
	//************************************
	    void CTreeNodeUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
    {
        const std::wstring_view name = StringUtil::TrimView(pstrNameView);
        if (name.empty()) {
            return;
        }

        if (StringUtil::EqualsNoCase(name, L"text")) pItemButton->SetText(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"horizattr")) pHoriz->ApplyAttributeList(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"dotlineattr")) pDottedLine->ApplyAttributeList(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"folderattr")) pFolderButton->ApplyAttributeList(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"checkboxattr")) pCheckBox->ApplyAttributeList(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"itemattr")) pItemButton->ApplyAttributeList(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"itemtextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetItemTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemhottextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetItemHotTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"selitemtextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetSelItemTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"selitemhottextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetSelItemHotTextColor(color);
        }
        else CListContainerElementUI::SetAttribute(pstrNameView, pstrValueView);
    }

	//************************************
	// 鍑芥暟鍚嶇О: GetTreeNodes
	// 杩斿洖绫诲瀷: DuiLib::CStdPtrArray
	// 鍑芥暟璇存槑: 
	//************************************
	CStdPtrArray CTreeNodeUI::GetTreeNodes()
	{
		return mTreeNodes;
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetChildNode
	// 杩斿洖绫诲瀷: CTreeNodeUI*
	// 鍙傛暟淇℃伅: int _nIndex
	// 鍑芥暟璇存槑: 
	//************************************
	CTreeNodeUI* CTreeNodeUI::GetChildNode( int _nIndex )
	{
		return static_cast<CTreeNodeUI*>(mTreeNodes.GetAt(_nIndex));
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetVisibleFolderBtn
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: bool _IsVisibled
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetVisibleFolderBtn( bool _IsVisibled )
	{
		pFolderButton->SetVisible(_IsVisibled);
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetVisibleFolderBtn
	// 杩斿洖绫诲瀷: bool
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::GetVisibleFolderBtn()
	{
		return pFolderButton->IsVisible();
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetVisibleCheckBtn
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: bool _IsVisibled
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetVisibleCheckBtn( bool _IsVisibled )
	{
		pCheckBox->SetVisible(_IsVisibled);
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetVisibleCheckBtn
	// 杩斿洖绫诲瀷: bool
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeNodeUI::GetVisibleCheckBtn()
	{
		return pCheckBox->IsVisible();
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetNodeIndex
	// 杩斿洖绫诲瀷: int
	// 鍑芥暟璇存槑: 鍙栧緱鍏ㄥ眬鏍戣鍥剧殑绱㈠紩
	//************************************
	int CTreeNodeUI::GetTreeIndex()
	{
		if(!pTreeView)
			return -1;

		for(int nIndex = 0;nIndex < pTreeView->GetCount();nIndex++){
			if(this == pTreeView->GetItemAt(nIndex))
				return nIndex;
		}

		return -1;
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetNodeIndex
	// 杩斿洖绫诲瀷: int
	// 鍑芥暟璇存槑: 鍙栧緱鐩稿浜庡厔寮熻妭鐐圭殑褰撳墠绱㈠紩
	//************************************
	int CTreeNodeUI::GetNodeIndex()
	{
		if(!GetParentNode() && !pTreeView)
			return -1;

		if(!GetParentNode() && pTreeView)
			return GetTreeIndex();

		return GetParentNode()->GetTreeNodes().Find(this);
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetLastNode
	// 杩斿洖绫诲瀷: CTreeNodeUI*
	// 鍑芥暟璇存槑:
	//************************************
	CTreeNodeUI* CTreeNodeUI::GetLastNode( )
	{
		if(!IsHasChild()) return this;

		CTreeNodeUI* nRetNode = NULL;
		for(int nIndex = 0;nIndex < GetTreeNodes().GetSize();nIndex++){
			CTreeNodeUI* pNode = static_cast<CTreeNodeUI*>(GetTreeNodes().GetAt(nIndex));
			if(!pNode) continue;
			if(pNode->IsHasChild())
				nRetNode = pNode->GetLastNode();
			else 
				nRetNode = pNode;
		}

		return nRetNode;
	}

	//************************************
	// 鍑芥暟鍚嶇О: CalLocation
	// 杩斿洖绫诲瀷: CTreeNodeUI*
	// 鍙傛暟淇℃伅: CTreeNodeUI * _pTreeNodeUI
	// 鍑芥暟璇存槑: 缂╄繘璁＄畻
	//************************************
	CTreeNodeUI* CTreeNodeUI::CalLocation( CTreeNodeUI* _pTreeNodeUI )
	{
		_pTreeNodeUI->GetDottedLine()->SetVisible(TRUE);
		_pTreeNodeUI->GetDottedLine()->SetFixedWidth(pDottedLine->GetFixedWidth()+16);
		_pTreeNodeUI->SetParentNode(this);
		_pTreeNodeUI->GetItemButton()->SetGroup(pItemButton->GetGroup());
		_pTreeNodeUI->SetTreeView(pTreeView);

		return _pTreeNodeUI;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetTextColor
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: DWORD _dwTextColor
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetItemTextColor( DWORD _dwItemTextColor )
	{
		m_dwItemTextColor	= _dwItemTextColor;
		pItemButton->SetTextColor(m_dwItemTextColor);
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetTextColor
	// 杩斿洖绫诲瀷: DWORD
	// 鍑芥暟璇存槑: 
	//************************************
	DWORD CTreeNodeUI::GetItemTextColor() const
	{
		return m_dwItemTextColor;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetTextHotColor
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: DWORD _dwTextHotColor
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetItemHotTextColor( DWORD _dwItemHotTextColor )
	{
		m_dwItemHotTextColor = _dwItemHotTextColor;
		Invalidate();
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetTextHotColor
	// 杩斿洖绫诲瀷: DWORD
	// 鍑芥暟璇存槑: 
	//************************************
	DWORD CTreeNodeUI::GetItemHotTextColor() const
	{
		return m_dwItemHotTextColor;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetSelItemTextColor
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: DWORD _dwSelItemTextColor
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetSelItemTextColor( DWORD _dwSelItemTextColor )
	{
		m_dwSelItemTextColor = _dwSelItemTextColor;
		Invalidate();
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetSelItemTextColor
	// 杩斿洖绫诲瀷: DWORD
	// 鍑芥暟璇存槑: 
	//************************************
	DWORD CTreeNodeUI::GetSelItemTextColor() const
	{
		return m_dwSelItemTextColor;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetSelHotItemTextColor
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: DWORD _dwSelHotItemTextColor
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeNodeUI::SetSelItemHotTextColor( DWORD _dwSelHotItemTextColor )
	{
		m_dwSelItemHotTextColor = _dwSelHotItemTextColor;
		Invalidate();
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetSelHotItemTextColor
	// 杩斿洖绫诲瀷: DWORD
	// 鍑芥暟璇存槑: 
	//************************************
	DWORD CTreeNodeUI::GetSelItemHotTextColor() const
	{
		return m_dwSelItemHotTextColor;
	}

	/*****************************************************************************/
	/*****************************************************************************/
	/*****************************************************************************/
	IMPLEMENT_DUICONTROL(CTreeViewUI)

		//************************************
		// 鍑芥暟鍚嶇О: CTreeViewUI
		// 杩斿洖绫诲瀷: 
		// 鍙傛暟淇℃伅: void
		// 鍑芥暟璇存槑: 
		//************************************
		CTreeViewUI::CTreeViewUI( void ) : m_bVisibleFolderBtn(TRUE),m_bVisibleCheckBtn(FALSE),m_uItemMinWidth(0)
	{
		this->GetHeader()->SetVisible(FALSE);
	}

	//************************************
	// 鍑芥暟鍚嶇О: ~CTreeViewUI
	// 杩斿洖绫诲瀷: 
	// 鍙傛暟淇℃伅: void
	// 鍑芥暟璇存槑: 
	//************************************
	CTreeViewUI::~CTreeViewUI( void )
	{

	}

	//************************************
	// 鍑芥暟鍚嶇О: GetClass
	// 杩斿洖绫诲瀷: std::wstring_view
	// 鍑芥暟璇存槑: 
	//************************************
	std::wstring_view CTreeViewUI::GetClass() const
	{
		return _T("TreeViewUI");
	}


	UINT CTreeViewUI::GetListType()
	{
		return LT_TREE;
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetInterface
	// 杩斿洖绫诲瀷: LPVOID
	// 鍙傛暟淇℃伅: std::wstring_view pstrName
	// 鍑芥暟璇存槑: 
	//************************************
	LPVOID CTreeViewUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("TreeView")) == 0) return static_cast<CTreeViewUI*>(this);
		return CListUI::GetInterface(pstrName);
	}

	//************************************
	// 鍑芥暟鍚嶇О: Add
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: CTreeNodeUI * pControl
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeViewUI::Add( CTreeNodeUI* pControl )
	{
		if (!pControl) return false;
		if (NULL == static_cast<CTreeNodeUI*>(pControl->GetInterface(_T("TreeNode")))) return false;

		pControl->OnNotify += MakeDelegate(this,&CTreeViewUI::OnDBClickItem);
		pControl->GetFolderButton()->OnNotify += MakeDelegate(this,&CTreeViewUI::OnFolderChanged);
		pControl->GetCheckBox()->OnNotify += MakeDelegate(this,&CTreeViewUI::OnCheckBoxChanged);

		pControl->SetVisibleFolderBtn(m_bVisibleFolderBtn);
		pControl->SetVisibleCheckBtn(m_bVisibleCheckBtn);
		if(m_uItemMinWidth > 0)
			pControl->SetMinWidth(m_uItemMinWidth);

		CListUI::Add(pControl);

		int nLevel = pControl->GetTreeLevel();
		int nFolderWidth = pControl->GetFolderButton()->GetFixedWidth();
		if(nFolderWidth <= 0) nFolderWidth = 16;
		if(!pControl->GetFolderButton()->IsVisible()) nFolderWidth = 0;
		pControl->GetFolderButton()->SetPadding(CDuiRect(nLevel * nFolderWidth, 0, 0, 0));

		if(pControl->GetCountChild() > 0) {
			int nCount = pControl->GetCountChild();
			for(int nIndex = 0;nIndex < nCount;nIndex++) {
				CTreeNodeUI* pNode = pControl->GetChildNode(nIndex);
				if(pNode) Add(pNode);
			}
		}

		pControl->SetTreeView(this);
		return true;
	}

	//************************************
	// 鍑芥暟鍚嶇О: AddAt
	// 杩斿洖绫诲瀷: long
	// 鍙傛暟淇℃伅: CTreeNodeUI * pControl
	// 鍙傛暟淇℃伅: int iIndex
	// 鍑芥暟璇存槑: 璇ユ柟娉曚笉浼氬皢寰呮彃鍏ョ殑鑺傜偣杩涜缂╀綅澶勭悊锛岃嫢鎵撶畻鎻掑叆鐨勮妭鐐逛负闈炴牴鑺傜偣锛岃浣跨敤AddAt(CTreeNodeUI* pControl,CTreeNodeUI* _IndexNode) 鏂规硶
	//************************************
	long CTreeViewUI::AddAt( CTreeNodeUI* pControl, int iIndex )
	{
		if (!pControl) return -1;
		if (NULL == static_cast<CTreeNodeUI*>(pControl->GetInterface(_T("TreeNode")))) return -1;
		pControl->OnNotify += MakeDelegate(this,&CTreeViewUI::OnDBClickItem);
		pControl->GetFolderButton()->OnNotify += MakeDelegate(this,&CTreeViewUI::OnFolderChanged);
		pControl->GetCheckBox()->OnNotify += MakeDelegate(this,&CTreeViewUI::OnCheckBoxChanged);
		pControl->SetVisibleFolderBtn(m_bVisibleFolderBtn);
		pControl->SetVisibleCheckBtn(m_bVisibleCheckBtn);

		if(m_uItemMinWidth > 0) {
			pControl->SetMinWidth(m_uItemMinWidth);
		}
		CListUI::AddAt(pControl, iIndex);

		int nLevel = pControl->GetTreeLevel();
		int nFolderWidth = pControl->GetFolderButton()->GetFixedWidth();
		if(nFolderWidth <= 0) nFolderWidth = 16;
		if(!pControl->GetFolderButton()->IsVisible()) nFolderWidth = 0;
		pControl->GetFolderButton()->SetPadding(CDuiRect(nLevel * nFolderWidth, 0, 0, 0));

		if(pControl->GetCountChild() > 0) {
			int nCount = pControl->GetCountChild();
			for(int nIndex = 0; nIndex < nCount; nIndex++) {
				CTreeNodeUI* pNode = pControl->GetChildNode(nIndex);
				if(pNode)
					return AddAt(pNode,iIndex+1);
			}
		}
		else {
			return iIndex + 1;
		}

		return -1;
	}

	//************************************
	// 鍑芥暟鍚嶇О: AddAt
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: CTreeNodeUI * pControl
	// 鍙傛暟淇℃伅: CTreeNodeUI * _IndexNode
	// 鍑芥暟璇存槑:
	//************************************
	bool CTreeViewUI::AddAt( CTreeNodeUI* pControl, CTreeNodeUI* _IndexNode )
	{
		if(!_IndexNode && !pControl)
			return false;

		int nItemIndex = -1;
		for(int nIndex = 0;nIndex < GetCount();nIndex++) {
			if(_IndexNode == GetItemAt(nIndex)) {
				nItemIndex = nIndex;
				break;
			}
		}

		if(nItemIndex == -1)
			return false;

		bool bRet = AddAt(pControl,nItemIndex) >= 0;
		if(bRet) {
			int nLevel = pControl->GetTreeLevel();
			int nFolderWidth = pControl->GetFolderButton()->GetFixedWidth();
			if(nFolderWidth <= 0) nFolderWidth = 16;
			if(!pControl->GetFolderButton()->IsVisible()) nFolderWidth = 0;
			pControl->GetFolderButton()->SetPadding(CDuiRect(nLevel * nFolderWidth, 0, 0, 0));
		}

		return bRet;
	}

	//************************************
	// 鍑芥暟鍚嶇О: Remove
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: CTreeNodeUI * pControl
	// 鍑芥暟璇存槑: pControl 瀵硅薄浠ュ強涓嬬殑鎵€鏈夎妭鐐瑰皢琚竴骞剁Щ闄?
	//************************************
	bool CTreeViewUI::Remove( CTreeNodeUI* pControl )
	{
		if(pControl->GetCountChild() > 0) {
			int nCount = pControl->GetCountChild();
			for(int nIndex = nCount - 1; nIndex >= 0; nIndex--) {
				CTreeNodeUI* pNode = pControl->GetChildNode(nIndex);
				if(pNode){
					pControl->Remove(pNode);
				}
			}
		}
		CListUI::Remove(pControl);
		return TRUE;
	}

	//************************************
	// 鍑芥暟鍚嶇О: RemoveAt
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: int iIndex
	// 鍑芥暟璇存槑: iIndex 绱㈠紩浠ュ強涓嬬殑鎵€鏈夎妭鐐瑰皢琚竴骞剁Щ闄?
	//************************************
	bool CTreeViewUI::RemoveAt( int iIndex )
	{
		CTreeNodeUI* pItem = (CTreeNodeUI*)GetItemAt(iIndex);
		Remove(pItem);
		return TRUE;
	}

	void CTreeViewUI::RemoveAll()
	{
		CListUI::RemoveAll();
	}

	//************************************
	// 鍑芥暟鍚嶇О: Notify
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: TNotifyUI & msg
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::Notify( TNotifyUI& msg )
	{

	}

	//************************************
	// 鍑芥暟鍚嶇О: OnCheckBoxChanged
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: void * param
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeViewUI::OnCheckBoxChanged( void* param )
	{
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if(pMsg->sType == DUI_MSGTYPE_SELECTCHANGED)
		{
			CCheckBoxUI* pCheckBox = (CCheckBoxUI*)pMsg->pSender;
			CTreeNodeUI* pItem = (CTreeNodeUI*)pCheckBox->GetParent()->GetParent();
			SetItemCheckBox(pCheckBox->GetCheck(),pItem);
			return TRUE;
		}
		return TRUE;
	}

	//************************************
	// 鍑芥暟鍚嶇О: OnFolderChanged
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: void * param
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeViewUI::OnFolderChanged( void* param )
	{
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if(pMsg->sType == DUI_MSGTYPE_SELECTCHANGED) {
			CCheckBoxUI* pFolder = (CCheckBoxUI*)pMsg->pSender;
			CTreeNodeUI* pItem = (CTreeNodeUI*)pFolder->GetParent()->GetParent();
			pItem->SetVisibleTag(!pFolder->GetCheck());
			SetItemExpand(!pFolder->GetCheck(),pItem);
			return TRUE;
		}
		return TRUE;
	}

	//************************************
	// 鍑芥暟鍚嶇О: OnDBClickItem
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: void * param
	// 鍑芥暟璇存槑:
	//************************************
	bool CTreeViewUI::OnDBClickItem( void* param )
	{
		TNotifyUI* pMsg = (TNotifyUI*)param;
		if (StringUtil::EqualsNoCase(pMsg->sType, DUI_MSGTYPE_TREEITEMDBCLICK)) {
			CTreeNodeUI* pItem		= static_cast<CTreeNodeUI*>(pMsg->pSender);
			CCheckBoxUI* pFolder	= pItem->GetFolderButton();
			pFolder->Selected(!pFolder->IsSelected());
			pItem->SetVisibleTag(!pFolder->GetCheck());
			SetItemExpand(!pFolder->GetCheck(),pItem);
			return TRUE;
		}
		return FALSE;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetItemCheckBox
	// 杩斿洖绫诲瀷: bool
	// 鍙傛暟淇℃伅: bool _Selected
	// 鍙傛暟淇℃伅: CTreeNodeUI * _TreeNode
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeViewUI::SetItemCheckBox( bool _Selected,CTreeNodeUI* _TreeNode /*= NULL*/ )
	{
		if(_TreeNode) {
			if(_TreeNode->GetCountChild() > 0) {
				int nCount = _TreeNode->GetCountChild();
				for(int nIndex = 0;nIndex < nCount;nIndex++) {
					CTreeNodeUI* pItem = _TreeNode->GetChildNode(nIndex);
					pItem->GetCheckBox()->Selected(_Selected);
					if(pItem->GetCountChild())
						SetItemCheckBox(_Selected,pItem);
				}
			}
			return TRUE;
		}
		else {
			int nIndex = 0;
			int nCount = GetCount();
			while(nIndex < nCount) {
				CTreeNodeUI* pItem = (CTreeNodeUI*)GetItemAt(nIndex);
				pItem->GetCheckBox()->Selected(_Selected);
				if(pItem->GetCountChild())
					SetItemCheckBox(_Selected,pItem);

				nIndex++;
			}
			return TRUE;
		}
		return FALSE;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetItemExpand
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: bool _Expanded
	// 鍙傛暟淇℃伅: CTreeNodeUI * _TreeNode
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::SetItemExpand( bool _Expanded,CTreeNodeUI* _TreeNode /*= NULL*/ )
	{
		if(_TreeNode) {
			if(_TreeNode->GetCountChild() > 0) {
				int nCount = _TreeNode->GetCountChild();
				for(int nIndex = 0;nIndex < nCount;nIndex++) {
					CTreeNodeUI* pItem = _TreeNode->GetChildNode(nIndex);
					pItem->SetVisible(_Expanded);
					if(pItem->GetCountChild() && !pItem->GetFolderButton()->IsSelected()) {
						SetItemExpand(_Expanded,pItem);
					}
				}
			}
		}
		else {
			int nIndex = 0;
			int nCount = GetCount();
			while(nIndex < nCount) {
				CTreeNodeUI* pItem = (CTreeNodeUI*)GetItemAt(nIndex);
				pItem->GetFolderButton()->Selected(!_Expanded);
				if(pItem->GetCountChild() && !pItem->GetFolderButton()->IsSelected()) {
					SetItemExpand(_Expanded, pItem);
				}
				nIndex++;
			}
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetVisibleFolderBtn
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: bool _IsVisibled
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::SetVisibleFolderBtn( bool _IsVisibled )
	{
		m_bVisibleFolderBtn = _IsVisibled;
		int nCount = this->GetCount();
		for(int nIndex = 0; nIndex < nCount; nIndex++) {
			CTreeNodeUI* pItem = static_cast<CTreeNodeUI*>(this->GetItemAt(nIndex));
			pItem->GetFolderButton()->SetVisible(m_bVisibleFolderBtn);
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetVisibleFolderBtn
	// 杩斿洖绫诲瀷: bool
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeViewUI::GetVisibleFolderBtn()
	{
		return m_bVisibleFolderBtn;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetVisibleCheckBtn
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: bool _IsVisibled
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::SetVisibleCheckBtn( bool _IsVisibled )
	{
		m_bVisibleCheckBtn = _IsVisibled;
		int nCount = this->GetCount();
		for(int nIndex = 0; nIndex < nCount; nIndex++) {
			CTreeNodeUI* pItem = static_cast<CTreeNodeUI*>(this->GetItemAt(nIndex));
			pItem->GetCheckBox()->SetVisible(m_bVisibleCheckBtn);
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetVisibleCheckBtn
	// 杩斿洖绫诲瀷: bool
	// 鍑芥暟璇存槑: 
	//************************************
	bool CTreeViewUI::GetVisibleCheckBtn()
	{
		return m_bVisibleCheckBtn;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetItemMinWidth
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: UINT _ItemMinWidth
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::SetItemMinWidth( UINT _ItemMinWidth )
	{
		m_uItemMinWidth = _ItemMinWidth;

		for(int nIndex = 0;nIndex < GetCount();nIndex++){
			CTreeNodeUI* pTreeNode = static_cast<CTreeNodeUI*>(GetItemAt(nIndex));
			if(pTreeNode) {
				pTreeNode->SetMinWidth(GetItemMinWidth());
			}
		}
		Invalidate();
	}

	//************************************
	// 鍑芥暟鍚嶇О: GetItemMinWidth
	// 杩斿洖绫诲瀷: UINT
	// 鍑芥暟璇存槑: 
	//************************************
	UINT CTreeViewUI::GetItemMinWidth()
	{
		return m_uItemMinWidth;
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetItemTextColor
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: DWORD _dwItemTextColor
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::SetItemTextColor( DWORD _dwItemTextColor )
	{
		for(int nIndex = 0;nIndex < GetCount();nIndex++){
			CTreeNodeUI* pTreeNode = static_cast<CTreeNodeUI*>(GetItemAt(nIndex));
			if(pTreeNode) {
				pTreeNode->SetItemTextColor(_dwItemTextColor);
			}
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetItemHotTextColor
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: DWORD _dwItemHotTextColor
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::SetItemHotTextColor( DWORD _dwItemHotTextColor )
	{
		for(int nIndex = 0;nIndex < GetCount();nIndex++){
			CTreeNodeUI* pTreeNode = static_cast<CTreeNodeUI*>(GetItemAt(nIndex));
			if(pTreeNode) {
				pTreeNode->SetItemHotTextColor(_dwItemHotTextColor);
			}
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetSelItemTextColor
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: DWORD _dwSelItemTextColor
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::SetSelItemTextColor( DWORD _dwSelItemTextColor )
	{
		for(int nIndex = 0;nIndex < GetCount();nIndex++){
			CTreeNodeUI* pTreeNode = static_cast<CTreeNodeUI*>(GetItemAt(nIndex));
			if(pTreeNode) {
				pTreeNode->SetSelItemTextColor(_dwSelItemTextColor);
			}
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetSelItemHotTextColor
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: DWORD _dwSelHotItemTextColor
	// 鍑芥暟璇存槑: 
	//************************************
	void CTreeViewUI::SetSelItemHotTextColor( DWORD _dwSelHotItemTextColor )
	{
		for(int nIndex = 0;nIndex < GetCount();nIndex++){
			CTreeNodeUI* pTreeNode = static_cast<CTreeNodeUI*>(GetItemAt(nIndex));
			if(pTreeNode) {
				pTreeNode->SetSelItemHotTextColor(_dwSelHotItemTextColor);
			}
		}
	}

	//************************************
	// 鍑芥暟鍚嶇О: SetAttribute
	// 杩斿洖绫诲瀷: void
	// 鍙傛暟淇℃伅: std::wstring_view pstrName
	// 鍙傛暟淇℃伅: std::wstring_view pstrValue
	// 鍑芥暟璇存槑: 
	//************************************
	    void CTreeViewUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
    {
        const std::wstring_view name = StringUtil::TrimView(pstrNameView);
        if (name.empty()) {
            return;
        }

        if (StringUtil::EqualsNoCase(name, L"visiblefolderbtn")) SetVisibleFolderBtn(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"visiblecheckbtn")) SetVisibleCheckBtn(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"itemminwidth")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetItemMinWidth(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemtextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetItemTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemhottextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetItemHotTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"selitemtextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetSelItemTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"selitemhottextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetSelItemHotTextColor(color);
        }
        else CListUI::SetAttribute(pstrNameView, pstrValueView);
    }

}

