#include "UIAnimation.h"
#pragma once

namespace FYUI {

	class FYUI_API CFadeButtonUI : public CButtonUI, public CUIAnimation
	{
		DECLARE_DUICONTROL(CFadeButtonUI)
	public:
		/**
		 * @brief 构造 CFadeButtonUI 对象
		 * @details 用于构造 CFadeButtonUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CFadeButtonUI();
		/**
		 * @brief 析构 CFadeButtonUI 对象
		 * @details 用于析构 CFadeButtonUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CFadeButtonUI();

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
		 * @brief 设置Normal图像
		 * @details 用于设置Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetNormalImage(std::wstring_view pStrImage);

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 执行 OnTimer 操作
		 * @details 用于执行 OnTimer 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nTimerID [in] 定时器标识
		 */
		void OnTimer( int nTimerID );
		/**
		 * @brief 绘制状态图像
		 * @details 用于绘制状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 执行 OnAnimationStart 操作
		 * @details 用于执行 OnAnimationStart 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 * @param bFirstLoop [in] 是否First循环
		 */
		virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
		virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
		/**
		 * @brief 执行 OnAnimationStop 操作
		 * @details 用于执行 OnAnimationStop 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void OnAnimationStop(INT nAnimationID) {}

		virtual CFadeButtonUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CFadeButtonUI* pControl) ;



	protected:
		std::wstring m_sOldImage;
		std::wstring m_sNewImage;
		std::wstring m_sLastImage;
		BYTE       m_bFadeAlpha;
		BOOL       m_bMouseHove;
		BOOL       m_bMouseLeave;
		enum{
			FADE_IN_ID			= 8,
			FADE_OUT_ID			= 9,
			FADE_ELLAPSE		= 10,
			FADE_FRAME_COUNT	= 30,
		};
	};

} 


