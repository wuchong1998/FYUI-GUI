#include "pch.h"
#include "UISlider.h"
#include "../Core/Render/UIRenderContext.h"

namespace FYUI
{
	namespace
	{
		int CalculateSliderValueFromPoint(const RECT& rcItem, const SIZE& szThumb, bool horizontal, const POINT& ptMouse, int nMin, int nMax)
		{
			const int range = nMax - nMin;
			if (range <= 0) {
				return nMin;
			}

			if (horizontal) {
				const int track = rcItem.right - rcItem.left - szThumb.cx;
				if (track <= 0) {
					return nMin;
				}
				if (ptMouse.x >= rcItem.right - szThumb.cx / 2) return nMax;
				if (ptMouse.x <= rcItem.left + szThumb.cx / 2) return nMin;

				const int offset = ptMouse.x - rcItem.left - szThumb.cx / 2;
				return nMin + static_cast<int>(static_cast<__int64>(range) * offset / track);
			}

			const int track = rcItem.bottom - rcItem.top - szThumb.cy;
			if (track <= 0) {
				return nMin;
			}
			if (ptMouse.y >= rcItem.bottom - szThumb.cy / 2) return nMin;
			if (ptMouse.y <= rcItem.top + szThumb.cy / 2) return nMax;

			const int offset = rcItem.bottom - ptMouse.y - szThumb.cy / 2;
			return nMin + static_cast<int>(static_cast<__int64>(range) * offset / track);
		}
	}

	IMPLEMENT_DUICONTROL(CSliderUI)
		CSliderUI::CSliderUI() : m_uButtonState(0), m_nStep(1),m_bSendMove(false)
	{
		m_uTextStyle = DT_SINGLELINE | DT_CENTER;
		m_szThumb.cx = m_szThumb.cy = 10;
	}

	std::wstring_view CSliderUI::GetClass() const
	{
		return _T("SliderUI");
	}

	UINT CSliderUI::GetControlFlags() const
	{
		if( IsEnabled() ) return UIFLAG_SETCURSOR;
		else return 0;
	}

