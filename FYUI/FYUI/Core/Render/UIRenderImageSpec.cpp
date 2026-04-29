#include "pch.h"
#include "../UIRender.h"

namespace FYUI
{
	namespace
	{
		bool IsImageSpecWhitespace(wchar_t ch)
		{
			return ch > L'\0' && ch <= L' ';
		}

		void SkipImageSpecWhitespace(std::wstring_view text, size_t& cursor)
		{
			while (cursor < text.size() && IsImageSpecWhitespace(text[cursor])) {
				++cursor;
			}
		}

		bool ParseImageSpec(std::wstring_view text, std::vector<std::pair<std::wstring_view, std::wstring_view>>& attributes)
		{
			size_t cursor = 0;
			while (cursor < text.size()) {
				SkipImageSpecWhitespace(text, cursor);
				if (cursor >= text.size()) {
					break;
				}

				const size_t nameStart = cursor;
				while (cursor < text.size() && text[cursor] != L'=' && !IsImageSpecWhitespace(text[cursor])) {
					++cursor;
				}
				const size_t nameEnd = cursor;

				SkipImageSpecWhitespace(text, cursor);
				if (cursor >= text.size() || text[cursor] != L'=') {
					return false;
				}
				++cursor;

				SkipImageSpecWhitespace(text, cursor);
				if (cursor >= text.size() || text[cursor] != L'\'') {
					return false;
				}
				++cursor;

				const size_t valueStart = cursor;
				const size_t valueEnd = text.find(L'\'', cursor);
				if (valueEnd == std::wstring_view::npos) {
					return false;
				}

				attributes.emplace_back(
					StringUtil::TrimView(text.substr(nameStart, nameEnd - nameStart)),
					text.substr(valueStart, valueEnd - valueStart));
				cursor = valueEnd + 1;
			}

			return true;
		}

		bool TryParseByteValue(std::wstring_view text, BYTE& value)
		{
			int intValue = 0;
			if (!StringUtil::TryParseInt(text, intValue) || intValue < 0 || intValue > 255) {
				return false;
			}

			value = static_cast<BYTE>(intValue);
			return true;
		}
	}

	bool MakeImageDest(const RECT& rcControl, const CDuiSize& szImage, const std::wstring& sAlign, const RECT& rcPadding, RECT& rcDest)
	{
		if (StringUtil::Find(sAlign, _T("left")) != -1)
		{
			rcDest.left = rcControl.left;
			rcDest.right = rcDest.left + szImage.cx;
		}
		else if (StringUtil::Find(sAlign, _T("hcenter")) != -1)
		{
			rcDest.left = rcControl.left + ((rcControl.right - rcControl.left) - szImage.cx) / 2;
			rcDest.right = rcDest.left + szImage.cx;
		}
		else if (StringUtil::Find(sAlign, _T("right")) != -1)
		{
			rcDest.left = rcControl.right - szImage.cx;
			rcDest.right = rcDest.left + szImage.cx;
		}

		if (StringUtil::Find(sAlign, _T("top")) != -1)
		{
			rcDest.top = rcControl.top;
			rcDest.bottom = rcDest.top + szImage.cy;
		}
		else if (StringUtil::Find(sAlign, _T("vcenter")) != -1)
		{
			rcDest.top = rcControl.top + ((rcControl.bottom - rcControl.top) - szImage.cy) / 2;
			rcDest.bottom = rcDest.top + szImage.cy;
		}
		else if (StringUtil::Find(sAlign, _T("bottom")) != -1)
		{
			rcDest.top = rcControl.bottom - szImage.cy;
			rcDest.bottom = rcDest.top + rcDest.top;
		}

		::OffsetRect(&rcDest, rcPadding.left, rcPadding.top);
		::OffsetRect(&rcDest, -rcPadding.right, -rcPadding.bottom);

		if (rcDest.right > rcControl.right)
			rcDest.right = rcControl.right;

		if (rcDest.bottom > rcControl.bottom)
			rcDest.bottom = rcControl.bottom;

		return true;
	}

