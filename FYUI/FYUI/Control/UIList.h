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
		/**
		 * @brief 获取子项文本
		 * @details 用于获取子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pList [in] 列表控件对象
		 * @param iItem [in] 子项下标
		 * @param iSubItem [in] 子项下标
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetItemText(CControlUI* pList, int iItem, int iSubItem) = 0;
		/**
		 * @brief 获取子项文本颜色
		 * @details 用于获取子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pList [in] 列表控件对象
		 * @param iItem [in] 子项下标
		 * @param iSubItem [in] 子项下标
		 * @param iState [in] 状态值
		 * @return DWORD 返回对应的数值结果
		 */
		virtual DWORD GetItemTextColor(CControlUI* pList, int iItem, int iSubItem, int iState) = 0;// iState锛?-姝ｅ父銆?-婵€娲汇€?-閫夋嫨銆?-绂佺敤
	};

	class IListOwnerUI
	{
	public:
		/**
		 * @brief 获取列表类型
		 * @details 用于获取列表类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetListType() = 0;
		/**
		 * @brief 获取列表信息
		 * @details 用于获取列表信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @return TListInfoUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual TListInfoUI* GetListInfo() = 0;
		/**
		 * @brief 获取当前选区
		 * @details 用于获取当前选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetCurSel() const = 0;
		/**
		 * @brief 选中指定子项
		 * @details 用于选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bTakeFocus [in] 是否设置输入焦点
		 * @param bIsClick [in] 是否按点击方式处理
		 * @param bSetScrollPos [in] 是否同步滚动到目标位置
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool SelectItem(int iIndex, bool bTakeFocus = false, bool bIsClick = false, bool bSetScrollPos = true) = 0;
		/**
		 * @brief 选中指定多选子项
		 * @details 用于选中指定多选子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bTakeFocus [in] 是否设置输入焦点
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool SelectMultiItem(int iIndex, bool bTakeFocus = false) = 0;
		/**
		 * @brief 取消选中指定子项
		 * @details 用于取消选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bOthers [in] 是否同时处理其他项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool UnSelectItem(int iIndex, bool bOthers = false) = 0;
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void DoEvent(TEventUI& event) = 0;
	};

	class IListUI : public IListOwnerUI
	{
	public:
		/**
		 * @brief 获取表头
		 * @details 用于获取表头。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListHeaderUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CListHeaderUI* GetHeader() const = 0;
		/**
		 * @brief 获取列表
		 * @details 用于获取列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CContainerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CContainerUI* GetList() const = 0;
		/**
		 * @brief 获取文本回调
		 * @details 用于获取文本回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @return IListCallbackUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual IListCallbackUI* GetTextCallback() const = 0;
		/**
		 * @brief 设置文本回调
		 * @details 用于设置文本回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pCallback [in] 回调接口对象
		 */
		virtual void SetTextCallback(IListCallbackUI* pCallback) = 0;
		/**
		 * @brief 展开子项
		 * @details 用于展开子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bExpand [in] 是否展开
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool ExpandItem(int iIndex, bool bExpand = true) = 0;
		/**
		 * @brief 获取Expanded子项
		 * @details 用于获取Expanded子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetExpandedItem() const = 0;

		/**
		 * @brief 设置多选选中
		 * @details 用于设置多选选中。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bMultiSel [in] 是否多选选区
		 */
		virtual void SetMultiSelect(bool bMultiSel) = 0;
		/**
		 * @brief 判断是否多选选中
		 * @details 用于判断是否多选选中。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsMultiSelect() const = 0;
		/**
		 * @brief 选中All子项集合
		 * @details 用于选中All子项集合。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void SelectAllItems() = 0;
		/**
		 * @brief 取消选中All子项集合
		 * @details 用于取消选中All子项集合。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void UnSelectAllItems() = 0;
		/**
		 * @brief 获取选中子项数量
		 * @details 用于获取选中子项数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetSelectItemCount() const = 0;
		/**
		 * @brief 获取Next选区子项
		 * @details 用于获取Next选区子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nItem [in] 子项数值
		 * @return int 返回对应的数值结果
		 */
		virtual int GetNextSelItem(int nItem) const = 0;
	};

	class IListItemUI
	{
	public:
		/**
		 * @brief 获取索引
		 * @details 用于获取索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetIndex() const = 0;
		/**
		 * @brief 设置索引
		 * @details 用于设置索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 */
		virtual void SetIndex(int iIndex) = 0;
		/**
		 * @brief 获取所属对象
		 * @details 用于获取所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return IListOwnerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual IListOwnerUI* GetOwner() = 0;
		/**
		 * @brief 设置所属对象
		 * @details 用于设置所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		virtual void SetOwner(CControlUI* pOwner) = 0;
		/**
		 * @brief 判断是否Selected
		 * @details 用于判断是否Selected。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsSelected() const = 0;
		/**
		 * @brief 执行 Select 操作
		 * @details 用于执行 Select 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelect [in] 是否选中
		 * @param bIsClick [in] 是否按点击方式处理
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Select(bool bSelect = true, bool bIsClick = false) = 0;
		/**
		 * @brief 选中多选
		 * @details 用于选中多选。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelect [in] 是否选中
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool SelectMulti(bool bSelect = true) = 0;
		/**
		 * @brief 判断是否Expanded
		 * @details 用于判断是否Expanded。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsExpanded() const = 0;
		/**
		 * @brief 执行 Expand 操作
		 * @details 用于执行 Expand 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bExpand [in] 是否展开
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Expand(bool bExpand = true) = 0;
		/**
		 * @brief 绘制子项文本
		 * @details 用于绘制子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
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
		/**
		 * @brief 构造 CListUI 对象
		 * @details 用于构造 CListUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

		/**
		 * @brief 获取滚动选中
		 * @details 用于获取滚动选中。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetScrollSelect();
		/**
		 * @brief 设置滚动选中
		 * @details 用于设置滚动选中。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bScrollSelect [in] 是否滚动选中
		 */
		void SetScrollSelect(bool bScrollSelect);
		/**
		 * @brief 获取当前选区
		 * @details 用于获取当前选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetCurSel() const;
		/**
		 * @brief 获取当前选区激活
		 * @details 用于获取当前选区激活。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetCurSelActivate() const;
		/**
		 * @brief 选中指定子项
		 * @details 用于选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bTakeFocus [in] 是否设置输入焦点
		 * @param bIsClick [in] 是否按点击方式处理
		 * @param bSetScrollPos [in] 是否同步滚动到目标位置
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SelectItem(int iIndex, bool bTakeFocus = false, bool bIsClick = false,bool bSetScrollPos=true);
		/**
		 * @brief 选中子项激活
		 * @details 用于选中子项激活。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SelectItemActivate(int iIndex);    // 鍙屽嚮閫変腑

		bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
		/**
		 * @brief 设置多选选中
		 * @details 用于设置多选选中。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bMultiSel [in] 是否多选选区
		 */
		void SetMultiSelect(bool bMultiSel);
		/**
		 * @brief 判断是否多选选中
		 * @details 用于判断是否多选选中。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsMultiSelect() const;
		/**
		 * @brief 取消选中指定子项
		 * @details 用于取消选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bOthers [in] 是否同时处理其他项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool UnSelectItem(int iIndex, bool bOthers = false);
		/**
		 * @brief 选中All子项集合
		 * @details 用于选中All子项集合。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SelectAllItems();
		/**
		 * @brief 取消选中All子项集合
		 * @details 用于取消选中All子项集合。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void UnSelectAllItems();
		/**
		 * @brief 获取选中子项数量
		 * @details 用于获取选中子项数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetSelectItemCount() const;
		/**
		 * @brief 获取Next选区子项
		 * @details 用于获取Next选区子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nItem [in] 子项数值
		 * @return int 返回对应的数值结果
		 */
		int GetNextSelItem(int nItem) const;

		/**
		 * @brief 获取表头
		 * @details 用于获取表头。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListHeaderUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CListHeaderUI* GetHeader() const;  
		/**
		 * @brief 获取列表
		 * @details 用于获取列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CContainerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CContainerUI* GetList() const;
		/**
		 * @brief 获取列表类型
		 * @details 用于获取列表类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetListType();
		/**
		 * @brief 获取列表信息
		 * @details 用于获取列表信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @return TListInfoUI* 返回结果对象指针，失败时返回 nullptr
		 */
		TListInfoUI* GetListInfo();

		/**
		 * @brief 获取子项At
		 * @details 用于获取子项At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* GetItemAt(int iIndex) const;
		/**
		 * @brief 获取子项索引
		 * @details 用于获取子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return int 返回对应的数值结果
		 */
		int GetItemIndex(CControlUI* pControl) const;
		/**
		 * @brief 设置子项索引
		 * @details 用于设置子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @param bUpdate [in] 是否更新
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate = true);
		/**
		 * @brief 获取数量
		 * @details 用于获取数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetCount() const;
		/**
		 * @brief 执行 Add 操作
		 * @details 用于执行 Add 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Add(CControlUI* pControl);
		/**
		 * @brief 添加At
		 * @details 用于添加At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddAt(CControlUI* pControl, int iIndex);
		/**
		 * @brief 执行 Remove 操作
		 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Remove(CControlUI* pControl);
		/**
		 * @brief 移除At
		 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveAt(int iIndex);
		/**
		 * @brief 移除全部子项
		 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RemoveAll();

		/**
		 * @brief 确保指定子项可见
		 * @details 用于确保指定子项可见。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 */
		void EnsureVisible(int iIndex);
		/**
		 * @brief 执行 Scroll 操作
		 * @details 用于执行 Scroll 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dx [in] dx参数
		 * @param dy [in] dy参数
		 */
		void Scroll(int dx, int dy);

		/**
		 * @brief 判断是否延迟销毁
		 * @details 用于判断是否延迟销毁。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsDelayedDestroy() const;
		/**
		 * @brief 设置延迟销毁
		 * @details 用于设置延迟销毁。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bDelayed [in] 是否延迟
		 */
		void SetDelayedDestroy(bool bDelayed);
		/**
		 * @brief 获取子控件内边距
		 * @details 用于获取子控件内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetChildPadding() const;
		/**
		 * @brief 设置子控件内边距
		 * @details 用于设置子控件内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iPadding [in] 内边距值
		 */
		void SetChildPadding(int iPadding);

		/**
		 * @brief 设置子项字体
		 * @details 用于设置子项字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetItemFont(int index);
		/**
		 * @brief 设置子项文本样式
		 * @details 设置列表子项文本的 `DT_*` 文本格式标志，常见用法包括控制水平/垂直对齐、
		 * 单行显示、自动换行、末尾省略和助记符处理。具体语义与 `CRenderEngine::DrawText` 的 `uStyle` 保持一致。
		 * @param uStyle [in] 文本样式标志
		 */
		void SetItemTextStyle(UINT uStyle);
		/**
		 * @brief 设置子项文本内边距
		 * @details 用于设置子项文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void SetItemTextPadding(RECT rc);
		/**
		 * @brief 设置子项文本颜色
		 * @details 用于设置子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetItemTextColor(DWORD dwTextColor);
		/**
		 * @brief 设置子项背景颜色
		 * @details 用于设置子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetItemBkColor(DWORD dwBkColor);
		/**
		 * @brief 设置子项背景图像
		 * @details 用于设置子项背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetItemBkImage(std::wstring_view pStrImage);
		/**
		 * @brief 设置Alternate背景
		 * @details 用于设置Alternate背景。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bAlternateBk [in] 是否Alternate背景
		 */
		void SetAlternateBk(bool bAlternateBk);
		/**
		 * @brief 设置Selected子项文本颜色
		 * @details 用于设置Selected子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetSelectedItemTextColor(DWORD dwTextColor);
		/**
		 * @brief 设置Selected子项背景颜色
		 * @details 用于设置Selected子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetSelectedItemBkColor(DWORD dwBkColor);
		/**
		 * @brief 设置Selected子项图像
		 * @details 用于设置Selected子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetSelectedItemImage(std::wstring_view pStrImage);
		/**
		 * @brief 设置热状态子项文本颜色
		 * @details 用于设置热状态子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetHotItemTextColor(DWORD dwTextColor);
		/**
		 * @brief 设置热状态子项背景颜色
		 * @details 用于设置热状态子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetHotItemBkColor(DWORD dwBkColor);
		/**
		 * @brief 设置热状态子项图像
		 * @details 用于设置热状态子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetHotItemImage(std::wstring_view pStrImage);
		/**
		 * @brief 设置Disabled子项文本颜色
		 * @details 用于设置Disabled子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetDisabledItemTextColor(DWORD dwTextColor);
		/**
		 * @brief 设置Disabled子项背景颜色
		 * @details 用于设置Disabled子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetDisabledItemBkColor(DWORD dwBkColor);
		/**
		 * @brief 设置Disabled子项图像
		 * @details 用于设置Disabled子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetDisabledItemImage(std::wstring_view pStrImage);
		/**
		 * @brief 设置子项行颜色
		 * @details 用于设置子项行颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwLineColor [in] 行颜色数值
		 */
		void SetItemLineColor(DWORD dwLineColor);
		/**
		 * @brief 设置子项显示行行
		 * @details 用于设置子项显示行行。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShowLine [in] 是否显示行
		 */
		void SetItemShowRowLine(bool bShowLine = false);
		/**
		 * @brief 设置子项显示列行
		 * @details 用于设置子项显示列行。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShowLine [in] 是否显示行
		 */
		void SetItemShowColumnLine(bool bShowLine = false);
		/**
		 * @brief 判断是否子项显示HTML 文本
		 * @details 用于判断是否子项显示HTML 文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsItemShowHtml();
		/**
		 * @brief 设置子项显示HTML 文本
		 * @details 用于设置子项显示HTML 文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShowHtml [in] 是否显示HTML 文本
		 */
		void SetItemShowHtml(bool bShowHtml = true);
		/**
		 * @brief 判断是否子项RSelected
		 * @details 用于判断是否子项RSelected。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsItemRSelected();
		/**
		 * @brief 设置子项RSelected
		 * @details 用于设置子项RSelected。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelected [in] 是否Selected
		 */
		void SetItemRSelected(bool bSelected = true);
		/**
		 * @brief 获取子项文本内边距
		 * @details 用于获取子项文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetItemTextPadding() const;
		/**
		 * @brief 获取子项文本颜色
		 * @details 用于获取子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemTextColor() const;
		/**
		 * @brief 获取子项背景颜色
		 * @details 用于获取子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemBkColor() const;
		/**
		 * @brief 获取子项背景图像
		 * @details 用于获取子项背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetItemBkImage() const;
		/**
		 * @brief 判断是否Alternate背景
		 * @details 用于判断是否Alternate背景。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsAlternateBk() const;
		/**
		 * @brief 获取Selected子项文本颜色
		 * @details 用于获取Selected子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectedItemTextColor() const;
		/**
		 * @brief 获取Selected子项背景颜色
		 * @details 用于获取Selected子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectedItemBkColor() const;
		/**
		 * @brief 获取Selected子项图像
		 * @details 用于获取Selected子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetSelectedItemImage() const;
		/**
		 * @brief 获取热状态子项文本颜色
		 * @details 用于获取热状态子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetHotItemTextColor() const;
		/**
		 * @brief 获取热状态子项背景颜色
		 * @details 用于获取热状态子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetHotItemBkColor() const;
		/**
		 * @brief 获取热状态子项图像
		 * @details 用于获取热状态子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetHotItemImage() const;
		/**
		 * @brief 获取Disabled子项文本颜色
		 * @details 用于获取Disabled子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisabledItemTextColor() const;
		/**
		 * @brief 获取Disabled子项背景颜色
		 * @details 用于获取Disabled子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisabledItemBkColor() const;
		/**
		 * @brief 获取Disabled子项图像
		 * @details 用于获取Disabled子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetDisabledItemImage() const;
		/**
		 * @brief 获取子项行颜色
		 * @details 用于获取子项行颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemLineColor() const;

		/**
		 * @brief 设置多选Expanding
		 * @details 用于设置多选Expanding。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bMultiExpandable [in] 是否多选Expandable
		 */
		void SetMultiExpanding(bool bMultiExpandable); 
		/**
		 * @brief 获取Expanded子项
		 * @details 用于获取Expanded子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetExpandedItem() const;
		/**
		 * @brief 展开子项
		 * @details 用于展开子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bExpand [in] 是否展开
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ExpandItem(int iIndex, bool bExpand = true);

		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		/**
		 * @brief 执行 Move 操作
		 * @details 用于执行 Move 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szOffset [in] 位移尺寸
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void Move(SIZE szOffset, bool bNeedInvalidate = true);
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 获取文本回调
		 * @details 用于获取文本回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @return IListCallbackUI* 返回结果对象指针，失败时返回 nullptr
		 */
		IListCallbackUI* GetTextCallback() const;
		/**
		 * @brief 设置文本回调
		 * @details 用于设置文本回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pCallback [in] 回调接口对象
		 */
		void SetTextCallback(IListCallbackUI* pCallback);

		/**
		 * @brief 获取滚动位置
		 * @details 用于获取滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetScrollPos() const;
		/**
		 * @brief 获取滚动Range
		 * @details 用于获取滚动Range。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetScrollRange() const;
		/**
		 * @brief 设置滚动位置
		 * @details 用于设置滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szPos [in] 位置尺寸参数
		 * @param bMsg [in] 是否Msg
		 */
		void SetScrollPos(SIZE szPos, bool bMsg = true);
		/**
		 * @brief 执行 LineUp 操作
		 * @details 用于执行 LineUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void LineUp();
		/**
		 * @brief 执行 LineDown 操作
		 * @details 用于执行 LineDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void LineDown();
		/**
		 * @brief 执行 PageUp 操作
		 * @details 用于执行 PageUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageUp();
		/**
		 * @brief 执行 PageDown 操作
		 * @details 用于执行 PageDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageDown();
		/**
		 * @brief 执行 HomeUp 操作
		 * @details 用于执行 HomeUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void HomeUp();
		/**
		 * @brief 执行 EndDown 操作
		 * @details 用于执行 EndDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EndDown();
		/**
		 * @brief 执行 LineLeft 操作
		 * @details 用于执行 LineLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void LineLeft();
		/**
		 * @brief 执行 LineRight 操作
		 * @details 用于执行 LineRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void LineRight();
		/**
		 * @brief 执行 PageLeft 操作
		 * @details 用于执行 PageLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageLeft();
		/**
		 * @brief 执行 PageRight 操作
		 * @details 用于执行 PageRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PageRight();
		/**
		 * @brief 执行 HomeLeft 操作
		 * @details 用于执行 HomeLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void HomeLeft();
		/**
		 * @brief 执行 EndRight 操作
		 * @details 用于执行 EndRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EndRight();
		/**
		 * @brief 启用滚动Bar
		 * @details 用于启用滚动Bar。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnableVertical [in] 是否启用垂直
		 * @param bEnableHorizontal [in] 是否启用水平
		 */
		void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
		/**
		 * @brief 获取垂直滚动Bar
		 * @details 用于获取垂直滚动Bar。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CScrollBarUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CScrollBarUI* GetVerticalScrollBar() const;
		/**
		 * @brief 获取水平滚动Bar
		 * @details 用于获取水平滚动Bar。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CScrollBarUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CScrollBarUI* GetHorizontalScrollBar() const;
		/**
		 * @brief 执行 SortItems 操作
		 * @details 用于执行 SortItems 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pfnCompare [in] pfnCompare参数
		 * @param dwData [in] 数据数值
		 * @return BOOL 返回 执行 SortItems 操作 的结果
		 */
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

		/**
		 * @brief 执行 CheckColumEditable 操作
		 * @details 用于执行 CheckColumEditable 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @return BOOL 返回 执行 CheckColumEditable 操作 的结果
		 */
		virtual BOOL CheckColumEditable(int nColum) { return FALSE; };
		/**
		 * @brief 执行 CheckColumComboBoxable 操作
		 * @details 用于执行 CheckColumComboBoxable 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @return BOOL 返回 执行 CheckColumComboBoxable 操作 的结果
		 */
		virtual BOOL CheckColumComboBoxable(int nColum) { return FALSE; };
		/**
		 * @brief 获取下拉框BoxUI
		 * @details 用于获取下拉框BoxUI。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CComboBoxUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CComboBoxUI* GetComboBoxUI() { return NULL; };

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CListUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CListUI* pControl) ;


	protected:
			/**
			 * @brief 执行 ResolveListItemByIndex 操作
			 * @details 用于执行 ResolveListItemByIndex 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 * @param pControl [in,out] 控件对象
			 * @param pListItem [in,out] 列表子项对象
			 * @param bRequireEnabled [in] 是否Require启用状态
			 * @param bRequireVisible [in] 是否Require可见状态
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool ResolveListItemByIndex(int iIndex, CControlUI*& pControl, IListItemUI*& pListItem, bool bRequireEnabled = false, bool bRequireVisible = false) const;
			/**
			 * @brief 执行 ReindexListItems 操作
			 * @details 用于执行 ReindexListItems 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iStart [in] Start值
			 */
			void ReindexListItems(int iStart);
			/**
			 * @brief 执行 RestoreSelectionAfterRemoval 操作
			 * @details 用于执行 RestoreSelectionAfterRemoval 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iRemovedIndex [in] Removed索引值
			 */
			void RestoreSelectionAfterRemoval(int iRemovedIndex);
			/**
			 * @brief 执行 ShiftSelectionIndices 操作
			 * @details 用于执行 ShiftSelectionIndices 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iStartIndex [in] Start索引值
			 * @param iDelta [in] Delta值
			 */
			void ShiftSelectionIndices(int iStartIndex, int iDelta);
			/**
			 * @brief 执行 ShiftTrackedItemIndices 操作
			 * @details 用于执行 ShiftTrackedItemIndices 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iStartIndex [in] Start索引值
			 * @param iDelta [in] Delta值
			 */
			void ShiftTrackedItemIndices(int iStartIndex, int iDelta);
			/**
			 * @brief 执行 RestoreTrackedStateAfterRemoval 操作
			 * @details 用于执行 RestoreTrackedStateAfterRemoval 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iRemovedIndex [in] Removed索引值
			 */
			void RestoreTrackedStateAfterRemoval(int iRemovedIndex);
			/**
			 * @brief 添加Selection索引
			 * @details 用于添加Selection索引。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 */
			void AddSelectionIndex(int iIndex);
			/**
			 * @brief 移除Selection索引
			 * @details 用于移除Selection索引。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 */
			void RemoveSelectionIndex(int iIndex);
		/**
		 * @brief 执行 NotifySelectionChanged 操作
		 * @details 用于执行 NotifySelectionChanged 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 */
		void NotifySelectionChanged(int iIndex);
		/**
		 * @brief 执行 NotifySelectionChanged 操作
		 * @details 用于执行 NotifySelectionChanged 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param iAnchorIndex [in] Anchor索引值
		 */
		void NotifySelectionChanged(int iIndex, int iAnchorIndex);
		/**
		 * @brief 执行 NotifySelectionCleared 操作
		 * @details 用于执行 NotifySelectionCleared 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void NotifySelectionCleared();
		/**
		 * @brief 应用Selection焦点And滚动
		 * @details 用于应用Selection焦点And滚动。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @param bTakeFocus [in] 是否设置输入焦点
		 * @param bSetScrollPos [in] 是否同步滚动到目标位置
		 */
		void ApplySelectionFocusAndScroll(CControlUI* pControl, int iIndex, bool bTakeFocus, bool bSetScrollPos);
		/**
		 * @brief 选中子项Range
		 * @details 用于选中子项Range。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iStart [in] Start值
		 * @param iEnd [in] End值
		 */
		void SelectItemRange(int iStart, int iEnd);
		/**
		 * @brief 获取列表Viewport矩形
		 * @details 用于获取列表Viewport矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetListViewportRect() const;
		/**
		 * @brief 获取最小选区子项索引
		 * @details 用于获取最小选区子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMinSelItemIndex();
		/**
		 * @brief 获取最大选区子项索引
		 * @details 用于获取最大选区子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMaxSelItemIndex();
		/**
		 * @brief 更新列表焦点状态
		 * @details 用于更新列表焦点状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bFocused [in] 是否Focused
		 */
		void UpdateListFocusState(bool bFocused);
		/**
		 * @brief 处理列表KeyDown事件
		 * @details 用于处理列表KeyDown事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in] 事件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HandleListKeyDownEvent(const TEventUI& event);
		/**
		 * @brief 处理列表滚动Wheel事件
		 * @details 用于处理列表滚动Wheel事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in] 事件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
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
		/**
		 * @brief 构造 CListBodyUI 对象
		 * @details 用于构造 CListBodyUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		CListBodyUI(CListUI* pOwner);


		/**
		 * @brief 获取滚动步长尺寸
		 * @details 用于获取滚动步长尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetScrollStepSize() const;
		/**
		 * @brief 设置滚动位置
		 * @details 用于设置滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szPos [in] 位置尺寸参数
		 * @param bMsg [in] 是否Msg
		 */
		void SetScrollPos(SIZE szPos, bool bMsg = true);
		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 执行 SortItems 操作
		 * @details 用于执行 SortItems 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pfnCompare [in] pfnCompare参数
		 * @param dwData [in] 数据数值
		 * @return BOOL 返回 执行 SortItems 操作 的结果
		 */
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CListBodyUI* pControl) ;

	protected:
		/**
		 * @brief 执行 ItemComareFunc 操作
		 * @details 用于执行 ItemComareFunc 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pvlocale [in] pvlocale参数
		 * @param item1 [in] 子项1参数
		 * @param item2 [in] 子项2参数
		 * @return int __cdecl 返回 执行 ItemComareFunc 操作 的结果
		 */
		static int __cdecl ItemComareFunc(void *pvlocale, const void *item1, const void *item2);
		/**
		 * @brief 执行 ItemComareFunc 操作
		 * @details 用于执行 ItemComareFunc 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param item1 [in] 子项1参数
		 * @param item2 [in] 子项2参数
		 * @return int __cdecl 返回 执行 ItemComareFunc 操作 的结果
		 */
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
		/**
		 * @brief 构造 CListHeaderUI 对象
		 * @details 用于构造 CListHeaderUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListHeaderUI();
		/**
		 * @brief 析构 CListHeaderUI 对象
		 * @details 用于析构 CListHeaderUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CListHeaderUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable);
		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 设置缩放比例表头
		 * @details 用于设置缩放比例表头。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bIsScale [in] 是否判断是否缩放比例
		 */
		void SetScaleHeader(bool bIsScale);
		/**
		 * @brief 判断是否缩放比例表头
		 * @details 用于判断是否缩放比例表头。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsScaleHeader() const;

		/**
		 * @brief 执行初始化逻辑
		 * @details 用于执行初始化逻辑。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void DoInit();
		/**
		 * @brief 执行后置绘制
		 * @details 用于执行后置绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void DoPostPaint(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListHeaderUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CListHeaderUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
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
		/**
		 * @brief 构造 CListHeaderItemUI 对象
		 * @details 用于构造 CListHeaderItemUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListHeaderItemUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;

		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetEnabled(bool bEnable = true);

		/**
		 * @brief 判断是否Dragable
		 * @details 用于判断是否Dragable。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsDragable() const;
		/**
		 * @brief 设置Dragable
		 * @details 用于设置Dragable。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bDragable [in] 是否Dragable
		 */
		void SetDragable(bool bDragable);
		/**
		 * @brief 获取分隔宽度
		 * @details 用于获取分隔宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSepWidth() const;
		/**
		 * @brief 设置分隔宽度
		 * @details 用于设置分隔宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iWidth [in] 宽度值
		 */
		void SetSepWidth(int iWidth);
		/**
		 * @brief 获取文本样式
		 * @details 用于获取文本样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetTextStyle() const;
		/**
		 * @brief 设置文本样式
		 * @details 设置列表头文本的 `DT_*` 文本格式标志，支持对齐、单行/换行、省略号等常见排版控制，
		 * 具体语义与 `CRenderEngine::DrawText` 的 `uStyle` 保持一致。
		 * @param uStyle [in] 文本样式标志
		 */
		void SetTextStyle(UINT uStyle);
		/**
		 * @brief 获取文本颜色
		 * @details 用于获取文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetTextColor() const;
		/**
		 * @brief 设置文本颜色
		 * @details 用于设置文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetTextColor(DWORD dwTextColor);
		/**
		 * @brief 设置文本内边距
		 * @details 用于设置文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void SetTextPadding(RECT rc);
		/**
		 * @brief 获取文本内边距
		 * @details 用于获取文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetTextPadding() const;
		/**
		 * @brief 设置字体
		 * @details 用于设置字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetFont(int index);
		/**
		 * @brief 判断是否显示HTML 文本
		 * @details 用于判断是否显示HTML 文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsShowHtml();
		/**
		 * @brief 设置显示HTML 文本
		 * @details 用于设置显示HTML 文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShowHtml [in] 是否显示HTML 文本
		 */
		void SetShowHtml(bool bShowHtml = true);
		/**
		 * @brief 获取Normal图像
		 * @details 用于获取Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetNormalImage() const;
		/**
		 * @brief 设置Normal图像
		 * @details 用于设置Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取热状态图像
		 * @details 用于获取热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetHotImage() const;
		/**
		 * @brief 设置热状态图像
		 * @details 用于设置热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Pushed图像
		 * @details 用于获取Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetPushedImage() const;
		/**
		 * @brief 设置Pushed图像
		 * @details 用于设置Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetPushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Focused图像
		 * @details 用于获取Focused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetFocusedImage() const;
		/**
		 * @brief 设置Focused图像
		 * @details 用于设置Focused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetFocusedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取分隔图像
		 * @details 用于获取分隔图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetSepImage() const;
		/**
		 * @brief 设置分隔图像
		 * @details 用于设置分隔图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetSepImage(std::wstring_view pStrImage);
		/**
		 * @brief 设置缩放比例
		 * @details 用于设置缩放比例。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nScale [in] 缩放比例数值
		 */
		void SetScale(int nScale);
		/**
		 * @brief 获取缩放比例
		 * @details 用于获取缩放比例。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetScale() const;

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 获取滑块矩形
		 * @details 用于获取滑块矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetThumbRect() const;

		/**
		 * @brief 绘制文本内容
		 * @details 用于绘制文本内容。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintText(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制状态图像
		 * @details 用于绘制状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListHeaderItemUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CListHeaderItemUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
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
		/**
		 * @brief 构造 CListElementUI 对象
		 * @details 用于构造 CListElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListElementUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetEnabled(bool bEnable = true);

		/**
		 * @brief 获取索引
		 * @details 用于获取索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetIndex() const;
		/**
		 * @brief 设置索引
		 * @details 用于设置索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 */
		void SetIndex(int iIndex);

		/**
		 * @brief 获取所属对象
		 * @details 用于获取所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return IListOwnerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		IListOwnerUI* GetOwner();
		/**
		 * @brief 设置所属对象
		 * @details 用于设置所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		void SetOwner(CControlUI* pOwner);
		/**
		 * @brief 设置可见状态
		 * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 */
		void SetVisible(bool bVisible = true);

		/**
		 * @brief 判断是否Selected
		 * @details 用于判断是否Selected。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsSelected() const;
		/**
		 * @brief 执行 Select 操作
		 * @details 用于执行 Select 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelect [in] 是否选中
		 * @param bIsClick [in] 是否按点击方式处理
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Select(bool bSelect = true,bool bIsClick=false);
		/**
		 * @brief 选中多选
		 * @details 用于选中多选。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelect [in] 是否选中
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SelectMulti(bool bSelect = true);
		/**
		 * @brief 判断是否Expanded
		 * @details 用于判断是否Expanded。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsExpanded() const;
		/**
		 * @brief 执行 Expand 操作
		 * @details 用于执行 Expand 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bExpand [in] 是否展开
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Expand(bool bExpand = true);

		/**
		 * @brief 触发重绘
		 * @details 用于触发重绘。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Invalidate(); // 鐩存帴CControl::Invalidate浼氬鑷存粴鍔ㄦ潯鍒锋柊锛岄噸鍐欏噺灏戝埛鏂板尯鍩?
		bool Activate();

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 绘制子项背景
		 * @details 用于绘制子项背景。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		void DrawItemBk(CPaintRenderContext& renderContext, const RECT& rcItem);
		/**
		 * @brief 绘制子项文本
		 * @details 用于绘制子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		virtual void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem);
		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListElementUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CListElementUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
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
		/**
		 * @brief 构造 CListLabelElementUI 对象
		 * @details 用于构造 CListLabelElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListLabelElementUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable);
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;

		/**
		 * @brief 绘制子项文本
		 * @details 用于绘制子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem) override;
	

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListLabelElementUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CListLabelElementUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CListLabelElementUI* pControl) ;
	public:
		
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListTextElementUI : public CListLabelElementUI
	{
		DECLARE_DUICONTROL(CListTextElementUI)
	public:
		/**
		 * @brief 构造 CListTextElementUI 对象
		 * @details 用于构造 CListTextElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListTextElementUI();
		/**
		 * @brief 析构 CListTextElementUI 对象
		 * @details 用于析构 CListTextElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CListTextElementUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;

		/**
		 * @brief 获取文本
		 * @details 用于获取文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetText(int iIndex) const;
		/**
		 * @brief 设置文本
		 * @details 用于设置文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param pstrText [in] 文本字符串
		 */
		void SetText(int iIndex, std::wstring_view pstrText);

		/**
		 * @brief 获取文本颜色
		 * @details 用于获取文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetTextColor(int iIndex) const;
		/**
		 * @brief 设置文本颜色
		 * @details 用于设置文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetTextColor(int iIndex, DWORD dwTextColor);

		/**
		 * @brief 设置所属对象
		 * @details 用于设置所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		void SetOwner(CControlUI* pOwner);
		/**
		 * @brief 获取LinkContent
		 * @details 用于获取LinkContent。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return 返回对应的字符串内容
		 */
		std::wstring* GetLinkContent(int iIndex);

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable);

		/**
		 * @brief 绘制子项文本
		 * @details 用于绘制子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem) override;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListTextElementUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CListTextElementUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
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
		/**
		 * @brief 构造 CListContainerElementUI 对象
		 * @details 用于构造 CListContainerElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListContainerElementUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

		/**
		 * @brief 获取索引
		 * @details 用于获取索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetIndex() const;
		/**
		 * @brief 设置索引
		 * @details 用于设置索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 */
		void SetIndex(int iIndex);

		/**
		 * @brief 获取所属对象
		 * @details 用于获取所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return IListOwnerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		IListOwnerUI* GetOwner();
		/**
		 * @brief 设置所属对象
		 * @details 用于设置所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		void SetOwner(CControlUI* pOwner);
		/**
		 * @brief 设置可见状态
		 * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 */
		void SetVisible(bool bVisible = true);
		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetEnabled(bool bEnable = true);

		/**
		 * @brief 判断是否Selected
		 * @details 用于判断是否Selected。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsSelected() const;
		/**
		 * @brief 执行 Select 操作
		 * @details 用于执行 Select 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelect [in] 是否选中
		 * @param bIsClick [in] 是否按点击方式处理
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Select(bool bSelect = true, bool bIsClick = false);
		/**
		 * @brief 选中多选
		 * @details 用于选中多选。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelect [in] 是否选中
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SelectMulti(bool bSelect = true);
		/**
		 * @brief 判断是否Expanded
		 * @details 用于判断是否Expanded。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsExpanded() const;
		/**
		 * @brief 执行 Expand 操作
		 * @details 用于执行 Expand 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bExpand [in] 是否展开
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Expand(bool bExpand = true);

		/**
		 * @brief 触发重绘
		 * @details 用于触发重绘。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Invalidate(); // 鐩存帴CControl::Invalidate浼氬鑷存粴鍔ㄦ潯鍒锋柊锛岄噸鍐欏噺灏戝埛鏂板尯鍩?
		bool Activate();

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;

		/**
		 * @brief 绘制子项文本
		 * @details 用于绘制子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		virtual void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem);
		/**
		 * @brief 绘制子项背景
		 * @details 用于绘制子项背景。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		virtual void DrawItemBk(CPaintRenderContext& renderContext, const RECT& rcItem);

		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true);

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CListContainerElementUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CListContainerElementUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CListContainerElementUI* pControl) ;

	protected:
		int m_iIndex;
		bool m_bSelected;
		UINT m_uButtonState;
		IListOwnerUI* m_pOwner;
	};

} 


