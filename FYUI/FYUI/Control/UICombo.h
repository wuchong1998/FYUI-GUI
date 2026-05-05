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
		/**
		 * @brief 构造 CComboUI 对象
		 * @details 用于构造 CComboUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CComboUI();

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
		 * @brief 执行初始化逻辑
		 * @details 用于执行初始化逻辑。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void DoInit();
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;

		/**
		 * @brief 获取文本
		 * @details 用于获取文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetText() const;
		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetEnabled(bool bEnable = true);

		/**
		 * @brief 设置文本样式
		 * @details 设置组合框当前文本的 `DT_*` 文本格式标志，支持对齐、单行/换行、省略号、
		 * 助记符处理和测量等常见选项，具体语义与 `CRenderEngine::DrawText` 的 `uStyle` 保持一致。
		 * @param uStyle [in] 文本样式标志
		 */
		void SetTextStyle(UINT uStyle);
		/**
		 * @brief 获取文本样式
		 * @details 用于获取文本样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetTextStyle() const;
		/**
		 * @brief 设置文本颜色
		 * @details 用于设置文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetTextColor(DWORD dwTextColor);
		/**
		 * @brief 获取文本颜色
		 * @details 用于获取文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetTextColor() const;
		/**
		 * @brief 设置Disabled文本颜色
		 * @details 用于设置Disabled文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetDisabledTextColor(DWORD dwTextColor);
		/**
		 * @brief 获取Disabled文本颜色
		 * @details 用于获取Disabled文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisabledTextColor() const;
		/**
		 * @brief 设置字体
		 * @details 用于设置字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetFont(int index);
		/**
		 * @brief 获取字体
		 * @details 用于获取字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetFont() const;
		/**
		 * @brief 获取文本内边距
		 * @details 用于获取文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetTextPadding() const;
		/**
		 * @brief 设置文本内边距
		 * @details 用于设置文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void SetTextPadding(RECT rc);
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
		 * @brief 判断是否显示阴影
		 * @details 用于判断是否显示阴影。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsShowShadow();
		/**
		 * @brief 设置显示阴影
		 * @details 用于设置显示阴影。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShow [in] 是否显示
		 */
		void SetShowShadow(bool bShow = true);

		/**
		 * @brief 获取拖放Box属性列表
		 * @details 用于获取拖放Box属性列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetDropBoxAttributeList();
		/**
		 * @brief 设置拖放Box属性列表
		 * @details 用于设置拖放Box属性列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrList [in] 列表字符串
		 */
		void SetDropBoxAttributeList(std::wstring_view pstrList);
		/**
		 * @brief 获取拖放Box尺寸
		 * @details 用于获取拖放Box尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetDropBoxSize() const;
		/**
		 * @brief 设置拖放Box尺寸
		 * @details 用于设置拖放Box尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szDropBox [in] 拖放Box尺寸参数
		 */
		void SetDropBoxSize(SIZE szDropBox);
		/**
		 * @brief 获取拖放BoxInset
		 * @details 用于获取拖放BoxInset。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetDropBoxInset() const;
		/**
		 * @brief 设置拖放BoxInset
		 * @details 用于设置拖放BoxInset。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szDropBox [in] 拖放Box尺寸参数
		 */
		void SetDropBoxInset(RECT szDropBox);
		/**
		 * @brief 获取拖放Box内边距
		 * @details 用于获取拖放Box内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetDropBoxPadding() const;
		/**
		 * @brief 设置拖放Box内边距
		 * @details 用于设置拖放Box内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szDropBoxPadding [in] 拖放Box内边距尺寸参数
		 */
		void SetDropBoxPadding(RECT szDropBoxPadding);
		/**
		 * @brief 获取拖放Box颜色
		 * @details 用于获取拖放Box颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDropBoxColor() const;
		/**
		 * @brief 设置拖放Box背景颜色
		 * @details 用于设置拖放Box背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetDropBoxBkColor(DWORD dwBkColor);
		/**
		 * @brief 判断是否Auto拖放Box尺寸
		 * @details 用于判断是否Auto拖放Box尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsAutoDropBoxSize();
		/**
		 * @brief 设置Auto拖放Box尺寸
		 * @details 用于设置Auto拖放Box尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bIsShow [in] 是否判断是否显示
		 */
		void SetAutoDropBoxSize(bool bIsShow);

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
		 * @brief 获取当前选区
		 * @details 用于获取当前选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetCurSel() const;  
		/**
		 * @brief 执行 InitCursel 操作
		 * @details 用于执行 InitCursel 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void InitCursel();
		/**
		 * @brief 选中指定子项
		 * @details 用于选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bTakeFocus [in] 是否设置输入焦点
		 * @param bIsClick [in] 是否按点击方式处理
		 * @param bSetScrollPos [in] 是否同步滚动到目标位置
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SelectItem(int iIndex, bool bTakeFocus = false, bool bIsClick=false, bool bSetScrollPos =true);
		/**
		 * @brief 执行 SendNotifyClick 操作
		 * @details 用于执行 SendNotifyClick 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SendNotifyClick();
		/**
		 * @brief 选中指定多选子项
		 * @details 用于选中指定多选子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bTakeFocus [in] 是否设置输入焦点
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SelectMultiItem(int iIndex, bool bTakeFocus = false);
		/**
		 * @brief 取消选中指定子项
		 * @details 用于取消选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bOthers [in] 是否同时处理其他项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool UnSelectItem(int iIndex, bool bOthers = false);
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
		 * @param bChildDelayed [in] 是否延迟处理子项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Remove(CControlUI* pControl,bool bChildDelayed =true);
		/**
		 * @brief 移除At
		 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bChildDelayed [in] 是否延迟处理子项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveAt(int iIndex,bool bChildDelayed =true);
		/**
		 * @brief 移除全部子项
		 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bChildDelayed [in] 是否延迟处理子项
		 */
		void RemoveAll(bool bChildDelayed =true);

		/**
		 * @brief 激活对象
		 * @details 用于激活对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Activate();

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
		 * @brief 获取Disabled图像
		 * @details 用于获取Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetDisabledImage() const;
		/**
		 * @brief 设置Disabled图像
		 * @details 用于设置Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetDisabledImage(std::wstring_view pStrImage);

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
		 * @brief 设置子项字体
		 * @details 用于设置子项字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetItemFont(int index);
		/**
		 * @brief 设置子项文本样式
		 * @details 设置下拉子项文本的 `DT_*` 文本格式标志，常用于控制左右对齐、垂直对齐、
		 * 单行显示、多行换行和省略号行为，具体语义与 `CRenderEngine::DrawText` 的 `uStyle` 保持一致。
		 * @param uStyle [in] 文本样式标志
		 */
		void SetItemTextStyle(UINT uStyle);
		/**
		 * @brief 获取子项文本内边距
		 * @details 用于获取子项文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetItemTextPadding() const;
		/**
		 * @brief 设置子项文本内边距
		 * @details 用于设置子项文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void SetItemTextPadding(RECT rc);
		/**
		 * @brief 获取子项文本颜色
		 * @details 用于获取子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemTextColor() const;
		/**
		 * @brief 设置子项文本颜色
		 * @details 用于设置子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetItemTextColor(DWORD dwTextColor);
		/**
		 * @brief 获取子项背景颜色
		 * @details 用于获取子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemBkColor() const;
		/**
		 * @brief 设置子项背景颜色
		 * @details 用于设置子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetItemBkColor(DWORD dwBkColor);
		/**
		 * @brief 获取子项背景图像
		 * @details 用于获取子项背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetItemBkImage() const;
		/**
		 * @brief 设置子项背景图像
		 * @details 用于设置子项背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetItemBkImage(std::wstring_view pStrImage);
		/**
		 * @brief 判断是否Alternate背景
		 * @details 用于判断是否Alternate背景。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsAlternateBk() const;
		/**
		 * @brief 设置Alternate背景
		 * @details 用于设置Alternate背景。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bAlternateBk [in] 是否Alternate背景
		 */
		void SetAlternateBk(bool bAlternateBk);
		/**
		 * @brief 获取Selected子项文本颜色
		 * @details 用于获取Selected子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectedItemTextColor() const;
		/**
		 * @brief 设置Selected子项文本颜色
		 * @details 用于设置Selected子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetSelectedItemTextColor(DWORD dwTextColor);
		/**
		 * @brief 获取Selected子项背景颜色
		 * @details 用于获取Selected子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelectedItemBkColor() const;
		/**
		 * @brief 设置Selected子项背景颜色
		 * @details 用于设置Selected子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetSelectedItemBkColor(DWORD dwBkColor);
		/**
		 * @brief 获取Selected子项图像
		 * @details 用于获取Selected子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetSelectedItemImage() const;
		/**
		 * @brief 设置Selected子项图像
		 * @details 用于设置Selected子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetSelectedItemImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取热状态子项文本颜色
		 * @details 用于获取热状态子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetHotItemTextColor() const;
		/**
		 * @brief 设置热状态子项文本颜色
		 * @details 用于设置热状态子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetHotItemTextColor(DWORD dwTextColor);
		/**
		 * @brief 获取热状态子项背景颜色
		 * @details 用于获取热状态子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetHotItemBkColor() const;
		/**
		 * @brief 设置热状态子项背景颜色
		 * @details 用于设置热状态子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetHotItemBkColor(DWORD dwBkColor);
		/**
		 * @brief 获取热状态子项图像
		 * @details 用于获取热状态子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetHotItemImage() const;
		/**
		 * @brief 设置热状态子项图像
		 * @details 用于设置热状态子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetHotItemImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Disabled子项文本颜色
		 * @details 用于获取Disabled子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisabledItemTextColor() const;
		/**
		 * @brief 设置Disabled子项文本颜色
		 * @details 用于设置Disabled子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetDisabledItemTextColor(DWORD dwTextColor);
		/**
		 * @brief 获取Disabled子项背景颜色
		 * @details 用于获取Disabled子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisabledItemBkColor() const;
		/**
		 * @brief 设置Disabled子项背景颜色
		 * @details 用于设置Disabled子项背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBkColor [in] 背景颜色值
		 */
		void SetDisabledItemBkColor(DWORD dwBkColor);
		/**
		 * @brief 获取Disabled子项图像
		 * @details 用于获取Disabled子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetDisabledItemImage() const;
		/**
		 * @brief 设置Disabled子项图像
		 * @details 用于设置Disabled子项图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetDisabledItemImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取子项行颜色
		 * @details 用于获取子项行颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemLineColor() const;
		/**
		 * @brief 设置子项行颜色
		 * @details 用于设置子项行颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwLineColor [in] 行颜色数值
		 */
		void SetItemLineColor(DWORD dwLineColor);
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
		 * @brief 判断是否显示Selected子项文本
		 * @details 用于判断是否显示Selected子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsShowSelectedItemText();
		/**
		 * @brief 设置显示Selected子项文本
		 * @details 用于设置显示Selected子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bIsShow [in] 是否判断是否显示
		 */
		void SetShowSelectedItemText(bool bIsShow);


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
		 * @return CComboUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CComboUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CComboUI* pControl) ;

	public:
		/**
		 * @brief 执行 SortItems 操作
		 * @details 用于执行 SortItems 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SortItems();
		/**
		 * @brief 执行 SortItems 操作
		 * @details 用于执行 SortItems 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pfnCompare [in] pfnCompare参数
		 * @param dwData [in] 数据数值
		 * @return BOOL 返回 执行 SortItems 操作 的结果
		 */
		BOOL SortItems(PULVCompareFunc pfnCompare, UINT_PTR dwData);

	protected:
		/**
		 * @brief 执行 ItemComareFunc 操作
		 * @details 用于执行 ItemComareFunc 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pvlocale [in] pvlocale参数
		 * @param item1 [in] 子项1参数
		 * @param item2 [in] 子项2参数
		 * @return int __cdecl 返回 执行 ItemComareFunc 操作 的结果
		 */
		static int __cdecl ItemComareFunc(void* pvlocale, const void* item1, const void* item2);
		/**
		 * @brief 执行 ItemComareFunc 操作
		 * @details 用于执行 ItemComareFunc 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param item1 [in] 子项1参数
		 * @param item2 [in] 子项2参数
		 * @return int __cdecl 返回 执行 ItemComareFunc 操作 的结果
		 */
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



