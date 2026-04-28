#pragma once

#include <Richedit.h>
namespace FYUI 
{

	class CTxtWinHost;

	class FYUI_API CRichEditUI : public CContainerUI, public IMessageFilterUI
	{
		DECLARE_DUICONTROL(CRichEditUI)
	public:
		CRichEditUI();
		~CRichEditUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetControlFlags() const;

		void SetEnabled(bool bEnabled);
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
		std::wstring GetText() const;
		void SetText(std::wstring_view pstrText);
		void SetShowText(std::wstring_view pstrText);
		bool IsModify() const;
		void SetModify(bool bModified = true) const;
		void GetSel(CHARRANGE &cr) const;
		void GetSel(long& nStartChar, long& nEndChar) const;
		int SetSel(CHARRANGE &cr);
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
		DWORD GetDefaultCharFormat(CHARFORMAT2 &cf) const;
		bool SetDefaultCharFormat(CHARFORMAT2 &cf);
		DWORD GetSelectionCharFormat(CHARFORMAT2 &cf) const;
		bool SetSelectionCharFormat(CHARFORMAT2 &cf);
		bool SetWordCharFormat(CHARFORMAT2 &cf);
		DWORD GetParaFormat(PARAFORMAT2 &pf) const;
		bool SetParaFormat(PARAFORMAT2 &pf);
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
		long StreamIn(int nFormat, EDITSTREAM &es);
		long StreamOut(int nFormat, EDITSTREAM &es);
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
		void PaintStatusImage(HDC hDC);

		void SetTipValue(std::wstring_view pStrTipValue);
		std::wstring_view GetTipValue();
		void SetTipValueColor(std::wstring_view pStrColor);
		DWORD GetTipValueColor();
		void SetTipValueAlign(UINT uAlign);
		UINT GetTipValueAlign();

		void SetTipValuePadding(RECT rcTipValuePadding);
		RECT GetTipValuePadding();

		void DoInit();
		bool SetDropAcceptFile(bool bAccept);
		// 娉ㄦ剰锛歍xSendMessage鍜孲endMessage鏄湁鍖哄埆鐨勶紝TxSendMessage娌℃湁multibyte鍜寀nicode鑷姩杞崲鐨勫姛鑳斤紝
		// 鑰宺ichedit2.0鍐呴儴鏄互unicode瀹炵幇鐨勶紝鍦╩ultibyte绋嬪簭涓紝蹇呴』鑷繁澶勭悊unicode鍒癿ultibyte鐨勮浆鎹?
		virtual HRESULT TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult) const; 
		IDropTarget* GetTxDropTarget();
		virtual bool OnTxViewChanged();
		virtual void OnTxNotify(DWORD iNotify, void *pv);

		CDuiSize GetNaturalSize(LONG width, LONG height);
		/**
		* @brief 鑾峰彇 RichEdit 涓墍鏈夋枃瀛楀湪褰撳墠鎺т欢瀹藉害绾︽潫涓嬬殑鎬婚珮搴?
		* @return int 鏂囧瓧鍐呭鐨勬€婚珮搴︼紙鐗╃悊鍍忕礌鍊硷級
		* @param  nRichEditWidth 榛樿涓?鑷姩鑾峰彇姝ゆ帶浠堕珮搴︽潵鑾峰彇鏂囨湰鍐呭楂樺害锛屽鏋滀紶鍏ュぇ浜?鐨勫€硷紝鍒欎娇鐢ㄦ鍊间綔涓哄搴︾害鏉熸潵璁＄畻鏂囨湰鍐呭楂樺害
		*/
		int GetTextContentHeight(int nRichEditWidth = 0);
		void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true);
		void LineUp(bool bScroolVisible = true);
		void LineDown(bool bScroolVisible =true);
		void PageUp();
		void PageDown();
		void HomeUp();
		void EndDown();
		void LineLeft();
		void LineRight();
		void PageLeft();
		void PageRight();
		void HomeLeft();
		void EndRight();

		SIZE EstimateSize(SIZE szAvailable);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void Move(SIZE szOffset, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		bool DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl);

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);
		CTxtWinHost* GetTextHost() const { return m_pTwh; }
		bool PreprocessCharMessage(UINT& uMsg, WPARAM& wParam);
		void FinalizeMessageDispatch(UINT uMsg, HRESULT hr, bool bWasHandled, bool& bHandled) const;

		virtual CRichEditUI* Clone();
		virtual void CopyData(CRichEditUI* pControl) ;


	protected:
		enum { 
			DEFAULT_TIMERID = 20,
		};
		std::wstring m_strLimitText;

		CTxtWinHost* m_pTwh;
		bool m_bVScrollBarFixing;
		bool m_bWantTab;
		bool m_bWantReturn;
		bool m_bWantCtrlReturn;
		bool m_bTransparent;
		bool m_bRich;
		bool m_bReadOnly;
		bool m_bWordWrap;
		DWORD m_dwTextColor;
		int m_iFont;
		int m_iLimitText;
		LONG m_lTwhStyle;
		bool m_bDrawCaret;
		bool m_bInited;

		bool  m_fAccumulateDBC ; // TRUE - need to cumulate ytes from 2 WM_CHAR msgs
		// we are in this mode when we receive VK_PROCESSKEY
		UINT m_chLeadByte; // use when we are in _fAccumulateDBC mode

		RECT m_rcTextPadding;
		UINT m_uButtonState;
		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sDisabledImage;
		std::wstring m_sTipValue;
		DWORD m_dwTipValueColor;
		UINT m_uTipValueAlign;
		RECT m_rcTipValuePadding = {0,0,0,0};
	};

} 



