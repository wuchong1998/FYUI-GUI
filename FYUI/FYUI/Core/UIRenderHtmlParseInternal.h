#pragma once

#include <windows.h>

#include <string>
#include <unordered_map>

namespace FYUI
{
	namespace RenderHtmlParseInternal
	{
		enum class HtmlTagType
		{
			Link,
			Bold,
			Color,
			Font,
			Italic,
			Paragraph,
			Selected,
			Underline,
			Raw
		};

		std::wstring TrimWideString(const std::wstring& text);
		std::wstring ToLowerWideString(std::wstring text);
		bool TryParseInteger(const std::wstring& text, int& value);
		bool TryParseHexColor(const std::wstring& text, DWORD& color);
		bool ParseHtmlAttributeMap(const std::wstring& text, std::unordered_map<std::wstring, std::wstring>& attributes);
		bool TryReadHtmlTag(const std::wstring& source, size_t& index, bool& isClosing, HtmlTagType& tagType, std::wstring& body);
		bool TryReadHtmlInlineTagBody(const std::wstring& source, size_t index, wchar_t expectedTagName, std::wstring& body, size_t& nextIndex);
	}
}
