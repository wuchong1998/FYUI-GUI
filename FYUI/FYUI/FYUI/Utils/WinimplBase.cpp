#pragma once
#include "pch.h"
#include <algorithm>
#include "WinImplBase.h"
namespace FYUI
{
	namespace
	{
		bool IsKeyDown(int key)
		{
			return ::GetKeyState(key) < 0;
		}

		WPARAM BuildClientMouseKeyState(bool leftButtonDown)
		{
			WPARAM keyState = 0;
			if (IsKeyDown(VK_SHIFT)) keyState |= MK_SHIFT;
			if (IsKeyDown(VK_CONTROL)) keyState |= MK_CONTROL;
			if (IsKeyDown(VK_RBUTTON)) keyState |= MK_RBUTTON;
			if (IsKeyDown(VK_MBUTTON)) keyState |= MK_MBUTTON;
			if (IsKeyDown(VK_XBUTTON1)) keyState |= MK_XBUTTON1;
			if (IsKeyDown(VK_XBUTTON2)) keyState |= MK_XBUTTON2;
			if (leftButtonDown) keyState |= MK_LBUTTON;
			return keyState;
		}

		LPARAM MakeClientPointParam(HWND hWnd, LPARAM screenParam)
		{
			POINT pt = { GET_X_LPARAM(screenParam), GET_Y_LPARAM(screenParam) };
			::ScreenToClient(hWnd, &pt);
			return MAKELPARAM(pt.x, pt.y);
		}

		bool ForwardNcMaxButtonMouseMessage(HWND hWnd, UINT uMsg, LPARAM lParam)
		{
			UINT clientMsg = 0;
			WPARAM keyState = 0;
			switch (uMsg) {
			case WM_NCLBUTTONDOWN:
				clientMsg = WM_LBUTTONDOWN;
				keyState = BuildClientMouseKeyState(true);
				break;
			case WM_NCLBUTTONUP:
				clientMsg = WM_LBUTTONUP;
				keyState = BuildClientMouseKeyState(false);
				break;
			case WM_NCLBUTTONDBLCLK:
				clientMsg = WM_LBUTTONDBLCLK;
				keyState = BuildClientMouseKeyState(true);
				break;
			case WM_NCMOUSEMOVE:
				clientMsg = WM_MOUSEMOVE;
				keyState = BuildClientMouseKeyState(IsKeyDown(VK_LBUTTON));
				break;
			case WM_NCMOUSEHOVER:
				clientMsg = WM_MOUSEHOVER;
				keyState = BuildClientMouseKeyState(IsKeyDown(VK_LBUTTON));
				break;
			case WM_NCMOUSELEAVE:
				clientMsg = WM_MOUSELEAVE;
				break;
			default:
				return false;
			}

			const LPARAM clientParam = (uMsg == WM_NCMOUSELEAVE) ? 0 : MakeClientPointParam(hWnd, lParam);
			::SendMessage(hWnd, clientMsg, keyState, clientParam);
			return true;
		}

		void SetNamedControlVisible(CPaintManagerUI& manager, const wchar_t* name, bool visible)
		{
			CControlUI* pControl = static_cast<CControlUI*>(manager.FindControl(name));
			if (pControl != nullptr) {
				pControl->SetVisible(visible);
			}
		}

		void UpdateMaxRestoreButtonVisibility(CPaintManagerUI& manager, bool isZoomed)
		{
			if (!manager.IsValid()) {
				return;
			}
			SetNamedControlVisible(manager, _T("maxbtn"), !isZoomed);
			SetNamedControlVisible(manager, _T("restorebtn"), isZoomed);
		}

