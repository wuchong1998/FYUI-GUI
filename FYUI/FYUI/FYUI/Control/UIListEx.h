#pragma once
#include "Layout/UIVerticalLayout.h"
#include "Layout/UIHorizontalLayout.h"

namespace FYUI {

	class CPaintRenderContext;

	class IListComboCallbackUI
	{
	public:
		virtual void GetItemComboTextArray(CControlUI* pCtrl, int iItem, int iSubItem) = 0;
	};

	class CComboBoxUI;
	class CListContainerHeaderItemUI;
	class CListTextExtElementUI;

	class FYUI_API CListExUI : public CListUI, public INotifyUI
	{
		DECLARE_DUICONTROL(CListExUI)

	public:
		CListExUI();

		std::wstring_view GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(std::wstring_view pstrName);

	public: 
		virtual void DoEvent(TEventUI& event);

		public:
			void InitListCtrl();
			bool Remove(CControlUI* pControl);
			bool RemoveAt(int iIndex);
			void RemoveAll();

		protected:
			CRichEditUI*		m_pEditUI;
			CComboBoxUI*	m_pComboBoxUI;

	public:
		virtual BOOL CheckColumEditable(int nColum);
		virtual CRichEditUI* GetEditUI();

		virtual BOOL CheckColumComboBoxable(int nColum);
		virtual CComboBoxUI* GetComboBoxUI();

		virtual BOOL CheckColumCheckBoxable(int nColum);

	public:
		virtual void Notify(TNotifyUI& msg);
		BOOL	m_bAddMessageFilter;
		int		m_nRow,m_nColum;
		void	SetEditRowAndColum(int nRow,int nColum) { m_nRow = nRow; m_nColum = nColum; };

	public:
		IListComboCallbackUI* m_pXCallback;
		virtual IListComboCallbackUI* GetTextArrayCallback() const;
		virtual void SetTextArrayCallback(IListComboCallbackUI* pCallback);

		public:
			void OnListItemClicked(int nIndex, int nColum, RECT* lpRCColum, std::wstring_view lpstrText);
			void OnListItemChecked(int nIndex, int nColum, BOOL bChecked);
			void DismissInlineEditorsForRow(int nRow);

		public:
			void SetColumItemColor(int nIndex, int nColum, DWORD iBKColor);
		BOOL GetColumItemColor(int nIndex, int nColum, DWORD& iBKColor);

	private:
		CListContainerHeaderItemUI* GetHeaderItemAt(int nColum) const;
		CListTextExtElementUI* GetTextExtItemAt(int nIndex) const;
		void HandleHeaderCheckedNotify(const TNotifyUI& msg);
		void HandleItemCheckedNotify(const TNotifyUI& msg);
		bool HandleInlineEditNotify(const TNotifyUI& msg);
		bool HandleInlineComboNotify(const TNotifyUI& msg);
		bool HandleInlineScrollNotify(const TNotifyUI& msg);
		void HideInlineEditControl();
		void HideInlineComboControl();
		void HideEditAndComboCtrl();
		void CommitInlineEditText();
		void CommitInlineComboSelection();
			void ShowInlineEdit(int nIndex, int nColum, const RECT& rcColumn, std::wstring_view lpstrText);
			void ShowInlineCombo(int nIndex, int nColum, const RECT& rcColumn, std::wstring_view lpstrText);
			void ActivateInlineCellEditor(int nIndex, int nColum, const RECT& rcColumn, std::wstring_view lpstrText);
			void SyncHeaderCheckState(int nColum, BOOL bChecked);
			void SyncInlineEditorAfterRemoval(int nRow);
		};

