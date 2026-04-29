#pragma once

#include "UIRenderD2DCacheTypesInternal.h"

#include <windows.h>
#include <dwrite.h>

#include <string>
#include <vector>

namespace FYUI
{
	struct DirectWriteTextLayoutBoundsInternal
	{
		float maxWidth = 1.0f;
		float maxHeight = 1.0f;
	};

	void NormalizePlainTextForDirectWriteInternal(
		const std::wstring& sourceText,
		UINT uStyle,
		std::wstring& normalizedText,
		std::vector<DWRITE_TEXT_RANGE>& hotkeyRanges);
	DWRITE_TEXT_ALIGNMENT ToDirectWriteTextAlignmentInternal(UINT uStyle);
	DWRITE_PARAGRAPH_ALIGNMENT ToDirectWriteParagraphAlignmentInternal(UINT uStyle);
	LONG ToDirectWriteCacheScale100Internal(float value);
	RenderD2DInternal::D2DTextFormatCacheKey BuildDirectWriteTextFormatCacheKeyInternal(
		const std::wstring& fontFamily,
		float fontSize,
		DWRITE_FONT_WEIGHT fontWeight,
		DWRITE_FONT_STYLE fontStyle,
		UINT uStyle,
		float lineSpacing = 0.0f,
		float baseline = 0.0f);
	bool CanCacheDirectWriteTextLayoutInternal(const std::wstring& sourceText);
	RenderD2DInternal::D2DTextLayoutCacheKey BuildDirectWriteTextLayoutCacheKeyInternal(
		const RenderD2DInternal::D2DTextFormatCacheKey& formatKey,
		const std::wstring& sourceText,
		const DirectWriteTextLayoutBoundsInternal& layoutBounds,
		UINT uStyle,
		bool underline,
		bool strikeout);
	bool CanCachePlainTextNormalizationInternal(const std::wstring& sourceText);
	RenderD2DInternal::PlainTextNormalizationCacheKey BuildPlainTextNormalizationCacheKeyInternal(
		const std::wstring& sourceText,
		UINT uStyle);
	bool CanCacheDirectWriteHtmlTextLayoutInternal(const std::wstring& parsedText, bool hasInlineImages);
	RenderD2DInternal::HtmlTextLayoutCacheKey BuildDirectWriteHtmlTextLayoutCacheKeyInternal(
		const RenderD2DInternal::D2DTextFormatCacheKey& formatKey,
		const std::wstring& parsedText,
		unsigned long long structureSignature,
		const DirectWriteTextLayoutBoundsInternal& layoutBounds,
		UINT uStyle);
	bool CanCacheDirectWriteHtmlParseInternal(const std::wstring& sourceText, bool hasInlineImages);
	RenderD2DInternal::HtmlParseCacheKey BuildDirectWriteHtmlParseCacheKeyInternal(
		UINT_PTR managerId,
		const std::wstring& sourceText,
		const std::wstring& hoverLink,
		DWORD textColor,
		int fontId,
		UINT uStyle);
	RenderD2DInternal::CachedTextLayoutValue BuildCachedDirectWriteTextLayoutValueInternal(
		IDWriteTextLayout* pTextLayout,
		const DWRITE_TEXT_METRICS& metrics,
		bool hasMetrics);
	HRESULT ResolveCachedDirectWriteTextMetricsInternal(
		const RenderD2DInternal::CachedTextLayoutValue& cachedValue,
		DWRITE_TEXT_METRICS* pTextMetrics,
		bool& usedCachedMetrics);
	size_t GetDirectWriteTextFormatCacheLimitInternal();
	size_t GetDirectWriteTextLayoutCacheLimitInternal();
	size_t GetDirectWritePlainTextNormalizationCacheLimitInternal();
	size_t GetDirectWriteHtmlParseCacheLimitInternal();
	size_t GetDirectWriteHtmlTextLayoutCacheLimitInternal();
	size_t GetDirectWriteHtmlHitTestMetricsCacheLimitInternal();
	DirectWriteTextLayoutBoundsInternal ResolveDirectWriteTextLayoutBoundsInternal(
		const RECT& rc,
		UINT uStyle,
		float fontSize,
		float minimumFallbackHeight = 0.0f);
	void ApplyDirectWriteTextMetricsToRectInternal(const DWRITE_TEXT_METRICS& metrics, RECT& rc);
	SIZE ToDirectWriteTextMetricsSizeInternal(const DWRITE_TEXT_METRICS& metrics);
}
