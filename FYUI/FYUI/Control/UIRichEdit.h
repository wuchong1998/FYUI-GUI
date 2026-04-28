#pragma once

#include <Richedit.h>

namespace FYUI
{
	class FYUI_API CRichEditUI : public CContainerUI, public IMessageFilterUI
	{
		DECLARE_DUICONTROL(CRichEditUI)

	public:
		CRichEditUI();
		~CRichEditUI();

		std::wstring_view GetClass() const override;
		LPVOID GetInterface(std::wstring_view pstrName) override;
		UINT GetControlFlags() const override;

		void SetEnabled(bool bEnabled) override;
		bool IsMultiLine();
		void SetMultiLine(bool bMultiLine);
		bool IsWantTab();
		void SetWantTab(bool bWantTab = true);
		bool IsWantReturn();
		void SetWantReturn(bool bWantReturn = true);
		bool IsWantCtrlReturn();
		void SetWantCtrlReturn(bool bWantCtrlReturn = true);
		bool IsTransparent();
		void SetTransparent(bool bTransparent = true);
		bool IsRich();
		void SetRich(bool bRich = true);
		bool IsReadOnly();
		void SetReadOnly(bool bReadOnly = true);
		bool IsWordWrap();
		void SetWordWrap(bool bWordWrap = true);
		int GetFont();
		void SetFont(int index);
		void SetFont(std::wstring_view pStrFontName, int nSize, bool bBold, bool bUnderline, bool bItalic);
		LONG GetWinStyle();
		void SetWinStyle(LONG lStyle);
		DWORD GetTextColor();
		void SetTextColor(DWORD dwTextColor);
		int GetLimitText();
		void SetLimitText(int iChars);

		void SetLimitTextString(std::wstring_view pStrLimitText);
		std::wstring GetLimitTextString() const;
		long GetTextLength(DWORD dwFlags = GTL_DEFAULT) const;
		std::wstring GetText() const override;
		void SetText(std::wstring_view pstrText) override;
		void SetShowText(std::wstring_view pstrText);
		bool IsModify() const;
		void SetModify(bool bModified = true) const;
		void GetSel(CHARRANGE& cr) const;
		void GetSel(long& nStartChar, long& nEndChar) const;
		int SetSel(CHARRANGE& cr);
		int SetSel(long nStartChar, long nEndChar);
		void ReplaceSel(std::wstring_view lpszNewText, bool bCanUndo);
		void ReplaceShowSel(std::wstring_view lpszNewText, bool bCanUndo);
		void ReplaceSelW(LPCWSTR lpszNewText, bool bCanUndo = false);
		std::wstring GetSelText() const;
		int SetSelAll();
		int SetSelNone();
		WORD GetSelectionType() const;
		bool GetZoom(int& nNum, int& nDen) const;
		bool SetZoom(int nNum, int nDen);
		bool SetZoomOff();
		bool GetAutoURLDetect() const;
		bool SetAutoURLDetect(bool bAutoDetect = true);
		DWORD GetEventMask() const;
		DWORD SetEventMask(DWORD dwEventMask);
		std::wstring GetTextRange(long nStartChar, long nEndChar) const;
		void HideSelection(bool bHide = true, bool bChangeStyle = false);
		void ScrollCaret();
		int InsertText(long nInsertAfterChar, std::wstring_view lpstrText, bool bCanUndo = false);
		int AppendText(std::wstring_view lpstrText, bool bCanUndo = false);
		DWORD GetDefaultCharFormat(CHARFORMAT2& cf) const;
		bool SetDefaultCharFormat(CHARFORMAT2& cf);
		DWORD GetSelectionCharFormat(CHARFORMAT2& cf) const;
		bool SetSelectionCharFormat(CHARFORMAT2& cf);
		bool SetWordCharFormat(CHARFORMAT2& cf);
		DWORD GetParaFormat(PARAFORMAT2& pf) const;
		bool SetParaFormat(PARAFORMAT2& pf);
		bool CanUndo();
		bool CanRedo();
		bool CanPaste();
		bool Redo();
		bool Undo();
		void Clear();
		void Copy();
		void Cut();
		void Paste();
		int GetLineCount() const;
		std::wstring GetLine(int nIndex, int nMaxLength) const;
		int LineIndex(int nLine = -1) const;
		int LineLength(int nLine = -1) const;
		bool LineScroll(int nLines, int nChars = 0);
		CDuiPoint GetCharPos(long lChar) const;
		long LineFromChar(long nIndex) const;
		CDuiPoint PosFromChar(UINT nChar) const;
		int CharFromPos(CDuiPoint pt) const;
		void EmptyUndoBuffer();
		UINT SetUndoLimit(UINT nLimit);
		long StreamIn(int nFormat, EDITSTREAM& es);
		long StreamOut(int nFormat, EDITSTREAM& es);
		void SetAccumulateDBCMode(bool bDBCMode);
		bool IsAccumulateDBCMode();

		RECT GetTextPadding() const;
		void SetTextPadding(RECT rc);
		std::wstring_view GetNormalImage();
		void SetNormalImage(std::wstring_view pStrImage);
		std::wstring_view GetHotImage();
		void SetHotImage(std::wstring_view pStrImage);
		std::wstring_view GetFocusedImage();
		void SetFocusedImage(std::wstring_view pStrImage);
		std::wstring_view GetDisabledImage();
		void SetDisabledImage(std::wstring_view pStrImage);
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

