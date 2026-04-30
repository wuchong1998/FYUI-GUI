#pragma once

#include <chrono>
#include <string_view>

namespace FYUI
{
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class IContainerUI
	{
	public:
		/**
		 * @brief 获取子项At
		 * @details 用于获取子项At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CControlUI* GetItemAt(int iIndex) const = 0;
		/**
		 * @brief 获取子项索引
		 * @details 用于获取子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return int 返回对应的数值结果
		 */
		virtual int GetItemIndex(CControlUI* pControl) const = 0;
		/**
		 * @brief 设置子项索引
		 * @details 用于设置子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @param bUpdate [in] 是否更新
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate = true) = 0;
		/**
		 * @brief 获取数量
		 * @details 用于获取数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetCount() const = 0;
		/**
		 * @brief 执行 Add 操作
		 * @details 用于执行 Add 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Add(CControlUI* pControl) = 0;
		/**
		 * @brief 添加At
		 * @details 用于添加At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool AddAt(CControlUI* pControl, int iIndex) = 0;
		/**
		 * @brief 执行 Remove 操作
		 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param bChildDelayed [in] 是否延迟处理子项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Remove(CControlUI* pControl, bool bChildDelayed = true) = 0;
		/**
		 * @brief 移除At
		 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bChildDelayed [in] 是否延迟处理子项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool RemoveAt(int iIndex, bool bChildDelayed = true) = 0;
		/**
		 * @brief 移除全部子项
		 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bChildDelayed [in] 是否延迟处理子项
		 */
		virtual void RemoveAll(bool bChildDelayed = true) = 0;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//
	class CScrollBarUI;

	class FYUI_API CContainerUI : public CControlUI, public IContainerUI
	{
		DECLARE_DUICONTROL(CContainerUI)

	public:
		/**
		 * @brief 构造 CContainerUI 对象
		 * @details 用于构造 CContainerUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CContainerUI();
		/**
		 * @brief 析构 CContainerUI 对象
		 * @details 用于析构 CContainerUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CContainerUI();

	public:
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
		 * @param bChildDelayed [in] 是否延迟处理子项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Remove(CControlUI* pControl, bool bChildDelayed = true);
		/**
		 * @brief 移除At
		 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bChildDelayed [in] 是否延迟处理子项
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveAt(int iIndex, bool bChildDelayed = true);
		/**
		 * @brief 移除全部子项
		 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bChildDelayed [in] 是否延迟处理子项
		 */
		void RemoveAll(bool bChildDelayed = true);

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 设置可见状态
		 * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 * @param bSendFocus [in] 是否Send焦点
		 */
		void SetVisible(bool bVisible = true, bool bSendFocus = true);
		/**
		 * @brief 设置内部可见状态
		 * @details 用于设置内部可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 */
		void SetInternVisible(bool bVisible = true);
		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnabled [in] 是否启用状态
		 */
		void SetEnabled(bool bEnabled);
		/**
		 * @brief 设置鼠标启用状态
		 * @details 用于设置鼠标启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetMouseEnabled(bool bEnable = true);

		/**
		 * @brief 获取Inset
		 * @details 用于获取Inset。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		virtual RECT GetInset() const;
		/**
		 * @brief 设置Inset
		 * @details 用于设置Inset。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcInset [in] Inset矩形区域
		 */
		virtual void SetInset(RECT rcInset); // 璁剧疆鍐呰竟璺濓紝鐩稿綋浜庤缃鎴峰尯
		virtual int GetChildPadding() const;
		/**
		 * @brief 设置子控件内边距
		 * @details 用于设置子控件内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iPadding [in] 内边距值
		 */
		virtual void SetChildPadding(int iPadding);
		/**
		 * @brief 获取子控件对齐方式
		 * @details 用于获取子控件对齐方式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetChildAlign() const;
		/**
		 * @brief 设置子控件对齐方式
		 * @details 用于设置子控件对齐方式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iAlign [in] 对齐方式值
		 */
		virtual void SetChildAlign(UINT iAlign);
		/**
		 * @brief 获取子控件V对齐方式
		 * @details 用于获取子控件V对齐方式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetChildVAlign() const;
		/**
		 * @brief 设置子控件V对齐方式
		 * @details 用于设置子控件V对齐方式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iVAlign [in] V对齐方式值
		 */
		virtual void SetChildVAlign(UINT iVAlign);
		/**
		 * @brief 判断是否Auto销毁
		 * @details 用于判断是否Auto销毁。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsAutoDestroy() const;
		/**
		 * @brief 设置Auto销毁
		 * @details 用于设置Auto销毁。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bAuto [in] 是否Auto
		 */
		virtual void SetAutoDestroy(bool bAuto);
		/**
		 * @brief 判断是否延迟销毁
		 * @details 用于判断是否延迟销毁。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsDelayedDestroy() const;
		/**
		 * @brief 设置延迟销毁
		 * @details 用于设置延迟销毁。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bDelayed [in] 是否延迟
		 */
		virtual void SetDelayedDestroy(bool bDelayed);


		/**
		 * @brief 判断是否鼠标子控件启用状态
		 * @details 用于判断是否鼠标子控件启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsMouseChildEnabled() const;
		/**
		 * @brief 设置鼠标子控件启用状态
		 * @details 用于设置鼠标子控件启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		virtual void SetMouseChildEnabled(bool bEnable = true);
		/**
		 * @brief 判断是否Container控件
		 * @details 用于判断是否Container控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsContainerControl() const override
		{
			return true;
		}

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CContainerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CContainerUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CContainerUI* pControl);

		/**
		 * @brief 设置H滚动Space
		 * @details 用于设置H滚动Space。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 */
		void SetHScrollSpace(int cx);
		/**
		 * @brief 设置V滚动Space
		 * @details 用于设置V滚动Space。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cy [in] cy参数
		 */
		void SetVScrollSpace(int cy);

		/**
		 * @brief 设置光标鼠标
		 * @details 用于设置光标鼠标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bCursorMouse [in] 是否光标鼠标
		 */
		void SetCursorMouse(bool bCursorMouse);

		/**
		 * @brief 判断是否固定Scrollbar
		 * @details 用于判断是否固定Scrollbar。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsFixedScrollbar();
		/**
		 * @brief 设置固定Scrollbar
		 * @details 用于设置固定Scrollbar。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bFixed [in] 是否固定
		 */
		void SetFixedScrollbar(bool bFixed);

		/**
		 * @brief 判断是否显示Scrollbar
		 * @details 用于判断是否显示Scrollbar。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsShowScrollbar();
		/**
		 * @brief 设置显示Scrollbar
		 * @details 用于设置显示Scrollbar。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShow [in] 是否显示
		 */
		void SetShowScrollbar(bool bShow);

		/**
		 * @brief 设置SmoothScrollbar
		 * @details 用于设置SmoothScrollbar。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSmooth [in] 是否Smooth
		 */
		void SetSmoothScrollbar(bool bSmooth);
		/**
		 * @brief 判断是否SmoothScrollbar
		 * @details 用于判断是否SmoothScrollbar。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsSmoothScrollbar();

		/**
		 * @brief 设置QuickScrolling
		 * @details 用于设置QuickScrolling。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bQuickScrolling [in] 是否QuickScrolling
		 */
		void SetQuickScrolling(bool bQuickScrolling);
		/**
		 * @brief 获取QuickScrolling
		 * @details 用于获取QuickScrolling。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetQuickScrolling();

		/**
		 * @brief 查找Selectable
		 * @details 用于查找Selectable。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bForward [in] 是否Forward
		 * @return int 返回对应的数值结果
		 */
		virtual int FindSelectable(int iIndex, bool bForward = true) const;

		/**
		 * @brief 获取客户区位置
		 * @details 用于获取客户区位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetClientPos() const;
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
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 设置分隔立即边框颜色
		 * @details 用于设置分隔立即边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwSepImmBorderColor [in] 分隔立即边框颜色数值
		 */
		void SetSepImmBorderColor(DWORD dwSepImmBorderColor);
		/**
		 * @brief 获取分隔立即边框颜色
		 * @details 用于获取分隔立即边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSepImmBorderColor() const;
		/**
		 * @brief 设置分隔立即Leave边框颜色
		 * @details 用于设置分隔立即Leave边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwSepImmBorderColor [in] 分隔立即边框颜色数值
		 */
		void SetSepImmLeaveBorderColor(DWORD dwSepImmBorderColor);
		/**
		 * @brief 获取分隔立即Leave边框颜色
		 * @details 用于获取分隔立即Leave边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSepImmLeaveBorderColor() const;

		/**
		 * @brief 设置管理器和父级关系
		 * @details 用于设置管理器和父级关系。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param pParent [in] 父级控件对象
		 * @param bInit [in] 是否执行初始化
		 */
		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
		/**
		 * @brief 查找控件
		 * @details 用于查找控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param Proc [in] Proc参数
		 * @param pData [in] 数据对象
		 * @param uFlags [in] Flags标志
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

		//浼樺寲鐗堟湰FindSubControl锛岃姣斾箣鍓嶇増鏈殑FindSubControl蹇?0鍊嶉€熷害浠ヤ笂
		/**
		 * @brief 查找Sub控件
		 * @details 用于查找Sub控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRoot [in] Root对象
		 * @param strSubControlName [in] Sub控件名称字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindSubControl(CControlUI* pRoot, std::wstring_view strSubControlName);
		/**
		 * @brief 查找Sub控件
		 * @details 用于查找Sub控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRoot [in] Root对象
		 * @param strSubControlName [in] Sub控件名称字符串
		 * @param strEliminateControlName [in] Eliminate控件名称字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindSubControl(CControlUI* pRoot, std::wstring_view strSubControlName, std::wstring_view strEliminateControlName);
		/**
		 * @brief 设置Sub控件文本
		 * @details 用于设置Sub控件文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @param pstrText [in] 文本字符串
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetSubControlText(std::wstring_view pstrSubControlName, std::wstring_view pstrText);
		/**
		 * @brief 设置Sub控件固定高度
		 * @details 用于设置Sub控件固定高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @param cy [in] cy参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetSubControlFixedHeight(std::wstring_view pstrSubControlName, int cy);
		/**
		 * @brief 设置Sub控件固定Wdith
		 * @details 用于设置Sub控件固定Wdith。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @param cx [in] cx参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetSubControlFixedWdith(std::wstring_view pstrSubControlName, int cx);
		/**
		 * @brief 设置Sub控件用户数据
		 * @details 用于设置Sub控件用户数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @param pstrText [in] 文本字符串
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetSubControlUserData(std::wstring_view pstrSubControlName, std::wstring_view pstrText);
		/**
		 * @brief 设置Sub控件属性
		 * @details 用于设置Sub控件属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetSubControlAttribute(std::wstring_view pstrSubControlName, std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 获取Sub控件文本
		 * @details 用于获取Sub控件文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetSubControlText(std::wstring_view pstrSubControlName);
		/**
		 * @brief 获取Sub控件固定高度
		 * @details 用于获取Sub控件固定高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @return int 返回对应的数值结果
		 */
		int GetSubControlFixedHeight(std::wstring_view pstrSubControlName);
		/**
		 * @brief 获取Sub控件固定Wdith
		 * @details 用于获取Sub控件固定Wdith。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @return int 返回对应的数值结果
		 */
		int GetSubControlFixedWdith(std::wstring_view pstrSubControlName);
		/**
		 * @brief 获取Sub控件用户数据
		 * @details 用于获取Sub控件用户数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetSubControlUserData(std::wstring_view pstrSubControlName);
		/*
		** 鏍规嵁鍚嶅瓧鏌ユ壘瀛愭帶浠讹紝杩斿洖鎵惧埌鐨勭涓€涓帶浠舵寚閽?
		*  param pstrSubControlName: 瀛愭帶浠跺悕瀛?
		*  param strEliminateControlName: 鎺掗櫎鏌愪釜鍚嶅瓧鐨勬帶浠?涓嶄細鍦ㄨ鍚嶇О鎺т欢杩涜瀛愭帶浠舵煡鎵?
		*  return CControlUI*: 鎵惧埌鐨勫瓙鎺т欢鎸囬拡锛屾湭鎵惧埌杩斿洖nullptr
		*/
		/**
		 * @brief 查找Sub控件
		 * @details 用于查找Sub控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSubControlName [in] Sub控件名称字符串
		 * @param strEliminateControlName [in] Eliminate控件名称字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindSubControl(std::wstring_view pstrSubControlName, std::wstring_view strEliminateControlName = {});

		/*
		** 鏍规嵁鍚嶅瓧鏌ユ壘瀛愭帶浠讹紝鍙煡鎵?m_items 閲岀殑瀛愭帶浠讹紝杩斿洖鎵惧埌鐨勭涓€涓帶浠舵寚閽?
		*  param pstrChildControlName: 瀛愭帶浠跺悕瀛?
		*  return CControlUI*: 鎵惧埌鐨勫瓙鎺т欢鎸囬拡锛屾湭鎵惧埌杩斿洖nullptr
		*/
		/**
		 * @brief 查找Itme控件
		 * @details 用于查找Itme控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrChildControlName [in] 子控件控件名称字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CControlUI* FindItmeControl(std::wstring_view pstrChildControlName);

		/**
		 * @brief 获取滚动位置
		 * @details 用于获取滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		virtual SIZE GetScrollPos() const;
		/**
		 * @brief 获取滚动Range
		 * @details 用于获取滚动Range。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		virtual SIZE GetScrollRange() const;
		/**
		 * @brief 设置滚动位置
		 * @details 用于设置滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szPos [in] 位置尺寸参数
		 * @param bMsg [in] 是否Msg
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		virtual void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true);
		/**
		 * @brief 设置滚动步长尺寸
		 * @details 用于设置滚动步长尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSize [in] 尺寸数值
		 */
		virtual void SetScrollStepSize(int nSize);
		/**
		 * @brief 获取滚动步长尺寸
		 * @details 用于获取滚动步长尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetScrollStepSize() const;
		/**
		 * @brief 执行 LineUp 操作
		 * @details 用于执行 LineUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		virtual void LineUp(bool bScroolVisible = true);
		/**
		 * @brief 执行 LineDown 操作
		 * @details 用于执行 LineDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		virtual void LineDown(bool bScroolVisible = true);
		/**
		 * @brief 执行 PageUp 操作
		 * @details 用于执行 PageUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void PageUp();
		/**
		 * @brief 执行 PageDown 操作
		 * @details 用于执行 PageDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void PageDown();
		/**
		 * @brief 执行 HomeUp 操作
		 * @details 用于执行 HomeUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void HomeUp();
		/**
		 * @brief 执行 EndDown 操作
		 * @details 用于执行 EndDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void EndDown();
		/**
		 * @brief 执行 LineLeft 操作
		 * @details 用于执行 LineLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void LineLeft();
		/**
		 * @brief 执行 LineRight 操作
		 * @details 用于执行 LineRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void LineRight();
		/**
		 * @brief 执行 PageLeft 操作
		 * @details 用于执行 PageLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void PageLeft();
		/**
		 * @brief 执行 PageRight 操作
		 * @details 用于执行 PageRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void PageRight();
		/**
		 * @brief 执行 HomeLeft 操作
		 * @details 用于执行 HomeLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void HomeLeft();
		/**
		 * @brief 执行 EndRight 操作
		 * @details 用于执行 EndRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void EndRight();
		/**
		 * @brief 启用滚动Bar
		 * @details 用于启用滚动Bar。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnableVertical [in] 是否启用垂直
		 * @param bEnableHorizontal [in] 是否启用水平
		 */
		virtual void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
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
	public:

		/**
		 * @brief 获取NowTime
		 * @details 用于获取NowTime。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetNowTime();


	public:
		UINT m_uState = 0;      

	protected:
		/**
		 * @brief 设置浮动位置
		 * @details 用于设置浮动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 */
		virtual void SetFloatPos(int iIndex);
		/**
		 * @brief 执行 ProcessScrollBar 操作
		 * @details 用于执行 ProcessScrollBar 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param cxRequired [in] cxRequired参数
		 * @param cyRequired [in] cyRequired参数
		 */
		virtual void ProcessScrollBar(RECT rc, int cxRequired, int cyRequired);
		/**
		 * @brief 获取滚动视图矩形
		 * @details 用于获取滚动视图矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetScrollViewRect() const;
		/**
		 * @brief 执行 ClampScrollPos 操作
		 * @details 用于执行 ClampScrollPos 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szPos [in] 位置尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE ClampScrollPos(SIZE szPos) const;
		/**
		 * @brief 获取Wheel滚动步长
		 * @details 用于获取Wheel滚动步长。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bHorizontal [in] 是否水平
		 * @return int 返回对应的数值结果
		 */
		int GetWheelScrollStep(bool bHorizontal) const;
		/**
		 * @brief 检查是否可以Use滚动渲染缓存
		 * @details 用于检查是否可以Use滚动渲染缓存。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcScrollView [in] 滚动视图矩形区域
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool CanUseScrollRenderCache(const RECT& rcScrollView) const;
		/**
		 * @brief 应用滚动位置
		 * @details 用于应用滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szPos [in] 位置尺寸参数
		 * @param bMsg [in] 是否Msg
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		void ApplyScrollPos(SIZE szPos, bool bMsg, bool bScroolVisible);
		/**
		 * @brief 执行 QueueSmoothScroll 操作
		 * @details 用于执行 QueueSmoothScroll 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szTarget [in] 目标尺寸参数
		 * @param bMsg [in] 是否Msg
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 */
		void QueueSmoothScroll(SIZE szTarget, bool bMsg = true, bool bScroolVisible = true);
		/**
		 * @brief 滚动ByDelta
		 * @details 用于滚动ByDelta。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cxDelta [in] cxDelta参数
		 * @param cyDelta [in] cyDelta参数
		 * @param bMsg [in] 是否Msg
		 * @param bScroolVisible [in] 是否Scrool可见状态
		 * @param bPreferSmooth [in] 是否PreferSmooth
		 */
		void ScrollByDelta(int cxDelta, int cyDelta, bool bMsg = true, bool bScroolVisible = true, bool bPreferSmooth = true);
		/**
		 * @brief 执行 AdvanceSmoothScroll 操作
		 * @details 用于执行 AdvanceSmoothScroll 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AdvanceSmoothScroll();
		/**
		 * @brief 执行 StopSmoothScroll 操作
		 * @details 用于执行 StopSmoothScroll 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSnapToTarget [in] 是否SnapTo目标
		 */
		void StopSmoothScroll(bool bSnapToTarget = false);


	protected:

		CStdPtrArray m_items;
		RECT m_rcInset;
		int m_iChildPadding;
		int m_nScrollStepSize;
		UINT m_iChildAlign;
		UINT m_iChildVAlign;
		bool m_bAutoDestroy;
		bool m_bDelayedDestroy;
		bool m_bMouseChildEnabled;
		bool m_bFixedScrollbar;
		bool m_bShowScrollbar;
		bool m_bSmoothScrollbar;
		bool m_bCursorMouse;
		bool m_bQuickScrolling = true;



		CScrollBarUI* m_pVerticalScrollBar;
		CScrollBarUI* m_pHorizontalScrollBar;
		std::wstring m_sVerticalScrollBarStyle;
		std::wstring m_sHorizontalScrollBarStyle;

	protected:

		DWORD m_dwSepImmBorderColor = 0;
		DWORD m_dwLeaveSepImmBorderColor = 0;

		
		bool   m_bInertiaActive;
		bool m_bSmoothScrollAnimating = false;
		bool m_bApplyingSmoothScroll = false;
		bool m_bSmoothScrollNotify = true;
		bool m_bSmoothScrollVisible = true;
		bool m_bSmoothScrollPeriodRaised = false;
		SIZE m_szSmoothScrollTarget = { 0, 0 };
		double m_fSmoothScrollPosX = 0.0;
		double m_fSmoothScrollPosY = 0.0;
		double m_fWheelRemainderX = 0.0;
		double m_fWheelRemainderY = 0.0;
		std::chrono::steady_clock::time_point m_smoothScrollLastTick;

		enum : UINT
		{
			SMOOTH_SCROLL_TIMERID = 0x5F10,
			SMOOTH_SCROLL_INTERVAL_MS = 4,
		};

	};

} 


