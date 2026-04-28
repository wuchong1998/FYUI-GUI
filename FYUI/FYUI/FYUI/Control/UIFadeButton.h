#include "UIAnimation.h"
#pragma once

namespace FYUI {

	class FYUI_API CFadeButtonUI : public CButtonUI, public CUIAnimation
	{
		DECLARE_DUICONTROL(CFadeButtonUI)
	public:
		CFadeButtonUI();
		virtual ~CFadeButtonUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		void SetNormalImage(std::wstring_view pStrImage);

		void DoEvent(TEventUI& event);
		void OnTimer( int nTimerID );
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

		virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
		virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
		virtual void OnAnimationStop(INT nAnimationID) {}

		virtual CFadeButtonUI* Clone();
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


