#include "pch.h"
#include "UIOption.h"
#include "../Core/UIRenderContext.h"

namespace FYUI
{
	IMPLEMENT_DUICONTROL(COptionUI)

	namespace
	{
		void DrawOptionStateColor(CPaintRenderContext& renderContext, const RECT& rcPaint, const RECT& rcItem, SIZE round, DWORD color)
		{
			if (round.cx > 0 || round.cy > 0) {
				CRenderEngine::DrawRoundColor(renderContext, rcItem, round.cx, round.cy, color);
				return;
			}
			CRenderEngine::DrawColor(renderContext, rcPaint, color);
		}
	}

	COptionUI::COptionUI()
		: m_bSelected(false)
		, m_iSelectedFont(-1)
		, m_dwSelectedTextColor(0)
		, m_dwSelectedBkColor(0)
		, m_dwSelectedDisableBkColor(0)
		, m_nSelectedStateCount(0)
	{
	}

	COptionUI::~COptionUI()
	{
		if( !m_sGroupName.empty() && m_pManager ) m_pManager->RemoveOptionGroup(m_sGroupName, this);
	}

	std::wstring_view COptionUI::GetClass() const
	{
		return _T("OptionUI");
	}

	LPVOID COptionUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, DUI_CTR_OPTION) == 0 ) return static_cast<COptionUI*>(this);
		return CButtonUI::GetInterface(pstrName);
	}

	void COptionUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		CControlUI::SetManager(pManager, pParent, bInit);
		if( bInit && !m_sGroupName.empty() ) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
	}

	std::wstring_view COptionUI::GetGroup() const
	{
		return m_sGroupName;
	}

	void COptionUI::SetGroup(std::wstring_view groupName)
	{
		if (groupName.empty()) {
			if (m_sGroupName.empty()) return;
			m_sGroupName.clear();
		}
		else {
			if (m_sGroupName == groupName) return;
			if (!m_sGroupName.empty() && m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
			m_sGroupName = groupName;
		}

		if (!m_sGroupName.empty()) {
			if (m_pManager) m_pManager->AddOptionGroup(m_sGroupName, this);
		}
		else {
			if (m_pManager) m_pManager->RemoveOptionGroup(m_sGroupName, this);
		}

		Selected(m_bSelected);
	}

	std::wstring_view COptionUI::GetGroupType() const
	{
		return m_sGroupType;
	}

	void COptionUI::SetGroupType(std::wstring_view groupType)
	{
		if (groupType.empty()) {
			if (m_sGroupType.empty()) return;
			m_sGroupType.clear();
		}
		else {
			m_sGroupType = groupType;
		}
	}

	bool COptionUI::IsSelected() const
	{
		return m_bSelected;
	}

	void COptionUI::Selected(bool bSelected, bool bMsg/* = true*/)
	{
		if(m_bSelected == bSelected) return;
		m_bSelected = bSelected;
		if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if( m_pManager != NULL ) {
			if( !m_sGroupName.empty() ) {
				if( m_bSelected ) {
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
						COptionUI* pControl = static_cast<COptionUI*>(aOptionGroup->GetAt(i));
						if( pControl != this && (m_sGroupType.empty() || m_sGroupType.compare(pControl->GetGroupType()) != 0)) {
							pControl->Selected(false, bMsg);
						}
					}
					if(bMsg) {
						m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
					}
				}
				else {
					if(bMsg) {
						m_pManager->SendNotify(this, DUI_MSGTYPE_UNSELECTED);
					}
				}
			}
			else {
				if(bMsg) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED);
				}
			}
		}

		Invalidate();
	}

	bool COptionUI::Activate()
	{
		if( !CButtonUI::Activate() ) return false;
		if( !m_sGroupName.empty() ) {
			Selected(true);
		}
		else Selected(!m_bSelected);

		Invalidate();
		return true;
	}

	std::wstring_view COptionUI::GetSelectedImage()
	{
		return m_sSelectedImage;
	}

	void COptionUI::SetSelectedImage(std::wstring_view image)
	{
		if (m_sSelectedImage == image)
			return;
		m_sSelectedImage = image;
		Invalidate();
	}

	std::wstring_view COptionUI::GetSelectedHotImage()
	{
		return m_sSelectedHotImage;
	}

	void COptionUI::SetSelectedHotImage(std::wstring_view image)
	{
		if (m_sSelectedHotImage == image)
			return;

		m_sSelectedHotImage = image;
		Invalidate();
	}

	std::wstring_view COptionUI::GetSelectedPushedImage()
	{
		return m_sSelectedPushedImage;
	}

	void COptionUI::SetSelectedPushedImage(std::wstring_view image)
	{
		if (m_sSelectedPushedImage == image)
			return;

		m_sSelectedPushedImage = image;
		Invalidate();
	}

	void COptionUI::SetSelectedTextColor(DWORD dwTextColor)
	{
		m_dwSelectedTextColor = dwTextColor;
	}

	DWORD COptionUI::GetSelectedTextColor()
	{
		if (m_dwSelectedTextColor == 0) m_dwSelectedTextColor = m_pManager->GetDefaultFontColor();
		return m_dwSelectedTextColor;
	}

	void COptionUI::SetSelectedBkColor( DWORD dwBkColor )
	{
		m_dwSelectedBkColor = dwBkColor;
	}

	DWORD COptionUI::GetSelectBkColor()
	{
		return m_dwSelectedBkColor;
	}

	void COptionUI::SetSelectedDisableBkColor(DWORD dwBkColor)
	{
		m_dwSelectedDisableBkColor = dwBkColor;
	}

	DWORD COptionUI::GetSelectedDisableBkColor()
	{
		return m_dwSelectedDisableBkColor;
	}

	std::wstring_view COptionUI::GetSelectedForedImage()
	{
		return m_sSelectedForeImage;
	}

	void COptionUI::SetSelectedForedImage(std::wstring_view image)
	{
		if (m_sSelectedForeImage == image)
			return;

		m_sSelectedForeImage = image;
		Invalidate();
	}

	void COptionUI::SetSelectedStateCount(int nCount)
	{
		m_nSelectedStateCount = nCount;
		Invalidate();
	}

	int COptionUI::GetSelectedStateCount() const
	{
		return m_nSelectedStateCount;
	}

	std::wstring_view COptionUI::GetSelectedStateImage()
	{
		return m_sSelectedStateImage;
	}

	void COptionUI::SetSelectedStateImage(std::wstring_view image)
	{
		m_sSelectedStateImage = image;
		Invalidate();
	}
	void COptionUI::SetSelectedFont(int index)
	{
		m_iSelectedFont = index;
		Invalidate();
	}

	int COptionUI::GetSelectedFont() const
	{
		return m_iSelectedFont;
	}
	void COptionUI::SetAttribute(std::wstring_view nameView, std::wstring_view valueView)
	{
		if (StringUtil::EqualsNoCase(nameView, L"group")) SetGroup(valueView);
		else if (StringUtil::EqualsNoCase(nameView, L"grouptype")) SetGroupType(valueView);
		else if (StringUtil::EqualsNoCase(nameView, L"selected")) Selected(StringUtil::ParseBool(valueView));
		else if (StringUtil::EqualsNoCase(nameView, L"selectedimage")) SetSelectedImage(valueView);
		else if (StringUtil::EqualsNoCase(nameView, L"selectedhotimage")) SetSelectedHotImage(valueView);
		else if (StringUtil::EqualsNoCase(nameView, L"selectedpushedimage")) SetSelectedPushedImage(valueView);
		else if (StringUtil::EqualsNoCase(nameView, L"selectedforeimage")) SetSelectedForedImage(valueView);
		else if (StringUtil::EqualsNoCase(nameView, L"selectedstateimage")) SetSelectedStateImage(valueView);
		else if (StringUtil::EqualsNoCase(nameView, L"selectedstatecount")) {
			int stateCount = 0;
			if (StringUtil::TryParseInt(valueView, stateCount)) {
				SetSelectedStateCount(stateCount);
			}
		}
		else if (StringUtil::EqualsNoCase(nameView, L"selectedbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(valueView, color)) {
				SetSelectedBkColor(color);
			}
		}
		else if (StringUtil::EqualsNoCase(nameView, L"selecteddisabledbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(valueView, color)) {
				SetSelectedDisableBkColor(color);
			}
		}
		else if (StringUtil::EqualsNoCase(nameView, L"selectedtextcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(valueView, color)) {
				SetSelectedTextColor(color);
			}
		}
		else if (StringUtil::EqualsNoCase(nameView, L"selectedfont")) {
			int selectedFont = 0;
			if (StringUtil::TryParseInt(valueView, selectedFont)) {
				SetSelectedFont(selectedFont);
			}
		}
		else CButtonUI::SetAttribute(nameView, valueView);
	}

	void COptionUI::PaintBkColor(CPaintRenderContext& renderContext)
	{
		if(IsSelected()) {
			const SIZE cxyBorderRound = GetBorderRound();

			if (m_dwSelectedDisableBkColor != 0 && !IsEnabled()) 
			{
				DrawOptionStateColor(renderContext, m_rcPaint, m_rcItem, cxyBorderRound, GetAdjustColor(m_dwSelectedDisableBkColor));
			}
			else if(m_dwSelectedBkColor != 0 ) {
				DrawOptionStateColor(renderContext, m_rcPaint, m_rcItem, cxyBorderRound, GetAdjustColor(m_dwSelectedBkColor));
			}
		}
		else {
			return CButtonUI::PaintBkColor(renderContext);
		}
	}

	void COptionUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		if(IsSelected()) {
			if(!m_sSelectedStateImage.empty() && m_nSelectedStateCount > 0)
			{
				TDrawInfo info;
				info.Parse(m_sSelectedStateImage, _T(""), m_pManager);
				const TImageInfo* pImage = m_pManager->GetImageEx(info.sImageName, info.sResType, info.dwMask, info.bHSL, info.bGdiplus);
				if(m_sSelectedImage.empty() && pImage != NULL)
				{
					SIZE szImage = {pImage->nX, pImage->nY};
					SIZE szStatus = {pImage->nX / m_nSelectedStateCount, pImage->nY};
					if( szImage.cx > 0 && szImage.cy > 0 )
					{
						RECT rcSrc = {0, 0, szImage.cx, szImage.cy};
						if(m_nSelectedStateCount > 0) {
							int iLeft = rcSrc.left + 0 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sSelectedImage = StringUtil::Format(
								L"res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}' corner='{},{},{},{}'",
								info.sImageName, info.sResType,
								info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom,
								iLeft, iTop, iRight, iBottom,
								info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
						}
						if(m_nSelectedStateCount > 1) {
							int iLeft = rcSrc.left + 1 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sSelectedHotImage = StringUtil::Format(
								L"res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}' corner='{},{},{},{}'",
								info.sImageName, info.sResType,
								info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom,
								iLeft, iTop, iRight, iBottom,
								info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
							m_sSelectedPushedImage = StringUtil::Format(
								L"res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}' corner='{},{},{},{}'",
								info.sImageName, info.sResType,
								info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom,
								iLeft, iTop, iRight, iBottom,
								info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
						}
						if(m_nSelectedStateCount > 2) {
							int iLeft = rcSrc.left + 2 * szStatus.cx;
							int iRight = iLeft + szStatus.cx;
							int iTop = rcSrc.top;
							int iBottom = iTop + szStatus.cy;
							m_sSelectedPushedImage = StringUtil::Format(
								L"res='{}' restype='{}' dest='{},{},{},{}' source='{},{},{},{}' corner='{},{},{},{}'",
								info.sImageName, info.sResType,
								info.rcDest.left, info.rcDest.top, info.rcDest.right, info.rcDest.bottom,
								iLeft, iTop, iRight, iBottom,
								info.rcCorner.left, info.rcCorner.top, info.rcCorner.right, info.rcCorner.bottom);
						}
					}
				}
			}

			if( (m_uButtonState & UISTATE_PUSHED) != 0 && !m_sSelectedPushedImage.empty()) {
				if( !DrawImage(renderContext, m_sSelectedPushedImage) ) {}
				else return;
			}
			else if( (m_uButtonState & UISTATE_HOT) != 0 && !m_sSelectedHotImage.empty()) {
				if( !DrawImage(renderContext, m_sSelectedHotImage) ) {}
				else return;
			}

			if( !m_sSelectedImage.empty() ) {
				if( !DrawImage(renderContext, m_sSelectedImage) ) {}
			}
		}
		else {
			CButtonUI::PaintStatusImage(renderContext);
		}
	}

	void COptionUI::PaintForeImage(CPaintRenderContext& renderContext)
	{
		if(IsSelected()) {
			if( !m_sSelectedForeImage.empty() ) {
				if( !DrawImage(renderContext, m_sSelectedForeImage) ) {}
				else return;
			}
		}

		return CButtonUI::PaintForeImage(renderContext);
	}

	void COptionUI::PaintText(CPaintRenderContext& renderContext)
	{
		if( (m_uButtonState & UISTATE_SELECTED) != 0 )
		{
			DWORD oldTextColor = m_dwTextColor;
			if( m_dwSelectedTextColor != 0 ) m_dwTextColor = m_dwSelectedTextColor;

			if( m_dwTextColor == 0 ) m_dwTextColor = m_pManager->GetDefaultFontColor();
			if( m_dwDisabledTextColor == 0 ) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

			int iFont = GetFont();
			if(GetSelectedFont() != -1) {
				iFont = GetSelectedFont();
			}
			std::wstring sText = GetText();
			if( sText.empty() ) return;
			int nLinks = 0;
			RECT rc = m_rcItem;
			RECT rcTextPadding = GetTextPadding();
			rc.left += rcTextPadding.left;
			rc.right -= rcTextPadding.right;
			rc.top += rcTextPadding.top;
			rc.bottom -= rcTextPadding.bottom;

			if( m_bShowHtml )
				CRenderEngine::DrawHtmlText(renderContext, rc, sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
					NULL, NULL, nLinks, iFont, m_uTextStyle);
			else
				CRenderEngine::DrawText(renderContext, rc, sText, IsEnabled()?m_dwTextColor:m_dwDisabledTextColor, \
					iFont, m_uTextStyle);

			m_dwTextColor = oldTextColor;
		}
		else
			CButtonUI::PaintText(renderContext);
	}

	COptionUI* COptionUI::Clone()
	{
		COptionUI* pClone = new COptionUI();
		pClone->CopyData(this);

		return pClone;
	}

	void COptionUI::CopyData(COptionUI* pControl)
	{
		m_bSelected = pControl->m_bSelected;
		m_sGroupName = pControl->m_sGroupName;
		m_sGroupType = pControl->m_sGroupType;

		m_iSelectedFont = pControl->m_iSelectedFont;

		m_dwSelectedBkColor = pControl->m_dwSelectedBkColor;
		m_dwSelectedTextColor = pControl->m_dwSelectedTextColor;

		m_sSelectedImage = pControl->m_sSelectedImage;
		m_sSelectedHotImage = pControl->m_sSelectedHotImage;
		m_sSelectedPushedImage = pControl->m_sSelectedPushedImage;
		m_sSelectedForeImage = pControl->m_sSelectedForeImage;

		m_nSelectedStateCount = pControl->m_nSelectedStateCount;
		m_sSelectedStateImage = pControl->m_sSelectedStateImage;
		__super::CopyData(pControl);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//
	IMPLEMENT_DUICONTROL(CCheckBoxUI)

		CCheckBoxUI::CCheckBoxUI() : m_bAutoCheck(false)
	{

	}

	std::wstring_view CCheckBoxUI::GetClass() const
	{
		return _T("CheckBoxUI");
	}
	LPVOID CCheckBoxUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, DUI_CTR_CHECKBOX) == 0 ) return static_cast<CCheckBoxUI*>(this);
		return COptionUI::GetInterface(pstrName);
	}

	void CCheckBoxUI::SetCheck(bool bCheck)
	{
		Selected(bCheck);
	}

	bool  CCheckBoxUI::GetCheck() const
	{
		return IsSelected();
	}

	void CCheckBoxUI::SetAttribute(std::wstring_view nameView, std::wstring_view valueView)
	{
		const std::wstring nameStorage(nameView);
		const std::wstring valueStorage(valueView);
		const wchar_t* pstrName = nameStorage.c_str();
		const wchar_t* pstrValue = valueStorage.c_str();

		if( StringUtil::CompareNoCase(pstrName, _T("EnableAutoCheck")) == 0 ) SetAutoCheck(StringUtil::ParseBool(pstrValue));

		COptionUI::SetAttribute(pstrName, pstrValue);
	}

	void CCheckBoxUI::SetAutoCheck(bool bEnable)
	{
		m_bAutoCheck = bEnable;
	}

	void CCheckBoxUI::DoEvent(TEventUI& event)
	{
		if( !IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND ) {
			if( m_pParent != NULL ) m_pParent->DoEvent(event);
			else COptionUI::DoEvent(event);
			return;
		}

		if( m_bAutoCheck && (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)) {
			if( ::PtInRect(&m_rcItem, event.ptMouse) && IsEnabled() ) {
				SetCheck(!GetCheck()); 
				m_pManager->SendNotify(this, DUI_MSGTYPE_CHECKCLICK, 0, 0);
				Invalidate();
			}
			return;
		}
		COptionUI::DoEvent(event);
	}

	void CCheckBoxUI::Selected(bool bSelected, bool bMsg/* = true*/)
	{
		if( m_bSelected == bSelected ) return;

		m_bSelected = bSelected;
		if( m_bSelected ) m_uButtonState |= UISTATE_SELECTED;
		else m_uButtonState &= ~UISTATE_SELECTED;

		if( m_pManager != NULL ) {
			if( !m_sGroupName.empty() ) {
				if( m_bSelected ) {
					CStdPtrArray* aOptionGroup = m_pManager->GetOptionGroup(m_sGroupName);
					for( int i = 0; i < aOptionGroup->GetSize(); i++ ) {
						COptionUI* pControl = static_cast<COptionUI*>(aOptionGroup->GetAt(i));
						if( pControl != this ) {
							pControl->Selected(false, bMsg);
						}
					}
					if(bMsg) {
						m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED, m_bSelected, 0);
					}
				}
			}
			else {
				if(bMsg) {
					m_pManager->SendNotify(this, DUI_MSGTYPE_SELECTCHANGED, m_bSelected, 0);
				}
			}
		}

		Invalidate();
	}

	CCheckBoxUI* CCheckBoxUI::Clone()
	{
		CCheckBoxUI* pClone = new CCheckBoxUI();
		pClone->CopyData(this);
		return pClone;
	}
	void CCheckBoxUI::CopyData(CCheckBoxUI* pControl)
	{
		m_bAutoCheck = pControl->m_bAutoCheck;
		__super::CopyData(pControl);
	}
}


