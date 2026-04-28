#include "pch.h"
#include "UIRichEdit.h"
#include "../Core/UIRenderBatchInternal.h"
#include "../Core/UIRenderContext.h"

#ifdef _USEIMM
#include <imm.h>
#pragma comment(lib, "imm32.lib")
#endif
// These constants are for backward compatibility. They are the 
// sizes used for initialization and reset in RichEdit 1.0

namespace FYUI {

#define ID_RICH_UNDO			101
#define ID_RICH_CUT				102
#define ID_RICH_COPY			103
#define ID_RICH_PASTE			104
#define ID_RICH_CLEAR			105
#define ID_RICH_SELECTALL		106
#define ID_RICH_REDO			107

	const LONG cInitTextMax = (32 * 1024) - 1;

	static HDC GetManagerMeasureDC(CPaintManagerUI* pManager)
	{
		if (pManager == NULL) {
			return NULL;
		}

		const RECT rcMeasure = { 0, 0, 1, 1 };
		CPaintRenderContext measureContext = pManager->CreateMeasureRenderContext(rcMeasure);
		return measureContext.GetDC();
	}

	static HDC GetManagerActiveOrMeasureDC(CPaintManagerUI* pManager)
	{
		if (pManager != NULL) {
			const CPaintRenderContext* pRenderContext = pManager->GetCurrentRenderContext();
			if (pRenderContext != NULL && pRenderContext->GetDC() != NULL) {
				return pRenderContext->GetDC();
			}
			return GetManagerMeasureDC(pManager);
		}
		return NULL;
	}

	static HWND GetRichEditPaintWindow(const CPaintManagerUI* pManager)
	{
		return pManager != NULL ? pManager->GetPaintWindow() : NULL;
	}

	static HWND GetRichEditPaintWindow(const CRichEditUI* pRichEdit)
	{
		return pRichEdit != NULL ? GetRichEditPaintWindow(pRichEdit->GetManager()) : NULL;
	}

	static bool ScreenToRichEditClient(CPaintManagerUI* pManager, POINT& pt)
	{
		HWND hWnd = GetRichEditPaintWindow(pManager);
		return hWnd != NULL && ::ScreenToClient(hWnd, &pt) != FALSE;
	}

	static bool ClientToRichEditScreen(CPaintManagerUI* pManager, POINT& pt)
	{
		HWND hWnd = GetRichEditPaintWindow(pManager);
		return hWnd != NULL && ::ClientToScreen(hWnd, &pt) != FALSE;
	}

	static bool FocusRichEditPaintWindow(CPaintManagerUI* pManager)
	{
		HWND hWnd = GetRichEditPaintWindow(pManager);
		if (hWnd == NULL) {
			return false;
		}
		::SetFocus(hWnd);
		return true;
	}

	static bool ResolveRichEditMessagePoint(CRichEditUI* pRichEdit, UINT uMsg, LPARAM lParam, POINT& pt)
	{
		if (pRichEdit == NULL || pRichEdit->GetManager() == NULL) {
			pt.x = 0;
			pt.y = 0;
			return false;
		}

		if (uMsg == WM_SETCURSOR) {
			return ::GetCursorPos(&pt) != FALSE && ScreenToRichEditClient(pRichEdit->GetManager(), pt);
		}

		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		if (uMsg == WM_MOUSEWHEEL || uMsg == WM_CONTEXTMENU) {
			return ScreenToRichEditClient(pRichEdit->GetManager(), pt);
		}
		return true;
	}

	static bool ResolveRichEditHoverControl(CRichEditUI* pRichEdit, UINT uMsg, LPARAM lParam, POINT& pt, CControlUI*& pHover)
	{
		pHover = NULL;
		if (pRichEdit == NULL || pRichEdit->GetManager() == NULL) {
			pt.x = 0;
			pt.y = 0;
			return false;
		}

		if (!ResolveRichEditMessagePoint(pRichEdit, uMsg, lParam, pt)) {
			return false;
		}
		pHover = pRichEdit->GetManager()->FindControl(pt);
		return true;
	}

