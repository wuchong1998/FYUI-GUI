#include "pch.h"
#include "UIComboBox.h"
#include "../Core/Render/UIRenderContext.h"

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CComboBoxUI)

		CComboBoxUI::CComboBoxUI()
	{
		m_nArrowWidth = 0;
	}

	std::wstring_view CComboBoxUI::GetClass() const
	{
		return _T("ComboBoxUI");
	}

	void CComboBoxUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("arrowimage")) == 0)
			m_sArrowImage = pstrValue;
		else
			CComboUI::SetAttribute(pstrName, pstrValue);
	}

	void CComboBoxUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		if (m_sArrowImage.empty())
			CComboUI::PaintStatusImage(renderContext);
		else
		{
			// get index
			if( IsFocused() ) m_uButtonState |= UISTATE_FOCUSED;
			else m_uButtonState &= ~ UISTATE_FOCUSED;
			if( !IsEnabled() ) m_uButtonState |= UISTATE_DISABLED;
			else m_uButtonState &= ~ UISTATE_DISABLED;

			int nIndex = 0;
			if ((m_uButtonState & UISTATE_DISABLED) != 0)
				nIndex = 4;
			else if ((m_uButtonState & UISTATE_PUSHED) != 0)
				nIndex = 2;
			else if ((m_uButtonState & UISTATE_HOT) != 0)
				nIndex = 1;
			else if ((m_uButtonState & UISTATE_FOCUSED) != 0)
				nIndex = 3;

			// make modify string
			std::wstring sModify = m_sArrowImage;

			int nPos1 = StringUtil::Find(sModify, _T("source"));
			int nPos2 = StringUtil::Find(sModify, _T('\''), nPos1 + 7);
			if (nPos2 == -1) return; //first
			int nPos3 = StringUtil::Find(sModify, _T('\''), nPos2 + 1);
			if (nPos3 == -1) return; //second

			CDuiRect rcBmpPart;
			if (!StringUtil::TryParseRect(std::wstring_view(sModify).substr(nPos2 + 1, nPos3 - nPos2 - 1), rcBmpPart)) {
				return;
			}

			m_nArrowWidth = rcBmpPart.GetWidth() / 5;
			rcBmpPart.left += nIndex * m_nArrowWidth;
			rcBmpPart.right = rcBmpPart.left + m_nArrowWidth;

			CDuiRect rcDest(0, 0, m_rcItem.right - m_rcItem.left, m_rcItem.bottom - m_rcItem.top);
			rcDest.Deflate(GetBorderSize(), GetBorderSize());
			rcDest.left = rcDest.right - m_nArrowWidth;

			std::wstring sSource = StringUtil::Mid(sModify, nPos1, nPos3 + 1 - nPos1);
			std::wstring sReplace = StringUtil::Format(L"source='{},{},{},{}' dest='{},{},{},{}'",
				rcBmpPart.left, rcBmpPart.top, rcBmpPart.right, rcBmpPart.bottom,
				rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);

			StringUtil::ReplaceAll(sModify, sSource, sReplace);

			// draw image
			if (!DrawImage(renderContext, m_sArrowImage, sModify))
			{}
		}
	}

	void CComboBoxUI::PaintText(CPaintRenderContext& renderContext)
	{
		RECT rcText = m_rcItem;
		const RECT rcTextPadding = GetTextPadding();
		rcText.left += rcTextPadding.left;
		rcText.right -= rcTextPadding.right;
		rcText.top += rcTextPadding.top;
		rcText.bottom -= rcTextPadding.bottom;

		rcText.right -= m_nArrowWidth; // add this line than CComboUI::PaintText

		if( m_iCurSel >= 0 ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[m_iCurSel]);
			IListItemUI* pElement = static_cast<IListItemUI*>(pControl->GetInterface(_T("ListItem")));
			if( pElement != NULL ) {
				pElement->DrawItemText(renderContext, rcText);
			}
			else {
				RECT rcOldPos = pControl->GetPos();
				pControl->SetPos(rcText);
				pControl->DoPaint(renderContext, NULL);
				pControl->SetPos(rcOldPos);
			}
		}
	}
}


