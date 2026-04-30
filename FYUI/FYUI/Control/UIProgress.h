#pragma once

namespace FYUI
{
	class FYUI_API CProgressUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CProgressUI)
	public:
		/**
		 * @brief 构造 CProgressUI 对象
		 * @details 用于构造 CProgressUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CProgressUI();

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
		 * @brief 判断是否显示文本
		 * @details 用于判断是否显示文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsShowText();
		/**
		 * @brief 设置显示文本
		 * @details 用于设置显示文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShowText [in] 是否显示文本
		 */
		void SetShowText(bool bShowText = true);
		/**
		 * @brief 判断是否水平
		 * @details 用于判断是否水平。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsHorizontal();
		/**
		 * @brief 设置水平
		 * @details 用于设置水平。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bHorizontal [in] 是否水平
		 */
		void SetHorizontal(bool bHorizontal = true);
		/**
		 * @brief 判断是否Stretch前景图像
		 * @details 用于判断是否Stretch前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsStretchForeImage();
		/**
		 * @brief 设置Stretch前景图像
		 * @details 用于设置Stretch前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bStretchForeImage [in] 是否Stretch前景图像
		 */
		void SetStretchForeImage(bool bStretchForeImage = true);
		/**
		 * @brief 获取最小值
		 * @details 用于获取最小值。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMinValue() const;
		/**
		 * @brief 设置最小值
		 * @details 用于设置最小值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nMin [in] 最小数值
		 */
		void SetMinValue(int nMin);
		/**
		 * @brief 获取最大值
		 * @details 用于获取最大值。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMaxValue() const;
		/**
		 * @brief 设置最大值
		 * @details 用于设置最大值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nMax [in] 最大数值
		 */
		void SetMaxValue(int nMax);
		/**
		 * @brief 获取值
		 * @details 用于获取值。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetValue() const;
		/**
		 * @brief 设置值
		 * @details 用于设置值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nValue [in] 值数值
		 */
		void SetValue(int nValue);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 绘制前景颜色
		 * @details 用于绘制前景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintForeColor(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制前景图像
		 * @details 用于绘制前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintForeImage(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 更新文本
		 * @details 用于更新文本。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void UpdateText();
		/**
		 * @brief 设置前景图像内边距
		 * @details 用于设置前景图像内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szForeImage [in] 前景图像尺寸参数
		 */
		void SetForeImagePadding(SIZE szForeImage);

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CProgressUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CProgressUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CProgressUI* pControl) ;


	protected:
		bool m_bShowText;
		bool m_bHorizontal;
		bool m_bStretchForeImage;
		int m_nMax;
		int m_nMin;
		int m_nValue;
		SIZE m_szForeImage;
		std::wstring m_sForeImageModify;
	};

} 


