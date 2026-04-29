#pragma once

#include <cstdint>
#include <functional>
#include <vector>

namespace FYUI
{
	class FYUI_API CVirtualListUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CVirtualListUI)

	public:
		using ItemIndex = std::uint64_t;
		using CreateItemCallback = std::function<CControlUI*(CVirtualListUI*)>;
		using BindItemCallback = std::function<void(CVirtualListUI*, CControlUI*, ItemIndex)>;
		using PaintItemCallback = std::function<void(CVirtualListUI*, CPaintRenderContext&, ItemIndex, const RECT&)>;
		using ItemEventCallback = std::function<void(CVirtualListUI*, ItemIndex, TEventUI&)>;

		/// Creates an empty virtual list and initializes the internal vertical scrollbar.
		CVirtualListUI();
		~CVirtualListUI() override;

		/// Returns the XML class name used by the dialog builder.
		std::wstring_view GetClass() const override;
		/// Returns the requested interface pointer when the caller asks for VirtualList/VirtualListUI.
		LPVOID GetInterface(std::wstring_view pstrName) override;

		/// Sets the logical item count in fixed-height mode or resizes the variable-height table in variable-height mode.
		/// This method clamps selection, hot state and scroll offset automatically after the data size changes.
		void SetItemCount(ItemIndex count);
		/// Returns the current logical item count.
		ItemIndex GetItemCount() const;

		/// Switches the list into fixed-height mode and applies the same height to every item.
		/// Any previously supplied per-item height table is cleared.
		void SetFixedItemHeight(int height);
		/// Returns the height used by fixed-height mode and also the fallback height for invalid per-item values.
		int GetFixedItemHeight() const;
		/// Returns true when the list is using a single fixed height for all items.
		bool IsFixedHeightMode() const;

		/// Copies a per-item height table and switches the list into variable-height mode.
		/// The number of items becomes heights.size().
		void SetItemHeights(const std::vector<int>& heights);
		/// Moves a per-item height table into the list and switches the list into variable-height mode.
		/// Prefer this overload when the caller can transfer ownership to avoid an extra vector copy.
		void SetItemHeights(std::vector<int>&& heights);
		/// Replaces the height of a single item.
		/// If the list is currently in fixed-height mode it will first expand into variable-height mode using the fixed height as the default value.
		bool SetItemHeight(ItemIndex index, int height);
		/// Copies a raw height array into the internal per-item height table and switches to variable-height mode.
		void SetItemHeights(const int* heights, size_t count);
		/// Clears the per-item height table and returns the list to fixed-height mode while keeping the current item count.
		void ClearItemHeights();
		/// Returns true when the list is using a per-item height table.
		bool IsVariableHeightMode() const;
		/// Returns the effective height of the specified item.
		/// In fixed-height mode this is always GetFixedItemHeight().
		int GetItemHeight(ItemIndex index) const;

		/// Sets how many extra rows the list realizes beyond the strictly visible range to reduce scroll pop-in.
		void SetOverscanItemCount(int count);
		/// Returns the current overscan row count.
		int GetOverscanItemCount() const;

		/// Sets the default background color drawn behind each realized item before the child control paints.
		void SetItemBkColor(DWORD dwColor);
		/// Returns the default background color drawn for normal item state.
		DWORD GetItemBkColor() const;
		/// Sets the background color drawn when the mouse is hovering an item.
		void SetItemHotBkColor(DWORD dwColor);
		/// Returns the hover background color.
		DWORD GetItemHotBkColor() const;
		/// Sets the background color drawn for the selected item.
		void SetItemSelectedBkColor(DWORD dwColor);
		/// Returns the selected-item background color.
		DWORD GetItemSelectedBkColor() const;

		/// Enables or disables click-to-cancel selection.
		/// When enabled, clicking the already selected item again will clear the selection.
		void SetAllowSelectionCancel(bool allow);
		/// Returns whether click-to-cancel selection is enabled.
		bool IsAllowSelectionCancel() const;

		/// Sets the callback that creates pooled child controls for realized items.
		/// Changing this callback recreates the current pool.
		void SetCreateItemCallback(CreateItemCallback callback);
		/// Sets the callback that binds one realized child control to a logical item index.
		void SetBindItemCallback(BindItemCallback callback);
		/// Sets an optional paint callback invoked before each realized child control paints.
		void SetPaintItemCallback(PaintItemCallback callback);
		/// Sets the callback invoked when an item receives a click event.
		void SetItemClickCallback(ItemEventCallback callback);
		/// Sets the callback invoked when an item receives a double-click event.
		void SetItemDoubleClickCallback(ItemEventCallback callback);

		/// Recomputes scrollbar state, refreshes realized items and invalidates the control.
		void Refresh();
		/// Rebinds a single realized item if it is currently visible.
		void RefreshItem(ItemIndex index);
		/// Scrolls the list until the target item becomes visible.
		/// When alignTop is true the item is aligned to the top of the viewport when possible.
		void EnsureVisible(ItemIndex index, bool alignTop = false);

		/// Returns the logical item indices currently realized in the visible pool.
		void GetDisplayCollection(std::vector<ItemIndex>& collection) const;
		/// Returns the currently realized logical item indices converted to int values when they fit.
		void GetDisplayCollection(std::vector<int>& collection) const;

		/// Selects the specified logical item.
		/// Selection automatically scrolls into view and can optionally move focus and send an itemselect notify.
		bool SelectItem(ItemIndex index, bool takeFocus = false, bool notify = true);
		/// Clears the current selection and optionally sends an itemselect notify with no active item.
		void ClearSelection(bool notify = true);
		/// Returns true when any item is currently selected.
		bool HasSelection() const;
		/// Clears the current selection only when the specified index is the selected item.
		bool UnselectItem(ItemIndex index, bool notify = true);
		/// Returns the logical index of the selected item, or an internal invalid marker when no item is selected.
		ItemIndex GetSelectedIndex() const;
		/// Returns true when the specified item is currently selected.
		bool IsItemSelected(ItemIndex index) const;
		/// Returns true when any realized or tracked item is currently in hot-hover state.
		bool HasHotItem() const;
		/// Returns the logical index currently in hot-hover state, or an internal invalid marker when no item is hot.
		ItemIndex GetHotItemIndex() const;

		/// Returns the current vertical scroll offset in logical pixels.
		long long GetScrollOffset() const;
		/// Sets the current vertical scroll offset in logical pixels.
		/// The value is always clamped to the valid content range.
		void SetScrollOffset(long long offset, bool notify = true);
		/// Returns the current scroll position using the framework SIZE contract.
		SIZE GetScrollPos() const override;
		/// Returns the scrollable range using the framework SIZE contract.
		SIZE GetScrollRange() const override;
		/// Sets the scroll position using the framework SIZE contract.
		void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true) override;
		/// Scrolls upward by one logical item.
		void LineUp(bool bScroolVisible = true) override;
		/// Scrolls downward by one logical item.
		void LineDown(bool bScroolVisible = true) override;
		/// Scrolls upward by one viewport.
		void PageUp() override;
		/// Scrolls downward by one viewport.
		void PageDown() override;
		/// Scrolls to the first item.
		void HomeUp() override;
		/// Scrolls to the last item.
		void EndDown() override;

		/// Updates the control rectangle and re-realizes visible pooled items.
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		/// Attaches the control to a paint manager and refreshes scrollbar and visible item state.
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;
		/// Handles mouse, keyboard and wheel events for selection, hot tracking and scrolling.
		void DoEvent(TEventUI& event) override;
		/// Paints the control background, item state backgrounds, realized child controls and scrollbar.
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/// Parses XML attributes that configure item count, height mode, overscan, selection cancel and item state colors.
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue) override;
		/// Clears the realized item pool owned by the virtual list.
		void RemoveAll(bool bChildDelayed = true) override;

	private:
		struct RealizedItem
		{
			CControlUI* control = nullptr;
			ItemIndex index = 0;
			RECT rect = {};
			bool active = false;
		};

		RECT GetViewRect() const;
		long long GetViewportHeight() const;
		long long GetContentHeight() const;
		long long GetMaxScrollOffset() const;
		long long ClampScrollOffset(long long offset) const;
		int GetScrollbarRange() const;
		int OffsetToScrollbarPos(long long offset) const;
		long long ScrollbarPosToOffset(int pos) const;
		void ConfigureScrollBar();
		void ApplyVariableItemHeights(std::vector<int>&& heights);
		void FinalizeDataChange(bool notifySelection);
		void SetHotItem(ItemIndex index, bool hot);
		void UpdateHotItemFromPoint(POINT pt);
		DWORD GetItemDrawColor(ItemIndex index) const;
		bool IsIndexRealized(ItemIndex index) const;

		void RebuildPrefixHeights();
		ItemIndex FindItemByOffset(long long offset) const;
		long long GetItemTop(ItemIndex index) const;
		long long GetItemBottom(ItemIndex index) const;
		void UpdateScrollBar();
		void RealizeVisibleItems();
		void EnsurePoolSize(size_t count);
		CControlUI* CreatePoolItem();
		void BindRealizedItem(RealizedItem& item);
		int HitTestRealizedItem(POINT pt) const;
		void DispatchItemEvent(int realizedIndex, TEventUI& event, const ItemEventCallback& callback);

	private:
		ItemIndex m_itemCount;
		int m_fixedItemHeight;
		int m_overscanItemCount;
		bool m_useVariableHeights;
		std::vector<int> m_itemHeights;
		std::vector<long long> m_prefixHeights;
		long long m_contentHeight;
		long long m_scrollOffset;
		ItemIndex m_firstVisibleIndex;
		ItemIndex m_lastVisibleIndex;
		ItemIndex m_selectedIndex;
		bool m_hasSelection;
		ItemIndex m_hotIndex;
		bool m_hasHotItem;
		bool m_allowSelectionCancel;
		bool m_updatingScrollBar;
		DWORD m_dwItemBkColor;
		DWORD m_dwItemHotBkColor;
		DWORD m_dwItemSelectedBkColor;
		std::vector<RealizedItem> m_realizedItems;
		CreateItemCallback m_createItemCallback;
		BindItemCallback m_bindItemCallback;
		PaintItemCallback m_paintItemCallback;
		ItemEventCallback m_itemClickCallback;
		ItemEventCallback m_itemDoubleClickCallback;
	};
}
