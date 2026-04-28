#pragma once 
namespace FYUI
{
	class FYUI_API CAnimationTabLayoutUI : public CTabLayoutUI, public CUIAnimation
	{
		DECLARE_DUICONTROL(CAnimationTabLayoutUI)
	public:
		CAnimationTabLayoutUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

			bool SelectItem( int iIndex ,bool bAnimation =true);
			bool Remove(CControlUI* pControl, bool bChildDelayed = true) override;
			bool RemoveAt(int iIndex, bool bChildDelayed = true) override;
			void RemoveAll(bool bChildDelayed = true) override;
			void AnimationSwitch(bool bAnimation =true);
		void DoEvent(TEventUI& event);
		void OnTimer( int nTimerID );

		virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
		virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
		virtual void OnAnimationStop(INT nAnimationID);

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		virtual CAnimationTabLayoutUI* Clone();
		virtual void CopyData(CAnimationTabLayoutUI* pControl) ;

		bool IsContainerControl() const override
		{
			return true;
		}

	protected:
			void UpdateAnimationDirection(int iIndex);
			void ResetAnimationState();
			void PrepareAnimationTargetControl();
			bool ShouldAnimateSwitch(bool bAnimation) const;
		void CompleteAnimationSwitch();
		RECT BuildAnimationStartRect() const;
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
