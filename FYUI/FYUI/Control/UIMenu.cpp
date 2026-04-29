#include "pch.h"
#include "UIMenu.h"
#include "../Core/Render/UIRenderContext.h"

namespace FYUI {

	/////////////////////////////////////////////////////////////////////////////////////
	//
	IMPLEMENT_DUICONTROL(CMenuUI)

		CMenuUI::CMenuUI():
		m_pWindow(NULL)
	{
		if (GetHeader() != NULL)
			GetHeader()->SetVisible(false);
	}

	CMenuUI::~CMenuUI()
	{

	}

	std::wstring_view CMenuUI::GetClass() const
	{
		return _T("MenuUI");
	}

	LPVOID CMenuUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("Menu")) == 0 ) return static_cast<CMenuUI*>(this);
		return CListUI::GetInterface(pstrName);
	}

	UINT CMenuUI::GetListType()
	{
		return LT_MENU;
	}

	void CMenuUI::DoEvent(TEventUI& event)
	{
		return __super::DoEvent(event);
	}

	bool CMenuUI::Add(CControlUI* pControl)
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return false;

		for (int i = 0; i < pMenuItem->GetCount(); ++i)
		{
			if (pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
			{
				(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
			}
		}
		return CListUI::Add(pControl);
	}

	bool CMenuUI::AddAt(CControlUI* pControl, int iIndex)
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return false;

		for (int i = 0; i < pMenuItem->GetCount(); ++i)
		{
			if (pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
			{
				(static_cast<CMenuElementUI*>(pMenuItem->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
			}
		}
		return CListUI::AddAt(pControl, iIndex);
	}

	int CMenuUI::GetItemIndex(CControlUI* pControl) const
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return -1;

		return __super::GetItemIndex(pControl);
	}

	bool CMenuUI::SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate )
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return false;

		return __super::SetItemIndex(pControl, iIndex, bUpdate);
	}

	bool CMenuUI::Remove(CControlUI* pControl)
	{
		CMenuElementUI* pMenuItem = static_cast<CMenuElementUI*>(pControl->GetInterface(_T("MenuElement")));
		if (pMenuItem == NULL)
			return false;

		return __super::Remove(pControl);
	}

	SIZE CMenuUI::EstimateSize(SIZE szAvailable)
	{
		SIZE szRootFixed = GetManager()->ScaleSize(m_cxyFixed);
		int cxFixed = szRootFixed.cx;
		int cyFixed = szRootFixed.cy;
		for( int it = 0; it < GetCount(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
			if( !pControl->IsVisible() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			cyFixed += sz.cy;
			if( cxFixed < sz.cx )
				cxFixed = sz.cx;
		}

		for (int it = 0; it < GetCount(); it++) {
			CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
			if (!pControl->IsVisible()) continue;

			pControl->SetFixedWidth(GetManager()->UnscaleValue(cxFixed));
		}

		return CDuiSize(cxFixed, cyFixed);
	}

	void CMenuUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		CListUI::SetAttribute(pstrName, pstrValue);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//

	CMenuWnd::CMenuWnd():
		m_pOwner(NULL),
		m_pLayout(),
		m_xml(_T("")),
		isClosing(false),
		m_bCaptured(false)
	{
		m_dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top;
		m_nSpacing = 0;
	}

	CMenuWnd::~CMenuWnd()
	{

	}

	void CMenuWnd::Close(UINT nRet)
	{
		ASSERT(::IsWindow(m_hWnd));
		if (!::IsWindow(m_hWnd)) return;
		PostMessage(WM_CLOSE, (WPARAM)nRet, 0L);
		isClosing = true;
	}


	BOOL CMenuWnd::Receive(ContextMenuParam param)
	{
		switch (param.wParam)
		{
		case 1:
			Close();
			break;
		case 2:
		{
			HWND hParent = GetParent(m_hWnd);
			while (hParent != NULL)
			{
				if (hParent == param.hWnd)
				{
					Close();
					break;
				}
				hParent = GetParent(hParent);
			}
		}
		break;
		default:
			break;
		}

		return TRUE;
	}

	CMenuWnd* CMenuWnd::CreateMenu(CMenuElementUI* pOwner, STRINGorID xml, POINT point, CPaintManagerUI* pMainPaintManager, MenuCheckInfoMap* pMenuCheckInfo /*= NULL*/, DWORD dwAlignment /*= eMenuAlignment_Left | eMenuAlignment_Top*/)
	{
		CMenuWnd* pMenu = new CMenuWnd;
		pMenu->Init(pOwner, xml, point, pMainPaintManager, pMenuCheckInfo, dwAlignment);
		return pMenu;
	}

	void CMenuWnd::DestroyMenu()
	{
		MenuCheckInfoMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			for (auto& entry : *mCheckInfos) {
				delete entry.second;
				entry.second = NULL;
			}
			mCheckInfos->clear();
		}
	}

	MenuItemInfo* CMenuWnd::SetMenuItemInfo(std::wstring_view pstrName, bool bChecked)
	{
		if(pstrName.empty()) return NULL;

		MenuCheckInfoMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			auto it = mCheckInfos->find(pstrName);
			if (it == mCheckInfos->end()) {
				MenuItemInfo* pItemInfo = new MenuItemInfo;
				if (pItemInfo == NULL) return NULL;
				const std::wstring name(pstrName);
				lstrcpy(pItemInfo->szName, name.c_str());
				pItemInfo->bChecked = bChecked;
				mCheckInfos->emplace(pstrName, pItemInfo);
				return pItemInfo;
			}
			it->second->bChecked = bChecked;
			return it->second;
		}
		return NULL;
	}

	void CMenuWnd::Init(CMenuElementUI* pOwner, STRINGorID xml, POINT point,
		CPaintManagerUI* pMainPaintManager, MenuCheckInfoMap* pMenuCheckInfo/* = NULL*/,
		DWORD dwAlignment/* = eMenuAlignment_Left | eMenuAlignment_Top*/)
	{

		m_BasedPoint = point;
		m_pOwner = pOwner;
		m_pLayout = NULL;
		m_xml = xml;
		m_dwAlignment = dwAlignment;

		// 婵″倹鐏夐弰顖欑缁狙嗗綅閸楁洜娈戦崚娑樼紦
		if (pOwner == NULL)
		{
			ASSERT(pMainPaintManager != NULL);
			CMenuWnd::GetGlobalContextMenuObserver().SetManger(pMainPaintManager);
			if (pMenuCheckInfo != NULL)
				CMenuWnd::GetGlobalContextMenuObserver().SetMenuCheckInfo(pMenuCheckInfo);
		}

		CMenuWnd::GetGlobalContextMenuObserver().AddReceiver(this);

		Create((m_pOwner == NULL) ? pMainPaintManager->GetPaintWindow() : m_pOwner->GetManager()->GetPaintWindow(), {}, WS_POPUP , WS_EX_TOOLWINDOW | WS_EX_TOPMOST, CDuiRect());

		// HACK: Don't deselect the parent's caption
		HWND hWndParent = m_hWnd;
		while( ::GetParent(hWndParent) != NULL ) hWndParent = ::GetParent(hWndParent);

		::ShowWindow(m_hWnd, SW_SHOW);
		::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);
	}

	std::wstring_view CMenuWnd::GetWindowClassName() const
	{
		return _T("DuiMenuWnd");
	}


	void CMenuWnd::Notify(TNotifyUI& msg)
	{
		if( CMenuWnd::GetGlobalContextMenuObserver().GetManager() != NULL) 
		{
			if( msg.sType == _T("click") || msg.sType == _T("valuechanged") /*|| msg.sType == _T("mouseenter")*/) 
			{
				CMenuWnd::GetGlobalContextMenuObserver().GetManager()->SendNotify(msg, true);
			}
		}

	}

	CControlUI* CMenuWnd::CreateControl(std::wstring_view pstrClassName)
	{
		if (StringUtil::CompareNoCase(pstrClassName, L"Menu") == 0)
		{
			return new CMenuUI();
		}
		else if (StringUtil::CompareNoCase(pstrClassName, L"MenuElement") == 0)
		{
			return new CMenuElementUI();
		}
		return NULL;
	}


	void CMenuWnd::OnFinalMessage(HWND hWnd)
	{
		RemoveObserver();
		if( m_pOwner != NULL ) {
			for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
				if( static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement"))) != NULL ) {
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pOwner->GetParent());
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetVisible(false);
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(false);
				}
			}
			m_pOwner->m_pWindow = NULL;
			m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
			m_pOwner->Invalidate();

			// 閸愬懘鍎撮崚娑樼紦閻ㄥ嫬鍞撮柈銊ュ灩闂?
			delete this;
		}
	}

	LRESULT CMenuWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bool bShowShadow = false;
		if( m_pOwner != NULL) {
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			RECT rcClient;
			::GetClientRect(*this, &rcClient);
			::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
				rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

			m_pm.Init(m_hWnd);
			m_pm.SyncDPIFrom(m_pOwner->GetManager());
			hParentWnd =m_pOwner->GetManager()->GetPaintWindow();
			// The trick is to add the items to the new container. Their owner gets
			// reassigned by this operation - which is why it is important to reassign
			// the items back to the righfull owner/manager when the window closes.
			m_pLayout = new CMenuUI();
			m_pm.SetForceUseSharedRes(true);
			m_pLayout->SetManager(&m_pm, NULL, true);
			const std::wstring_view pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(L"Menu");
			if( !pDefaultAttributes.empty() ) {
				m_pLayout->ApplyAttributeList(pDefaultAttributes);
			}
			m_pLayout->GetList()->SetAutoDestroy(false);

			for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
				if(m_pOwner->GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL ){
					(static_cast<CMenuElementUI*>(m_pOwner->GetItemAt(i)))->SetOwner(m_pLayout);
					m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
				}
			}

			CShadowUI *pShadow = m_pOwner->GetManager()->GetShadow();
			pShadow->CopyShadow(m_pm.GetShadow());
			bShowShadow = m_pm.GetShadow()->IsShowShadow();
			m_pm.GetShadow()->ShowShadow(false);
			m_pm.SetRenderBackend(m_pOwner->GetManager()->GetRenderBackend());
			m_pm.SetDirect2DRenderMode(m_pOwner->GetManager()->GetDirect2DRenderMode());
			m_pm.SetRenderDiagnosticsEnabled(
				m_pOwner->GetManager()->IsRenderDiagnosticsEnabled(),
				m_pOwner->GetManager()->IsRenderDiagnosticsDebugTraceEnabled());
			m_pm.SetLayered(m_pOwner->GetManager()->IsLayered());
			m_pm.AttachDialog(m_pLayout);
			m_pm.AddNotifier(this);

			ResizeSubMenu();
		}
		else {
			m_pm.Init(m_hWnd);
			CPaintManagerUI* pContextManager = CMenuWnd::GetGlobalContextMenuObserver().GetManager();
			if (pContextManager != NULL) {
				m_pm.SyncDPIFrom(pContextManager);
				m_pm.SetRenderBackend(pContextManager->GetRenderBackend());
				m_pm.SetDirect2DRenderMode(pContextManager->GetDirect2DRenderMode());
				m_pm.SetRenderDiagnosticsEnabled(
					pContextManager->IsRenderDiagnosticsEnabled(),
					pContextManager->IsRenderDiagnosticsDebugTraceEnabled());
				m_pm.SetLayered(pContextManager->IsLayered());
			}
			CDialogBuilder builder;

			CControlUI* pRoot = builder.Create(m_xml, {}, this, &m_pm);
			bShowShadow = m_pm.GetShadow()->IsShowShadow();
			m_pm.GetShadow()->ShowShadow(false);
			m_pm.AttachDialog(pRoot);
			m_pm.AddNotifier(this);

			ResizeMenu();
		}
		GetMenuUI()->m_pWindow = this;
		m_pm.GetShadow()->ShowShadow(bShowShadow);
		m_pm.GetShadow()->Create(&m_pm);
		return 0;
	}

	CMenuUI* CMenuWnd::GetMenuUI()
	{
		return static_cast<CMenuUI*>(m_pm.GetRoot());
	}

	void CMenuWnd::ResizeMenu()
	{
		CControlUI* pRoot = m_pm.GetRoot();
		CMenuUI* pMenuRoot = static_cast<CMenuUI*>(pRoot);

#if defined(WIN32) && !defined(UNDER_CE)
		MONITORINFO oMonitor = {}; 
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromPoint(m_BasedPoint, MONITOR_DEFAULTTONEAREST), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
#else
		CDuiRect rcWork;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
		SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };
		szAvailable = pRoot->EstimateSize(szAvailable);
		m_pm.SetInitSize(szAvailable.cx, szAvailable.cy);

		//韫囧懘銆忛弰鐤nu閺嶅洨顒锋担婊€璐焫ml閻ㄥ嫭鐗撮懞鍌滃仯
		pMenuRoot = static_cast<CMenuUI*>(pRoot);
		ASSERT(pMenuRoot);

		SIZE szInit = m_pm.GetInitSize();
		CDuiRect rc;
		CDuiPoint point = m_BasedPoint;
		rc.left = point.x;
		rc.top = point.y;
		rc.right = rc.left + szInit.cx;
		rc.bottom = rc.top + szInit.cy;

		int nWidth = rc.GetWidth();
		int nHeight = rc.GetHeight();

		if (m_dwAlignment & eMenuAlignment_Right)
		{
			rc.right = point.x;
			rc.left = rc.right - nWidth;
		}

		if (m_dwAlignment & eMenuAlignment_Bottom)
		{
			rc.bottom = point.y;
			rc.top = rc.bottom - nHeight;
		}
		if ( rc.top < 0 )
		{
			int nHeight = rc.bottom - rc.top;
			rc.top = 0;
			rc.bottom = nHeight;
		}
		else if ( rc.bottom > rcWork.bottom )
		{
			int nHeight = rc.bottom - rc.top;
			rc.bottom = rcWork.bottom;
			rc.top = rc.bottom - nHeight;
		}

		SetForegroundWindow(m_hWnd);
		MoveWindow(m_hWnd, rc.left, rc.top, rc.GetWidth(), rc.GetHeight(), FALSE);
		SetWindowPos(m_hWnd, HWND_TOPMOST, rc.left, rc.top, rc.GetWidth(), rc.GetHeight() + pMenuRoot->GetInset().bottom + pMenuRoot->GetInset().top, SWP_SHOWWINDOW);
	}

	void CMenuWnd::ResizeSubMenu()
	{
		// Position the popup window in absolute space
		RECT rcOwner = m_pOwner->GetPos();
		RECT rcOwnerScreen = rcOwner;
		::MapWindowRect(m_pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rcOwnerScreen);
		RECT rc = rcOwnerScreen;

		int cxFixed = 0;
		int cyFixed = 0;

#if defined(WIN32) && !defined(UNDER_CE)
		MONITORINFO oMonitor = {}; 
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromRect(&rcOwnerScreen, MONITOR_DEFAULTTONEAREST), &oMonitor);
		CDuiRect rcWork = oMonitor.rcWork;