	/////////////////////////////////////////////////////////////////////////////////////
	//
	class FYUI_API CListContainerHeaderItemUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CListContainerHeaderItemUI)

	public:
		CListContainerHeaderItemUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetControlFlags() const;

		void SetEnabled(BOOL bEnable = TRUE);

		BOOL IsDragable() const;
		void SetDragable(BOOL bDragable);
		DWORD GetSepWidth() const;
		void SetSepWidth(int iWidth);
		DWORD GetTextStyle() const;
		void SetTextStyle(UINT uStyle);
		DWORD GetTextColor() const;
		void SetTextColor(DWORD dwTextColor);
		void SetTextPadding(RECT rc);
		RECT GetTextPadding() const;
		void SetFont(int index);
		BOOL IsShowHtml();
		void SetShowHtml(BOOL bShowHtml = TRUE);
		std::wstring_view GetNormalImage() const;
		void SetNormalImage(std::wstring_view pStrImage);
		std::wstring_view GetHotImage() const;
		void SetHotImage(std::wstring_view pStrImage);
		std::wstring_view GetPushedImage() const;
		void SetPushedImage(std::wstring_view pStrImage);
		std::wstring_view GetFocusedImage() const;
		void SetFocusedImage(std::wstring_view pStrImage);
		std::wstring_view GetSepImage() const;
		void SetSepImage(std::wstring_view pStrImage);

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		RECT GetThumbRect() const;

		void PaintText(CPaintRenderContext& renderContext) override;
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

	protected:
		POINT ptLastMouse;
		BOOL m_bDragable;
		UINT m_uButtonState;
		int m_iSepWidth;
		DWORD m_dwTextColor;
		int m_iFont;
		UINT m_uTextStyle;
		BOOL m_bShowHtml;
		RECT m_rcTextPadding;
		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sPushedImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sSepImage;
		std::wstring m_sSepImageModify;

		//閺€顖涘瘮缂傛牞绶?
		BOOL m_bEditable;

		//閺€顖涘瘮缂佸嫬鎮庡?
		BOOL m_bComboable;

		//閺€顖涘瘮婢跺秹鈧顢?
		BOOL m_bCheckBoxable;

	public:
		BOOL GetColumeEditable();
		void SetColumeEditable(BOOL bEnable);

		BOOL GetColumeComboable();
		void SetColumeComboable(BOOL bEnable);

		BOOL GetColumeCheckable();
		void SetColumeCheckable(BOOL bEnable);

	public:
		void SetCheck(BOOL bCheck);
		BOOL GetCheck();

	private:
		UINT	m_uCheckBoxState;
		BOOL	m_bChecked;

		std::wstring m_sCheckBoxNormalImage;
		std::wstring m_sCheckBoxHotImage;
		std::wstring m_sCheckBoxPushedImage;
		std::wstring m_sCheckBoxFocusedImage;
		std::wstring m_sCheckBoxDisabledImage;

		std::wstring m_sCheckBoxSelectedImage;
		std::wstring m_sCheckBoxForeImage;

		SIZE m_cxyCheckBox;

	public:
		BOOL DrawCheckBoxImage(CPaintRenderContext& renderContext, const std::wstring& pStrImage, const std::wstring& pStrModify = std::wstring());
		std::wstring_view GetCheckBoxNormalImage();
		void SetCheckBoxNormalImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxHotImage();
		void SetCheckBoxHotImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxPushedImage();
		void SetCheckBoxPushedImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxFocusedImage();
		void SetCheckBoxFocusedImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxDisabledImage();
		void SetCheckBoxDisabledImage(std::wstring_view pStrImage);

		std::wstring_view GetCheckBoxSelectedImage();
		void SetCheckBoxSelectedImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxForeImage();
		void SetCheckBoxForeImage(std::wstring_view pStrImage);

		void GetCheckBoxRect(RECT &rc);	

		int GetCheckBoxWidth() const;       // 鐎圭偤妾径褍鐨担宥囩枂娴ｈ法鏁etPos閼惧嘲褰囬敍宀冪箹闁插苯绶遍崚鎵畱閺勵垶顣╃拋鍓ф畱閸欏倽鈧啫鈧?
		void SetCheckBoxWidth(int cx);      // 妫板嫯顔曢惃鍕棘閼板啫鈧?
		int GetCheckBoxHeight() const;      // 鐎圭偤妾径褍鐨担宥囩枂娴ｈ法鏁etPos閼惧嘲褰囬敍宀冪箹闁插苯绶遍崚鎵畱閺勵垶顣╃拋鍓ф畱閸欏倽鈧啫鈧?
		void SetCheckBoxHeight(int cy);     // 妫板嫯顔曢惃鍕棘閼板啫鈧?


	public:
		CContainerUI* m_pOwner;
		void SetOwner(CContainerUI* pOwner);
		CContainerUI* GetOwner();
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListTextExtElementUI : public CListLabelElementUI
	{
		DECLARE_DUICONTROL(CListTextExtElementUI)

	public:
		CListTextExtElementUI();
		~CListTextExtElementUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetControlFlags() const;

		std::wstring_view GetText(int iIndex) const;
		void SetText(int iIndex, std::wstring_view pstrText);
		void SetText(int iIndex, const std::wstring& pstrText) { SetText(iIndex, std::wstring_view(pstrText)); }

			void SetOwner(CControlUI* pOwner);
			void SetVisible(bool bVisible = true);
			void SetEnabled(bool bEnable = true);
			std::wstring* GetLinkContent(int iIndex);

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);

		void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem) override;

	protected:
		enum { MAX_LINK = 8 };
		int m_nLinks;
		RECT m_rcLinks[MAX_LINK];
		std::wstring m_sLinks[MAX_LINK];
		int m_nHoverLink;
		CListUI* m_pOwner;
		CStdPtrArray m_aTexts;
		mutable std::wstring m_sTextCache;

	private:
		UINT	m_uCheckBoxState;
		BOOL	m_bChecked;

		std::wstring m_sCheckBoxNormalImage;
		std::wstring m_sCheckBoxHotImage;
		std::wstring m_sCheckBoxPushedImage;
		std::wstring m_sCheckBoxFocusedImage;
		std::wstring m_sCheckBoxDisabledImage;

		std::wstring m_sCheckBoxSelectedImage;
		std::wstring m_sCheckBoxForeImage;

		SIZE m_cxyCheckBox;

	public:
		virtual bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		virtual void PaintStatusImage(CPaintRenderContext& renderContext) override;
		BOOL DrawCheckBoxImage(CPaintRenderContext& renderContext, const std::wstring& pStrImage, const std::wstring& pStrModify, RECT& rcCheckBox);
		std::wstring_view GetCheckBoxNormalImage();
		void SetCheckBoxNormalImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxHotImage();
		void SetCheckBoxHotImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxPushedImage();
		void SetCheckBoxPushedImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxFocusedImage();
		void SetCheckBoxFocusedImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxDisabledImage();
		void SetCheckBoxDisabledImage(std::wstring_view pStrImage);

		std::wstring_view GetCheckBoxSelectedImage();
		void SetCheckBoxSelectedImage(std::wstring_view pStrImage);
		std::wstring_view GetCheckBoxForeImage();
		void SetCheckBoxForeImage(std::wstring_view pStrImage);

		void GetCheckBoxRect(int nIndex, RECT &rc);	
		void GetColumRect(int nColum, RECT &rc);

		int GetCheckBoxWidth() const;       // 鐎圭偤妾径褍鐨担宥囩枂娴ｈ法鏁etPos閼惧嘲褰囬敍宀冪箹闁插苯绶遍崚鎵畱閺勵垶顣╃拋鍓ф畱閸欏倽鈧啫鈧?
		void SetCheckBoxWidth(int cx);      // 妫板嫯顔曢惃鍕棘閼板啫鈧?
		int GetCheckBoxHeight() const;      // 鐎圭偤妾径褍鐨担宥囩枂娴ｈ法鏁etPos閼惧嘲褰囬敍宀冪箹闁插苯绶遍崚鎵畱閺勵垶顣╃拋鍓ф畱閸欏倽鈧啫鈧?
		void SetCheckBoxHeight(int cy);     // 妫板嫯顔曢惃鍕棘閼板啫鈧?

		void SetCheck(BOOL bCheck);
		BOOL GetCheck() const;

	public:
		int HitTestColum(POINT ptMouse);
		BOOL CheckColumEditable(int nColum);

	private:
		bool HandleLinkCursorEvent(const TEventUI& event);
		bool HandleLinkClickEvent(const TEventUI& event);
		void UpdateLinkHoverState(const TEventUI& event);
		void NotifyInlineCellClick(const TEventUI& event, CListExUI* pListCtrl);
		void HandleColumnCheckBoxEvent(const TEventUI& event, CListExUI* pListCtrl);

		typedef struct tagColumColorNode
		{
			BOOL  bEnable;
			DWORD iTextColor;
			DWORD iBKColor;
		}COLUMCOLORNODE;

		COLUMCOLORNODE ColumCorlorArray[UILIST_MAX_COLUMNS];

	public:
		void SetColumItemColor(int nColum, DWORD iBKColor);
		BOOL GetColumItemColor(int nColum, DWORD& iBKColor);

	};
} 



