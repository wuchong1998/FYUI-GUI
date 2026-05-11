#include "pch.h"
#include "UICombo.h"
#include "../Core/Render/UIRenderContext.h"

namespace FYUI {
	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	class CComboWnd : public CWindowWnd, public INotifyUI
	{
	public:
		void Init(CComboUI* pOwner);
		std::wstring_view GetWindowClassName() const override;
		void OnFinalMessage(HWND hWnd);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void Notify(TNotifyUI& msg) override;

		void EnsureVisible(int iIndex);
		void Scroll(int dx, int dy);

#if(_WIN32_WINNT >= 0x0501)
		virtual UINT GetClassStyle() const;
#endif
		bool IsHitItem(POINT ptMouse);

		// 推进一次滑动动画帧（展开或收起，由 m_bClosing 决定方向）；返回 true 表示动画已完成。
		bool TickExpandAnimation();
		// 启动收起动画：把 m_rcFinal 重新指派为"当前窗口位置"，保证收起从当前可见状态开始。
		void StartCollapseAnimation();
	public:
		CPaintManagerUI m_pm;
		CComboUI* m_pOwner;
		CVerticalLayoutUI* m_pLayout;
		int m_iOldSel;
		bool m_bHitItem;

		// === 滑动动画状态 ===
		// 动画结束时的目标矩形（屏幕坐标）
		RECT m_rcFinal{};
		// 动画起始矩形：高度=0，根据展开方向贴在最终矩形的顶/底
		RECT m_rcStart{};
		// 实际展开方向：true=向下展开，false=向上展开（综合 owner 偏好与屏幕空间检查后）
		bool m_bExpandDown{ true };
		// 当前是否处于"收起动画"阶段（用于让 TickExpandAnimation 反向插值，并拦截重入关闭）
		bool m_bClosing{ false };
		// 动画总时长（毫秒），<=0 表示无动画
		int  m_nAnimDurationMs{ 0 };
		// 动画起始 tick（GetTickCount）
		DWORD m_dwAnimStart{ 0 };
		// 动画 timer 的 ID（0 表示当前没有 timer 在跑）
		UINT_PTR m_idAnimTimer{ 0 };
	};

	// 滑动展开动画专用 timer id（避免与 CPaintManagerUI 内部使用的 timer 冲突）
	static constexpr UINT_PTR kComboExpandTimerId = 0xC0B0;

	void CComboWnd::Notify(TNotifyUI& msg)
	{
		if (msg.sType == _T("windowinit"))
		{
			EnsureVisible(m_iOldSel);
		}
		else if(msg.sType == _T("click")) {
			// 濞村鐦禒锝囩垳
			std::wstring sName = msg.pSender->GetName();
			CControlUI* pCtrl = msg.pSender;
			while(pCtrl != NULL) {
				IListItemUI* pListItem = (IListItemUI*)pCtrl->GetInterface(DUI_CTR_LISTITEM);
				if(pListItem != NULL ) {
					break;
				}
				pCtrl = pCtrl->GetParent();
			}
			if( m_pOwner->GetManager() != NULL ) m_pOwner->GetManager()->SendNotify(msg.pSender, DUI_MSGTYPE_CLICK, 0, 0);
		}
	}

	void CComboWnd::Init(CComboUI* pOwner)
	{
		m_bHitItem = false;
		m_pOwner = pOwner;
		m_pLayout = NULL;
		m_iOldSel = m_pOwner->GetCurSel();

		// Position the popup window in absolute space
		SIZE szDrop = m_pOwner->GetDropBoxSize();
		RECT rcInset = m_pOwner->GetDropBoxInset();

		const RECT rcOwner = pOwner->GetPos();

		// 估算下拉内容总高度（含 inset），用于自动尺寸 / 反向计算。
		SIZE szAvailableMeasure = { rcOwner.right - rcOwner.left, szDrop.cy };
		if (szDrop.cx > 0) szAvailableMeasure.cx = szDrop.cx;
		int cyFixed = rcInset.top;
		for (int it = 0; it < pOwner->GetCount(); it++) {
			CControlUI* pControl = static_cast<CControlUI*>(pOwner->GetItemAt(it));
			if (!pControl->IsVisible()) continue;
			SIZE sz = pControl->EstimateSize(szAvailableMeasure);
			cyFixed += sz.cy;
		}
		cyFixed += 4;
		const int cyContent = m_pOwner->m_bIsAutoDropBoxSize ? MIN(cyFixed, szDrop.cy) : szDrop.cy;

		const RECT rcDropPadding = m_pOwner->GetDropBoxPadding();

		// 构造一个候选 rect：基于 rcOwner 在 client 坐标，再追加 padding。
		auto buildCandidate = [&](bool bDownward) {
			RECT rc = rcOwner;
			if (bDownward) {
				rc.top = rcOwner.bottom;
				rc.bottom = rc.top + cyContent;
			}
			else {
				rc.bottom = rcOwner.top;
				rc.top = rc.bottom - cyContent;
			}
			if (szDrop.cx > 0) rc.right = rc.left + szDrop.cx;
			rc.left += rcDropPadding.left;
			rc.right += rcDropPadding.left;
			rc.top += rcDropPadding.top;
			rc.bottom += rcDropPadding.top;
			::MapWindowRect(pOwner->GetManager()->GetPaintWindow(), HWND_DESKTOP, &rc);
			return rc;
		};

		// 按用户偏好生成首选方向 rect；若超出屏幕工作区且反向能放下，则反转。
		bool bDownward = m_pOwner->m_bExpansionDirection;
		RECT rcCandidate = buildCandidate(bDownward);

		MONITORINFO oMonitor = {};
		oMonitor.cbSize = sizeof(oMonitor);
		::GetMonitorInfo(::MonitorFromRect(&rcCandidate, MONITOR_DEFAULTTONEAREST), &oMonitor);
		const CDuiRect rcWork = oMonitor.rcWork;
		if (bDownward && rcCandidate.bottom > rcWork.bottom) {
			RECT rcAlt = buildCandidate(false);
			if (rcAlt.top >= rcWork.top) { bDownward = false; rcCandidate = rcAlt; }
		}
		else if (!bDownward && rcCandidate.top < rcWork.top) {
			RECT rcAlt = buildCandidate(true);
			if (rcAlt.bottom <= rcWork.bottom) { bDownward = true; rcCandidate = rcAlt; }
		}

		m_rcFinal = rcCandidate;
		m_bExpandDown = bDownward;
		m_nAnimDurationMs = m_pOwner->m_nExpansionAnimDuration;

		// 起始矩形：高度=0，贴在最终矩形的顶（向下展开）或底（向上展开）。
		m_rcStart = m_rcFinal;
		if (m_bExpandDown) m_rcStart.bottom = m_rcStart.top;
		else               m_rcStart.top = m_rcStart.bottom;

		const RECT rcCreate = (m_nAnimDurationMs > 0) ? m_rcStart : m_rcFinal;

		Create(pOwner->GetManager()->GetPaintWindow(), {}, WS_POPUP, WS_EX_TOOLWINDOW, rcCreate);
		// HACK: Don't deselect the parent's caption
		HWND hWndParent = m_hWnd;
		while (::GetParent(hWndParent) != NULL) hWndParent = ::GetParent(hWndParent);
		::ShowWindow(m_hWnd, SW_SHOW);
		::SendMessage(hWndParent, WM_NCACTIVATE, TRUE, 0L);

		// 启动滑动展开动画。
		if (m_nAnimDurationMs > 0) {
			m_dwAnimStart = ::GetTickCount();
			m_idAnimTimer = ::SetTimer(m_hWnd, kComboExpandTimerId, 16, NULL);
		}
	}

