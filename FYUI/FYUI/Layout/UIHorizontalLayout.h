#pragma once


namespace FYUI
{
	class FYUI_API CHorizontalLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CHorizontalLayoutUI)
	public:
		/**
		* @brief 构造 CHorizontalLayoutUI 对象
		* @details 用于构造 CHorizontalLayoutUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		*/
		CHorizontalLayoutUI();

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
		* @brief 设置分隔宽度
		* @details 用于设置分隔宽度。具体行为由当前对象状态以及传入参数共同决定。
		* @param iWidth [in] 宽度值
		*/
		void SetSepWidth(int iWidth);
		/**
		* @brief 获取分隔宽度
		* @details 用于获取分隔宽度。具体行为由当前对象状态以及传入参数共同决定。
		* @return int 返回对应的数值结果
		*/
		int GetSepWidth() const;
		/**
		* @brief 设置分隔立即Mode
		* @details 用于设置分隔立即Mode。具体行为由当前对象状态以及传入参数共同决定。
		* @param bImmediately [in] 是否Immediately
		*/
		void SetSepImmMode(bool bImmediately);
		/**
		* @brief 判断是否分隔立即Mode
		* @details 用于判断是否分隔立即Mode。具体行为由当前对象状态以及传入参数共同决定。
		* @return bool 操作成功返回 true，否则返回 false
		*/
		bool IsSepImmMode() const;
		/**
		 * @brief 设置拖拽虚影颜色
		 * @details 非即时模式（sepimm=false）下拖拽分隔条时在 DoPostPaint 中绘制的虚影填充颜色。
		 * @param dwColor [in] ARGB 颜色数值
		 */
		void SetSepGhostColor(DWORD dwColor);
		/**
		 * @brief 获取拖拽虚影颜色
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSepGhostColor() const;
		/**
		 * @brief 设置拖拽虚影厚度
		 * @details 非即时模式下虚影矩形在分隔条法线方向的厚度（逻辑像素）；<=0 表示沿用 sepwidth 的绝对值。
		 * @param iSize [in] 厚度（逻辑像素）
		 */
		void SetSepGhostSize(int iSize);
		/**
		 * @brief 获取拖拽虚影厚度
		 * @return int 返回对应的数值结果
		 */
		int GetSepGhostSize() const;

		/**
		* @brief 设置属性
		* @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		* @param pstrName [in] 属性名称
		* @param pstrValue [in] 属性值
		*/
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void DoEvent(TEventUI& event);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void DoPostPaint(CPaintRenderContext& renderContext) override;
		RECT GetThumbRect(bool bUseNew = false) const;

		/**
		 * @brief 判断是否处于鼠标悬停（Hot）状态
		 * @details 当鼠标悬停在分隔条上（即时模式下）时返回 true，供基类 PaintBorder 选用 HotBorderColor。
		 * @return bool 处于悬停态返回 true
		 */
		bool IsHot() const override;
		/**
		 * @brief 判断是否处于按下（Pushed）状态
		 * @details 当用户按住分隔条进行拖拽（CAPTURED）时返回 true，供基类 PaintBorder 选用 PushedBorderColor。
		 * @return bool 处于拖拽态返回 true
		 */
		bool IsPushed() const override;

		/**
		* @brief 克隆当前对象
		* @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		* @return CHorizontalLayoutUI* 返回结果对象指针，失败时返回 nullptr
		*/
		virtual CHorizontalLayoutUI* Clone();
		/**
		* @brief 复制对象数据
		* @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		* @param pControl [in] 控件对象
		*/
		virtual void CopyData(CHorizontalLayoutUI* pControl) ;

		/**
		* @brief 判断是否Container控件
		* @details 用于判断是否Container控件。具体行为由当前对象状态以及传入参数共同决定。
		* @return bool 操作成功返回 true，否则返回 false
		*/
		bool IsContainerControl() const override
		{
			return true;
		}

	protected:
		int m_iSepWidth;
		UINT m_uButtonState;
		POINT ptLastMouse;
		RECT m_rcNewPos;
		bool m_bImmMode;
		// 鼠标是否悬停在分隔条上，作为 IsHot() 的依据
		bool m_bSepHover = false;
		// 拖拽虚影颜色（sepimm=false 时生效），默认半透明黑
		DWORD m_dwSepGhostColor = 0xAA000000;
		// 拖拽虚影厚度（逻辑像素）；<=0 表示沿用 |m_iSepWidth|
		int m_iSepGhostSize = 0;


	};
}