#else
		CDuiRect rcWork;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWork);
#endif
		SIZE szAvailable = { rcWork.right - rcWork.left, rcWork.bottom - rcWork.top };

		for( int it = 0; it < m_pOwner->GetCount(); it++ ) {
			if(m_pOwner->GetItemAt(it)->GetInterface(_T("MenuElement")) != NULL ){
				CControlUI* pControl = static_cast<CControlUI*>(m_pOwner->GetItemAt(it));
				SIZE sz = pControl->EstimateSize(szAvailable);
				cyFixed += sz.cy;
				if( cxFixed < sz.cx ) cxFixed = sz.cx;
			}
		}

		RECT rcWindow;
		GetWindowRect(m_pOwner->GetManager()->GetPaintWindow(), &rcWindow);

		rc.top = rcOwnerScreen.top;
		rc.bottom = rc.top + cyFixed;
		rc.left = rcWindow.right;
		rc.right = rc.left + cxFixed;
		rc.right += 2;

		bool bReachBottom = false;
		bool bReachRight = false;
		LONG chRightAlgin = 0;
		LONG chBottomAlgin = 0;

		int nDpiSpacing = m_pOwner->GetManager()->ScaleValue(m_nSpacing);

		RECT rcPreWindow = {0};
		MenuObserverImpl::Iterator iterator(CMenuWnd::GetGlobalContextMenuObserver());
		MenuMenuReceiverImplBase* pReceiver = iterator.next();
		while( pReceiver != NULL ) {
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			if( pContextMenu != NULL ) {
				GetWindowRect(pContextMenu->GetHWND(), &rcPreWindow);

				bReachRight = rcPreWindow.left >= rcWindow.right;
				bReachBottom = rcPreWindow.top >= rcWindow.bottom;
				if( pContextMenu->GetHWND() == m_pOwner->GetManager()->GetPaintWindow() ||  bReachBottom || bReachRight )
					break;
			}
			pReceiver = iterator.next();
		}

		if (bReachBottom)
		{
			rc.bottom = rcWindow.top;
			rc.top = rc.bottom - cyFixed;
		}

		if (bReachRight)
		{
			rc.right = rcWindow.left;
			rc.left = rc.right - cxFixed;
		}

		if( rc.bottom > rcWork.bottom )
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - cyFixed;
		}

		if (rc.right > rcWork.right)
		{
			nDpiSpacing *= -1;
			rc.right = rcWindow.left;
			rc.left = rc.right - cxFixed;
		}

		if( rc.top < rcWork.top )
		{
			rc.top = rcOwnerScreen.top;
			rc.bottom = rc.top + cyFixed;
		}

		if (rc.left < rcWork.left)
		{
			rc.left = rcWindow.right;
			rc.right = rc.left + cxFixed;
		}

		MoveWindow(m_hWnd, rc.left + nDpiSpacing , rc.top, rc.right - rc.left, rc.bottom - rc.top + m_pLayout->GetInset().top + m_pLayout->GetInset().bottom, FALSE);
	}

	void CMenuWnd::setDPI(int DPI) {
		m_pm.SetDPI(DPI);
	}
	void CMenuWnd::SetSubMenuSpacing(int nSpacing) {
		m_nSpacing = nSpacing;
	}


	LRESULT CMenuWnd::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		HWND hFocusWnd = (HWND)wParam;

		BOOL bInMenuWindowList = FALSE;
		ContextMenuParam param;
		param.hWnd = GetHWND();

		MenuObserverImpl::Iterator iterator(CMenuWnd::GetGlobalContextMenuObserver());
		MenuMenuReceiverImplBase* pReceiver = iterator.next();
		while( pReceiver != NULL ) {
			CMenuWnd* pContextMenu = dynamic_cast<CMenuWnd*>(pReceiver);
			HWND hWnd = m_pm.GetTooltipWindow();
			if (IsWindowVisible(hWnd))
			{
				int A=0;
			}
			if( (pContextMenu != NULL && pContextMenu->GetHWND() ==  hFocusWnd ) ) {
				bInMenuWindowList = TRUE;
				break;
			}
			pReceiver = iterator.next();
		}

		if( !bInMenuWindowList ) {
			param.wParam = 1;
			CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
			return 0;
		}
		return 0;
	}

	LRESULT CMenuWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
		if( !::IsIconic(*this) ) {
			CDuiRect rcWnd;
			::GetWindowRect(*this, &rcWnd);
			rcWnd.Offset(-rcWnd.left, -rcWnd.top);
			rcWnd.right++; rcWnd.bottom++;
			HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
			::SetWindowRgn(*this, hRgn, TRUE);
			::DeleteObject(hRgn);
		}
		bHandled = FALSE;
		return 0;
	}

	LRESULT CMenuWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch( uMsg )
		{
		case WM_CREATE:       
			lRes = OnCreate(uMsg, wParam, lParam, bHandled); 
			break;
		case WM_KILLFOCUS:       
			lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); 
			break;
		case WM_KEYDOWN:
		{
			if( wParam == VK_ESCAPE /*|| wParam == VK_LEFT*/)
				Close();
			else if (wParam == VK_RETURN)
			{
				//PostMessageA(GetParent(m_hWnd),WM_KEYDOWN,VK_RETURN,0);
			}
		}

		break;
		case WM_SIZE:
			lRes = OnSize(uMsg, wParam, lParam, bHandled);
			break;
		case WM_CLOSE:
			if( m_pOwner != NULL )
			{
				m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
				m_pOwner->SetPos(m_pOwner->GetPos());
				m_pOwner->SetFocus();
			}
			break;
		case WM_CONTEXTMENU:
		case WM_RBUTTONUP:
			if(m_bCaptured) {
				m_bCaptured = false;
				ReleaseCapture();
				if( m_pOwner != NULL )
				{
					m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
					m_pOwner->SetPos(m_pOwner->GetPos());
					m_pOwner->SetFocus();
				}
			}
			break;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
			m_bCaptured = true;
			SetCapture(m_hWnd);
			return 0L;
		default:
			bHandled = FALSE;
			break;
		}

		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	IMPLEMENT_DUICONTROL(CMenuElementUI)

		CMenuElementUI::CMenuElementUI():
		m_pWindow(NULL),
		m_bDrawLine(false),
		m_dwLineColor((DWORD)DEFAULT_LINE_COLOR),
		m_bCheckItem(false),
		m_bShowExplandIcon(false)
	{
		m_cxyFixed.cy = ITEM_DEFAULT_HEIGHT;
		m_cxyFixed.cx = ITEM_DEFAULT_WIDTH;
		m_szIconSize.cy = ITEM_DEFAULT_ICON_SIZE;
		m_szIconSize.cx = ITEM_DEFAULT_ICON_SIZE;

		m_rcLinePadding.top = m_rcLinePadding.bottom = 0;
		m_rcLinePadding.left = DEFAULT_LINE_LEFT_INSET;
		m_rcLinePadding.right = DEFAULT_LINE_RIGHT_INSET;
	}

	CMenuElementUI::~CMenuElementUI()
	{}

	std::wstring_view CMenuElementUI::GetClass() const
	{
		return _T("MenuElementUI");
	}

	LPVOID CMenuElementUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("MenuElement")) == 0 ) return static_cast<CMenuElementUI*>(this);    
		return CListContainerElementUI::GetInterface(pstrName);
	}

	void CMenuElementUI::SetOwner(CControlUI* pOwner)
	{
		if (pOwner != NULL) {
			CListContainerElementUI::SetOwner(pOwner);
		}
		else {
			m_pOwner = NULL;
		}

		for (int i = 0; i < GetCount(); ++i) {
			CControlUI* pChild = GetItemAt(i);
			if (pChild == NULL) {
				continue;
			}

			CMenuElementUI* pMenuChild = static_cast<CMenuElementUI*>(pChild->GetInterface(_T("MenuElement")));
			if (pMenuChild != NULL) {
				pMenuChild->SetOwner(pOwner);
			}
		}
	}

	bool CMenuElementUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		const RECT& rcPaint = renderContext.GetPaintRect();
		SIZE cxyFixed = GetFixedSize();
		RECT rcLinePadding = GetLinePadding();

		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

		if(m_bDrawLine)
		{
			RECT rcLine = { m_rcItem.left +  rcLinePadding.left, m_rcItem.top + cxyFixed.cy/2, m_rcItem.right - rcLinePadding.right, m_rcItem.top + cxyFixed.cy/2 };
			CRenderEngine::DrawLine(renderContext, rcLine, 1, m_dwLineColor);
		}
		else
		{
			CRenderClip clip;
			CRenderClip::GenerateClip(renderContext, rcTemp, clip);
			CMenuElementUI::DrawItemBk(renderContext, m_rcItem);
			DrawItemText(renderContext, m_rcItem);
			DrawItemIcon(renderContext, m_rcItem);
			DrawItemExpland(renderContext, m_rcItem);

			if( m_items.GetSize() > 0 ) {
				RECT rc = m_rcItem;

				RECT rcInset = GetInset();
				rc.left += rcInset.left;
				rc.top += rcInset.top;
				rc.right -= rcInset.right;
				rc.bottom -= rcInset.bottom;
				if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
				if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

				if( !::IntersectRect(&rcTemp, &rcPaint, &rc) ) {
					for( int it = 0; it < m_items.GetSize(); it++ ) {
						CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
						if( pControl == pStopControl ) return false;
						if( !pControl->IsVisible() ) continue;
						if( pControl->GetInterface(_T("MenuElement")) != NULL ) continue;
						if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
						if( pControl->IsFloat() ) {
							if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
							if( !pControl->Paint(renderContext, pStopControl) ) return false;
						}
					}
				}
				else {
					CRenderClip childClip;
					CRenderClip::GenerateClip(renderContext, rcTemp, childClip);
					for( int it = 0; it < m_items.GetSize(); it++ ) {
						CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
						if( pControl == pStopControl ) return false;
						if( !pControl->IsVisible() ) continue;
						if( pControl->GetInterface(_T("MenuElement")) != NULL ) continue;
						if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
						if( pControl->IsFloat() ) {
							if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
							CRenderClip::UseOldClipBegin(renderContext, childClip);
							if( !pControl->Paint(renderContext, pStopControl) ) {
								CRenderClip::UseOldClipEnd(renderContext, childClip);
								return false;
							}
							CRenderClip::UseOldClipEnd(renderContext, childClip);
						}
						else {
							if( !::IntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) continue;
							if( !pControl->Paint(renderContext, pStopControl) ) return false;
						}
					}
				}
			}
		}

		if( m_pVerticalScrollBar != NULL ) {
			if( m_pVerticalScrollBar == pStopControl ) return false;
			if (m_pVerticalScrollBar->IsVisible()) {
				if( ::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos()) ) {
					if( !m_pVerticalScrollBar->Paint(renderContext, pStopControl) ) return false;
				}
			}
		}

		if( m_pHorizontalScrollBar != NULL ) {
			if( m_pHorizontalScrollBar == pStopControl ) return false;
			if (m_pHorizontalScrollBar->IsVisible()) {
				if( ::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos()) ) {
					if( !m_pHorizontalScrollBar->Paint(renderContext, pStopControl) ) return false;
				}
			}
		}
		return true;
	}

	void CMenuElementUI::DrawItemIcon(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		if (!m_strIcon.empty() && !(m_bCheckItem && !GetChecked())) {
			if (m_pOwner == NULL) return;
			SIZE cxyFixed = GetFixedSize();
			SIZE szIconSize = GetIconSize();
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
			RECT rcDest =
			{
				(rcTextPadding.left - szIconSize.cx) / 2,
				(cxyFixed.cy - szIconSize.cy) / 2,
				(rcTextPadding.left - szIconSize.cx) / 2 + szIconSize.cx,
				(cxyFixed.cy - szIconSize.cy) / 2 + szIconSize.cy
			};
			std::wstring pStrImage;
			pStrImage = StringUtil::Format(L"dest='{},{},{},{}'", rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);
			DrawImage(renderContext, m_strIcon, pStrImage);
		}
	}

	void CMenuElementUI::DrawItemExpland(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		if (m_bShowExplandIcon) {
			std::wstring strExplandIcon;
			strExplandIcon = GetManager()->GetDefaultAttributeList(L"ExplandIcon");
			if (strExplandIcon.empty()) {
				return;
			}
			SIZE cxyFixed = GetManager()->ScaleSize(m_cxyFixed);
			int padding = GetManager()->ScaleValue(ITEM_DEFAULT_EXPLAND_ICON_WIDTH) / 3;
			const TDrawInfo* pDrawInfo = GetManager()->GetDrawInfo(strExplandIcon);
			const TImageInfo *pImageInfo = GetManager()->GetImageEx(pDrawInfo->sImageName, NULL, 0, false, pDrawInfo->bGdiplus);
			if (!pImageInfo) {
				return;
			}
			RECT rcDest =
			{
				cxyFixed.cx - pImageInfo->nX - padding,
				(cxyFixed.cy - pImageInfo->nY) / 2,
				cxyFixed.cx - pImageInfo->nX - padding + pImageInfo->nX,
				(cxyFixed.cy - pImageInfo->nY) / 2 + pImageInfo->nY
			};
			rcDest = PixelsToLogical(rcDest);
			std::wstring pStrImage;
			pStrImage = StringUtil::Format(L"dest='{},{},{},{}'", rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);
			DrawImage(renderContext, strExplandIcon, pStrImage);
		}
	}

	void CMenuElementUI::DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem)
	{
		std::wstring sText = GetText();
		if( sText.empty() ) return;

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
		int nLinks = 0;
		RECT rcText = rcItem;
		RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
		rcText.left += rcTextPadding.left;
		rcText.right -= rcTextPadding.right;
		rcText.top += rcTextPadding.top;
		rcText.bottom -= rcTextPadding.bottom;

		if( pInfo->bShowHtml )
			CRenderEngine::DrawHtmlText(renderContext, rcText, sText, iTextColor, NULL, NULL, nLinks, pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
		else
		CRenderEngine::DrawText(renderContext, rcText, sText, iTextColor, pInfo->nFont, DT_SINGLELINE | pInfo->uTextStyle);
	}

	SIZE CMenuElementUI::EstimateSize(SIZE szAvailable)
	{
		SIZE cxyFixed = GetManager()->ScaleSize(m_cxyFixed);
		if (m_pOwner == NULL) {
			return cxyFixed;
		}
		SIZE cXY = {0};
		for( int it = 0; it < GetCount(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(GetItemAt(it));
			if( !pControl->IsVisible() ) continue;
			SIZE sz = pControl->EstimateSize(szAvailable);
			cXY.cy += sz.cy;
			if( cXY.cx < sz.cx ) cXY.cx = sz.cx;
		}
		if(cXY.cy == 0) {
			std::wstring sText = GetText();
			TListInfoUI* pInfo = m_pOwner->GetListInfo();
			DWORD iTextColor = pInfo->dwTextColor;
			RECT rcText = { 0, 0, MAX(szAvailable.cx, m_cxyFixed.cx), 9999999 };
			RECT rcTextPadding = GetManager()->ScaleRect(pInfo->rcTextPadding);
			rcText.left += rcTextPadding.left;
			rcText.right -= rcTextPadding.right;
			CPaintRenderContext measureContext = m_pManager->CreateMeasureRenderContext(rcText);
			if( pInfo->bShowHtml ) {   
				int nLinks = 0;
				CRenderEngine::DrawHtmlText(measureContext, rcText, sText, iTextColor, NULL, NULL, nLinks, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle);
			}
			else {
		CRenderEngine::DrawText(measureContext, rcText, sText, iTextColor, pInfo->nFont, DT_CALCRECT | pInfo->uTextStyle);
			}
			cXY.cx = rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right ;
			cXY.cy = rcText.bottom - rcText.top + rcTextPadding.top + rcTextPadding.bottom;
		}

		if( cxyFixed.cy != 0 ) cXY.cy = cxyFixed.cy;
		if ( cXY.cx < cxyFixed.cx )
			cXY.cx =  cxyFixed.cx;

		return cXY;
	}

	void CMenuElementUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			CListContainerElementUI::DoEvent(event);
			if( m_pWindow ) return;
			bool hasSubMenu = false;
			for( int i = 0; i < GetCount(); ++i )
			{
				if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL )
				{
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

					hasSubMenu = true;
				}
			}
			if( hasSubMenu )
			{
				if (m_pOwner == NULL) return;
				m_pOwner->SelectItem(GetIndex(), true);
				CreateMenuWnd();
			}
			else
			{
				ContextMenuParam param;
				param.hWnd = m_pManager->GetPaintWindow();
				param.wParam = 2;
				CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
				if (m_pOwner == NULL) return;
				m_pOwner->SelectItem(GetIndex(), true);
			}
			return;
		}


		if (event.Type == UIEVENT_MOUSELEAVE) {

			bool hasSubMenu = false;
			for (int i = 0; i < GetCount(); ++i)
			{
				if (GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL)
				{

					hasSubMenu = true;
				}
			}

			if (!hasSubMenu) {
				if (m_pOwner == NULL) return;
				m_pOwner->SelectItem(-1, true);
			}
		}

		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( IsEnabled() ){
				CListContainerElementUI::DoEvent(event);

				if( m_pWindow ) return;

				bool hasSubMenu = false;
				for( int i = 0; i < GetCount(); ++i ) {
					if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL ) {
						(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
						(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);

						hasSubMenu = true;
					}
				}
				if( hasSubMenu )
				{
					CreateMenuWnd();
				}
				else
				{
					SetChecked(!GetChecked());

					bool isClosing = false;
					CMenuUI* menuUI=static_cast<CMenuUI*>(GetManager()->GetRoot());
					isClosing = (menuUI->m_pWindow->isClosing);
					if (IsWindow(GetManager()->GetPaintWindow()) && !isClosing) {
						if (CMenuWnd::GetGlobalContextMenuObserver().GetManager() != NULL) {
							MenuCmd* pMenuCmd = new MenuCmd();
							lstrcpy(pMenuCmd->szName, GetName().c_str());
							lstrcpy(pMenuCmd->szUserData, GetUserData().c_str());
							lstrcpy(pMenuCmd->szText, GetText().c_str());
							pMenuCmd->bChecked = GetChecked();
							if (!PostMessage(CMenuWnd::GetGlobalContextMenuObserver().GetManager()->GetPaintWindow(), WM_MENUCLICK, (WPARAM)pMenuCmd, (LPARAM)this)) {
								delete pMenuCmd;
								pMenuCmd = NULL;
							}
						}
					}
					ContextMenuParam param;
					param.hWnd = m_pManager->GetPaintWindow();
					param.wParam = 1;
					CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
				}
			}

			return;
		}

		if ( event.Type == UIEVENT_KEYDOWN && event.chKey == VK_RIGHT )
		{
			if( m_pWindow ) return;
			bool hasSubMenu = false;
			for( int i = 0; i < GetCount(); ++i ) {
				if( GetItemAt(i)->GetInterface(_T("MenuElement")) != NULL ) {
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetVisible(true);
					(static_cast<CMenuElementUI*>(GetItemAt(i)->GetInterface(_T("MenuElement"))))->SetInternVisible(true);
					hasSubMenu = true;
				}
			}
			if( hasSubMenu ) {
				if (m_pOwner == NULL) return;
				m_pOwner->SelectItem(GetIndex(), true);
				CreateMenuWnd();
			}
			else
			{
				ContextMenuParam param;
				param.hWnd = m_pManager->GetPaintWindow();
				param.wParam = 2;
				CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
				if (m_pOwner == NULL) return;
				m_pOwner->SelectItem(GetIndex(), true);
			}

			return;
		}

		CListContainerElementUI::DoEvent(event);
	}

	CMenuWnd* CMenuElementUI::GetMenuWnd()
	{
		return m_pWindow;
	}

	void CMenuElementUI::CreateMenuWnd()
	{
		if( m_pWindow ) return;

		m_pWindow = new CMenuWnd();
		ASSERT(m_pWindow);

		ContextMenuParam param;
		param.hWnd = m_pManager->GetPaintWindow();
		param.wParam = 2;
		CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(param);
		m_pWindow->m_nSpacing = m_nSpacing;
		m_pWindow->Init(static_cast<CMenuElementUI*>(this), _T(""), CDuiPoint(), NULL);
	}

	void CMenuElementUI::SetLineType()
	{
		m_bDrawLine = true;
		if (m_cxyFixed.cy == 0 || m_cxyFixed.cy == ITEM_DEFAULT_HEIGHT)
			SetFixedHeight(DEFAULT_LINE_HEIGHT);

		SetMouseChildEnabled(false);
		SetMouseEnabled(false);
		SetEnabled(false);
	}

	void CMenuElementUI::SetLineColor(DWORD color)
	{
		m_dwLineColor = color;
	}

	DWORD CMenuElementUI::GetLineColor() const
	{
		return m_dwLineColor;
	}
	void CMenuElementUI::SetLinePadding(RECT rcPadding)
	{
		m_rcLinePadding = rcPadding;
	}

	RECT CMenuElementUI::GetLinePadding() const
	{
		RECT rcLinePadding = m_rcLinePadding;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcLinePadding);
		return rcLinePadding;
	}

	void CMenuElementUI::SetIcon(std::wstring_view strIcon)
	{
		m_strIcon = strIcon;
	}

	void CMenuElementUI::SetIconSize(LONG cx, LONG cy)
	{
		m_szIconSize.cx = cx;
		m_szIconSize.cy = cy;
	}

	SIZE CMenuElementUI::GetIconSize()
	{
		SIZE szIconSize = m_szIconSize;
		if(m_pManager != NULL) m_pManager->ScaleSize(&szIconSize);
		return szIconSize;
	}


	void CMenuElementUI::SetChecked(bool bCheck/* = true*/)
	{
		SetItemInfo(GetName(), bCheck);
	}

	bool CMenuElementUI::GetChecked() const
	{
		const std::wstring_view pstrName = GetNameView();
		if (pstrName.empty()) return false;

		MenuCheckInfoMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			const auto it = mCheckInfos->find(pstrName);
			if (it != mCheckInfos->end() && it->second != NULL) {
				return it->second->bChecked;
			}
		}
		return false;

	}

	void CMenuElementUI::SetCheckItem(bool bCheckItem/* = false*/)
	{
		m_bCheckItem = bCheckItem;
	}

	bool CMenuElementUI::GetCheckItem() const
	{
		return m_bCheckItem;
	}

	void CMenuElementUI::SetShowExplandIcon(bool bShow)
	{
		m_bShowExplandIcon = bShow;
	}

	void FYUI::CMenuElementUI::SetSubMenuSpacing(int nSpacing)
	{
		m_nSpacing = nSpacing;
	}

	void CMenuElementUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
	{
		const std::wstring_view name = StringUtil::TrimView(pstrNameView);
		if (name.empty()) {
			return;
		}

		if (StringUtil::EqualsNoCase(name, L"icon")) {
			SetIcon(pstrValueView);
		}
		else if (StringUtil::EqualsNoCase(name, L"iconsize")) {
			SIZE size = { 0, 0 };
			if (StringUtil::TryParseSize(pstrValueView, size)) {
				SetIconSize(size.cx, size.cy);
			}
		}
		else if (StringUtil::EqualsNoCase(name, L"checkitem")) {
			SetCheckItem(StringUtil::ParseBool(pstrValueView));		
		}
		else if (StringUtil::EqualsNoCase(name, L"ischeck")) {		
			MenuCheckInfoMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
			if (mCheckInfos != NULL)
			{
				bool bFind = false;
				for (const auto& entry : *mCheckInfos) {
					MenuItemInfo* itemInfo = entry.second;
					if (itemInfo != NULL && lstrcmpi(itemInfo->szName, GetName().c_str()) == 0) {
						bFind = true;
						break;
					}
				}
				if (!bFind) SetChecked(StringUtil::ParseBool(pstrValueView));
			}
		}	
		else if (StringUtil::EqualsNoCase(name, L"linetype")) {
			if (StringUtil::ParseBool(pstrValueView))
				SetLineType();
		}
		else if (StringUtil::EqualsNoCase(name, L"expland")) {
			SetShowExplandIcon(StringUtil::ParseBool(pstrValueView));
		}
		else if (StringUtil::EqualsNoCase(name, L"linecolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) {
				SetLineColor(color);
			}
		}
		else if (StringUtil::EqualsNoCase(name, L"linepadding")) {
			RECT inset = { 0 };
			if (StringUtil::TryParseRect(pstrValueView, inset)) {
				SetLinePadding(inset);
			}
		}
		else if (StringUtil::EqualsNoCase(name, L"height")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetFixedHeight(value);
		}
		else if (StringUtil::EqualsNoCase(name, L"childwndpacing")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetSubMenuSpacing(value);
		}
		else
			CListContainerElementUI::SetAttribute(pstrNameView, pstrValueView);
	}


	MenuItemInfo* CMenuElementUI::GetItemInfo(std::wstring_view pstrName)
	{
		if(pstrName.empty()) return NULL;

		MenuCheckInfoMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			const auto it = mCheckInfos->find(pstrName);
			if (it != mCheckInfos->end()) {
				return it->second;
			}
		}

		return NULL;
	}

	MenuItemInfo* CMenuElementUI::SetItemInfo(std::wstring_view pstrName, bool bChecked)
	{
		if(pstrName.empty()) return NULL;

		MenuCheckInfoMap* mCheckInfos = CMenuWnd::GetGlobalContextMenuObserver().GetMenuCheckInfo();
		if (mCheckInfos != NULL)
		{
			auto it = mCheckInfos->find(pstrName);
			if (it == mCheckInfos->end()) {
				MenuItemInfo* pItemInfo = new MenuItemInfo;
				if (pItemInfo == NULL) return NULL;
				const std::wstring name(pstrName);
				lstrcpy(pItemInfo->szName, name.c_str());
				pItemInfo->bChecked = bChecked;
				mCheckInfos->emplace(pstrName, pItemInfo);
				return pItemInfo;
			}
			it->second->bChecked = bChecked;
			return it->second;
		}
		return NULL;
	}
} // namespace DuiLib


