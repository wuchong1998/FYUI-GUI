#include "pch.h"
#include "UILabel.h"
#include "..\Core\UIRenderContext.h"

#include <atlconv.h>
namespace FYUI
{
	IMPLEMENT_DUICONTROL(CLabelUI)

		CLabelUI::CLabelUI() : m_uTextStyle(DT_VCENTER | DT_SINGLELINE), m_dwTextColor(0), 
		m_dwDisabledTextColor(0),
		m_iFont(-1),
		m_bShowHtml(false),
		m_bAutoCalcWidth(false),
		m_bAutoCalcHeight(false),
		m_bNeedEstimateSize(false)
	{
		m_cxyFixedLast.cx = m_cxyFixedLast.cy = 0;
		m_szAvailableLast.cx = m_szAvailableLast.cy = 0;
		::ZeroMemory(&m_rcTextPadding, sizeof(m_rcTextPadding));
	}

	CLabelUI::~CLabelUI()
	{
	}

	std::wstring_view CLabelUI::GetClass() const
	{
		return _T("LabelUI");
	}

	LPVOID CLabelUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("Label")) == 0 ) return static_cast<CLabelUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	UINT CLabelUI::GetControlFlags() const
	{
		return IsEnabled() ? UIFLAG_SETCURSOR : 0;
	}
	void CLabelUI::SetTextStyle(UINT uStyle)
	{
		m_uTextStyle = uStyle;
		Invalidate();
	}

	UINT CLabelUI::GetTextStyle() const
	{
		return m_uTextStyle;
	}

	void CLabelUI::SetTextColor(DWORD dwTextColor)
	{
		if (m_dwTextColor == dwTextColor)
			return;
		m_dwTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CLabelUI::GetTextColor() const
	{
		return m_dwTextColor;
	}

	void CLabelUI::SetDisabledTextColor(DWORD dwTextColor)
	{
		m_dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CLabelUI::GetDisabledTextColor() const
	{
		return m_dwDisabledTextColor;
	}

	void CLabelUI::SetFont(int index)
	{
		if (m_iFont == index)
			return;
		m_iFont = index;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	int CLabelUI::GetFont() const
	{
		return m_iFont;
	}

	RECT CLabelUI::GetTextPadding() const
	{
		RECT rcTextPadding = m_rcTextPadding;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcTextPadding);
		return rcTextPadding;
	}

	void CLabelUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	bool CLabelUI::IsShowHtml()
	{
		return m_bShowHtml;
	}

	void CLabelUI::SetShowHtml(bool bShowHtml)
	{
		if( m_bShowHtml == bShowHtml ) return;

		m_bShowHtml = bShowHtml;
		m_bNeedEstimateSize = true;
		Invalidate();
	}

	void CLabelUI::SetFollowSize(bool bFollowSize)
	{
		m_bFollowSize = bFollowSize;
	}

	bool CLabelUI::GetFollowSize() const
	{
		return m_bFollowSize;
	}

	SIZE CLabelUI::EstimateSize(SIZE szAvailable)
	{
		if(GetFollowSize()) 
			return CControlUI::EstimateSize(szAvailable);
		RECT rcTextPadding = GetTextPadding();
		if (m_cxyFixed.cx > 0 && m_cxyFixed.cy > 0) {
			return GetFixedSize();
		}

		if ((szAvailable.cx != m_szAvailableLast.cx || szAvailable.cy != m_szAvailableLast.cy)) {
			m_bNeedEstimateSize = true;
		}

		if (m_bNeedEstimateSize) {
			std::wstring sText = GetText();
			m_bNeedEstimateSize = false;
			m_szAvailableLast = szAvailable;
			m_cxyFixedLast = GetFixedSize();
			// 閼奉亜濮╃拋锛勭暬鐎硅棄瀹?
			if ((m_uTextStyle & DT_SINGLELINE) != 0) {
				// 妤傛ê瀹?
				if (m_cxyFixedLast.cy == 0) {
					m_cxyFixedLast.cy = m_pManager->GetFontInfo(m_iFont)->tm.tmHeight + 8;
					m_cxyFixedLast.cy += rcTextPadding.top + rcTextPadding.bottom;
				}
				// 鐎硅棄瀹?
				if (m_cxyFixedLast.cx == 0) {
					if(m_bAutoCalcWidth) {
						RECT rcText = { 0, 0, 9999999, m_cxyFixedLast.cy };
						CPaintRenderContext measureContext = m_pManager->CreateMeasureRenderContext(rcText);
						if( m_bShowHtml ) {
							int nLinks = 0;
							CRenderEngine::DrawHtmlText(measureContext, rcText, sText, 0, NULL, NULL, nLinks, m_iFont, DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER);
						}
						else {
		CRenderEngine::DrawText(measureContext, rcText, sText, 0, m_iFont, DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER);
						}
						m_cxyFixedLast.cx = rcText.right - rcText.left + GetManager()->ScaleValue(m_rcTextPadding.left + m_rcTextPadding.right);
					}
				}
			}
			// 閼奉亜濮╃拋锛勭暬妤傛ê瀹?
			else if(m_cxyFixedLast.cy == 0) {
				if(m_bAutoCalcHeight) {
					RECT rcText = { 0, 0, m_cxyFixedLast.cx, 9999999 };
					rcText.left += rcTextPadding.left;
					rcText.right -= rcTextPadding.right;
					CPaintRenderContext measureContext = m_pManager->CreateMeasureRenderContext(rcText);
					if( m_bShowHtml ) {
						int nLinks = 0;
						CRenderEngine::DrawHtmlText(measureContext, rcText, sText, 0, NULL, NULL, nLinks, m_iFont, DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER);
					}
					else {
		CRenderEngine::DrawText(measureContext, rcText, sText, 0, m_iFont, DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER);
					}
					m_cxyFixedLast.cy = rcText.bottom - rcText.top + rcTextPadding.top + rcTextPadding.bottom;
				}
			}

		}
		return m_cxyFixedLast;
	}

	void CLabelUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			m_bFocused = true;
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			m_bFocused = false;
			return;
		}
		CControlUI::DoEvent(event);
	}

	void CLabelUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		const std::wstring valueText(pstrValue);
		if( StringUtil::CompareNoCase(pstrName, _T("align")) == 0 ) {
			if( valueText.find(_T("left")) != std::wstring::npos ) {
				m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_uTextStyle |= DT_LEFT;
			}
			if( valueText.find(_T("center")) != std::wstring::npos ) {
				m_uTextStyle &= ~(DT_LEFT | DT_RIGHT );
				m_uTextStyle |= DT_CENTER;
			}
			if( valueText.find(_T("right")) != std::wstring::npos ) {
				m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_uTextStyle |= DT_RIGHT;
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("valign")) == 0 ) {
			if( valueText.find(_T("top")) != std::wstring::npos ) {
				m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER | DT_WORDBREAK);
				m_uTextStyle |= (DT_TOP | DT_SINGLELINE);
			}
			if( valueText.find(_T("vcenter")) != std::wstring::npos ) {
				m_uTextStyle &= ~(DT_TOP | DT_BOTTOM | DT_WORDBREAK);            
				m_uTextStyle |= (DT_VCENTER | DT_SINGLELINE);
			}
			if( valueText.find(_T("bottom")) != std::wstring::npos ) {
				m_uTextStyle &= ~(DT_TOP | DT_VCENTER | DT_WORDBREAK);
				m_uTextStyle |= (DT_BOTTOM | DT_SINGLELINE);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("endellipsis")) == 0 ) {
			if( StringUtil::CompareNoCase(pstrValue, _T("true")) == 0 ) m_uTextStyle |= DT_END_ELLIPSIS;
			else m_uTextStyle &= ~DT_END_ELLIPSIS;
		}   
		else if( StringUtil::CompareNoCase(pstrName, _T("wordbreak")) == 0 ) {
			if( StringUtil::CompareNoCase(pstrValue, _T("true")) == 0 ) {
				m_uTextStyle &= ~DT_SINGLELINE;
				m_uTextStyle |= DT_WORDBREAK | DT_EDITCONTROL;
			}
			else {
				m_uTextStyle &= ~DT_WORDBREAK & ~DT_EDITCONTROL;
				m_uTextStyle |= DT_SINGLELINE;
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("noprefix")) == 0 ) {
			if( StringUtil::CompareNoCase(pstrValue, _T("true")) == 0)
			{
				m_uTextStyle |= DT_NOPREFIX;
			}
			else
			{
				m_uTextStyle = m_uTextStyle & ~DT_NOPREFIX;
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("font")) == 0 ) {
			int fontIndex = 0;
			if (StringUtil::TryParseInt(pstrValue, fontIndex)) {
				SetFont(fontIndex);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("textcolor")) == 0 ) {
			DWORD clrColor = 0;
			if (StringUtil::TryParseColor(pstrValue, clrColor)) {
				SetTextColor(clrColor);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("disabledtextcolor")) == 0 ) {
			DWORD clrColor = 0;
			if (StringUtil::TryParseColor(pstrValue, clrColor)) {
				SetDisabledTextColor(clrColor);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("textpadding")) == 0 ) {
			RECT rcTextPadding = { 0 };
			if (StringUtil::TryParseRect(pstrValue, rcTextPadding)) {
				SetTextPadding(rcTextPadding);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("showhtml")) == 0 ) SetShowHtml(StringUtil::ParseBool(pstrValue));
		else if( StringUtil::CompareNoCase(pstrName, _T("autocalcwidth")) == 0 ) {
			SetAutoCalcWidth(StringUtil::ParseBool(pstrValue));
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("autocalcheight")) == 0 ) {
			SetAutoCalcHeight(StringUtil::ParseBool(pstrValue));
		}

		else if( StringUtil::CompareNoCase(pstrName, _T("followsize")) == 0 ) {
			SetFollowSize(StringUtil::ParseBool(pstrValue));
		}
		else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CLabelUI::PaintText(CPaintRenderContext& renderContext)
	{
		/* std::wstring str = GetName();
		RECT rc1 = GetPos();*/
		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		RECT rc = m_rcItem;
		RECT m_rcTextPadding = CLabelUI::m_rcTextPadding;
		m_rcTextPadding = ScaleRect(m_rcTextPadding);
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		std::wstring sText = GetText();
		if( sText.empty() ) return;
		int nLinks = 0;
		if( IsEnabled() ) {
			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(renderContext, rc, sText, m_dwTextColor, \
					NULL, NULL, nLinks, m_iFont, m_uTextStyle);
			else
		CRenderEngine::DrawText(renderContext, rc, sText, m_dwTextColor, \
			m_iFont, m_uTextStyle);
		}
		else {
			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(renderContext, rc, sText, m_dwDisabledTextColor, \
					NULL, NULL, nLinks, m_iFont, m_uTextStyle);
			else
		CRenderEngine::DrawText(renderContext, rc, sText, m_dwDisabledTextColor, \
			m_iFont, m_uTextStyle);
		}
	}

	bool CLabelUI::GetAutoCalcWidth() const
	{
		return m_bAutoCalcWidth;
	}

	void CLabelUI::SetAutoCalcWidth(bool bAutoCalcWidth)
	{
		m_bAutoCalcWidth = bAutoCalcWidth;
	}

	bool CLabelUI::GetAutoCalcHeight() const
	{
		return m_bAutoCalcHeight;
	}

	void CLabelUI::SetAutoCalcHeight(bool bAutoCalcHeight)
	{
		m_bAutoCalcHeight = bAutoCalcHeight;
	}

	void CLabelUI::SetText(std::wstring_view pstrText)
	{
		if (GetText() == pstrText)
			return;
		m_szAvailableLast .cx = m_szAvailableLast.cy =0;
		CControlUI::SetText(pstrText);
		if(GetAutoCalcWidth() || GetAutoCalcHeight()) {
			NeedParentUpdate();
		}
	}

	CLabelUI* CLabelUI::Clone()
	{
		CLabelUI* pClone = new CLabelUI();
		pClone->CopyData(this);
		return pClone;
	}
	void CLabelUI::CopyData(CLabelUI* pControl)
	{
		m_dwTextColor = pControl->m_dwTextColor;
		m_dwDisabledTextColor = pControl->m_dwDisabledTextColor;
		m_iFont = pControl->m_iFont;
		m_uTextStyle = pControl->m_uTextStyle;
		m_rcTextPadding = pControl->m_rcTextPadding;
		m_bShowHtml = pControl->m_bShowHtml;
		m_bAutoCalcWidth = pControl->m_bAutoCalcWidth; 
		m_bAutoCalcHeight = pControl->m_bAutoCalcHeight;
		m_szAvailableLast = pControl->m_szAvailableLast;
		m_cxyFixedLast = pControl->m_cxyFixedLast;
		m_bNeedEstimateSize = pControl->m_bNeedEstimateSize;
		__super::CopyData(pControl);
	}
}


