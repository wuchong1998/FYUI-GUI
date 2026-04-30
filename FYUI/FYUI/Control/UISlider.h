#pragma once

namespace FYUI
{
	class FYUI_API CSliderUI : public CProgressUI
	{
		DECLARE_DUICONTROL(CSliderUI)
	public:
		/**
		 * @brief 构造 CSliderUI 对象
		 * @details 用于构造 CSliderUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CSliderUI();

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
		 * @brief 获取Change步长
		 * @details 用于获取Change步长。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetChangeStep();
		/**
		 * @brief 设置Change步长
		 * @details 用于设置Change步长。具体行为由当前对象状态以及传入参数共同决定。
		 * @param step [in] 步长参数
		 */
		void SetChangeStep(int step);
		/**
		 * @brief 设置滑块尺寸
		 * @details 用于设置滑块尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szXY [in] XY尺寸参数
		 */
		void SetThumbSize(SIZE szXY);
		/**
		 * @brief 获取滑块矩形
		 * @details 用于获取滑块矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetThumbRect() const;
		/**
		 * @brief 获取滑块图像
		 * @details 用于获取滑块图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetThumbImage() const;
		/**
		 * @brief 设置滑块图像
		 * @details 用于设置滑块图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetThumbImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取滑块热状态图像
		 * @details 用于获取滑块热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetThumbHotImage() const;
		/**
		 * @brief 设置滑块热状态图像
		 * @details 用于设置滑块热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetThumbHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取滑块Pushed图像
		 * @details 用于获取滑块Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetThumbPushedImage() const;
		/**
		 * @brief 设置滑块Pushed图像
		 * @details 用于设置滑块Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetThumbPushedImage(std::wstring_view pStrImage);

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
		 * @brief 绘制前景图像
		 * @details 用于绘制前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintForeImage(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 设置值
		 * @details 用于设置值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nValue [in] 值数值
		 */
		void SetValue(int nValue);
		/**
		 * @brief 设置检查是否可以Send移动
		 * @details 用于设置检查是否可以Send移动。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bCanSend [in] 是否检查是否可以Send
		 */
		void SetCanSendMove(bool bCanSend);
		/**
		 * @brief 获取检查是否可以Send移动
		 * @details 用于获取检查是否可以Send移动。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetCanSendMove() const;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CSliderUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CSliderUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CSliderUI* pControl) ;

	protected:
		SIZE m_szThumb;
		UINT m_uButtonState;
		int m_nStep;

		std::wstring m_sThumbImage;
		std::wstring m_sThumbHotImage;
		std::wstring m_sThumbPushedImage;

		std::wstring m_sImageModify;
		bool	   m_bSendMove;
	};
}


