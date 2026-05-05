#pragma once
namespace FYUI
{
	class FYUI_API CLabelUI : public CControlUI
	{
		DECLARE_DUICONTROL(CLabelUI)
	public:
		/**
		 * @brief 构造 CLabelUI 对象
		 * @details 用于构造 CLabelUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CLabelUI();
		/**
		 * @brief 析构 CLabelUI 对象
		 * @details 用于析构 CLabelUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CLabelUI();

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
		 * @brief 设置文本样式
		 * @details 设置标签文本绘制时的 `DT_*` 文本格式标志。常用组合包括：
		 * `DT_LEFT`/`DT_CENTER`/`DT_RIGHT`、`DT_TOP`/`DT_VCENTER`/`DT_BOTTOM`、
		 * `DT_SINGLELINE`、`DT_WORDBREAK`、`DT_END_ELLIPSIS`、`DT_NOPREFIX`、`DT_CALCRECT`。
		 * 具体语义与 `CRenderEngine::DrawText` 的 `uStyle` 保持一致。
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
		 * @brief 设置Follow尺寸
		 * @details 用于设置Follow尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bFollowSize [in] 是否Follow尺寸
		 */
		void SetFollowSize(bool bFollowSize);
		/**
		 * @brief 获取Follow尺寸
		 * @details 用于获取Follow尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetFollowSize() const;


		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable);
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
		 * @brief 获取AutoCalc宽度
		 * @details 用于获取AutoCalc宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool GetAutoCalcWidth() const;
		/**
		 * @brief 设置AutoCalc宽度
		 * @details 用于设置AutoCalc宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bAutoCalcWidth [in] 是否AutoCalc宽度
		 */
		virtual void SetAutoCalcWidth(bool bAutoCalcWidth);
		/**
		 * @brief 获取AutoCalc高度
		 * @details 用于获取AutoCalc高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool GetAutoCalcHeight() const;
		/**
		 * @brief 设置AutoCalc高度
		 * @details 用于设置AutoCalc高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bAutoCalcHeight [in] 是否AutoCalc高度
		 */
		virtual void SetAutoCalcHeight(bool bAutoCalcHeight);
		using CControlUI::SetText;
		/**
		 * @brief 设置文本
		 * @details 用于设置文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrText [in] 文本字符串
		 */
		virtual void SetText(std::wstring_view pstrText) override;


		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CLabelUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CLabelUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CLabelUI* pControl);

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
		DWORD	m_dwTextColor;
		DWORD	m_dwDisabledTextColor;
		int		m_iFont;
		UINT	m_uTextStyle;
		RECT	m_rcTextPadding;
		bool	m_bShowHtml;
		bool	m_bAutoCalcWidth;
		bool	m_bAutoCalcHeight;
		bool    m_bFollowSize = false;
		SIZE    m_szAvailableLast;
		SIZE    m_cxyFixedLast;
		bool    m_bNeedEstimateSize;
	};

}

