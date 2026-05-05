#include "pch.h"
#include "UIButton.h"
#include "..\Core\Render\UIRenderContext.h"

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CButtonUI)

	namespace
	{
		void DrawButtonStateColor(CPaintRenderContext& renderContext, const RECT& rcPaint, const RECT& rcItem, SIZE round, DWORD color)
		{
			if (round.cx > 0 || round.cy > 0) {
				CRenderEngine::DrawRoundColor(renderContext, rcItem, round.cx, round.cy, color);
				return;
			}
			CRenderEngine::DrawColor(renderContext, rcPaint, color);
		}
	}

	CButtonUI::CButtonUI()
		: m_uButtonState(0)
		, m_iHotFont(-1)
		, m_iPushedFont(-1)
		, m_iFocusedFont(-1)
		, m_dwHotTextColor(0)
		, m_dwPushedTextColor(0)
		, m_dwFocusedTextColor(0)
		, m_dwHotBkColor(0)
		, m_dwPushedBkColor(0)
		, m_dwDisabledBkColor(0)
		, m_dwHotBorderColor(0)
		, m_dwPushedBorderColor(0)
		, m_dwDisabledBorderColor(0)
		, m_iBindTabIndex(-1)
		, m_nStateCount(0)
	{
		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
	}

	std::wstring_view CButtonUI::GetClass() const
	{
		return _T("ButtonUI");
	}

	LPVOID CButtonUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, DUI_CTR_BUTTON) == 0 ) return static_cast<CButtonUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	UINT CButtonUI::GetControlFlags() const
	{
		return (IsKeyboardEnabled() ? UIFLAG_TABSTOP : 0) | (IsEnabled() ? UIFLAG_SETCURSOR : 0);
	}

	void CButtonUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else CLabelUI::DoEvent(event);
			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			Invalidate();
		}
		if( event.Type == UIEVENT_KEYDOWN )
		{
			if (IsKeyboardEnabled()) {
				if( event.chKey == VK_SPACE || event.chKey == VK_RETURN ) {
					Activate();
					return;
				}
			}
		}		
		if( event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() )
			{
				m_uButtonState |= UISTATE_PUSHED | UISTATE_CAPTURED;
				Invalidate();
				if(IsRichEvent())
					m_pManager->SendNotify(this, DUI_MSGTYPE_BUTTONDOWN);

				if ( event.Type == UIEVENT_DBLCLICK )
					m_pManager->SendNotify(this, DUI_MSGTYPE_DBCLICK);
			}
			return;
		}	
		if( event.Type == UIEVENT_MOUSEMOVE )
		{
			if ((m_uButtonState & UISTATE_CAPTURED) != 0)
			{
				const UINT previousState = m_uButtonState;
				if (::PtInRect(&m_rcItem, event.ptMouse))
					m_uButtonState |= UISTATE_PUSHED;
				else m_uButtonState &= ~UISTATE_PUSHED;
				if (previousState != m_uButtonState) {
					Invalidate();
				}
			}

			return;
		}
		if( event.Type == UIEVENT_BUTTONUP)
		{
			if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
				m_uButtonState &= ~(UISTATE_PUSHED | UISTATE_CAPTURED);
				Invalidate();
				if( ::PtInRect(&m_rcItem, event.ptMouse) ) Activate();				
			}
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			if( IsContextMenuUsed() ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
			}
			return;
		}
		if( event.Type == UIEVENT_MOUSEENTER )
		{
			if( IsEnabled() ) 
			{
				if ((m_uButtonState & UISTATE_HOT) == 0) {
					m_uButtonState |= UISTATE_HOT;
					Invalidate();
					if(IsRichEvent()) 
						m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSEENTER);
				}
			}
		}
		if( event.Type == UIEVENT_MOUSELEAVE )
		{
			if( IsEnabled() ) 
			{
				if ((m_uButtonState & UISTATE_HOT) != 0) {
					m_uButtonState &= ~UISTATE_HOT;
					Invalidate();

					if(IsRichEvent())
						m_pManager->SendNotify(this, DUI_MSGTYPE_MOUSELEAVE);
				}
			}
		}
		CLabelUI::DoEvent(event);
	}

	bool CButtonUI::Activate()
	{
		std::wstring strName = GetName();
		if( !CControlUI::Activate() ) return false;
		if( m_pManager != NULL )
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_CLICK);
			BindTriggerTabSel();
		}
		return true;
	}

	void CButtonUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButtonState |= UISTATE_DISABLED;
		}
		else {
			m_uButtonState &= ~UISTATE_DISABLED;
		}
	}


	void CButtonUI::SetHotFont(int index)
	{
		m_iHotFont = index;
		Invalidate();
	}

	int CButtonUI::GetHotFont() const
	{
		return m_iHotFont;
	}

	void CButtonUI::SetPushedFont(int index)
	{
		m_iPushedFont = index;
		Invalidate();
	}

	int CButtonUI::GetPushedFont() const
	{
		return m_iPushedFont;
	}

	void CButtonUI::SetFocusedFont(int index)
	{
		m_iFocusedFont = index;
		Invalidate();
	}

	int CButtonUI::GetFocusedFont() const
	{
		return m_iFocusedFont;
	}

	void CButtonUI::SetHotBkColor( DWORD dwColor )
	{
		m_dwHotBkColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetHotBkColor() const
	{
		return m_dwHotBkColor;
	}

	void CButtonUI::SetPushedBkColor( DWORD dwColor )
	{
		m_dwPushedBkColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetPushedBkColor() const
	{
		return m_dwPushedBkColor;
	}

	void CButtonUI::SetDisabledBkColor( DWORD dwColor )
	{
		m_dwDisabledBkColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetDisabledBkColor() const
	{
		return m_dwDisabledBkColor;
	}

	void CButtonUI::SetHotTextColor(DWORD dwColor)
	{
		m_dwHotTextColor = dwColor;
	}

	DWORD CButtonUI::GetHotTextColor() const
	{
		return m_dwHotTextColor;
	}

	void CButtonUI::SetPushedTextColor(DWORD dwColor)
	{
		m_dwPushedTextColor = dwColor;
	}

	DWORD CButtonUI::GetPushedTextColor() const
	{
		return m_dwPushedTextColor;
	}

	void CButtonUI::SetFocusedTextColor(DWORD dwColor)
	{
		m_dwFocusedTextColor = dwColor;
	}

	DWORD CButtonUI::GetFocusedTextColor() const
	{
		return m_dwFocusedTextColor;
	}

	void CButtonUI::SetHotBorderColor(DWORD dwColor)
	{
		if (m_dwHotBorderColor == dwColor) return;

		m_dwHotBorderColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetHotBorderColor() const
	{
		return m_dwHotBorderColor;
	}

	void CButtonUI::SetPushedBorderColor(DWORD dwColor)
	{
		if (m_dwPushedBorderColor == dwColor) return;

		m_dwPushedBorderColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetPushedBorderColor() const
	{
		return m_dwPushedBorderColor;
	}

	void CButtonUI::SetDisabledBorderColor(DWORD dwColor)
	{
		if (m_dwDisabledBorderColor == dwColor) return;

		m_dwDisabledBorderColor = dwColor;
		Invalidate();
	}

	DWORD CButtonUI::GetDisabledBorderColor() const
	{
		return m_dwDisabledBorderColor;
	}

	std::wstring_view CButtonUI::GetNormalImage() const
	{
		return m_sNormalImage;
	}

	void CButtonUI::SetNormalImage(std::wstring_view pStrImage)
	{
		if (pStrImage == m_sNormalImage)
			return;
		m_sNormalImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CButtonUI::GetHotImage() const
	{
		return m_sHotImage;
	}

	void CButtonUI::SetHotImage(std::wstring_view pStrImage)
	{
		if (pStrImage == m_sHotImage)
			return;
		m_sHotImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CButtonUI::GetPushedImage() const
	{
		return m_sPushedImage;
	}

	void CButtonUI::SetPushedImage(std::wstring_view pStrImage)
	{
		if (pStrImage == m_sPushedImage)
			return;
		m_sPushedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CButtonUI::GetFocusedImage() const
	{
		return m_sFocusedImage;
	}

	void CButtonUI::SetFocusedImage(std::wstring_view pStrImage)
	{
		if (pStrImage == m_sFocusedImage)
			return;
		m_sFocusedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CButtonUI::GetDisabledImage() const
	{
		return m_sDisabledImage;
	}

	void CButtonUI::SetDisabledImage(std::wstring_view pStrImage)
	{
		if (pStrImage == m_sDisabledImage)
			return;

		m_sDisabledImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CButtonUI::GetHotForeImage() const
	{
		return m_sHotForeImage;
	}

	void CButtonUI::SetHotForeImage(std::wstring_view pStrImage)
	{
		if (pStrImage == m_sHotForeImage)
			return;

		m_sHotForeImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CButtonUI::GetPushedForeImage() const
	{
		return m_sPushedForeImage;
	}

	void CButtonUI::SetPushedForeImage(std::wstring_view pStrImage)
	{
		if (pStrImage == m_sPushedForeImage)
			return;

		m_sPushedForeImage.assign(pStrImage);
		Invalidate();
	}

	void CButtonUI::SetStateCount(int nCount)
	{
		m_nStateCount = nCount;
		Invalidate();
	}

	int CButtonUI::GetStateCount() const
	{
		return m_nStateCount;
	}

	void CButtonUI::SetState(UINT uButtonState)
	{
		m_uButtonState = uButtonState;
		Invalidate();
	}

	std::wstring_view CButtonUI::GetStateImage() const
	{
		return m_sStateImage;
	}

	void CButtonUI::SetStateImage(std::wstring_view pStrImage)
	{
		m_sNormalImage.clear();
		m_sStateImage.assign(pStrImage);
		Invalidate();
	}

	void CButtonUI::BindTabIndex(int _BindTabIndex )
	{
		if( _BindTabIndex >= 0)
			m_iBindTabIndex	= _BindTabIndex;
	}

	void CButtonUI::BindTabLayoutName(std::wstring_view _TabLayoutName)
	{
		m_sBindTabLayoutName.assign(_TabLayoutName);
	}

	void CButtonUI::BindTriggerTabSel( int _SetSelectIndex /*= -1*/ )
	{
		const std::wstring_view pstrName = GetBindTabLayoutName();
		if(pstrName.empty() || (GetBindTabLayoutIndex() < 0 && _SetSelectIndex < 0))
			return;

		CTabLayoutUI* pTabLayout = static_cast<CTabLayoutUI*>(GetManager()->FindControl(pstrName));
		if(!pTabLayout) return;
		pTabLayout->SelectItem(_SetSelectIndex >=0?_SetSelectIndex:GetBindTabLayoutIndex());
	}

	void CButtonUI::RemoveBindTabIndex()
	{
		m_iBindTabIndex	= -1;
		m_sBindTabLayoutName.clear();
	}

	int CButtonUI::GetBindTabLayoutIndex()
	{
		return m_iBindTabIndex;
	}

	std::wstring_view CButtonUI::GetBindTabLayoutName() const
	{
		return m_sBindTabLayoutName;
	}

	void CButtonUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
	{
		const std::wstring_view name = StringUtil::TrimView(pstrNameView);
		if (name.empty()) {
			return;
		}

		if (StringUtil::EqualsNoCase(name, L"normalimage")) SetNormalImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"hotimage")) SetHotImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"pushedimage")) SetPushedImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"focusedimage")) SetFocusedImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"disabledimage")) SetDisabledImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"hotforeimage")) SetHotForeImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"pushedforeimage")) SetPushedForeImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"stateimage")) SetStateImage(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"statecount")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetStateCount(value);
		}
		else if (StringUtil::EqualsNoCase(name, L"bindtabindex")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) BindTabIndex(value);
		}
		else if (StringUtil::EqualsNoCase(name, L"bindtablayoutname")) BindTabLayoutName(pstrValueView);
		else if (StringUtil::EqualsNoCase(name, L"hotbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetHotBkColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"pushedbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetPushedBkColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"disabledbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetDisabledBkColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"hottextcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetHotTextColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"pushedtextcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetPushedTextColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"focusedtextcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetFocusedTextColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"hotbordercolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetHotBorderColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"pushedbordercolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetPushedBorderColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"disabledbordercolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetDisabledBorderColor(color);
		}
		else if (StringUtil::EqualsNoCase(name, L"hotfont")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetHotFont(value);
		}
		else if (StringUtil::EqualsNoCase(name, L"pushedfont")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetPushedFont(value);
		}
		else if (StringUtil::EqualsNoCase(name, L"focuedfont")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetFocusedFont(value);
		}
		else CLabelUI::SetAttribute(pstrNameView, pstrValueView);
	}

	void CButtonUI::PaintText(CPaintRenderContext& renderContext)
	{
		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;

		if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		std::wstring sText = GetText();
		if( sText.empty() ) return;

		RECT m_rcTextPadding = CButtonUI::m_rcTextPadding;
		m_rcTextPadding = ScaleRect(m_rcTextPadding);
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;

		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0) )
			clrColor = GetPushedTextColor();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0) )
			clrColor = GetHotTextColor();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0) )
			clrColor = GetFocusedTextColor();

		int iFont = GetFont();
		if( ((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedFont() != -1) )
			iFont = GetPushedFont();
		else if( ((m_uButtonState & UISTATE_HOT) != 0) && (GetHotFont() != -1) )
			iFont = GetHotFont();
		else if( ((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedFont() != -1) )
			iFont = GetFocusedFont();

		if( m_bShowHtml )
			CRenderEngine::DrawHtmlText(renderContext, rc, sText, clrColor, \
				NULL, NULL, nLinks, iFont, m_uTextStyle);
		else
			CRenderEngine::DrawText(renderContext, rc, sText, clrColor, \
				iFont, m_uTextStyle);
	}

	void CButtonUI::PaintBkColor(CPaintRenderContext& renderContext)
	{
		const SIZE cxyBorderRound = GetBorderRound();
		if( (m_uButtonState & UISTATE_DISABLED) != 0 ) {
			if(m_dwDisabledBkColor != 0) {
				DrawButtonStateColor(renderContext, m_rcPaint, m_rcItem, cxyBorderRound, GetAdjustColor(m_dwDisabledBkColor));
				return;
			}
		}
		else if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if(m_dwPushedBkColor != 0) {
				DrawButtonStateColor(renderContext, m_rcPaint, m_rcItem, cxyBorderRound, GetAdjustColor(m_dwPushedBkColor));
				return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if(m_dwHotBkColor != 0) {
				DrawButtonStateColor(renderContext, m_rcPaint, m_rcItem, cxyBorderRound, GetAdjustColor(m_dwHotBkColor));
				return;
			}
		}

		return CControlUI::PaintBkColor(renderContext);
	}

	void CButtonUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		if(!m_sStateImage.empty() && m_nStateCount > 0)
		{
			TDrawInfo info;
			info.Parse(m_sStateImage, _T(""), m_pManager);
			const TImageInfo* pImage = m_pManager->GetImageEx(info.sImageName, info.sResType, info.dwMask, info.bHSL, info.bGdiplus);
			if(m_sNormalImage.empty() && pImage != NULL)
			{
				SIZE szImage = {pImage->nX, pImage->nY};
				SIZE szStatus = {pImage->nX / m_nStateCount, pImage->nY};
				if( szImage.cx > 0 && szImage.cy > 0 )
				{
					RECT rcSrc = {0, 0, szImage.cx, szImage.cy};
					if(m_nStateCount > 0) {
						int iLeft = rcSrc.left + 0 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sNormalImage = StringUtil::Format(L"res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'",
							info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom,
							iLeft, iTop, iRight, iBottom);
					}
					if(m_nStateCount > 1) {
						int iLeft = rcSrc.left + 1 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sHotImage = StringUtil::Format(L"res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'",
							info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom,
							iLeft, iTop, iRight, iBottom);
						m_sPushedImage = m_sHotImage;
					}
					if(m_nStateCount > 2) {
						int iLeft = rcSrc.left + 2 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sPushedImage = StringUtil::Format(L"res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'",
							info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom,
							iLeft, iTop, iRight, iBottom);
					}
					if(m_nStateCount > 3) {
						int iLeft = rcSrc.left + 3 * szStatus.cx;
						int iRight = iLeft + szStatus.cx;
						int iTop = rcSrc.top;
						int iBottom = iTop + szStatus.cy;
						m_sDisabledImage = StringUtil::Format(L"res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}'",
							info.sImageName, info.sResType, info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom,
							iLeft, iTop, iRight, iBottom);
					}
				}
			}
		}

		if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~ UISTATE_FOCUSED;
		if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~ UISTATE_DISABLED;
		if(!::IsWindowEnabled(m_pManager->GetPaintWindow())) {
			m_uButtonState &= UISTATE_DISABLED;
		}
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
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHotImage.empty() ) {
				if( !DrawImage(renderContext, m_sHotImage) ) {}
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
			if( !DrawImage(renderContext, m_sNormalImage) ) {}
		}
	}

	void CButtonUI::PaintBorder(CPaintRenderContext& renderContext)
	{
		if ((m_uButtonState & UISTATE_DISABLED) != 0) {
			if (m_dwDisabledBorderColor != 0) {
				DrawBorder(renderContext, m_rcItem, GetAdjustColor(m_dwDisabledBorderColor), m_nBorderSize, m_rcBorderSize, m_cxyBorderRound, m_nBorderStyle);
				return;
			}
		}
		else if ((m_uButtonState & UISTATE_PUSHED) != 0) {
			if (m_dwPushedBorderColor != 0) {
				DrawBorder(renderContext, m_rcItem, GetAdjustColor(m_dwPushedBorderColor), m_nBorderSize, m_rcBorderSize, m_cxyBorderRound, m_nBorderStyle);
				return;
			}
		}
		else if ((m_uButtonState & UISTATE_HOT) != 0) {
			if (m_dwHotBorderColor != 0) {
				DrawBorder(renderContext, m_rcItem, GetAdjustColor(m_dwHotBorderColor), m_nBorderSize, m_rcBorderSize, m_cxyBorderRound, m_nBorderStyle);
				return;
			}
		}
		return CControlUI::PaintBorder(renderContext);
	}

	void CButtonUI::PaintForeImage(CPaintRenderContext& renderContext)
	{
		if( (m_uButtonState & UISTATE_PUSHED) != 0 ) {
			if( !m_sPushedForeImage.empty() ) {
				if( !DrawImage(renderContext, m_sPushedForeImage) ) {}
				else return;
			}
		}
		else if( (m_uButtonState & UISTATE_HOT) != 0 ) {
			if( !m_sHotForeImage.empty() ) {
				if( !DrawImage(renderContext, m_sHotForeImage) ) {}
				else return;
			}
		}
		if(!m_sForeImage.empty() ) {
			if( !DrawImage(renderContext, m_sForeImage) ) {}
		}
	}

	void CButtonUI::DrawBorder(CPaintRenderContext& renderContext, const RECT& rcItem, const DWORD& dwBorderColor, const int& nBorderSize, const RECT& rcBorderSize, const SIZE& cxyBorderRound, const int& nBorderStyle)
	{
		if (dwBorderColor != 0) {
			//禄颅脭虏陆脟卤脽驴貌
			if (nBorderSize > 0 && (cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0)) {
				CRenderEngine::DrawRoundRect(renderContext, rcItem, nBorderSize, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(dwBorderColor), nBorderStyle);
			}
			else {
				if (rcBorderSize.left > 0 || rcBorderSize.top > 0 || rcBorderSize.right > 0 || rcBorderSize.bottom > 0) {
					RECT rcBorder;

					if (rcBorderSize.left > 0) {
						rcBorder = rcItem;
						rcBorder.right = rcBorder.left;
						CRenderEngine::DrawLine(renderContext, rcBorder, rcBorderSize.left, GetAdjustColor(dwBorderColor), nBorderStyle);
					}
					if (rcBorderSize.top > 0) {
						rcBorder = rcItem;
						rcBorder.bottom = rcBorder.top;
						CRenderEngine::DrawLine(renderContext, rcBorder, rcBorderSize.top, GetAdjustColor(dwBorderColor), nBorderStyle);
					}
					if (rcBorderSize.right > 0) {
						rcBorder = rcItem;
						rcBorder.right -= 1;
						rcBorder.left = rcBorder.right;
						CRenderEngine::DrawLine(renderContext, rcBorder, rcBorderSize.right, GetAdjustColor(dwBorderColor), nBorderStyle);
					}
					if (rcBorderSize.bottom > 0) {
						rcBorder = rcItem;
						rcBorder.bottom -= 1;
						rcBorder.top = rcBorder.bottom;
						CRenderEngine::DrawLine(renderContext, rcBorder, rcBorderSize.bottom, GetAdjustColor(dwBorderColor), nBorderStyle);
					}
				}
				else if (nBorderSize > 0) {
					CRenderEngine::DrawRect(renderContext, rcItem, nBorderSize, GetAdjustColor(dwBorderColor), nBorderStyle);
				}
			}
		}
	}

	CButtonUI* CButtonUI::Clone()
	{
		CButtonUI* pClone = new CButtonUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CButtonUI::CopyData(CButtonUI* pControl)
	{
		m_uButtonState = pControl->m_uButtonState;

		m_iHotFont = pControl->m_iHotFont;
		m_iPushedFont = pControl->m_iPushedFont ;
		m_iFocusedFont = pControl->m_iFocusedFont;

		m_dwHotBkColor = pControl->m_dwHotBkColor;
		m_dwPushedBkColor = pControl->m_dwPushedBkColor;
		m_dwDisabledBkColor = pControl->m_dwDisabledBkColor;
		m_dwHotTextColor = pControl->m_dwHotTextColor;
		m_dwPushedTextColor = pControl->m_dwPushedTextColor;
		m_dwFocusedTextColor = pControl->m_dwFocusedTextColor;
		m_dwHotBorderColor = pControl->m_dwHotBorderColor;
		m_dwPushedBorderColor = pControl->m_dwPushedBorderColor;
		m_dwDisabledBorderColor = pControl->m_dwDisabledBorderColor;

		m_sNormalImage= pControl->m_sNormalImage;
		m_sHotImage= pControl->m_sHotImage;
		m_sHotForeImage= pControl->m_sHotForeImage;
		m_sPushedImage= pControl->m_sPushedImage;
		m_sPushedForeImage= pControl->m_sPushedForeImage;
		m_sFocusedImage= pControl->m_sFocusedImage;
		m_sDisabledImage= pControl->m_sDisabledImage;
		m_nStateCount= pControl->m_nStateCount;
		m_sStateImage= pControl->m_sStateImage;

		m_iBindTabIndex= pControl->m_iBindTabIndex;
		m_sBindTabLayoutName= pControl->m_sBindTabLayoutName;

		__super::CopyData(pControl);
	}



}