		bool TryApplyDwmRoundCorner(HWND hWnd, SIZE roundCorner)
		{
			HMODULE hDwmApi = ::LoadLibraryW(L"dwmapi.dll");
			if (hDwmApi == NULL) {
				return false;
			}

			using DwmSetWindowAttributeProc = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
			auto setWindowAttribute = reinterpret_cast<DwmSetWindowAttributeProc>(
				::GetProcAddress(hDwmApi, "DwmSetWindowAttribute"));
			if (setWindowAttribute == nullptr) {
				::FreeLibrary(hDwmApi);
				return false;
			}

			constexpr DWORD kDwmWindowCornerPreference = 33;
			constexpr DWORD kDwmCornerDoNotRound = 1;
			constexpr DWORD kDwmCornerRound = 2;
			const DWORD preference = (roundCorner.cx > 0 && roundCorner.cy > 0)
				? kDwmCornerRound
				: kDwmCornerDoNotRound;
			const HRESULT hr = setWindowAttribute(
				hWnd,
				kDwmWindowCornerPreference,
				&preference,
				sizeof(preference));
			::FreeLibrary(hDwmApi);
			return SUCCEEDED(hr);
		}

		void ApplyWindowRoundRegion(HWND hWnd, const RECT& rcWindow, SIZE roundCorner)
		{
			if (roundCorner.cx <= 0 || roundCorner.cy <= 0) {
				TryApplyDwmRoundCorner(hWnd, roundCorner);
				::SetWindowRgn(hWnd, NULL, TRUE);
				return;
			}

			if (TryApplyDwmRoundCorner(hWnd, roundCorner)) {
				::SetWindowRgn(hWnd, NULL, TRUE);
				return;
			}

			HRGN hRgn = ::CreateRoundRectRgn(rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom, roundCorner.cx, roundCorner.cy);
			if (hRgn == NULL) {
				return;
			}
			if (::SetWindowRgn(hWnd, hRgn, TRUE) == 0) {
				::DeleteObject(hRgn);
			}
		}

		bool IsStaticControlClass(const std::wstring& className)
		{
			static constexpr std::wstring_view kStaticClassNames[] = {
				L"controlui",
				L"textui",
				L"labelui",
				L"containerui",
				L"horizontallayoutui",
				L"verticallayoutui",
				L"tablayoutui",
				L"childlayoutui",
				L"dialoglayoutui",
				L"progresscontainerui",
			};

			for (std::wstring_view staticClassName : kStaticClassNames) {
				if (className == staticClassName) {
					return true;
				}
			}
			return false;
		}

		RECT ResolveCaptionHitRect(const RECT& rcClient, RECT rcCaption)
		{
			if (rcCaption.bottom < 0) {
				rcCaption.bottom = rcClient.bottom;
			}

			RECT rcHit = {
				rcClient.left + rcCaption.left,
				rcClient.top + rcCaption.top,
				rcClient.right - rcCaption.right,
				rcClient.top + rcCaption.bottom
			};

			const LONG clientWidth = rcClient.right - rcClient.left;
			if (rcCaption.right > rcCaption.left && rcCaption.right >= clientWidth / 2) {
				// Some existing XML uses caption="left,top,right,bottom" as an absolute rect.
				rcHit.right = (std::min)(rcClient.left + rcCaption.right, rcClient.right);
			}

			if (rcHit.left < rcClient.left) rcHit.left = rcClient.left;
			if (rcHit.top < rcClient.top) rcHit.top = rcClient.top;
			if (rcHit.right > rcClient.right) rcHit.right = rcClient.right;
			if (rcHit.bottom > rcClient.bottom) rcHit.bottom = rcClient.bottom;
			return rcHit;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//
	DUI_BEGIN_MESSAGE_MAP(WindowImplBase, CNotifyPump)
		DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
		DUI_END_MESSAGE_MAP()

		void WindowImplBase::OnFinalMessage( HWND hWnd )
	{
		m_pm.RemovePreMessageFilter(this);
		m_pm.RemoveNotifier(this);
		m_pm.ReapObjects(m_pm.GetRoot());
	}

	LRESULT WindowImplBase::ResponseDefaultKeyEvent(WPARAM wParam)
	{
		if (wParam == VK_RETURN)
		{
			return FALSE;
		}
		else if (wParam == VK_ESCAPE)
		{
			return TRUE;
		}

		return FALSE;
	}



	UINT WindowImplBase::GetClassStyle() const
	{
		return CS_DBLCLKS;
	}

	CControlUI* WindowImplBase::CreateControl(std::wstring_view pstrClass)
	{
		(void)pstrClass;
		return NULL;
	}

	std::wstring WindowImplBase::QueryControlText(std::wstring_view lpstrId, std::wstring_view lpstrType)
	{
		(void)lpstrId;
		(void)lpstrType;
		return {};
	}

	LRESULT WindowImplBase::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled)
	{
		bHandled = false;
		if (uMsg == WM_KEYDOWN)
		{
			switch (wParam)
			{
			case VK_RETURN:
			case VK_ESCAPE:
			{
				const LRESULT result = ResponseDefaultKeyEvent(wParam);
				bHandled = (result != FALSE);
				return result;
			}
			default:
				break;
			}
		}
		return FALSE;
	}