	std::wstring_view CComboWnd::GetWindowClassName() const
	{
		return _T("ComboWnd");
	}

	void CComboWnd::OnFinalMessage(HWND hWnd)
	{
		// 销毁前确保动画 timer 被清理，避免遗留 timer 投递到已销毁窗口。
		if (m_idAnimTimer != 0) {
			::KillTimer(hWnd, m_idAnimTimer);
			m_idAnimTimer = 0;
		}
		m_pOwner->m_pWindow = NULL;
		m_pOwner->m_uButtonState &= ~ UISTATE_PUSHED;
		m_pOwner->Invalidate();
		delete this;
	}

	bool CComboWnd::TickExpandAnimation()
	{
		if (m_nAnimDurationMs <= 0) return true;
		const DWORD now = ::GetTickCount();
		const DWORD elapsed = now - m_dwAnimStart;
		double t = static_cast<double>(elapsed) / static_cast<double>(m_nAnimDurationMs);
		bool bDone = false;
		if (t >= 1.0) { t = 1.0; bDone = true; }
		// 展开：ease-out cubic（起步快、收尾慢，"飞出来"的感觉）
		// 收起：ease-in cubic（起步慢、收尾快，"被吸回去"的感觉），与展开视觉对称
		double eased;
		if (m_bClosing) {
			eased = t * t * t;
		}
		else {
			const double inv = 1.0 - t;
			eased = 1.0 - inv * inv * inv;
		}
		// 收起时方向反过来：从当前 m_rcFinal（启动收起时已经被改写为"当前位置"）滑向 m_rcStart
		const RECT& from = m_bClosing ? m_rcFinal : m_rcStart;
		const RECT& to   = m_bClosing ? m_rcStart : m_rcFinal;
		RECT rc;
		rc.left = m_rcFinal.left;
		rc.right = m_rcFinal.right;
		rc.top    = from.top    + static_cast<LONG>((to.top    - from.top   ) * eased);
		rc.bottom = from.bottom + static_cast<LONG>((to.bottom - from.bottom) * eased);
		::SetWindowPos(m_hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
			SWP_NOZORDER | SWP_NOACTIVATE);
		return bDone;
	}

	void CComboWnd::StartCollapseAnimation()
	{
		// 若展开 timer 还在跑（动画途中被关闭），先停掉，并把"当前窗口位置"作为收起动画的起点，
		// 避免视觉跳到完全展开后再收起。
		if (m_idAnimTimer != 0) {
			::KillTimer(m_hWnd, m_idAnimTimer);
			m_idAnimTimer = 0;
		}
		RECT rcNow = {};
		::GetWindowRect(m_hWnd, &rcNow);
		m_rcFinal = rcNow;                // 收起动画起点（屏幕坐标）
		// m_rcStart 维持创建时的"折叠态"，作为收起动画终点
		m_bClosing = true;
		m_dwAnimStart = ::GetTickCount();
		m_idAnimTimer = ::SetTimer(m_hWnd, kComboExpandTimerId, 16, NULL);
	}

	bool CComboWnd::IsHitItem(POINT ptMouse)
	{
		CControlUI* pControl = m_pm.FindControl(ptMouse);
		if(pControl != NULL) {
			LPVOID pInterface = pControl->GetInterface(DUI_CTR_SCROLLBAR);
			if(pInterface) return false;

			while(pControl != NULL) {
				IListItemUI* pListItem = (IListItemUI*)pControl->GetInterface(DUI_CTR_LISTITEM);
				if(pListItem != NULL ) {
					return true;
				}
				pControl = pControl->GetParent();
			}
		}

		return false;
	}

