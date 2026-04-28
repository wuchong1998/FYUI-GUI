#pragma once

namespace FYUI
{
	/////////////////////////////////////////////////////////////////////////////////////
	//

	typedef int (CALLBACK *PULVCompareFunc)(UINT_PTR, UINT_PTR, UINT_PTR);

	class CListHeaderUI;

#define UILIST_MAX_COLUMNS 32

	typedef struct tagTListInfoUI
	{
		int nColumns;
		RECT rcColumn[UILIST_MAX_COLUMNS];
		int nFont;
		UINT uTextStyle;
		RECT rcTextPadding;
		DWORD dwTextColor;
		DWORD dwBkColor;
		std::wstring sBkImage;
		bool bAlternateBk;
		DWORD dwSelectedTextColor;
		DWORD dwSelectedBkColor;
		std::wstring sSelectedImage;
		DWORD dwHotTextColor;
		DWORD dwHotBkColor;
		std::wstring sHotImage;
		DWORD dwDisabledTextColor;
		DWORD dwDisabledBkColor;
		std::wstring sDisabledImage;
		std::wstring sForeImage;
		std::wstring sHotForeImage;
		std::wstring sSelectedForeImage;

		DWORD dwLineColor;
		bool bShowRowLine;
		bool bShowColumnLine;
		bool bShowHtml;
		bool bMultiExpandable;
		bool bRSelected;
	} TListInfoUI;


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class IListCallbackUI
	{
	public:
		virtual std::wstring_view GetItemText(CControlUI* pList, int iItem, int iSubItem) = 0;
		virtual DWORD GetItemTextColor(CControlUI* pList, int iItem, int iSubItem, int iState) = 0;// iState锛?-姝ｅ父銆?-婵€娲汇€?-閫夋嫨銆?-绂佺敤
	};

	class IListOwnerUI
	{
	public:
		virtual UINT GetListType() = 0;
		virtual TListInfoUI* GetListInfo() = 0;
		virtual int GetCurSel() const = 0;
		virtual bool SelectItem(int iIndex, bool bTakeFocus = false, bool bIsClick = false, bool bSetScrollPos = true) = 0;
		virtual bool SelectMultiItem(int iIndex, bool bTakeFocus = false) = 0;
		virtual bool UnSelectItem(int iIndex, bool bOthers = false) = 0;
		virtual void DoEvent(TEventUI& event) = 0;
	};

	class IListUI : public IListOwnerUI
	{
	public:
		virtual CListHeaderUI* GetHeader() const = 0;
		virtual CContainerUI* GetList() const = 0;
		virtual IListCallbackUI* GetTextCallback() const = 0;
		virtual void SetTextCallback(IListCallbackUI* pCallback) = 0;
		virtual bool ExpandItem(int iIndex, bool bExpand = true) = 0;
		virtual int GetExpandedItem() const = 0;

		virtual void SetMultiSelect(bool bMultiSel) = 0;
		virtual bool IsMultiSelect() const = 0;
		virtual void SelectAllItems() = 0;
		virtual void UnSelectAllItems() = 0;
		virtual int GetSelectItemCount() const = 0;
		virtual int GetNextSelItem(int nItem) const = 0;
	};

	class IListItemUI
	{
	public:
		virtual int GetIndex() const = 0;
		virtual void SetIndex(int iIndex) = 0;
		virtual IListOwnerUI* GetOwner() = 0;
		virtual void SetOwner(CControlUI* pOwner) = 0;
		virtual bool IsSelected() const = 0;
		virtual bool Select(bool bSelect = true, bool bIsClick = false) = 0;
		virtual bool SelectMulti(bool bSelect = true) = 0;
		virtual bool IsExpanded() const = 0;
		virtual bool Expand(bool bExpand = true) = 0;
		virtual void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem) = 0;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CListBodyUI;
	class CListHeaderUI;
	class CComboBoxUI;
	class FYUI_API CListUI : public CVerticalLayoutUI, public IListUI
	{
		DECLARE_DUICONTROL(CListUI)

	public:
		CListUI();

		std::wstring_view GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		bool GetScrollSelect();
		void SetScrollSelect(bool bScrollSelect);
		int GetCurSel() const;
		int GetCurSelActivate() const;
		bool SelectItem(int iIndex, bool bTakeFocus = false, bool bIsClick = false,bool bSetScrollPos=true);
		bool SelectItemActivate(int iIndex);    // 鍙屽嚮閫変腑

		bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
		void SetMultiSelect(bool bMultiSel);
		bool IsMultiSelect() const;
		bool UnSelectItem(int iIndex, bool bOthers = false);
		void SelectAllItems();
		void UnSelectAllItems();
		int GetSelectItemCount() const;
		int GetNextSelItem(int nItem) const;

		CListHeaderUI* GetHeader() const;  
		CContainerUI* GetList() const;
		UINT GetListType();
		TListInfoUI* GetListInfo();

		CControlUI* GetItemAt(int iIndex) const;
		int GetItemIndex(CControlUI* pControl) const;
		bool SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate = true);
		int GetCount() const;
		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl);
		bool RemoveAt(int iIndex);
		void RemoveAll();

		void EnsureVisible(int iIndex);
		void Scroll(int dx, int dy);

		bool IsDelayedDestroy() const;
		void SetDelayedDestroy(bool bDelayed);
		int GetChildPadding() const;
		void SetChildPadding(int iPadding);

		void SetItemFont(int index);
		void SetItemTextStyle(UINT uStyle);
		void SetItemTextPadding(RECT rc);
		void SetItemTextColor(DWORD dwTextColor);
		void SetItemBkColor(DWORD dwBkColor);
		void SetItemBkImage(std::wstring_view pStrImage);
		void SetAlternateBk(bool bAlternateBk);
		void SetSelectedItemTextColor(DWORD dwTextColor);
		void SetSelectedItemBkColor(DWORD dwBkColor);
		void SetSelectedItemImage(std::wstring_view pStrImage);
		void SetHotItemTextColor(DWORD dwTextColor);
		void SetHotItemBkColor(DWORD dwBkColor);
		void SetHotItemImage(std::wstring_view pStrImage);
		void SetDisabledItemTextColor(DWORD dwTextColor);
		void SetDisabledItemBkColor(DWORD dwBkColor);
		void SetDisabledItemImage(std::wstring_view pStrImage);
		void SetItemLineColor(DWORD dwLineColor);
		void SetItemShowRowLine(bool bShowLine = false);
		void SetItemShowColumnLine(bool bShowLine = false);
		bool IsItemShowHtml();
		void SetItemShowHtml(bool bShowHtml = true);
		bool IsItemRSelected();
		void SetItemRSelected(bool bSelected = true);
		RECT GetItemTextPadding() const;
		DWORD GetItemTextColor() const;
		DWORD GetItemBkColor() const;
		std::wstring_view GetItemBkImage() const;
		bool IsAlternateBk() const;
		DWORD GetSelectedItemTextColor() const;
		DWORD GetSelectedItemBkColor() const;
		std::wstring_view GetSelectedItemImage() const;
		DWORD GetHotItemTextColor() const;
		DWORD GetHotItemBkColor() const;
		std::wstring_view GetHotItemImage() const;
		DWORD GetDisabledItemTextColor() const;
		DWORD GetDisabledItemBkColor() const;
		std::wstring_view GetDisabledItemImage() const;
		DWORD GetItemLineColor() const;

		void SetMultiExpanding(bool bMultiExpandable); 
		int GetExpandedItem() const;
		bool ExpandItem(int iIndex, bool bExpand = true);

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void Move(SIZE szOffset, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		IListCallbackUI* GetTextCallback() const;
		void SetTextCallback(IListCallbackUI* pCallback);

		SIZE GetScrollPos() const;
		SIZE GetScrollRange() const;
		void SetScrollPos(SIZE szPos, bool bMsg = true);
		void LineUp();
		void LineDown();
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
		void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
		virtual CScrollBarUI* GetVerticalScrollBar() const;
		virtual CScrollBarUI* GetHorizontalScrollBar() const;
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

		virtual BOOL CheckColumEditable(int nColum) { return FALSE; };
		virtual CRichEditUI* GetEditUI() { return NULL; };
		virtual BOOL CheckColumComboBoxable(int nColum) { return FALSE; };
		virtual CComboBoxUI* GetComboBoxUI() { return NULL; };

		virtual CListUI* Clone();
		virtual void CopyData(CListUI* pControl) ;


	protected:
			bool ResolveListItemByIndex(int iIndex, CControlUI*& pControl, IListItemUI*& pListItem, bool bRequireEnabled = false, bool bRequireVisible = false) const;
			void ReindexListItems(int iStart);
			void RestoreSelectionAfterRemoval(int iRemovedIndex);
			void ShiftSelectionIndices(int iStartIndex, int iDelta);
			void ShiftTrackedItemIndices(int iStartIndex, int iDelta);
			void RestoreTrackedStateAfterRemoval(int iRemovedIndex);
			void AddSelectionIndex(int iIndex);
			void RemoveSelectionIndex(int iIndex);
		void NotifySelectionChanged(int iIndex);
		void NotifySelectionChanged(int iIndex, int iAnchorIndex);
		void NotifySelectionCleared();
		void ApplySelectionFocusAndScroll(CControlUI* pControl, int iIndex, bool bTakeFocus, bool bSetScrollPos);
		void SelectItemRange(int iStart, int iEnd);
		RECT GetListViewportRect() const;
		int GetMinSelItemIndex();
		int GetMaxSelItemIndex();
		void UpdateListFocusState(bool bFocused);
		bool HandleListKeyDownEvent(const TEventUI& event);
		bool HandleListScrollWheelEvent(const TEventUI& event);

	protected:
		bool m_bScrollSelect;
		bool m_bMultiSel;
		int m_iCurSel;
		int m_iFirstSel;
		CStdPtrArray m_aSelItems;
		int m_iCurSelActivate;  // 鍙屽嚮鐨勫垪
		int m_iExpandedItem;
		IListCallbackUI* m_pCallback;
		CListBodyUI* m_pList;
		CListHeaderUI* m_pHeader;
		TListInfoUI m_ListInfo;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListBodyUI : public CVerticalLayoutUI
	{
	public:
		CListBodyUI(CListUI* pOwner);


		int GetScrollStepSize() const;
		void SetScrollPos(SIZE szPos, bool bMsg = true);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

		virtual void CopyData(CListBodyUI* pControl) ;

	protected:
		static int __cdecl ItemComareFunc(void *pvlocale, const void *item1, const void *item2);
		int __cdecl ItemComareFunc(const void *item1, const void *item2);
	protected:
		CListUI* m_pOwner;
		PULVCompareFunc m_pCompareFunc;
		UINT_PTR m_compareData;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListHeaderUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CListHeaderUI)
	public:
		CListHeaderUI();
		virtual ~CListHeaderUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		SIZE EstimateSize(SIZE szAvailable);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void SetScaleHeader(bool bIsScale);
		bool IsScaleHeader() const;

		void DoInit();
		void DoPostPaint(CPaintRenderContext& renderContext) override;

		virtual CListHeaderUI* Clone();
		virtual void CopyData(CListHeaderUI* pControl) ;

	private:
		bool m_bIsScaleHeader;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListHeaderItemUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CListHeaderItemUI)

	public:
		CListHeaderItemUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetControlFlags() const;

		void SetEnabled(bool bEnable = true);

		bool IsDragable() const;
		void SetDragable(bool bDragable);
		DWORD GetSepWidth() const;
		void SetSepWidth(int iWidth);
		DWORD GetTextStyle() const;
		void SetTextStyle(UINT uStyle);
		DWORD GetTextColor() const;
		void SetTextColor(DWORD dwTextColor);
		void SetTextPadding(RECT rc);
		RECT GetTextPadding() const;
		void SetFont(int index);
		bool IsShowHtml();
		void SetShowHtml(bool bShowHtml = true);
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
		void SetScale(int nScale);
		int GetScale() const;

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		RECT GetThumbRect() const;

		void PaintText(CPaintRenderContext& renderContext) override;
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

		virtual CListHeaderItemUI* Clone();
		virtual void CopyData(CListHeaderItemUI* pControl) ;

	protected:
		POINT ptLastMouse;
		bool m_bDragable;
		UINT m_uButtonState;
		int m_iSepWidth;
		DWORD m_dwTextColor;
		int m_iFont;
		UINT m_uTextStyle;
		bool m_bShowHtml;
		RECT m_rcTextPadding;
		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sPushedImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sSepImage;
		std::wstring m_sSepImageModify;
		int m_nScale;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListElementUI : public CControlUI, public IListItemUI
	{
	public:
		CListElementUI();

		std::wstring_view GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		void SetEnabled(bool bEnable = true);

		int GetIndex() const;
		void SetIndex(int iIndex);

		IListOwnerUI* GetOwner();
		void SetOwner(CControlUI* pOwner);
		void SetVisible(bool bVisible = true);

		bool IsSelected() const;
		bool Select(bool bSelect = true,bool bIsClick=false);
		bool SelectMulti(bool bSelect = true);
		bool IsExpanded() const;
		bool Expand(bool bExpand = true);

		void Invalidate(); // 鐩存帴CControl::Invalidate浼氬鑷存粴鍔ㄦ潯鍒锋柊锛岄噸鍐欏噺灏戝埛鏂板尯鍩?
		bool Activate();

		void DoEvent(TEventUI& event);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void DrawItemBk(CPaintRenderContext& renderContext, const RECT& rcItem);
		virtual void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem);
		virtual CListElementUI* Clone();
		virtual void CopyData(CListElementUI* pControl) ;

	protected:
		int m_iIndex;
		bool m_bSelected;
		UINT m_uButtonState;
		IListOwnerUI* m_pOwner;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListLabelElementUI : public CListElementUI
	{
		DECLARE_DUICONTROL(CListLabelElementUI)
	public:
		CListLabelElementUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;

		void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem) override;
	

		virtual CListLabelElementUI* Clone();
		virtual void CopyData(CListLabelElementUI* pControl) ;
	public:
		
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListTextElementUI : public CListLabelElementUI
	{
		DECLARE_DUICONTROL(CListTextElementUI)
	public:
		CListTextElementUI();
		~CListTextElementUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetControlFlags() const;

		std::wstring_view GetText(int iIndex) const;
		void SetText(int iIndex, std::wstring_view pstrText);

		DWORD GetTextColor(int iIndex) const;
		void SetTextColor(int iIndex, DWORD dwTextColor);

		void SetOwner(CControlUI* pOwner);
		std::wstring* GetLinkContent(int iIndex);

		void DoEvent(TEventUI& event);
		SIZE EstimateSize(SIZE szAvailable);

		void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem) override;

		virtual CListTextElementUI* Clone();
		virtual void CopyData(CListTextElementUI* pControl) ;

	protected:
		enum { MAX_LINK = 8 };
		int m_nLinks;
		RECT m_rcLinks[MAX_LINK];
		std::wstring m_sLinks[MAX_LINK];
		int m_nHoverLink;
		IListUI* m_pOwner;
		CStdPtrArray m_aTexts;
		CStdPtrArray m_aTextColors;
		mutable std::wstring m_sTextResourceCache;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListContainerElementUI : public CHorizontalLayoutUI, public IListItemUI
	{
		DECLARE_DUICONTROL(CListContainerElementUI)
	public:
		CListContainerElementUI();

		std::wstring_view GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		int GetIndex() const;
		void SetIndex(int iIndex);

		IListOwnerUI* GetOwner();
		void SetOwner(CControlUI* pOwner);
		void SetVisible(bool bVisible = true);
		void SetEnabled(bool bEnable = true);

		bool IsSelected() const;
		bool Select(bool bSelect = true, bool bIsClick = false);
		bool SelectMulti(bool bSelect = true);
		bool IsExpanded() const;
		bool Expand(bool bExpand = true);

		void Invalidate(); // 鐩存帴CControl::Invalidate浼氬鑷存粴鍔ㄦ潯鍒锋柊锛岄噸鍐欏噺灏戝埛鏂板尯鍩?
		bool Activate();

		void DoEvent(TEventUI& event);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;

		virtual void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem);
		virtual void DrawItemBk(CPaintRenderContext& renderContext, const RECT& rcItem);

		void SetPos(RECT rc, bool bNeedInvalidate = true);

		virtual CListContainerElementUI* Clone();
		virtual void CopyData(CListContainerElementUI* pControl) ;

	protected:
		int m_iIndex;
		bool m_bSelected;
		UINT m_uButtonState;
		IListOwnerUI* m_pOwner;
	};

} 


