#include "pch.h"
#include "UIRenderDirectWriteTextInternal.h"
#include "UIRenderTextSharedInternal.h"

#include <algorithm>
#include <cmath>

namespace FYUI
{
	void NormalizePlainTextForDirectWriteInternal(
		const std::wstring& sourceText,
		UINT uStyle,
		std::wstring& normalizedText,
		std::vector<DWRITE_TEXT_RANGE>& hotkeyRanges)
	{
		NormalizePlainTextSharedContent(sourceText, normalizedText);

		hotkeyRanges.clear();
		if ((uStyle & DT_NOPREFIX) != 0 || normalizedText.empty()) {
			return;
		}

		std::wstring displayText;
		displayText.reserve(normalizedText.length());
		bool underlineNext = false;
		for (size_t i = 0; i < normalizedText.length();) {
			if (normalizedText[i] == L'&') {
				if (i + 1 < normalizedText.length() && normalizedText[i + 1] == L'&') {
					displayText.push_back(L'&');
					i += 2;
					underlineNext = false;
					continue;
				}

				underlineNext = true;
				++i;
				continue;
			}

			const UINT32 runLength = GetUtf16CodeUnitLength(normalizedText, i);
			const UINT32 rangeStart = static_cast<UINT32>(displayText.length());
			displayText.append(normalizedText, i, runLength);
			if (underlineNext && normalizedText[i] != L'\r' && normalizedText[i] != L'\n') {
				DWRITE_TEXT_RANGE range = { rangeStart, runLength };
				hotkeyRanges.push_back(range);
			}
			underlineNext = false;
			i += runLength;
		}

		normalizedText.swap(displayText);
	}

	DWRITE_TEXT_ALIGNMENT ToDirectWriteTextAlignmentInternal(UINT uStyle)
	{
		if ((uStyle & DT_CENTER) != 0) {
			return DWRITE_TEXT_ALIGNMENT_CENTER;
		}
		if ((uStyle & DT_RIGHT) != 0) {
			return DWRITE_TEXT_ALIGNMENT_TRAILING;
		}
		return DWRITE_TEXT_ALIGNMENT_LEADING;
	}

	DWRITE_PARAGRAPH_ALIGNMENT ToDirectWriteParagraphAlignmentInternal(UINT uStyle)
	{
		if ((uStyle & DT_VCENTER) != 0) {
			return DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
		}
		if ((uStyle & DT_BOTTOM) != 0) {
			return DWRITE_PARAGRAPH_ALIGNMENT_FAR;
		}
		return DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
	}

	LONG ToDirectWriteCacheScale100Internal(float value)
	{
		return static_cast<LONG>(std::lround(value * 100.0f));
	}

	RenderD2DInternal::D2DTextFormatCacheKey BuildDirectWriteTextFormatCacheKeyInternal(
		const std::wstring& fontFamily,
		float fontSize,
		DWRITE_FONT_WEIGHT fontWeight,
		DWRITE_FONT_STYLE fontStyle,
		UINT uStyle,
		float lineSpacing,
		float baseline)
	{
		RenderD2DInternal::D2DTextFormatCacheKey key;
		key.fontFamily = ResolveTextFallbackFontFamilyInternal(fontFamily);
		key.fontSize100 = ToDirectWriteCacheScale100Internal(ResolveTextFallbackFontSizeInternal(fontSize));
		key.lineSpacing100 = ToDirectWriteCacheScale100Internal(lineSpacing > 0.0f ? lineSpacing : 0.0f);
		key.baseline100 = ToDirectWriteCacheScale100Internal(baseline > 0.0f ? baseline : 0.0f);
		key.fontWeight = static_cast<UINT>(fontWeight);
		key.fontStyle = static_cast<UINT>(fontStyle);
		key.textAlignment = static_cast<UINT>(ToDirectWriteTextAlignmentInternal(uStyle));
		key.paragraphAlignment = static_cast<UINT>(ToDirectWriteParagraphAlignmentInternal(uStyle));
		key.wordWrapping = static_cast<UINT>((uStyle & DT_SINGLELINE) != 0 ? DWRITE_WORD_WRAPPING_NO_WRAP : DWRITE_WORD_WRAPPING_WRAP);
		return key;
	}

