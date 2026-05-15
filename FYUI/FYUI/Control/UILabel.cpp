#include "pch.h"
#include "UILabel.h"
#include "..\Core\Render\UIRenderContext.h"

#include <atlconv.h>
namespace FYUI
{
	IMPLEMENT_DUICONTROL(CLabelUI)

		CLabelUI::CLabelUI() : m_uTextStyle(DT_VCENTER | DT_SINGLELINE), m_dwTextColor(0), 
		m_dwDisabledTextColor(0),
		m_iFont(-1),
		m_bShowHtml(false),
		m_bAutoTextWidth(false),
		m_bAutoTextHeight(false),
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
		if (m_uTextStyle == uStyle) return;
		m_uTextStyle = uStyle;
		m_bNeedEstimateSize = true;
		if (GetAutoTextWidth() || GetAutoTextHeight()) NeedParentUpdate();
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
		if (GetAutoTextWidth() || GetAutoTextHeight()) NeedParentUpdate();
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
		if (m_rcTextPadding.left == rc.left && m_rcTextPadding.top == rc.top
			&& m_rcTextPadding.right == rc.right && m_rcTextPadding.bottom == rc.bottom) {
			return;
		}
		m_rcTextPadding = rc;
		m_bNeedEstimateSize = true;
		if (GetAutoTextWidth() || GetAutoTextHeight()) NeedParentUpdate();
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
		if (GetAutoTextWidth() || GetAutoTextHeight()) NeedParentUpdate();
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
		const SIZE szFixed = GetFixedSize();
		if (szFixed.cx > 0 && szFixed.cy > 0) {
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
			if ((m_uTextStyle & DT_SINGLELINE) != 0) {
		
				if (m_cxyFixedLast.cy == 0) {
					m_cxyFixedLast.cy = m_pManager->GetFontInfo(m_iFont)->tm.tmHeight + GetManager()->ScaleValue(8);
					m_cxyFixedLast.cy += rcTextPadding.top + rcTextPadding.bottom;
				}
				if (m_cxyFixedLast.cx == 0) {
					if(m_bAutoTextWidth) {
						RECT rcText = { 0, 0, 9999999, m_cxyFixedLast.cy };
						CPaintRenderContext measureContext = m_pManager->CreateMeasureRenderContext(rcText);
						if( m_bShowHtml ) {
							int nLinks = 0;
							CRenderEngine::DrawHtmlText(measureContext, rcText, sText, 0, NULL, NULL, nLinks, m_iFont, DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER);
						}
						else {
		CRenderEngine::DrawText(measureContext, rcText, sText, 0, m_iFont, DT_CALCRECT | m_uTextStyle & ~DT_RIGHT & ~DT_CENTER);
						}
						m_cxyFixedLast.cx = rcText.right - rcText.left + rcTextPadding.left + rcTextPadding.right;
					}
				}
			}
			else if(m_cxyFixedLast.cy == 0) {
				if(m_bAutoTextHeight) {
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
		const UINT uOldTextStyle = m_uTextStyle;
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
		else if( StringUtil::CompareNoCase(pstrName, _T("autotextwidth")) == 0 ) {
			SetAutoTextWidth(StringUtil::ParseBool(pstrValue));
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("autotextheight")) == 0 ) {
			const bool bEnable = StringUtil::ParseBool(pstrValue);
			SetAutoTextHeight(bEnable);
			if (bEnable) {
				m_uTextStyle &= ~DT_SINGLELINE;
				m_uTextStyle |= DT_WORDBREAK | DT_EDITCONTROL;
			}
		}

		else if( StringUtil::CompareNoCase(pstrName, _T("followsize")) == 0 ) {
			SetFollowSize(StringUtil::ParseBool(pstrValue));
		}
		else CControlUI::SetAttribute(pstrName, pstrValue);

		if (m_uTextStyle != uOldTextStyle) {
			m_bNeedEstimateSize = true;
			if (GetAutoTextWidth() || GetAutoTextHeight()) NeedParentUpdate();
			Invalidate();
		}
	}

	void CLabelUI::PaintText(CPaintRenderContext& renderContext)
	{

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		RECT rc = m_rcItem;
		RECT rcTextPadding = GetTextPadding();
		rc.left += rcTextPadding.left;
		rc.right -= rcTextPadding.right;
		rc.top += rcTextPadding.top;
		rc.bottom -= rcTextPadding.bottom;

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

	bool CLabelUI::GetAutoTextWidth() const
	{
		return m_bAutoTextWidth;
	}

	void CLabelUI::SetAutoTextWidth(bool bAutoTextWidth)
	{
		if (m_bAutoTextWidth == bAutoTextWidth)
			return;
		m_bAutoTextWidth = bAutoTextWidth;
		m_bNeedEstimateSize = true;
		NeedParentUpdate();
	}

	bool CLabelUI::GetAutoTextHeight() const
	{
		return m_bAutoTextHeight;
	}

	void CLabelUI::SetAutoTextHeight(bool bAutoTextHeight)
	{
		if (m_bAutoTextHeight == bAutoTextHeight)
			return;
		m_bAutoTextHeight = bAutoTextHeight;
		m_bNeedEstimateSize = true;
		NeedParentUpdate();
	}

	void CLabelUI::SetText(std::wstring_view pstrText)
	{
		if (GetText() == pstrText)
			return;
		m_szAvailableLast .cx = m_szAvailableLast.cy =0;
		CControlUI::SetText(pstrText);
		if(GetAutoTextWidth() || GetAutoTextHeight()) {
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
		m_bAutoTextWidth = pControl->m_bAutoTextWidth;
		m_bAutoTextHeight = pControl->m_bAutoTextHeight;
		m_szAvailableLast = pControl->m_szAvailableLast;
		m_cxyFixedLast = pControl->m_cxyFixedLast;
		m_bNeedEstimateSize = pControl->m_bNeedEstimateSize;
		__super::CopyData(pControl);
	}
}