	static bool IsRichEditMouseMessage(UINT uMsg)
	{
		return (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || uMsg == WM_SETCURSOR;
	}

	static bool IsRichEditKeyboardMessage(UINT uMsg)
	{
#ifdef _UNICODE
		return uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST;
#else
		return (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST) || uMsg == WM_CHAR || uMsg == WM_IME_CHAR;
#endif
	}

	static bool ShouldIgnoreRichEditMessage(CRichEditUI* pRichEdit, UINT uMsg, WPARAM wParam)
	{
		if (pRichEdit == NULL || !pRichEdit->IsVisible() || !pRichEdit->IsEnabled()) {
			return true;
		}
		if (!pRichEdit->IsMouseEnabled() && uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) {
			return true;
		}
		return uMsg == WM_MOUSEWHEEL && (LOWORD(wParam) & MK_CONTROL) == 0;
	}

	static bool PrepareRichEditMouseMessage(CRichEditUI* pRichEdit, UINT uMsg, LPARAM lParam, bool& bWasHandled);
	static bool HandleRichEditCommandMessageDispatch(CRichEditUI* pRichEdit, WPARAM wParam, bool& bHandled);
	static bool HandleRichEditContextMenuMessage(CRichEditUI* pRichEdit, LPARAM lParam, bool& bWasHandled);
	static bool HandleRichEditImeCompositionMessage(CRichEditUI* pRichEdit);
	static bool HandleRichEditImeStartCompositionMessage(CRichEditUI* pRichEdit, CTxtWinHost* pTextHost);

	static bool HandleRichEditCommandMessage(CRichEditUI* pRichEdit, UINT uCmd)
	{
		if (pRichEdit == NULL) {
			return false;
		}

		switch(uCmd) {
		case ID_RICH_UNDO:
			pRichEdit->Undo();
			return true;
		case ID_RICH_REDO:
			pRichEdit->Redo();
			return true;
		case ID_RICH_CUT:
			pRichEdit->Cut();
			return true;
		case ID_RICH_COPY:
			pRichEdit->Copy();
			return true;
		case ID_RICH_PASTE:
			pRichEdit->Paste();
			return true;
		case ID_RICH_CLEAR:
			pRichEdit->Clear();
			return true;
		case ID_RICH_SELECTALL:
			pRichEdit->SetSelAll();
			return true;
		default:
			return false;
		}
	}

	static bool ShowRichEditContextMenu(CRichEditUI* pRichEdit, LPARAM lParam)
	{
		if (pRichEdit == NULL || pRichEdit->GetManager() == NULL || !pRichEdit->IsContextMenuUsed()) {
			return false;
		}

		POINT pt = {};
		CControlUI* pHover = NULL;
		if (!ResolveRichEditHoverControl(pRichEdit, WM_CONTEXTMENU, lParam, pt, pHover) || pHover != pRichEdit) {
			return false;
		}

		HMENU hPopMenu = CreatePopupMenu();
		AppendMenu(hPopMenu, 0, ID_RICH_UNDO, _T("閹俱倝鏀?&U)"));
		AppendMenu(hPopMenu, 0, ID_RICH_REDO, _T("闁插秴浠?&R)"));
		AppendMenu(hPopMenu, MF_SEPARATOR, 0, _T(""));
		AppendMenu(hPopMenu, 0, ID_RICH_CUT, _T("閸擃亜鍨?&X)"));
		AppendMenu(hPopMenu, 0, ID_RICH_COPY, _T("婢跺秴鍩?&C)"));
		AppendMenu(hPopMenu, 0, ID_RICH_PASTE, _T("缁ê绗?&V)"));
		AppendMenu(hPopMenu, 0, ID_RICH_CLEAR, _T("濞撳懐鈹?&L)"));
		AppendMenu(hPopMenu, MF_SEPARATOR, 0, _T(""));
		AppendMenu(hPopMenu, 0, ID_RICH_SELECTALL, _T("閸忋劑鈧?&A)"));

		UINT uUndo = (pRichEdit->CanUndo() ? 0 : MF_GRAYED);
		EnableMenuItem(hPopMenu, ID_RICH_UNDO, MF_BYCOMMAND | uUndo);
		UINT uRedo = (pRichEdit->CanRedo() ? 0 : MF_GRAYED);
		EnableMenuItem(hPopMenu, ID_RICH_REDO, MF_BYCOMMAND | uRedo);
		UINT uSel = ((pRichEdit->GetSelectionType() != SEL_EMPTY) ? 0 : MF_GRAYED);
		UINT uReadonly = pRichEdit->IsReadOnly() ? MF_GRAYED : 0;
		EnableMenuItem(hPopMenu, ID_RICH_CUT, MF_BYCOMMAND | uSel | uReadonly);
		EnableMenuItem(hPopMenu, ID_RICH_COPY, MF_BYCOMMAND | uSel);
		EnableMenuItem(hPopMenu, ID_RICH_CLEAR, MF_BYCOMMAND | uSel | uReadonly);
		EnableMenuItem(hPopMenu, ID_RICH_PASTE, MF_BYCOMMAND | uReadonly);

		ClientToRichEditScreen(pRichEdit->GetManager(), pt);
		TrackPopupMenu(hPopMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, GetRichEditPaintWindow(pRichEdit), NULL);
		DestroyMenu(hPopMenu);
		return true;
	}

	static void GetManagerLogPixels(CPaintManagerUI* pManager, LONG& xPerInch, LONG& yPerInch)
	{
		xPerInch = 96;
		yPerInch = 96;
		HDC hMeasureDC = GetManagerMeasureDC(pManager);
		if (hMeasureDC != NULL) {
			xPerInch = ::GetDeviceCaps(hMeasureDC, LOGPIXELSX);
			yPerInch = ::GetDeviceCaps(hMeasureDC, LOGPIXELSY);
			if (xPerInch == 0) xPerInch = 96;
			if (yPerInch == 0) yPerInch = 96;
		}
	}

	class CTxtWinHost;
	static bool IsRichEditPaintWindowFocused(CPaintManagerUI* pManager);
	static bool IsRichEditCaretVisibleInAncestors(RECT& rcCaret, CControlUI* pControl);
	static void InvalidateLayeredRichEditCaret(CTxtWinHost* pTextHost, CPaintManagerUI* pManager, const RECT& rcItem, CControlUI* pControl, bool& bDrawCaret);
	static void ToggleRichEditCaretVisibility(CTxtWinHost* pTextHost);
	static void HandleRichEditDefaultTimer(CTxtWinHost* pTextHost, CPaintManagerUI* pManager, bool bFocused, bool& bDrawCaret, const RECT& rcItem, CControlUI* pControl);
	static bool TryMeasureRichEditNaturalSize(CTxtWinHost* pTextHost, HDC hMeasureDC, LONG& lWidth, LONG& lHeight);
	static bool TryMeasureRichEditNaturalSizeForManager(CTxtWinHost* pTextHost, CPaintManagerUI* pManager, LONG& lWidth, LONG& lHeight);

	EXTERN_C const IID IID_ITextServices = { // 8d33f740-cf58-11ce-a89d-00aa006cadc5
		0x8d33f740,
		0xcf58,
		0x11ce,
		{0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
	};

	EXTERN_C const IID IID_ITextHost = { /* c5bdd8d0-d26e-11ce-a89e-00aa006cadc5 */
		0xc5bdd8d0,
		0xd26e,
		0x11ce,
		{0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}
	};

#ifndef LY_PER_INCH
#define LY_PER_INCH 1440
#endif

#ifndef HIMETRIC_PER_INCH
#define HIMETRIC_PER_INCH 2540
#endif

#include <textserv.h>

	class CTxtWinHost : public ITextHost
	{
	public:
		CTxtWinHost();
		BOOL Init(CRichEditUI *re , const CREATESTRUCT *pcs);
		virtual ~CTxtWinHost();

		ITextServices* GetTextServices(void) { return pserv; }
		void SetClientRect(RECT *prc);
		RECT* GetClientRect() { return &rcClient; }
		BOOL IsWordWrap(void) { return fWordWrap; }
		void SetWordWrap(BOOL fWordWrap);
		BOOL IsReadOnly();
		void SetReadOnly(BOOL fReadOnly);

		void SetFont(HFONT hFont);
		void SetColor(DWORD dwColor);
		SIZEL* GetExtent();
		void SetExtent(SIZEL *psizelExtent);
		void LimitText(LONG nChars);
		BOOL IsCaptured();
		BOOL IsShowCaret();
		void NeedFreshCaret();
		INT GetCaretWidth();
		INT GetCaretHeight();

		BOOL GetAllowBeep();
		void SetAllowBeep(BOOL fAllowBeep);
		WORD GetDefaultAlign();
		void SetDefaultAlign(WORD wNewAlign);
		BOOL GetRichTextFlag();
		void SetRichTextFlag(BOOL fNew);
		LONG GetDefaultLeftIndent();
		void SetDefaultLeftIndent(LONG lNewIndent);
		BOOL SetSaveSelection(BOOL fSaveSelection);
		HRESULT OnTxInPlaceDeactivate();
		HRESULT OnTxInPlaceActivate(LPCRECT prcClient);
		BOOL GetActiveState(void) { return fInplaceActive; }
		BOOL DoSetCursor(RECT *prc, POINT *pt);
		void SetTransparent(BOOL fTransparent);
		void GetControlRect(LPRECT prc);
		LONG SetAccelPos(LONG laccelpos);
		WCHAR SetPasswordChar(WCHAR chPasswordChar);
		void SetDisabled(BOOL fOn);
		LONG SetSelBarWidth(LONG lSelBarWidth);
		BOOL GetTimerState();

		void SetCharFormat(CHARFORMAT2W &c);
		void SetParaFormat(PARAFORMAT2 &p);

		// -----------------------------
		//	IUnknown interface
		// -----------------------------
		virtual HRESULT _stdcall QueryInterface(REFIID riid, void **ppvObject);
		virtual ULONG _stdcall AddRef(void);
		virtual ULONG _stdcall Release(void);

		// -----------------------------
		//	ITextHost interface
		// -----------------------------
		virtual HDC TxGetDC();
		virtual INT TxReleaseDC(HDC hdc);
		virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow);
		virtual BOOL TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags);
		virtual BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw);
		virtual BOOL TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw);
		virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode);
		virtual void TxViewChange(BOOL fUpdate);
		virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);
		virtual BOOL TxShowCaret(BOOL fShow);
		virtual BOOL TxSetCaretPos(INT x, INT y);
		virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout);
		virtual void TxKillTimer(UINT idTimer);
		virtual void TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll);
		virtual void TxSetCapture(BOOL fCapture);
		virtual void TxSetFocus();
		virtual void TxSetCursor(HCURSOR hcur, BOOL fText);
		virtual BOOL TxScreenToClient (LPPOINT lppt);
		virtual BOOL TxClientToScreen (LPPOINT lppt);
		virtual HRESULT TxActivate( LONG * plOldState );
		virtual HRESULT TxDeactivate( LONG lNewState );
		virtual HRESULT TxGetClientRect(LPRECT prc);
		virtual HRESULT TxGetViewInset(LPRECT prc);
		virtual HRESULT TxGetCharFormat(const CHARFORMATW **ppCF );
		virtual HRESULT TxGetParaFormat(const PARAFORMAT **ppPF);
		virtual COLORREF TxGetSysColor(int nIndex);
		virtual HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle);
		virtual HRESULT TxGetMaxLength(DWORD *plength);
		virtual HRESULT TxGetScrollBars(DWORD *pdwScrollBar);
		virtual HRESULT TxGetPasswordChar(wchar_t *pch);
		virtual HRESULT TxGetAcceleratorPos(LONG *pcp);
		virtual HRESULT TxGetExtent(LPSIZEL lpExtent);
		virtual HRESULT OnTxCharFormatChange (const CHARFORMATW * pcf);
		virtual HRESULT OnTxParaFormatChange (const PARAFORMAT * ppf);
		virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);
		virtual HRESULT TxNotify(DWORD iNotify, void *pv);
		virtual HIMC TxImmGetContext(void);
		virtual void TxImmReleaseContext(HIMC himc);
		virtual HRESULT TxGetSelectionBarWidth (LONG *lSelBarWidth);

	private:
		CRichEditUI *m_re;
		ULONG	cRefs;					// Reference Count
		ITextServices	*pserv;		    // pointer to Text Services object
		// Properties

		DWORD		dwStyle;				// style bits

		unsigned	fEnableAutoWordSel	:1;	// enable Word style auto word selection?
		unsigned	fWordWrap			:1;	// Whether control should word wrap
		unsigned	fAllowBeep			:1;	// Whether beep is allowed
		unsigned	fRich				:1;	// Whether control is rich text
		unsigned	fSaveSelection		:1;	// Whether to save the selection when inactive
		unsigned	fInplaceActive		:1; // Whether control is inplace active
		unsigned	fTransparent		:1; // Whether control is transparent
		unsigned	fTimer				:1;	// A timer is set
		unsigned    fCaptured           :1;
		unsigned    fShowCaret          :1;
		unsigned    fNeedFreshCaret     :1; // 娣囶喗顒滈弨鐟板綁婢堆冪毈閸氬海鍋ｉ崙璇插従娴犳牔缍呯純顔煎斧閺夈儱鍘滈弽鍥︾瑝閼宠姤绉烽梽銈囨畱闂傤噣顣?

		INT         iCaretWidth;
		INT         iCaretHeight;
		INT         iCaretLastWidth;
		INT         iCaretLastHeight;
		LONG		lSelBarWidth;			// Width of the selection bar
		LONG  		cchTextMost;			// maximum text size
		DWORD		dwEventMask;			// DoEvent mask to pass on to parent window
		LONG		icf;
		LONG		ipf;
		RECT		rcClient;				// Client Rect for this control
		SIZEL		sizelExtent;			// Extent array
		CHARFORMAT2W cf;					// Default character format
		PARAFORMAT2	pf;					    // Default paragraph format
		LONG		laccelpos;				// Accelerator position
		WCHAR		chPasswordChar;		    // Password character
	};

	static bool PrepareRichEditMouseMessage(CRichEditUI* pRichEdit, UINT uMsg, LPARAM lParam, bool& bWasHandled)
	{
		if (pRichEdit == NULL || pRichEdit->GetManager() == NULL || pRichEdit->GetTextHost() == NULL) {
			return false;
		}

		CTxtWinHost* pTextHost = pRichEdit->GetTextHost();
		if (!pTextHost->IsCaptured()) {
			switch (uMsg) {
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MOUSEMOVE:
			{
				POINT pt = {};
				CControlUI* pHover = NULL;
				if (!ResolveRichEditHoverControl(pRichEdit, uMsg, lParam, pt, pHover) || pHover != pRichEdit) {
					bWasHandled = false;
					return false;
				}
				break;
			}
			}
		}

		DWORD dwHitResult = pTextHost->IsCaptured() ? HITRESULT_HIT : HITRESULT_OUTSIDE;
		if (dwHitResult == HITRESULT_OUTSIDE) {
			RECT rc = {};
			pTextHost->GetControlRect(&rc);

			POINT pt = {};
			ResolveRichEditMessagePoint(pRichEdit, uMsg, lParam, pt);

			if (::PtInRect(&rc, pt) && !pRichEdit->GetManager()->IsCaptured()) {
				dwHitResult = HITRESULT_HIT;
			}
		}
		if (dwHitResult != HITRESULT_HIT) {
			return false;
		}

		if (uMsg == WM_SETCURSOR || uMsg == WM_MOUSEMOVE) {
			bWasHandled = false;
		}
		else if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK || uMsg == WM_RBUTTONDOWN) {
			if (!pRichEdit->GetManager()->IsNoActivate()) {
				FocusRichEditPaintWindow(pRichEdit->GetManager());
			}
			pRichEdit->SetFocus();
		}
		return true;
	}

	static bool HandleRichEditCommandMessageDispatch(CRichEditUI* pRichEdit, WPARAM wParam, bool& bHandled)
	{
		bHandled = FALSE;
		if (pRichEdit == NULL || !pRichEdit->IsFocused()) {
			return false;
		}
		HandleRichEditCommandMessage(pRichEdit, static_cast<UINT>(wParam));
		return true;
	}

	static bool HandleRichEditContextMenuMessage(CRichEditUI* pRichEdit, LPARAM lParam, bool& bWasHandled)
	{
		if (ShowRichEditContextMenu(pRichEdit, lParam)) {
			return true;
		}
		bWasHandled = false;
		return false;
	}

	static bool PrepareRichEditMessageDispatch(
		CRichEditUI* pRichEdit,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam,
		bool& bHandled,
		bool& bWasHandled)
	{
		if (uMsg == WM_IME_COMPOSITION) {
			HandleRichEditImeCompositionMessage(pRichEdit);
			return false;
		}
		if (IsRichEditMouseMessage(uMsg)) {
			return PrepareRichEditMouseMessage(pRichEdit, uMsg, lParam, bWasHandled);
		}
		if (IsRichEditKeyboardMessage(uMsg)) {
			return pRichEdit != NULL && pRichEdit->IsFocused();
		}
#ifdef _USEIMM
		if (uMsg == WM_IME_STARTCOMPOSITION) {
			HandleRichEditImeStartCompositionMessage(pRichEdit, pRichEdit != NULL ? pRichEdit->GetTextHost() : NULL);
			bWasHandled = false;
			return false;
		}
#endif
		if (uMsg == WM_CONTEXTMENU) {
			return HandleRichEditContextMenuMessage(pRichEdit, lParam, bWasHandled);
		}
		if (uMsg == WM_COMMAND) {
			return HandleRichEditCommandMessageDispatch(pRichEdit, wParam, bHandled);
		}
		if (uMsg == WM_HELP) {
			bWasHandled = false;
			return true;
		}
		return false;
	}

	static ITextServices* GetRichEditTextServices(CTxtWinHost* pTextHost)
	{
		return pTextHost != NULL ? pTextHost->GetTextServices() : NULL;
	}

	static HRESULT SendRichEditTextServiceMessage(CTxtWinHost* pTextHost, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
	{
		ITextServices* pTextServices = GetRichEditTextServices(pTextHost);
		if (pTextServices == NULL) {
			return S_FALSE;
		}
		return pTextServices->TxSendMessage(msg, wParam, lParam, pResult);
	}

	static void NotifyRichEditTextServiceFocus(CTxtWinHost* pTextHost, UINT uMsg)
	{
		if (pTextHost == NULL) {
			return;
		}
		pTextHost->OnTxInPlaceActivate(NULL);
		SendRichEditTextServiceMessage(pTextHost, uMsg, 0, 0, 0);
	}

	static void ApplyRichEditFocusState(CTxtWinHost* pTextHost, CPaintManagerUI* pManager, bool bFocused, UINT nTimerId, bool& bFocusedState, CRichEditUI* pRichEdit)
	{
		if (pTextHost != NULL) {
			NotifyRichEditTextServiceFocus(pTextHost, bFocused ? WM_SETFOCUS : WM_KILLFOCUS);
		}
		if (pManager != NULL) {
			if (bFocused) pManager->SetTimer(pRichEdit, nTimerId, ::GetCaretBlinkTime());
			else pManager->KillTimer(pRichEdit);
		}
		bFocusedState = bFocused;
		if (pRichEdit != NULL) {
			pRichEdit->Invalidate();
		}
	}

	static bool IsRichEditPaintWindowFocused(CPaintManagerUI* pManager)
	{
		return pManager != NULL && ::GetFocus() == GetRichEditPaintWindow(pManager);
	}

	static bool TryGetRichEditCaretPos(POINT& ptCaret)
	{
		ptCaret.x = 0;
		ptCaret.y = 0;
		return ::GetCaretPos(&ptCaret) != FALSE;
	}

	static bool TryGetRichEditCaretRect(CTxtWinHost* pTextHost, RECT& rcCaret)
	{
		if (pTextHost == NULL) {
			return false;
		}

		POINT ptCaret = {};
		if (!TryGetRichEditCaretPos(ptCaret)) {
			return false;
		}

		rcCaret.left = ptCaret.x;
		rcCaret.top = ptCaret.y;
		rcCaret.right = ptCaret.x + pTextHost->GetCaretWidth();
		rcCaret.bottom = ptCaret.y + pTextHost->GetCaretHeight();
		return true;
	}

	static void UpdateRichEditImeCompositionWindow(HWND hWnd, const POINT& ptCaret, DWORD dwStyle, LONG xOffset, HFONT hFont = NULL)
	{
		if (hWnd == NULL) {
			return;
		}

		HIMC hIMC = ::ImmGetContext(hWnd);
		if (hIMC == NULL) {
			return;
		}

		COMPOSITIONFORM composition = {};
		composition.dwStyle = dwStyle;
		composition.ptCurrentPos.x = ptCaret.x + xOffset;
		composition.ptCurrentPos.y = ptCaret.y;
		::ImmSetCompositionWindow(hIMC, &composition);

		if (hFont != NULL) {
			LOGFONT lf = {};
			if (::GetObject(hFont, sizeof(LOGFONT), &lf) == sizeof(LOGFONT)) {
				::ImmSetCompositionFont(hIMC, &lf);
			}
		}

		::ImmReleaseContext(hWnd, hIMC);
	}

	static bool HandleRichEditImeCompositionMessage(CRichEditUI* pRichEdit)
	{
		if (pRichEdit == NULL || pRichEdit->GetManager() == NULL) {
			return false;
		}

		POINT point = {};
		if (TryGetRichEditCaretPos(point)) {
			UpdateRichEditImeCompositionWindow(GetRichEditPaintWindow(pRichEdit), point, CFS_POINT, 0);
		}
		return true;
	}

	static bool HandleRichEditImeStartCompositionMessage(CRichEditUI* pRichEdit, CTxtWinHost* pTextHost)
	{
		if (pRichEdit == NULL || !pRichEdit->IsFocused() || pRichEdit->GetManager() == NULL || pTextHost == NULL) {
			return false;
		}

		POINT ptCaret = {};
		if (!TryGetRichEditCaretPos(ptCaret)) {
			return true;
		}

		UpdateRichEditImeCompositionWindow(
			GetRichEditPaintWindow(pRichEdit),
			ptCaret,
			CFS_FORCE_POSITION,
			pTextHost->GetCaretWidth(),
			pRichEdit->GetManager()->GetFont(pRichEdit->GetFont()));
		return true;
	}

	static void NotifyRichEditHostWindow(CRichEditUI* pRichEdit, DWORD iNotify, void* pv)
	{
		if (pRichEdit == NULL || pRichEdit->GetManager() == NULL || pv == NULL) {
			return;
		}

		HWND hWnd = GetRichEditPaintWindow(pRichEdit);
		if (hWnd == NULL) {
			return;
		}

		LONG nId = GetWindowLong(hWnd, GWL_ID);
		NMHDR* pHdr = static_cast<NMHDR*>(pv);
		pHdr->hwndFrom = hWnd;
		pHdr->idFrom = nId;
		pHdr->code = iNotify;
		::SendMessage(hWnd, WM_NOTIFY, static_cast<WPARAM>(nId), reinterpret_cast<LPARAM>(pv));
	}

	static bool IsRichEditCaretVisibleInAncestors(RECT& rcCaret, CControlUI* pControl)
	{
		if (pControl == NULL) {
			return false;
		}

		CControlUI* pParent = pControl;
		RECT rcTemp = rcCaret;
		RECT rcParent = {};
		while ((pParent = pParent->GetParent()) != NULL) {
			rcTemp = rcCaret;
			rcParent = pParent->GetPos();
			if (!::IntersectRect(&rcCaret, &rcTemp, &rcParent)) {
				return false;
			}
		}
		return true;
	}

	static void InvalidateLayeredRichEditCaret(CTxtWinHost* pTextHost, CPaintManagerUI* pManager, const RECT& rcItem, CControlUI* pControl, bool& bDrawCaret)
	{
		if (pTextHost == NULL || pManager == NULL || pControl == NULL || !pTextHost->IsShowCaret()) {
			return;
		}

		bDrawCaret = !bDrawCaret;
		RECT rcCaret = {};
		if (!TryGetRichEditCaretRect(pTextHost, rcCaret)) {
			return;
		}
		RECT rcTemp = rcCaret;
		if (!::IntersectRect(&rcCaret, &rcTemp, &rcItem)) {
			return;
		}
		if (!IsRichEditCaretVisibleInAncestors(rcCaret, pControl)) {
			return;
		}
		pManager->Invalidate(rcCaret);
	}

	static void ToggleRichEditCaretVisibility(CTxtWinHost* pTextHost)
	{
		if (pTextHost == NULL) {
			return;
		}
		if (pTextHost->IsShowCaret()) pTextHost->TxShowCaret(FALSE);
		else pTextHost->TxShowCaret(TRUE);
	}

	static void ForwardRichEditTextServiceTimer(CTxtWinHost* pTextHost, WPARAM wParam, LPARAM lParam)
	{
		if (pTextHost != NULL) {
			SendRichEditTextServiceMessage(pTextHost, WM_TIMER, wParam, lParam, 0);
		}
	}

	static void HandleRichEditDefaultTimer(CTxtWinHost* pTextHost, CPaintManagerUI* pManager, bool bFocused, bool& bDrawCaret, const RECT& rcItem, CControlUI* pControl)
	{
		if (!bFocused || pTextHost == NULL || !IsRichEditPaintWindowFocused(pManager)) {
			return;
		}

		if (pManager != NULL && pManager->IsLayered()) {
			InvalidateLayeredRichEditCaret(pTextHost, pManager, rcItem, pControl, bDrawCaret);
			return;
		}

		ToggleRichEditCaretVisibility(pTextHost);
	}

	static void DrawLayeredRichEditCaret(HDC hDC, CPaintManagerUI* pManager, bool bFocused, bool bDrawCaret, CTxtWinHost* pTextHost, const RECT& rcItem, DWORD dwTextColor)
	{
		if (hDC == NULL || pManager == NULL || !pManager->IsLayered() || !bFocused || !bDrawCaret || pTextHost == NULL || !pTextHost->IsShowCaret()) {
			return;
		}

		POINT ptCaret = {};
		if (!TryGetRichEditCaretPos(ptCaret)) {
			return;
		}
		if (!::PtInRect(&rcItem, ptCaret)) {
			return;
		}

		RECT rcCaret = { ptCaret.x, ptCaret.y, ptCaret.x, ptCaret.y + pTextHost->GetCaretHeight() };
		CPaintRenderContext renderContext(nullptr, hDC, rcCaret);
		CRenderEngine::DrawLine(renderContext, rcCaret, pTextHost->GetCaretWidth(), dwTextColor);
	}

	static CPaintRenderContext CreateRichEditRenderContext(CPaintManagerUI* pManager, HDC hDC, const RECT& rcPaint)
	{
		if (pManager != NULL) {
			if (const CPaintRenderContext* pCurrentRenderContext = pManager->GetCurrentRenderContext()) {
				if (pCurrentRenderContext->GetDC() == hDC) {
					return CPaintRenderContext(
						pManager,
						hDC,
						pCurrentRenderContext->GetPaintRect(),
						pCurrentRenderContext->GetActiveBackend(),
						pCurrentRenderContext->GetActiveDirect2DRenderMode());
				}
			}
			return CPaintRenderContext(
				pManager,
				hDC,
				rcPaint,
				pManager->GetActiveRenderBackend(),
				pManager->GetActiveDirect2DRenderMode());
		}
		return CPaintRenderContext(nullptr, hDC, rcPaint);
	}

	// Convert Pixels on the X axis to Himetric
	LONG DXtoHimetricX(LONG dx, LONG xPerInch)
	{
		return (LONG) MulDiv(dx, HIMETRIC_PER_INCH, xPerInch);
	}

	// Convert Pixels on the Y axis to Himetric
	LONG DYtoHimetricY(LONG dy, LONG yPerInch)
	{
		return (LONG) MulDiv(dy, HIMETRIC_PER_INCH, yPerInch);
	}

	HRESULT InitDefaultCharFormat(CRichEditUI* re, CHARFORMAT2W* pcf, HFONT hfont) 
	{
		memset(pcf, 0, sizeof(CHARFORMAT2W));
		// 鐎瑰鍙忓Λ鈧弻銉窗Clone() 鐠侯垰绶炴稉?Manager 閸欘垵鍏樼亸姘弓鐠佸墽鐤?
		if(re->GetManager() == NULL) return E_FAIL;
		if(hfont == NULL) {
			hfont = re->GetManager()->GetFont(re->GetFont());
		}
		if(hfont == NULL) return E_FAIL;
		LOGFONT lf;
		::GetObject(hfont, sizeof(LOGFONT), &lf);

		DWORD dwColor = re->GetTextColor();
		if(re->GetManager()->IsLayered()) {
			CRenderEngine::CheckAlphaColor(dwColor);
		}
		pcf->cbSize = sizeof(CHARFORMAT2W);
		pcf->crTextColor = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
		LONG xPixPerInch = 96;
		LONG yPixPerInch = 96;
		GetManagerLogPixels(re->GetManager(), xPixPerInch, yPixPerInch);
		pcf->yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
		pcf->yOffset = 0;
		pcf->dwEffects = 0;
		pcf->dwMask = CFM_SIZE | CFM_OFFSET | CFM_FACE | CFM_CHARSET | CFM_COLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
		if(lf.lfWeight >= FW_BOLD)
			pcf->dwEffects |= CFE_BOLD;
		if(lf.lfItalic)
			pcf->dwEffects |= CFE_ITALIC;
		if(lf.lfUnderline)
			pcf->dwEffects |= CFE_UNDERLINE;
		pcf->bCharSet = lf.lfCharSet;
		pcf->bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
		wcscpy_s(pcf->szFaceName, _countof(pcf->szFaceName), lf.lfFaceName);
#else
		//need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
		MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, pcf->szFaceName, LF_FACESIZE) ;
#endif

		return S_OK;
	}

	HRESULT InitDefaultParaFormat(CRichEditUI* re, PARAFORMAT2* ppf) 
	{	
		memset(ppf, 0, sizeof(PARAFORMAT2));
		ppf->cbSize = sizeof(PARAFORMAT2);
		ppf->dwMask = PFM_ALL;
		ppf->wAlignment = PFA_LEFT;
		ppf->cTabCount = 1;
		ppf->rgxTabs[0] = lDefaultTab;

		return S_OK;
	}

	HRESULT CreateHost(CRichEditUI *re, const CREATESTRUCT *pcs, CTxtWinHost **pptec)
	{
		HRESULT hr = E_FAIL;
		CTxtWinHost *phost = new CTxtWinHost();
		if(phost) {
			if (phost->Init(re, pcs)) {
				*pptec = phost;
				hr = S_OK;
			}
		}

		if (FAILED(hr)) {
			delete phost;
		}

		return TRUE;
	}

	CTxtWinHost::CTxtWinHost() : m_re(NULL)
	{
		::ZeroMemory(&cRefs, sizeof(CTxtWinHost) - offsetof(CTxtWinHost, cRefs));
		cchTextMost = cInitTextMax;
		laccelpos = -1;
	}

	CTxtWinHost::~CTxtWinHost()
	{
		// 鐎瑰鍙忓Λ鈧弻銉窗婵″倹鐏?Init 婢惰精瑙﹂敍鍫濐洤DLL閸旂姾娴囨径杈Е閿涘绱漰serv 閸欘垵鍏樻稉?NULL
		if(pserv) {
			pserv->OnTxInPlaceDeactivate();
			pserv->Release();
		}
	}

	////////////////////// Create/Init/Destruct Commands ///////////////////////

	BOOL CTxtWinHost::Init(CRichEditUI *re, const CREATESTRUCT *pcs)
	{
		IUnknown *pUnk = NULL;
		HRESULT hr;

		m_re = re;
		// Initialize Reference count
		cRefs = 1;

		// Create and cache CHARFORMAT for this control
		if (FAILED(InitDefaultCharFormat(re, &cf, NULL)))
			return FALSE;

		// Create and cache PARAFORMAT for this control
		if(FAILED(InitDefaultParaFormat(re, &pf)))
			return FALSE;

		// edit controls created without a window are multiline by default
		// so that paragraph formats can be
		dwStyle = ES_MULTILINE;

		// edit controls are rich by default
		fRich = re->IsRich();

		cchTextMost = re->GetLimitText();

		if (pcs )
		{
			dwStyle = pcs->style;

			if ( !(dwStyle & (ES_AUTOHSCROLL | WS_HSCROLL)) )
			{
				fWordWrap = TRUE;
			}
		}

		if( !(dwStyle & ES_LEFT) )
		{
			if(dwStyle & ES_CENTER)
				pf.wAlignment = PFA_CENTER;
			else if(dwStyle & ES_RIGHT)
				pf.wAlignment = PFA_RIGHT;
		}

		fInplaceActive = TRUE;

		PCreateTextServices TextServicesProc = NULL;
#ifdef _UNICODE		
		HMODULE hmod = LoadLibrary(_T("Msftedit.dll"));
#else
		HMODULE hmod = LoadLibrary(_T("Riched20.dll"));
#endif
		if (hmod) {
			TextServicesProc = (PCreateTextServices)GetProcAddress(hmod,"CreateTextServices");
		}
		if (TextServicesProc != NULL) {
			hr = TextServicesProc(NULL, this, &pUnk);
		}

		// 鐎瑰鍙忓Λ鈧弻銉窗DLL閸旂姾娴囨径杈Е閹存渿reateTextServices婢惰精瑙﹂弮?pUnk 娑?NULL
		if(pUnk == NULL)
		{
			goto err;
		}

		hr = pUnk->QueryInterface(IID_ITextServices,(void **)&pserv);

		// Whether the previous call succeeded or failed we are done
		// with the private interface.
		pUnk->Release();

		if(FAILED(hr))
		{
			goto err;
		}

		// Set window text
		if(pcs && pcs->lpszName)
		{
#ifdef _UNICODE		
			if(FAILED(pserv->TxSetText((wchar_t *)pcs->lpszName)))
				goto err;
#else
			size_t iLen = strlen(pcs->lpszName);
			LPWSTR lpText = new WCHAR[iLen + 1];
			::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
			::MultiByteToWideChar(CP_ACP, 0, pcs->lpszName, -1, (LPWSTR)lpText, iLen) ;
			if(FAILED(pserv->TxSetText((LPWSTR)lpText))) {
				delete[] lpText;
				goto err;
			}
			delete[] lpText;
#endif
		}

		return TRUE;

	err:
		return FALSE;
	}

	/////////////////////////////////  IUnknown ////////////////////////////////


	HRESULT CTxtWinHost::QueryInterface(REFIID riid, void **ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;
		*ppvObject = NULL;

		if (IsEqualIID(riid, IID_IUnknown) 
			|| IsEqualIID(riid, IID_ITextHost)) 
		{
			AddRef();
			*ppvObject = (ITextHost *) this;
			hr = S_OK;
		}

		return hr;
	}

	ULONG CTxtWinHost::AddRef(void)
	{
		return ++cRefs;
	}

	ULONG CTxtWinHost::Release(void)
	{
		ULONG c_Refs = --cRefs;

		if (c_Refs == 0)
		{
			delete this;
		}

		return c_Refs;
	}

	/////////////////////////////////  Far East Support  //////////////////////////////////////

	HIMC CTxtWinHost::TxImmGetContext(void)
	{
		return NULL;
	}

	void CTxtWinHost::TxImmReleaseContext(HIMC himc)
	{
		//::ImmReleaseContext( hwnd, himc );
	}

	//////////////////////////// ITextHost Interface  ////////////////////////////

	HDC CTxtWinHost::TxGetDC()
	{
		return GetManagerActiveOrMeasureDC(m_re->GetManager());
	}

	int CTxtWinHost::TxReleaseDC(HDC hdc)
	{
		return 1;
	}

	BOOL CTxtWinHost::TxShowScrollBar(INT fnBar, BOOL fShow)
	{
		CScrollBarUI* pVerticalScrollBar = m_re->GetVerticalScrollBar();
		CScrollBarUI* pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
		if( fnBar == SB_VERT && pVerticalScrollBar ) {
			pVerticalScrollBar->SetVisible(fShow == TRUE);
		}
		else if( fnBar == SB_HORZ && pHorizontalScrollBar ) {
			pHorizontalScrollBar->SetVisible(fShow == TRUE);
		}
		else if( fnBar == SB_BOTH ) {
			if( pVerticalScrollBar ) pVerticalScrollBar->SetVisible(fShow == TRUE);
			if( pHorizontalScrollBar ) pHorizontalScrollBar->SetVisible(fShow == TRUE);
		}
		return TRUE;
	}

	BOOL CTxtWinHost::TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags)
	{
		if( fuSBFlags == SB_VERT ) {
			m_re->EnableScrollBar(true, m_re->GetHorizontalScrollBar() != NULL);
			m_re->GetVerticalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
		}
		else if( fuSBFlags == SB_HORZ ) {
			m_re->EnableScrollBar(m_re->GetVerticalScrollBar() != NULL, true);
			m_re->GetHorizontalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
		}
		else if( fuSBFlags == SB_BOTH ) {
			m_re->EnableScrollBar(true, true);
			m_re->GetVerticalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
			m_re->GetHorizontalScrollBar()->SetVisible(fuArrowflags != ESB_DISABLE_BOTH);
		}
		return TRUE;
	}

	BOOL CTxtWinHost::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
	{
		CScrollBarUI* pVerticalScrollBar = m_re->GetVerticalScrollBar();
		CScrollBarUI* pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
		if( fnBar == SB_VERT && pVerticalScrollBar ) {
			if( nMaxPos - nMinPos - rcClient.bottom + rcClient.top <= 0 ) {
				pVerticalScrollBar->SetVisible(false);
			}
			else {
				pVerticalScrollBar->SetVisible(true);
				pVerticalScrollBar->SetScrollRange(nMaxPos - nMinPos - rcClient.bottom + rcClient.top);
			}
		}
		else if( fnBar == SB_HORZ && pHorizontalScrollBar ) {
			if( nMaxPos - nMinPos - rcClient.right + rcClient.left <= 0 ) {
				pHorizontalScrollBar->SetVisible(false);
			}
			else {
				pHorizontalScrollBar->SetVisible(true);
				pHorizontalScrollBar->SetScrollRange(nMaxPos - nMinPos - rcClient.right + rcClient.left);
			}   
		}
		return TRUE;
	}

	BOOL CTxtWinHost::TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw)
	{
		CScrollBarUI* pVerticalScrollBar = m_re->GetVerticalScrollBar();
		CScrollBarUI* pHorizontalScrollBar = m_re->GetHorizontalScrollBar();
		if( fnBar == SB_VERT && pVerticalScrollBar ) {
			pVerticalScrollBar->SetScrollPos(nPos);
		}
		else if( fnBar == SB_HORZ && pHorizontalScrollBar ) {
			pHorizontalScrollBar->SetScrollPos(nPos);
		}
		return TRUE;
	}

	void CTxtWinHost::TxInvalidateRect(LPCRECT prc, BOOL fMode)
	{
		if( prc == NULL ) {
			m_re->GetManager()->Invalidate(rcClient);
			return;
		}
		RECT rc = *prc;
		m_re->GetManager()->Invalidate(rc);
	}

	void CTxtWinHost::TxViewChange(BOOL fUpdate) 
	{
		if( m_re->OnTxViewChanged() ) m_re->Invalidate();
	}

	BOOL CTxtWinHost::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
	{
		iCaretWidth = xWidth;
		iCaretHeight = yHeight;
		HWND hWnd = GetRichEditPaintWindow(m_re != NULL ? m_re->GetManager() : NULL);
		return hWnd != NULL ? ::CreateCaret(hWnd, hbmp, xWidth, yHeight) : FALSE;
	}

	BOOL CTxtWinHost::TxShowCaret(BOOL fShow)
	{
		fShowCaret = fShow;
		HWND hWnd = GetRichEditPaintWindow(m_re != NULL ? m_re->GetManager() : NULL);
		if (hWnd == NULL) {
			return FALSE;
		}
		if(fShow)
			return ::ShowCaret(hWnd);
		else
			return ::HideCaret(hWnd);
	}

	BOOL CTxtWinHost::TxSetCaretPos(INT x, INT y)
	{
		POINT ptCaret = { 0 };
		::GetCaretPos(&ptCaret);
		RECT rcCaret = { ptCaret.x, ptCaret.y, ptCaret.x + iCaretLastWidth, ptCaret.y + iCaretLastHeight };
		if( m_re->GetManager()->IsLayered() ) m_re->GetManager()->Invalidate(rcCaret);
		else if( fNeedFreshCaret == TRUE ) {
			m_re->GetManager()->Invalidate(rcCaret);
			fNeedFreshCaret = FALSE;
		}
		rcCaret.left = x;
		rcCaret.top = y;
		rcCaret.right = x + iCaretWidth;
		rcCaret.bottom = y + iCaretHeight;
		if( m_re->GetManager()->IsLayered() ) m_re->GetManager()->Invalidate(rcCaret);
		iCaretLastWidth = iCaretWidth;
		iCaretLastHeight = iCaretHeight;
		return ::SetCaretPos(x, y);
	}

	BOOL CTxtWinHost::TxSetTimer(UINT idTimer, UINT uTimeout)
	{
		fTimer = TRUE;
		return m_re->GetManager()->SetTimer(m_re, idTimer, uTimeout) == TRUE;
	}

	void CTxtWinHost::TxKillTimer(UINT idTimer)
	{
		m_re->GetManager()->KillTimer(m_re, idTimer);
		fTimer = FALSE;
	}

	void CTxtWinHost::TxScrollWindowEx (INT dx, INT dy, LPCRECT lprcScroll,	LPCRECT lprcClip,	HRGN hrgnUpdate, LPRECT lprcUpdate,	UINT fuScroll)	
	{
		return;
	}

	void CTxtWinHost::TxSetCapture(BOOL fCapture)
	{
		if (fCapture) m_re->GetManager()->SetCapture();
		else m_re->GetManager()->ReleaseCapture();
		fCaptured = fCapture;
	}

	void CTxtWinHost::TxSetFocus()
	{
		m_re->SetFocus();
	}

	void CTxtWinHost::TxSetCursor(HCURSOR hcur,	BOOL fText)
	{
		::SetCursor(hcur);
	}

	BOOL CTxtWinHost::TxScreenToClient(LPPOINT lppt)
	{
		if (lppt == NULL) {
			return FALSE;
		}
		return ScreenToRichEditClient(m_re != NULL ? m_re->GetManager() : NULL, *lppt);
	}

	BOOL CTxtWinHost::TxClientToScreen(LPPOINT lppt)
	{
		if (lppt == NULL) {
			return FALSE;
		}
		return ClientToRichEditScreen(m_re != NULL ? m_re->GetManager() : NULL, *lppt);
	}

	HRESULT CTxtWinHost::TxActivate(LONG *plOldState)
	{
		return S_OK;
	}

	HRESULT CTxtWinHost::TxDeactivate(LONG lNewState)
	{
		return S_OK;
	}

	HRESULT CTxtWinHost::TxGetClientRect(LPRECT prc)
	{
		*prc = rcClient;
		GetControlRect(prc);
		return NOERROR;
	}

	HRESULT CTxtWinHost::TxGetViewInset(LPRECT prc) 
	{
		prc->left = prc->right = prc->top = prc->bottom = 0;
		return NOERROR;	
	}

	HRESULT CTxtWinHost::TxGetCharFormat(const CHARFORMATW **ppCF)
	{
		*ppCF = &cf;
		return NOERROR;
	}

	HRESULT CTxtWinHost::TxGetParaFormat(const PARAFORMAT **ppPF)
	{
		*ppPF = &pf;
		return NOERROR;
	}

	COLORREF CTxtWinHost::TxGetSysColor(int nIndex) 
	{
		return ::GetSysColor(nIndex);
	}

	HRESULT CTxtWinHost::TxGetBackStyle(TXTBACKSTYLE *pstyle)
	{
		*pstyle = !fTransparent ? TXTBACK_OPAQUE : TXTBACK_TRANSPARENT;
		return NOERROR;
	}

	HRESULT CTxtWinHost::TxGetMaxLength(DWORD *pLength)
	{
		*pLength = cchTextMost;
		return NOERROR;
	}

	HRESULT CTxtWinHost::TxGetScrollBars(DWORD *pdwScrollBar)
	{
		*pdwScrollBar =  dwStyle & (WS_VSCROLL | WS_HSCROLL | ES_AUTOVSCROLL | 
			ES_AUTOHSCROLL | ES_DISABLENOSCROLL);

		return NOERROR;
	}

	HRESULT CTxtWinHost::TxGetPasswordChar(wchar_t *pch)
	{
#ifdef _UNICODE
		*pch = chPasswordChar;
#else
		::WideCharToMultiByte(CP_ACP, 0, &chPasswordChar, 1, pch, 1, NULL, NULL) ;
#endif
		return NOERROR;
	}

	HRESULT CTxtWinHost::TxGetAcceleratorPos(LONG *pcp)
	{
		*pcp = laccelpos;
		return S_OK;
	} 										   

	HRESULT CTxtWinHost::OnTxCharFormatChange(const CHARFORMATW *pcf)
	{
		return S_OK;
	}

	HRESULT CTxtWinHost::OnTxParaFormatChange(const PARAFORMAT *ppf)
	{
		return S_OK;
	}

	HRESULT CTxtWinHost::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) 
	{
		DWORD dwProperties = 0;

		if (fRich)
		{
			dwProperties = TXTBIT_RICHTEXT;
		}

		if (dwStyle & ES_MULTILINE)
		{
			dwProperties |= TXTBIT_MULTILINE;
		}

		if (dwStyle & ES_READONLY)
		{
			dwProperties |= TXTBIT_READONLY;
		}

		if (dwStyle & ES_PASSWORD)
		{
			dwProperties |= TXTBIT_USEPASSWORD;
		}

		if (!(dwStyle & ES_NOHIDESEL))
		{
			dwProperties |= TXTBIT_HIDESELECTION;
		}

		if (fEnableAutoWordSel)
		{
			dwProperties |= TXTBIT_AUTOWORDSEL;
		}

		if (fWordWrap)
		{
			dwProperties |= TXTBIT_WORDWRAP;
		}

		if (fAllowBeep)
		{
			dwProperties |= TXTBIT_ALLOWBEEP;
		}

		if (fSaveSelection)
		{
			dwProperties |= TXTBIT_SAVESELECTION;
		}

		*pdwBits = dwProperties & dwMask; 
		return NOERROR;
	}


	HRESULT CTxtWinHost::TxNotify(DWORD iNotify, void *pv)
	{
		if( iNotify == EN_REQUESTRESIZE ) {
			RECT rc;
			REQRESIZE *preqsz = (REQRESIZE *)pv;
			GetControlRect(&rc);
			rc.bottom = rc.top + preqsz->rc.bottom;
			rc.right  = rc.left + preqsz->rc.right;
			SetClientRect(&rc);
			return S_OK;
		}
		m_re->OnTxNotify(iNotify, pv);
		return S_OK;
	}

	HRESULT CTxtWinHost::TxGetExtent(LPSIZEL lpExtent)
	{
		*lpExtent = sizelExtent;
		return S_OK;
	}

	HRESULT	CTxtWinHost::TxGetSelectionBarWidth (LONG *plSelBarWidth)
	{
		*plSelBarWidth = lSelBarWidth;
		return S_OK;
	}

	void CTxtWinHost::SetWordWrap(BOOL _fWordWrap)
	{
		fWordWrap = _fWordWrap;
		pserv->OnTxPropertyBitsChange(TXTBIT_WORDWRAP, fWordWrap ? TXTBIT_WORDWRAP : 0);
	}

	BOOL CTxtWinHost::IsReadOnly()
	{
		return (dwStyle & ES_READONLY) != 0;
	}

	void CTxtWinHost::SetReadOnly(BOOL fReadOnly)
	{
		if (fReadOnly)
		{
			dwStyle |= ES_READONLY;
		}
		else
		{
			dwStyle &= ~ES_READONLY;
		}

		pserv->OnTxPropertyBitsChange(TXTBIT_READONLY, 
			fReadOnly ? TXTBIT_READONLY : 0);
	}

	void CTxtWinHost::SetFont(HFONT hFont) 
	{
		if( hFont == NULL ) return;
		LOGFONT lf;
		::GetObject(hFont, sizeof(LOGFONT), &lf);
		LONG xPixPerInch = 96;
		LONG yPixPerInch = 96;
		GetManagerLogPixels(m_re->GetManager(), xPixPerInch, yPixPerInch);
		cf.yHeight = -lf.lfHeight * LY_PER_INCH / yPixPerInch;
		if(lf.lfWeight >= FW_BOLD) cf.dwEffects |= CFE_BOLD;
		else cf.dwEffects &= ~CFE_BOLD;
		if(lf.lfItalic) cf.dwEffects |= CFE_ITALIC;
		else cf.dwEffects &= ~CFE_ITALIC;
		if(lf.lfUnderline) cf.dwEffects |= CFE_UNDERLINE;
		else cf.dwEffects &= ~CFE_UNDERLINE;
		cf.bCharSet = lf.lfCharSet;
		cf.bPitchAndFamily = lf.lfPitchAndFamily;
#ifdef _UNICODE
		wcscpy_s(cf.szFaceName, _countof(cf.szFaceName), lf.lfFaceName);
#else
		//need to thunk pcf->szFaceName to a standard char string.in this case it's easy because our thunk is also our copy
		MultiByteToWideChar(CP_ACP, 0, lf.lfFaceName, LF_FACESIZE, cf.szFaceName, LF_FACESIZE) ;
#endif

		pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
			TXTBIT_CHARFORMATCHANGE);
	}

	void CTxtWinHost::SetColor(DWORD dwColor)
	{
		cf.crTextColor = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
		pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
			TXTBIT_CHARFORMATCHANGE);
	}

	SIZEL* CTxtWinHost::GetExtent() 
	{
		return &sizelExtent;
	}

	void CTxtWinHost::SetExtent(SIZEL *psizelExtent) 
	{ 
		sizelExtent = *psizelExtent; 
		pserv->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
	}

	void CTxtWinHost::LimitText(LONG nChars)
	{
		cchTextMost = nChars;
		if( cchTextMost <= 0 ) cchTextMost = cInitTextMax;
		pserv->OnTxPropertyBitsChange(TXTBIT_MAXLENGTHCHANGE, TXTBIT_MAXLENGTHCHANGE);
	}

	BOOL CTxtWinHost::IsCaptured()
	{
		return fCaptured;
	}

	BOOL CTxtWinHost::IsShowCaret()
	{
		return fShowCaret;
	}

	void CTxtWinHost::NeedFreshCaret()
	{
		fNeedFreshCaret = TRUE;
	}

	INT CTxtWinHost::GetCaretWidth()
	{
		return iCaretWidth;
	}

	INT CTxtWinHost::GetCaretHeight()
	{
		return iCaretHeight;
	}

	BOOL CTxtWinHost::GetAllowBeep()
	{
		return fAllowBeep;
	}

	void CTxtWinHost::SetAllowBeep(BOOL fAllowBeep)
	{
		fAllowBeep = fAllowBeep;

		pserv->OnTxPropertyBitsChange(TXTBIT_ALLOWBEEP, 
			fAllowBeep ? TXTBIT_ALLOWBEEP : 0);
	}

	WORD CTxtWinHost::GetDefaultAlign()
	{
		return pf.wAlignment;
	}

	void CTxtWinHost::SetDefaultAlign(WORD wNewAlign)
	{
		pf.wAlignment = wNewAlign;

		// Notify control of property change
		pserv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
	}

	BOOL CTxtWinHost::GetRichTextFlag()
	{
		return fRich;
	}

	void CTxtWinHost::SetRichTextFlag(BOOL fNew)
	{
		fRich = fNew;

		pserv->OnTxPropertyBitsChange(TXTBIT_RICHTEXT, 
			fNew ? TXTBIT_RICHTEXT : 0);
	}

	LONG CTxtWinHost::GetDefaultLeftIndent()
	{
		return pf.dxOffset;
	}

	void CTxtWinHost::SetDefaultLeftIndent(LONG lNewIndent)
	{
		pf.dxOffset = lNewIndent;

		pserv->OnTxPropertyBitsChange(TXTBIT_PARAFORMATCHANGE, 0);
	}

	void CTxtWinHost::SetClientRect(RECT *prc) 
	{
		rcClient = *prc;

		LONG xPerInch = 96;
		LONG yPerInch = 96;
		GetManagerLogPixels(m_re->GetManager(), xPerInch, yPerInch);
		sizelExtent.cx = DXtoHimetricX(rcClient.right - rcClient.left, xPerInch);
		sizelExtent.cy = DYtoHimetricY(rcClient.bottom - rcClient.top, yPerInch);

		pserv->OnTxPropertyBitsChange(TXTBIT_VIEWINSETCHANGE, TXTBIT_VIEWINSETCHANGE);
	}

	BOOL CTxtWinHost::SetSaveSelection(BOOL f_SaveSelection)
	{
		BOOL fResult = f_SaveSelection;

		fSaveSelection = f_SaveSelection;

		// notify text services of property change
		pserv->OnTxPropertyBitsChange(TXTBIT_SAVESELECTION, 
			fSaveSelection ? TXTBIT_SAVESELECTION : 0);

		return fResult;		
	}

	HRESULT	CTxtWinHost::OnTxInPlaceDeactivate()
	{
		HRESULT hr = pserv->OnTxInPlaceDeactivate();

		if (SUCCEEDED(hr))
		{
			fInplaceActive = FALSE;
		}

		return hr;
	}

	HRESULT	CTxtWinHost::OnTxInPlaceActivate(LPCRECT prcClient)
	{
		fInplaceActive = TRUE;

		HRESULT hr = pserv->OnTxInPlaceActivate(prcClient);

		if (FAILED(hr))
		{
			fInplaceActive = FALSE;
		}

		return hr;
	}

	BOOL CTxtWinHost::DoSetCursor(RECT *prc, POINT *pt)
	{
		RECT rc = prc ? *prc : rcClient;
		ITextServices* pTextServices = GetRichEditTextServices(this);

		// Is this in our rectangle?
		if (pTextServices != NULL && PtInRect(&rc, *pt))
		{
			RECT *prcClient = (!fInplaceActive || prc) ? &rc : NULL;
			pTextServices->OnTxSetCursor(DVASPECT_CONTENT,	-1, NULL, NULL,  GetManagerActiveOrMeasureDC(m_re->GetManager()),
				NULL, prcClient, pt->x, pt->y);

			return TRUE;
		}

		return FALSE;
	}

	void CTxtWinHost::GetControlRect(LPRECT prc)
	{
		prc->top = rcClient.top;
		prc->bottom = rcClient.bottom;
		prc->left = rcClient.left;
		prc->right = rcClient.right;
	}

	void CTxtWinHost::SetTransparent(BOOL f_Transparent)
	{
		fTransparent = f_Transparent;

		// notify text services of property change
		pserv->OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 0);
	}

	LONG CTxtWinHost::SetAccelPos(LONG l_accelpos)
	{
		LONG laccelposOld = l_accelpos;

		laccelpos = l_accelpos;

		// notify text services of property change
		pserv->OnTxPropertyBitsChange(TXTBIT_SHOWACCELERATOR, 0);

		return laccelposOld;
	}

	WCHAR CTxtWinHost::SetPasswordChar(WCHAR ch_PasswordChar)
	{
		WCHAR chOldPasswordChar = chPasswordChar;

		chPasswordChar = ch_PasswordChar;

		// notify text services of property change
		pserv->OnTxPropertyBitsChange(TXTBIT_USEPASSWORD, 
			(chPasswordChar != 0) ? TXTBIT_USEPASSWORD : 0);

		return chOldPasswordChar;
	}

	void CTxtWinHost::SetDisabled(BOOL fOn)
	{
		cf.dwMask	 |= CFM_COLOR | CFM_DISABLED;
		cf.dwEffects |= CFE_AUTOCOLOR | CFE_DISABLED;

		if( !fOn )
		{
			cf.dwEffects &= ~CFE_DISABLED;
		}

		pserv->OnTxPropertyBitsChange(TXTBIT_CHARFORMATCHANGE, 
			TXTBIT_CHARFORMATCHANGE);
	}

	LONG CTxtWinHost::SetSelBarWidth(LONG l_SelBarWidth)
	{
		LONG lOldSelBarWidth = lSelBarWidth;

		lSelBarWidth = l_SelBarWidth;

		if (lSelBarWidth)
		{
			dwStyle |= ES_SELECTIONBAR;
		}
		else
		{
			dwStyle &= (~ES_SELECTIONBAR);
		}

		pserv->OnTxPropertyBitsChange(TXTBIT_SELBARCHANGE, TXTBIT_SELBARCHANGE);

		return lOldSelBarWidth;
	}

	BOOL CTxtWinHost::GetTimerState()
	{
		return fTimer;
	}

	void CTxtWinHost::SetCharFormat(CHARFORMAT2W &c)
	{
		cf = c;
	}

	void CTxtWinHost::SetParaFormat(PARAFORMAT2 &p)
	{
		pf = p;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//
	IMPLEMENT_DUICONTROL(CRichEditUI)
		CRichEditUI::CRichEditUI() : m_pTwh(NULL), m_bVScrollBarFixing(false), m_bWantTab(true), m_bWantReturn(true), 
		m_bWantCtrlReturn(true), m_bTransparent(true), m_bRich(true), m_bReadOnly(false), m_bWordWrap(false), m_dwTextColor(0), m_iFont(-1), 
		m_iLimitText(cInitTextMax), m_lTwhStyle(ES_MULTILINE), m_bDrawCaret(true), m_bInited(false), m_chLeadByte(0),m_uButtonState(0),
		m_dwTipValueColor(0xFFBAC0C5), m_uTipValueAlign(DT_SINGLELINE | DT_LEFT)
	{
#ifndef _UNICODE
		m_fAccumulateDBC =true;
#else
		m_fAccumulateDBC= false;
#endif
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
	}

	CRichEditUI::~CRichEditUI()
	{
		if( m_pTwh ) {
			m_pTwh->Release();
			m_pManager->RemoveMessageFilter(this);
		}
	}

	std::wstring_view CRichEditUI::GetClass() const
	{
		return _T("RichEditUI");
	}

	LPVOID CRichEditUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_RICHEDIT) == 0) return static_cast<CRichEditUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CRichEditUI::GetControlFlags() const
	{
		if( !IsEnabled() ) return CControlUI::GetControlFlags();

		return UIFLAG_SETCURSOR | UIFLAG_TABSTOP;
	}

	void CRichEditUI::SetEnabled(bool bEnabled)
	{
		CContainerUI::SetEnabled(bEnabled);
		if(m_pTwh) {
			if(IsEnabled()) {
				m_pTwh->SetColor(GetTextColor());
			}
			else {
				m_pTwh->SetColor (m_pManager->GetDefaultDisabledColor());
			}
		}
	}



	bool CRichEditUI::IsMultiLine()
	{
		return (m_lTwhStyle & ES_MULTILINE) == ES_MULTILINE;
	}

	void CRichEditUI::SetMultiLine(bool bMultiLine)
	{
		if(!bMultiLine) m_lTwhStyle &= ~ES_MULTILINE;
		else  m_lTwhStyle |= ES_MULTILINE;
	}

	bool CRichEditUI::IsWantTab()
	{
		return m_bWantTab;
	}

	void CRichEditUI::SetWantTab(bool bWantTab)
	{
		m_bWantTab = bWantTab;
	}

	bool CRichEditUI::IsWantReturn()
	{
		return m_bWantReturn;
	}

	void CRichEditUI::SetWantReturn(bool bWantReturn)
	{
		m_bWantReturn = bWantReturn;
	}

	bool CRichEditUI::IsWantCtrlReturn()
	{
		return m_bWantCtrlReturn;
	}

	void CRichEditUI::SetWantCtrlReturn(bool bWantCtrlReturn)
	{
		m_bWantCtrlReturn = bWantCtrlReturn;
	}

	bool CRichEditUI::IsTransparent()
	{
		return m_bTransparent;
	}

	void CRichEditUI::SetTransparent(bool bTransparent)
	{
		m_bTransparent = bTransparent;
		if( m_pTwh ) m_pTwh->SetTransparent(bTransparent);
	}

	bool CRichEditUI::IsRich()
	{
		return m_bRich;
	}

	void CRichEditUI::SetRich(bool bRich)
	{
		m_bRich = bRich;
		if( m_pTwh ) m_pTwh->SetRichTextFlag(bRich);
	}

	bool CRichEditUI::IsReadOnly()
	{
		return m_bReadOnly;
	}

	void CRichEditUI::SetReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		if( m_pTwh ) m_pTwh->SetReadOnly(bReadOnly);
	}

	bool CRichEditUI::IsWordWrap()
	{
		return m_bWordWrap;
	}

	void CRichEditUI::SetWordWrap(bool bWordWrap)
	{
		m_bWordWrap = bWordWrap;
		if( m_pTwh ) m_pTwh->SetWordWrap(bWordWrap);
	}

	int CRichEditUI::GetFont()
	{
		return m_iFont;
	}

	void CRichEditUI::SetFont(int index)
	{
		m_iFont = index;
		if( m_pTwh ) {
			m_pTwh->SetFont(GetManager()->GetFont(m_iFont));
		}
	}

	void CRichEditUI::SetFont(std::wstring_view pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
	{
		if( m_pTwh ) {
			const std::wstring fontName(pStrFontName);
			LOGFONT lf = { 0 };
			::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
			wcsncpy_s(lf.lfFaceName, LF_FACESIZE, fontName.c_str(), _TRUNCATE);
			lf.lfCharSet = DEFAULT_CHARSET;
			lf.lfHeight = -nSize;
			if( bBold ) lf.lfWeight += FW_BOLD;
			if( bUnderline ) lf.lfUnderline = TRUE;
			if( bItalic ) lf.lfItalic = TRUE;
			HFONT hFont = ::CreateFontIndirect(&lf);
			if( hFont == NULL ) return;
			m_pTwh->SetFont(hFont);
			::DeleteObject(hFont);
		}
	}

	LONG CRichEditUI::GetWinStyle()
	{
		return m_lTwhStyle;
	}

	void CRichEditUI::SetWinStyle(LONG lStyle)
	{
		m_lTwhStyle = lStyle;
	}

	DWORD CRichEditUI::GetTextColor()
	{
		return m_dwTextColor;
	}

	void CRichEditUI::SetTextColor(DWORD dwTextColor)
	{
		m_dwTextColor = dwTextColor;
		if( m_pTwh ) {
			m_pTwh->SetColor(dwTextColor);
		}
	}

	int CRichEditUI::GetLimitText()
	{
		return m_iLimitText;
	}

	void CRichEditUI::SetLimitText(int iChars)
	{
		m_iLimitText = iChars;
		if( m_pTwh ) {
			m_pTwh->LimitText(m_iLimitText);
		}
	}

	void CRichEditUI::SetLimitTextString(std::wstring_view pStrLimitText)
	{
		m_strLimitText = pStrLimitText;
	}

	std::wstring CRichEditUI::GetLimitTextString() const
	{
		return m_strLimitText;
	}

	long CRichEditUI::GetTextLength(DWORD dwFlags) const
	{
		GETTEXTLENGTHEX textLenEx;
		textLenEx.flags = dwFlags;
#ifdef _UNICODE
		textLenEx.codepage = 1200;
#else
		textLenEx.codepage = CP_ACP;
#endif
		LRESULT lResult =0;
		TxSendMessage(EM_GETTEXTLENGTHEX, (WPARAM)&textLenEx, 0, &lResult);
		return (long)lResult;
	}

	std::wstring CRichEditUI::GetText() const
	{
		long lLen = GetTextLength(GTL_DEFAULT);
		wchar_t* lpText = NULL;
		GETTEXTEX gt;
		gt.flags = GT_DEFAULT;
#ifdef _UNICODE
		gt.cb = sizeof(wchar_t) * (lLen + 1) ;
		gt.codepage = 1200;
		lpText = new wchar_t[lLen + 1];
		::ZeroMemory(lpText, (lLen + 1) * sizeof(wchar_t));
#else
		gt.cb = sizeof(wchar_t) * lLen * 2 + 1;
		gt.codepage = CP_ACP;
		lpText = new wchar_t[lLen * 2 + 1];
		::ZeroMemory(lpText, (lLen * 2 + 1) * sizeof(wchar_t));
#endif
		gt.lpDefaultChar = NULL;
		gt.lpUsedDefChar = NULL;
		TxSendMessage(EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)lpText, 0);
		std::wstring sText(lpText);
		delete[] lpText;
		return sText;
	}

	void CRichEditUI::SetText(std::wstring_view pstrText)
	{
		std::wstring strLimitText = GetLimitTextString();
		if (strLimitText.empty() == false)
		{
			std::wstring strText(pstrText);
			std::wstring strOutText;
			for (size_t i = 0; i < strText.size(); ++i)
			{
				if (strLimitText.find(strText[i]) != std::wstring::npos)
				{
					strOutText.push_back(strText[i]);
				}
			}
			m_sText = strOutText;
		}
		else
		{
			m_sText = pstrText;
		}

		if( !m_pTwh ) return;
		SetSel(0, -1);
		ReplaceSel(m_sText.c_str(), FALSE);
	}

	void CRichEditUI::SetShowText(std::wstring_view pstrText)
	{
		// 鐏炲繗鏂€ EN_CHANGE
		LRESULT lResult;
		this->TxSendMessage(EM_GETEVENTMASK, 0, 0, &lResult);
		DWORD dwMask = (DWORD)lResult;
		this->TxSendMessage(EM_SETEVENTMASK, 0, dwMask & ~ENM_CHANGE, 0);

		SetSel(0, -1);
		ReplaceShowSel(pstrText, FALSE);

		// 閹垹顦?EN_CHANGE
		this->TxSendMessage(EM_SETEVENTMASK, 0, dwMask, 0);
	}


	bool CRichEditUI::IsModify() const
	{ 
		if( !m_pTwh ) return false;
		LRESULT lResult;
		TxSendMessage(EM_GETMODIFY, 0, 0, &lResult);
		return (BOOL)lResult == TRUE;
	}

	void CRichEditUI::SetModify(bool bModified) const
	{ 
		TxSendMessage(EM_SETMODIFY, bModified, 0, 0);
	}

	void CRichEditUI::GetSel(CHARRANGE &cr) const
	{ 
		TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0); 
	}

	void CRichEditUI::GetSel(long& nStartChar, long& nEndChar) const
	{
		CHARRANGE cr;
		TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0); 
		nStartChar = cr.cpMin;
		nEndChar = cr.cpMax;
	}

	int CRichEditUI::SetSel(CHARRANGE &cr)
	{ 
		LRESULT lResult;
		TxSendMessage(EM_EXSETSEL, 0, (LPARAM)&cr, &lResult); 
		return (int)lResult;
	}

	int CRichEditUI::SetSel(long nStartChar, long nEndChar)
	{
		CHARRANGE cr;
		cr.cpMin = nStartChar;
		cr.cpMax = nEndChar;
		LRESULT lResult;
		TxSendMessage(EM_EXSETSEL, 0, (LPARAM)&cr, &lResult); 
		return (int)lResult;
	}

	void CRichEditUI::ReplaceSel(std::wstring_view lpszNewText, bool bCanUndo)
	{
		const std::wstring newText(lpszNewText);
#ifdef _UNICODE		
		TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)newText.c_str(), 0); 