	bool CanCacheDirectWriteTextLayoutInternal(const std::wstring& sourceText)
	{
		return sourceText.length() <= 512;
	}

	RenderD2DInternal::D2DTextLayoutCacheKey BuildDirectWriteTextLayoutCacheKeyInternal(
		const RenderD2DInternal::D2DTextFormatCacheKey& formatKey,
		const std::wstring& sourceText,
		const DirectWriteTextLayoutBoundsInternal& layoutBounds,
		UINT uStyle,
		bool underline,
		bool strikeout)
	{
		RenderD2DInternal::D2DTextLayoutCacheKey key;
		key.formatKey = formatKey;
		key.text = sourceText;
		key.maxWidth100 = ToDirectWriteCacheScale100Internal(layoutBounds.maxWidth);
		key.maxHeight100 = ToDirectWriteCacheScale100Internal(layoutBounds.maxHeight);
		key.endEllipsis = (uStyle & DT_END_ELLIPSIS) != 0;
		key.underline = underline;
		key.strikeout = strikeout;
		key.noPrefix = (uStyle & DT_NOPREFIX) != 0;
		return key;
	}

	bool CanCachePlainTextNormalizationInternal(const std::wstring& sourceText)
	{
		return sourceText.length() <= 1024;
	}

	RenderD2DInternal::PlainTextNormalizationCacheKey BuildPlainTextNormalizationCacheKeyInternal(
		const std::wstring& sourceText,
		UINT uStyle)
	{
		RenderD2DInternal::PlainTextNormalizationCacheKey key;
		key.text = sourceText;
		key.noPrefix = (uStyle & DT_NOPREFIX) != 0;
		return key;
	}

	bool CanCacheDirectWriteHtmlTextLayoutInternal(const std::wstring& parsedText, bool hasInlineImages)
	{
		return !hasInlineImages && parsedText.length() <= 2048;
	}

	RenderD2DInternal::HtmlTextLayoutCacheKey BuildDirectWriteHtmlTextLayoutCacheKeyInternal(
		const RenderD2DInternal::D2DTextFormatCacheKey& formatKey,
		const std::wstring& parsedText,
		unsigned long long structureSignature,
		const DirectWriteTextLayoutBoundsInternal& layoutBounds,
		UINT uStyle)
	{
		RenderD2DInternal::HtmlTextLayoutCacheKey key;
		key.formatKey = formatKey;
		key.text = parsedText;
		key.structureSignature = structureSignature;
		key.maxWidth100 = ToDirectWriteCacheScale100Internal(layoutBounds.maxWidth);
		key.maxHeight100 = ToDirectWriteCacheScale100Internal(layoutBounds.maxHeight);
		key.endEllipsis = (uStyle & DT_END_ELLIPSIS) != 0;
		return key;
	}

	bool CanCacheDirectWriteHtmlParseInternal(const std::wstring& sourceText, bool hasInlineImages)
	{
		return !hasInlineImages && sourceText.length() <= 1024;
	}

	RenderD2DInternal::HtmlParseCacheKey BuildDirectWriteHtmlParseCacheKeyInternal(
		UINT_PTR managerId,
		const std::wstring& sourceText,
		const std::wstring& hoverLink,
		DWORD textColor,
		int fontId,
		UINT uStyle)
	{
		RenderD2DInternal::HtmlParseCacheKey key;
		key.managerId = managerId;
		key.text = sourceText;
		key.hoverLink = hoverLink;
		key.textColor = textColor;
		key.fontId = fontId;
		key.uStyle = uStyle;
		return key;
	}