	LRESULT WindowImplBase::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

#if defined(WIN32) && !defined(UNDER_CE)
	LRESULT WindowImplBase::OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if( ::IsIconic(*this) ) bHandled = FALSE;
		return (wParam == 0) ? TRUE : FALSE;
	}

	LRESULT WindowImplBase::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT WindowImplBase::OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0;
	}


	BOOL WindowImplBase::IsInStaticControl(CControlUI *pControl)
	{
		BOOL bRet = FALSE;
		if (!pControl)
		{
			return bRet;
		}

		const std::wstring_view controlClass = pControl->GetClass();
		std::wstring strClassName(controlClass.data(), controlClass.length());
		StringUtil::MakeLower(strClassName);
		if (IsStaticControlClass(strClassName))
		{
			CControlUI* pParent = pControl->GetParent();
			while (pParent)
			{
				const std::wstring_view parentClass = pParent->GetClass();
				strClassName.assign(parentClass.data(), parentClass.length());
				StringUtil::MakeLower(strClassName);
				if (!IsStaticControlClass(strClassName))
				{
					return bRet;
				}

				pParent = pParent->GetParent();
			}

			bRet = TRUE;
		}

		return bRet;
	}


	LRESULT WindowImplBase::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
		::ScreenToClient(*this, &pt);

		RECT rcClient = {};
		if (!::GetClientRect(*this, &rcClient)) {
			return HTCLIENT;
		}

		CControlUI* pMaxBtn = static_cast<CControlUI*>(m_pm.FindControl(_T("maxbtn")));
		CControlUI* pRestoreBtn = static_cast<CControlUI*>(m_pm.FindControl(_T("restorebtn")));
		if (pMaxBtn && pMaxBtn->IsVisible()) {
			RECT rcBtn = pMaxBtn->GetPos();
			if (::PtInRect(&rcBtn, pt)) {
				return HTMAXBUTTON;
			}
		}
		else if (pRestoreBtn && pRestoreBtn->IsVisible()) {
			RECT rcBtn = pRestoreBtn->GetPos();
			if (::PtInRect(&rcBtn, pt)) {
				return HTMAXBUTTON;
			}
		}

		if (!::IsZoomed(*this))
		{
			RECT rcSizeBox = m_pm.GetSizeBox();
			if (pt.y < rcClient.top + rcSizeBox.top)
			{
				if (pt.x < rcClient.left + rcSizeBox.left) return HTTOPLEFT;
				if (pt.x > rcClient.right - rcSizeBox.right) return HTTOPRIGHT;
				return HTTOP;
			}
			else if (pt.y > rcClient.bottom - rcSizeBox.bottom)
			{
				if (pt.x < rcClient.left + rcSizeBox.left) return HTBOTTOMLEFT;
				if (pt.x > rcClient.right - rcSizeBox.right) return HTBOTTOMRIGHT;
				return HTBOTTOM;
			}

			if (pt.x < rcClient.left + rcSizeBox.left) return HTLEFT;
			if (pt.x > rcClient.right - rcSizeBox.right) return HTRIGHT;
		}

		const RECT rcCaption = ResolveCaptionHitRect(rcClient, m_pm.GetCaptionRect());
		if (::PtInRect(&rcCaption, pt))
		{
			CControlUI* pControl = m_pm.FindControl(pt);
			if (pControl == nullptr || IsInStaticControl(pControl))
			{
				return HTCAPTION;
			}
		}

		return HTCLIENT;
	}

	LRESULT WindowImplBase::OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		MONITORINFO Monitor = {};
		Monitor.cbSize = sizeof(Monitor);
		if (!::GetMonitorInfo(::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &Monitor)) {
			bHandled = FALSE;
			return 0;
		}
		RECT rcWork = Monitor.rcWork;
		RECT rcMonitor = Monitor.rcMonitor;
		const int workWidth = rcWork.right - rcWork.left;
		const int workHeight = rcWork.bottom - rcWork.top;

		LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;
		lpMMI->ptMaxPosition.x = rcWork.left - rcMonitor.left;
		lpMMI->ptMaxPosition.y = rcWork.top - rcMonitor.top;
		lpMMI->ptMaxSize.x = workWidth;
		lpMMI->ptMaxSize.y = workHeight;
		lpMMI->ptMaxTrackSize.x = m_pm.GetMaxInfo().cx == 0 ? workWidth : m_pm.GetMaxInfo().cx;
		lpMMI->ptMaxTrackSize.y = m_pm.GetMaxInfo().cy == 0 ? workHeight : m_pm.GetMaxInfo().cy;
		lpMMI->ptMinTrackSize.x = m_pm.GetMinInfo().cx;
		lpMMI->ptMinTrackSize.y = m_pm.GetMinInfo().cy;

		bHandled = TRUE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}
	LRESULT WindowImplBase::OnDpiChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE;
		const UINT changeDpi = LOWORD(wParam);
		const RECT* pSuggestedWindowRect = reinterpret_cast<const RECT*>(lParam);
		m_pm.SetDPI(static_cast<int>(changeDpi), pSuggestedWindowRect);
		return 0;
	}