#else
		int iLen = static_cast<int>(newText.size());
		LPWSTR lpText = new WCHAR[iLen + 1];
		::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
		::MultiByteToWideChar(CP_ACP, 0, newText.c_str(), -1, (LPWSTR)lpText, iLen) ;
		TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpText, 0); 
		delete[] lpText;
#endif
	}

	void CRichEditUI::ReplaceShowSel(std::wstring_view lpszNewText, bool bCanUndo)
	{
		const std::wstring newText(lpszNewText);
#ifdef _UNICODE		
		LRESULT lResult;
		TxSendMessage(EM_GETEVENTMASK, 0, 0, &lResult);
		DWORD dwMask = (DWORD)lResult;
		TxSendMessage(EM_SETEVENTMASK, 0, dwMask & ~ENM_CHANGE, 0);

		TxSendMessage(EM_REPLACESEL, (WPARAM)bCanUndo, (LPARAM)newText.c_str(), 0);

		TxSendMessage(EM_SETEVENTMASK, 0, dwMask, 0);
#else
		int iLen = static_cast<int>(newText.size());
		LPWSTR lpText = new WCHAR[iLen + 1];
		::ZeroMemory(lpText, (iLen + 1) * sizeof(WCHAR));
		::MultiByteToWideChar(CP_ACP, 0, newText.c_str(), -1, (LPWSTR)lpText, iLen) ;
		TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpText, 0); 
		delete[] lpText;
