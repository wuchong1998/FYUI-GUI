#include "pch.h"
#include "VirtualListUI.h"
#include "UILabel.h"

#include <algorithm>
#include <limits>

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CVirtualListUI)

	namespace
	{
		constexpr long long kMaxScrollBarRange = static_cast<long long>((std::numeric_limits<int>::max)() - 1);
		constexpr std::uint64_t kInvalidVirtualIndex = (std::numeric_limits<std::uint64_t>::max)();

		int ClampPositiveHeight(int height, int fallback)
		{
			return (std::max)(1, height > 0 ? height : fallback);
		}

		int ClampToInt(long long value)
		{
			if (value <= 0) return 0;
			if (value >= kMaxScrollBarRange) return static_cast<int>(kMaxScrollBarRange);
			return static_cast<int>(value);
		}

		size_t ClampItemCountToSize(CVirtualListUI::ItemIndex count)
		{
			const auto maxSize = static_cast<CVirtualListUI::ItemIndex>((std::numeric_limits<size_t>::max)());
			return static_cast<size_t>((std::min)(count, maxSize));
		}
	}

	CVirtualListUI::CVirtualListUI()
		: m_itemCount(0),
		m_fixedItemHeight(24),
		m_overscanItemCount(4),
		m_useVariableHeights(false),
		m_contentHeight(0),
		m_scrollOffset(0),
		m_firstVisibleIndex(0),
		m_lastVisibleIndex(0),
		m_selectedIndex(kInvalidVirtualIndex),
		m_hasSelection(false),
		m_hotIndex(kInvalidVirtualIndex),
		m_hasHotItem(false),
		m_allowSelectionCancel(false),
		m_updatingScrollBar(false),
		m_dwItemBkColor(0),
		m_dwItemHotBkColor(0xFFEAF2FA),
		m_dwItemSelectedBkColor(0xFFDCEBFA)
	{
		SetMouseChildEnabled(true);
		EnableScrollBar(true, false);
	}

	CVirtualListUI::~CVirtualListUI()
	{
	}

	std::wstring_view CVirtualListUI::GetClass() const
	{
		return L"VirtualList";
	}

	LPVOID CVirtualListUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::EqualsNoCase(pstrName, L"VirtualList") ||
			StringUtil::EqualsNoCase(pstrName, L"VirtualListUI")) {
			return static_cast<CVirtualListUI*>(this);
		}
		return CContainerUI::GetInterface(pstrName);
	}

	void CVirtualListUI::SetItemCount(ItemIndex count)
	{
		if (m_useVariableHeights) {
			m_itemHeights.resize(ClampItemCountToSize(count), m_fixedItemHeight);
			count = static_cast<ItemIndex>(m_itemHeights.size());
		}
		if (m_itemCount == count) {
			FinalizeDataChange(false);
			return;
		}

		m_itemCount = count;
		if (m_useVariableHeights) {
			RebuildPrefixHeights();
		}
		else {
			m_contentHeight = static_cast<long long>(m_itemCount) * ClampPositiveHeight(m_fixedItemHeight, 24);
		}
		FinalizeDataChange(false);
	}

	CVirtualListUI::ItemIndex CVirtualListUI::GetItemCount() const
	{
		return m_itemCount;
	}

	void CVirtualListUI::SetFixedItemHeight(int height)
	{
		m_fixedItemHeight = ClampPositiveHeight(height, 24);
		m_useVariableHeights = false;
		m_itemHeights.clear();
		m_prefixHeights.clear();
		m_contentHeight = static_cast<long long>(m_itemCount) * m_fixedItemHeight;
		FinalizeDataChange(false);
	}

	int CVirtualListUI::GetFixedItemHeight() const
	{
		return m_fixedItemHeight;
	}

	bool CVirtualListUI::IsFixedHeightMode() const
	{
		return !m_useVariableHeights;
	}

	void CVirtualListUI::SetItemHeights(const std::vector<int>& heights)
	{
		ApplyVariableItemHeights(std::vector<int>(heights.begin(), heights.end()));
	}

	void CVirtualListUI::SetItemHeights(std::vector<int>&& heights)
	{
		ApplyVariableItemHeights(std::move(heights));
	}

	bool CVirtualListUI::SetItemHeight(ItemIndex index, int height)
	{
		if (index >= m_itemCount) return false;
		if (!m_useVariableHeights) {
			m_itemHeights.assign(ClampItemCountToSize(m_itemCount), m_fixedItemHeight);
			m_useVariableHeights = true;
		}

		const size_t itemIndex = static_cast<size_t>(index);
		if (itemIndex >= m_itemHeights.size()) return false;
		m_itemHeights[itemIndex] = ClampPositiveHeight(height, m_fixedItemHeight);
		RebuildPrefixHeights();
		FinalizeDataChange(false);
		return true;
	}

	void CVirtualListUI::SetItemHeights(const int* heights, size_t count)
	{
		if (heights == nullptr || count == 0) {
			ClearItemHeights();
			return;
		}
		std::vector<int> values(heights, heights + count);
		ApplyVariableItemHeights(std::move(values));
	}

	void CVirtualListUI::ClearItemHeights()
	{
		m_useVariableHeights = false;
		m_itemHeights.clear();
		m_prefixHeights.clear();
		m_contentHeight = static_cast<long long>(m_itemCount) * ClampPositiveHeight(m_fixedItemHeight, 24);
		FinalizeDataChange(false);
	}

	bool CVirtualListUI::IsVariableHeightMode() const
	{
		return m_useVariableHeights;
	}

	int CVirtualListUI::GetItemHeight(ItemIndex index) const
	{
		if (index >= m_itemCount) return 0;
		if (!m_useVariableHeights) return m_fixedItemHeight;
		return ClampPositiveHeight(m_itemHeights[static_cast<size_t>(index)], m_fixedItemHeight);
	}

	void CVirtualListUI::SetOverscanItemCount(int count)
	{
		m_overscanItemCount = (std::max)(0, count);
		Refresh();
	}

	int CVirtualListUI::GetOverscanItemCount() const
	{
		return m_overscanItemCount;
	}

	void CVirtualListUI::SetItemBkColor(DWORD dwColor)
	{
		m_dwItemBkColor = dwColor;
		Invalidate();
	}

	DWORD CVirtualListUI::GetItemBkColor() const
	{
		return m_dwItemBkColor;
	}

	void CVirtualListUI::SetItemHotBkColor(DWORD dwColor)
	{
		m_dwItemHotBkColor = dwColor;
		Invalidate();
	}

	DWORD CVirtualListUI::GetItemHotBkColor() const
	{
		return m_dwItemHotBkColor;
	}

	void CVirtualListUI::SetItemSelectedBkColor(DWORD dwColor)
	{
		m_dwItemSelectedBkColor = dwColor;
		Invalidate();
	}

	DWORD CVirtualListUI::GetItemSelectedBkColor() const
	{
		return m_dwItemSelectedBkColor;
	}

	void CVirtualListUI::SetAllowSelectionCancel(bool allow)
	{
		m_allowSelectionCancel = allow;
	}

	bool CVirtualListUI::IsAllowSelectionCancel() const
	{
		return m_allowSelectionCancel;
	}

	void CVirtualListUI::SetCreateItemCallback(CreateItemCallback callback)
	{
		m_createItemCallback = std::move(callback);
		RemoveAll();
		Refresh();
	}

	void CVirtualListUI::SetBindItemCallback(BindItemCallback callback)
	{
		m_bindItemCallback = std::move(callback);
		Refresh();
	}

	void CVirtualListUI::SetPaintItemCallback(PaintItemCallback callback)
	{
		m_paintItemCallback = std::move(callback);
		Invalidate();
	}

	void CVirtualListUI::SetItemClickCallback(ItemEventCallback callback)
	{
		m_itemClickCallback = std::move(callback);
	}

	void CVirtualListUI::SetItemDoubleClickCallback(ItemEventCallback callback)
	{
		m_itemDoubleClickCallback = std::move(callback);
	}

	void CVirtualListUI::Refresh()
	{
		UpdateScrollBar();
		RealizeVisibleItems();
		Invalidate();
	}

	void CVirtualListUI::RefreshItem(ItemIndex index)
	{
		for (RealizedItem& item : m_realizedItems) {
			if (item.active && item.index == index) {
				BindRealizedItem(item);
				Invalidate();
				return;
			}
		}
	}

	void CVirtualListUI::EnsureVisible(ItemIndex index, bool alignTop)
	{
		if (index >= m_itemCount) return;
		const long long top = GetItemTop(index);
		const long long bottom = GetItemBottom(index);
		const long long viewHeight = GetViewportHeight();
		long long target = m_scrollOffset;
		if (alignTop || top < m_scrollOffset) target = top;
		else if (bottom > m_scrollOffset + viewHeight) target = bottom - viewHeight;
		SetScrollOffset(target);
	}

	void CVirtualListUI::GetDisplayCollection(std::vector<ItemIndex>& collection) const
	{
		collection.clear();
		for (const RealizedItem& item : m_realizedItems) {
			if (item.active) collection.push_back(item.index);
		}
	}

	void CVirtualListUI::GetDisplayCollection(std::vector<int>& collection) const
	{
		collection.clear();
		for (const RealizedItem& item : m_realizedItems) {
			if (item.active && item.index <= static_cast<ItemIndex>((std::numeric_limits<int>::max)())) {
				collection.push_back(static_cast<int>(item.index));
			}
		}
	}

	bool CVirtualListUI::SelectItem(ItemIndex index, bool takeFocus, bool notify)
	{
		if (index >= m_itemCount) return false;
		const bool selectionChanged = !m_hasSelection || m_selectedIndex != index;
		const bool hadSelection = m_hasSelection;
		const ItemIndex previousSelection = m_selectedIndex;
		m_selectedIndex = index;
		m_hasSelection = true;
		EnsureVisible(index);
		if (takeFocus) SetFocus();
		if (selectionChanged) {
			if (notify && m_pManager != nullptr) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, static_cast<WPARAM>(index));
			}
			InvalidateSelectionChange(previousSelection, hadSelection);
		}
		return true;
	}

	void CVirtualListUI::ClearSelection(bool notify)
	{
		if (!m_hasSelection) return;
		const ItemIndex previousSelection = m_selectedIndex;
		m_selectedIndex = kInvalidVirtualIndex;
		m_hasSelection = false;
		if (notify && m_pManager != nullptr) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, static_cast<WPARAM>(-1));
		}
		InvalidateSelectionChange(previousSelection, true);
	}

	bool CVirtualListUI::HasSelection() const
	{
		return m_hasSelection;
	}

	bool CVirtualListUI::UnselectItem(ItemIndex index, bool notify)
	{
		if (!m_hasSelection || m_selectedIndex != index) return false;
		ClearSelection(notify);
		return true;
	}

	CVirtualListUI::ItemIndex CVirtualListUI::GetSelectedIndex() const
	{
		return m_selectedIndex;
	}

	bool CVirtualListUI::IsItemSelected(ItemIndex index) const
	{
		return m_hasSelection && m_selectedIndex == index;
	}

	bool CVirtualListUI::HasHotItem() const
	{
		return m_hasHotItem;
	}

	CVirtualListUI::ItemIndex CVirtualListUI::GetHotItemIndex() const
	{
		return m_hotIndex;
	}

	long long CVirtualListUI::GetScrollOffset() const
	{
		return m_scrollOffset;
	}

	void CVirtualListUI::SetScrollOffset(long long offset, bool notify)
	{
		const long long newOffset = ClampScrollOffset(offset);
		if (m_scrollOffset == newOffset) {
			UpdateScrollBar();
			return;
		}
		m_scrollOffset = newOffset;
		UpdateScrollBar();
		RealizeVisibleItems();
		if (notify && m_pManager != nullptr) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL, 0, static_cast<LPARAM>(m_scrollOffset));
		}
		Invalidate();
	}

	SIZE CVirtualListUI::GetScrollPos() const
	{
		return CDuiSize(0, ClampToInt(m_scrollOffset));
	}

	SIZE CVirtualListUI::GetScrollRange() const
	{
		return CDuiSize(0, ClampToInt(GetMaxScrollOffset()));
	}

	void CVirtualListUI::SetScrollPos(SIZE szPos, bool bMsg, bool)
	{
		if (m_updatingScrollBar) return;
		const long long target = GetMaxScrollOffset() > kMaxScrollBarRange
			? ScrollbarPosToOffset(szPos.cy)
			: static_cast<long long>(szPos.cy);
		SetScrollOffset(target, bMsg);
	}

	void CVirtualListUI::LineUp(bool)
	{
		SetScrollOffset(m_scrollOffset - GetItemHeight(FindItemByOffset(m_scrollOffset)));
	}

	void CVirtualListUI::LineDown(bool)
	{
		SetScrollOffset(m_scrollOffset + GetItemHeight(FindItemByOffset(m_scrollOffset)));
	}

	void CVirtualListUI::PageUp()
	{
		SetScrollOffset(m_scrollOffset - GetViewportHeight());
	}

	void CVirtualListUI::PageDown()
	{
		SetScrollOffset(m_scrollOffset + GetViewportHeight());
	}

	void CVirtualListUI::HomeUp()
	{
		SetScrollOffset(0);
	}

	void CVirtualListUI::EndDown()
	{
		SetScrollOffset(GetMaxScrollOffset());
	}

	void CVirtualListUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		ConfigureScrollBar();
		UpdateScrollBar();
		RealizeVisibleItems();
	}

	void CVirtualListUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CContainerUI::SetManager(pManager, pParent, bInit);
		ConfigureScrollBar();
		UpdateScrollBar();
		RealizeVisibleItems();
	}

	void CVirtualListUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pParent != nullptr) m_pParent->DoEvent(event);
			else CContainerUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_MOUSEMOVE || event.Type == UIEVENT_MOUSEENTER) {
			UpdateHotItemFromPoint(event.ptMouse);
		}
		else if (event.Type == UIEVENT_MOUSELEAVE) {
			SetHotItem(kInvalidVirtualIndex, false);
		}

		if (event.Type == UIEVENT_SCROLLWHEEL) {
			const WORD code = LOWORD(event.wParam);
			if (code == SB_LINEUP) LineUp();
			else if (code == SB_LINEDOWN) LineDown();
			else CContainerUI::DoEvent(event);
			return;
		}
		if (event.Type == UIEVENT_KEYDOWN) {
			switch (event.chKey) {
			case VK_UP: LineUp(); return;
			case VK_DOWN: LineDown(); return;
			case VK_PRIOR: PageUp(); return;
			case VK_NEXT: PageDown(); return;
			case VK_HOME: HomeUp(); return;
			case VK_END: EndDown(); return;
			default: break;
			}
		}

		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK) {
			const int realized = HitTestRealizedItem(event.ptMouse);
			if (realized >= 0) {
				const ItemIndex index = m_realizedItems[static_cast<size_t>(realized)].index;
				if (event.Type == UIEVENT_BUTTONDOWN && m_allowSelectionCancel && IsItemSelected(index)) {
					ClearSelection(true);
				}
				else {
					SelectItem(index, true, true);
				}

				if (m_pManager != nullptr) {
					m_pManager->SendNotify(
						this,
						event.Type == UIEVENT_DBLCLICK ? DUI_MSGTYPE_ITEMDBCLICK : DUI_MSGTYPE_ITEMCLICK,
						static_cast<WPARAM>(index));
				}
				DispatchItemEvent(realized, event, event.Type == UIEVENT_DBLCLICK ? m_itemDoubleClickCallback : m_itemClickCallback);
				return;
			}

			if (event.Type == UIEVENT_BUTTONDOWN && m_allowSelectionCancel && m_hasSelection) {
				ClearSelection(true);
			}
		}

		CContainerUI::DoEvent(event);
	}

	bool CVirtualListUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		RECT rcPaint = renderContext.GetPaintRect();
		RECT rcTemp = {};
		if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) return true;

		CControlUI::DoPaint(renderContext, pStopControl);

		RECT rcView = GetViewRect();
		::IntersectRect(&rcView, &rcView, &m_rcItem);
		if (::IntersectRect(&rcTemp, &rcPaint, &rcView)) {
			CRenderClip clip;
			CRenderClip::GenerateClip(renderContext, rcTemp, clip);
			for (RealizedItem& item : m_realizedItems) {
				if (!item.active || item.control == nullptr || !item.control->IsVisible()) continue;

				const DWORD drawColor = GetItemDrawColor(item.index);
				if (drawColor != 0) {
					CRenderEngine::DrawColor(renderContext, item.rect, GetAdjustColor(drawColor));
				}

				if (m_paintItemCallback) m_paintItemCallback(this, renderContext, item.index, item.rect);
				item.control->Paint(renderContext, pStopControl);
			}
		}

		if (m_pVerticalScrollBar != nullptr && m_pVerticalScrollBar->IsVisible() && m_pVerticalScrollBar != pStopControl) {
			m_pVerticalScrollBar->Paint(renderContext, pStopControl);
		}
		return true;
	}

	void CVirtualListUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		const std::wstring_view name = StringUtil::TrimView(pstrName);
		if (StringUtil::EqualsNoCase(name, L"itemcount")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetItemCount(static_cast<ItemIndex>((std::max)(0, value)));
		}
		else if (StringUtil::EqualsNoCase(name, L"itemheight") ||
			StringUtil::EqualsNoCase(name, L"fixeditemheight") ||
			StringUtil::EqualsNoCase(name, L"elementheight")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetFixedItemHeight(value);
		}
		else if (StringUtil::EqualsNoCase(name, L"overscan") ||
			StringUtil::EqualsNoCase(name, L"overscanitemcount")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetOverscanItemCount(value);
		}
		else if (StringUtil::EqualsNoCase(name, L"itembkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValue, color)) SetItemBkColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"itemhotbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValue, color)) SetItemHotBkColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"itemselectedbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValue, color)) SetItemSelectedBkColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"allowselectioncancel") ||
			StringUtil::EqualsNoCase(name, L"allowcancelselection")) {
			SetAllowSelectionCancel(StringUtil::ParseBool(pstrValue));
		}
		else if (StringUtil::EqualsNoCase(name, L"vscrollbar")) {
			EnableScrollBar(StringUtil::ParseBool(pstrValue), false);
			ConfigureScrollBar();
			UpdateScrollBar();
		}
		else {
			CContainerUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CVirtualListUI::RemoveAll(bool bChildDelayed)
	{
		m_realizedItems.clear();
		m_firstVisibleIndex = 0;
		m_lastVisibleIndex = 0;
		SetHotItem(kInvalidVirtualIndex, false);
		CContainerUI::RemoveAll(bChildDelayed);
	}

	RECT CVirtualListUI::GetViewRect() const
	{
		RECT rc = m_rcItem;
		RECT inset = GetInset();
		rc.left += inset.left;
		rc.top += inset.top;
		rc.right -= inset.right;
		rc.bottom -= inset.bottom;
		if (m_pVerticalScrollBar != nullptr && m_pVerticalScrollBar->IsVisible()) rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if (rc.right < rc.left) rc.right = rc.left;
		if (rc.bottom < rc.top) rc.bottom = rc.top;
		return rc;
	}

	long long CVirtualListUI::GetViewportHeight() const
	{
		const RECT rc = GetViewRect();
		return (std::max<long long>)(0, static_cast<long long>(rc.bottom - rc.top));
	}

	long long CVirtualListUI::GetContentHeight() const
	{
		return m_contentHeight;
	}

	long long CVirtualListUI::GetMaxScrollOffset() const
	{
		return (std::max<long long>)(0, GetContentHeight() - GetViewportHeight());
	}

	long long CVirtualListUI::ClampScrollOffset(long long offset) const
	{
		return (std::max<long long>)(0, (std::min)(offset, GetMaxScrollOffset()));
	}

	int CVirtualListUI::GetScrollbarRange() const
	{
		return ClampToInt(GetMaxScrollOffset());
	}

	int CVirtualListUI::OffsetToScrollbarPos(long long offset) const
	{
		const long long maxOffset = GetMaxScrollOffset();
		if (maxOffset <= 0) return 0;
		if (maxOffset <= kMaxScrollBarRange) return ClampToInt(offset);
		return static_cast<int>((offset * kMaxScrollBarRange) / maxOffset);
	}

	long long CVirtualListUI::ScrollbarPosToOffset(int pos) const
	{
		const long long maxOffset = GetMaxScrollOffset();
		if (maxOffset <= 0) return 0;
		if (maxOffset <= kMaxScrollBarRange) return ClampScrollOffset(pos);
		return ClampScrollOffset((static_cast<long long>(pos) * maxOffset) / kMaxScrollBarRange);
	}

	void CVirtualListUI::ConfigureScrollBar()
	{
		if (m_pVerticalScrollBar == nullptr) return;
		m_pVerticalScrollBar->SetOwner(this);
		m_pVerticalScrollBar->SetHorizontal(false);
		if (m_pManager != nullptr) {
			std::wstring_view style = m_sVerticalScrollBarStyle;
			if (!style.empty()) {
				const std::wstring_view namedStyle = m_pManager->GetStyle(style);
				m_pVerticalScrollBar->ApplyAttributeList(namedStyle.empty() ? style : namedStyle);
			}
			else {
				const std::wstring_view defaultStyle = m_pManager->GetDefaultAttributeList(L"VScrollBar");
				style = defaultStyle.empty() ? m_pManager->GetDefaultAttributeList(L"ScrollBar") : defaultStyle;
				if (!style.empty()) {
					const std::wstring_view namedStyle = m_pManager->GetStyle(style);
					m_pVerticalScrollBar->ApplyAttributeList(namedStyle.empty() ? style : namedStyle);
				}
			}
			m_pVerticalScrollBar->SetShow(m_bShowScrollbar);
		}
	}

	void CVirtualListUI::ApplyVariableItemHeights(std::vector<int>&& heights)
	{
		m_itemHeights = std::move(heights);
		for (int& height : m_itemHeights) {
			height = ClampPositiveHeight(height, m_fixedItemHeight);
		}
		m_useVariableHeights = true;
		m_itemCount = static_cast<ItemIndex>(m_itemHeights.size());
		RebuildPrefixHeights();
		FinalizeDataChange(false);
	}

	void CVirtualListUI::FinalizeDataChange(bool notifySelection)
	{
		bool selectionChanged = false;
		if (!m_hasSelection) {
			m_selectedIndex = kInvalidVirtualIndex;
		}
		else if (m_selectedIndex >= m_itemCount) {
			m_hasSelection = false;
			m_selectedIndex = kInvalidVirtualIndex;
			selectionChanged = true;
		}

		if (m_itemCount == 0 || (m_hasHotItem && m_hotIndex >= m_itemCount)) {
			m_hasHotItem = false;
			m_hotIndex = kInvalidVirtualIndex;
		}

		m_scrollOffset = ClampScrollOffset(m_scrollOffset);
		m_firstVisibleIndex = m_itemCount == 0 ? 0 : (std::min)(m_firstVisibleIndex, m_itemCount - 1);
		m_lastVisibleIndex = m_itemCount == 0 ? 0 : (std::min)(m_lastVisibleIndex, m_itemCount - 1);

		UpdateScrollBar();
		RealizeVisibleItems();
		if (selectionChanged && notifySelection && m_pManager != nullptr) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_ITEMSELECT, static_cast<WPARAM>(-1));
		}
		Invalidate();
	}

	void CVirtualListUI::InvalidateRealizedItem(ItemIndex index)
	{
		if (index == kInvalidVirtualIndex || m_pManager == nullptr) return;
		for (const RealizedItem& item : m_realizedItems) {
			if (!item.active || item.index != index) continue;
			RECT invalidateRc = item.rect;
			if (!::IsRectEmpty(&invalidateRc)) {
				m_pManager->Invalidate(invalidateRc);
			}
			return;
		}
	}

	void CVirtualListUI::InvalidateSelectionChange(ItemIndex previousIndex, bool hadPreviousSelection)
	{
		bool invalidated = false;
		if (hadPreviousSelection) {
			InvalidateRealizedItem(previousIndex);
			invalidated = IsIndexRealized(previousIndex);
		}
		if (m_hasSelection) {
			InvalidateRealizedItem(m_selectedIndex);
			invalidated = invalidated || IsIndexRealized(m_selectedIndex);
		}
		if (!invalidated) {
			Invalidate();
		}
	}

	void CVirtualListUI::SetHotItem(ItemIndex index, bool hot)
	{
		if (!hot) index = kInvalidVirtualIndex;
		if (m_hasHotItem == hot && m_hotIndex == index) return;
		const bool hadHotItem = m_hasHotItem;
		const ItemIndex previousHotIndex = m_hotIndex;
		m_hasHotItem = hot;
		m_hotIndex = index;
		bool invalidated = false;
		if (hadHotItem) {
			InvalidateRealizedItem(previousHotIndex);
			invalidated = IsIndexRealized(previousHotIndex);
		}
		if (m_hasHotItem) {
			InvalidateRealizedItem(m_hotIndex);
			invalidated = invalidated || IsIndexRealized(m_hotIndex);
		}
		if (!invalidated) {
			Invalidate();
		}
	}

	void CVirtualListUI::UpdateHotItemFromPoint(POINT pt)
	{
		const RECT rcView = GetViewRect();
		if (!::PtInRect(&rcView, pt)) {
			SetHotItem(kInvalidVirtualIndex, false);
			return;
		}

		const int realized = HitTestRealizedItem(pt);
		if (realized < 0) {
			SetHotItem(kInvalidVirtualIndex, false);
			return;
		}

		SetHotItem(m_realizedItems[static_cast<size_t>(realized)].index, true);
	}

	DWORD CVirtualListUI::GetItemDrawColor(ItemIndex index) const
	{
		if (m_hasSelection && m_selectedIndex == index && m_dwItemSelectedBkColor != 0) return m_dwItemSelectedBkColor;
		if (m_hasHotItem && m_hotIndex == index && m_dwItemHotBkColor != 0) return m_dwItemHotBkColor;
		return m_dwItemBkColor;
	}

	bool CVirtualListUI::IsIndexRealized(ItemIndex index) const
	{
		for (const RealizedItem& item : m_realizedItems) {
			if (item.active && item.index == index) return true;
		}
		return false;
	}

	void CVirtualListUI::RebuildPrefixHeights()
	{
		m_prefixHeights.assign(m_itemHeights.size() + 1, 0);
		long long total = 0;
		for (size_t i = 0; i < m_itemHeights.size(); ++i) {
			total += ClampPositiveHeight(m_itemHeights[i], m_fixedItemHeight);
			m_prefixHeights[i + 1] = total;
		}
		m_contentHeight = total;
	}

	CVirtualListUI::ItemIndex CVirtualListUI::FindItemByOffset(long long offset) const
	{
		if (m_itemCount == 0) return 0;
		offset = ClampScrollOffset(offset);
		if (!m_useVariableHeights) {
			return (std::min<ItemIndex>)(m_itemCount - 1, static_cast<ItemIndex>(offset / ClampPositiveHeight(m_fixedItemHeight, 24)));
		}
		auto it = std::upper_bound(m_prefixHeights.begin(), m_prefixHeights.end(), offset);
		if (it == m_prefixHeights.begin()) return 0;
		const size_t index = static_cast<size_t>((it - m_prefixHeights.begin()) - 1);
		return (std::min<ItemIndex>)(m_itemCount - 1, static_cast<ItemIndex>(index));
	}

	long long CVirtualListUI::GetItemTop(ItemIndex index) const
	{
		if (index >= m_itemCount) return GetContentHeight();
		if (!m_useVariableHeights) return static_cast<long long>(index) * ClampPositiveHeight(m_fixedItemHeight, 24);
		return m_prefixHeights[static_cast<size_t>(index)];
	}

	long long CVirtualListUI::GetItemBottom(ItemIndex index) const
	{
		if (index >= m_itemCount) return GetContentHeight();
		return GetItemTop(index) + GetItemHeight(index);
	}

	void CVirtualListUI::UpdateScrollBar()
	{
		if (m_pVerticalScrollBar == nullptr) return;
		const int range = GetScrollbarRange();
		const bool visible = range > 0 && m_bShowScrollbar;
		m_updatingScrollBar = true;
		m_pVerticalScrollBar->SetVisible(visible);
		m_pVerticalScrollBar->SetScrollRange(range);
		m_pVerticalScrollBar->SetScrollPos(OffsetToScrollbarPos(m_scrollOffset));
		m_pVerticalScrollBar->SetLineSize((std::max)(1, m_fixedItemHeight));
		if (visible) {
			RECT rcBar = m_rcItem;
			RECT inset = GetInset();
			rcBar.left = rcBar.right - m_pVerticalScrollBar->GetFixedWidth() - inset.right;
			rcBar.right -= inset.right;
			rcBar.top += inset.top;
			rcBar.bottom -= inset.bottom;
			m_pVerticalScrollBar->SetPos(rcBar, false);
		}
		m_updatingScrollBar = false;
	}

	void CVirtualListUI::RealizeVisibleItems()
	{
		const RECT rcView = GetViewRect();
		const long long viewHeight = GetViewportHeight();
		if (m_itemCount == 0 || viewHeight <= 0) {
			m_firstVisibleIndex = 0;
			m_lastVisibleIndex = 0;
			for (RealizedItem& item : m_realizedItems) {
				item.active = false;
				if (item.control != nullptr) item.control->SetVisible(false, false);
			}
			if (m_hasHotItem && !IsIndexRealized(m_hotIndex)) {
				m_hasHotItem = false;
				m_hotIndex = kInvalidVirtualIndex;
			}
			return;
		}

		const ItemIndex firstVisible = FindItemByOffset(m_scrollOffset);
		const size_t leadingOverscan = (std::min)(static_cast<size_t>(m_overscanItemCount), static_cast<size_t>(firstVisible));
		const ItemIndex first = firstVisible - static_cast<ItemIndex>(leadingOverscan);
		const long long visibleEnd = m_scrollOffset + viewHeight;

		ItemIndex index = first;
		long long itemTop = GetItemTop(index);
		size_t trailingOverscanRemaining = static_cast<size_t>(m_overscanItemCount);
		size_t needed = 0;
		while (index < m_itemCount) {
			if (itemTop >= visibleEnd) {
				if (trailingOverscanRemaining == 0) break;
				--trailingOverscanRemaining;
			}
			++needed;
			itemTop += GetItemHeight(index);
			++index;
		}
		EnsurePoolSize(needed);

		index = first;
		itemTop = GetItemTop(index);
		trailingOverscanRemaining = static_cast<size_t>(m_overscanItemCount);
		size_t slot = 0;
		m_firstVisibleIndex = first;
		m_lastVisibleIndex = first;
		while (slot < m_realizedItems.size() && index < m_itemCount) {
			if (itemTop >= visibleEnd) {
				if (trailingOverscanRemaining == 0) break;
				--trailingOverscanRemaining;
			}

			RealizedItem& item = m_realizedItems[slot];
			const int height = GetItemHeight(index);
			item.index = index;
			item.active = true;
			item.rect = {
				rcView.left,
				static_cast<LONG>(rcView.top + itemTop - m_scrollOffset),
				rcView.right,
				static_cast<LONG>(rcView.top + itemTop - m_scrollOffset + height)
			};
			BindRealizedItem(item);
			itemTop += height;
			m_lastVisibleIndex = index;
			++index;
			++slot;
		}

		for (; slot < m_realizedItems.size(); ++slot) {
			m_realizedItems[slot].active = false;
			if (m_realizedItems[slot].control != nullptr) m_realizedItems[slot].control->SetVisible(false, false);
		}

		if (m_hasHotItem && !IsIndexRealized(m_hotIndex)) {
			m_hasHotItem = false;
			m_hotIndex = kInvalidVirtualIndex;
		}
	}

	void CVirtualListUI::EnsurePoolSize(size_t count)
	{
		while (m_realizedItems.size() < count) {
			RealizedItem item;
			item.control = CreatePoolItem();
			if (item.control == nullptr) break;
			CContainerUI::Add(item.control);
			m_realizedItems.push_back(item);
		}
	}

	CControlUI* CVirtualListUI::CreatePoolItem()
	{
		if (m_createItemCallback) return m_createItemCallback(this);

		CLabelUI* label = new CLabelUI();
		label->SetTextPadding(CDuiRect(8, 0, 8, 0));
		label->SetTextColor(0xFF243042);
		label->SetBkColor(0x00FFFFFF);
		return label;
	}

	void CVirtualListUI::BindRealizedItem(RealizedItem& item)
	{
		if (item.control == nullptr) return;
		item.control->SetVisible(true, false);
		item.control->SetFixedHeight(static_cast<int>(item.rect.bottom - item.rect.top), false);
		item.control->SetPos(item.rect, false);
		item.control->SetUserData(std::to_wstring(item.index));
		if (m_bindItemCallback) {
			m_bindItemCallback(this, item.control, item.index);
		}
		else if (CLabelUI* label = dynamic_cast<CLabelUI*>(item.control)) {
			label->SetText(L"Virtual item " + std::to_wstring(item.index));
		}
	}

	int CVirtualListUI::HitTestRealizedItem(POINT pt) const
	{
		for (size_t i = 0; i < m_realizedItems.size(); ++i) {
			const RealizedItem& item = m_realizedItems[i];
			if (!item.active) continue;
			if (pt.x >= item.rect.left && pt.x < item.rect.right && pt.y >= item.rect.top && pt.y < item.rect.bottom) {
				return static_cast<int>(i);
			}
		}
		return -1;
	}

	void CVirtualListUI::DispatchItemEvent(int realizedIndex, TEventUI& event, const ItemEventCallback& callback)
	{
		if (realizedIndex < 0 || static_cast<size_t>(realizedIndex) >= m_realizedItems.size()) return;
		if (callback) callback(this, m_realizedItems[static_cast<size_t>(realizedIndex)].index, event);
	}
}
