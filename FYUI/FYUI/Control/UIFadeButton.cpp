#include "pch.h"
#include "UIFadeButton.h"
#include "../Core/Render/UIRenderContext.h"

namespace FYUI {
	IMPLEMENT_DUICONTROL(CFadeButtonUI)

		CFadeButtonUI::CFadeButtonUI(): m_bMouseHove( FALSE ), m_bMouseLeave( FALSE )
	{
		Attach(this);
	}

	CFadeButtonUI::~CFadeButtonUI()
	{
		StopAnimation();
	}

	std::wstring_view CFadeButtonUI::GetClass() const
	{
		return _T("FadeButtonUI");
	}

	LPVOID CFadeButtonUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("FadeButton")) == 0 ) 
			return static_cast<CFadeButtonUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void CFadeButtonUI::SetNormalImage(std::wstring_view pStrImage)
	{
		m_sNormalImage = pStrImage;
		m_sLastImage = m_sNormalImage;
	}

	void CFadeButtonUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_TIMER ) {
			OnTimer(static_cast<int>(event.wParam));
		}
		else if(IsEnabled()) {
			if( event.Type == UIEVENT_MOUSEENTER && !IsAnimationRunning( FADE_IN_ID ) )
			{
				m_bFadeAlpha = 0;
				m_bMouseHove = TRUE;
				StopAnimation(FADE_OUT_ID);
				StartAnimation(FADE_ELLAPSE, FADE_FRAME_COUNT, FADE_IN_ID);
				Invalidate();
				return;
			}
			else if( event.Type == UIEVENT_MOUSELEAVE && !IsAnimationRunning( FADE_OUT_ID ) )
			{
				m_bFadeAlpha = 0;
				m_bMouseLeave = TRUE;
				StopAnimation(FADE_IN_ID);
				StartAnimation(FADE_ELLAPSE, FADE_FRAME_COUNT, FADE_OUT_ID);
				Invalidate();
				return;
			}
		}
		CButtonUI::DoEvent( event );
	}

	void CFadeButtonUI::OnTimer( int nTimerID )
	{
		OnAnimationElapse( nTimerID );
	}

	void CFadeButtonUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if( !m_sDisabledImage.empty() ) {
				if( !DrawImage(renderContext, m_sDisabledImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sPushedImage.empty() ) {
				if( !DrawImage(renderContext, m_sPushedImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_FOCUSED) != 0 ) {
			if( !m_sFocusedImage.empty() ) {
				if( !DrawImage(renderContext, m_sFocusedImage) ) {}
				else return;
			}
		}
		if( !m_sNormalImage.empty() ) {
			if( m_bMouseHove ) {
				m_bMouseHove = FALSE;
				m_sLastImage = m_sHotImage;
			}

			if( m_bMouseLeave ) {
				m_bMouseLeave = FALSE;
				m_sLastImage = m_sNormalImage;
			}

			if( IsAnimationRunning(FADE_IN_ID) || IsAnimationRunning(FADE_OUT_ID)) {
				m_sOldImage = m_sNormalImage;
				m_sNewImage = m_sHotImage;
				if( IsAnimationRunning(FADE_OUT_ID) ) {
					m_sOldImage = m_sHotImage;
					m_sNewImage = m_sNormalImage;
				}
				std::wstring sFadeOut, sFadeIn;
				sFadeOut = StringUtil::Format(L"fade='{}'", 255 - m_bFadeAlpha);
				sFadeIn = StringUtil::Format(L"fade='{}'", m_bFadeAlpha);
				if( !DrawImage(renderContext, m_sOldImage, sFadeOut) ) {}
				if( !DrawImage(renderContext, m_sNewImage, sFadeIn) ) {}
				return;
			}
			else {
				if(m_sLastImage.empty()) m_sLastImage = m_sNormalImage;
				if( !DrawImage(renderContext, m_sLastImage) ) {}
				return;
			}
		}
	}

	void CFadeButtonUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID)
	{
		m_bFadeAlpha = (BYTE)((nCurFrame / (double)nTotalFrame) * 255);
		m_bFadeAlpha = m_bFadeAlpha == 0 ? 10 : m_bFadeAlpha;
		Invalidate();
	}

	CFadeButtonUI* CFadeButtonUI::Clone()
	{
		CFadeButtonUI* pClone = new CFadeButtonUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CFadeButtonUI::CopyData(CFadeButtonUI* pControl)
	{
		m_sOldImage = pControl->m_sOldImage;
		m_sNewImage = pControl->m_sNewImage;
		m_sLastImage = pControl->m_sLastImage;
		m_bFadeAlpha = pControl->m_bFadeAlpha ;
		m_bMouseHove = pControl->m_bMouseHove;
		m_bMouseLeave = pControl->m_bMouseLeave;
	}

} // namespace DuiLib


