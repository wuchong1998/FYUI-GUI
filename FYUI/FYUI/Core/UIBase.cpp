#include "pch.h"
#include "UIBase.h"
namespace FYUI 
{

    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    void FYUI_API DUI__Trace(std::wstring_view pstrFormat, ...)
    {
#ifdef _DEBUG
        wchar_t szBuffer[2048] = { 0 };
        const std::wstring formatStorage(pstrFormat);
        va_list args;
        va_start(args, pstrFormat);
        _vstprintf_s(szBuffer, _countof(szBuffer), formatStorage.c_str(), args);
        va_end(args);
        std::wstring strMsg = szBuffer;
        strMsg += _T("\n");
        OutputDebugString(strMsg.c_str());

#endif
    }

    std::wstring_view DUI__TraceMsg(UINT uMsg)
    {
#define MSGDEF(x) if(uMsg==x) return _T(#x)
        MSGDEF(WM_SETCURSOR);
        MSGDEF(WM_NCHITTEST);
        MSGDEF(WM_NCPAINT);
        MSGDEF(WM_PAINT);
        MSGDEF(WM_ERASEBKGND);
        MSGDEF(WM_NCMOUSEMOVE);
        MSGDEF(WM_MOUSEMOVE);
        MSGDEF(WM_MOUSELEAVE);
        MSGDEF(WM_MOUSEHOVER);
        MSGDEF(WM_NOTIFY);
        MSGDEF(WM_COMMAND);
        MSGDEF(WM_MEASUREITEM);
        MSGDEF(WM_DRAWITEM);
        MSGDEF(WM_LBUTTONDOWN);
        MSGDEF(WM_LBUTTONUP);
        MSGDEF(WM_LBUTTONDBLCLK);
        MSGDEF(WM_RBUTTONDOWN);
        MSGDEF(WM_RBUTTONUP);
        MSGDEF(WM_RBUTTONDBLCLK);
        MSGDEF(WM_SETFOCUS);
        MSGDEF(WM_KILLFOCUS);
        MSGDEF(WM_MOVE);
        MSGDEF(WM_SIZE);
        MSGDEF(WM_SIZING);
        MSGDEF(WM_MOVING);
        MSGDEF(WM_GETMINMAXINFO);
        MSGDEF(WM_CAPTURECHANGED);
        MSGDEF(WM_WINDOWPOSCHANGED);
        MSGDEF(WM_WINDOWPOSCHANGING);
        MSGDEF(WM_NCCALCSIZE);
        MSGDEF(WM_NCCREATE);
        MSGDEF(WM_NCDESTROY);
        MSGDEF(WM_TIMER);
        MSGDEF(WM_KEYDOWN);
        MSGDEF(WM_KEYUP);
        MSGDEF(WM_CHAR);
        MSGDEF(WM_SYSKEYDOWN);
        MSGDEF(WM_SYSKEYUP);
        MSGDEF(WM_SYSCOMMAND);
        MSGDEF(WM_SYSCHAR);
        MSGDEF(WM_VSCROLL);
        MSGDEF(WM_HSCROLL);
        MSGDEF(WM_CHAR);
        MSGDEF(WM_SHOWWINDOW);
        MSGDEF(WM_PARENTNOTIFY);
        MSGDEF(WM_CREATE);
        MSGDEF(WM_NCACTIVATE);
        MSGDEF(WM_ACTIVATE);
        MSGDEF(WM_ACTIVATEAPP);
        MSGDEF(WM_CLOSE);
        MSGDEF(WM_DESTROY);
        MSGDEF(WM_GETICON);
        MSGDEF(WM_GETTEXT);
        MSGDEF(WM_GETTEXTLENGTH);
        static wchar_t szMsg[10];
        ::wsprintf(szMsg, _T("0x%04X"), uMsg);
        return szMsg;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    //
    //

    //////////////////////////////////////////////////////////////////////////
    //
    DUI_BASE_BEGIN_MESSAGE_MAP(CNotifyPump)
        DUI_END_MESSAGE_MAP()

        static const DUI_MSGMAP_ENTRY* DuiFindMessageEntry(const DUI_MSGMAP_ENTRY* lpEntry, TNotifyUI& msg)
    {
        std::wstring sMsgType = msg.sType;
        std::wstring sCtrlName = msg.pSender->GetName();
        const DUI_MSGMAP_ENTRY* pMsgTypeEntry = NULL;
        while (lpEntry->nSig != DuiSig_end)
        {
            if (lpEntry->sMsgType == sMsgType)
            {
                if (!lpEntry->sCtrlName.empty())
                {
                    if (lpEntry->sCtrlName == sCtrlName)
                    {
                        return lpEntry;
                    }
                }
                else
                {
                    pMsgTypeEntry = lpEntry;
                }
            }
            lpEntry++;
        }
        return pMsgTypeEntry;
    }

    bool CNotifyPump::AddVirtualWnd(std::wstring strName, CNotifyPump* pObject)
    {
        if (m_VirtualWndMap.find(strName) == m_VirtualWndMap.end())
        {
            m_VirtualWndMap.emplace(std::move(strName), pObject);
            return true;
        }
        return false;
    }

    bool CNotifyPump::RemoveVirtualWnd(std::wstring strName)
    {
        const auto it = m_VirtualWndMap.find(strName);
        if (it != m_VirtualWndMap.end())
        {
            m_VirtualWndMap.erase(it);
            return true;
        }
        return false;
    }

    bool CNotifyPump::LoopDispatch(TNotifyUI& msg)
    {
        const DUI_MSGMAP_ENTRY* lpEntry = NULL;
        const DUI_MSGMAP* pMessageMap = NULL;

#ifndef FYUI_STATIC
        for (pMessageMap = GetMessageMap(); pMessageMap != NULL; pMessageMap = (*pMessageMap->pfnGetBaseMap)())
#else
        for (pMessageMap = GetMessageMap(); pMessageMap != NULL; pMessageMap = pMessageMap->pBaseMap)
#endif
        {
#ifndef FYUI_STATIC
            ASSERT(pMessageMap != (*pMessageMap->pfnGetBaseMap)());
#else
		ASSERT(pMessageMap != pMessageMap->pBaseMap);
#endif
            if ((lpEntry = DuiFindMessageEntry(pMessageMap->lpEntries, msg)) != NULL)
            {
                goto LDispatch;
            }
        }
        return false;

    LDispatch:
        union DuiMessageMapFunctions mmf;
        mmf.pfn = lpEntry->pfn;

        bool bRet = false;
        int nSig;
        nSig = lpEntry->nSig;
        switch (nSig)
        {
        default:
            ASSERT(FALSE);
            break;
        case DuiSig_lwl:
            (this->*mmf.pfn_Notify_lwl)(msg.wParam, msg.lParam);
            bRet = true;
            break;
        case DuiSig_vn:
            (this->*mmf.pfn_Notify_vn)(msg);
            bRet = true;
            break;
        }
        return bRet;
    }

    void CNotifyPump::NotifyPump(TNotifyUI& msg)
    {
        ///閬嶅巻铏氭嫙绐楀彛
        if (!msg.sVirtualWnd.empty()) {
            for (const auto& entry : m_VirtualWndMap) {
                if (StringUtil::CompareNoCase(entry.first, msg.sVirtualWnd) == 0) {
                    CNotifyPump* pObject = entry.second;
                    if (pObject && pObject->LoopDispatch(msg))
                        return;
                }
            }
        }

        ///
        //閬嶅巻涓荤獥鍙?
        LoopDispatch(msg);
    }

    //////////////////////////////////////////////////////////////////////////
    ///
    CWindowWnd::CWindowWnd() : m_hWnd(NULL), m_OldWndProc(::DefWindowProc), m_bSubclassed(false), m_bUnicode(false), m_bFakeModal(false)
    {
    }

    HWND CWindowWnd::GetHWND() const
    {
        return m_hWnd;
    }

    UINT CWindowWnd::GetClassStyle() const
    {
        return 0;
    }

    std::wstring_view CWindowWnd::GetSuperClassName() const
    {
        return {};
    }

    CWindowWnd::operator HWND() const
    {
        return m_hWnd;
    }
    void CWindowWnd::EnableUnicode()
    {
        m_bUnicode = true;
    }
    HWND CWindowWnd::CreateDuiWindow(HWND hwndParent, std::wstring_view pstrWindowName, DWORD dwStyle /*=0*/, DWORD dwExStyle /*=0*/)
    {
        return Create(hwndParent, pstrWindowName, dwStyle, dwExStyle, 0, 0, 0, 0, NULL);
    }

    HWND CWindowWnd::Create(HWND hwndParent, std::wstring_view pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu)
    {
        return Create(hwndParent, pstrName, dwStyle, dwExStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hMenu);
    }

    HWND CWindowWnd::Create(HWND hwndParent, std::wstring_view pstrName, DWORD dwStyle, DWORD dwExStyle, int x, int y, int cx, int cy, HMENU hMenu)
    {
        const std::wstring_view superClassName = GetSuperClassName();
        if (!superClassName.empty() && !RegisterSuperclass()) return NULL;
        if (superClassName.empty() && !RegisterWindowClass()) return NULL;

        const std::wstring classNameStorage(GetWindowClassName());
        const std::wstring nameStorage(pstrName);
        const wchar_t* classNamePtr = classNameStorage.empty() ? nullptr : classNameStorage.c_str();
        const wchar_t* namePtr = nameStorage.empty() ? nullptr : nameStorage.c_str();
        HINSTANCE hInstance = CPaintManagerUI::GetInstance();
        if (hInstance == NULL) hInstance = ::GetModuleHandleW(NULL);
        m_hWnd = ::CreateWindowExW(dwExStyle, classNamePtr, namePtr, dwStyle, x, y, cx, cy, hwndParent, hMenu, 
            hInstance, this);
        ASSERT(m_hWnd != NULL);
        return m_hWnd;
    }

    HWND CWindowWnd::Subclass(HWND hWnd)
    {
        ASSERT(::IsWindow(hWnd));
        ASSERT(m_hWnd == NULL);
        m_OldWndProc = SubclassWindow(hWnd, __WndProc);
        if (m_OldWndProc == NULL) return NULL;
        m_bSubclassed = true;
        m_hWnd = hWnd;
        ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(this));
        return m_hWnd;
    }

