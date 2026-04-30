#pragma once
namespace FYUI
{
	class FYUI_API CButtonUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CButtonUI)

	public:
		/**
		 * @brief 构造 CButtonUI 对象
		 * @details 用于构造 CButtonUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CButtonUI();

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
		 * @brief 激活对象
		 * @details 用于激活对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Activate();
		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetEnabled(bool bEnable = true);
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);

		/**
		 * @brief 获取Normal图像
		 * @details 用于获取Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetNormalImage() const;
		/**
		 * @brief 设置Normal图像
		 * @details 用于设置Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取热状态图像
		 * @details 用于获取热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetHotImage() const;
		/**
		 * @brief 设置热状态图像
		 * @details 用于设置热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Pushed图像
		 * @details 用于获取Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetPushedImage() const;
		/**
		 * @brief 设置Pushed图像
		 * @details 用于设置Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetPushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Focused图像
		 * @details 用于获取Focused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetFocusedImage() const;
		/**
		 * @brief 设置Focused图像
		 * @details 用于设置Focused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetFocusedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Disabled图像
		 * @details 用于获取Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetDisabledImage() const;
		/**
		 * @brief 设置Disabled图像
		 * @details 用于设置Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetDisabledImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取热状态前景图像
		 * @details 用于获取热状态前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetHotForeImage() const;
		/**
		 * @brief 设置热状态前景图像
		 * @details 用于设置热状态前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetHotForeImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Pushed前景图像
		 * @details 用于获取Pushed前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetPushedForeImage() const;
		/**
		 * @brief 设置Pushed前景图像
		 * @details 用于设置Pushed前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetPushedForeImage(std::wstring_view pStrImage);
		/**
		 * @brief 设置状态数量
		 * @details 用于设置状态数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nCount [in] 数量数值
		 */
		void SetStateCount(int nCount);
		/**
		 * @brief 获取状态数量
		 * @details 用于获取状态数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetStateCount() const;
		/**
		 * @brief 设置状态
		 * @details 用于设置状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uButtonState [in] 按钮状态标志
		 */
		void SetState(UINT uButtonState);
		/**
		 * @brief 获取状态图像
		 * @details 用于获取状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetStateImage() const;
		/**
		 * @brief 设置状态图像
		 * @details 用于设置状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		virtual void SetStateImage(std::wstring_view pStrImage);

		/**
		 * @brief 执行 BindTabIndex 操作
		 * @details 用于执行 BindTabIndex 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _BindTabIndex [in] Bind页签索引参数
		 */
		void BindTabIndex(int _BindTabIndex);
		/**
		 * @brief 执行 BindTabLayoutName 操作
		 * @details 用于执行 BindTabLayoutName 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _TabLayoutName [in] 页签布局名称参数
		 */
		void BindTabLayoutName(std::wstring_view _TabLayoutName);
		/**
		 * @brief 执行 BindTriggerTabSel 操作
		 * @details 用于执行 BindTriggerTabSel 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _SetSelectIndex [in] 设置选中索引参数
		 */
		void BindTriggerTabSel(int _SetSelectIndex = -1);
		/**
		 * @brief 移除Bind页签索引
		 * @details 用于移除Bind页签索引。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RemoveBindTabIndex();
		/**
		 * @brief 获取Bind页签布局索引
		 * @details 用于获取Bind页签布局索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int	 GetBindTabLayoutIndex();
		/**
		 * @brief 获取Bind页签布局名称
		 * @details 用于获取Bind页签布局名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetBindTabLayoutName() const;

		/**
		 * @brief 设置热状态字体
		 * @details 用于设置热状态字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetHotFont(int index);
		/**
		 * @brief 获取热状态字体
		 * @details 用于获取热状态字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetHotFont() const;
		/**
		 * @brief 设置Pushed字体
		 * @details 用于设置Pushed字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetPushedFont(int index);
		/**
		 * @brief 获取Pushed字体
		 * @details 用于获取Pushed字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetPushedFont() const;
		/**
		 * @brief 设置Focused字体
		 * @details 用于设置Focused字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetFocusedFont(int index);
		/**
		 * @brief 获取Focused字体
		 * @details 用于获取Focused字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetFocusedFont() const;

		/**
		 * @brief 设置热状态背景颜色
		 * @details 用于设置热状态背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetHotBkColor(DWORD dwColor);
		/**
		 * @brief 获取热状态背景颜色
		 * @details 用于获取热状态背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetHotBkColor() const;
		/**
		 * @brief 设置Pushed背景颜色
		 * @details 用于设置Pushed背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetPushedBkColor(DWORD dwColor);
		/**
		 * @brief 获取Pushed背景颜色
		 * @details 用于获取Pushed背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetPushedBkColor() const;
		/**
		 * @brief 设置Disabled背景颜色
		 * @details 用于设置Disabled背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetDisabledBkColor(DWORD dwColor);
		/**
		 * @brief 获取Disabled背景颜色
		 * @details 用于获取Disabled背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisabledBkColor() const;
		/**
		 * @brief 设置热状态文本颜色
		 * @details 用于设置热状态文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetHotTextColor(DWORD dwColor);
		/**
		 * @brief 获取热状态文本颜色
		 * @details 用于获取热状态文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetHotTextColor() const;
		/**
		 * @brief 设置Pushed文本颜色
		 * @details 用于设置Pushed文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetPushedTextColor(DWORD dwColor);
		/**
		 * @brief 获取Pushed文本颜色
		 * @details 用于获取Pushed文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetPushedTextColor() const;
		/**
		 * @brief 设置Focused文本颜色
		 * @details 用于设置Focused文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetFocusedTextColor(DWORD dwColor);
		/**
		 * @brief 获取Focused文本颜色
		 * @details 用于获取Focused文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetFocusedTextColor() const;

		/**
		 * @brief 设置热状态边框颜色
		 * @details 用于设置热状态边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetHotBorderColor(DWORD dwColor);
		/**
		 * @brief 获取热状态边框颜色
		 * @details 用于获取热状态边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetHotBorderColor() const;
		/**
		 * @brief 设置Pushed边框颜色
		 * @details 用于设置Pushed边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetPushedBorderColor(DWORD dwColor);
		/**
		 * @brief 获取Pushed边框颜色
		 * @details 用于获取Pushed边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetPushedBorderColor() const;
		/**
		 * @brief 设置Disabled边框颜色
		 * @details 用于设置Disabled边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 */
		void SetDisabledBorderColor(DWORD dwColor);
		/**
		 * @brief 获取Disabled边框颜色
		 * @details 用于获取Disabled边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisabledBorderColor() const;

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
		 * @brief 绘制背景颜色
		 * @details 用于绘制背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintBkColor(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制状态图像
		 * @details 用于绘制状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintStatusImage(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制边框
		 * @details 用于绘制边框。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintBorder(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制前景图像
		 * @details 用于绘制前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintForeImage(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 绘制边框
		 * @details 用于绘制边框。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 * @param dwBorderColor [in] 边框颜色数值
		 * @param nBorderSize [in] 边框尺寸数值
		 * @param rcBorderSize [in] 边框尺寸矩形区域
		 * @param cxyBorderRound [in] cxy边框圆角参数
		 * @param nBorderStyle [in] 边框样式数值
		 */
		void DrawBorder(CPaintRenderContext& renderContext, const RECT& rcItem, const DWORD& dwBorderColor, const int& nBorderSize, const RECT& rcBorderSize, const SIZE& cxyBorderRound, const int& nBorderStyle);

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CButtonUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CButtonUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CButtonUI* pControl) ;
		/**
		 * @brief 判断是否Container控件
		 * @details 用于判断是否Container控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsContainerControl() const override
		{
			return false;
		}
	protected:
		UINT m_uButtonState;

		int		m_iHotFont;
		int		m_iPushedFont;
		int		m_iFocusedFont;

		DWORD m_dwHotBkColor;
		DWORD m_dwPushedBkColor;
		DWORD m_dwDisabledBkColor;
		DWORD m_dwHotTextColor;
		DWORD m_dwPushedTextColor;
		DWORD m_dwFocusedTextColor;
		DWORD m_dwHotBorderColor;
		DWORD m_dwPushedBorderColor;
		DWORD m_dwDisabledBorderColor;

		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sHotForeImage;
		std::wstring m_sPushedImage;
		std::wstring m_sPushedForeImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sDisabledImage;
		int m_nStateCount;
		std::wstring m_sStateImage;

		int			m_iBindTabIndex;
		std::wstring	m_sBindTabLayoutName;
	};

}	


