#pragma once

namespace FYUI {
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CComboWnd;

	class FYUI_API CComboUI : public CContainerUI, public IListOwnerUI
	{
		DECLARE_DUICONTROL(CComboUI)
		friend class CComboWnd;
	public:
		CComboUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		void DoInit();
		UINT GetControlFlags() const;

		std::wstring GetText() const;
		void SetEnabled(bool bEnable = true);

		void SetTextStyle(UINT uStyle);
		UINT GetTextStyle() const;
		void SetTextColor(DWORD dwTextColor);
		DWORD GetTextColor() const;
		void SetDisabledTextColor(DWORD dwTextColor);
		DWORD GetDisabledTextColor() const;
		void SetFont(int index);
		int GetFont() const;
		RECT GetTextPadding() const;
		void SetTextPadding(RECT rc);
		bool IsShowHtml();
		void SetShowHtml(bool bShowHtml = true);
		bool IsShowShadow();
		void SetShowShadow(bool bShow = true);

		std::wstring GetDropBoxAttributeList();
		void SetDropBoxAttributeList(std::wstring_view pstrList);
		SIZE GetDropBoxSize() const;
		void SetDropBoxSize(SIZE szDropBox);
		RECT GetDropBoxInset() const;
		void SetDropBoxInset(RECT szDropBox);
		RECT GetDropBoxPadding() const;
		void SetDropBoxPadding(RECT szDropBoxPadding);
		DWORD GetDropBoxColor() const;
		void SetDropBoxBkColor(DWORD dwBkColor);
		bool IsAutoDropBoxSize();
		void SetAutoDropBoxSize(bool bIsShow);

		UINT GetListType();
		TListInfoUI* GetListInfo();
		int GetCurSel() const;  
		void InitCursel();
		bool SelectItem(int iIndex, bool bTakeFocus = false, bool bIsClick=false, bool bSetScrollPos =true);
		void SendNotifyClick();
		bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
		bool UnSelectItem(int iIndex, bool bOthers = false);
		bool SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate = true);

		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl,bool bChildDelayed =true);
		bool RemoveAt(int iIndex,bool bChildDelayed =true);
		void RemoveAll(bool bChildDelayed =true);

		bool Activate();

		std::wstring_view GetNormalImage() const;
		void SetNormalImage(std::wstring_view pStrImage);
		std::wstring_view GetHotImage() const;
		void SetHotImage(std::wstring_view pStrImage);
		std::wstring_view GetPushedImage() const;
		void SetPushedImage(std::wstring_view pStrImage);
		std::wstring_view GetFocusedImage() const;
		void SetFocusedImage(std::wstring_view pStrImage);
		std::wstring_view GetDisabledImage() const;
		void SetDisabledImage(std::wstring_view pStrImage);

		bool GetScrollSelect();
		void SetScrollSelect(bool bScrollSelect);

		void SetItemFont(int index);
		void SetItemTextStyle(UINT uStyle);
		RECT GetItemTextPadding() const;
		void SetItemTextPadding(RECT rc);
		DWORD GetItemTextColor() const;
		void SetItemTextColor(DWORD dwTextColor);
		DWORD GetItemBkColor() const;
		void SetItemBkColor(DWORD dwBkColor);
		std::wstring_view GetItemBkImage() const;
		void SetItemBkImage(std::wstring_view pStrImage);
		bool IsAlternateBk() const;
		void SetAlternateBk(bool bAlternateBk);
		DWORD GetSelectedItemTextColor() const;
		void SetSelectedItemTextColor(DWORD dwTextColor);
		DWORD GetSelectedItemBkColor() const;
		void SetSelectedItemBkColor(DWORD dwBkColor);
		std::wstring_view GetSelectedItemImage() const;
		void SetSelectedItemImage(std::wstring_view pStrImage);
		DWORD GetHotItemTextColor() const;
		void SetHotItemTextColor(DWORD dwTextColor);
		DWORD GetHotItemBkColor() const;
		void SetHotItemBkColor(DWORD dwBkColor);
		std::wstring_view GetHotItemImage() const;
		void SetHotItemImage(std::wstring_view pStrImage);
		DWORD GetDisabledItemTextColor() const;
		void SetDisabledItemTextColor(DWORD dwTextColor);
		DWORD GetDisabledItemBkColor() const;
		void SetDisabledItemBkColor(DWORD dwBkColor);
		std::wstring_view GetDisabledItemImage() const;
		void SetDisabledItemImage(std::wstring_view pStrImage);
		DWORD GetItemLineColor() const;
		void SetItemLineColor(DWORD dwLineColor);
		bool IsItemShowHtml();
		void SetItemShowHtml(bool bShowHtml = true);
		bool IsShowSelectedItemText();
		void SetShowSelectedItemText(bool bIsShow);


		SIZE EstimateSize(SIZE szAvailable);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void Move(SIZE szOffset, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void PaintText(CPaintRenderContext& renderContext) override;
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

		virtual CComboUI* Clone();
		virtual void CopyData(CComboUI* pControl) ;

	public:
		void SortItems();
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

	protected:
		static int __cdecl ItemComareFunc(void* pvlocale, const void* item1, const void* item2);
		int __cdecl ItemComareFunc(const void* item1, const void* item2);

	protected:
		PULVCompareFunc m_pCompareFunc;
		UINT_PTR m_compareData;

	protected:
		CComboWnd* m_pWindow;

		int m_iCurSel;
		DWORD	m_dwTextColor;
		DWORD	m_dwDisabledTextColor;
		int		m_iFont;
		UINT	m_uTextStyle;
		RECT	m_rcTextPadding;
		bool	m_bShowHtml;
		bool	m_bShowShadow;
		std::wstring m_sDropBoxAttributes;
		SIZE m_szDropBox;
		RECT m_rcDropBox;
		RECT m_rcDropBoxPadding;
		DWORD   m_dwDropBoxBkcolor;
		UINT m_uButtonState;

		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sPushedImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sDisabledImage;

		bool m_bScrollSelect;
		TListInfoUI m_ListInfo;
		bool m_bShowSelectedItemText;
		bool m_bIsAutoDropBoxSize;
	};

} 



