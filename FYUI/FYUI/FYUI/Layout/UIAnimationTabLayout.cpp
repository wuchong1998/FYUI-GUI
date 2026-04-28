#include "pch.h"
#include "UIAnimationTabLayout.h"

namespace FYUI 
{
	IMPLEMENT_DUICONTROL(CAnimationTabLayoutUI)

		CAnimationTabLayoutUI::CAnimationTabLayoutUI() : 
		m_bIsVerticalDirection( false ), 
		m_nPositiveDirection( 1 ),
		m_pCurrentControl( NULL ),
		m_bControlVisibleFlag( false )
	{
		Attach(this);
	}

	std::wstring_view CAnimationTabLayoutUI::GetClass() const
	{
		return _T("AnimationTabLayoutUI");
	}

	LPVOID CAnimationTabLayoutUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("AnimationTabLayout")) == 0 ) 
			return static_cast<CAnimationTabLayoutUI*>(this);
		return CTabLayoutUI::GetInterface(pstrName);
	}

		void CAnimationTabLayoutUI::UpdateAnimationDirection(int iIndex)
		{
			if( iIndex > m_iCurSel ) m_nPositiveDirection = -1;
			if( iIndex < m_iCurSel ) m_nPositiveDirection = 1;
		}

		void CAnimationTabLayoutUI::ResetAnimationState()
		{
			if( IsAnimationRunning(TAB_ANIMATION_ID) ) {
				StopAnimation(TAB_ANIMATION_ID);
			}
			m_pCurrentControl = NULL;
			m_bControlVisibleFlag = false;
			m_rcItemOld = m_rcItem;
			m_rcCurPos = m_rcItem;
		}

	void CAnimationTabLayoutUI::PrepareAnimationTargetControl()
	{
		m_bControlVisibleFlag = false;
		m_pCurrentControl = static_cast<CControlUI*>(GetSelectedItemControl());
		if( m_pCurrentControl != NULL ) {
			m_pCurrentControl->SetVisible(false);
		}
	}

	bool CAnimationTabLayoutUI::ShouldAnimateSwitch(bool bAnimation) const
	{
		return bAnimation && m_pCurrentControl != NULL;
	}

	void CAnimationTabLayoutUI::CompleteAnimationSwitch()
	{
		if( m_pCurrentControl != NULL ) {
			m_bControlVisibleFlag = true;
			m_pCurrentControl->SetVisible(true);
		}
		m_rcItem = m_rcItemOld;
		m_rcCurPos = m_rcItemOld;
		SetPos(m_rcItemOld);
		NeedParentUpdate();
	}

	RECT CAnimationTabLayoutUI::BuildAnimationStartRect() const
	{
		RECT rcStart = m_rcItem;
		if( !m_bIsVerticalDirection )
		{
			const int cxWidth = m_rcItem.right - m_rcItem.left;
			rcStart.left -= cxWidth * m_nPositiveDirection - 52 * m_nPositiveDirection;
			rcStart.right -= cxWidth * m_nPositiveDirection - 52 * m_nPositiveDirection;
		}
		else
		{
			const int cyHeight = m_rcItem.bottom - m_rcItem.top;
			rcStart.top -= cyHeight * m_nPositiveDirection;
			rcStart.bottom -= cyHeight * m_nPositiveDirection;
		}
		return rcStart;
	}

	void CAnimationTabLayoutUI::AdvanceAnimationFrame(int nTotalFrame, int nCurFrame)
	{
		if( !m_bControlVisibleFlag && m_pCurrentControl != NULL ) {
			m_bControlVisibleFlag = true;
			m_pCurrentControl->SetVisible(true);
		}

		if( !m_bIsVerticalDirection )
		{
			const int iStepLen = (m_rcItemOld.right - m_rcItemOld.left) * m_nPositiveDirection / nTotalFrame;
			if( nCurFrame != nTotalFrame )
			{
				m_rcCurPos.left += iStepLen;
				m_rcCurPos.right += iStepLen;
			}
			else
			{
				m_rcItem = m_rcCurPos = m_rcItemOld;
			}
		}
		else
		{
			const int iStepLen = (m_rcItemOld.bottom - m_rcItemOld.top) * m_nPositiveDirection / nTotalFrame;
			if( nCurFrame != nTotalFrame )
			{
				m_rcCurPos.top += iStepLen;
				m_rcCurPos.bottom += iStepLen;
			}
			else
			{
				m_rcItem = m_rcCurPos = m_rcItemOld;
			}
		}
	}

		bool CAnimationTabLayoutUI::SelectItem( int iIndex ,bool bAnimation)
		{
			if( iIndex < 0 || iIndex >= m_items.GetSize() ) return false;
			if( iIndex == m_iCurSel ) {
				if( IsAnimationRunning(TAB_ANIMATION_ID) ) {
					StopAnimation(TAB_ANIMATION_ID);
				}
				else {
					UpdateSelectionVisibility(m_iCurSel, true);
					SetPos(m_rcItem);
					NeedParentUpdate();
				}
				return true;
			}
			ResetAnimationState();
			UpdateAnimationDirection(iIndex);

		int iOldSel = m_iCurSel;
		m_iCurSel = iIndex;
		UpdateSelectionVisibility(m_iCurSel, true);
		PrepareAnimationTargetControl();

		NeedParentUpdate();
		AnimationSwitch(bAnimation);

		if( m_pManager != NULL ) {
			m_pManager->SetNextTabControl();
			m_pManager->SendNotify(this, _T("tabselect"), m_iCurSel, iOldSel);
		}
			return true;
		}

		bool CAnimationTabLayoutUI::Remove(CControlUI* pControl, bool bChildDelayed)
		{
			ResetAnimationState();
			return CTabLayoutUI::Remove(pControl, bChildDelayed);
		}

		bool CAnimationTabLayoutUI::RemoveAt(int iIndex, bool bChildDelayed)
		{
			ResetAnimationState();
			return CTabLayoutUI::RemoveAt(iIndex, bChildDelayed);
		}

		void CAnimationTabLayoutUI::RemoveAll(bool bChildDelayed)
		{
			ResetAnimationState();
			CTabLayoutUI::RemoveAll(bChildDelayed);
		}

	void CAnimationTabLayoutUI::AnimationSwitch(bool bAnimation)
	{
		m_rcItemOld = m_rcItem;
		m_rcCurPos = BuildAnimationStartRect();

		if( !ShouldAnimateSwitch(bAnimation) ) {
			CompleteAnimationSwitch();
			return;
		}

		StopAnimation( TAB_ANIMATION_ID );
		StartAnimation( TAB_ANIMATION_ELLAPSE, TAB_ANIMATION_FRAME_COUNT, TAB_ANIMATION_ID );
	}

	void CAnimationTabLayoutUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_TIMER ) 
		{
			OnTimer(static_cast<int>(event.wParam));
		}
		__super::DoEvent( event );
	}

	void CAnimationTabLayoutUI::OnTimer( int nTimerID )
	{
		OnAnimationElapse( nTimerID );
	}

	void CAnimationTabLayoutUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
	{
		(void)nAnimationID;
		AdvanceAnimationFrame(nTotalFrame, nCurFrame);
		SetPos(m_rcCurPos);
	}

	void CAnimationTabLayoutUI::OnAnimationStop(INT nAnimationID) 
	{
		(void)nAnimationID;
		CompleteAnimationSwitch();
	}

	void CAnimationTabLayoutUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("animation_direction")) == 0 && StringUtil::CompareNoCase(pstrValue, _T("vertical")) == 0 ) m_bIsVerticalDirection = true; // pstrValue = "vertical" or "horizontal"
		return CTabLayoutUI::SetAttribute(pstrName, pstrValue);
	}
	CAnimationTabLayoutUI* CAnimationTabLayoutUI::Clone()
	{
		CAnimationTabLayoutUI* pClone = new CAnimationTabLayoutUI();
		pClone->CopyData(this);
		return pClone;
	}
	void CAnimationTabLayoutUI::CopyData(CAnimationTabLayoutUI* pControl)
	{
		m_bIsVerticalDirection = pControl->m_bIsVerticalDirection;
		m_nPositiveDirection = pControl->m_nPositiveDirection;
		m_rcCurPos = pControl->m_rcCurPos;
		m_rcItemOld = pControl->m_rcItemOld;
		m_pCurrentControl = pControl->m_pCurrentControl;
		m_bControlVisibleFlag = pControl->m_bControlVisibleFlag;
		__super::CopyData(pControl);
	}

} // namespace DuiLib