		void SetTipValue(std::wstring_view pStrTipValue);
		std::wstring_view GetTipValue();
		void SetTipValueColor(std::wstring_view pStrColor);
		DWORD GetTipValueColor();
		void SetTipValueAlign(UINT uAlign);
		UINT GetTipValueAlign();
		void SetTipValuePadding(RECT rcTipValuePadding);
		RECT GetTipValuePadding();

		void DoInit() override;
		bool SetDropAcceptFile(bool bAccept);
		HRESULT TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT* plresult) const;
		IDropTarget* GetTxDropTarget();
		bool OnTxViewChanged();
		void OnTxNotify(DWORD iNotify, void* pv);

		CDuiSize GetNaturalSize(LONG width, LONG height);
		int GetTextContentHeight(int nRichEditWidth = 0);
		void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true) override;
		void LineUp(bool bScroolVisible = true) override;
		void LineDown(bool bScroolVisible = true) override;
		void PageUp() override;
		void PageDown() override;
		void HomeUp() override;
		void EndDown() override;
		void LineLeft() override;
		void LineRight() override;
		void PageLeft() override;
		void PageRight() override;
		void HomeLeft() override;
		void EndRight() override;

		SIZE EstimateSize(SIZE szAvailable) override;
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void Move(SIZE szOffset, bool bNeedInvalidate = true) override;
		void DoEvent(TEventUI& event) override;
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue) override;

		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;
		bool PreprocessCharMessage(UINT& uMsg, WPARAM& wParam);
		void FinalizeMessageDispatch(UINT uMsg, HRESULT hr, bool bWasHandled, bool& bHandled) const;

		CRichEditUI* Clone() override;
		void CopyData(CRichEditUI* pControl);

	protected:
		enum { DEFAULT_TIMERID = 20 };

	private:
		struct TextLine
		{
			size_t start = 0;
			size_t length = 0;
			int width = 0;
		};

		void ResetCaretBlink();
		void PushUndoState();
		void MarkTextChanged(bool bCanUndo);
		void NormalizeText(std::wstring& text) const;
		void ReplaceRange(size_t start, size_t end, std::wstring_view replacement, bool bCanUndo);
		void DeleteSelection(bool bCanUndo);
		void MoveCaret(size_t pos, bool bKeepAnchor);
		void MoveCaretVertical(int lineDelta, bool bKeepAnchor);
		void EnsureLayout() const;
		void InvalidateLayout() const;
		RECT GetEditRect() const;
		RECT GetViewRect() const;
		int GetLineHeight() const;
		int MeasureTextWidth(std::wstring_view text) const;
		size_t HitTest(CDuiPoint pt) const;
		CDuiPoint CharPos(size_t index) const;
		size_t CurrentLineIndex() const;
		void UpdateScrollBars();
		void ApplyScrollBarStyle(CScrollBarUI* pScrollBar, std::wstring_view styleName, std::wstring_view defaultName);
		void ScrollToCaret();
		void UpdateImeCompositionWindow();
		void DrawSelection(CPaintRenderContext& renderContext, const RECT& rcView);
		void DrawTextLines(CPaintRenderContext& renderContext, const RECT& rcView);
		void DrawCaret(CPaintRenderContext& renderContext, const RECT& rcView);
		void DrawTipText(CPaintRenderContext& renderContext, const RECT& rcView);
		std::wstring GetClipboardText() const;
		void SetClipboardText(std::wstring_view text) const;
		void HandleKeyDown(TEventUI& event);
		void HandleChar(TEventUI& event);

	protected:
		std::wstring m_strLimitText;
		bool m_bWantTab;
		bool m_bWantReturn;
		bool m_bWantCtrlReturn;
		bool m_bTransparent;
		bool m_bRich;
		bool m_bReadOnly;
		bool m_bWordWrap;
		bool m_bHideSelection;
		bool m_bAutoURLDetect;
		bool m_bModified;
		bool m_bDrawCaret;
		bool m_bInited;
		bool m_bDraggingSelection;
		bool m_fAccumulateDBC;
		UINT m_chLeadByte;
		DWORD m_dwTextColor;
		DWORD m_dwEventMask;
		int m_iFont;
		int m_iLimitText;
		LONG m_lTwhStyle;
		UINT m_uUndoLimit;
		int m_nZoomNum;
		int m_nZoomDen;
		size_t m_nCaret;
		size_t m_nAnchor;
		int m_nPreferredCaretX;
		RECT m_rcTextPadding;
		UINT m_uButtonState;
		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sDisabledImage;
		std::wstring m_sTipValue;
		DWORD m_dwTipValueColor;
		UINT m_uTipValueAlign;
		RECT m_rcTipValuePadding;
		CHARFORMAT2W m_defaultCharFormat;
		CHARFORMAT2W m_selectionCharFormat;
		PARAFORMAT2 m_paraFormat;
		std::vector<std::wstring> m_undoStack;
		std::vector<std::wstring> m_redoStack;
		mutable bool m_bLayoutDirty;
		mutable int m_nLayoutWidth;
		mutable int m_nContentWidth;
		mutable int m_nContentHeight;
		mutable std::vector<TextLine> m_lines;
	};
}