	LPVOID CSliderUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, DUI_CTR_SLIDER) == 0 ) return static_cast<CSliderUI*>(this);
		return CProgressUI::GetInterface(pstrName);
	}

	void CSliderUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if( !IsEnabled() ) {
			m_uButtonState = 0;
		}
	}

	int CSliderUI::GetChangeStep()
	{
		return m_nStep;
	}

	void CSliderUI::SetChangeStep(int step)
	{
		m_nStep = step;
	}

	void CSliderUI::SetThumbSize(SIZE szXY)
	{
		m_szThumb = szXY;
	}

	RECT CSliderUI::GetThumbRect() const
	{
		RECT rcThumb = {0};
		SIZE m_szThumb = CSliderUI::m_szThumb;
		if (GetManager() != NULL) {
			m_szThumb = GetManager()->ScaleSize(m_szThumb);
		}
		if( m_bHorizontal ) {
			int left = m_rcItem.left + (m_rcItem.right - m_rcItem.left - m_szThumb.cx) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
			int top = (m_rcItem.bottom + m_rcItem.top - m_szThumb.cy) / 2;
			rcThumb = CDuiRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
		}
		else {
			int left = (m_rcItem.right + m_rcItem.left - m_szThumb.cx) / 2;
			int top = m_rcItem.bottom - m_szThumb.cy - (m_rcItem.bottom - m_rcItem.top - m_szThumb.cy) * (m_nValue - m_nMin) / (m_nMax - m_nMin);
			rcThumb = CDuiRect(left, top, left + m_szThumb.cx, top + m_szThumb.cy); 
		}
		if(m_pManager != NULL) {
			//rcThumb = ScaleRect(rcThumb);
		}
		return rcThumb;
	}

	std::wstring_view CSliderUI::GetThumbImage() const
	{
		return m_sThumbImage;
	}

	void CSliderUI::SetThumbImage(std::wstring_view pStrImage)
	{
		m_sThumbImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CSliderUI::GetThumbHotImage() const
	{
		return m_sThumbHotImage;
	}

	void CSliderUI::SetThumbHotImage(std::wstring_view pStrImage)
	{
		m_sThumbHotImage = pStrImage;
		Invalidate();
	}

	std::wstring_view CSliderUI::GetThumbPushedImage() const
	{
		return m_sThumbPushedImage;
	}

	void CSliderUI::SetThumbPushedImage(std::wstring_view pStrImage)
	{
		m_sThumbPushedImage = pStrImage;
		Invalidate();
	}

	void CSliderUI::SetValue(int nValue)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) return;
		CProgressUI::SetValue(nValue);
	}

	void CSliderUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CProgressUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK ) {
			if( IsEnabled() ) {
				m_uButtonState |= UISTATE_CAPTURED;

				SIZE szThumb = m_szThumb;
				if (m_pManager != NULL) {
					szThumb = m_pManager->ScaleSize(szThumb);
				}
				const int nValue = CalculateSliderValueFromPoint(m_rcItem, szThumb, m_bHorizontal, event.ptMouse, m_nMin, m_nMax);
				if(m_nValue != nValue && nValue >= m_nMin && nValue <= m_nMax) {
					m_nValue = nValue;
					Invalidate();
				}
				UpdateText();
			}
			return;
		}

		if( event.Type == UIEVENT_BUTTONUP || event.Type == UIEVENT_RBUTTONUP) {
			if( IsEnabled() ) {
				int nValue = 0;
				const int previousValue = m_nValue;
				SIZE szThumb = m_szThumb;
				if (m_pManager != NULL) {
					szThumb = m_pManager->ScaleSize(szThumb);
				}
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
				}
				if( m_bHorizontal ) {
					if( event.ptMouse.x >= m_rcItem.right - szThumb.cx / 2 ) nValue = m_nMax;
					else if( event.ptMouse.x <= m_rcItem.left + szThumb.cx / 2 ) nValue = m_nMin;
					else nValue = m_nMin + (m_nMax - m_nMin) * (event.ptMouse.x - m_rcItem.left - szThumb.cx / 2 ) / (m_rcItem.right - m_rcItem.left - szThumb.cx);
				}
				else {
					if( event.ptMouse.y >= m_rcItem.bottom - szThumb.cy / 2 ) nValue = m_nMin;
					else if( event.ptMouse.y <= m_rcItem.top + szThumb.cy / 2  ) nValue = m_nMax;
					else nValue = m_nMin + (m_nMax - m_nMin) * (m_rcItem.bottom - event.ptMouse.y - szThumb.cy / 2 ) / (m_rcItem.bottom - m_rcItem.top - szThumb.cy);
				}
				if(nValue >= m_nMin && nValue <= m_nMax) {
					m_nValue =nValue;
					if (m_nValue != previousValue) {
						m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
						Invalidate();
					}
				}
				UpdateText();
				return;
			}
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			return;
		}
		if( event.Type == UIEVENT_SCROLLWHEEL ) 
		{
			if( IsEnabled() ) {
				switch( LOWORD(event.wParam) ) {
				case SB_LINEUP:
					SetValue(GetValue() + GetChangeStep());
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
					return;
				case SB_LINEDOWN:
					SetValue(GetValue() - GetChangeStep());
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED);
					return;
				}
			}
		}
		if( event.Type == UIEVENT_MOUSEMOVE ) {
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				SIZE szThumb = m_szThumb;
				if (m_pManager != NULL) {
					szThumb = m_pManager->ScaleSize(szThumb);
				}
				const int previousValue = m_nValue;
				m_nValue = CalculateSliderValueFromPoint(m_rcItem, szThumb, m_bHorizontal, event.ptMouse, m_nMin, m_nMax);
				if (m_nValue != previousValue && m_bSendMove) {
					UpdateText();
					m_pManager->SendNotify(this, DUI_MSGTYPE_VALUECHANGED_MOVE);
				}
				if (m_nValue != previousValue) {
					Invalidate();
				}
			}

			POINT pt = event.ptMouse;
			RECT rcThumb = GetThumbRect();
			const UINT previousState = m_uButtonState;
			if( IsEnabled() && ::PtInRect(&rcThumb, event.ptMouse) ) {
				m_uButtonState |= UISTATE_HOT;
			}
			else {
				m_uButtonState &= ~UISTATE_HOT;
			}
			if (previousState != m_uButtonState) {
				Invalidate();
			}
			return;
		}
		if( event.Type == UIEVENT_SETCURSOR )
		{
			RECT rcThumb = GetThumbRect();
			if( IsEnabled()) {
				::SetCursor(::LoadCursor(NULL, IDC_HAND));
				return;
			}
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) {
				if ((m_uButtonState & UISTATE_HOT) != 0) {
					m_uButtonState &= ~UISTATE_HOT;
					Invalidate();
				}
			}
			return;
		}
		CControlUI::DoEvent(event);
	}

	void CSliderUI::SetCanSendMove(bool bCanSend)
	{
		m_bSendMove = bCanSend;
	}
	bool CSliderUI::GetCanSendMove() const
	{
		return m_bSendMove;
	}

	CSliderUI* CSliderUI::Clone()
	{
		CSliderUI* pClone = new CSliderUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CSliderUI::CopyData(CSliderUI* pControl)
	{
		m_szThumb = pControl->m_szThumb;
		m_uButtonState = pControl->m_uButtonState;
		m_nStep = pControl->m_nStep;

		m_sThumbImage = pControl->m_sThumbImage;
		m_sThumbHotImage = pControl->m_sThumbHotImage;
		m_sThumbPushedImage = pControl->m_sThumbPushedImage;

		m_sImageModify = pControl->m_sImageModify;
		m_bSendMove = pControl->m_bSendMove;
		__super::CopyData(pControl);
	}

	void CSliderUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("thumbimage")) == 0 ) SetThumbImage(pstrValue);
		else if( StringUtil::CompareNoCase(pstrName, _T("thumbhotimage")) == 0 ) SetThumbHotImage(pstrValue);
		else if( StringUtil::CompareNoCase(pstrName, _T("thumbpushedimage")) == 0 ) SetThumbPushedImage(pstrValue);
		else if( StringUtil::CompareNoCase(pstrName, _T("thumbsize")) == 0 ) {
			SIZE szXY = {0};
			if (StringUtil::TryParseSize(pstrValue, szXY)) {
				SetThumbSize(szXY);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("step")) == 0 ) {
			int step = 0;
			if (StringUtil::TryParseInt(pstrValue, step)) {
				SetChangeStep(step);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("sendmove")) == 0 ) {
			SetCanSendMove(StringUtil::ParseBool(pstrValue));
		}
		else CProgressUI::SetAttribute(pstrName, pstrValue);
	}

	void CSliderUI::PaintForeImage(CPaintRenderContext& renderContext)
	{
		CProgressUI::PaintForeImage(renderContext);

		RECT rcThumb = GetThumbRect();
		rcThumb.left -= m_rcItem.left;
		rcThumb.top -= m_rcItem.top;
		rcThumb.right -= m_rcItem.left;
		rcThumb.bottom -= m_rcItem.top;
		rcThumb = PixelsToLogical(rcThumb);

		if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
			if( !m_sThumbPushedImage.empty() ) {
				m_sImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
				if( !DrawImage(renderContext, m_sThumbPushedImage, m_sImageModify) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sThumbHotImage.empty() ) {
				m_sImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
				if( !DrawImage(renderContext, m_sThumbHotImage, m_sImageModify) ) {}
				else return;
			}
		}

		if( !m_sThumbImage.empty() ) {
			m_sImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcThumb.left, rcThumb.top, rcThumb.right, rcThumb.bottom);
			if( !DrawImage(renderContext, m_sThumbImage, m_sImageModify) ) {}
			else return;
		}
	}

}