	void GetDrawInfo(TDrawInfo** pDrawInfo, bool bUseRes, bool bUseHSL, DWORD& dwMask, std::wstring& sImageResType, CPaintManagerUI* pManager)
	{
		if ((*pDrawInfo)->sDrawString.empty()) return;

		std::wstring sImageName = (*pDrawInfo)->sDrawString;

		std::vector<std::pair<std::wstring_view, std::wstring_view>> attributes;
		if (!ParseImageSpec((*pDrawInfo)->sDrawString, attributes)) {
			return;
		}

		for (const auto& [item, value] : attributes)
		{
			if (value.empty()) {
				continue;
			}

			if (item == _T("file"))
			{
				sImageName = value;
			}
			else if (item == _T("res"))
			{
				bUseRes = true;
				sImageName = value;
			}
			else if (item == _T("restype"))
			{
				sImageResType = std::wstring(value);
			}
			else if (item == _T("color"))
			{
				bUseRes = true;
				sImageName = value;
			}
			else if (item == _T("dest"))
			{
				StringUtil::TryParseRect(value, (*pDrawInfo)->rcDest);
			}
			else if (item == _T("source"))
			{
				StringUtil::TryParseRect(value, (*pDrawInfo)->rcSource);
			}
			else if (item == _T("corner") || item == _T("scale9"))
			{
				StringUtil::TryParseRect(value, (*pDrawInfo)->rcCorner);
			}
			else if (item == _T("mask"))
			{
				StringUtil::TryParseColor(value, (*pDrawInfo)->dwMask);
			}
			else if (item == _T("fade"))
			{
				int fade = 0;
				if (StringUtil::TryParseInt(value, fade) && fade >= 0) {
					(*pDrawInfo)->uFade = static_cast<UINT>(fade);
				}
			}
			else if (item == _T("hole"))
			{
				(*pDrawInfo)->bHole = StringUtil::ParseBool(value);
			}
			else if (item == _T("xtiled"))
			{
				(*pDrawInfo)->bTiledX = StringUtil::ParseBool(value);
			}
			else if (item == _T("ytiled"))
			{
				(*pDrawInfo)->bTiledY = StringUtil::ParseBool(value);
			}
			else if (item == _T("hsl"))
			{
				bUseHSL = StringUtil::ParseBool(value);
			}
		}
		(*pDrawInfo)->sImageName = sImageName;
		StringUtil::MakeLower(sImageName);
		if (StringUtil::Find(sImageName, L".svg") != -1)
		{
			(*pDrawInfo)->bSvg = true;
		}

		(*pDrawInfo)->rcDest = pManager->ScaleRect((*pDrawInfo)->rcDest);
		(*pDrawInfo)->rcSource = pManager->ScaleRect((*pDrawInfo)->rcSource);
		(*pDrawInfo)->rcCorner = pManager->ScaleRect((*pDrawInfo)->rcCorner);
	}

	void GetDrawInfo2(std::wstring& sDrawString, RECT& rcCorner, RECT& rcDest, CPaintManagerUI* pManager)
	{
		if (sDrawString.empty()) return;
		rcCorner = { 0,0,0,0 };
		rcDest = { 0,0,0,0 };

		std::vector<std::pair<std::wstring_view, std::wstring_view>> attributes;
		if (!ParseImageSpec(sDrawString, attributes)) {
			return;
		}

		for (const auto& [item, value] : attributes)
		{
			if (value.empty()) {
				continue;
			}

			if (item == _T("dest"))
			{
				StringUtil::TryParseRect(value, rcDest);
			}
			else if (item == _T("corner") || item == _T("scale9"))
			{
				StringUtil::TryParseRect(value, rcCorner);
			}
		}

		rcDest = pManager->ScaleRect(rcDest);
		rcCorner = pManager->ScaleRect(rcCorner);
	}

} // namespace DuiLib


