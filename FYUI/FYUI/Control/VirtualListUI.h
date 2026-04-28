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

		CVirtualListUI();
		~CVirtualListUI() override;

		std::wstring_view GetClass() const override;
		LPVOID GetInterface(std::wstring_view pstrName) override;

		void SetItemCount(ItemIndex count);
		ItemIndex GetItemCount() const;

		void SetFixedItemHeight(int height);
		int GetFixedItemHeight() const;
		bool IsFixedHeightMode() const;

		void SetItemHeights(std::vector<int> heights);
		void SetItemHeights(const int* heights, size_t count);
		void ClearItemHeights();
		bool IsVariableHeightMode() const;
		int GetItemHeight(ItemIndex index) const;

		void SetOverscanItemCount(int count);
		int GetOverscanItemCount() const;

		void SetCreateItemCallback(CreateItemCallback callback);
		void SetBindItemCallback(BindItemCallback callback);
		void SetPaintItemCallback(PaintItemCallback callback);
		void SetItemClickCallback(ItemEventCallback callback);
		void SetItemDoubleClickCallback(ItemEventCallback callback);

		void Refresh();
		void RefreshItem(ItemIndex index);
		void EnsureVisible(ItemIndex index, bool alignTop = false);

		void GetDisplayCollection(std::vector<ItemIndex>& collection) const;
		void GetDisplayCollection(std::vector<int>& collection) const;

		bool SelectItem(ItemIndex index, bool takeFocus = false, bool notify = true);
		void ClearSelection(bool notify = true);
		bool HasSelection() const;
		ItemIndex GetSelectedIndex() const;
		bool IsItemSelected(ItemIndex index) const;

		long long GetScrollOffset() const;
		void SetScrollOffset(long long offset, bool notify = true);
		SIZE GetScrollPos() const override;
		SIZE GetScrollRange() const override;
		void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true) override;
		void LineUp(bool bScroolVisible = true) override;
		void LineDown(bool bScroolVisible = true) override;
		void PageUp() override;
		void PageDown() override;
		void HomeUp() override;
		void EndDown() override;

		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		void DoEvent(TEventUI& event) override;
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue) override;
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
		bool m_updatingScrollBar;
		std::vector<RealizedItem> m_realizedItems;
		CreateItemCallback m_createItemCallback;
		BindItemCallback m_bindItemCallback;
		PaintItemCallback m_paintItemCallback;
		ItemEventCallback m_itemClickCallback;
		ItemEventCallback m_itemDoubleClickCallback;
	};
}