	RenderD2DInternal::CachedTextLayoutValue BuildCachedDirectWriteTextLayoutValueInternal(
		IDWriteTextLayout* pTextLayout,
		const DWRITE_TEXT_METRICS& metrics,
		bool hasMetrics)
	{
		RenderD2DInternal::CachedTextLayoutValue cachedValue;
		cachedValue.layout = pTextLayout;
		cachedValue.metrics = metrics;
		cachedValue.hasMetrics = hasMetrics;
		return cachedValue;
	}

	HRESULT ResolveCachedDirectWriteTextMetricsInternal(
		const RenderD2DInternal::CachedTextLayoutValue& cachedValue,
		DWRITE_TEXT_METRICS* pTextMetrics,
		bool& usedCachedMetrics)
	{
		usedCachedMetrics = false;
		if (pTextMetrics == nullptr) {
			return S_OK;
		}

		if (cachedValue.hasMetrics) {
			*pTextMetrics = cachedValue.metrics;
			usedCachedMetrics = true;
			return S_OK;
		}

		if (!cachedValue.layout) {
			ZeroMemory(pTextMetrics, sizeof(DWRITE_TEXT_METRICS));
			return E_FAIL;
		}

		ZeroMemory(pTextMetrics, sizeof(DWRITE_TEXT_METRICS));
		return cachedValue.layout->GetMetrics(pTextMetrics);
	}

	size_t GetDirectWriteTextFormatCacheLimitInternal()
	{
		return 64;
	}

	size_t GetDirectWriteTextLayoutCacheLimitInternal()
	{
		return 256;
	}

	size_t GetDirectWritePlainTextNormalizationCacheLimitInternal()
	{
		return 256;
	}

	size_t GetDirectWriteHtmlParseCacheLimitInternal()
	{
		return 64;
	}

	size_t GetDirectWriteHtmlTextLayoutCacheLimitInternal()
	{
		return 128;
	}

	size_t GetDirectWriteHtmlHitTestMetricsCacheLimitInternal()
	{
		return 256;
	}

	DirectWriteTextLayoutBoundsInternal ResolveDirectWriteTextLayoutBoundsInternal(
		const RECT& rc,
		UINT uStyle,
		float fontSize,
		float minimumFallbackHeight)
	{
		const bool calculateRect = (uStyle & DT_CALCRECT) != 0;
		DirectWriteTextLayoutBoundsInternal bounds;
		bounds.maxWidth = static_cast<float>(rc.right - rc.left);
		bounds.maxHeight = static_cast<float>(rc.bottom - rc.top);

		if (bounds.maxWidth <= 0.0f) {
			bounds.maxWidth = calculateRect ? 4096.0f : 1.0f;
		}
		if (bounds.maxHeight <= 0.0f) {
			float defaultHeight = fontSize > 0.0f ? fontSize * 2.0f : 32.0f;
			if (minimumFallbackHeight > 0.0f) {
				defaultHeight = (std::max)(defaultHeight, minimumFallbackHeight);
			}
			bounds.maxHeight = calculateRect ? 4096.0f : defaultHeight;
		}

		return bounds;
	}

	void ApplyDirectWriteTextMetricsToRectInternal(const DWRITE_TEXT_METRICS& metrics, RECT& rc)
	{
		rc.right = rc.left + static_cast<LONG>(std::ceil(metrics.widthIncludingTrailingWhitespace));
		rc.bottom = rc.top + static_cast<LONG>(std::ceil(metrics.height));
	}

	SIZE ToDirectWriteTextMetricsSizeInternal(const DWRITE_TEXT_METRICS& metrics)
	{
		SIZE size = {};
		size.cx = static_cast<LONG>(std::ceil(metrics.widthIncludingTrailingWhitespace));
		size.cy = static_cast<LONG>(std::ceil(metrics.height));
		return size;
	}
}
