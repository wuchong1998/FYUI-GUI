#include "pch.h"
#include "UIButtonEx.h"
#include "../Core/UIRenderContext.h"
#include "../Core/UIRenderSurface.h"
#include <cmath>

namespace FYUI
{
	namespace
	{
		std::wstring ButtonExToWideText(std::wstring_view pstrText)
		{
			return std::wstring(pstrText);
		}

		bool TryGetButtonExIconGlyph(std::wstring_view pstrText, std::wstring& glyphText)
		{
			const std::wstring text = ButtonExToWideText(pstrText);
			const size_t tagPos = text.find(L"&#x");
			if (tagPos == std::wstring::npos) {
				return false;
			}

			size_t hexEnd = tagPos + 3;
			while (hexEnd < text.length()) {
				const wchar_t ch = text[hexEnd];
				if ((ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F')) {
					++hexEnd;
					continue;
				}
				break;
			}
			if (hexEnd <= tagPos + 3) {
				return false;
			}

			const std::wstring hexValue = text.substr(tagPos + 3, hexEnd - (tagPos + 3));
			const unsigned long codePoint = std::wcstoul(hexValue.c_str(), nullptr, 16);
			if (codePoint == 0 || codePoint > 0x10FFFFUL) {
				return false;
			}

			glyphText.clear();
			if (codePoint <= 0xFFFFUL) {
				glyphText.push_back(static_cast<wchar_t>(codePoint));
			}
			else {
				const unsigned long surrogateValue = codePoint - 0x10000UL;
				glyphText.push_back(static_cast<wchar_t>(0xD800 + (surrogateValue >> 10)));
				glyphText.push_back(static_cast<wchar_t>(0xDC00 + (surrogateValue & 0x3FF)));
			}
			return true;
		}

	}

	IMPLEMENT_DUICONTROL(CButtonExUI)

	CButtonExUI::CButtonExUI()
	{

	}

	std::wstring_view CButtonExUI::GetClass() const
	{
		return _T("ButtonExUI");
	}

	LPVOID CButtonExUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_BUTTONEX) == 0) return static_cast<CButtonExUI*>(this);
		return nullptr;
	}

	void CButtonExUI::PaintText(CPaintRenderContext& renderContext)
	{
		if (IsFocused()) m_uButtonState |= UISTATE_FOCUSED;
		else m_uButtonState &= ~UISTATE_FOCUSED;
		if (!IsEnabled()) m_uButtonState |= UISTATE_DISABLED;
		else m_uButtonState &= ~UISTATE_DISABLED;

		if (m_dwTextColor == 0) m_dwTextColor = m_pManager->GetDefaultFontColor();
		if (m_dwDisabledTextColor == 0) m_dwDisabledTextColor = m_pManager->GetDefaultDisabledColor();

		std::wstring sText = GetText();
		if (sText.empty()) return;

		RECT m_rcTextPadding = CButtonExUI::m_rcTextPadding;
		m_rcTextPadding = ScaleRect(m_rcTextPadding);
		int nLinks = 0;
		RECT rc = m_rcItem;
		rc.left += m_rcTextPadding.left;
		rc.right -= m_rcTextPadding.right;
		rc.top += m_rcTextPadding.top;
		rc.bottom -= m_rcTextPadding.bottom;

		DWORD clrColor = IsEnabled() ? m_dwTextColor : m_dwDisabledTextColor;

		if (((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedTextColor() != 0))
			clrColor = GetPushedTextColor();
		else if (((m_uButtonState & UISTATE_HOT) != 0) && (GetHotTextColor() != 0))
			clrColor = GetHotTextColor();
		else if (((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedTextColor() != 0))
			clrColor = GetFocusedTextColor();
		else if (((m_uButtonState & UISTATE_HOT) != 0))
			sText = GetHotText();

		int iFont = GetFont();
		if (((m_uButtonState & UISTATE_PUSHED) != 0) && (GetPushedFont() != -1))
			iFont = GetPushedFont();
		else if (((m_uButtonState & UISTATE_HOT) != 0) && (GetHotFont() != -1))
			iFont = GetHotFont();
		else if (((m_uButtonState & UISTATE_FOCUSED) != 0) && (GetFocusedFont() != -1))
			iFont = GetFocusedFont();

		if (m_bShowHtml)
			CRenderEngine::DrawHtmlText(renderContext, rc, sText, clrColor, \
				NULL, NULL, nLinks, iFont, m_uTextStyle);
		else
			DrawTextExBtn(renderContext, rc, sText, clrColor, \
				iFont, m_uTextStyle);
	}

	void CButtonExUI::DrawTextExBtn(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view pstrText, DWORD dwTextColor, int iFont, UINT uStyle)
	{
		CPaintManagerUI* pManager = renderContext.GetManager();
		if (pstrText.empty() || pManager == NULL) return;

		const LONG cx = rc.right - rc.left;
		const LONG cy = rc.bottom - rc.top;
		if (cx <= 0 || cy <= 0) return;

		std::wstring iconGlyph;
		const std::wstring* pDrawText = TryGetButtonExIconGlyph(pstrText, iconGlyph) ? &iconGlyph : nullptr;
		const std::wstring resolvedText = pDrawText != nullptr ? *pDrawText : ButtonExToWideText(pstrText);
		if (resolvedText.empty()) {
			return;
		}

		CRenderEngine::DrawText(renderContext, rc, resolvedText.c_str(), dwTextColor, iFont, uStyle);
	}

	CButtonExUI* CButtonExUI::Clone()
	{
		CButtonExUI* pClone = new CButtonExUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CButtonExUI::CopyData(CButtonExUI* pControl)
	{
		__super::CopyData(pControl);
	}

}


