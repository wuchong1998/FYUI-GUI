#pragma once 
namespace FYUI
{
	class FYUI_API CAnimationTabLayoutUI : public CTabLayoutUI, public CUIAnimation
	{
		DECLARE_DUICONTROL(CAnimationTabLayoutUI)
	public:
		/**
		 * @brief 构造 CAnimationTabLayoutUI 对象
		 * @details 用于构造 CAnimationTabLayoutUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CAnimationTabLayoutUI();

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
			 * @brief 选中指定子项
			 * @details 用于选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 * @param bAnimation [in] 是否动画
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool SelectItem( int iIndex ,bool bAnimation =true);
			/**
			 * @brief 执行 Remove 操作
			 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param pControl [in] 控件对象
			 * @param bChildDelayed [in] 是否延迟处理子项
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool Remove(CControlUI* pControl, bool bChildDelayed = true) override;
			/**
			 * @brief 移除At
			 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 * @param bChildDelayed [in] 是否延迟处理子项
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool RemoveAt(int iIndex, bool bChildDelayed = true) override;
			/**
			 * @brief 移除全部子项
			 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
			 * @param bChildDelayed [in] 是否延迟处理子项
			 */
			void RemoveAll(bool bChildDelayed = true) override;
			/**
			 * @brief 执行 AnimationSwitch 操作
			 * @details 用于执行 AnimationSwitch 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param bAnimation [in] 是否动画
			 */
			void AnimationSwitch(bool bAnimation =true);
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
		virtual void OnAnimationStop(INT nAnimationID);

		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CAnimationTabLayoutUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CAnimationTabLayoutUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CAnimationTabLayoutUI* pControl) ;

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
			/**
			 * @brief 更新动画Direction
			 * @details 用于更新动画Direction。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 */
			void UpdateAnimationDirection(int iIndex);
			/**
			 * @brief 重置动画状态
			 * @details 用于重置动画状态。具体行为由当前对象状态以及传入参数共同决定。
			 */
			void ResetAnimationState();
			/**
			 * @brief 执行 PrepareAnimationTargetControl 操作
			 * @details 用于执行 PrepareAnimationTargetControl 操作。具体行为由当前对象状态以及传入参数共同决定。
			 */
			void PrepareAnimationTargetControl();
			/**
			 * @brief 执行 ShouldAnimateSwitch 操作
			 * @details 用于执行 ShouldAnimateSwitch 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param bAnimation [in] 是否动画
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool ShouldAnimateSwitch(bool bAnimation) const;
		/**
		 * @brief 执行 CompleteAnimationSwitch 操作
		 * @details 用于执行 CompleteAnimationSwitch 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void CompleteAnimationSwitch();
		/**
		 * @brief 执行 BuildAnimationStartRect 操作
		 * @details 用于执行 BuildAnimationStartRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT BuildAnimationStartRect() const;
		/**
		 * @brief 执行 AdvanceAnimationFrame 操作
		 * @details 用于执行 AdvanceAnimationFrame 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nTotalFrame [in] Total帧数值
		 * @param nCurFrame [in] 当前帧数值
		 */
		void AdvanceAnimationFrame(int nTotalFrame, int nCurFrame);
		bool m_bIsVerticalDirection;
		int m_nPositiveDirection;
		RECT m_rcCurPos;
		RECT m_rcItemOld;
		CControlUI* m_pCurrentControl;
		bool m_bControlVisibleFlag;
		enum
		{
			TAB_ANIMATION_ID = 1,

			TAB_ANIMATION_ELLAPSE = 10,
			TAB_ANIMATION_FRAME_COUNT = 15,
		};
	};
}
