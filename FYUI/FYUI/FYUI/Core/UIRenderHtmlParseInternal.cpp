#include "pch.h"
#include "UIRenderHtmlParseInternal.h"
#include "../Utils/Utils.h"

#include <algorithm>
#include <cwctype>

namespace FYUI
{
	namespace RenderHtmlParseInternal
	{
		std::wstring TrimWideString(const std::wstring& text)
		{
			size_t start = 0;
			while (start < text.length() && std::iswspace(text[start]) != 0) {
				++start;
			}

			size_t end = text.length();
			while (end > start && std::iswspace(text[end - 1]) != 0) {
				--end;
			}

			return text.substr(start, end - start);
		}

		std::wstring ToLowerWideString(std::wstring text)
		{
			std::transform(text.begin(), text.end(), text.begin(), [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });
			return text;
		}

		bool TryParseInteger(const std::wstring& text, int& value)
		{
			const std::wstring trimmed = TrimWideString(text);
			if (trimmed.empty()) {
				return false;
			}

			size_t index = 0;
			if (trimmed[0] == L'-' || trimmed[0] == L'+') {
				index = 1;
			}
			if (index >= trimmed.length()) {
				return false;
			}
			for (; index < trimmed.length(); ++index) {
				if (std::iswdigit(trimmed[index]) == 0) {
					return false;
				}
			}

			return StringUtil::TryParseInt(trimmed, value);
		}

		bool TryParseHexColor(const std::wstring& text, DWORD& color)
		{
			std::wstring trimmed = TrimWideString(text);
			if (!trimmed.empty() && trimmed[0] == L'#') {
				trimmed.erase(trimmed.begin());
			}
			if (trimmed.length() != 6 && trimmed.length() != 8) {
				return false;
			}

			for (wchar_t ch : trimmed) {
				if (!((ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F'))) {
					return false;
				}
			}

			color = std::wcstoul(trimmed.c_str(), nullptr, 16);
			if (trimmed.length() == 6) {
				color |= 0xFF000000;
			}
			return true;
		}

		bool ParseHtmlAttributeMap(const std::wstring& text, std::unordered_map<std::wstring, std::wstring>& attributes)
		{
			size_t index = 0;
			while (index < text.length()) {
				while (index < text.length() && std::iswspace(text[index]) != 0) {
					++index;
				}
				if (index >= text.length()) {
					break;
				}

				const size_t keyStart = index;
				while (index < text.length() && text[index] != L'=' && std::iswspace(text[index]) == 0) {
					++index;
				}
				const std::wstring key = ToLowerWideString(TrimWideString(text.substr(keyStart, index - keyStart)));
				while (index < text.length() && std::iswspace(text[index]) != 0) {
					++index;
				}
				if (key.empty() || index >= text.length() || text[index] != L'=') {
					break;
				}

				++index;
				while (index < text.length() && std::iswspace(text[index]) != 0) {
					++index;
				}
				if (index >= text.length() || (text[index] != L'\'' && text[index] != L'"')) {
					break;
				}

				const wchar_t quote = text[index++];
				const size_t valueStart = index;
				while (index < text.length() && text[index] != quote) {
					++index;
				}
				if (index >= text.length()) {
					break;
				}

				attributes[key] = text.substr(valueStart, index - valueStart);
				++index;
			}

			return !attributes.empty();
		}

		bool TryReadHtmlTag(const std::wstring& source, size_t& index, bool& isClosing, HtmlTagType& tagType, std::wstring& body)
		{
			if (index >= source.length()) {
				return false;
			}
			const wchar_t openDelim = source[index];
			if (openDelim != L'<' && openDelim != L'{') {
				return false;
			}

			const wchar_t closeDelim = openDelim == L'<' ? L'>' : L'}';
			const size_t tagEnd = source.find(closeDelim, index + 1);
			if (tagEnd == std::wstring::npos) {
				return false;
			}

			size_t cursor = index + 1;
			isClosing = false;
			if (cursor < tagEnd && source[cursor] == L'/') {
				isClosing = true;
				++cursor;
			}
			if (cursor >= tagEnd) {
				return false;
			}

			const wchar_t tagName = static_cast<wchar_t>(std::towlower(source[cursor]));
			switch (tagName)
			{
			case L'a':
				tagType = HtmlTagType::Link;
				break;
			case L'b':
				tagType = HtmlTagType::Bold;
				break;
			case L'c':
				tagType = HtmlTagType::Color;
				break;
			case L'f':
				tagType = HtmlTagType::Font;
				break;
			case L'i':
				tagType = HtmlTagType::Italic;
				break;
			case L'p':
				tagType = HtmlTagType::Paragraph;
				break;
			case L's':
				tagType = HtmlTagType::Selected;
				break;
			case L'r':
				tagType = HtmlTagType::Raw;
				break;
			case L'u':
				tagType = HtmlTagType::Underline;
				break;
			default:
				return false;
			}

			++cursor;
			body = TrimWideString(source.substr(cursor, tagEnd - cursor));
			index = tagEnd + 1;
			return true;
		}

		bool TryReadHtmlInlineTagBody(const std::wstring& source, size_t index, wchar_t expectedTagName, std::wstring& body, size_t& nextIndex)
		{
			if (index >= source.length()) {
				return false;
			}

			const wchar_t openDelim = source[index];
			if (openDelim != L'<' && openDelim != L'{') {
				return false;
			}

			const wchar_t closeDelim = openDelim == L'<' ? L'>' : L'}';
			const size_t tagEnd = source.find(closeDelim, index + 1);
			if (tagEnd == std::wstring::npos || index + 1 >= tagEnd) {
				return false;
			}

			if (std::towlower(source[index + 1]) != expectedTagName) {
				return false;
			}

			body = TrimWideString(source.substr(index + 2, tagEnd - (index + 2)));
			nextIndex = tagEnd + 1;
			return true;
		}
	}
}
