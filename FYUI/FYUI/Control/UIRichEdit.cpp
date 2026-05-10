#include "pch.h"
#include "UIRichEdit.h"

#include <algorithm>
#include <imm.h>

#pragma comment(lib, "imm32.lib")

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CRichEditUI)

	namespace
	{
		const LONG kDefaultTextMax = (32 * 1024) - 1;
		const DWORD kSelectionColor = 0x663399FF;
		const DWORD kCaretColor = 0xFF1F2937;
		// caret 在文本末尾时需要在视图右边界内预留的像素数，避免与 rcView.right 相切后被 IntersectRect 裁成空矩形
		const int kCaretMargin = 2;
		const UINT kRichEditMenuUndo = 51001;
		const UINT kRichEditMenuRedo = 51002;
		const UINT kRichEditMenuCut = 51003;
		const UINT kRichEditMenuCopy = 51004;
		const UINT kRichEditMenuPaste = 51005;
		const UINT kRichEditMenuDelete = 51006;
		const UINT kRichEditMenuSelectAll = 51007;

		size_t ClampIndex(long value, size_t length)
		{
			if (value < 0) return length;
			return (std::min)(static_cast<size_t>(value), length);
		}

		void NormalizeRange(size_t& start, size_t& end)
		{
			if (start > end) std::swap(start, end);
		}

		bool IsPrintableChar(wchar_t ch)
		{
			return ch >= L' ' || ch == L'\t';
		}
	}

	CRichEditUI::CRichEditUI()
		: m_bWantTab(true),
		m_bWantReturn(true),
		m_bWantCtrlReturn(true),
		m_bTransparent(true),
		m_bRich(true),
		m_bReadOnly(false),
		m_bPasswordMode(false),
		m_bWordWrap(true),
		m_bHideSelection(false),
		m_bAutoURLDetect(false),
		m_bModified(false),
		m_bDrawCaret(true),
		m_bInited(false),
		m_bDraggingSelection(false),
		m_fAccumulateDBC(false),
		m_chLeadByte(0),
		m_dwTextColor(0),
		m_uTextStyle(DT_LEFT | DT_TOP),
		m_dwEventMask(0),
		m_iFont(-1),
		m_iLimitText(kDefaultTextMax),
		m_lTwhStyle(ES_MULTILINE),
		m_uUndoLimit(100),
		m_nZoomNum(0),
		m_nZoomDen(0),
		m_nCaret(0),
		m_nAnchor(0),
		m_nPreferredCaretX(-1),
		m_uButtonState(0),
		m_dwTipValueColor(0xFFBAC0C5),
		m_uTipValueAlign(DT_SINGLELINE | DT_LEFT),
		m_rcTipValuePadding{ 0, 0, 0, 0 },
		m_bLayoutDirty(true),
		m_nLayoutWidth(0),
		m_nContentWidth(0),
		m_nContentHeight(0),
		m_nManualScrollX(0)
	{
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
		::ZeroMemory(&m_defaultCharFormat, sizeof(m_defaultCharFormat));
		::ZeroMemory(&m_selectionCharFormat, sizeof(m_selectionCharFormat));
		::ZeroMemory(&m_paraFormat, sizeof(m_paraFormat));
		m_defaultCharFormat.cbSize = sizeof(m_defaultCharFormat);
		m_selectionCharFormat.cbSize = sizeof(m_selectionCharFormat);
		m_paraFormat.cbSize = sizeof(m_paraFormat);
		SetCursor(DUI_IBEAM);
		SetKeyboardEnabled(true);
		EnableScrollBar(true, false);
	}

	CRichEditUI::~CRichEditUI()
	{
		if (m_pManager != NULL) {
			m_pManager->RemovePreMessageFilter(this);
			m_pManager->RemoveMessageFilter(this);
		}
	}

	std::wstring_view CRichEditUI::GetClass() const
	{
		return _T("RichEditUI");
	}

	LPVOID CRichEditUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::EqualsNoCase(pstrName, DUI_CTR_RICHEDIT) ||
			StringUtil::EqualsNoCase(pstrName, DUI_CTR_EDIT) ||
			StringUtil::EqualsNoCase(pstrName, _T("RichEditUI"))) {
			return static_cast<CRichEditUI*>(this);
		}
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CRichEditUI::GetControlFlags() const
	{
		return IsEnabled() ? UIFLAG_TABSTOP | UIFLAG_SETCURSOR : 0;
	}

	void CRichEditUI::SetEnabled(bool bEnabled)
	{
		CContainerUI::SetEnabled(bEnabled);
		// CContainerUI::SetEnabled 仅同步 m_items 中的子控件，但 m_pVerticalScrollBar / m_pHorizontalScrollBar
		// 不在 m_items 列表中，需在此显式同步，否则禁用时依然能拖动滚动条 thumb。
		if (m_pVerticalScrollBar != NULL) m_pVerticalScrollBar->SetEnabled(bEnabled);
		if (m_pHorizontalScrollBar != NULL) m_pHorizontalScrollBar->SetEnabled(bEnabled);
		if (!bEnabled) {
			// 进入禁用态：收拾可能正在进行的交互，避免状态悬挂导致后续行为异常。
			if (m_bDraggingSelection) {
				m_bDraggingSelection = false;
				if (m_pManager && m_pManager->IsCaptured()) m_pManager->ReleaseCapture();
			}
			m_uButtonState &= ~UISTATE_HOT;
		}
		Invalidate();
	}

	bool CRichEditUI::IsMultiLine() { return (m_lTwhStyle & ES_MULTILINE) != 0; }
	void CRichEditUI::SetMultiLine(bool bMultiLine)
	{
		if (bMultiLine) m_lTwhStyle |= ES_MULTILINE;
		else m_lTwhStyle &= ~ES_MULTILINE;
		std::wstring text = GetText();
		NormalizeText(text);
		CControlUI::SetText(text);
		InvalidateLayout();
		UpdateScrollBars();
	}
	bool CRichEditUI::IsWantTab() { return m_bWantTab; }
	void CRichEditUI::SetWantTab(bool bWantTab) { m_bWantTab = bWantTab; }
	bool CRichEditUI::IsWantReturn() { return m_bWantReturn; }
	void CRichEditUI::SetWantReturn(bool bWantReturn) { m_bWantReturn = bWantReturn; }
	bool CRichEditUI::IsWantCtrlReturn() { return m_bWantCtrlReturn; }
	void CRichEditUI::SetWantCtrlReturn(bool bWantCtrlReturn) { m_bWantCtrlReturn = bWantCtrlReturn; }
	bool CRichEditUI::IsTransparent() { return m_bTransparent; }
	void CRichEditUI::SetTransparent(bool bTransparent) { m_bTransparent = bTransparent; Invalidate(); }
	bool CRichEditUI::IsRich() { return m_bRich; }
	void CRichEditUI::SetRich(bool bRich) { m_bRich = bRich; }
	bool CRichEditUI::IsReadOnly() { return m_bReadOnly; }
	void CRichEditUI::SetReadOnly(bool bReadOnly) { m_bReadOnly = bReadOnly; }
	bool CRichEditUI::IsPasswordMode() const { return m_bPasswordMode; }
	void CRichEditUI::SetPasswordMode(bool bPasswordMode)
	{
		if (m_bPasswordMode == bPasswordMode) return;
		m_bPasswordMode = bPasswordMode;
		if (m_bPasswordMode) m_lTwhStyle |= ES_PASSWORD;
		else m_lTwhStyle &= ~ES_PASSWORD;
		InvalidateLayout();
		UpdateScrollBars();
		Invalidate();
	}
	bool CRichEditUI::IsWordWrap() { return m_bWordWrap; }
	void CRichEditUI::SetWordWrap(bool bWordWrap)
	{
		if (m_bWordWrap == bWordWrap) return;
		m_bWordWrap = bWordWrap;
		EnableScrollBar(true, !m_bWordWrap);
		InvalidateLayout();
		UpdateScrollBars();
	}
	int CRichEditUI::GetFont() { return m_iFont; }
	void CRichEditUI::SetFont(int index)
	{
		if (m_iFont == index) return;
		m_iFont = index;
		InvalidateLayout();
		UpdateScrollBars();
	}
	void CRichEditUI::SetFont(std::wstring_view pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic)
	{
		if (m_pManager == NULL) return;
		m_iFont = m_pManager->GetFontIndex(pStrFontName, nSize, bBold, bUnderline, bItalic, false);
		InvalidateLayout();
		UpdateScrollBars();
	}
	LONG CRichEditUI::GetWinStyle() { return m_lTwhStyle; }
	void CRichEditUI::SetWinStyle(LONG lStyle)
	{
		m_lTwhStyle = lStyle;
		m_bReadOnly = (m_lTwhStyle & ES_READONLY) != 0;
		m_bPasswordMode = (m_lTwhStyle & ES_PASSWORD) != 0;
		EnableScrollBar((m_lTwhStyle & WS_VSCROLL) != 0 || IsMultiLine(), (m_lTwhStyle & WS_HSCROLL) != 0);
		InvalidateLayout();
		UpdateScrollBars();
	}
	DWORD CRichEditUI::GetTextColor() { return m_dwTextColor; }
	void CRichEditUI::SetTextColor(DWORD dwTextColor) { m_dwTextColor = dwTextColor; Invalidate(); }
	int CRichEditUI::GetLimitText() { return m_iLimitText; }
	void CRichEditUI::SetLimitText(int iChars) { m_iLimitText = (std::max)(0, iChars); }
	void CRichEditUI::SetLimitTextString(std::wstring_view pStrLimitText) { m_strLimitText.assign(pStrLimitText); }
	std::wstring CRichEditUI::GetLimitTextString() const { return m_strLimitText; }
	long CRichEditUI::GetTextLength(DWORD) const { return static_cast<long>(GetText().size()); }
	std::wstring CRichEditUI::GetText() const { return CControlUI::GetText(); }
	void CRichEditUI::SetText(std::wstring_view pstrText)
	{
		std::wstring text(pstrText);
		NormalizeText(text);
		if (m_iLimitText >= 0 && text.size() > static_cast<size_t>(m_iLimitText)) {
			text.resize(static_cast<size_t>(m_iLimitText));
		}
		CControlUI::SetText(text);
		m_nCaret = m_nAnchor = (std::min)(m_nCaret, text.size());
		InvalidateLayout();
		UpdateScrollBars();
	}
	void CRichEditUI::SetShowText(std::wstring_view pstrText) { SetText(pstrText); }
	bool CRichEditUI::IsModify() const { return m_bModified; }
	void CRichEditUI::SetModify(bool bModified) const { const_cast<CRichEditUI*>(this)->m_bModified = bModified; }

	void CRichEditUI::GetSel(CHARRANGE& cr) const
	{
		size_t start = m_nAnchor;
		size_t end = m_nCaret;
		NormalizeRange(start, end);
		cr.cpMin = static_cast<LONG>(start);
		cr.cpMax = static_cast<LONG>(end);
	}
	void CRichEditUI::GetSel(long& nStartChar, long& nEndChar) const
	{
		CHARRANGE cr = {};
		GetSel(cr);
		nStartChar = cr.cpMin;
		nEndChar = cr.cpMax;
	}
	int CRichEditUI::SetSel(CHARRANGE& cr) { return SetSel(cr.cpMin, cr.cpMax); }
	int CRichEditUI::SetSel(long nStartChar, long nEndChar)
	{
		const size_t length = GetText().size();
		m_nAnchor = ClampIndex(nStartChar, length);
		m_nCaret = ClampIndex(nEndChar, length);
		m_nPreferredCaretX = -1;
		ScrollCaret();
		Invalidate();
		return 0;
	}
	void CRichEditUI::ReplaceSel(std::wstring_view lpszNewText, bool bCanUndo) { ReplaceRange(m_nAnchor, m_nCaret, lpszNewText, bCanUndo); }
	void CRichEditUI::ReplaceShowSel(std::wstring_view lpszNewText, bool bCanUndo) { ReplaceSel(lpszNewText, bCanUndo); }
	void CRichEditUI::ReplaceSelW(LPCWSTR lpszNewText, bool bCanUndo) { ReplaceSel(lpszNewText != NULL ? std::wstring_view(lpszNewText) : std::wstring_view(), bCanUndo); }
	std::wstring CRichEditUI::GetSelText() const
	{
		size_t start = m_nAnchor;
		size_t end = m_nCaret;
		NormalizeRange(start, end);
		const std::wstring text = GetText();
		return text.substr(start, end - start);
	}
	int CRichEditUI::SetSelAll() { return SetSel(0, -1); }
	int CRichEditUI::SetSelNone() { return SetSel(static_cast<long>(m_nCaret), static_cast<long>(m_nCaret)); }
	WORD CRichEditUI::GetSelectionType() const { return m_nCaret == m_nAnchor ? SEL_EMPTY : SEL_TEXT; }
	bool CRichEditUI::GetZoom(int& nNum, int& nDen) const { nNum = m_nZoomNum; nDen = m_nZoomDen; return m_nZoomNum > 0 && m_nZoomDen > 0; }
	bool CRichEditUI::SetZoom(int nNum, int nDen) { if (nNum <= 0 || nDen <= 0) return false; m_nZoomNum = nNum; m_nZoomDen = nDen; InvalidateLayout(); return true; }
	bool CRichEditUI::SetZoomOff() { m_nZoomNum = 0; m_nZoomDen = 0; InvalidateLayout(); return true; }
	bool CRichEditUI::GetAutoURLDetect() const { return m_bAutoURLDetect; }
	bool CRichEditUI::SetAutoURLDetect(bool bAutoDetect) { m_bAutoURLDetect = bAutoDetect; return true; }
	DWORD CRichEditUI::GetEventMask() const { return m_dwEventMask; }
	DWORD CRichEditUI::SetEventMask(DWORD dwEventMask) { const DWORD old = m_dwEventMask; m_dwEventMask = dwEventMask; return old; }
	std::wstring CRichEditUI::GetTextRange(long nStartChar, long nEndChar) const
	{
		const std::wstring text = GetText();
		size_t start = ClampIndex(nStartChar, text.size());
		size_t end = ClampIndex(nEndChar, text.size());
		NormalizeRange(start, end);
		return text.substr(start, end - start);
	}
	void CRichEditUI::HideSelection(bool bHide, bool) { m_bHideSelection = bHide; Invalidate(); }
	void CRichEditUI::ScrollCaret() { ScrollToCaret(); }
	int CRichEditUI::InsertText(long nInsertAfterChar, std::wstring_view lpstrText, bool bCanUndo)
	{
		const size_t insertPos = ClampIndex(nInsertAfterChar + 1, GetText().size());
		ReplaceRange(insertPos, insertPos, lpstrText, bCanUndo);
		return static_cast<int>(insertPos + lpstrText.size());
	}
	int CRichEditUI::AppendText(std::wstring_view lpstrText, bool bCanUndo)
	{
		ReplaceRange(GetText().size(), GetText().size(), lpstrText, bCanUndo);
		return static_cast<int>(GetText().size());
	}

	DWORD CRichEditUI::GetDefaultCharFormat(CHARFORMAT2& cf) const { cf = m_defaultCharFormat; return cf.dwMask; }
	bool CRichEditUI::SetDefaultCharFormat(CHARFORMAT2& cf) { m_defaultCharFormat = reinterpret_cast<CHARFORMAT2W&>(cf); return true; }
	DWORD CRichEditUI::GetSelectionCharFormat(CHARFORMAT2& cf) const { cf = reinterpret_cast<const CHARFORMAT2&>(m_selectionCharFormat); return cf.dwMask; }
	bool CRichEditUI::SetSelectionCharFormat(CHARFORMAT2& cf) { m_selectionCharFormat = reinterpret_cast<CHARFORMAT2W&>(cf); return true; }
	bool CRichEditUI::SetWordCharFormat(CHARFORMAT2& cf) { return SetSelectionCharFormat(cf); }
	DWORD CRichEditUI::GetParaFormat(PARAFORMAT2& pf) const { pf = m_paraFormat; return pf.dwMask; }
	bool CRichEditUI::SetParaFormat(PARAFORMAT2& pf) { m_paraFormat = pf; return true; }

	bool CRichEditUI::CanUndo() { return !m_undoStack.empty(); }
	bool CRichEditUI::CanRedo() { return !m_redoStack.empty(); }
	bool CRichEditUI::CanPaste() { return !m_bReadOnly && ::IsClipboardFormatAvailable(CF_UNICODETEXT) != FALSE; }
	bool CRichEditUI::Redo()
	{
		if (m_redoStack.empty()) return false;
		m_undoStack.push_back(GetText());
		CControlUI::SetText(m_redoStack.back());
		m_redoStack.pop_back();
		m_nCaret = m_nAnchor = GetText().size();
		MarkTextChanged(false);
		return true;
	}
	bool CRichEditUI::Undo()
	{
		if (m_undoStack.empty()) return false;
		m_redoStack.push_back(GetText());
		CControlUI::SetText(m_undoStack.back());
		m_undoStack.pop_back();
		m_nCaret = m_nAnchor = GetText().size();
		MarkTextChanged(false);
		return true;
	}
	void CRichEditUI::Clear() { if (!m_bReadOnly) DeleteSelection(true); }
	void CRichEditUI::Copy() { SetClipboardText(GetSelText()); }
	void CRichEditUI::Cut() { if (!m_bReadOnly) { Copy(); DeleteSelection(true); } }
	void CRichEditUI::Paste() { if (!m_bReadOnly) ReplaceSel(GetClipboardText(), true); }

	int CRichEditUI::GetLineCount() const { EnsureLayout(); return static_cast<int>(m_lines.size()); }
	std::wstring CRichEditUI::GetLine(int nIndex, int nMaxLength) const
	{
		EnsureLayout();
		if (nIndex < 0 || nIndex >= static_cast<int>(m_lines.size())) return {};
		const TextLine& line = m_lines[static_cast<size_t>(nIndex)];
		std::wstring value = GetText().substr(line.start, line.length);
		if (nMaxLength >= 0 && value.size() > static_cast<size_t>(nMaxLength)) value.resize(static_cast<size_t>(nMaxLength));
		return value;
	}
	int CRichEditUI::LineIndex(int nLine) const
	{
		EnsureLayout();
		const size_t lineIndex = nLine < 0 ? CurrentLineIndex() : static_cast<size_t>(nLine);
		return lineIndex < m_lines.size() ? static_cast<int>(m_lines[lineIndex].start) : -1;
	}
	int CRichEditUI::LineLength(int nLine) const
	{
		EnsureLayout();
		const size_t lineIndex = nLine < 0 ? CurrentLineIndex() : static_cast<size_t>(nLine);
		return lineIndex < m_lines.size() ? static_cast<int>(m_lines[lineIndex].length) : 0;
	}
	bool CRichEditUI::LineScroll(int nLines, int nChars)
	{
		SIZE pos = GetScrollPos();
		pos.cy += nLines * GetLineHeight();
		pos.cx += nChars * (std::max)(1, MeasureTextWidth(L"M"));
		SetScrollPos(pos);
		return true;
	}
	CDuiPoint CRichEditUI::GetCharPos(long lChar) const { return CharPos(ClampIndex(lChar, GetText().size())); }
	long CRichEditUI::LineFromChar(long nIndex) const
	{
		EnsureLayout();
		const size_t index = ClampIndex(nIndex, GetText().size());
		for (size_t i = 0; i < m_lines.size(); ++i) {
			if (index >= m_lines[i].start && index <= m_lines[i].start + m_lines[i].length) return static_cast<long>(i);
		}
		return static_cast<long>(m_lines.empty() ? 0 : m_lines.size() - 1);
	}
	CDuiPoint CRichEditUI::PosFromChar(UINT nChar) const { return CharPos((std::min)(static_cast<size_t>(nChar), GetText().size())); }
	int CRichEditUI::CharFromPos(CDuiPoint pt) const { return static_cast<int>(HitTest(pt)); }
	void CRichEditUI::EmptyUndoBuffer() { m_undoStack.clear(); m_redoStack.clear(); }
	UINT CRichEditUI::SetUndoLimit(UINT nLimit)
	{
		const UINT old = m_uUndoLimit;
		m_uUndoLimit = nLimit;
		while (m_undoStack.size() > m_uUndoLimit) m_undoStack.erase(m_undoStack.begin());
		return old;
	}
	long CRichEditUI::StreamIn(int, EDITSTREAM& es)
	{
		if (es.pfnCallback == NULL) return 0;
		std::string bytes;
		char buffer[4096] = {};
		LONG read = 0;
		while (es.pfnCallback(es.dwCookie, reinterpret_cast<LPBYTE>(buffer), sizeof(buffer), &read) == 0 && read > 0) {
			bytes.append(buffer, buffer + static_cast<size_t>(read));
		}
		std::wstring text(bytes.begin(), bytes.end());
		SetText(text);
		return static_cast<long>(GetTextLength());
	}
	long CRichEditUI::StreamOut(int, EDITSTREAM& es)
	{
		if (es.pfnCallback == NULL) return 0;
		const std::wstring text = GetText();
		LONG written = 0;
		es.pfnCallback(es.dwCookie, reinterpret_cast<LPBYTE>(const_cast<wchar_t*>(text.data())), static_cast<LONG>(text.size() * sizeof(wchar_t)), &written);
		return static_cast<long>(written);
	}
	void CRichEditUI::SetAccumulateDBCMode(bool bDBCMode) { m_fAccumulateDBC = bDBCMode; }
	bool CRichEditUI::IsAccumulateDBCMode() { return m_fAccumulateDBC; }

	RECT CRichEditUI::GetTextPadding() const
	{
		RECT rc = m_rcTextPadding;
		if (m_pManager != NULL) m_pManager->ScaleRect(&rc);
		return rc;
	}
	void CRichEditUI::SetTextPadding(RECT rc) { m_rcTextPadding = rc; InvalidateLayout(); UpdateScrollBars(); }
	std::wstring_view CRichEditUI::GetNormalImage() { return m_sNormalImage; }
	void CRichEditUI::SetNormalImage(std::wstring_view pStrImage) { m_sNormalImage = pStrImage; Invalidate(); }
	std::wstring_view CRichEditUI::GetHotImage() { return m_sHotImage; }
	void CRichEditUI::SetHotImage(std::wstring_view pStrImage) { m_sHotImage = pStrImage; Invalidate(); }
	std::wstring_view CRichEditUI::GetFocusedImage() { return m_sFocusedImage; }
	void CRichEditUI::SetFocusedImage(std::wstring_view pStrImage) { m_sFocusedImage = pStrImage; Invalidate(); }
	std::wstring_view CRichEditUI::GetDisabledImage() { return m_sDisabledImage; }
	void CRichEditUI::SetDisabledImage(std::wstring_view pStrImage) { m_sDisabledImage = pStrImage; Invalidate(); }

	void CRichEditUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		if (IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~UISTATE_FOCUSED;
		if (!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~UISTATE_DISABLED;
		if ((m_uButtonState & UISTATE_DISABLED) != 0 && !m_sDisabledImage.empty() && DrawImage(renderContext, m_sDisabledImage)) return;
		if ((m_uButtonState & UISTATE_FOCUSED) != 0 && !m_sFocusedImage.empty() && DrawImage(renderContext, m_sFocusedImage)) return;
		if ((m_uButtonState & UISTATE_HOT) != 0 && !m_sHotImage.empty() && DrawImage(renderContext, m_sHotImage)) return;
		if (!m_sNormalImage.empty()) DrawImage(renderContext, m_sNormalImage);
	}

	void CRichEditUI::SetTipValue(std::wstring_view pStrTipValue) { m_sTipValue = pStrTipValue; Invalidate(); }
	std::wstring_view CRichEditUI::GetTipValue() { return m_sTipValue; }
	void CRichEditUI::SetTipValueColor(std::wstring_view pStrColor) { DWORD color = 0; if (StringUtil::TryParseColor(pStrColor, color)) m_dwTipValueColor = color; }
	DWORD CRichEditUI::GetTipValueColor() { return m_dwTipValueColor; }
	void CRichEditUI::SetTipValueAlign(UINT uAlign) { m_uTipValueAlign = uAlign; Invalidate(); }
	UINT CRichEditUI::GetTipValueAlign() { return m_uTipValueAlign; }
	void CRichEditUI::SetTipValuePadding(RECT rcTipValuePadding) { m_rcTipValuePadding = rcTipValuePadding; Invalidate(); }
	RECT CRichEditUI::GetTipValuePadding() { return m_rcTipValuePadding; }

	void CRichEditUI::DoInit()
	{
		if (m_bInited) return;
		CContainerUI::DoInit();
		if (m_pManager != NULL) {
			m_pManager->AddPreMessageFilter(this);
			m_pManager->AddMessageFilter(this);
			ApplyScrollBarStyle(m_pVerticalScrollBar, m_sVerticalScrollBarStyle, L"VScrollBar");
			ApplyScrollBarStyle(m_pHorizontalScrollBar, m_sHorizontalScrollBarStyle, L"HScrollBar");
		}
		m_bInited = true;
		UpdateScrollBars();
	}

	bool CRichEditUI::SetDropAcceptFile(bool bAccept) { SetDropEnable(bAccept); return bAccept; }
	HRESULT CRichEditUI::TxSendMessage(UINT, WPARAM, LPARAM, LRESULT* plresult) const { if (plresult) *plresult = 0; return E_NOTIMPL; }
	IDropTarget* CRichEditUI::GetTxDropTarget() { return NULL; }
	bool CRichEditUI::OnTxViewChanged() { Invalidate(); return true; }
	void CRichEditUI::OnTxNotify(DWORD, void*) {}
	CDuiSize CRichEditUI::GetNaturalSize(LONG width, LONG height)
	{
		return CDuiSize(width > 0 ? width : m_nContentWidth, height > 0 ? height : GetTextContentHeight(width));
	}
	int CRichEditUI::GetTextContentHeight(int nRichEditWidth)
	{
		if (nRichEditWidth > 0 && nRichEditWidth != m_nLayoutWidth) {
			m_nLayoutWidth = 0;
			m_bLayoutDirty = true;
		}
		EnsureLayout();
		return m_nContentHeight;
	}

	void CRichEditUI::SetScrollPos(SIZE szPos, bool, bool)
	{
		if (m_pVerticalScrollBar != NULL) m_pVerticalScrollBar->SetScrollPos(static_cast<int>((std::max<LONG>)(0L, szPos.cy)));
		if (m_pHorizontalScrollBar != NULL) {
			m_pHorizontalScrollBar->SetScrollPos(static_cast<int>((std::max<LONG>)(0L, szPos.cx)));
		}
		else {
			// 无水平滚动条（单行 / hscrollbar=false）场景仍需保持水平滚动以保证 caret 与文本可见。
			// range 额外预留 kCaretMargin 以保证文本末尾时 caret 仍能在视图内绘制。
			EnsureLayout();
			const RECT rcView = GetViewRect();
			const int viewW = (std::max<int>)(0, static_cast<int>(rcView.right - rcView.left));
			const int range = (std::max)(0, m_nContentWidth + kCaretMargin - viewW);
			const LONG cx = (std::max<LONG>)(0L, szPos.cx);
			m_nManualScrollX = static_cast<int>((std::min<LONG>)(cx, static_cast<LONG>(range)));
		}
		UpdateImeCompositionWindow();
		Invalidate();
	}

	SIZE CRichEditUI::GetScrollPos() const
	{
		SIZE sz = CContainerUI::GetScrollPos();
		if (m_pHorizontalScrollBar == NULL) sz.cx = m_nManualScrollX;
		return sz;
	}
	void CRichEditUI::LineUp(bool) { SIZE pos = GetScrollPos(); pos.cy -= GetLineHeight(); SetScrollPos(pos); }
	void CRichEditUI::LineDown(bool) { SIZE pos = GetScrollPos(); pos.cy += GetLineHeight(); SetScrollPos(pos); }
	void CRichEditUI::PageUp() { SIZE pos = GetScrollPos(); RECT rc = GetViewRect(); pos.cy -= rc.bottom - rc.top; SetScrollPos(pos); }
	void CRichEditUI::PageDown() { SIZE pos = GetScrollPos(); RECT rc = GetViewRect(); pos.cy += rc.bottom - rc.top; SetScrollPos(pos); }
	void CRichEditUI::HomeUp() { SIZE pos = GetScrollPos(); pos.cy = 0; SetScrollPos(pos); }
	void CRichEditUI::EndDown() { SIZE pos = GetScrollPos(); pos.cy = GetScrollRange().cy; SetScrollPos(pos); }
	void CRichEditUI::LineLeft() { SIZE pos = GetScrollPos(); pos.cx -= MeasureTextWidth(L"M"); SetScrollPos(pos); }
	void CRichEditUI::LineRight() { SIZE pos = GetScrollPos(); pos.cx += MeasureTextWidth(L"M"); SetScrollPos(pos); }
	void CRichEditUI::PageLeft() { SIZE pos = GetScrollPos(); RECT rc = GetViewRect(); pos.cx -= rc.right - rc.left; SetScrollPos(pos); }
	void CRichEditUI::PageRight() { SIZE pos = GetScrollPos(); RECT rc = GetViewRect(); pos.cx += rc.right - rc.left; SetScrollPos(pos); }
	void CRichEditUI::HomeLeft() { SIZE pos = GetScrollPos(); pos.cx = 0; SetScrollPos(pos); }
	void CRichEditUI::EndRight() { SIZE pos = GetScrollPos(); pos.cx = GetScrollRange().cx; SetScrollPos(pos); }

	SIZE CRichEditUI::EstimateSize(SIZE szAvailable)
	{
		SIZE sz = GetFixedSize();
		if (sz.cx == 0) sz.cx = szAvailable.cx;
		if (sz.cy == 0) sz.cy = static_cast<LONG>((std::max<int>)(GetLineHeight() + GetTextPadding().top + GetTextPadding().bottom, 24));
		return sz;
	}

	void CRichEditUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		const LONG oldWidth = m_rcItem.right - m_rcItem.left;
		CControlUI::SetPos(rc, bNeedInvalidate);
		const LONG newWidth = m_rcItem.right - m_rcItem.left;
		if (newWidth != oldWidth) {
			InvalidateLayout();
		}
		UpdateScrollBars();
	}

	void CRichEditUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		CContainerUI::Move(szOffset, bNeedInvalidate);
	}

	void CRichEditUI::DoEvent(TEventUI& event)
	{
		// 禁用态拒绝所有交互式输入：避免点击选中文字、键盘移动 caret、滚轮滚动、右键菜单等操作。
		// enter/leave/focus/timer 仍交由基类，以保持 hover 视觉、disabledimage 切换等被动行为正常。
		if (!IsEnabled()) {
			switch (event.Type) {
			case UIEVENT_BUTTONDOWN:
			case UIEVENT_BUTTONUP:
			case UIEVENT_DBLCLICK:
			case UIEVENT_RBUTTONDOWN:
			case UIEVENT_MOUSEMOVE:
			case UIEVENT_CONTEXTMENU:
			case UIEVENT_SCROLLWHEEL:
			case UIEVENT_KEYDOWN:
			case UIEVENT_KEYUP:
			case UIEVENT_CHAR:
				return;
			default:
				break;
			}
		}
		if (event.Type == UIEVENT_SETFOCUS) {
			CControlUI::DoEvent(event);
			ResetCaretBlink();
			UpdateImeCompositionWindow();
			return;
		}
		if (event.Type == UIEVENT_KILLFOCUS) {
			if (m_pManager) m_pManager->KillTimer(this, DEFAULT_TIMERID);
			CControlUI::DoEvent(event);
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER) {
			// 与 CButtonUI / CListContainerElementUI 等控件保持一致：在鼠标进入/离开时显式维护 UISTATE_HOT，
			// 否则 PaintStatusImage 中针对 hotimage 的判断永远不会成立，hover 时图片无法切换。
			if (IsEnabled()) {
				if ((m_uButtonState & UISTATE_HOT) == 0) {
					m_uButtonState |= UISTATE_HOT;
					Invalidate();
				}
			}
			CContainerUI::DoEvent(event);
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE) {
			if (IsEnabled()) {
				if ((m_uButtonState & UISTATE_HOT) != 0) {
					m_uButtonState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
			CContainerUI::DoEvent(event);
			return;
		}
		if (event.Type == UIEVENT_TIMER && event.wParam == DEFAULT_TIMERID) {
			m_bDrawCaret = !m_bDrawCaret;
			Invalidate();
			return;
		}
		if (event.Type == UIEVENT_BUTTONDOWN) {
			SetFocus();
			const size_t pos = HitTest(event.ptMouse);
			MoveCaret(pos, (event.wKeyState & MK_SHIFT) != 0);
			m_bDraggingSelection = true;
			if (m_pManager) m_pManager->SetCapture();
			return;
		}
		if (event.Type == UIEVENT_MOUSEMOVE && m_bDraggingSelection) {
			MoveCaret(HitTest(event.ptMouse), true);
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP && m_bDraggingSelection) {
			m_bDraggingSelection = false;
			if (m_pManager) m_pManager->ReleaseCapture();
			return;
		}
		if (event.Type == UIEVENT_CONTEXTMENU) {
			SetFocus();
			ShowContextMenu(event.ptMouse);
			return;
		}
		if (event.Type == UIEVENT_SCROLLWHEEL) {
			CContainerUI::DoEvent(event);
			Invalidate();
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN) {
			HandleKeyDown(event);
			return;
		}
		if (event.Type == UIEVENT_CHAR) {
			HandleChar(event);
			return;
		}
		CContainerUI::DoEvent(event);
	}

	bool CRichEditUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		RECT rcPaint = renderContext.GetPaintRect();
		RECT rcTemp = {};
		if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) return true;

		CRenderClip clip;
		CRenderClip::GenerateClip(renderContext, rcTemp, clip);
		CControlUI::DoPaint(renderContext, pStopControl);

		RECT rcView = GetViewRect();
		if (::IntersectRect(&rcTemp, &rcPaint, &rcView)) {
			CRenderClip textClip;
			CRenderClip::GenerateClip(renderContext, rcTemp, textClip);
			if (GetText().empty() && !IsFocused()) {
				DrawTipText(renderContext, rcView);
			}
			else {
				DrawSelection(renderContext, rcView);
				DrawTextLines(renderContext, rcView);
				DrawCaret(renderContext, rcView);
			}
		}

		// scrollfloat=true 时滚动条悬浮显示，仅在鼠标 hover / 正在拖拽时绘制；scrollfloat=false 时始终显示。与 CContainerUI::DoPaint 保持一致。
		// 额外约束：禁用态（!IsEnabled()）下，scrollfloat=true 的 hover 显示需求被抑制，避免禁用控件 hover 时还冒出滚动条；
		// scrollfloat=false 的占位滚动条仍按原逻辑绘制（IsScrollFloatShown 在 m_bScrollFloat=false 时直接返回 true）。
		const bool bScrollVisualAllowed = IsEnabled() || !m_bScrollFloat;
		if (m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() && m_pVerticalScrollBar != pStopControl && IsScrollFloatShown() && bScrollVisualAllowed) {
			m_pVerticalScrollBar->Paint(renderContext, pStopControl);
		}
		if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar != pStopControl && IsScrollFloatShown() && bScrollVisualAllowed) {
			m_pHorizontalScrollBar->Paint(renderContext, pStopControl);
		}
		return true;
	}

	void CRichEditUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		const std::wstring_view name = StringUtil::TrimView(pstrName);
		if (StringUtil::EqualsNoCase(name, L"vscrollbar")) {
			EnableScrollBar(StringUtil::ParseBool(pstrValue), m_pHorizontalScrollBar != NULL);
			ApplyScrollBarStyle(m_pVerticalScrollBar, m_sVerticalScrollBarStyle, L"VScrollBar");
			UpdateScrollBars();
		}
		else if (StringUtil::EqualsNoCase(name, L"hscrollbar")) {
			EnableScrollBar(m_pVerticalScrollBar != NULL, StringUtil::ParseBool(pstrValue));
			ApplyScrollBarStyle(m_pHorizontalScrollBar, m_sHorizontalScrollBarStyle, L"HScrollBar");
			UpdateScrollBars();
		}
		else if (StringUtil::EqualsNoCase(name, L"vscrollbarstyle")) {
			m_sVerticalScrollBarStyle.assign(pstrValue);
			EnableScrollBar(true, m_pHorizontalScrollBar != NULL);
			ApplyScrollBarStyle(m_pVerticalScrollBar, m_sVerticalScrollBarStyle, L"VScrollBar");
			UpdateScrollBars();
		}
		else if (StringUtil::EqualsNoCase(name, L"hscrollbarstyle")) {
			m_sHorizontalScrollBarStyle.assign(pstrValue);
			EnableScrollBar(m_pVerticalScrollBar != NULL, true);
			ApplyScrollBarStyle(m_pHorizontalScrollBar, m_sHorizontalScrollBarStyle, L"HScrollBar");
			UpdateScrollBars();
		}
		else if (StringUtil::EqualsNoCase(name, L"multiline")) SetMultiLine(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"wanttab")) SetWantTab(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"wantreturn")) SetWantReturn(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"wantctrlreturn")) SetWantCtrlReturn(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"transparent")) SetTransparent(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"rich")) SetRich(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"readonly")) SetReadOnly(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"password")) SetPasswordMode(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"wordwrap")) SetWordWrap(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::EqualsNoCase(name, L"font")) { int value = 0; if (StringUtil::TryParseInt(pstrValue, value)) SetFont(value); }
		else if (StringUtil::EqualsNoCase(name, L"textcolor")) { DWORD color = 0; if (StringUtil::TryParseColor(pstrValue, color)) SetTextColor(color); }
		else if (StringUtil::EqualsNoCase(name, L"maxchar")) { int value = 0; if (StringUtil::TryParseInt(pstrValue, value)) SetLimitText(value); }
		else if (StringUtil::EqualsNoCase(name, L"limittext")) SetLimitTextString(pstrValue);
		else if (StringUtil::EqualsNoCase(name, L"normalimage")) SetNormalImage(pstrValue);
		else if (StringUtil::EqualsNoCase(name, L"hotimage")) SetHotImage(pstrValue);
		else if (StringUtil::EqualsNoCase(name, L"focusedimage")) SetFocusedImage(pstrValue);
		else if (StringUtil::EqualsNoCase(name, L"disabledimage")) SetDisabledImage(pstrValue);
		else if (StringUtil::EqualsNoCase(name, L"textpadding")) { RECT rc = {}; if (StringUtil::TryParseRect(pstrValue, rc)) SetTextPadding(rc); }
		else if (StringUtil::EqualsNoCase(name, L"tipvalue")) SetTipValue(pstrValue);
		else if (StringUtil::EqualsNoCase(name, L"tipvaluecolor")) SetTipValueColor(pstrValue);
		else if (StringUtil::EqualsNoCase(name, L"tipvaluepadding")) { RECT rc = {}; if (StringUtil::TryParseRect(pstrValue, rc)) SetTipValuePadding(rc); }
		else if (StringUtil::EqualsNoCase(name, L"tipvaluealign")) {
			std::wstring lower(pstrValue);
			StringUtil::MakeLower(lower);
			UINT u = (m_uTipValueAlign & ~(DT_LEFT | DT_CENTER | DT_RIGHT)) | DT_SINGLELINE;
			if (lower.find(L"center") != std::wstring::npos) u |= DT_CENTER;
			else if (lower.find(L"right") != std::wstring::npos) u |= DT_RIGHT;
			else u |= DT_LEFT;
			m_uTipValueAlign = u;
		}
		else if (StringUtil::EqualsNoCase(name, L"tipvaluevalign")) {
			std::wstring lower(pstrValue);
			StringUtil::MakeLower(lower);
			UINT u = (m_uTipValueAlign & ~(DT_TOP | DT_VCENTER | DT_BOTTOM)) | DT_SINGLELINE;
			if (lower.find(L"vcenter") != std::wstring::npos || lower == L"center") u |= DT_VCENTER;
			else if (lower.find(L"bottom") != std::wstring::npos) u |= DT_BOTTOM;
			else u |= DT_TOP;
			m_uTipValueAlign = u;
		}
		else if (StringUtil::EqualsNoCase(name, L"align")) {
			std::wstring lower(pstrValue);
			StringUtil::MakeLower(lower);
			UINT uStyle = m_uTextStyle & ~(DT_LEFT | DT_CENTER | DT_RIGHT);
			if (lower.find(L"center") != std::wstring::npos) uStyle |= DT_CENTER;
			else if (lower.find(L"right") != std::wstring::npos) uStyle |= DT_RIGHT;
			else uStyle |= DT_LEFT;
			SetTextStyle(uStyle);
		}
		else if (StringUtil::EqualsNoCase(name, L"valign")) {
			std::wstring lower(pstrValue);
			StringUtil::MakeLower(lower);
			UINT uStyle = m_uTextStyle & ~(DT_TOP | DT_VCENTER | DT_BOTTOM);
			if (lower.find(L"vcenter") != std::wstring::npos || lower == L"center") uStyle |= DT_VCENTER;
			else if (lower.find(L"bottom") != std::wstring::npos) uStyle |= DT_BOTTOM;
			else uStyle |= DT_TOP;
			SetTextStyle(uStyle);
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	LRESULT CRichEditUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM, bool& bHandled)
	{
		bHandled = false;
		if (!IsFocused() || !IsVisible() || !IsEnabled()) return 0;
		if (uMsg == WM_KEYDOWN && wParam == VK_TAB && m_bWantTab && !m_bReadOnly) {
			ReplaceSel(L"\t", true);
			bHandled = true;
		}
		return 0;
	}

	bool CRichEditUI::PreprocessCharMessage(UINT&, WPARAM&) { return true; }
	void CRichEditUI::FinalizeMessageDispatch(UINT, HRESULT, bool bWasHandled, bool& bHandled) const { bHandled = bWasHandled; }

	CRichEditUI* CRichEditUI::Clone()
	{
		CRichEditUI* pClone = new CRichEditUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CRichEditUI::CopyData(CRichEditUI* pControl)
	{
		SetWinStyle(pControl->GetWinStyle());
		SetMultiLine(pControl->IsMultiLine());
		SetWantTab(pControl->IsWantTab());
		SetWantReturn(pControl->IsWantReturn());
		SetWantCtrlReturn(pControl->IsWantCtrlReturn());
		SetTransparent(pControl->IsTransparent());
		SetRich(pControl->IsRich());
		SetReadOnly(pControl->IsReadOnly());
		SetPasswordMode(pControl->IsPasswordMode());
		SetWordWrap(pControl->IsWordWrap());
		SetTextColor(pControl->GetTextColor());
		SetTextStyle(pControl->GetTextStyle());
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
		m_dwTipValueColor = pControl->m_dwTipValueColor;
		CContainerUI::CopyData(pControl);
		SetText(pControl->GetText());
	}

	void CRichEditUI::ResetCaretBlink()
	{
		m_bDrawCaret = true;
		if (m_pManager != NULL) {
			m_pManager->SetTimer(this, DEFAULT_TIMERID, ::GetCaretBlinkTime());
		}
		Invalidate();
	}

	void CRichEditUI::PushUndoState()
	{
		if (m_uUndoLimit == 0) return;
		m_undoStack.push_back(GetText());
		while (m_undoStack.size() > m_uUndoLimit) m_undoStack.erase(m_undoStack.begin());
		m_redoStack.clear();
	}

	void CRichEditUI::MarkTextChanged(bool)
	{
		m_bModified = true;
		InvalidateLayout();
		UpdateScrollBars();
		ScrollToCaret();
		Invalidate();
		if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_TEXTCHANGED);
	}

	void CRichEditUI::NormalizeText(std::wstring& text) const
	{
		StringUtil::ReplaceAll(text, L"\r\n", L"\n");
		StringUtil::ReplaceAll(text, L"\r", L"\n");
		if ((m_lTwhStyle & ES_MULTILINE) == 0) {
			std::replace(text.begin(), text.end(), L'\n', L' ');
		}
	}

	void CRichEditUI::ReplaceRange(size_t start, size_t end, std::wstring_view replacement, bool bCanUndo)
	{
		if (m_bReadOnly) return;
		std::wstring text = GetText();
		NormalizeRange(start, end);
		start = (std::min)(start, text.size());
		end = (std::min)(end, text.size());
		std::wstring insert(replacement);
		NormalizeText(insert);
		const size_t selected = end - start;
		if (m_iLimitText >= 0 && text.size() - selected + insert.size() > static_cast<size_t>(m_iLimitText)) {
			const size_t allowed = static_cast<size_t>(m_iLimitText) - (text.size() - selected);
			insert.resize(allowed);
		}
		if (bCanUndo) PushUndoState();
		text.replace(start, selected, insert);
		CControlUI::SetText(text);
		m_nCaret = m_nAnchor = start + insert.size();
		m_nPreferredCaretX = -1;
		MarkTextChanged(bCanUndo);
		UpdateImeCompositionWindow();
		ResetCaretBlink();
	}

	void CRichEditUI::DeleteSelection(bool bCanUndo)
	{
		if (m_nCaret == m_nAnchor) return;
		ReplaceRange(m_nAnchor, m_nCaret, std::wstring_view(), bCanUndo);
	}

	void CRichEditUI::MoveCaret(size_t pos, bool bKeepAnchor)
	{
		pos = (std::min)(pos, GetText().size());
		if (!bKeepAnchor) m_nAnchor = pos;
		m_nCaret = pos;
		m_nPreferredCaretX = -1;
		ScrollToCaret();
		UpdateImeCompositionWindow();
		ResetCaretBlink();
	}

	void CRichEditUI::MoveCaretVertical(int lineDelta, bool bKeepAnchor)
	{
		EnsureLayout();
		if (m_lines.empty()) return;
		const size_t oldLine = CurrentLineIndex();
		const CDuiPoint caretPos = CharPos(m_nCaret);
		if (m_nPreferredCaretX < 0) m_nPreferredCaretX = caretPos.x;
		int newLine = static_cast<int>(oldLine) + lineDelta;
		newLine = (std::max)(0, (std::min)(newLine, static_cast<int>(m_lines.size()) - 1));
		const TextLine& line = m_lines[static_cast<size_t>(newLine)];
		size_t best = line.start;
		int bestDelta = INT_MAX;
		for (size_t i = 0; i <= line.length; ++i) {
			const int x = CharPos(line.start + i).x;
			const int delta = std::abs(x - m_nPreferredCaretX);
			if (delta < bestDelta) {
				bestDelta = delta;
				best = line.start + i;
			}
		}
		if (!bKeepAnchor) m_nAnchor = best;
		m_nCaret = best;
		ScrollToCaret();
		UpdateImeCompositionWindow();
		ResetCaretBlink();
	}

	void CRichEditUI::EnsureLayout() const
	{
		const RECT rcView = GetViewRect();
		const int viewWidth = (std::max<int>)(1, static_cast<int>(rcView.right - rcView.left));
		if (!m_bLayoutDirty && m_nLayoutWidth == viewWidth) return;

		m_lines.clear();
		m_nLayoutWidth = viewWidth;
		m_nContentWidth = 0;
		const std::wstring text = GetDisplayText();
		const int maxWidth = m_bWordWrap ? viewWidth : INT_MAX / 4;

		auto pushEmptyLine = [&](size_t at) {
			TextLine line;
			line.start = at;
			line.length = 0;
			line.width = 0;
			line.charOffsets.assign(1, 0);
			m_lines.push_back(std::move(line));
		};

		size_t paragraphStart = 0;
		while (paragraphStart <= text.size()) {
			size_t paragraphEnd = text.find(L'\n', paragraphStart);
			const bool hasNewline = paragraphEnd != std::wstring::npos;
			if (!hasNewline) paragraphEnd = text.size();
			const size_t paraLen = paragraphEnd - paragraphStart;

			if (paraLen == 0) {
				pushEmptyLine(paragraphStart);
			}
			else {
				// One GDI call gives prefix widths for the whole paragraph.
				std::vector<int> prefix;
				CRenderEngine::GetTextPrefixWidths(m_pManager, m_iFont, text.data() + paragraphStart, paraLen, prefix);
				if (prefix.size() != paraLen) {
					// GDI failed (no manager / no font). Fall back to a zero-width line so we
					// don't loop forever; layout will retry once the manager is wired up.
					pushEmptyLine(paragraphStart);
				}
				else {
					size_t lineRel = 0;
					while (lineRel < paraLen) {
						const int baseWidth = (lineRel == 0) ? 0 : prefix[lineRel - 1];
						const size_t remaining = paraLen - lineRel;

						// Largest k in [1, remaining] s.t. prefix[lineRel+k-1]-baseWidth <= maxWidth.
						// Pure array indexing — no measurement.
						size_t k = 1;
						{
							size_t lo = 1;
							size_t hi = remaining;
							while (lo <= hi) {
								const size_t mid = (lo + hi) / 2;
								const int w = prefix[lineRel + mid - 1] - baseWidth;
								if (w <= maxWidth || mid == 1) {
									k = mid;
									lo = mid + 1;
								}
								else {
									if (mid == 0) break;
									hi = mid - 1;
								}
							}
						}

						// Word-wrap: backtrack to nearest preceding whitespace if mid-word.
						// Tab（\t）是宽度控制符，不是词边界（参考 CSS 的 tab-size / Word 的行为），
						// 不应作为回退断点；否则在一行末尾/中间插入 Tab 时会从 Tab 位置直接换行。
						if (m_bWordWrap && lineRel + k < paraLen) {
							for (size_t i = k; i > 1; --i) {
								const wchar_t wc = text[paragraphStart + lineRel + i - 1];
								if (iswspace(wc) && wc != L'\t') {
									k = i;
									break;
								}
							}
						}

						TextLine line;
						line.start = paragraphStart + lineRel;
						line.length = k;
						line.width = prefix[lineRel + k - 1] - baseWidth;
						line.charOffsets.assign(k + 1, 0);
						for (size_t i = 1; i <= k; ++i) {
							line.charOffsets[i] = prefix[lineRel + i - 1] - baseWidth;
						}
						m_nContentWidth = (std::max)(m_nContentWidth, line.width);
						m_lines.push_back(std::move(line));
						lineRel += k;
					}
				}
			}

			if (!hasNewline) break;
			paragraphStart = paragraphEnd + 1;
			if (paragraphStart == text.size()) {
				pushEmptyLine(paragraphStart);
				break;
			}
		}
		if (m_lines.empty()) pushEmptyLine(0);
		m_nContentHeight = static_cast<int>(m_lines.size()) * GetLineHeight();
		m_bLayoutDirty = false;
	}

	void CRichEditUI::InvalidateLayout() const { m_bLayoutDirty = true; }

	RECT CRichEditUI::GetEditRect() const
	{
		RECT rc = m_rcItem;
		RECT padding = GetTextPadding();
		rc.left += padding.left;
		rc.top += padding.top;
		rc.right -= padding.right;
		rc.bottom -= padding.bottom;
		return rc;
	}

	RECT CRichEditUI::GetViewRect() const
	{
		RECT rc = GetEditRect();
		if (!m_bScrollFloat && m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if (!m_bScrollFloat && m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible()) rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
		if (rc.right < rc.left) rc.right = rc.left;
		if (rc.bottom < rc.top) rc.bottom = rc.top;
		return rc;
	}

	int CRichEditUI::GetLineHeight() const
	{
		TFontInfo* font = m_pManager != NULL ? m_pManager->GetFontInfo(m_iFont) : NULL;
		return (std::max<int>)(font != NULL ? static_cast<int>(font->tm.tmHeight) : 16, 16);
	}

	std::wstring CRichEditUI::GetDisplayText() const
	{
		std::wstring text = GetText();
		if (!m_bPasswordMode) return text;
		for (wchar_t& ch : text) {
			if (ch != L'\n') ch = L'*';
		}
		return text;
	}

	int CRichEditUI::MeasureTextWidth(std::wstring_view text) const
	{
		if (text.empty() || m_pManager == NULL) return 0;
		RECT rc = { 0, 0, 100000, GetLineHeight() + 8 };
		CPaintRenderContext measureContext = m_pManager->CreateMeasureRenderContext(rc);
		std::wstring value(text);
		std::replace(value.begin(), value.end(), L'\t', L' ');
		CRenderEngine::DrawText(measureContext, rc, value, 0xFF000000, m_iFont, DT_CALCRECT | DT_SINGLELINE | DT_NOPREFIX);
		return rc.right - rc.left;
	}

	UINT CRichEditUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	void CRichEditUI::SetTextStyle(UINT uStyle)
	{
		if (m_uTextStyle == uStyle) return;
		m_uTextStyle = uStyle;
		Invalidate();
	}

	int CRichEditUI::GetLineXOffset(int lineWidth, int viewWidth) const
	{
		if (lineWidth >= viewWidth) return 0;
		if ((m_uTextStyle & DT_CENTER) != 0) return (viewWidth - lineWidth) / 2;
		if ((m_uTextStyle & DT_RIGHT) != 0) return viewWidth - lineWidth;
		return 0;
	}

	int CRichEditUI::GetTextYOffset(int viewHeight) const
	{
		if (m_nContentHeight >= viewHeight) return 0;
		if ((m_uTextStyle & DT_VCENTER) != 0) return (viewHeight - m_nContentHeight) / 2;
		if ((m_uTextStyle & DT_BOTTOM) != 0) return viewHeight - m_nContentHeight;
		return 0;
	}

	size_t CRichEditUI::HitTest(CDuiPoint pt) const
	{
		EnsureLayout();
		if (m_lines.empty()) return 0;
		const RECT rcView = GetViewRect();
		const SIZE scroll = GetScrollPos();
		const int lineHeight = GetLineHeight();
		const int viewW = (std::max<int>)(0, static_cast<int>(rcView.right - rcView.left));
		const int viewH = (std::max<int>)(0, static_cast<int>(rcView.bottom - rcView.top));
		const int yOff = GetTextYOffset(viewH);
		int lineIndex = (pt.y - rcView.top - yOff + scroll.cy) / (std::max)(1, lineHeight);
		lineIndex = (std::max)(0, (std::min)(lineIndex, static_cast<int>(m_lines.size()) - 1));
		const TextLine& line = m_lines[static_cast<size_t>(lineIndex)];
		const int xOff = GetLineXOffset(line.width, viewW);
		const int localX = pt.x - rcView.left - xOff + scroll.cx;

		if (line.length == 0 || localX <= 0) return line.start;
		if (localX >= line.width) return line.start + line.length;

		// O(log N) lookup in the cached charOffsets table populated by EnsureLayout.
		auto it = std::upper_bound(line.charOffsets.begin(), line.charOffsets.end(), localX);
		if (it == line.charOffsets.begin()) return line.start;
		const size_t k = static_cast<size_t>((it - line.charOffsets.begin()) - 1);
		if (k + 1 < line.charOffsets.size()) {
			const int dLo = std::abs(line.charOffsets[k] - localX);
			const int dHi = std::abs(line.charOffsets[k + 1] - localX);
			return line.start + ((dHi < dLo) ? (k + 1) : k);
		}
		return line.start + k;
	}

	CDuiPoint CRichEditUI::CharPos(size_t index) const
	{
		EnsureLayout();
		const RECT rcView = GetViewRect();
		const SIZE scroll = GetScrollPos();
		const int viewW = (std::max<int>)(0, static_cast<int>(rcView.right - rcView.left));
		const int viewH = (std::max<int>)(0, static_cast<int>(rcView.bottom - rcView.top));
		const int yOff = GetTextYOffset(viewH);
		for (size_t i = 0; i < m_lines.size(); ++i) {
			const TextLine& line = m_lines[i];
			if (index >= line.start && index <= line.start + line.length) {
				const size_t count = (std::min)(index - line.start, line.length);
				const int charOff = (count < line.charOffsets.size()) ? line.charOffsets[count] : 0;
				const int xOff = GetLineXOffset(line.width, viewW);
				const int x = rcView.left - scroll.cx + xOff + charOff;
				const int y = rcView.top - scroll.cy + yOff + static_cast<int>(i) * GetLineHeight();
				return CDuiPoint(x, y);
			}
		}
		return CDuiPoint(rcView.left + GetLineXOffset(0, viewW), rcView.top + yOff);
	}

	size_t CRichEditUI::CurrentLineIndex() const
	{
		EnsureLayout();
		for (size_t i = 0; i < m_lines.size(); ++i) {
			if (m_nCaret >= m_lines[i].start && m_nCaret <= m_lines[i].start + m_lines[i].length) return i;
		}
		return m_lines.empty() ? 0 : m_lines.size() - 1;
	}

	void CRichEditUI::UpdateScrollBars()
	{
		if (m_pManager == NULL) return;
		for (int pass = 0; pass < 3; ++pass) {
			EnsureLayout();
			const RECT rcView = GetViewRect();
			const int viewWidth = (std::max<int>)(0, static_cast<int>(rcView.right - rcView.left));
			const int viewHeight = (std::max<int>)(0, static_cast<int>(rcView.bottom - rcView.top));
			const bool oldVVisible = m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible();
			const bool oldHVisible = m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible();
			const bool needV = m_pVerticalScrollBar != NULL && m_bShowScrollbar && m_nContentHeight > viewHeight;
			const bool needH = m_pHorizontalScrollBar != NULL && m_bShowScrollbar && !m_bWordWrap && m_nContentWidth > viewWidth;
			if (m_pVerticalScrollBar != NULL) m_pVerticalScrollBar->SetVisible(needV);
			if (m_pHorizontalScrollBar != NULL) m_pHorizontalScrollBar->SetVisible(needH);
			if (oldVVisible == needV && oldHVisible == needH) break;
			InvalidateLayout();
		}

		EnsureLayout();
		RECT rcEdit = GetEditRect();
		RECT rcView = GetViewRect();
		const int viewWidth = (std::max<int>)(0, static_cast<int>(rcView.right - rcView.left));
		const int viewHeight = (std::max<int>)(0, static_cast<int>(rcView.bottom - rcView.top));
		if (m_pVerticalScrollBar != NULL) {
			const int barWidth = m_pVerticalScrollBar->GetFixedWidth();
			const int range = (std::max)(0, m_nContentHeight - viewHeight);
			m_pVerticalScrollBar->SetVisible(range > 0 && m_bShowScrollbar);
			m_pVerticalScrollBar->SetScrollRange(range);
			m_pVerticalScrollBar->SetLineSize(GetLineHeight());
			// Float 模式：滚动条悬浮在内容上方，并内缩 GetVSpace 像素与边界保持距离（GetVSpace 已含 DPI 缩放），与 CContainerUI 行为一致。
			const int nVSpace = m_bScrollFloat ? m_pVerticalScrollBar->GetVSpace() : 0;
			RECT rcBar = { rcEdit.right - barWidth - nVSpace, rcEdit.top, rcEdit.right - nVSpace, rcEdit.bottom };
			if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible()) rcBar.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
			m_pVerticalScrollBar->SetPos(rcBar, false);
		}
		if (m_pHorizontalScrollBar != NULL) {
			const int barHeight = m_pHorizontalScrollBar->GetFixedHeight();
			// “是否显示滚动条”仍以文本实际宽度为准；但允许滚动上限额外加上 kCaretMargin，以保证末尾 caret 可见。
			const int textRange = (std::max)(0, m_nContentWidth - viewWidth);
			const int scrollRange = (std::max)(0, m_nContentWidth + kCaretMargin - viewWidth);
			m_pHorizontalScrollBar->SetVisible(textRange > 0 && m_bShowScrollbar && !m_bWordWrap);
			m_pHorizontalScrollBar->SetScrollRange(scrollRange);
			m_pHorizontalScrollBar->SetLineSize(MeasureTextWidth(L"M"));
			// Float 模式：滚动条悬浮在内容上方，并内缩 GetHSpace 像素与边界保持距离（GetHSpace 已含 DPI 缩放），与 CContainerUI 行为一致。
			const int nHSpace = m_bScrollFloat ? m_pHorizontalScrollBar->GetHSpace() : 0;
			RECT rcBar = { rcEdit.left, rcEdit.bottom - barHeight - nHSpace, rcEdit.right, rcEdit.bottom - nHSpace };
			if (m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()) rcBar.right -= m_pVerticalScrollBar->GetFixedWidth();
			m_pHorizontalScrollBar->SetPos(rcBar, false);
		}
		else {
			// 无水平滚动条时同步收敛内部偏移，避免内容变短后仍残留过大偏移。
			const int range = (std::max)(0, m_nContentWidth + kCaretMargin - viewWidth);
			if (m_nManualScrollX > range) m_nManualScrollX = range;
			if (m_nManualScrollX < 0) m_nManualScrollX = 0;
		}
	}

	void CRichEditUI::ScrollToCaret()
	{
		UpdateScrollBars();
		const RECT rcView = GetViewRect();
		SIZE pos = GetScrollPos();
		const CDuiPoint caret = CharPos(m_nCaret);
		const int lineHeight = GetLineHeight();
		if (caret.y < rcView.top) pos.cy -= rcView.top - caret.y;
		else if (caret.y + lineHeight > rcView.bottom) pos.cy += caret.y + lineHeight - rcView.bottom;
		if (caret.x < rcView.left) pos.cx -= rcView.left - caret.x;
		else if (caret.x + 2 > rcView.right) pos.cx += caret.x + 2 - rcView.right;
		SetScrollPos(pos);
	}

	void CRichEditUI::ApplyScrollBarStyle(CScrollBarUI* pScrollBar, std::wstring_view styleName, std::wstring_view defaultName)
	{
		if (pScrollBar == NULL || m_pManager == NULL) return;
		bool styleApplied = false;
		if (!styleName.empty()) {
			const std::wstring_view style = m_pManager->GetStyle(styleName);
			pScrollBar->ApplyAttributeList(style.empty() ? styleName : style);
			styleApplied = true;
		}
		else {
			const std::wstring_view defaultStyle = m_pManager->GetDefaultAttributeList(defaultName);
			const std::wstring_view commonStyle = defaultStyle.empty() ? m_pManager->GetDefaultAttributeList(L"ScrollBar") : defaultStyle;
			if (!commonStyle.empty()) {
				pScrollBar->ApplyAttributeList(commonStyle);
				styleApplied = true;
			}
		}
		if (!styleApplied) {
			pScrollBar->ApplyDefaultStyle();
		}
		pScrollBar->SetShow(m_bShowScrollbar);
	}

	bool CRichEditUI::TryGetCachedCaretPoint(POINT& ptCaret) const
	{
		ptCaret = {};
		if (m_bLayoutDirty || m_lines.empty()) return false;

		RECT rcView = m_rcItem;
		rcView.left += m_rcTextPadding.left;
		rcView.top += m_rcTextPadding.top;
		rcView.right -= m_rcTextPadding.right;
		rcView.bottom -= m_rcTextPadding.bottom;
		if (!m_bScrollFloat && m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()) rcView.right -= m_pVerticalScrollBar->GetFixedWidth();
		if (!m_bScrollFloat && m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible()) rcView.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
		if (rcView.right < rcView.left) rcView.right = rcView.left;
		if (rcView.bottom < rcView.top) rcView.bottom = rcView.top;

		const SIZE scroll = GetScrollPos();
		const std::wstring text = GetDisplayText();
		const size_t caret = (std::min)(m_nCaret, text.size());
		const int lineHeight = GetLineHeight();
		const int viewW = (std::max<int>)(0, static_cast<int>(rcView.right - rcView.left));
		const int viewH = (std::max<int>)(0, static_cast<int>(rcView.bottom - rcView.top));
		const int yOff = GetTextYOffset(viewH);
		for (size_t i = 0; i < m_lines.size(); ++i) {
			const TextLine& line = m_lines[i];
			if (caret >= line.start && caret <= line.start + line.length) {
				const size_t count = (std::min)(caret - line.start, line.length);
				const int charOff = (count < line.charOffsets.size()) ? line.charOffsets[count] : 0;
				const int xOff = GetLineXOffset(line.width, viewW);
				ptCaret.x = rcView.left - scroll.cx + xOff + charOff;
				ptCaret.y = rcView.top - scroll.cy + yOff + static_cast<LONG>(i) * lineHeight;
				return true;
			}
		}

		ptCaret.x = rcView.left + GetLineXOffset(0, viewW);
		ptCaret.y = rcView.top + yOff;
		return true;
	}

	void CRichEditUI::UpdateImeCompositionWindow()
	{
		if (m_pManager == NULL || !IsFocused() || !IsVisible() || !IsEnabled()) return;
		HWND hWnd = m_pManager->GetPaintWindow();
		if (hWnd == NULL) return;
		HIMC hImc = ::ImmGetContext(hWnd);
		if (hImc == NULL) return;

		POINT pt = {};
		if (!TryGetCachedCaretPoint(pt)) {
			::ImmReleaseContext(hWnd, hImc);
			return;
		}
		const int lineHeight = GetLineHeight();

		COMPOSITIONFORM composition = {};
		composition.dwStyle = CFS_FORCE_POSITION;
		composition.ptCurrentPos = pt;
		::ImmSetCompositionWindow(hImc, &composition);

		CANDIDATEFORM candidate = {};
		candidate.dwIndex = 0;
		candidate.dwStyle = CFS_CANDIDATEPOS;
		candidate.ptCurrentPos = { pt.x, pt.y + lineHeight };
		::ImmSetCandidateWindow(hImc, &candidate);

		HFONT hFont = m_pManager->GetFont(m_iFont);
		LOGFONT lf = {};
		if (hFont != NULL && ::GetObject(hFont, sizeof(LOGFONT), &lf) == sizeof(LOGFONT)) {
			::ImmSetCompositionFont(hImc, &lf);
		}
		::ImmReleaseContext(hWnd, hImc);
	}

	void CRichEditUI::DrawSelection(CPaintRenderContext& renderContext, const RECT& rcView)
	{
		if (m_nCaret == m_nAnchor || (m_bHideSelection && !IsFocused())) return;
		EnsureLayout();
		size_t selStart = m_nAnchor;
		size_t selEnd = m_nCaret;
		NormalizeRange(selStart, selEnd);
		const std::wstring text = GetText();
		const std::wstring displayText = GetDisplayText();
		const SIZE scroll = GetScrollPos();
		const int lineHeight = GetLineHeight();
		const int blankWidth = (std::max)(MeasureTextWidth(L" "), 4);
		const int viewW = (std::max<int>)(0, static_cast<int>(rcView.right - rcView.left));
		const int viewH = (std::max<int>)(0, static_cast<int>(rcView.bottom - rcView.top));
		const int yOff = GetTextYOffset(viewH);
		for (size_t i = 0; i < m_lines.size(); ++i) {
			const TextLine& line = m_lines[i];
			const size_t lineStart = line.start;
			const size_t lineEnd = line.start + line.length;
			const bool hasNewline = lineEnd < text.size() && text[lineEnd] == L'\n';
			const size_t logicalEnd = hasNewline ? lineEnd + 1 : lineEnd;
			if (selEnd <= lineStart || selStart >= logicalEnd) continue;

			const int xOff = GetLineXOffset(line.width, viewW);
			const LONG lineLeft = rcView.left - scroll.cx + xOff;
			const LONG lineTop = rcView.top - scroll.cy + yOff + static_cast<LONG>(i) * lineHeight;
			LONG left = lineLeft;
			LONG right = lineLeft;
			if (selStart > lineStart && selStart <= lineEnd) {
				const size_t cnt = selStart - lineStart;
				if (cnt < line.charOffsets.size()) left += line.charOffsets[cnt];
			}
			if (selEnd <= lineEnd) {
				const size_t cnt = selEnd - lineStart;
				if (cnt < line.charOffsets.size()) right += line.charOffsets[cnt];
			}
			else {
				right += line.width;
				if (hasNewline && selEnd >= logicalEnd) right += blankWidth;
			}
			if (line.length == 0) {
				right = left + blankWidth;
			}
			if (right <= left) right = left + blankWidth;

			RECT rcSel = { left, lineTop, right, lineTop + lineHeight };
			RECT rcClipped = {};
			if (::IntersectRect(&rcClipped, &rcSel, &rcView)) {
				CRenderEngine::DrawRoundColor(renderContext, rcClipped, 0, 0, kSelectionColor);
			}
		}
	}

	void CRichEditUI::DrawTextLines(CPaintRenderContext& renderContext, const RECT& rcView)
	{
		EnsureLayout();
		const SIZE scroll = GetScrollPos();
		const int lineHeight = GetLineHeight();
		const DWORD color = IsEnabled()
			? (m_dwTextColor != 0 ? m_dwTextColor : (m_pManager ? m_pManager->GetDefaultFontColor() : 0xFF000000))
			: (m_pManager ? m_pManager->GetDefaultDisabledColor() : 0xFF999999);
		const std::wstring text = GetDisplayText();
		const int viewW = (std::max<int>)(0, static_cast<int>(rcView.right - rcView.left));
		const int viewH = (std::max<int>)(0, static_cast<int>(rcView.bottom - rcView.top));
		const int yOff = GetTextYOffset(viewH);
		for (size_t i = 0; i < m_lines.size(); ++i) {
			const TextLine& line = m_lines[i];
			const int xOff = GetLineXOffset(line.width, viewW);
			RECT rcLine = {
				rcView.left - scroll.cx + xOff,
				rcView.top - scroll.cy + yOff + static_cast<LONG>(i) * lineHeight,
				rcView.right,
				rcView.top - scroll.cy + yOff + static_cast<LONG>(i + 1) * lineHeight
			};
			if (rcLine.bottom < rcView.top || rcLine.top > rcView.bottom) continue;
			RECT rcDraw = rcLine;
			// 绘制端把 \t 替换为空格，与 GetTextPrefixWidths / MeasureTextWidth 的度量保持一致；
			// 否则 GDI 在 DT_SINGLELINE 下对 \t 绘制宽度 ≈ 0，会与 charOffsets 记录的 Tab=1空格宽度错位，
			// 造成 Tab 后的字符视觉位置与 caret / HitTest 计算不符。字符数不变，charOffsets 无需调整。
			std::wstring drawText(text.data() + line.start, line.length);
			std::replace(drawText.begin(), drawText.end(), L'\t', L' ');
			CRenderEngine::DrawText(renderContext, rcDraw, drawText, color, m_iFont, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
		}
	}

	void CRichEditUI::DrawCaret(CPaintRenderContext& renderContext, const RECT& rcView)
	{
		if (!IsFocused() || !m_bDrawCaret || !IsEnabled() || m_bReadOnly) return;
		const CDuiPoint pt = CharPos(m_nCaret);
		RECT rcCaret = { pt.x, pt.y, pt.x + 1, pt.y + GetLineHeight() };
		// IntersectRect 遵循“右下不含”语义，需额外检查裁剪后矩形是否仍有面积，避免 caret 贴边被当作不相交从而不绘制。
		RECT rcDraw = {};
		if (::IntersectRect(&rcDraw, &rcCaret, &rcView) && rcDraw.right > rcDraw.left && rcDraw.bottom > rcDraw.top) {
			CRenderEngine::DrawRoundColor(renderContext, rcDraw, 0, 0, kCaretColor);
		}
	}

	void CRichEditUI::DrawTipText(CPaintRenderContext& renderContext, const RECT& rcView)
	{
		if (m_sTipValue.empty()) return;
		RECT rc = rcView;
		RECT padding = m_rcTipValuePadding;
		if (m_pManager != NULL) m_pManager->ScaleRect(&padding);
		rc.left += padding.left;
		rc.right -= padding.right;
		rc.top += padding.top;
		rc.bottom -= padding.bottom;
		CRenderEngine::DrawText(renderContext, rc, m_sTipValue, m_dwTipValueColor, m_iFont, m_uTipValueAlign);
	}

	void CRichEditUI::ShowContextMenu(CDuiPoint pt)
	{
		if (m_pManager == NULL || !IsEnabled()) return;
		HWND hWnd = m_pManager->GetPaintWindow();
		if (hWnd == NULL) return;

		if (pt.x < m_rcItem.left || pt.x > m_rcItem.right || pt.y < m_rcItem.top || pt.y > m_rcItem.bottom) {
			const CDuiPoint caret = CharPos(m_nCaret);
			pt.x = caret.x;
			pt.y = caret.y + GetLineHeight();
		}

		HMENU hMenu = ::CreatePopupMenu();
		if (hMenu == NULL) return;

		::AppendMenuW(hMenu, MF_STRING, kRichEditMenuUndo, L"撤销\tCtrl+Z");
		::AppendMenuW(hMenu, MF_STRING, kRichEditMenuRedo, L"恢复\tCtrl+Y");
		::AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
		::AppendMenuW(hMenu, MF_STRING, kRichEditMenuCut, L"剪切\tCtrl+X");
		::AppendMenuW(hMenu, MF_STRING, kRichEditMenuCopy, L"复制\tCtrl+C");
		::AppendMenuW(hMenu, MF_STRING, kRichEditMenuPaste, L"粘贴\tCtrl+V");
		::AppendMenuW(hMenu, MF_STRING, kRichEditMenuDelete, L"删除");
		::AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
		::AppendMenuW(hMenu, MF_STRING, kRichEditMenuSelectAll, L"全选\tCtrl+A");

		const UINT hasSelection = GetSelectionType() != SEL_EMPTY ? MF_ENABLED : MF_GRAYED;
		const UINT canEdit = !m_bReadOnly ? MF_ENABLED : MF_GRAYED;
		::EnableMenuItem(hMenu, kRichEditMenuUndo, MF_BYCOMMAND | (CanUndo() ? MF_ENABLED : MF_GRAYED));
		::EnableMenuItem(hMenu, kRichEditMenuRedo, MF_BYCOMMAND | (CanRedo() ? MF_ENABLED : MF_GRAYED));
		::EnableMenuItem(hMenu, kRichEditMenuCut, MF_BYCOMMAND | hasSelection | canEdit);
		::EnableMenuItem(hMenu, kRichEditMenuCopy, MF_BYCOMMAND | hasSelection);
		::EnableMenuItem(hMenu, kRichEditMenuPaste, MF_BYCOMMAND | (CanPaste() ? MF_ENABLED : MF_GRAYED));
		::EnableMenuItem(hMenu, kRichEditMenuDelete, MF_BYCOMMAND | hasSelection | canEdit);
		::EnableMenuItem(hMenu, kRichEditMenuSelectAll, MF_BYCOMMAND | (GetText().empty() ? MF_GRAYED : MF_ENABLED));

		POINT screenPt = { pt.x, pt.y };
		::ClientToScreen(hWnd, &screenPt);
		::SetForegroundWindow(hWnd);
		const UINT cmd = ::TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, screenPt.x, screenPt.y, 0, hWnd, NULL);
		::DestroyMenu(hMenu);

		switch (cmd) {
		case kRichEditMenuUndo: Undo(); break;
		case kRichEditMenuRedo: Redo(); break;
		case kRichEditMenuCut: Cut(); break;
		case kRichEditMenuCopy: Copy(); break;
		case kRichEditMenuPaste: Paste(); break;
		case kRichEditMenuDelete: Clear(); break;
		case kRichEditMenuSelectAll: SetSelAll(); break;
		default: break;
		}
	}

	std::wstring CRichEditUI::GetClipboardText() const
	{
		std::wstring text;
		if (::OpenClipboard(m_pManager ? m_pManager->GetPaintWindow() : NULL)) {
			HANDLE data = ::GetClipboardData(CF_UNICODETEXT);
			if (data != NULL) {
				LPCWSTR value = static_cast<LPCWSTR>(::GlobalLock(data));
				if (value != NULL) {
					text = value;
					::GlobalUnlock(data);
				}
			}
			::CloseClipboard();
		}
		return text;
	}

	void CRichEditUI::SetClipboardText(std::wstring_view text) const
	{
		if (text.empty() || !::OpenClipboard(m_pManager ? m_pManager->GetPaintWindow() : NULL)) return;
		::EmptyClipboard();
		const SIZE_T bytes = (text.size() + 1) * sizeof(wchar_t);
		HGLOBAL data = ::GlobalAlloc(GMEM_MOVEABLE, bytes);
		if (data != NULL) {
			void* buffer = ::GlobalLock(data);
			if (buffer != NULL) {
				memcpy(buffer, text.data(), text.size() * sizeof(wchar_t));
				static_cast<wchar_t*>(buffer)[text.size()] = L'\0';
				::GlobalUnlock(data);
				::SetClipboardData(CF_UNICODETEXT, data);
			}
			else {
				::GlobalFree(data);
			}
		}
		::CloseClipboard();
	}

	void CRichEditUI::HandleKeyDown(TEventUI& event)
	{
		const bool ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		const bool shift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
		const std::wstring text = GetText();
		if (ctrl) {
			switch (event.chKey) {
			case L'A': SetSelAll(); return;
			case L'C': Copy(); return;
			case L'X': Cut(); return;
			case L'V': Paste(); return;
			case L'Z': Undo(); return;
			case L'Y': Redo(); return;
			default: break;
			}
		}
		switch (event.chKey) {
		case VK_LEFT: MoveCaret(m_nCaret > 0 ? m_nCaret - 1 : 0, shift); break;
		case VK_RIGHT: MoveCaret((std::min)(m_nCaret + 1, text.size()), shift); break;
		case VK_HOME: {
			EnsureLayout();
			const TextLine& line = m_lines[CurrentLineIndex()];
			MoveCaret(line.start, shift);
			break;
		}
		case VK_END: {
			EnsureLayout();
			const TextLine& line = m_lines[CurrentLineIndex()];
			MoveCaret(line.start + line.length, shift);
			break;
		}
		case VK_UP: MoveCaretVertical(-1, shift); break;
		case VK_DOWN: MoveCaretVertical(1, shift); break;
		case VK_PRIOR: PageUp(); MoveCaretVertical(-((std::max<int>)(1, static_cast<int>(GetViewRect().bottom - GetViewRect().top) / GetLineHeight())), shift); break;
		case VK_NEXT: PageDown(); MoveCaretVertical((std::max<int>)(1, static_cast<int>(GetViewRect().bottom - GetViewRect().top) / GetLineHeight()), shift); break;
		case VK_BACK:
			if (!m_bReadOnly) {
				if (m_nCaret != m_nAnchor) DeleteSelection(true);
				else if (m_nCaret > 0) ReplaceRange(m_nCaret - 1, m_nCaret, {}, true);
			}
			break;
		case VK_DELETE:
			if (!m_bReadOnly) {
				if (m_nCaret != m_nAnchor) DeleteSelection(true);
				else if (m_nCaret < text.size()) ReplaceRange(m_nCaret, m_nCaret + 1, {}, true);
			}
			break;
		default:
			break;
		}
	}

	void CRichEditUI::HandleChar(TEventUI& event)
	{
		if (m_bReadOnly) return;
		const wchar_t ch = event.chKey;
		if (ch == L'\r' || ch == L'\n') {
			const bool ctrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
			if (IsMultiLine() && (m_bWantReturn || (ctrl && m_bWantCtrlReturn))) ReplaceSel(L"\n", true);
			return;
		}
		if (ch == L'\t') {
			if (m_bWantTab) ReplaceSel(L"\t", true);
			return;
		}
		if (IsPrintableChar(ch)) {
			const wchar_t value[2] = { ch, 0 };
			ReplaceSel(value, true);
		}
	}
}
