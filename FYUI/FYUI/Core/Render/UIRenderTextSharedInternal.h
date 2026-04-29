#pragma once

#include "UIRenderContext.h"

#include <string>
#include <string_view>

namespace FYUI
{
	class CPaintManagerUI;
	struct tagTFontInfo;
	typedef tagTFontInfo TFontInfo;

	TFontInfo* ResolveTextFallbackFontInfoInternal(CPaintManagerUI* pManager, int iFont);
	std::wstring ResolveTextFallbackFontFamilyInternal(std::wstring_view fontFamily);
	std::wstring ResolveTextFallbackFontFamilyInternal(const TFontInfo* pFontInfo);
	float ResolveTextFallbackFontSizeInternal(float fontSize);
	DWORD ResolveTextColorInternal(CPaintManagerUI* pManager, DWORD dwColor);
	void UpdateTextFallbackFontMetricsInternal(CPaintRenderContext& renderContext, TFontInfo* pFontInfo);
	std::wstring ToWideText(std::wstring_view text);
	UINT32 GetUtf16CodeUnitLength(const std::wstring& text, size_t index);
	void NormalizePlainTextSharedContent(const std::wstring& sourceText, std::wstring& normalizedText);
	std::wstring BuildGdiplusFallbackTextInternal(std::wstring_view text);
}
