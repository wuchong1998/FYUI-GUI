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
		 * @details 按指定文本格式绘制按钮文本，`uStyle` 的含义与 `CRenderEngine::DrawText` 保持一致，
		 * 可使用 `DT_LEFT`、`DT_CENTER`、`DT_RIGHT`、`DT_VCENTER`、`DT_SINGLELINE`、
		 * `DT_WORDBREAK`、`DT_END_ELLIPSIS`、`DT_NOPREFIX` 等常见 `DT_*` 标志。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in,out] 矩形区域
		 * @param pstrText [in] 文本字符串
		 * @param dwTextColor [in] 文本颜色值
		 * @param iFont [in] 字体值
		 * @param uStyle [in] 文本样式标志
		 */
		void    DrawTextExBtn(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view pstrText, DWORD dwTextColor, int iFont, UINT uStyle);
		/**
		 * @brief 绘制文本ExBtn
		 * @details 该重载用于兼容 `std::wstring` 形式的按钮文本输入，`uStyle` 的语义与主重载一致。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in,out] 矩形区域
		 * @param pstrText [in] 文本字符串
		 * @param dwTextColor [in] 文本颜色值
		 * @param iFont [in] 字体值
		 * @param uStyle [in] 文本样式标志
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