#endif

	}
	void CRichEditUI::ReplaceSelW(LPCWSTR lpszNewText, bool bCanUndo)
	{
		TxSendMessage(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM)lpszNewText, 0); 
	}

	std::wstring CRichEditUI::GetSelText() const
	{
		if( !m_pTwh ) return std::wstring();
		CHARRANGE cr;
		cr.cpMin = cr.cpMax = 0;
		TxSendMessage(EM_EXGETSEL, 0, (LPARAM)&cr, 0);
		LPWSTR lpText = NULL;
		lpText = new WCHAR[cr.cpMax - cr.cpMin + 1];
		::ZeroMemory(lpText, (cr.cpMax - cr.cpMin + 1) * sizeof(WCHAR));
		TxSendMessage(EM_GETSELTEXT, 0, (LPARAM)lpText, 0);
		std::wstring sText;
		sText = (LPCWSTR)lpText;
		delete[] lpText;
		return sText;
	}

	int CRichEditUI::SetSelAll()
	{
		return SetSel(0, -1);
	}

	int CRichEditUI::SetSelNone()
	{
		return SetSel(-1, 0);
	}

	bool CRichEditUI::GetZoom(int& nNum, int& nDen) const
	{
		LRESULT lResult;
		TxSendMessage(EM_GETZOOM, (WPARAM)&nNum, (LPARAM)&nDen, &lResult);
		return (BOOL)lResult == TRUE;
	}

	bool CRichEditUI::SetZoom(int nNum, int nDen)
	{
		if (nNum < 0 || nNum > 64) return false;
		if (nDen < 0 || nDen > 64) return false;
		LRESULT lResult;
		TxSendMessage(EM_SETZOOM, nNum, nDen, &lResult);
		return (BOOL)lResult == TRUE;
	}

	bool CRichEditUI::SetZoomOff()
	{
		LRESULT lResult;
		TxSendMessage(EM_SETZOOM, 0, 0, &lResult);
		return (BOOL)lResult == TRUE;
	}

	WORD CRichEditUI::GetSelectionType() const
	{
		LRESULT lResult;
		TxSendMessage(EM_SELECTIONTYPE, 0, 0, &lResult);
		return (WORD)lResult;
	}

	bool CRichEditUI::GetAutoURLDetect() const
	{
		LRESULT lResult;
		TxSendMessage(EM_GETAUTOURLDETECT, 0, 0, &lResult);
		return (BOOL)lResult == TRUE;
	}

	bool CRichEditUI::SetAutoURLDetect(bool bAutoDetect)
	{
		LRESULT lResult;
		TxSendMessage(EM_AUTOURLDETECT, bAutoDetect, 0, &lResult);
		return (BOOL)lResult == FALSE;
	}

	DWORD CRichEditUI::GetEventMask() const
	{
		LRESULT lResult;
		TxSendMessage(EM_GETEVENTMASK, 0, 0, &lResult);
		return (DWORD)lResult;
	}

	DWORD CRichEditUI::SetEventMask(DWORD dwEventMask)
	{
		LRESULT lResult;
		TxSendMessage(EM_SETEVENTMASK, 0, dwEventMask, &lResult);
		return (DWORD)lResult;
	}

	std::wstring CRichEditUI::GetTextRange(long nStartChar, long nEndChar) const
	{
		TEXTRANGEW tr = { 0 };
		tr.chrg.cpMin = nStartChar;
		tr.chrg.cpMax = nEndChar;
		LPWSTR lpText = NULL;
		lpText = new WCHAR[nEndChar - nStartChar + 1];
		::ZeroMemory(lpText, (nEndChar - nStartChar + 1) * sizeof(WCHAR));
		tr.lpstrText = lpText;
		TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&tr, 0);
		std::wstring sText;
		sText = (LPCWSTR)lpText;
		delete[] lpText;
		return sText;
	}

	void CRichEditUI::HideSelection(bool bHide, bool bChangeStyle)
	{
		TxSendMessage(EM_HIDESELECTION, bHide, bChangeStyle, 0);
	}

	void CRichEditUI::ScrollCaret()
	{
		TxSendMessage(EM_SCROLLCARET, 0, 0, 0);
	}

	int CRichEditUI::InsertText(long nInsertAfterChar, std::wstring_view lpstrText, bool bCanUndo)
	{
		int nRet = SetSel(nInsertAfterChar, nInsertAfterChar);
		ReplaceSel(lpstrText, bCanUndo);
		return nRet;
	}

	int CRichEditUI::AppendText(std::wstring_view lpstrText, bool bCanUndo)
	{
		int nRet = SetSel(-1, -1);
		ReplaceSel(lpstrText, bCanUndo);
		return nRet;
	}

	DWORD CRichEditUI::GetDefaultCharFormat(CHARFORMAT2 &cf) const
	{
		cf.cbSize = sizeof(CHARFORMAT2);
		LRESULT lResult;
		TxSendMessage(EM_GETCHARFORMAT, 0, (LPARAM)&cf, &lResult);
		return (DWORD)lResult;
	}

	bool CRichEditUI::SetDefaultCharFormat(CHARFORMAT2 &cf)
	{
		if( !m_pTwh ) return false;
		cf.cbSize = sizeof(CHARFORMAT2);
		LRESULT lResult;
		TxSendMessage(EM_SETCHARFORMAT, 0, (LPARAM)&cf, &lResult);
		if( (BOOL)lResult == TRUE ) {
			CHARFORMAT2W cfw;
			cfw.cbSize = sizeof(CHARFORMAT2W);
			TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM)&cfw, 0);
			m_pTwh->SetCharFormat(cfw);
			return true;
		}
		return false;
	}

	DWORD CRichEditUI::GetSelectionCharFormat(CHARFORMAT2 &cf) const
	{
		cf.cbSize = sizeof(CHARFORMAT2);
		LRESULT lResult;
		TxSendMessage(EM_GETCHARFORMAT, 1, (LPARAM)&cf, &lResult);
		return (DWORD)lResult;
	}

	bool CRichEditUI::SetSelectionCharFormat(CHARFORMAT2 &cf)
	{
		if( !m_pTwh ) return false;
		cf.cbSize = sizeof(CHARFORMAT2);
		LRESULT lResult;
		TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, &lResult);
		return (BOOL)lResult == TRUE;
	}

	bool CRichEditUI::SetWordCharFormat(CHARFORMAT2 &cf)
	{
		if( !m_pTwh ) return false;
		cf.cbSize = sizeof(CHARFORMAT2);
		LRESULT lResult;
		TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION|SCF_WORD, (LPARAM)&cf, &lResult); 
		return (BOOL)lResult == TRUE;
	}

	DWORD CRichEditUI::GetParaFormat(PARAFORMAT2 &pf) const
	{
		pf.cbSize = sizeof(PARAFORMAT2);
		LRESULT lResult;
		TxSendMessage(EM_GETPARAFORMAT, 0, (LPARAM)&pf, &lResult);
		return (DWORD)lResult;
	}

	bool CRichEditUI::SetParaFormat(PARAFORMAT2 &pf)
	{
		if( !m_pTwh ) return false;
		pf.cbSize = sizeof(PARAFORMAT2);
		LRESULT lResult;
		TxSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf, &lResult);
		if( (BOOL)lResult == TRUE ) {
			m_pTwh->SetParaFormat(pf);
			return true;
		}
		return false;
	}

	bool CRichEditUI::CanUndo()
	{
		if( !m_pTwh ) return false;
		LRESULT lResult;
		TxSendMessage(EM_CANUNDO, 0, 0, &lResult);
		return (BOOL)lResult == TRUE; 
	}

	bool CRichEditUI::CanRedo()
	{
		if( !m_pTwh ) return false;
		LRESULT lResult;
		TxSendMessage(EM_CANREDO, 0, 0, &lResult);
		return (BOOL)lResult == TRUE; 
	}

	bool CRichEditUI::CanPaste()
	{
		if( !m_pTwh ) return false;
		LRESULT lResult;
		TxSendMessage(EM_CANPASTE, 0, 0, &lResult);
		return (BOOL)lResult == TRUE; 
	}
	bool CRichEditUI::Redo()
	{ 
		if( !m_pTwh ) return false;
		LRESULT lResult;
		TxSendMessage(EM_REDO, 0, 0, &lResult);
		return (BOOL)lResult == TRUE; 
	}

	bool CRichEditUI::Undo()
	{ 
		if( !m_pTwh ) return false;
		LRESULT lResult;
		TxSendMessage(EM_UNDO, 0, 0, &lResult);
		return (BOOL)lResult == TRUE; 
	}

	void CRichEditUI::Clear()
	{ 
		TxSendMessage(WM_CLEAR, 0, 0, 0); 
	}

	void CRichEditUI::Copy()
	{ 
		TxSendMessage(WM_COPY, 0, 0, 0); 
	}

	void CRichEditUI::Cut()
	{ 
		TxSendMessage(WM_CUT, 0, 0, 0); 
	}

	void CRichEditUI::Paste()
	{ 
		TxSendMessage(WM_PASTE, 0, 0, 0); 
	}

	int CRichEditUI::GetLineCount() const
	{ 
		if( !m_pTwh ) return 0;
		LRESULT lResult;
		TxSendMessage(EM_GETLINECOUNT, 0, 0, &lResult);
		return (int)lResult; 
	}

	std::wstring CRichEditUI::GetLine(int nIndex, int nMaxLength) const
	{
		LPWSTR lpText = NULL;
		lpText = new WCHAR[nMaxLength + 1];
		::ZeroMemory(lpText, (nMaxLength + 1) * sizeof(WCHAR));
		*(LPWORD)lpText = (WORD)nMaxLength;
		TxSendMessage(EM_GETLINE, nIndex, (LPARAM)lpText, 0);
		std::wstring sText;
		sText = (LPCWSTR)lpText;
		delete[] lpText;
		return sText;
	}

	int CRichEditUI::LineIndex(int nLine) const
	{
		LRESULT lResult;
		TxSendMessage(EM_LINEINDEX, nLine, 0, &lResult);
		return (int)lResult;
	}

	int CRichEditUI::LineLength(int nLine) const
	{
		LRESULT lResult;
		TxSendMessage(EM_LINELENGTH, nLine, 0, &lResult);
		return (int)lResult;
	}

	bool CRichEditUI::LineScroll(int nLines, int nChars)
	{
		LRESULT lResult;
		TxSendMessage(EM_LINESCROLL, nChars, nLines, &lResult);
		return (BOOL)lResult == TRUE;
	}

	CDuiPoint CRichEditUI::GetCharPos(long lChar) const
	{ 
		CDuiPoint pt; 
		TxSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, (LPARAM)lChar, 0); 
		return pt;
	}

	long CRichEditUI::LineFromChar(long nIndex) const
	{ 
		if( !m_pTwh ) return 0L;
		LRESULT lResult;
		TxSendMessage(EM_EXLINEFROMCHAR, 0, nIndex, &lResult);
		return (long)lResult;
	}

	CDuiPoint CRichEditUI::PosFromChar(UINT nChar) const
	{ 
		POINTL pt; 
		TxSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, nChar, 0); 
		return CDuiPoint(pt.x, pt.y); 
	}

	int CRichEditUI::CharFromPos(CDuiPoint pt) const
	{ 
		POINTL ptl = {pt.x, pt.y}; 
		if( !m_pTwh ) return 0;
		LRESULT lResult;
		TxSendMessage(EM_CHARFROMPOS, 0, (LPARAM)&ptl, &lResult);
		return (int)lResult; 
	}

	void CRichEditUI::EmptyUndoBuffer()
	{ 
		TxSendMessage(EM_EMPTYUNDOBUFFER, 0, 0, 0); 
	}

	UINT CRichEditUI::SetUndoLimit(UINT nLimit)
	{ 
		if( !m_pTwh ) return 0;
		LRESULT lResult;
		TxSendMessage(EM_SETUNDOLIMIT, (WPARAM) nLimit, 0, &lResult);
		return (UINT)lResult; 
	}

	long CRichEditUI::StreamIn(int nFormat, EDITSTREAM &es)
	{ 
		if( !m_pTwh ) return 0L;
		LRESULT lResult;
		TxSendMessage(EM_STREAMIN, nFormat, (LPARAM)&es, &lResult);
		return (long)lResult;
	}

	long CRichEditUI::StreamOut(int nFormat, EDITSTREAM &es)
	{ 
		if( !m_pTwh ) return 0L;
		LRESULT lResult;
		TxSendMessage(EM_STREAMOUT, nFormat, (LPARAM)&es, &lResult);
		return (long)lResult; 
	}

	void CRichEditUI::SetAccumulateDBCMode( bool bDBCMode )
	{
		m_fAccumulateDBC = bDBCMode;
	}

	bool CRichEditUI::IsAccumulateDBCMode()
	{
		return m_fAccumulateDBC;
	}

	void CRichEditUI::DoInit()
	{
		if(m_bInited)
			return ;

		CREATESTRUCT cs;
		cs.style = m_lTwhStyle;
		cs.x = 0;
		cs.y = 0;
		cs.cy = 0;
		cs.cx = 0;
		cs.lpszName = m_sText.c_str();
		CreateHost(this, &cs, &m_pTwh);
		if( m_pTwh ) {
			if( m_bTransparent ) m_pTwh->SetTransparent(TRUE);
			LRESULT lResult;
			SendRichEditTextServiceMessage(m_pTwh, EM_SETLANGOPTIONS, 0, 0, &lResult);
			SendRichEditTextServiceMessage(m_pTwh, EM_SETEVENTMASK, 0, ENM_DROPFILES|ENM_LINK|ENM_CHANGE, &lResult);
			m_pTwh->OnTxInPlaceActivate(NULL);
			m_pManager->AddMessageFilter(this);
			//m_pManager->SetTimer(this, DEFAULT_TIMERID, ::GetCaretBlinkTime());
			if (!m_bEnabled) {
				m_pTwh->SetColor(m_pManager->GetDefaultDisabledColor());
			}
		}

		m_bInited= true;
	}

	HRESULT CRichEditUI::TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult) const
	{
		if( m_pTwh ) {
			if( msg == WM_KEYDOWN && wparam == VK_RETURN ) {
				if( !m_bWantReturn || (::GetKeyState(VK_CONTROL) < 0 && !m_bWantCtrlReturn) ) {
					if( m_pManager != NULL ) m_pManager->SendNotify((CControlUI*)this, DUI_MSGTYPE_RETURN);
					return S_OK;
				}
			}
			return SendRichEditTextServiceMessage(m_pTwh, msg, wparam, lparam, plresult);
		}
		return S_FALSE;
	}

	IDropTarget* CRichEditUI::GetTxDropTarget()
	{
		ITextServices* pTextServices = GetRichEditTextServices(m_pTwh);
		IDropTarget *pdt = NULL;
		if( pTextServices != NULL && pTextServices->TxGetDropTarget(&pdt) == NOERROR ) return pdt;
		return NULL;
	}

	bool CRichEditUI::OnTxViewChanged()
	{
		return true;
	}

	bool CRichEditUI::SetDropAcceptFile(bool bAccept) 
	{
		LRESULT lResult;
		TxSendMessage(EM_SETEVENTMASK, 0,ENM_DROPFILES|ENM_LINK, // ENM_CHANGE| ENM_CORRECTTEXT | ENM_DRAGDROPDONE | ENM_DROPFILES | ENM_IMECHANGE | ENM_LINK | ENM_OBJECTPOSITIONS | ENM_PROTECTED | ENM_REQUESTRESIZE | ENM_SCROLL | ENM_SELCHANGE | ENM_UPDATE,
			&lResult);
		return (BOOL)lResult == FALSE;
	}

	void CRichEditUI::OnTxNotify(DWORD iNotify, void *pv)
	{
		switch(iNotify)
		{ 
		case EN_CHANGE:
		{
			GetManager()->SendNotify(this, DUI_MSGTYPE_TEXTCHANGED);
			break;
		}
		case EN_DROPFILES:   
		case EN_MSGFILTER:   
		case EN_OLEOPFAILED:   
		case EN_PROTECTED:   
		case EN_SAVECLIPBOARD:   
		case EN_SELCHANGE:   
		case EN_STOPNOUNDO:   
		case EN_LINK:   
		case EN_OBJECTPOSITIONS:   
		case EN_DRAGDROPDONE:   
		{
			NotifyRichEditHostWindow(this, iNotify, pv);
		}
		break;
		}
	}

	CDuiSize CRichEditUI::GetNaturalSize(LONG width, LONG height)
	{
		if (width < 0)
		{
			width = 0;
		}
		if (height < 0)
		{
			height = 0;
		}
		CDuiSize sz(0, 0);
		LONG lWidth = width;
		LONG lHeight = height;
		if (m_pTwh) {
			TryMeasureRichEditNaturalSizeForManager(m_pTwh, m_pManager, lWidth, lHeight);
		}
		sz.cx = (int)lWidth;
		sz.cy = (int)lHeight;
		return sz;
	}
	int CRichEditUI::GetTextContentHeight(int nRichEditWidth)
	{
		if (!m_pTwh) return 0;

		if ( nRichEditWidth == 0 )
			nRichEditWidth = GetPos ().right - GetPos ().left;

		RECT rcTextPadding = GetTextPadding();
		LONG lWidth = (nRichEditWidth) - rcTextPadding.left - rcTextPadding.right;
		if (lWidth <= 0) lWidth = 1;
		LONG lHeight = 0;
		TryMeasureRichEditNaturalSizeForManager(m_pTwh, m_pManager, lWidth, lHeight);

		// 閸旂姳绗?TextPadding 閻ㄥ嫪绗傛稉瀣敶鏉堢绐?
		lHeight += rcTextPadding.top + rcTextPadding.bottom;
		return (int)lHeight;
	}

	// 婢舵俺顢戦棃鐎漣ch閺嶇厧绱￠惃鍓卛chedit閺堝绔存稉顏呯泊閸斻劍娼痓ug閿涘苯婀張鈧崥搴濈鐞涘本妲哥粚楦款攽閺冭绱滾ineDown閸滃etScrollPos閺冪姵纭跺姘З閸掔増娓堕崥?
	// 瀵洖鍙唅Pos鐏忚鲸妲告稉杞扮啊娣囶喗顒滄潻娆庨嚋bug
	void CRichEditUI::SetScrollPos(SIZE szPos, bool bMsg,bool bScroolVisible)
	{
		int cx = 0;
		int cy = 0;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
			m_pVerticalScrollBar->SetScrollPos(szPos.cy);
			cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
		}
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
			m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
			cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
		}
		if( cy != 0 ) {
			int iPos = 0;
			if( m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
				iPos = m_pVerticalScrollBar->GetScrollPos();
			WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, m_pVerticalScrollBar->GetScrollPos());
			TxSendMessage(WM_VSCROLL, wParam, 0L, 0);
			if( m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
				if( cy > 0 && m_pVerticalScrollBar->GetScrollPos() <= iPos )
					m_pVerticalScrollBar->SetScrollPos(iPos);
			}
		}
		if( cx != 0 ) {
			WPARAM wParam = MAKEWPARAM(SB_THUMBPOSITION, m_pHorizontalScrollBar->GetScrollPos());
			TxSendMessage(WM_HSCROLL, wParam, 0L, 0);
		}
	}

	void CRichEditUI::LineUp(bool bScroolVisible)
	{
		TxSendMessage(WM_VSCROLL, SB_LINEUP, 0L, 0);
	}

	void CRichEditUI::LineDown(bool bScroolVisible)
	{
		int iPos = 0;
		if( m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) 
			iPos = m_pVerticalScrollBar->GetScrollPos();
		TxSendMessage(WM_VSCROLL, SB_LINEDOWN, 0L, 0);
		if( m_pTwh && !m_bRich && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			if( m_pVerticalScrollBar->GetScrollPos() <= iPos )
				m_pVerticalScrollBar->SetScrollPos(m_pVerticalScrollBar->GetScrollRange());
		}
	}

	void CRichEditUI::PageUp()
	{
		TxSendMessage(WM_VSCROLL, SB_PAGEUP, 0L, 0);
	}

	void CRichEditUI::PageDown()
	{
		TxSendMessage(WM_VSCROLL, SB_PAGEDOWN, 0L, 0);
	}

	void CRichEditUI::HomeUp()
	{
		TxSendMessage(WM_VSCROLL, SB_TOP, 0L, 0);
	}

	void CRichEditUI::EndDown()
	{
		TxSendMessage(WM_VSCROLL, SB_BOTTOM, 0L, 0);
	}

	void CRichEditUI::LineLeft()
	{
		TxSendMessage(WM_HSCROLL, SB_LINELEFT, 0L, 0);
	}

	void CRichEditUI::LineRight()
	{
		TxSendMessage(WM_HSCROLL, SB_LINERIGHT, 0L, 0);
	}

	void CRichEditUI::PageLeft()
	{
		TxSendMessage(WM_HSCROLL, SB_PAGELEFT, 0L, 0);
	}

	void CRichEditUI::PageRight()
	{
		TxSendMessage(WM_HSCROLL, SB_PAGERIGHT, 0L, 0);
	}

	void CRichEditUI::HomeLeft()
	{
		TxSendMessage(WM_HSCROLL, SB_LEFT, 0L, 0);
	}

	void CRichEditUI::EndRight()
	{
		TxSendMessage(WM_HSCROLL, SB_RIGHT, 0L, 0);
	}

	void CRichEditUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETCURSOR && IsEnabled() )
		{
			if( m_pTwh && m_pTwh->DoSetCursor(NULL, &event.ptMouse) ) {
				return;
			}
		}
		else if( event.Type == UIEVENT_WINDOWSIZE ) {
			if( m_pTwh ) m_pTwh->NeedFreshCaret();
		}

		else if( event.Type == UIEVENT_SETFOCUS ) {
			ApplyRichEditFocusState(m_pTwh, m_pManager, true, DEFAULT_TIMERID, m_bFocused, this);
			return;
		}

		if( event.Type == UIEVENT_KILLFOCUS )  {
			ApplyRichEditFocusState(m_pTwh, m_pManager, false, DEFAULT_TIMERID, m_bFocused, this);
			return;
		}
		else if( event.Type == UIEVENT_TIMER ) {
			if( event.wParam == DEFAULT_TIMERID )
			{
				HandleRichEditDefaultTimer(m_pTwh, m_pManager, IsFocused(), m_bDrawCaret, m_rcItem, this);
				return;
			}
			ForwardRichEditTextServiceTimer(m_pTwh, event.wParam, event.lParam);
			return;
		}
		if( event.Type == UIEVENT_SCROLLWHEEL ) {
			if( (event.wKeyState & MK_CONTROL) != 0  ) {
				return;
			}
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) 
		{
			return;
		}

		if( event.Type == UIEVENT_MOUSEMOVE ) 
		{
			return;
		}

		if( event.Type == UIEVENT_MOUSEENTER ) 
		{
			return;
		}
		if( event.Type == UIEVENT_BUTTONUP ) 
		{
			return;
		}
		if( event.Type > UIEVENT__KEYBEGIN && event.Type < UIEVENT__KEYEND )
		{
			return;
		}
		CContainerUI::DoEvent(event);
	}

	SIZE CRichEditUI::EstimateSize(SIZE szAvailable)
	{
		return CContainerUI::EstimateSize(szAvailable);
	}

	void CRichEditUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;

		RECT rcScrollView = rc;
		bool bVScrollBarVisiable = false;
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			bVScrollBarVisiable = true;
			rc.top -= m_pVerticalScrollBar->GetScrollPos();
			rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
			rc.bottom += m_pVerticalScrollBar->GetScrollRange();
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
			rcScrollView.right -= m_pVerticalScrollBar->GetFixedWidth();
		}
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			rc.left -= m_pHorizontalScrollBar->GetScrollPos();
			rc.right -= m_pHorizontalScrollBar->GetScrollPos();
			rc.right += m_pHorizontalScrollBar->GetScrollRange();
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
			rcScrollView.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
		}

		if( m_pTwh != NULL ) {
			RECT rcTextPadding = GetTextPadding();
			RECT rcScrollTextView = rcScrollView;
			rcScrollTextView.left += rcTextPadding.left;
			rcScrollTextView.right -= rcTextPadding.right;
			rcScrollTextView.top += rcTextPadding.top;
			rcScrollTextView.bottom -= rcTextPadding.bottom;
			RECT rcText = rc;
			rcText.left += rcTextPadding.left;
			rcText.right -= rcTextPadding.right;
			rcText.top += rcTextPadding.top;
			rcText.bottom -= rcTextPadding.bottom;
			m_pTwh->SetClientRect(&rcScrollTextView);

			if( bVScrollBarVisiable && (!m_pVerticalScrollBar->IsVisible() || m_bVScrollBarFixing) ) {
				LONG lWidth = rcText.right - rcText.left + m_pVerticalScrollBar->GetFixedWidth();
				LONG lHeight = 0;
				if( TryMeasureRichEditNaturalSizeForManager(m_pTwh, GetManager(), lWidth, lHeight) &&
					lHeight > rcText.bottom - rcText.top ) {
					m_pVerticalScrollBar->SetVisible(true);
					m_pVerticalScrollBar->SetScrollPos(0);
					m_bVScrollBarFixing = true;
				}
				else {
					if( m_bVScrollBarFixing ) {
						m_pVerticalScrollBar->SetVisible(false);
						m_bVScrollBarFixing = false;
					}
				}
			}
		}

		if( m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() ) {
			RECT rcScrollBarPos = { rcScrollView.right, rcScrollView.top, 
				rcScrollView.right + m_pVerticalScrollBar->GetFixedWidth(), rcScrollView.bottom};
			m_pVerticalScrollBar->SetPos(rcScrollBarPos, false);
		}
		if( m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() ) {
			RECT rcScrollBarPos = { rcScrollView.left, rcScrollView.bottom, rcScrollView.right, 
				rcScrollView.bottom + m_pHorizontalScrollBar->GetFixedHeight()};
			m_pHorizontalScrollBar->SetPos(rcScrollBarPos, false);
		}

		for( int it = 0; it < m_items.GetSize(); it++ ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it);
			}
			else {
				SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };
				if( sz.cx < pControl->GetMinWidth() ) sz.cx = pControl->GetMinWidth();
				if( sz.cx > pControl->GetMaxWidth() ) sz.cx = pControl->GetMaxWidth();
				if( sz.cy < pControl->GetMinHeight() ) sz.cy = pControl->GetMinHeight();
				if( sz.cy > pControl->GetMaxHeight() ) sz.cy = pControl->GetMaxHeight();
				RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy };
				pControl->SetPos(rcCtrl, false);
			}
		}
	}

	void CRichEditUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CContainerUI::Move(szOffset, bNeedInvalidate);
		if( m_pTwh != NULL ) {
			RECT rc = m_rcItem;
			RECT rcInset = GetInset();
			rc.left += rcInset.left;
			rc.top += rcInset.top;
			rc.right -= rcInset.right;
			rc.bottom -= rcInset.bottom;

			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
			if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
			m_pTwh->SetClientRect(&rc);
		}
	}

	static void DrawRichEditTextServices(CTxtWinHost* pTextHost, HDC hDrawDC, const RECT& rcPaint)
	{
		ITextServices* pTextServices = GetRichEditTextServices(pTextHost);
		if (pTextServices == NULL || hDrawDC == NULL) {
			return;
		}

		RECT rcControl = {};
		pTextHost->GetControlRect(&rcControl);
		pTextServices->TxDraw(
			DVASPECT_CONTENT,
			0,
			NULL,
			NULL,
			hDrawDC,
			NULL,
			(RECTL*)&rcControl,
			NULL,
			const_cast<RECT*>(&rcPaint),
			NULL,
			NULL,
			0);
	}

	static bool TryMeasureRichEditNaturalSize(CTxtWinHost* pTextHost, HDC hMeasureDC, LONG& lWidth, LONG& lHeight)
	{
		ITextServices* pTextServices = GetRichEditTextServices(pTextHost);
		if (pTextServices == NULL || hMeasureDC == NULL) {
			return false;
		}

		SIZEL szExtent = { -1, -1 };
		const HRESULT hr = pTextServices->TxGetNaturalSize(
			DVASPECT_CONTENT,
			hMeasureDC,
			NULL,
			NULL,
			TXTNS_FITTOCONTENT,
			&szExtent,
			&lWidth,
			&lHeight);
		return SUCCEEDED(hr);
	}

	static bool TryMeasureRichEditNaturalSizeForManager(CTxtWinHost* pTextHost, CPaintManagerUI* pManager, LONG& lWidth, LONG& lHeight)
	{
		return TryMeasureRichEditNaturalSize(pTextHost, GetManagerMeasureDC(pManager), lWidth, lHeight);
	}

	static bool ShouldRefreshRichEditVScrollFix(CTxtWinHost* pTextHost, CPaintManagerUI* pManager, LONG lWidth, LONG lAvailableHeight)
	{
		LONG lHeight = 0;
		return TryMeasureRichEditNaturalSizeForManager(pTextHost, pManager, lWidth, lHeight) && lHeight <= lAvailableHeight;
	}

	bool CRichEditUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		HDC hDC = renderContext.GetDC();
		const RECT& rcPaint = renderContext.GetPaintRect();
		RECT rcTemp = { 0 };
		if( !::IntersectRect(&rcTemp, &rcPaint, &m_rcItem) ) return true;

		CRenderClip clip;
		CRenderClip::GenerateClip(renderContext, rcTemp, clip);
		CControlUI::DoPaint(renderContext, pStopControl);

		if( m_pTwh ) {
			FlushDirect2DBatchForContextInternal(renderContext);
			DrawRichEditTextServices(m_pTwh, hDC, rcTemp);
			if( m_bVScrollBarFixing ) {
				RECT rc = {};
				m_pTwh->GetControlRect(&rc);
				LONG lWidth = rc.right - rc.left + m_pVerticalScrollBar->GetFixedWidth();
				if( ShouldRefreshRichEditVScrollFix(m_pTwh, GetManager(), lWidth, rc.bottom - rc.top) ) {
					NeedUpdate();
				}
			}
		}

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
					if( !::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos()) ) continue;
					if( pControl->IsFloat() ) {
						if( !::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos()) ) continue;
						CRenderClip::UseOldClipBegin(renderContext, childClip);
						if( !pControl->Paint(renderContext, pStopControl) ) return false;
						CRenderClip::UseOldClipEnd(renderContext, childClip);
					}
					else {
						if( !::IntersectRect(&rcTemp, &rc, &pControl->GetPos()) ) continue;
						if( !pControl->Paint(renderContext, pStopControl) ) return false;
					}
				}
			}
		}

		DrawLayeredRichEditCaret(hDC, m_pManager, IsFocused(), m_bDrawCaret, m_pTwh, m_rcItem, GetTextColor());

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
		// 缂佹ê鍩楅幓鎰仛閺傚洤鐡?
		std::wstring sDrawText = GetText();
		if(sDrawText.empty() && !m_bFocused) {
			DWORD dwTextColor = GetTipValueColor();
			std::wstring sTipValue(GetTipValue());
			RECT rc = m_rcItem;
			RECT rcTextPadding = GetTipValuePadding();
			rc.left += rcTextPadding.left;
			rc.right -= rcTextPadding.right;
			rc.top += rcTextPadding.top;
			rc.bottom -= rcTextPadding.bottom;
			UINT uTextAlign = GetTipValueAlign();
			if(IsMultiLine()) uTextAlign |= DT_TOP;
			else uTextAlign |= DT_VCENTER;
			CPaintRenderContext renderContext = CreateRichEditRenderContext(m_pManager, hDC, rc);
			CRenderEngine::DrawText(renderContext, rc, sTipValue, dwTextColor, m_iFont, uTextAlign);
		}
		return true;
	}

	bool CRichEditUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
	{
		CPaintRenderContext renderContext = CreateRichEditRenderContext(m_pManager, hDC, rcPaint);
		return DoPaint(renderContext, pStopControl);
	}

	std::wstring_view CRichEditUI::GetNormalImage()
	{
		return m_sNormalImage;
	}

	void CRichEditUI::SetNormalImage(std::wstring_view pStrImage)
	{
		m_sNormalImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CRichEditUI::GetHotImage()
	{
		return m_sHotImage;
	}

	void CRichEditUI::SetHotImage(std::wstring_view pStrImage)
	{
		m_sHotImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CRichEditUI::GetFocusedImage()
	{
		return m_sFocusedImage;
	}

	void CRichEditUI::SetFocusedImage(std::wstring_view pStrImage)
	{
		m_sFocusedImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CRichEditUI::GetDisabledImage()
	{
		return m_sDisabledImage;
	}

	void CRichEditUI::SetDisabledImage(std::wstring_view pStrImage)
	{
		m_sDisabledImage = pStrImage;
		Invalidate();
	}

	RECT CRichEditUI::GetTextPadding() const
	{
		RECT rcTextPadding = m_rcTextPadding;
		if(m_pManager) m_pManager->ScaleRect(&rcTextPadding);
		return rcTextPadding;
	}

	void CRichEditUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		Invalidate();
	}

	void CRichEditUI::SetTipValue(std::wstring_view pStrTipValue)
	{
		m_sTipValue	= pStrTipValue;
		Invalidate();
	}

	std::wstring_view CRichEditUI::GetTipValue()
	{
		return m_sTipValue;
	}

	void CRichEditUI::SetTipValueColor(std::wstring_view pStrColorView)
	{
		DWORD color = 0;
		if (StringUtil::TryParseColor(pStrColorView, color)) {
			m_dwTipValueColor = color;
			Invalidate();
		}
	}

	DWORD CRichEditUI::GetTipValueColor()
	{
		return m_dwTipValueColor;
	}

	void CRichEditUI::SetTipValueAlign(UINT uAlign)
	{
		m_uTipValueAlign = uAlign;
		if(GetText().empty()) Invalidate();
	}

	UINT CRichEditUI::GetTipValueAlign()
	{
		return m_uTipValueAlign;
	}

	void CRichEditUI::SetTipValuePadding(RECT rcTipValuePadding)
	{
		m_rcTipValuePadding = rcTipValuePadding;	
	}

	RECT CRichEditUI::GetTipValuePadding()
	{
		return m_rcTipValuePadding;
	}

	void CRichEditUI::PaintStatusImage(CPaintRenderContext& renderContext)
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
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusedImage.empty() ) {
				if( !DrawImage(renderContext, m_sFocusedImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT ) != 0 ) {
			if( !m_sHotImage.empty() ) {
				if( !DrawImage(renderContext, m_sHotImage) ) {}
				else return;
			}
		}

		if( !m_sNormalImage.empty() ) {
			if( !DrawImage(renderContext, m_sNormalImage) ) {}
			else return;
		}
	}

	void CRichEditUI::PaintStatusImage(HDC hDC)
	{
		RECT rcPaint = m_rcPaint;
		if (rcPaint.right <= rcPaint.left || rcPaint.bottom <= rcPaint.top) {
			rcPaint = m_rcItem;
		}
		CPaintRenderContext renderContext = CreateRichEditRenderContext(m_pManager, hDC, rcPaint);
		PaintStatusImage(renderContext);
	}

	void CRichEditUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
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

		if (StringUtil::EqualsNoCase(name, L"vscrollbar")) {
			if (StringUtil::ParseBool(pstrValueView)) m_lTwhStyle |= ES_DISABLENOSCROLL | WS_VSCROLL;
			if (m_pTwh) m_pTwh->TxEnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);
		}
		else if (StringUtil::EqualsNoCase(name, L"autovscroll")) {
			if (StringUtil::ParseBool(pstrValueView)) m_lTwhStyle |= ES_AUTOVSCROLL;
			if (m_pTwh) m_pTwh->TxShowScrollBar(SB_VERT, true);
		}
		else if (StringUtil::EqualsNoCase(name, L"hscrollbar")) {
			if (StringUtil::ParseBool(pstrValueView)) m_lTwhStyle |= ES_DISABLENOSCROLL | WS_HSCROLL;
			if (m_pTwh) m_pTwh->TxEnableScrollBar(SB_HORZ, ESB_ENABLE_BOTH);
		}
		else if (StringUtil::EqualsNoCase(name, L"autohscroll")) {
			if (StringUtil::ParseBool(pstrValueView)) m_lTwhStyle |= ES_AUTOHSCROLL;
			if (m_pTwh) m_pTwh->TxShowScrollBar(SB_HORZ, true);
		}
		else if (StringUtil::EqualsNoCase(name, L"multiline")) {
			SetMultiLine(StringUtil::ParseBool(pstrValueView));
		}
		else if (StringUtil::EqualsNoCase(name, L"wanttab")) {
			SetWantTab(StringUtil::ParseBool(pstrValueView));
		}
		else if (StringUtil::EqualsNoCase(name, L"wantreturn")) {
			SetWantReturn(StringUtil::ParseBool(pstrValueView));
		}
		else if (StringUtil::EqualsNoCase(name, L"wantctrlreturn")) {
			SetWantCtrlReturn(StringUtil::ParseBool(pstrValueView));
		}
		else if (StringUtil::EqualsNoCase(name, L"transparent")) {
			SetTransparent(StringUtil::ParseBool(pstrValueView));
		}
		else if (StringUtil::EqualsNoCase(name, L"rich")) {
			SetRich(StringUtil::ParseBool(pstrValueView));
		}
		else if (StringUtil::EqualsNoCase(name, L"readonly")) {
			if (StringUtil::ParseBool(pstrValueView)) {
				m_lTwhStyle |= ES_READONLY;
				m_bReadOnly = true;
			}
		}
		else if (StringUtil::EqualsNoCase(name, L"password")) {
			if (StringUtil::ParseBool(pstrValueView)) m_lTwhStyle |= ES_PASSWORD;
		}
		else if (StringUtil::EqualsNoCase(name, L"align")) {
			if (contains(L"left")) {
				m_lTwhStyle &= ~(ES_CENTER | ES_RIGHT);
				m_lTwhStyle |= ES_LEFT;
			}
			if (contains(L"center")) {
				m_lTwhStyle &= ~(ES_LEFT | ES_RIGHT);
				m_lTwhStyle |= ES_CENTER;
			}
			if (contains(L"right")) {
				m_lTwhStyle &= ~(ES_LEFT | ES_CENTER);
				m_lTwhStyle |= ES_RIGHT;
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
		else if (StringUtil::EqualsNoCase(name, L"maxchar")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetLimitText(value);
		}
		else if (StringUtil::EqualsNoCase(name, L"limittext")) SetLimitTextString(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"normalimage")) SetNormalImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"hotimage")) SetHotImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"focusedimage")) SetFocusedImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"disabledimage")) SetDisabledImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"textpadding")) {
			RECT rect = { 0 };
			if (StringUtil::TryParseRect(pstrValueView, rect)) SetTextPadding(rect);
		}
		else if (StringUtil::EqualsNoCase(name, L"tipvalue")) SetTipValue(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"tipvaluecolor")) SetTipValueColor(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"tipvaluealign")) {
			if (contains(L"left")) {
				m_uTipValueAlign = DT_SINGLELINE | DT_LEFT;
			}
			if (contains(L"center")) {
				m_uTipValueAlign = DT_SINGLELINE | DT_CENTER;
			}
			if (contains(L"right")) {
				m_uTipValueAlign = DT_SINGLELINE | DT_RIGHT;
			}
		}
		else if (StringUtil::EqualsNoCase(name, L"tipvaluepadding")) {
			RECT rect = { 0 };
			if (StringUtil::TryParseRect(pstrValueView, rect)) SetTipValuePadding(rect);
		}
		else CContainerUI::SetAttribute(pstrNameView, pstrValueView);
	}

	LRESULT CRichEditUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
	{
		if( ShouldIgnoreRichEditMessage(this, uMsg, wParam) ) return 0;

		// WM_IME_COMPOSITION is handled in PrepareRichEditMessageDispatch().
			// 鐟欙絽鍠呭顔胯拫鏉堟挸鍙嗗▔鏇氱秴缂冾喖绱撶敮鍝ユ畱闂傤噣顣?

		bool bWasHandled = true;
		if( !PrepareRichEditMessageDispatch(this, uMsg, wParam, lParam, bHandled, bWasHandled) ) return 0;
		if (!PreprocessCharMessage(uMsg, wParam)) return 0;
		LRESULT lResult = 0;
		HRESULT Hr = TxSendMessage(uMsg, wParam, lParam, &lResult);
		FinalizeMessageDispatch(uMsg, Hr, bWasHandled, bHandled);
		return lResult;
		}

	bool CRichEditUI::PreprocessCharMessage(UINT& uMsg, WPARAM& wParam)
	{
		if (uMsg != WM_CHAR) {
			return true;
		}
#ifndef _UNICODE
		if (IsAccumulateDBCMode())
		{
			if ((GetKeyState(VK_KANA) & 0x1))
			{
				SetAccumulateDBCMode(false);
				m_chLeadByte = 0;
			}
			else
			{
				if (!m_chLeadByte)
				{
					if (IsDBCSLeadByte((WORD)wParam))
					{
						m_chLeadByte = (WORD)wParam << 8;
						return false;
					}
				}
				else
				{
					wParam |= m_chLeadByte;
					uMsg = WM_IME_CHAR;
					m_chLeadByte = 0;
				}
			}
		}
#endif
		return true;
	}

	void CRichEditUI::FinalizeMessageDispatch(UINT uMsg, HRESULT hr, bool bWasHandled, bool& bHandled) const
	{
		if (hr == S_OK) {
			bHandled = bWasHandled;
			return;
		}
		if (IsRichEditKeyboardMessage(uMsg)) {
			bHandled = bWasHandled;
			return;
		}
		if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) {
			CTxtWinHost* pTextHost = GetTextHost();
			if (pTextHost != NULL && pTextHost->IsCaptured()) {
				bHandled = bWasHandled;
			}
		}
	}

	} // namespace DuiLib

