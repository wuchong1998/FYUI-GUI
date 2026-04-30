#pragma once

namespace FYUI
{
	class FYUI_API CRingUI : public CLabelUI
	{
		enum
		{ 
			RING_TIMERID = 100,
		};
		DECLARE_DUICONTROL(CRingUI)
	public:
		/**
		 * @brief 执行 CRingUI 操作
		 * @details 用于执行 CRingUI 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CRingUI();
		/**
		 * @brief 执行 ~CRingUI 操作
		 * @details 用于执行 ~CRingUI 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CRingUI();

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
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 设置背景图像
		 * @details 用于设置背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetBkImage(std::wstring_view pStrImage);	
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void DoEvent(TEventUI& event);
		/**
		 * @brief 绘制背景图像
		 * @details 用于绘制背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintBkImage(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CRingUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CRingUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CRingUI* pControl) ;

	private:
		/**
		 * @brief 执行 InitImage 操作
		 * @details 用于执行 InitImage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void InitImage();
		/**
		 * @brief 删除图像
		 * @details 用于删除图像。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void DeleteImage();

	public:
		float m_fCurAngle;
		TImageInfo* m_pBkimageInfo;
	};
}
