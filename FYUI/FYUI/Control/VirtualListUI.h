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

		/**
		 * @brief 创建一个空的虚拟列表控件
		 * @details 初始化虚拟列表的内部状态，并默认创建和启用内部竖向滚动条。
		 */
		CVirtualListUI();
		~CVirtualListUI() override;

		/**
		 * @brief 获取控件的 XML 类名
		 * @return std::wstring_view 返回 XML 构建器使用的类名 VirtualList
		 */
		std::wstring_view GetClass() const override;
		/**
		 * @brief 获取控件接口
		 * @details 当外部请求 VirtualList 或 VirtualListUI 接口时，返回当前对象指针。
		 * @param pstrName [in] 接口名称
		 * @return LPVOID 返回匹配的接口指针，未匹配时返回基类接口
		 */
		LPVOID GetInterface(std::wstring_view pstrName) override;

		/**
		 * @brief 设置虚拟列表的逻辑子项数量
		 * @details 固定高度模式下直接修改子项数量；可变高度模式下会同步调整内部高度表大小。
		 * 数量变化后会自动校正当前选中项、hot 状态和滚动偏移，保证控件稳定运行。
		 * @param count [in] 子项数量
		 */
		void SetItemCount(ItemIndex count);
		/**
		 * @brief 获取当前虚拟列表的逻辑子项数量
		 * @return ItemIndex 返回当前子项数量
		 */
		ItemIndex GetItemCount() const;

		/**
		 * @brief 设置固定高度模式下每个子项的高度
		 * @details 调用后虚拟列表会切换为固定高度模式，并清空之前设置的逐项高度表。
		 * @param height [in] 固定高度
		 */
		void SetFixedItemHeight(int height);
		/**
		 * @brief 获取固定高度模式使用的子项高度
		 * @details 该值同时也是逐项高度模式下非法高度值的回退高度。
		 * @return int 返回固定高度值
		 */
		int GetFixedItemHeight() const;
		/**
		 * @brief 判断当前是否为固定高度模式
		 * @return bool 固定高度模式返回 true，否则返回 false
		 */
		bool IsFixedHeightMode() const;

		/**
		 * @brief 按拷贝方式设置逐项高度表
		 * @details 控件会切换到逐项高度模式，虚拟列表的子项数量会自动变为 heights.size()。
		 * @param heights [in] 每个子项对应的高度数组
		 */
		void SetItemHeights(const std::vector<int>& heights);
		/**
		 * @brief 按移动方式设置逐项高度表
		 * @details 控件会切换到逐项高度模式，适用于外部可以转移 vector 所有权的场景，
		 * 这样可以避免一次额外的拷贝，提高大数据量场景下的效率。
		 * @param heights [in] 每个子项对应的高度数组
		 */
		void SetItemHeights(std::vector<int>&& heights);
		/**
		 * @brief 修改单个子项的高度
		 * @details 如果当前仍处于固定高度模式，会先以固定高度为默认值扩展出内部高度表，
		 * 然后再修改指定子项的高度。
		 * @param index [in] 子项下标
		 * @param height [in] 新的子项高度
		 * @return bool 修改成功返回 true，index 越界时返回 false
		 */
		bool SetItemHeight(ItemIndex index, int height);
		/**
		 * @brief 使用原始数组设置逐项高度表
		 * @details 控件会将外部数组拷贝到内部高度表，并切换到逐项高度模式。
		 * @param heights [in] 高度数组指针
		 * @param count [in] 高度数组数量
		 */
		void SetItemHeights(const int* heights, size_t count);
		/**
		 * @brief 清空逐项高度表
		 * @details 调用后控件会恢复为固定高度模式，但保持当前子项数量不变。
		 */
		void ClearItemHeights();
		/**
		 * @brief 判断当前是否为逐项高度模式
		 * @return bool 逐项高度模式返回 true，否则返回 false
		 */
		bool IsVariableHeightMode() const;
		/**
		 * @brief 获取指定子项的实际高度
		 * @details 固定高度模式下始终返回 GetFixedItemHeight()；
		 * 逐项高度模式下返回对应子项的高度。
		 * @param index [in] 子项下标
		 * @return int 返回子项高度，越界时返回 0
		 */
		int GetItemHeight(ItemIndex index) const;

		/**
		 * @brief 设置超前缓存的子项数量
		 * @details 控件会在可视区域之外额外创建一部分前后子项，减少滚动时的闪烁和重建频率。
		 * @param count [in] 额外缓存的子项数量
		 */
		void SetOverscanItemCount(int count);
		/**
		 * @brief 获取当前超前缓存的子项数量
		 * @return int 返回当前 overscan 数量
		 */
		int GetOverscanItemCount() const;

		/**
		 * @brief 设置子项默认背景色
		 * @details 在子项控件真正绘制内容前，由虚拟列表先绘制该背景色。
		 * @param dwColor [in] 默认背景色
		 */
		void SetItemBkColor(DWORD dwColor);
		/**
		 * @brief 获取子项默认背景色
		 * @return DWORD 返回默认背景色
		 */
		DWORD GetItemBkColor() const;
		/**
		 * @brief 设置子项 hot 状态背景色
		 * @param dwColor [in] hot 状态背景色
		 */
		void SetItemHotBkColor(DWORD dwColor);
		/**
		 * @brief 获取子项 hot 状态背景色
		 * @return DWORD 返回 hot 状态背景色
		 */
		DWORD GetItemHotBkColor() const;
		/**
		 * @brief 设置子项选中状态背景色
		 * @param dwColor [in] 选中状态背景色
		 */
		void SetItemSelectedBkColor(DWORD dwColor);
		/**
		 * @brief 获取子项选中状态背景色
		 * @return DWORD 返回选中状态背景色
		 */
		DWORD GetItemSelectedBkColor() const;

		/**
		 * @brief 设置是否允许再次点击已选中项时取消选择
		 * @details 启用后，当用户点击当前已选中的子项时，会直接清空选中状态。
		 * @param allow [in] 是否允许取消选择
		 */
		void SetAllowSelectionCancel(bool allow);
		/**
		 * @brief 获取是否允许点击取消选择
		 * @return bool 允许返回 true，否则返回 false
		 */
		bool IsAllowSelectionCancel() const;

		/**
		 * @brief 设置创建可视子项控件的回调
		 * @details 该回调用于生成池化的子控件。修改该回调后会重新创建当前池中的子项控件。
		 * @param callback [in] 创建子项控件的回调
		 */
		void SetCreateItemCallback(CreateItemCallback callback);
		/**
		 * @brief 设置绑定子项数据的回调
		 * @details 每次可视池中的某个子控件要绑定到新的逻辑子项时，都会调用该回调。
		 * @param callback [in] 绑定回调
		 */
		void SetBindItemCallback(BindItemCallback callback);
		/**
		 * @brief 设置子项绘制前的附加绘制回调
		 * @details 在每个可视子项控件绘制前调用，适合绘制额外装饰内容。
		 * @param callback [in] 绘制回调
		 */
		void SetPaintItemCallback(PaintItemCallback callback);
		/**
		 * @brief 设置子项点击回调
		 * @param callback [in] 点击回调
		 */
		void SetItemClickCallback(ItemEventCallback callback);
		/**
		 * @brief 设置子项双击回调
		 * @param callback [in] 双击回调
		 */
		void SetItemDoubleClickCallback(ItemEventCallback callback);

		/**
		 * @brief 刷新虚拟列表
		 * @details 重新计算滚动条状态、更新当前可视子项池，并触发重绘。
		 */
		void Refresh();
		/**
		 * @brief 刷新单个子项
		 * @details 当指定子项当前处于可视池中时，会重新绑定该子项控件并触发重绘。
		 * @param index [in] 子项下标
		 */
		void RefreshItem(ItemIndex index);
		/**
		 * @brief 确保指定子项滚动到可视区域内
		 * @details 如果目标子项不在当前可视范围中，会自动滚动到可见位置。
		 * 当 alignTop 为 true 时，优先将该子项对齐到视口顶部。
		 * @param index [in] 子项下标
		 * @param alignTop [in] 是否尽量顶对齐
		 */
		void EnsureVisible(ItemIndex index, bool alignTop = false);

		/**
		 * @brief 获取当前已创建并显示在池中的逻辑子项下标集合
		 * @param collection [out] 输出当前显示中的子项下标集合
		 */
		void GetDisplayCollection(std::vector<ItemIndex>& collection) const;
		/**
		 * @brief 获取当前已创建并显示在池中的逻辑子项下标集合
		 * @details 当下标值能够放入 int 时，会转换后输出到 collection 中。
		 * @param collection [out] 输出当前显示中的子项下标集合
		 */
		void GetDisplayCollection(std::vector<int>& collection) const;

		/**
		 * @brief 选中指定子项
		 * @details 选中后会自动滚动到可见区域，可选择是否抢占焦点以及是否发送 itemselect 通知。
		 * @param index [in] 子项下标
		 * @param takeFocus [in] 是否设置输入焦点
		 * @param notify [in] 是否发送通知
		 * @return bool 选中成功返回 true，index 越界时返回 false
		 */
		bool SelectItem(ItemIndex index, bool takeFocus = false, bool notify = true);
		/**
		 * @brief 清空当前选中状态
		 * @param notify [in] 是否发送 itemselect 通知
		 */
		void ClearSelection(bool notify = true);
		/**
		 * @brief 判断当前是否存在选中项
		 * @return bool 存在选中项返回 true，否则返回 false
		 */
		bool HasSelection() const;
		/**
		 * @brief 取消指定子项的选中状态
		 * @details 只有当 index 恰好等于当前选中项时才会真正取消选择。
		 * @param index [in] 子项下标
		 * @param notify [in] 是否发送通知
		 * @return bool 取消成功返回 true，否则返回 false
		 */
		bool UnselectItem(ItemIndex index, bool notify = true);
		/**
		 * @brief 获取当前选中项下标
		 * @return ItemIndex 返回当前选中项下标；无选中项时返回内部无效值
		 */
		ItemIndex GetSelectedIndex() const;
		/**
		 * @brief 判断指定子项是否被选中
		 * @param index [in] 子项下标
		 * @return bool 已选中返回 true，否则返回 false
		 */
		bool IsItemSelected(ItemIndex index) const;
		/**
		 * @brief 判断当前是否存在 hot 状态子项
		 * @return bool 存在 hot 状态子项返回 true，否则返回 false
		 */
		bool HasHotItem() const;
		/**
		 * @brief 获取当前 hot 状态子项下标
		 * @return ItemIndex 返回当前 hot 状态子项下标；无 hot 项时返回内部无效值
		 */
		ItemIndex GetHotItemIndex() const;

		/**
		 * @brief 获取当前垂直滚动偏移
		 * @return long long 返回当前逻辑像素单位的滚动偏移
		 */
		long long GetScrollOffset() const;
		/**
		 * @brief 设置当前垂直滚动偏移
		 * @details 偏移值会自动限制在有效内容范围内，避免越界。
		 * @param offset [in] 目标滚动偏移
		 * @param notify [in] 是否发送滚动通知
		 */
		void SetScrollOffset(long long offset, bool notify = true);
		/**
		 * @brief 获取当前滚动位置
		 * @details 按 FYUI 的 SIZE 约定返回滚动位置。
		 * @return SIZE 返回当前滚动位置
		 */
		SIZE GetScrollPos() const override;
		/**
		 * @brief 获取滚动范围
		 * @details 按 FYUI 的 SIZE 约定返回滚动范围。
		 * @return SIZE 返回当前滚动范围
		 */
		SIZE GetScrollRange() const override;
		/**
		 * @brief 设置滚动位置
		 * @param szPos [in] 目标滚动位置
		 * @param bMsg [in] 是否发送通知
		 * @param bScroolVisible [in] 保留参数
		 */
		void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true) override;
		/**
		 * @brief 向上滚动一个逻辑子项
		 * @param bScroolVisible [in] 保留参数
		 */
		void LineUp(bool bScroolVisible = true) override;
		/**
		 * @brief 向下滚动一个逻辑子项
		 * @param bScroolVisible [in] 保留参数
		 */
		void LineDown(bool bScroolVisible = true) override;
		/**
		 * @brief 向上翻一页
		 */
		void PageUp() override;
		/**
		 * @brief 向下翻一页
		 */
		void PageDown() override;
		/**
		 * @brief 滚动到第一个子项
		 */
		void HomeUp() override;
		/**
		 * @brief 滚动到最后一个子项
		 */
		void EndDown() override;

		/**
		 * @brief 设置控件位置
		 * @details 更新控件矩形、滚动条位置以及当前可视池中的子项布局。
		 * @param rc [in] 控件矩形
		 * @param bNeedInvalidate [in] 是否触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		/**
		 * @brief 设置控件所属管理器
		 * @details 当控件挂接到管理器后，会同步刷新滚动条和当前可视子项状态。
		 * @param pManager [in] 管理器指针
		 * @param pParent [in] 父控件指针
		 * @param bInit [in] 是否初始化
		 */
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;
		/**
		 * @brief 处理控件事件
		 * @details 处理鼠标移动、点击、双击、键盘和滚轮事件，用于实现 hot 跟踪、选择与滚动。
		 * @param event [in] 事件参数
		 */
		void DoEvent(TEventUI& event) override;
		/**
		 * @brief 绘制控件内容
		 * @details 负责绘制控件背景、子项状态背景、当前可视池中的子控件以及滚动条。
		 * @param renderContext [in] 绘制上下文
		 * @param pStopControl [in] 停止绘制控件
		 * @return bool 绘制完成返回 true
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/**
		 * @brief 解析 XML 属性
		 * @details 支持解析子项数量、高度模式、overscan、是否允许取消选择以及子项状态背景色等属性。
		 * @param pstrName [in] 属性名
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue) override;
		/**
		 * @brief 清空当前可视池中的子项控件
		 * @param bChildDelayed [in] 是否延迟删除子控件
		 */
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