	LRESULT CComboWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if( uMsg == WM_CREATE ) {
			m_pm.SetForceUseSharedRes(true);
			m_pm.Init(m_hWnd);
			if (m_pOwner->GetManager() != NULL) {
				m_pm.SyncDPIFrom(m_pOwner->GetManager());
				m_pm.SetRenderBackend(m_pOwner->GetManager()->GetRenderBackend());
				m_pm.SetDirect2DRenderMode(m_pOwner->GetManager()->GetDirect2DRenderMode());
				m_pm.SetRenderDiagnosticsEnabled(
					m_pOwner->GetManager()->IsRenderDiagnosticsEnabled(),
					m_pOwner->GetManager()->IsRenderDiagnosticsDebugTraceEnabled());
			}
			m_pm.SetLayered(true);
			// The trick is to add the items to the new container. Their owner gets
			// reassigned by this operation - which is why it is important to reassign
			// the items back to the righfull owner/manager when the window closes.
			m_pLayout = new CVerticalLayoutUI;
			m_pLayout->SetManager(&m_pm, NULL, true);
			const std::wstring_view pDefaultAttributes = m_pOwner->GetManager()->GetDefaultAttributeList(L"VerticalLayout");
			if( !pDefaultAttributes.empty() ) {
				m_pLayout->ApplyAttributeList(pDefaultAttributes);
			}
			m_pLayout->SetInset(CDuiRect(1, 1, 1, 1));
			m_pLayout->SetBkColor(m_pOwner->GetDropBoxColor());
			m_pLayout->SetBorderColor(0xFFC6C7D2);
			m_pLayout->SetBorderSize(1);
			m_pLayout->SetAutoDestroy(false);
			m_pLayout->EnableScrollBar();
			m_pLayout->ApplyAttributeList(m_pOwner->GetDropBoxAttributeList());
			for( int i = 0; i < m_pOwner->GetCount(); i++ ) {
				m_pLayout->Add(static_cast<CControlUI*>(m_pOwner->GetItemAt(i)));
			}
			CShadowUI *pShadow = m_pOwner->GetManager()->GetShadow();
			pShadow->CopyShadow(m_pm.GetShadow());
			m_pm.GetShadow()->ShowShadow(m_pOwner->IsShowShadow());
			m_pm.AttachDialog(m_pLayout);
			m_pm.AddNotifier(this);
			return 0;
		}
		else if( uMsg == WM_CLOSE ) {
			// 首次收到关闭请求且启用动画时，拦截 WM_CLOSE 启动收起动画；
			// 动画结束后会再次 PostMessage(WM_CLOSE)，那时 m_bClosing==true 走真销毁分支。
			if (!m_bClosing && m_nAnimDurationMs > 0) {
				StartCollapseAnimation();
				return 0;
			}
			m_pOwner->SetManager(m_pOwner->GetManager(), m_pOwner->GetParent(), false);
			RECT rcNull = { 0 };
			for( int i = 0; i < m_pOwner->GetCount(); i++ ) static_cast<CControlUI*>(m_pOwner->GetItemAt(i))->SetPos(rcNull);
			m_pOwner->SetFocus();
		}
		else if( uMsg == WM_LBUTTONDOWN ) {
			POINT pt = { 0 };
			::GetCursorPos(&pt);
			::ScreenToClient(m_pm.GetPaintWindow(), &pt);
			m_bHitItem = IsHitItem(pt);
		}
		else if( uMsg == WM_LBUTTONUP ) {
			POINT pt = { 0 };
			::GetCursorPos(&pt);
			::ScreenToClient(m_pm.GetPaintWindow(), &pt);
			// 收起动画期间忽略 item 命中关闭，避免重入。
			if(!m_bClosing && m_bHitItem && IsHitItem(pt)) {
				PostMessage(WM_KILLFOCUS);
			}
			m_bHitItem = false;
		}
		else if( uMsg == WM_KEYDOWN )
		{
			// 收起动画期间不再响应键盘关闭/导航，避免重入。
			if (m_bClosing) return 0;
			switch( wParam ) {
			case VK_ESCAPE:
				m_pOwner->SelectItem(m_iOldSel, true);
				EnsureVisible(m_iOldSel);
			case VK_RETURN:
				PostMessage(WM_KILLFOCUS);
				break;
			default:
				TEventUI event;
				event.Type = UIEVENT_KEYDOWN;
				event.chKey = (wchar_t)wParam;
				m_pOwner->DoEvent(event);
				EnsureVisible(m_pOwner->GetCurSel());
				return 0;
			}
		}
		else if( uMsg == WM_MOUSEWHEEL ) {
			int zDelta = (int) (short) HIWORD(wParam);
			TEventUI event = { 0 };
			event.Type = UIEVENT_SCROLLWHEEL;
			event.wParam = MAKELPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, 0);
			event.lParam = lParam;
			event.dwTimestamp = ::GetTickCount();
			if(m_pOwner->GetScrollSelect()) {
				m_pOwner->DoEvent(event);
				EnsureVisible(m_pOwner->GetCurSel());
				return 0;
			}
			else {
				m_pLayout->DoEvent(event);
				return 0;
			}
		}
		else if( uMsg == WM_KILLFOCUS ) {
			// 收起动画期间忽略后续 KILLFOCUS，避免重复触发 WM_CLOSE 打断收起动画。
			if( !m_bClosing && m_hWnd != (HWND) wParam ) PostMessage(WM_CLOSE);
		}
		else if (uMsg == WM_TIMER && wParam == kComboExpandTimerId) {
			// 滑动动画帧推进：m_bClosing==false 时是展开（ease-out cubic）；
			// m_bClosing==true 时是收起（ease-in cubic）。收起完成后 PostMessage(WM_CLOSE) 触发真正销毁。
			if (TickExpandAnimation()) {
				if (m_idAnimTimer != 0) {
					::KillTimer(m_hWnd, m_idAnimTimer);
					m_idAnimTimer = 0;
				}
				if (m_bClosing) {
					PostMessage(WM_CLOSE);
				}
			}
			return 0;
		}

		LRESULT lRes = 0;
		if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	void CComboWnd::EnsureVisible(int iIndex)
	{
		if( m_pOwner->GetCurSel() < 0 ) return;
		m_pLayout->FindSelectable(m_pOwner->GetCurSel(), false);
		RECT rcItem = m_pLayout->GetItemAt(iIndex)->GetPos();
		RECT rcList = m_pLayout->GetPos();
		CScrollBarUI* pHorizontalScrollBar = m_pLayout->GetHorizontalScrollBar();
		if( pHorizontalScrollBar && pHorizontalScrollBar->IsVisible() ) rcList.bottom -= pHorizontalScrollBar->GetFixedHeight();
		int iPos = m_pLayout->GetScrollPos().cy;
		if( rcItem.top >= rcList.top && rcItem.bottom < rcList.bottom ) return;
		int dx = 0;
		if( rcItem.top < rcList.top ) dx = rcItem.top - rcList.top;
		if( rcItem.bottom > rcList.bottom ) dx = rcItem.bottom - rcList.bottom;
		Scroll(0, dx);
	}

	void CComboWnd::Scroll(int dx, int dy)
	{
		if( dx == 0 && dy == 0 ) return;
		SIZE sz = m_pLayout->GetScrollPos();
		m_pLayout->SetScrollPos(CDuiSize(sz.cx + dx, sz.cy + dy));
	}

#if(_WIN32_WINNT >= 0x0501)
	UINT CComboWnd::GetClassStyle() const
	{
		return __super::GetClassStyle();
		if(m_pOwner->IsShowShadow()) {
			return __super::GetClassStyle();

		}
		else {
			return __super::GetClassStyle() | CS_DROPSHADOW;
		}
	}
