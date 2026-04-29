#include "pch.h"
#include "../UIManager.h"
#include "UIRenderD2DSharedInternal.h"
#include "UIRenderTextSharedInternal.h"

namespace FYUI
{
	namespace
	{
		constexpr std::wstring_view kDefaultTextFallbackFontFamily = L"Microsoft YaHei";
	}

	TFontInfo* ResolveTextFallbackFontInfoInternal(CPaintManagerUI* pManager, int iFont)
	{
		if (pManager == nullptr) {
			return nullptr;
		}

		TFontInfo* pFontInfo = pManager->GetFontInfo(iFont);
		if (pFontInfo == nullptr) {
			pFontInfo = pManager->GetDefaultFontInfo();
		}
		return pFontInfo;
	}

	std::wstring ResolveTextFallbackFontFamilyInternal(std::wstring_view fontFamily)
	{
		return fontFamily.empty() ? std::wstring(kDefaultTextFallbackFontFamily) : ToWideText(fontFamily);
	}

	std::wstring ResolveTextFallbackFontFamilyInternal(const TFontInfo* pFontInfo)
	{
		if (pFontInfo == nullptr) {
			return std::wstring(kDefaultTextFallbackFontFamily);
		}
		return ResolveTextFallbackFontFamilyInternal(std::wstring_view(pFontInfo->sFontName));
	}

	float ResolveTextFallbackFontSizeInternal(float fontSize)
	{
		return fontSize > 0.0f ? fontSize : 12.0f;
	}

	DWORD ResolveTextColorInternal(CPaintManagerUI* pManager, DWORD dwColor)
	{
		if (dwColor == 0) {
			if (pManager != nullptr) {
				dwColor = pManager->GetDefaultFontColor();
			}
			if (dwColor == 0) {
				dwColor = 0xFF000000;
			}
		}
		return RenderD2DInternal::NormalizeRenderableColor(dwColor);
	}

	void UpdateTextFallbackFontMetricsInternal(CPaintRenderContext& renderContext, TFontInfo* pFontInfo)
	{
		HDC hMetricsNativeDC = renderContext.GetDC();
		if (hMetricsNativeDC == NULL || pFontInfo == nullptr || pFontInfo->hFont == NULL) {
			return;
		}

		HFONT hPreviousFont = (HFONT)::SelectObject(hMetricsNativeDC, pFontInfo->hFont);
		::GetTextMetrics(hMetricsNativeDC, &pFontInfo->tm);
		::SelectObject(hMetricsNativeDC, hPreviousFont);
	}

	namespace
	{
		bool IsHexDigit(wchar_t ch)
		{
			return (ch >= L'0' && ch <= L'9')
				|| (ch >= L'a' && ch <= L'f')
				|| (ch >= L'A' && ch <= L'F');
		}

		bool IsSimpleWhitespace(wchar_t ch)
		{
			return ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n';
		}

		void ReplaceAllText(std::wstring& text, const wchar_t* from, const wchar_t* to)
		{
			if (from == nullptr || to == nullptr) {
				return;
			}

			const size_t fromLen = wcslen(from);
			const size_t toLen = wcslen(to);
			size_t pos = 0;
			while ((pos = text.find(from, pos)) != std::wstring::npos) {
				text.replace(pos, fromLen, to);
				pos += toLen;
			}
		}

		bool TryDecodeSingleFontIconEntity(const std::wstring& text, std::wstring& glyphText)
		{
			size_t start = 0;
			while (start < text.length() && IsSimpleWhitespace(text[start])) {
				++start;
			}

			size_t end = text.length();
			while (end > start && IsSimpleWhitespace(text[end - 1])) {
				--end;
			}

			if (end <= start || (end - start) < 4 || text[start] != L'&' || text[start + 1] != L'#' || (text[start + 2] != L'x' && text[start + 2] != L'X')) {
				return false;
			}

			size_t hexBegin = start + 3;
			size_t hexEnd = hexBegin;
			while (hexEnd < end && IsHexDigit(text[hexEnd])) {
				++hexEnd;
			}
			if (hexEnd == hexBegin) {
				return false;
			}

			size_t pos = hexEnd;
			if (pos < end && text[pos] == L';') {
				++pos;
			}
			while (pos < end && IsSimpleWhitespace(text[pos])) {
				++pos;
			}
			if (pos != end) {
				return false;
			}

			const std::wstring hexValue = text.substr(hexBegin, hexEnd - hexBegin);
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

	std::wstring ToWideText(std::wstring_view text)
	{
		return std::wstring(text);
	}

	UINT32 GetUtf16CodeUnitLength(const std::wstring& text, size_t index)
	{
		if (index + 1 < text.length()
			&& text[index] >= 0xD800 && text[index] <= 0xDBFF
			&& text[index + 1] >= 0xDC00 && text[index + 1] <= 0xDFFF) {
			return 2;
		}
		return 1;
	}

	void NormalizePlainTextSharedContent(const std::wstring& sourceText, std::wstring& normalizedText)
	{
		normalizedText.clear();
		normalizedText.reserve(sourceText.length());
		for (size_t i = 0; i < sourceText.length(); ++i) {
			if (sourceText[i] == L'\r') {
				if (i + 1 < sourceText.length() && sourceText[i + 1] == L'\n') {
					++i;
				}
				normalizedText.push_back(L'\n');
				continue;
			}
			normalizedText.push_back(sourceText[i]);
		}

		ReplaceAllText(normalizedText, L"&#10;", L"\n");
		ReplaceAllText(normalizedText, L"&#x0a;", L"\n");
		ReplaceAllText(normalizedText, L"&#x0A;", L"\n");

		std::wstring escapedNormalized;
		escapedNormalized.reserve(normalizedText.length());
		for (size_t i = 0; i < normalizedText.length(); ++i) {
			if (normalizedText[i] == L'\\' && i + 1 < normalizedText.length() && normalizedText[i + 1] == L'n') {
				escapedNormalized.push_back(L'\n');
				++i;
				continue;
			}
			escapedNormalized.push_back(normalizedText[i]);
		}
		normalizedText.swap(escapedNormalized);

		std::wstring iconGlyph;
		if (TryDecodeSingleFontIconEntity(normalizedText, iconGlyph)) {
			normalizedText.swap(iconGlyph);
		}
	}

	std::wstring BuildGdiplusFallbackTextInternal(std::wstring_view text)
	{
		std::wstring drawText = ToWideText(text);
		std::wstring normalizedText;
		NormalizePlainTextSharedContent(drawText, normalizedText);
		return normalizedText;
	}
}
