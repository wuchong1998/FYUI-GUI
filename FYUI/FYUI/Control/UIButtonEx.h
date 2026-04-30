#pragma once

namespace FYUI
{
	class FYUI_API CButtonExUI : public CButtonUI
	{
		DECLARE_DUICONTROL(CButtonExUI)

	public:
		/**
		 * @brief 构造 CButtonExUI 对象
		 * @details 用于构造 CButtonExUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CButtonExUI();

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
		LPVOID  GetInterface(std::wstring_view pstrName);
		/**
		 * @brief 绘制文本内容
		 * @details 用于绘制文本内容。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void    PaintText(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制文本ExBtn
		 * @details 用于绘制文本ExBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in,out] 矩形区域
		 * @param pstrText [in] 文本字符串
		 * @param dwTextColor [in] 文本颜色值
		 * @param iFont [in] 字体值
		 * @param uStyle [in] 样式标志
		 */
		void    DrawTextExBtn(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view pstrText, DWORD dwTextColor, int iFont, UINT uStyle);
		/**
		 * @brief 绘制文本ExBtn
		 * @details 用于绘制文本ExBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in,out] 矩形区域
		 * @param pstrText [in] 文本字符串
		 * @param dwTextColor [in] 文本颜色值
		 * @param iFont [in] 字体值
		 * @param uStyle [in] 样式标志
		 */
		void    DrawTextExBtn(CPaintRenderContext& renderContext, RECT& rc, const std::wstring& pstrText, DWORD dwTextColor, int iFont, UINT uStyle)
		{
			DrawTextExBtn(renderContext, rc, std::wstring_view(pstrText), dwTextColor, iFont, uStyle);
		}

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CButtonExUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CButtonExUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CButtonExUI* pControl) ;

	};

}