#endif
	////////////////////////////////////////////////////////
	IMPLEMENT_DUICONTROL(CComboUI)

		CComboUI::CComboUI() : m_uTextStyle(DT_VCENTER | DT_SINGLELINE)
		, m_dwTextColor(0)
		, m_dwDisabledTextColor(0)
		, m_iFont(-1)
		, m_bShowHtml(false)
		, m_pWindow(NULL)
		, m_iCurSel(-1)
		, m_uButtonState(0)
		, m_bScrollSelect(true)
		, m_bShowShadow(false)
		, m_bShowSelectedItemText(true)
		, m_bIsAutoDropBoxSize(true)
		, m_bExpansionDirection(true)
		, m_nExpansionAnimDuration(180)
	{
		m_szDropBox = CDuiSize(0, 150);
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
		::ZeroMemory(&m_rcDropBox, sizeof(m_rcDropBox));

		m_rcDropBoxPadding = {0,0,0,0};
		m_dwDropBoxBkcolor = 0xFFFFFFFF;
		m_ListInfo.nColumns = 0;
		m_ListInfo.nFont = -1;
		m_ListInfo.uTextStyle = DT_VCENTER;
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
		m_ListInfo.bShowHtml = false;
		m_ListInfo.bMultiExpandable = false;
		::ZeroMemory(&m_ListInfo.rcTextPadding, sizeof(m_ListInfo.rcTextPadding));
		::ZeroMemory(&m_ListInfo.rcColumn, sizeof(m_ListInfo.rcColumn));

		m_pCompareFunc = NULL;
		m_compareData = NULL;
	}

	std::wstring_view CComboUI::GetClass() const
	{
		return _T("ComboUI");
	}

	LPVOID CComboUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, DUI_CTR_COMBO) == 0 ) return static_cast<CComboUI*>(this);
		if( StringUtil::CompareNoCase(pstrName, _T("IListOwner")) == 0 ) return static_cast<IListOwnerUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CComboUI::GetControlFlags() const
	{
		return UIFLAG_TABSTOP | UIFLAG_SETCURSOR;
	}

	void CComboUI::DoInit()
	{
	}

	UINT CComboUI::GetListType()
	{
		return LT_COMBO;
	}

	TListInfoUI* CComboUI::GetListInfo()
	{
		return &m_ListInfo;
	}

	int CComboUI::GetCurSel() const
	{
		return m_iCurSel;
	}
	void CComboUI::InitCursel()
	{

		m_iCurSel = -1;
	}

	bool CComboUI::SelectItem(int iIndex, bool bTakeFocus, bool bIsClick, bool bSetScrollPos)
	{
		if( iIndex == m_iCurSel ) return true;
		int iOldSel = m_iCurSel;
		if( m_iCurSel >= 0 ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
			if( !pControl ) return false;
			IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) pListItem->Select(false);
			m_iCurSel = -1;
		}
		if( iIndex < 0 ) return false;
		if( m_items.GetSize() == 0 ) return false;
		if( iIndex >= m_items.GetSize() ) iIndex = m_items.GetSize() - 1;
		CControlUI* pControl = static_cast<CControlUI*>(m_items[iIndex]);
		if( !pControl || !pControl->IsEnabled() ) return false;
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem == NULL ) return false;
		m_iCurSel = iIndex;
		if( m_pWindow != NULL || bTakeFocus ) pControl->SetFocus();
		pListItem->Select(true);
		if (bIsClick)
		{
			if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_COMBO_ITEMSELECT, m_iCurSel, iOldSel);
		}
		else
		{
			if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, m_iCurSel, iOldSel);
		}

		Invalidate();

		return true;
	}

	void CComboUI::SendNotifyClick()
	{
		if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, m_iCurSel, 0);
	}

	bool CComboUI::SelectMultiItem(int iIndex, bool bTakeFocus)
	{
		return SelectItem(iIndex, bTakeFocus);
	}

	bool CComboUI::UnSelectItem(int iIndex, bool bOthers)
	{
		return false;
	}

	bool CComboUI::SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate)
	{
		int iOrginIndex = GetItemIndex(pControl);
		if( iOrginIndex == -1 ) return false;
		if( iOrginIndex == iIndex ) return true;

		IListItemUI* pSelectedListItem = NULL;
		if( m_iCurSel >= 0 ) pSelectedListItem = 
			static_cast<IListItemUI*>(GetItemAt(m_iCurSel)->GetInterface(_T("ListItem")));
		if( !CContainerUI::SetItemIndex(pControl, iIndex, bUpdate) ) return false;
		int iMinIndex = min(iOrginIndex, iIndex);
		int iMaxIndex = max(iOrginIndex, iIndex);
		for(int i = iMinIndex; i < iMaxIndex + 1; ++i) {
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) {
				pListItem->SetIndex(i);
			}
		}
		if( m_iCurSel >= 0 && pSelectedListItem != NULL ) m_iCurSel = pSelectedListItem->GetIndex();
		return true;
	}

	bool CComboUI::Add(CControlUI* pControl)
	{
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) 
		{
			pListItem->SetOwner(this);
			pListItem->SetIndex(m_items.GetSize());
		}
		return CContainerUI::Add(pControl);
	}

	bool CComboUI::AddAt(CControlUI* pControl, int iIndex)
	{
		if (!CContainerUI::AddAt(pControl, iIndex)) return false;

		// The list items should know about us
		IListItemUI* pListItem = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
		if( pListItem != NULL ) {
			pListItem->SetOwner(this);
			pListItem->SetIndex(iIndex);
		}

		for(int i = iIndex + 1; i < GetCount(); ++i) {
			CControlUI* p = GetItemAt(i);
			pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) {
				pListItem->SetIndex(i);
			}
		}
		if( m_iCurSel >= iIndex ) m_iCurSel += 1;
		return true;
	}

	bool CComboUI::Remove(CControlUI* pControl,bool bChildDelayed)
	{
		int iIndex = GetItemIndex(pControl);
		if (iIndex == -1) return false;

		if (!CContainerUI::RemoveAt(iIndex,bChildDelayed)) return false;

		for(int i = iIndex; i < GetCount(); ++i) {
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) {
				pListItem->SetIndex(i);
			}
		}

		if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
			int iSel = m_iCurSel;
			m_iCurSel = -1;
			SelectItem(FindSelectable(iSel, false));
		}
		else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
		return true;
	}

	bool CComboUI::RemoveAt(int iIndex,bool bChildDelayed)
	{
		if (!CContainerUI::RemoveAt(iIndex,bChildDelayed)) return false;

		for(int i = iIndex; i < GetCount(); ++i) {
			CControlUI* p = GetItemAt(i);
			IListItemUI* pListItem = static_cast<IListItemUI*>(p->GetInterface(_T("ListItem")));
			if( pListItem != NULL ) pListItem->SetIndex(i);
		}

		if( iIndex == m_iCurSel && m_iCurSel >= 0 ) {
			int iSel = m_iCurSel;
			m_iCurSel = -1;
			SelectItem(FindSelectable(iSel, false));
		}
		else if( iIndex < m_iCurSel ) m_iCurSel -= 1;
		return true;
	}

	void CComboUI::RemoveAll(bool bChildDelayed)
	{
		m_iCurSel = -1;
		CContainerUI::RemoveAll(bChildDelayed);
	}

	void CComboUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_BUTTONDOWN )
		{
			if( IsEnabled() ) {
				Activate();
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
			}
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP )
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~ UISTATE_CAPTURED;
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			return;
		}
		if( event.Type == UIEVENT_KEYDOWN )
		{
			if (!IsKeyboardEnabled())
			{
				return;
			}
			switch( event.chKey ) {
			case VK_F4:
				Activate();
				return;
			case VK_UP:
				SelectItem(FindSelectable(m_iCurSel - 1, false));
				return;
			case VK_DOWN:
				SelectItem(FindSelectable(m_iCurSel + 1, true));
				return;
			case VK_PRIOR:
				SelectItem(FindSelectable(m_iCurSel - 1, false));
				return;
			case VK_NEXT:
				SelectItem(FindSelectable(m_iCurSel + 1, true));
				return;
			case VK_HOME:
				SelectItem(FindSelectable(0, false));
				return;
			case VK_END:
				SelectItem(FindSelectable(GetCount() - 1, true));
				return;
			}
		}
		if( event.Type == UIEVENT_SCROLLWHEEL )
		{
			if(GetScrollSelect()) {
				bool bDownward = LOWORD(event.wParam) == SB_LINEDOWN;
				SelectItem(FindSelectable(m_iCurSel + (bDownward ? 1 : -1), bDownward));
			}
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse ) ) {
				if( (m_uButtonState & UISTATE_HOT) == 0  ) {
					m_uButtonState |= UISTATE_HOT;
					Invalidate();
				}
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( (m_uButtonState & UISTATE_HOT) != 0 ) {
				m_uButtonState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		CControlUI::DoEvent(event);
	}

	SIZE CComboUI::EstimateSize(SIZE szAvailable)
	{
		const SIZE fixedSize = GetFixedSize();
		if( fixedSize.cy == 0 ) return CDuiSize(fixedSize.cx, m_pManager->GetDefaultFontInfo()->tm.tmHeight + ScaleValue(12));
		return CControlUI::EstimateSize(szAvailable);
	}

	bool CComboUI::Activate()
	{
		if( !CControlUI::Activate() ) return false;
		if( m_pManager != NULL ) m_pManager->SendNotify(this, DUI_MSGTYPE_PREDROPDOWN);
		if( m_pWindow ) return true;
		m_pWindow = new CComboWnd();
		ASSERT(m_pWindow);
		m_pWindow->Init(this);
		if( m_pManager != NULL ) m_pManager->SendNotify(this, DUI_MSGTYPE_DROPDOWN);
		Invalidate();
		return true;
	}

	std::wstring CComboUI::GetText() const
	{
		if( m_iCurSel < 0 || !m_bShowSelectedItemText) return CControlUI::GetText();

		CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
		return pControl->GetText();
	}

	void CComboUI::SetEnabled(bool bEnable)
	{
		CContainerUI::SetEnabled(bEnable);
		if( !IsEnabled() ) m_uButtonState = 0;
	}

	std::wstring CComboUI::GetDropBoxAttributeList()
	{
		return m_sDropBoxAttributes;
	}

	void CComboUI::SetDropBoxAttributeList(std::wstring_view pstrList)
	{
		m_sDropBoxAttributes.assign(pstrList);
	}

	SIZE CComboUI::GetDropBoxSize() const
	{
		if (m_pManager != NULL) {
			return m_pManager->ScaleSize(m_szDropBox);
		}
		return m_szDropBox;
	}

	void CComboUI::SetDropBoxSize(SIZE szDropBox)
	{
		m_szDropBox = szDropBox;
	}

	RECT CComboUI::GetDropBoxInset() const
	{
		RECT rcDropBox = m_rcDropBox;
		if (m_pManager != NULL) {
			m_pManager->ScaleRect(&rcDropBox);
		}
		return rcDropBox;
	}

	void CComboUI::SetDropBoxInset(RECT rcDropBox)
	{
		m_rcDropBox = rcDropBox;
	}

	RECT CComboUI::GetDropBoxPadding() const 
	{
		RECT rcDropBoxPadding = m_rcDropBoxPadding;
		if (m_pManager != NULL) {
			m_pManager->ScaleRect(&rcDropBoxPadding);
		}
		return rcDropBoxPadding;
	}

	void CComboUI::SetDropBoxPadding(RECT szDropBoxPadding)
	{

		m_rcDropBoxPadding = szDropBoxPadding;
	}

	DWORD CComboUI::GetDropBoxColor() const
	{	
		return m_dwDropBoxBkcolor;
	}

	void CComboUI::SetDropBoxBkColor(DWORD dwBkColor)
	{		
		m_dwDropBoxBkcolor = dwBkColor;
		Invalidate();
	}

	bool CComboUI::IsAutoDropBoxSize()
	{
		return m_bIsAutoDropBoxSize;
	}

	void CComboUI::SetAutoDropBoxSize(bool bIsShow)
	{
		if(bIsShow == m_bIsAutoDropBoxSize) return ;

		m_bIsAutoDropBoxSize = bIsShow;
		Invalidate();
	}

	bool CComboUI::IsExpansionDirection() const
	{
		return m_bExpansionDirection;
	}

	void CComboUI::SetExpansionDirection(bool bDownward)
	{
		m_bExpansionDirection = bDownward;
	}

	int CComboUI::GetExpansionAnimDuration() const
	{
		return m_nExpansionAnimDuration;
	}

	void CComboUI::SetExpansionAnimDuration(int nMs)
	{
		m_nExpansionAnimDuration = nMs;
	}


	void CComboUI::SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}

	UINT CComboUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	void CComboUI::SetTextColor(DWORD dwTextColor)
	{
		m_dwTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CComboUI::GetTextColor() const
	{
		return m_dwTextColor;
	}

	void CComboUI::SetDisabledTextColor(DWORD dwTextColor)
	{
		m_dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CComboUI::GetDisabledTextColor() const
	{
		return m_dwDisabledTextColor;
	}

	void CComboUI::SetFont(int index)
	{
		m_iFont = index;
		Invalidate();
	}

	int CComboUI::GetFont() const
	{
		return m_iFont;
	}

	RECT CComboUI::GetTextPadding() const
	{
		RECT rcTextPadding = m_rcTextPadding;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcTextPadding);
		return rcTextPadding;
	}

	void CComboUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		Invalidate();
	}

	bool CComboUI::IsShowHtml()
	{
		return m_bShowHtml;
	}

	void CComboUI::SetShowHtml(bool bShowHtml)
	{
		if( m_bShowHtml == bShowHtml ) return;

		m_bShowHtml = bShowHtml;
		Invalidate();
	}

	bool CComboUI::IsShowShadow()
	{
		return m_bShowShadow;
	}

	void CComboUI::SetShowShadow(bool bShow)
	{
		if( m_bShowShadow == bShow ) return;

		m_bShowShadow = bShow;
		Invalidate();
	}

	std::wstring_view CComboUI::GetNormalImage() const
	{
		return m_sNormalImage;
	}

	void CComboUI::SetNormalImage(std::wstring_view pStrImage)
	{
		m_sNormalImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CComboUI::GetHotImage() const
	{
		return m_sHotImage;
	}

	void CComboUI::SetHotImage(std::wstring_view pStrImage)
	{
		m_sHotImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CComboUI::GetPushedImage() const
	{
		return m_sPushedImage;
	}

	void CComboUI::SetPushedImage(std::wstring_view pStrImage)
	{
		m_sPushedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CComboUI::GetFocusedImage() const
	{
		return m_sFocusedImage;
	}

	void CComboUI::SetFocusedImage(std::wstring_view pStrImage)
	{
		m_sFocusedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CComboUI::GetDisabledImage() const
	{
		return m_sDisabledImage;
	}

	void CComboUI::SetDisabledImage(std::wstring_view pStrImage)
	{
		m_sDisabledImage.assign(pStrImage);
		Invalidate();
	}

	bool CComboUI::GetScrollSelect()
	{
		return m_bScrollSelect;
	}

	void CComboUI::SetScrollSelect(bool bScrollSelect)
	{
		m_bScrollSelect = bScrollSelect;
	}

	void CComboUI::SetItemFont(int index)
	{
		m_ListInfo.nFont = index;
		Invalidate();
	}

	void CComboUI::SetItemTextStyle(UINT uStyle)
	{
		m_ListInfo.uTextStyle = uStyle;
		Invalidate();
	}

	RECT CComboUI::GetItemTextPadding() const
	{
		return m_ListInfo.rcTextPadding;
	}

	void CComboUI::SetItemTextPadding(RECT rc)
	{
		m_ListInfo.rcTextPadding = rc;
		Invalidate();
	}

	void CComboUI::SetItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwTextColor = dwTextColor;
		Invalidate();
	}

	void CComboUI::SetItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwBkColor = dwBkColor;
	}

	void CComboUI::SetItemBkImage(std::wstring_view pStrImage)
	{
		m_ListInfo.sBkImage.assign(pStrImage);
	}

	DWORD CComboUI::GetItemTextColor() const
	{
		return m_ListInfo.dwTextColor;
	}

	DWORD CComboUI::GetItemBkColor() const
	{
		return m_ListInfo.dwBkColor;
	}

	std::wstring_view CComboUI::GetItemBkImage() const
	{
		return m_ListInfo.sBkImage;
	}

	bool CComboUI::IsAlternateBk() const
	{
		return m_ListInfo.bAlternateBk;
	}

	void CComboUI::SetAlternateBk(bool bAlternateBk)
	{
		m_ListInfo.bAlternateBk = bAlternateBk;
	}

	void CComboUI::SetSelectedItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwSelectedTextColor = dwTextColor;
	}

	void CComboUI::SetSelectedItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwSelectedBkColor = dwBkColor;
	}

	void CComboUI::SetSelectedItemImage(std::wstring_view pStrImage)
	{
		m_ListInfo.sSelectedImage.assign(pStrImage);
	}

	DWORD CComboUI::GetSelectedItemTextColor() const
	{
		return m_ListInfo.dwSelectedTextColor;
	}

	DWORD CComboUI::GetSelectedItemBkColor() const
	{
		return m_ListInfo.dwSelectedBkColor;
	}

	std::wstring_view CComboUI::GetSelectedItemImage() const
	{
		return m_ListInfo.sSelectedImage;
	}

	void CComboUI::SetHotItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwHotTextColor = dwTextColor;
	}

	void CComboUI::SetHotItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwHotBkColor = dwBkColor;
	}

	void CComboUI::SetHotItemImage(std::wstring_view pStrImage)
	{
		m_ListInfo.sHotImage.assign(pStrImage);
	}

	DWORD CComboUI::GetHotItemTextColor() const
	{
		return m_ListInfo.dwHotTextColor;
	}

	DWORD CComboUI::GetHotItemBkColor() const
	{
		return m_ListInfo.dwHotBkColor;
	}

	std::wstring_view CComboUI::GetHotItemImage() const
	{
		return m_ListInfo.sHotImage;
	}

	void CComboUI::SetDisabledItemTextColor(DWORD dwTextColor)
	{
		m_ListInfo.dwDisabledTextColor = dwTextColor;
	}

	void CComboUI::SetDisabledItemBkColor(DWORD dwBkColor)
	{
		m_ListInfo.dwDisabledBkColor = dwBkColor;
	}

	void CComboUI::SetDisabledItemImage(std::wstring_view pStrImage)
	{
		m_ListInfo.sDisabledImage.assign(pStrImage);
	}

	DWORD CComboUI::GetDisabledItemTextColor() const
	{
		return m_ListInfo.dwDisabledTextColor;
	}

	DWORD CComboUI::GetDisabledItemBkColor() const
	{
		return m_ListInfo.dwDisabledBkColor;
	}

	std::wstring_view CComboUI::GetDisabledItemImage() const
	{
		return m_ListInfo.sDisabledImage;
	}

	DWORD CComboUI::GetItemLineColor() const
	{
		return m_ListInfo.dwLineColor;
	}

	void CComboUI::SetItemLineColor(DWORD dwLineColor)
	{
		m_ListInfo.dwLineColor = dwLineColor;
	}

	bool CComboUI::IsItemShowHtml()
	{
		return m_ListInfo.bShowHtml;
	}

	void CComboUI::SetItemShowHtml(bool bShowHtml)
	{
		if( m_ListInfo.bShowHtml == bShowHtml ) return;

		m_ListInfo.bShowHtml = bShowHtml;
		Invalidate();
	}

	bool CComboUI::IsShowSelectedItemText()
	{
		return m_bShowSelectedItemText;
	}

	void CComboUI::SetShowSelectedItemText(bool bIsShow)
	{
		if (bIsShow == m_bShowSelectedItemText) return ;

		m_bShowSelectedItemText = bIsShow;
		Invalidate();
	}

	void CComboUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		if(!::EqualRect(&rc, &m_rcItem)) {
			// 闂呮劘妫屾稉瀣缁愭褰?
			if(m_pWindow && ::IsWindow(m_pWindow->GetHWND())) m_pWindow->Close();
			// 閹碘偓閺堝鍘撶槐鐘层亣鐏忓繒鐤嗘稉?
			RECT rcNull = { 0 };
			for( int i = 0; i < m_items.GetSize(); i++ ) static_cast<CControlUI*>(m_items[i])->SetPos(rcNull);
			// 鐠嬪啯鏆ｆ担宥囩枂
			CControlUI::SetPos(rc, bNeedInvalidate);
		}
	}

	void CComboUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CControlUI::Move(szOffset, bNeedInvalidate);
	}

	    void CComboUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
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

        if (StringUtil::EqualsNoCase(name, L"align")) {
            if (contains(L"left")) {
                m_uTextStyle &= ~(DT_CENTER | DT_RIGHT | DT_SINGLELINE);
                m_uTextStyle |= DT_LEFT;
            }
            if (contains(L"center")) {
                m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
                m_uTextStyle |= DT_CENTER;
            }
            if (contains(L"right")) {
                m_uTextStyle &= ~(DT_LEFT | DT_CENTER | DT_SINGLELINE);
                m_uTextStyle |= DT_RIGHT;
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"valign")) {
            if (contains(L"top")) {
                m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER);
                m_uTextStyle |= (DT_TOP | DT_SINGLELINE);
            }
            if (contains(L"vcenter")) {
                m_uTextStyle &= ~(DT_TOP | DT_BOTTOM);
                m_uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
            }
            if (contains(L"bottom")) {
                m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
                m_uTextStyle |= (DT_BOTTOM | DT_SINGLELINE);
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"endellipsis")) {
            if (StringUtil::ParseBool(pstrValueView)) m_uTextStyle |= DT_END_ELLIPSIS;
            else m_uTextStyle &= ~DT_END_ELLIPSIS;
        }
        else if (StringUtil::EqualsNoCase(name, L"wordbreak")) {
            if (StringUtil::ParseBool(pstrValueView)) {
                m_uTextStyle &= ~DT_SINGLELINE;
                m_uTextStyle |= DT_WORDBREAK | DT_EDITCONTROL;
            }
            else {
                m_uTextStyle &= ~(DT_WORDBREAK | DT_EDITCONTROL);
                m_uTextStyle |= DT_SINGLELINE;
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"font")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetFont(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"textcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"disabledtextcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetDisabledTextColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"textpadding")) {
            RECT rect = { 0 };
            if (StringUtil::TryParseRect(pstrValueView, rect)) SetTextPadding(rect);
        }
        else if (StringUtil::EqualsNoCase(name, L"showhtml")) SetShowHtml(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"showshadow")) SetShowShadow(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"normalimage")) SetNormalImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"hotimage")) SetHotImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"pushedimage")) SetPushedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"focusedimage")) SetFocusedImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"disabledimage")) SetDisabledImage(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"scrollselect")) SetScrollSelect(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"dropbox")) SetDropBoxAttributeList(pstrValueView);
        else if (StringUtil::EqualsNoCase(name, L"dropboxsize")) {
            SIZE size = { 0, 0 };
            if (StringUtil::TryParseSize(pstrValueView, size)) SetDropBoxSize(size);
        }
        else if (StringUtil::EqualsNoCase(name, L"dropboxinset")) {
            RECT rect = { 0 };
            if (StringUtil::TryParseRect(pstrValueView, rect)) SetDropBoxInset(rect);
        }
        else if (StringUtil::EqualsNoCase(name, L"dropboxpadding")) {
            RECT rect = { 0 };
            if (StringUtil::TryParseRect(pstrValueView, rect)) SetDropBoxPadding(rect);
        }
        else if (StringUtil::EqualsNoCase(name, L"dropboxbkcolor")) {
            DWORD color = 0;
            if (StringUtil::TryParseColor(pstrValueView, color)) SetDropBoxBkColor(color);
        }
        else if (StringUtil::EqualsNoCase(name, L"autodropboxsize")) SetAutoDropBoxSize(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"expansion_direction")
              || StringUtil::EqualsNoCase(name, L"expansiondirection")) {
            SetExpansionDirection(StringUtil::ParseBool(pstrValueView));
        }
        else if (StringUtil::EqualsNoCase(name, L"expansion_animation_duration")
              || StringUtil::EqualsNoCase(name, L"expansionanimationduration")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetExpansionAnimDuration(value);
        }
        else if (StringUtil::EqualsNoCase(name, L"itemfont")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) SetItemFont(value);
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
        else if (StringUtil::EqualsNoCase(name, L"itemshowhtml")) SetItemShowHtml(StringUtil::ParseBool(pstrValueView));
        else if (StringUtil::EqualsNoCase(name, L"showselecteditemtext")) SetShowSelectedItemText(StringUtil::ParseBool(pstrValueView));
        else CContainerUI::SetAttribute(pstrNameView, pstrValueView);
    }

	void CComboUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.empty() ) {
				if( !DrawImage(renderContext, m_sDisabledImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sPushedImage.empty() ) {
				if( !DrawImage(renderContext, m_sPushedImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHotImage.empty() ) {
				if( !DrawImage(renderContext, m_sHotImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusedImage.empty() ) {
				if( !DrawImage(renderContext, m_sFocusedImage) ) {}
				else return;
			}
		}

		if( !m_sNormalImage.empty() ) {
			if( !DrawImage(renderContext, m_sNormalImage) ) {}
			else return;
		}
	}

	void CComboUI::PaintText(CPaintRenderContext& renderContext)
	{
		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		RECT rc = m_rcItem;
		const RECT rcTextPadding = GetTextPadding();
		rc.left += rcTextPadding.left;
		rc.right -= rcTextPadding.right;
		rc.top += rcTextPadding.top;
		rc.bottom -= rcTextPadding.bottom;

		std::wstring sText = GetText();
		if( sText.empty() ) return;
		int nLinks = 0;
		if( IsEnabled() ) {
			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(renderContext, rc, sText, m_dwTextColor, NULL, NULL, nLinks, m_iFont, m_uTextStyle);
			else
			CRenderEngine::DrawText(renderContext, rc, sText, m_dwTextColor, m_iFont, m_uTextStyle);
		}
		else {
			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(renderContext, rc, sText, m_dwDisabledTextColor, NULL, NULL, nLinks, m_iFont, m_uTextStyle);
			else
			CRenderEngine::DrawText(renderContext, rc, sText, m_dwDisabledTextColor, m_iFont, m_uTextStyle);
		}
	}

	CComboUI* CComboUI::Clone()
	{
		CComboUI* pClone = new CComboUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CComboUI::CopyData(CComboUI* pControl)
	{
		m_iCurSel = pControl->m_iCurSel;
		m_dwTextColor = pControl->m_dwTextColor;
		m_dwDisabledTextColor = pControl->m_dwDisabledTextColor;
		m_iFont = pControl->m_iFont;
		m_uTextStyle = pControl->m_uTextStyle;
		m_rcTextPadding = pControl->m_rcTextPadding;
		m_bShowHtml = pControl->m_bShowHtml;
		m_bShowShadow = pControl->m_bShowShadow;
		m_sDropBoxAttributes = pControl->m_sDropBoxAttributes;
		m_szDropBox = pControl->m_szDropBox;
		m_rcDropBox = pControl->m_rcDropBox;
		m_rcDropBoxPadding = pControl->m_rcDropBoxPadding;
		m_dwDropBoxBkcolor = pControl->m_dwDropBoxBkcolor;
		m_uButtonState = pControl->m_uButtonState;

		m_sNormalImage = pControl->m_sNormalImage;
		m_sHotImage = pControl->m_sHotImage;
		m_sPushedImage = pControl->m_sPushedImage;
		m_sFocusedImage = pControl->m_sFocusedImage;
		m_sDisabledImage = pControl->m_sDisabledImage;

		m_bScrollSelect = pControl->m_bScrollSelect;
		m_ListInfo = pControl->m_ListInfo;
		m_bShowSelectedItemText = pControl->m_bShowSelectedItemText;
		m_bIsAutoDropBoxSize = pControl->m_bIsAutoDropBoxSize;
		m_bExpansionDirection = pControl->m_bExpansionDirection;
		m_nExpansionAnimDuration = pControl->m_nExpansionAnimDuration;
		__super::CopyData(pControl);
	}

	BOOL CComboUI::SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData)
	{
		if (!pfnCompare)
			return FALSE;
		m_pCompareFunc = pfnCompare;
		m_compareData = dwData;

		qsort_s(m_items.c_str(), m_items.GetSize(), sizeof(CControlUI*), CComboUI::ItemComareFunc, this);
		IListItemUI* pItem = NULL;
		for (int i = 0; i < m_items.GetSize(); ++i)
		{
			pItem = (IListItemUI*)(static_cast<CControlUI*>(m_items[i])->GetInterface(TEXT("ListItem")));
			if (pItem)
			{
				pItem->SetIndex(i);
				pItem->Select(false);
			}
		}

		if (m_pManager)
		{
			SetPos(GetPos());
			Invalidate();
		}

		return TRUE;
	}

	static int __cdecl ComareFunc(void* pvlocale, const void* item1, const void* item2)
	{
		CComboUI* pThis = (CComboUI*)pvlocale;

		if (!pThis || !item1 || !item2)
			return 0;

		CControlUI* pControl1 = *(CControlUI**)item1;
		CControlUI* pControl2 = *(CControlUI**)item2;
		if (!pControl1 || !pControl2)
		{
			return 0;
		}
		return pControl1->GetText().compare(pControl2->GetText());
	}

	void CComboUI::SortItems()
	{
		qsort_s(m_items.c_str(), m_items.GetSize(), sizeof(CControlUI*), ComareFunc, this);
		IListItemUI* pItem = NULL;
		for (int i = 0; i < m_items.GetSize(); ++i)
		{
			pItem = (IListItemUI*)(static_cast<CControlUI*>(m_items[i])->GetInterface(TEXT("ListItem")));
			if (pItem)
			{
				pItem->SetIndex(i);
				pItem->Select(false);
			}
		}

		if (m_pManager)
		{
			SetPos(GetPos());
			Invalidate();
		}

		return ;
	}

	int __cdecl CComboUI::ItemComareFunc(void* pvlocale, const void* item1, const void* item2)
	{
		CComboUI* pThis = (CComboUI*)pvlocale;
		if (!pThis || !item1 || !item2)
			return 0;
		return pThis->ItemComareFunc(item1, item2);
	}

	int __cdecl CComboUI::ItemComareFunc(const void* item1, const void* item2)
	{
		if (!m_pCompareFunc)
		{
			return 0;
		}
		CControlUI* pControl1 = *(CControlUI**)item1;
		CControlUI* pControl2 = *(CControlUI**)item2;
		return m_pCompareFunc((UINT_PTR)pControl1, (UINT_PTR)pControl2, m_compareData);
	}
} // namespace DuiLib


