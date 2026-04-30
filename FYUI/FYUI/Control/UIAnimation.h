
#pragma once

namespace FYUI {

	class FYUI_API IUIAnimation
	{
	public:
		/**
		 * @brief 析构 IUIAnimation 对象
		 * @details 用于析构 IUIAnimation 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~IUIAnimation() { NULL; }

		virtual BOOL StartAnimation(int nElapse, int nTotalFrame, int nAnimationID = 0, BOOL bLoop = FALSE) = 0;
		/**
		 * @brief 执行 StopAnimation 操作
		 * @details 用于执行 StopAnimation 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void StopAnimation(int nAnimationID = 0) = 0;
		/**
		 * @brief 判断是否动画Running
		 * @details 用于判断是否动画Running。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 * @return BOOL 返回 判断是否动画Running 的结果
		 */
		virtual BOOL IsAnimationRunning(int nAnimationID) = 0;
		/**
		 * @brief 获取当前帧
		 * @details 用于获取当前帧。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 * @return int 返回对应的数值结果
		 */
		virtual int GetCurrentFrame(int nAnimationID = 0) = 0;
		/**
		 * @brief 设置当前帧
		 * @details 用于设置当前帧。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nFrame [in] 帧数值
		 * @param nAnimationID [in] 动画ID数值
		 * @return BOOL 返回 设置当前帧 的结果
		 */
		virtual BOOL SetCurrentFrame(int nFrame, int nAnimationID = 0) = 0;

		/**
		 * @brief 执行 OnAnimationStep 操作
		 * @details 用于执行 OnAnimationStep 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nTotalFrame [in] Total帧数值
		 * @param nCurFrame [in] 当前帧数值
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void OnAnimationStep(int nTotalFrame, int nCurFrame, int nAnimationID) = 0;
		/**
		 * @brief 执行 OnAnimationStart 操作
		 * @details 用于执行 OnAnimationStart 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 * @param bFirstLoop [in] 是否First循环
		 */
		virtual void OnAnimationStart(int nAnimationID, BOOL bFirstLoop) = 0;
		/**
		 * @brief 执行 OnAnimationStop 操作
		 * @details 用于执行 OnAnimationStop 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void OnAnimationStop(int nAnimationID) = 0;

		/**
		 * @brief 执行 OnAnimationElapse 操作
		 * @details 用于执行 OnAnimationElapse 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void OnAnimationElapse(int nAnimationID) = 0;
	};

	class FYUI_API CAnimationData
	{
	public:
		/**
		 * @brief 构造 CAnimationData 对象
		 * @details 用于构造 CAnimationData 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nElipse [in] Elipse数值
		 * @param nFrame [in] 帧数值
		 * @param nID [in] ID数值
		 * @param bLoop [in] 是否循环
		 */
		CAnimationData(int nElipse, int nFrame, int nID, BOOL bLoop)
		{
			m_bFirstLoop = TRUE;
			m_nCurFrame = 0;
			m_nElapse = nElipse;
			m_nTotalFrame = nFrame;
			m_bLoop = bLoop;
			m_nAnimationID = nID;
		}

		//protected:
	public:
		friend class CDUIAnimation;

		int m_nAnimationID;
		int m_nElapse;

		int m_nTotalFrame;
		int m_nCurFrame;

		BOOL m_bLoop;
		BOOL m_bFirstLoop;
	};

	class FYUI_API CUIAnimation: public IUIAnimation
	{
		struct Imp;
	public:
		/**
		 * @brief 构造 CUIAnimation 对象
		 * @details 用于构造 CUIAnimation 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		CUIAnimation(CControlUI* pOwner);
		/**
		 * @brief 构造 CUIAnimation 对象
		 * @details 用于构造 CUIAnimation 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CUIAnimation();
		/**
		 * @brief 析构 CUIAnimation 对象
		 * @details 用于析构 CUIAnimation 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CUIAnimation();

	public:
		/**
		 * @brief 执行 Attach 操作
		 * @details 用于执行 Attach 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		void Attach(CControlUI* pOwner);

		/**
		 * @brief 执行 StartAnimation 操作
		 * @details 用于执行 StartAnimation 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nElapse [in] 定时周期（毫秒）
		 * @param nTotalFrame [in] Total帧数值
		 * @param nAnimationID [in] 动画ID数值
		 * @param bLoop [in] 是否循环
		 * @return BOOL 返回 执行 StartAnimation 操作 的结果
		 */
		virtual BOOL StartAnimation(int nElapse, int nTotalFrame, int nAnimationID = 0, BOOL bLoop = FALSE);
		/**
		 * @brief 执行 StopAnimation 操作
		 * @details 用于执行 StopAnimation 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void StopAnimation(int nAnimationID = 0);
		/**
		 * @brief 判断是否动画Running
		 * @details 用于判断是否动画Running。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 * @return BOOL 返回 判断是否动画Running 的结果
		 */
		virtual BOOL IsAnimationRunning(int nAnimationID);
		/**
		 * @brief 获取当前帧
		 * @details 用于获取当前帧。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 * @return int 返回对应的数值结果
		 */
		virtual int GetCurrentFrame(int nAnimationID = 0);
		/**
		 * @brief 设置当前帧
		 * @details 用于设置当前帧。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nFrame [in] 帧数值
		 * @param nAnimationID [in] 动画ID数值
		 * @return BOOL 返回 设置当前帧 的结果
		 */
		virtual BOOL SetCurrentFrame(int nFrame, int nAnimationID = 0);

		/**
		 * @brief 执行 OnAnimationStart 操作
		 * @details 用于执行 OnAnimationStart 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 * @param bFirstLoop [in] 是否First循环
		 */
		virtual void OnAnimationStart(int nAnimationID, BOOL bFirstLoop) {};
		/**
		 * @brief 执行 OnAnimationStep 操作
		 * @details 用于执行 OnAnimationStep 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nTotalFrame [in] Total帧数值
		 * @param nCurFrame [in] 当前帧数值
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void OnAnimationStep(int nTotalFrame, int nCurFrame, int nAnimationID) {};
		/**
		 * @brief 执行 OnAnimationStop 操作
		 * @details 用于执行 OnAnimationStop 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void OnAnimationStop(int nAnimationID) {};

		/**
		 * @brief 执行 OnAnimationElapse 操作
		 * @details 用于执行 OnAnimationElapse 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 */
		virtual void OnAnimationElapse(int nAnimationID);

	protected:
		/**
		 * @brief 获取动画数据ByID
		 * @details 用于获取动画数据ByID。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nAnimationID [in] 动画ID数值
		 * @return CAnimationData* 返回结果对象指针，失败时返回 nullptr
		 */
		CAnimationData* GetAnimationDataByID(int nAnimationID);

	protected:
		CControlUI* m_pControl;
		Imp * m_pImp;
	};

} 