#endif

	LRESULT WindowImplBase::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SIZE szRoundCorner = m_pm.GetRoundCorner();
#if defined(WIN32) && !defined(UNDER_CE)
		if( !::IsIconic(*this) ) {
			if (::IsZoomed(*this)) {
				::SetWindowRgn(*this, NULL, TRUE);
			}
			else {
				CDuiRect rcWnd;
				::GetWindowRect(*this, &rcWnd);
				rcWnd.Offset(-rcWnd.left, -rcWnd.top);
				rcWnd.right++; rcWnd.bottom++;
				ApplyWindowRoundRegion(*this, rcWnd, szRoundCorner);
			}
			UpdateMaxRestoreButtonVisibility(m_pm, ::IsZoomed(*this));
		}
#endif
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if ((wParam & 0xFFF0) == SC_CLOSE)
		{
			bHandled = TRUE;
			Close(0);
			return 0;
		}
#if defined(WIN32) && !defined(UNDER_CE)
		BOOL bZoomed = ::IsZoomed(*this);
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		if( ::IsZoomed(*this) != bZoomed ) {
			UpdateMaxRestoreButtonVisibility(m_pm, ::IsZoomed(*this));
		}
#else
		LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
#endif
		return lRes;
	}

	LRESULT WindowImplBase::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		// 璋冩暣绐楀彛鏍峰紡
		const LONG styleValue = GetStyle() | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		::SetWindowLong(*this, GWL_STYLE, styleValue);


		// 鍏宠仈UI绠＄悊鍣?
		m_pm.Init(m_hWnd, GetManagerName());
		m_pm.SyncDPIFromWindow(m_hWnd);
		// 娉ㄥ唽PreMessage鍥炶皟
		m_pm.AddPreMessageFilter(this);
		InitResource();

		// 鍒涘缓涓荤獥鍙?
		CControlUI* pRoot=NULL;
		CDialogBuilder builder;
		std::wstring sSkinType = GetSkinType();
		std::wstring skinFile = GetSkinFile();
		if (!sSkinType.empty()) {
			STRINGorID xml(_ttoi(skinFile.c_str()));
			pRoot = builder.Create(xml, sSkinType, this, &m_pm);
		}
		else {
			pRoot = builder.Create(STRINGorID(skinFile.c_str()), {}, this, &m_pm);
		}

		if (pRoot == NULL) {
			std::wstring sError = L"Load skin file failed: ";
			sError += skinFile;
			MessageBox(NULL, sError.c_str(), _T("Duilib") ,MB_OK|MB_ICONERROR);
			return -1;
		}
		m_pm.AttachDialog(pRoot);
		// 娣诲姞Notify浜嬩欢鎺ュ彛
		m_pm.AddNotifier(this);
		// 绐楀彛鍒濆鍖栧畬姣?
		InitWindow();
		return 0;
	}

	LRESULT WindowImplBase::OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LRESULT WindowImplBase::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;
		BOOL bHandled = TRUE;
		switch (uMsg)
		{
		case WM_CREATE:			lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
		case WM_CLOSE:			lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
		case WM_DESTROY:		lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
#if defined(WIN32) && !defined(UNDER_CE)
		case WM_NCACTIVATE:		lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
		case WM_NCCALCSIZE:		lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
		case WM_NCPAINT:		lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
		case WM_NCHITTEST:		lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
		case WM_NCLBUTTONDOWN:
		case WM_NCLBUTTONUP:
		case WM_NCLBUTTONDBLCLK: {
			if (wParam == HTMAXBUTTON) {
				ForwardNcMaxButtonMouseMessage(m_hWnd, uMsg, lParam);
				return 0;
			}
			bHandled = FALSE;
			break;
		}
		case WM_NCMOUSEHOVER:
		case WM_NCMOUSELEAVE:
		case WM_NCMOUSEMOVE: {
			if (wParam == HTMAXBUTTON) {
				ForwardNcMaxButtonMouseMessage(m_hWnd, uMsg, lParam);
				return 0;
			}
			bHandled = FALSE;
			break;
		}
		case WM_GETMINMAXINFO:	lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEWHEEL:		lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled); break;