FYUI::CRichEditUI* FYUI::CRichEditUI::Clone()
{
	CRichEditUI* pClone = new CRichEditUI();
	pClone->CopyData(this);
	return pClone;
}

void FYUI::CRichEditUI::CopyData(CRichEditUI* pControl)
{
	// 缁楊兛绔村銉窗閸忓牐顔曠純?DoInit 閹碘偓闂団偓閻ㄥ嫭鍨氶崨妯哄綁闁插骏绱欏銈嗘 m_pTwh=NULL閿涘奔绮庢穱顔芥暭閹存劕鎲抽崣姗€鍣洪敍?
	// DoInit -> CreateHost -> CTxtWinHost::Init 娴兼矮绮犳潻娆庣昂閹存劕鎲抽崣姗€鍣虹拠璇插絿閸婂吋娼甸崚婵嗩潗閸?TextServices
	SetWinStyle(pControl->GetWinStyle());

	SetMultiLine(pControl->IsMultiLine());
	SetWantTab(pControl->IsWantTab());
	SetWantReturn(pControl->IsWantReturn());
	SetWantCtrlReturn(pControl->IsWantCtrlReturn());
	SetTransparent(pControl->IsTransparent());
	SetRich(pControl->IsRich());
	SetReadOnly(pControl->IsReadOnly());
	SetWordWrap(pControl->IsWordWrap());
	SetTextColor(pControl->GetTextColor());
	SetFont(pControl->GetFont());
	SetLimitText(pControl->GetLimitText());

	SetNormalImage(pControl->GetNormalImage());
	SetHotImage(pControl->GetHotImage());
	SetFocusedImage(pControl->GetFocusedImage());
	SetDisabledImage(pControl->GetDisabledImage());

	SetTextPadding(pControl->m_rcTextPadding);

	SetTipValue(pControl->GetTipValue());
	SetTipValueAlign(pControl->GetTipValueAlign());
	SetTipValuePadding(pControl->GetTipValuePadding());

	m_bVScrollBarFixing = pControl->m_bVScrollBarFixing;
	m_bDrawCaret = pControl->m_bDrawCaret;

	m_fAccumulateDBC  = pControl->m_fAccumulateDBC;
	m_chLeadByte = pControl->m_chLeadByte; 
	m_dwTipValueColor =pControl->m_dwTipValueColor;
	m_uButtonState = pControl->m_uButtonState;
	m_bInited = false;

	// 缁楊兛绨╁銉窗鐠嬪啰鏁ら悥鍓佽 CopyData
	// CContainerUI::CopyData 閸愬懘鍎存导姘崇殶閻?SetManager(m_pManager, nullptr, true)
	// 鏉╂瑤绱扮憴锕€褰?Init() -> DoInit() -> CreateHost()閿涘奔绮犻懓灞藉灡瀵?m_pTwh
	__super::CopyData(pControl);

	// 缁楊兛绗佸銉窗m_pTwh 瀹告煡鈧俺绻?DoInit 閸掓稑缂撶€瑰本鐦敍宀€骞囬崷銊ュ讲娴犮儱鐣ㄩ崗銊ユ勾鐠佸墽鐤嗛弬鍥ㄦ拱
	// 娑斿澧犻弨鎯ф躬 __super::CopyData 閸撳秹娼伴弮璁圭礉m_pTwh 娑?NULL閿涘etText 閺冪姵纭堕崥灞绢劄閸?TextServices
	if(m_pTwh) {
		SetText(pControl->GetText());
	}
}



