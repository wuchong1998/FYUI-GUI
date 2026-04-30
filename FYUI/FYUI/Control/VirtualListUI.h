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
		 * @brief 构造 CVirtualListUI 对象
		 * @details 用于构造 CVirtualListUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CVirtualListUI();
		/**
		 * @brief 析构 CVirtualListUI 对象
		 * @details 用于析构 CVirtualListUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CVirtualListUI() override;

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const override;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName) override;

		/**
		 * @brief 设置子项数量
		 * @details 用于设置子项数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @param count [in] 数量参数
		 */
		void SetItemCount(ItemIndex count);
		/**
		 * @brief 获取子项数量
		 * @details 用于获取子项数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ItemIndex 返回 获取子项数量 的结果
		 */
		ItemIndex GetItemCount() const;

		/**
		 * @brief 设置固定子项高度
		 * @details 用于设置固定子项高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param height [in] 高度参数
		 */
		void SetFixedItemHeight(int height);
		/**
		 * @brief 获取固定子项高度
		 * @details 用于获取固定子项高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetFixedItemHeight() const;
		/**
		 * @brief 判断是否固定高度Mode
		 * @details 用于判断是否固定高度Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsFixedHeightMode() const;

		/**
		 * @brief 设置子项Heights
		 * @details 用于设置子项Heights。具体行为由当前对象状态以及传入参数共同决定。
		 * @param heights [in] heights参数
		 */
		void SetItemHeights(const std::vector<int>& heights);
		/**
		 * @brief 设置子项Heights
		 * @details 用于设置子项Heights。具体行为由当前对象状态以及传入参数共同决定。
		 * @param heights [in,out] heights参数
		 */
		void SetItemHeights(std::vector<int>&& heights);
		/**
		 * @brief 设置子项高度
		 * @details 用于设置子项高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @param height [in] 高度参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetItemHeight(ItemIndex index, int height);
		/**
		 * @brief 设置子项Heights
		 * @details 用于设置子项Heights。具体行为由当前对象状态以及传入参数共同决定。
		 * @param heights [in] heights参数
		 * @param count [in] 数量参数
		 */
		void SetItemHeights(const int* heights, size_t count);
		/**
		 * @brief 清空子项Heights
		 * @details 用于清空子项Heights。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ClearItemHeights();
		/**
		 * @brief 判断是否Variable高度Mode
		 * @details 用于判断是否Variable高度Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsVariableHeightMode() const;
		/**
		 * @brief 获取子项高度
		 * @details 用于获取子项高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @return int 返回对应的数值结果
		 */
		int GetItemHeight(ItemIndex index) const;

		/**
		 * @brief 设置Overscan子项数量
		 * @details 用于设置Overscan子项数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @param count [in] 数量参数
		 */
		void SetOverscanItemCount(int count);
		/**
		 * @brief 获取Overscan子项数量
		 * @details 用于获取Overscan子项数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetOverscanItemCount() const;

		/**
		 * @brief 设置子项背景颜色
		 * @details 用于设置子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetItemBkColor(DWORD dwColor);
		/**
		 * @brief 获取子项背景颜色
		 * @details 用于获取子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemBkColor() const;
		/**
		 * @brief 设置子项热状态背景颜色
		 * @details 用于设置子项热状态背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetItemHotBkColor(DWORD dwColor);
		/**
		 * @brief 获取子项热状态背景颜色
		 * @details 用于获取子项热状态背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemHotBkColor() const;
		/**
		 * @brief 设置子项Selected背景颜色
		 * @details 用于设置子项Selected背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetItemSelectedBkColor(DWORD dwColor);
		/**
		 * @brief 获取子项Selected背景颜色
		 * @details 用于获取子项Selected背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemSelectedBkColor() const;

		/**
		 * @brief 设置AllowSelectionCancel
		 * @details 用于设置AllowSelectionCancel。具体行为由当前对象状态以及传入参数共同决定。
		 * @param allow [in] allow参数
		 */
		void SetAllowSelectionCancel(bool allow);
		/**
		 * @brief 判断是否AllowSelectionCancel
		 * @details 用于判断是否AllowSelectionCancel。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsAllowSelectionCancel() const;

		/**
		 * @brief 设置创建子项回调
		 * @details 用于设置创建子项回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @param callback [in] 回调参数
		 */
		void SetCreateItemCallback(CreateItemCallback callback);
		/**
		 * @brief 设置Bind子项回调
		 * @details 用于设置Bind子项回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @param callback [in] 回调参数
		 */
		void SetBindItemCallback(BindItemCallback callback);
		/**
		 * @brief 设置绘制子项回调
		 * @details 用于设置绘制子项回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @param callback [in] 回调参数
		 */
		void SetPaintItemCallback(PaintItemCallback callback);
		/**
		 * @brief 设置子项点击回调
		 * @details 用于设置子项点击回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @param callback [in] 回调参数
		 */
		void SetItemClickCallback(ItemEventCallback callback);
		/**
		 * @brief 设置子项Double点击回调
		 * @details 用于设置子项Double点击回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @param callback [in] 回调参数
		 */
		void SetItemDoubleClickCallback(ItemEventCallback callback);

		/**
		 * @brief 执行 Refresh 操作
		 * @details 用于执行 Refresh 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Refresh();
		/**
		 * @brief 执行 RefreshItem 操作
		 * @details 用于执行 RefreshItem 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void RefreshItem(ItemIndex index);
		/**
		 * @brief 确保指定子项可见
		 * @details 用于确保指定子项可见。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @param alignTop [in] 对齐方式Top参数
		 */
		void EnsureVisible(ItemIndex index, bool alignTop = false);

		/**
		 * @brief 获取DisplayCollection
		 * @details 用于获取DisplayCollection。具体行为由当前对象状态以及传入参数共同决定。
		 * @param collection [in,out] collection参数
		 */
		void GetDisplayCollection(std::vector<ItemIndex>& collection) const;
		/**
		 * @brief 获取DisplayCollection
		 * @details 用于获取DisplayCollection。具体行为由当前对象状态以及传入参数共同决定。
		 * @param collection [in,out] collection参数
		 */
		void GetDisplayCollection(std::vector<int>& collection) const;

		/**
		 * @brief 选中指定子项
		 * @details 用于选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @param takeFocus [in] 是否设置输入焦点
		 * @param notify [in] 是否发送通知
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SelectItem(ItemIndex index, bool takeFocus = false, bool notify = true);
		/**
		 * @brief 清空Selection
		 * @details 用于清空Selection。具体行为由当前对象状态以及传入参数共同决定。
		 * @param notify [in] 是否发送通知
		 */
		void ClearSelection(bool notify = true);
		/**
		 * @brief 判断是否具有Selection
		 * @details 用于判断是否具有Selection。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HasSelection() const;
		/**
		 * @brief 执行 UnselectItem 操作
		 * @details 用于执行 UnselectItem 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @param notify [in] 是否发送通知
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool UnselectItem(ItemIndex index, bool notify = true);
		/**
		 * @brief 获取Selected索引
		 * @details 用于获取Selected索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ItemIndex 返回 获取Selected索引 的结果
		 */
		ItemIndex GetSelectedIndex() const;
		/**
		 * @brief 判断是否子项Selected
		 * @details 用于判断是否子项Selected。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsItemSelected(ItemIndex index) const;
		/**
		 * @brief 判断是否具有热状态子项
		 * @details 用于判断是否具有热状态子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HasHotItem() const;
		/**
		 * @brief 获取热状态子项索引
		 * @details 用于获取热状态子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ItemIndex 返回 获取热状态子项索引 的结果
		 */
		ItemIndex GetHotItemIndex() const;

		/**
		 * @brief 获取滚动Offset
		 * @details 用于获取滚动Offset。具体行为由当前对象状态以及传入参数共同决定。
		 * @return long long 返回 获取滚动Offset 的结果
		 */
		long long GetScrollOffset() const;
		/**
		 * @brief 设置滚动Offset
		 * @details 用于设置滚动Offset。具体行为由当前对象状态以及传入参数共同决定。
		 * @param offset [in] offset参数
		 * @param notify [in] 是否发送通知
		 */
		void SetScrollOffset(long long offset, bool notify = true);
		/**
		 * @brief 获取滚动位置
		 * @details 用于获取滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetScrollPos() const override;
		/**
		 * @brief 获取滚动Range
		 * @details 用于获取滚动Range。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetScrollRange() const override;
		/**
		 * @brief 设置滚动位置
		 * @details 用于设置滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szPos [in] 位置尺寸参数
		 * @param bMsg [in] 是否Msg
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true) override;
		/**
		 * @brief 执行 LineUp 操作
		 * @details 用于执行 LineUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		void LineUp(bool bScroolVisible = true) override;
		/**
		 * @brief 执行 LineDown 操作
		 * @details 用于执行 LineDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		void LineDown(bool bScroolVisible = true) override;
		/**
		 * @brief 执行 PageUp 操作
		 * @details 用于执行 PageUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageUp() override;
		/**
		 * @brief 执行 PageDown 操作
		 * @details 用于执行 PageDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageDown() override;
		/**
		 * @brief 执行 HomeUp 操作
		 * @details 用于执行 HomeUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void HomeUp() override;
		/**
		 * @brief 执行 EndDown 操作
		 * @details 用于执行 EndDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EndDown() override;

		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true) override;
		/**
		 * @brief 设置管理器和父级关系
		 * @details 用于设置管理器和父级关系。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param pParent [in] 父级控件对象
		 * @param bInit [in] 是否执行初始化
		 */
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true) override;
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event) override;
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue) override;
		/**
		 * @brief 移除全部子项
		 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bChildDelayed [in] 是否延迟处理子项
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

		/**
		 * @brief 获取视图矩形
		 * @details 用于获取视图矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetViewRect() const;
		/**
		 * @brief 获取Viewport高度
		 * @details 用于获取Viewport高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return long long 返回 获取Viewport高度 的结果
		 */
		long long GetViewportHeight() const;
		/**
		 * @brief 获取Content高度
		 * @details 用于获取Content高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return long long 返回 获取Content高度 的结果
		 */
		long long GetContentHeight() const;
		/**
		 * @brief 获取最大滚动Offset
		 * @details 用于获取最大滚动Offset。具体行为由当前对象状态以及传入参数共同决定。
		 * @return long long 返回 获取最大滚动Offset 的结果
		 */
		long long GetMaxScrollOffset() const;
		/**
		 * @brief 执行 ClampScrollOffset 操作
		 * @details 用于执行 ClampScrollOffset 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param offset [in] offset参数
		 * @return long long 返回 执行 ClampScrollOffset 操作 的结果
		 */
		long long ClampScrollOffset(long long offset) const;
		/**
		 * @brief 获取ScrollbarRange
		 * @details 用于获取ScrollbarRange。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetScrollbarRange() const;
		/**
		 * @brief 执行 OffsetToScrollbarPos 操作
		 * @details 用于执行 OffsetToScrollbarPos 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param offset [in] offset参数
		 * @return int 返回对应的数值结果
		 */
		int OffsetToScrollbarPos(long long offset) const;
		/**
		 * @brief 滚动bar位置ToOffset
		 * @details 用于滚动bar位置ToOffset。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pos [in] 位置参数
		 * @return long long 返回 滚动bar位置ToOffset 的结果
		 */
		long long ScrollbarPosToOffset(int pos) const;
		/**
		 * @brief 执行 ConfigureScrollBar 操作
		 * @details 用于执行 ConfigureScrollBar 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ConfigureScrollBar();
		/**
		 * @brief 应用Variable子项Heights
		 * @details 用于应用Variable子项Heights。具体行为由当前对象状态以及传入参数共同决定。
		 * @param heights [in,out] heights参数
		 */
		void ApplyVariableItemHeights(std::vector<int>&& heights);
		/**
		 * @brief 执行 FinalizeDataChange 操作
		 * @details 用于执行 FinalizeDataChange 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param notifySelection [in] 通知Selection参数
		 */
		void FinalizeDataChange(bool notifySelection);
		/**
		 * @brief 执行 InvalidateRealizedItem 操作
		 * @details 用于执行 InvalidateRealizedItem 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void InvalidateRealizedItem(ItemIndex index);
		/**
		 * @brief 执行 InvalidateSelectionChange 操作
		 * @details 用于执行 InvalidateSelectionChange 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param previousIndex [in] previous索引参数
		 * @param hadPreviousSelection [in] hadPreviousSelection参数
		 */
		void InvalidateSelectionChange(ItemIndex previousIndex, bool hadPreviousSelection);
		/**
		 * @brief 设置热状态子项
		 * @details 用于设置热状态子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @param hot [in] 热状态参数
		 */
		void SetHotItem(ItemIndex index, bool hot);
		/**
		 * @brief 更新热状态子项FromPoint
		 * @details 用于更新热状态子项FromPoint。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 */
		void UpdateHotItemFromPoint(POINT pt);
		/**
		 * @brief 获取子项绘制颜色
		 * @details 用于获取子项绘制颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemDrawColor(ItemIndex index) const;
		/**
		 * @brief 判断是否索引Realized
		 * @details 用于判断是否索引Realized。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsIndexRealized(ItemIndex index) const;

		/**
		 * @brief 执行 RebuildPrefixHeights 操作
		 * @details 用于执行 RebuildPrefixHeights 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RebuildPrefixHeights();
		/**
		 * @brief 查找子项ByOffset
		 * @details 用于查找子项ByOffset。具体行为由当前对象状态以及传入参数共同决定。
		 * @param offset [in] offset参数
		 * @return ItemIndex 返回 查找子项ByOffset 的结果
		 */
		ItemIndex FindItemByOffset(long long offset) const;
		/**
		 * @brief 获取子项Top
		 * @details 用于获取子项Top。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @return long long 返回 获取子项Top 的结果
		 */
		long long GetItemTop(ItemIndex index) const;
		/**
		 * @brief 获取子项Bottom
		 * @details 用于获取子项Bottom。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 * @return long long 返回 获取子项Bottom 的结果
		 */
		long long GetItemBottom(ItemIndex index) const;
		/**
		 * @brief 更新滚动Bar
		 * @details 用于更新滚动Bar。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void UpdateScrollBar();
		/**
		 * @brief 执行 RealizeVisibleItems 操作
		 * @details 用于执行 RealizeVisibleItems 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RealizeVisibleItems();
		/**
		 * @brief 确保Pool尺寸
		 * @details 用于确保Pool尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param count [in] 数量参数
		 */
		void EnsurePoolSize(size_t count);
		/**
		 * @brief 创建Pool子项
		 * @details 用于创建Pool子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* CreatePoolItem();
		/**
		 * @brief 执行 BindRealizedItem 操作
		 * @details 用于执行 BindRealizedItem 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param item [in,out] 子项参数
		 */
		void BindRealizedItem(RealizedItem& item);
		/**
		 * @brief 执行 HitTestRealizedItem 操作
		 * @details 用于执行 HitTestRealizedItem 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return int 返回对应的数值结果
		 */
		int HitTestRealizedItem(POINT pt) const;
		/**
		 * @brief 执行 DispatchItemEvent 操作
		 * @details 用于执行 DispatchItemEvent 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param realizedIndex [in] realized索引参数
		 * @param event [in,out] 事件对象
		 * @param callback [in] 回调参数
		 */
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