#endif
		case WM_SIZE:			lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
		case WM_CHAR:		lRes = OnChar(uMsg, wParam, lParam, bHandled); break;
		case WM_SYSCOMMAND:		lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
		case WM_KEYDOWN:		lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
		case WM_KILLFOCUS:		lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_SETFOCUS:		lRes = OnSetFocus(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONUP:		lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled); break;
		case WM_LBUTTONDOWN:	lRes = OnLButtonDown(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEMOVE:		lRes = OnMouseMove(uMsg, wParam, lParam, bHandled); break;
		case WM_MOUSEHOVER:		lRes = OnMouseHover(uMsg, wParam, lParam, bHandled); break;
		case WM_DPICHANGED:
		{
			lRes = OnDpiChange(uMsg, wParam, lParam, bHandled); break;

		}
		default:				bHandled = FALSE; break;
		}
		
		if (bHandled) return lRes;

		lRes = HandleCustomMessage(uMsg, wParam, lParam, bHandled);
		if (bHandled) return lRes;

		if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes))
			return lRes;
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	}

	LRESULT WindowImplBase::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = FALSE;
		return 0;
	}

	LONG WindowImplBase::GetStyle()
	{
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;

		return styleValue;
	}

	void WindowImplBase::OnClick(TNotifyUI& msg)
	{
		if (msg.pSender == nullptr) {
			return;
		}
		std::wstring sCtrlName = msg.pSender->GetName();
		if( sCtrlName == _T("minbtn")) { 
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
			return; 
		}
		else if( sCtrlName == _T("maxbtn")) { 
			SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
			return; 
		}
		else if( sCtrlName == _T("restorebtn")) { 
			SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 
			return; 
		}
		else if (sCtrlName == _T("closebtn")) {
			Close(IDCANCEL);
			return;
		}
		return;
	}

	void WindowImplBase::Notify(TNotifyUI& msg)
	{
		return CNotifyPump::NotifyPump(msg);
	}
}