    void CWindowWnd::Unsubclass()
    {
        ASSERT(::IsWindow(m_hWnd));
        if (!::IsWindow(m_hWnd)) return;
        if (!m_bSubclassed) return;
        SubclassWindow(m_hWnd, m_OldWndProc);
        m_OldWndProc = ::DefWindowProc;
        m_bSubclassed = false;
    }

    void CWindowWnd::ShowWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
    {
        ASSERT(::IsWindow(m_hWnd));
        if (!::IsWindow(m_hWnd)) return;
        ::ShowWindow(m_hWnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
    }

    UINT CWindowWnd::ShowModal()
    {
        ASSERT(::IsWindow(m_hWnd));
        UINT nRet = 0;
        HWND hWndParent = GetWindowOwner(m_hWnd);
        ::ShowWindow(m_hWnd, SW_SHOWNORMAL);
        ::EnableWindow(hWndParent, FALSE);
        MSG msg = { 0 };
        while (::IsWindow(m_hWnd) && ::GetMessage(&msg, NULL, 0, 0)) {
            if (msg.message == WM_CLOSE && msg.hwnd == m_hWnd) {
                nRet = (int)msg.wParam;
                ::EnableWindow(hWndParent, TRUE);
                ::SetFocus(hWndParent);
            }
            if (!CPaintManagerUI::TranslateMessage(&msg)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
            if (msg.message == WM_QUIT) break;
        }
        ::EnableWindow(hWndParent, TRUE);
        ::SetFocus(hWndParent);
        if (msg.message == WM_QUIT) ::PostQuitMessage((int)msg.wParam);
        return nRet;
    }

    void CWindowWnd::ShowModalFake()
    {
        ASSERT(::IsWindow(m_hWnd));
        auto p_hwnd = GetWindowOwner(m_hWnd);
        ASSERT(::IsWindow(p_hwnd));
        ASSERT(p_hwnd);
        ::EnableWindow(p_hwnd, FALSE);
        ShowWindow();
        m_bFakeModal = true;
    }

    void CWindowWnd::Close(UINT nRet)
    {
        if (m_bFakeModal)
        {
            auto parent_hwnd = GetWindowOwner(m_hWnd);
            ASSERT(::IsWindow(parent_hwnd));
            ::EnableWindow(parent_hwnd, TRUE);
            ::SetFocus(parent_hwnd);
            m_bFakeModal = false;
        }

        ASSERT(::IsWindow(m_hWnd));
        if (!::IsWindow(m_hWnd)) return;
        PostMessage(WM_CLOSE, (WPARAM)nRet, 0L);
    }

    void CWindowWnd::CenterWindow()
    {
        ASSERT(::IsWindow(m_hWnd));
        ASSERT((GetWindowStyle(m_hWnd) & WS_CHILD) == 0);
        RECT rcDlg = { 0 };
        ::GetWindowRect(m_hWnd, &rcDlg);
        RECT rcArea = { 0 };
        RECT rcCenter = { 0 };
        HWND hWnd = *this;
        HWND hWndParent = ::GetParent(m_hWnd);
        HWND hWndCenter = ::GetWindowOwner(m_hWnd);
        if (hWndCenter != NULL)
            hWnd = hWndCenter;

        // 澶勭悊澶氭樉绀哄櫒妯″紡涓嬪睆骞曞眳涓?
        MONITORINFO oMonitor = {};
        oMonitor.cbSize = sizeof(oMonitor);
        ::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
        rcArea = oMonitor.rcWork;

        if (hWndCenter == NULL)
            rcCenter = rcArea;
        else
            ::GetWindowRect(hWndCenter, &rcCenter);

        int DlgWidth = rcDlg.right - rcDlg.left;
        int DlgHeight = rcDlg.bottom - rcDlg.top;

        // Find dialog's upper left based on rcCenter
        int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
        int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

        // The dialog is outside the screen, move it inside
        if (xLeft < rcArea.left) xLeft = rcArea.left;
        else if (xLeft + DlgWidth > rcArea.right) xLeft = rcArea.right - DlgWidth;
        if (yTop < rcArea.top) yTop = rcArea.top;
        else if (yTop + DlgHeight > rcArea.bottom) yTop = rcArea.bottom - DlgHeight;
        ::SetWindowPos(m_hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

void CWindowWnd::SetIcon(UINT nRes)
{
	HICON hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
		(::GetSystemMetrics(SM_CXICON) + 15) & ~15, (::GetSystemMetrics(SM_CYICON) + 15) & ~15,	// 闃叉楂楧PI涓嬪浘鏍囨ā绯?
		LR_DEFAULTCOLOR);
	ASSERT(hIcon);
	::SendMessage(m_hWnd, WM_SETICON, (WPARAM) TRUE, (LPARAM) hIcon);

	hIcon = (HICON)::LoadImage(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON,
		(::GetSystemMetrics(SM_CXICON) + 15) & ~15, (::GetSystemMetrics(SM_CYICON) + 15) & ~15,	// 闃叉楂楧PI涓嬪浘鏍囨ā绯?
		LR_DEFAULTCOLOR);
	ASSERT(hIcon);
	::SendMessage(m_hWnd, WM_SETICON, (WPARAM) FALSE, (LPARAM) hIcon);
}

    bool CWindowWnd::RegisterWindowClass()
    {
        const std::wstring classNameStorage(GetWindowClassName());
        HINSTANCE hInstance = CPaintManagerUI::GetInstance();
        if (hInstance == NULL) hInstance = ::GetModuleHandleW(NULL);
        WNDCLASSW wc = { 0 };
        wc.style = GetClassStyle();
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hIcon = NULL;
        wc.lpfnWndProc = CWindowWnd::__WndProc;
        wc.hInstance = hInstance;
        wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = classNameStorage.empty() ? nullptr : classNameStorage.c_str();
        ATOM ret = ::RegisterClassW(&wc);
        ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
        return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    bool CWindowWnd::RegisterSuperclass()
    {
        const std::wstring superClassNameStorage(GetSuperClassName());
        const std::wstring classNameStorage(GetWindowClassName());
        HINSTANCE hInstance = CPaintManagerUI::GetInstance();
        if (hInstance == NULL) hInstance = ::GetModuleHandleW(NULL);

        // Get the class information from an existing
        // window so we can subclass it later on...
        WNDCLASSEXW wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEXW);
        if (!::GetClassInfoExW(NULL, superClassNameStorage.c_str(), &wc)) {
            if (!::GetClassInfoExW(hInstance, superClassNameStorage.c_str(), &wc)) {
                ASSERT(!"Unable to locate window class");
                return NULL;
            }
        }
        m_OldWndProc = wc.lpfnWndProc;
        wc.lpfnWndProc = CWindowWnd::__ControlProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = classNameStorage.empty() ? nullptr : classNameStorage.c_str();
        ATOM ret = ::RegisterClassExW(&wc);
        ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
        return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    LRESULT CALLBACK CWindowWnd::__WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CWindowWnd* pThis = NULL;
        if (uMsg == WM_NCCREATE) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
            pThis->m_hWnd = hWnd;
            ::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
        }
        else {
            pThis = reinterpret_cast<CWindowWnd*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
            if (uMsg == WM_NCDESTROY && pThis != NULL) {
                LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
                ::SetWindowLongPtr(pThis->m_hWnd, GWLP_USERDATA, 0L);
                if (pThis->m_bSubclassed) pThis->Unsubclass();
                pThis->m_hWnd = NULL;
                pThis->OnFinalMessage(hWnd);
                return lRes;
            }
        }
        if (pThis != NULL) {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else {
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    LRESULT CALLBACK CWindowWnd::__ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CWindowWnd* pThis = NULL;
        if (uMsg == WM_NCCREATE) {
            LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
            pThis = static_cast<CWindowWnd*>(lpcs->lpCreateParams);
            ::SetProp(hWnd, _T("WndX"), (HANDLE)pThis);
            pThis->m_hWnd = hWnd;
        }
        else {
            pThis = reinterpret_cast<CWindowWnd*>(::GetProp(hWnd, _T("WndX")));
            if (uMsg == WM_NCDESTROY && pThis != NULL) {
                LRESULT lRes = ::CallWindowProc(pThis->m_OldWndProc, hWnd, uMsg, wParam, lParam);
                if (pThis->m_bSubclassed) pThis->Unsubclass();
                ::SetProp(hWnd, _T("WndX"), NULL);
                pThis->m_hWnd = NULL;
                pThis->OnFinalMessage(hWnd);
                return lRes;
            }
        }
        if (pThis != NULL) {
            return pThis->HandleMessage(uMsg, wParam, lParam);
        }
        else {
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    LRESULT CWindowWnd::SendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
    {
        ASSERT(::IsWindow(m_hWnd));
        return ::SendMessage(m_hWnd, uMsg, wParam, lParam);
    }

    LRESULT CWindowWnd::PostMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
    {
        ASSERT(::IsWindow(m_hWnd));
        return ::PostMessage(m_hWnd, uMsg, wParam, lParam);
    }

    void CWindowWnd::ResizeClient(int cx /*= -1*/, int cy /*= -1*/)
    {
        ASSERT(::IsWindow(m_hWnd));
        RECT rc = { 0 };
        if (!::GetClientRect(m_hWnd, &rc)) return;
        if (cx != -1) rc.right = cx;
        if (cy != -1) rc.bottom = cy;
        if (!::AdjustWindowRectEx(&rc, GetWindowStyle(m_hWnd), (!(GetWindowStyle(m_hWnd) & WS_CHILD) && (::GetMenu(m_hWnd) != NULL)), GetWindowExStyle(m_hWnd))) return;
        ::SetWindowPos(m_hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
    }

    LRESULT CWindowWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (m_hWnd == NULL || !::IsWindow(m_hWnd)) {
            return 0;
        }
        WNDPROC oldWndProc = (m_OldWndProc != NULL) ? m_OldWndProc : ::DefWindowProc;
        return ::CallWindowProc(oldWndProc, m_hWnd, uMsg, wParam, lParam);
    }

    void CWindowWnd::OnFinalMessage(HWND /*hWnd*/)
    {
    }

} // namespace DuiLib


