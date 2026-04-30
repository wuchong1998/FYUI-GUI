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

	/**
	 * @brief 执行 NormalizePlainTextForDirectWriteInternal 操作
	 * @details 用于执行 NormalizePlainTextForDirectWriteInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param sourceText [in] 来源文本参数
	 * @param uStyle [in] 样式标志
	 * @param normalizedText [in,out] normalized文本参数
	 * @param hotkeyRanges [in,out] hotkeyRanges参数
	 */
	void NormalizePlainTextForDirectWriteInternal(
		const std::wstring& sourceText,
		UINT uStyle,
		std::wstring& normalizedText,
		std::vector<DWRITE_TEXT_RANGE>& hotkeyRanges);
	/**
	 * @brief 执行 ToDirectWriteTextAlignmentInternal 操作
	 * @details 用于执行 ToDirectWriteTextAlignmentInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param uStyle [in] 样式标志
	 * @return DWRITE_TEXT_ALIGNMENT 返回 执行 ToDirectWriteTextAlignmentInternal 操作 的结果
	 */
	DWRITE_TEXT_ALIGNMENT ToDirectWriteTextAlignmentInternal(UINT uStyle);
	/**
	 * @brief 执行 ToDirectWriteParagraphAlignmentInternal 操作
	 * @details 用于执行 ToDirectWriteParagraphAlignmentInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param uStyle [in] 样式标志
	 * @return DWRITE_PARAGRAPH_ALIGNMENT 返回 执行 ToDirectWriteParagraphAlignmentInternal 操作 的结果
	 */
	DWRITE_PARAGRAPH_ALIGNMENT ToDirectWriteParagraphAlignmentInternal(UINT uStyle);
	/**
	 * @brief 执行 ToDirectWriteCacheScale100Internal 操作
	 * @details 用于执行 ToDirectWriteCacheScale100Internal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param value [in] 值参数
	 * @return LONG 返回对应的数值结果
	 */
	LONG ToDirectWriteCacheScale100Internal(float value);
	/**
	 * @brief 执行 BuildDirectWriteTextFormatCacheKeyInternal 操作
	 * @details 用于执行 BuildDirectWriteTextFormatCacheKeyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param fontFamily [in] 字体Family参数
	 * @param fontSize [in] 字体尺寸参数
	 * @param fontWeight [in] 字体Weight参数
	 * @param fontStyle [in] 字体样式参数
	 * @param uStyle [in] 样式标志
	 * @param lineSpacing [in] 行Spacing参数
	 * @param baseline [in] baseline参数
	 * @return RenderD2DInternal::D2DTextFormatCacheKey 返回 执行 BuildDirectWriteTextFormatCacheKeyInternal 操作 的结果
	 */
	RenderD2DInternal::D2DTextFormatCacheKey BuildDirectWriteTextFormatCacheKeyInternal(
		const std::wstring& fontFamily,
		float fontSize,
		DWRITE_FONT_WEIGHT fontWeight,
		DWRITE_FONT_STYLE fontStyle,
		UINT uStyle,
		float lineSpacing = 0.0f,
		float baseline = 0.0f);
	/**
	 * @brief 检查是否可以缓存直接写入文本布局Internal
	 * @details 用于检查是否可以缓存直接写入文本布局Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param sourceText [in] 来源文本参数
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool CanCacheDirectWriteTextLayoutInternal(const std::wstring& sourceText);
	/**
	 * @brief 执行 BuildDirectWriteTextLayoutCacheKeyInternal 操作
	 * @details 用于执行 BuildDirectWriteTextLayoutCacheKeyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param formatKey [in] formatKey参数
	 * @param sourceText [in] 来源文本参数
	 * @param layoutBounds [in] 布局Bounds参数
	 * @param uStyle [in] 样式标志
	 * @param underline [in] underline参数
	 * @param strikeout [in] strikeout参数
	 * @return RenderD2DInternal::D2DTextLayoutCacheKey 返回 执行 BuildDirectWriteTextLayoutCacheKeyInternal 操作 的结果
	 */
	RenderD2DInternal::D2DTextLayoutCacheKey BuildDirectWriteTextLayoutCacheKeyInternal(
		const RenderD2DInternal::D2DTextFormatCacheKey& formatKey,
		const std::wstring& sourceText,
		const DirectWriteTextLayoutBoundsInternal& layoutBounds,
		UINT uStyle,
		bool underline,
		bool strikeout);
	/**
	 * @brief 检查是否可以缓存Plain文本NormalizationInternal
	 * @details 用于检查是否可以缓存Plain文本NormalizationInternal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param sourceText [in] 来源文本参数
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool CanCachePlainTextNormalizationInternal(const std::wstring& sourceText);
	/**
	 * @brief 执行 BuildPlainTextNormalizationCacheKeyInternal 操作
	 * @details 用于执行 BuildPlainTextNormalizationCacheKeyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param sourceText [in] 来源文本参数
	 * @param uStyle [in] 样式标志
	 * @return RenderD2DInternal::PlainTextNormalizationCacheKey 返回 执行 BuildPlainTextNormalizationCacheKeyInternal 操作 的结果
	 */
	RenderD2DInternal::PlainTextNormalizationCacheKey BuildPlainTextNormalizationCacheKeyInternal(
		const std::wstring& sourceText,
		UINT uStyle);
	/**
	 * @brief 检查是否可以缓存直接写入HTML 文本文本布局Internal
	 * @details 用于检查是否可以缓存直接写入HTML 文本文本布局Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param parsedText [in] parsed文本参数
	 * @param hasInlineImages [in] 判断是否具有InlineImages参数
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool CanCacheDirectWriteHtmlTextLayoutInternal(const std::wstring& parsedText, bool hasInlineImages);
	/**
	 * @brief 执行 BuildDirectWriteHtmlTextLayoutCacheKeyInternal 操作
	 * @details 用于执行 BuildDirectWriteHtmlTextLayoutCacheKeyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param formatKey [in] formatKey参数
	 * @param parsedText [in] parsed文本参数
	 * @param structureSignature [in] structureSignature参数
	 * @param layoutBounds [in] 布局Bounds参数
	 * @param uStyle [in] 样式标志
	 * @return RenderD2DInternal::HtmlTextLayoutCacheKey 返回 执行 BuildDirectWriteHtmlTextLayoutCacheKeyInternal 操作 的结果
	 */
	RenderD2DInternal::HtmlTextLayoutCacheKey BuildDirectWriteHtmlTextLayoutCacheKeyInternal(
		const RenderD2DInternal::D2DTextFormatCacheKey& formatKey,
		const std::wstring& parsedText,
		unsigned long long structureSignature,
		const DirectWriteTextLayoutBoundsInternal& layoutBounds,
		UINT uStyle);
	/**
	 * @brief 检查是否可以缓存直接写入HTML 文本解析Internal
	 * @details 用于检查是否可以缓存直接写入HTML 文本解析Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param sourceText [in] 来源文本参数
	 * @param hasInlineImages [in] 判断是否具有InlineImages参数
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool CanCacheDirectWriteHtmlParseInternal(const std::wstring& sourceText, bool hasInlineImages);
	/**
	 * @brief 执行 BuildDirectWriteHtmlParseCacheKeyInternal 操作
	 * @details 用于执行 BuildDirectWriteHtmlParseCacheKeyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param managerId [in] 管理器Id参数
	 * @param sourceText [in] 来源文本参数
	 * @param hoverLink [in] hoverLink参数
	 * @param textColor [in] 文本颜色参数
	 * @param fontId [in] 字体Id参数
	 * @param uStyle [in] 样式标志
	 * @return RenderD2DInternal::HtmlParseCacheKey 返回 执行 BuildDirectWriteHtmlParseCacheKeyInternal 操作 的结果
	 */
	RenderD2DInternal::HtmlParseCacheKey BuildDirectWriteHtmlParseCacheKeyInternal(
		UINT_PTR managerId,
		const std::wstring& sourceText,
		const std::wstring& hoverLink,
		DWORD textColor,
		int fontId,
		UINT uStyle);
	/**
	 * @brief 执行 BuildCachedDirectWriteTextLayoutValueInternal 操作
	 * @details 用于执行 BuildCachedDirectWriteTextLayoutValueInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pTextLayout [in] 文本布局对象
	 * @param metrics [in] 度量信息参数
	 * @param hasMetrics [in] 判断是否具有度量信息参数
	 * @return RenderD2DInternal::CachedTextLayoutValue 返回 执行 BuildCachedDirectWriteTextLayoutValueInternal 操作 的结果
	 */
	RenderD2DInternal::CachedTextLayoutValue BuildCachedDirectWriteTextLayoutValueInternal(
		IDWriteTextLayout* pTextLayout,
		const DWRITE_TEXT_METRICS& metrics,
		bool hasMetrics);
	/**
	 * @brief 执行 ResolveCachedDirectWriteTextMetricsInternal 操作
	 * @details 用于执行 ResolveCachedDirectWriteTextMetricsInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param cachedValue [in] cached值参数
	 * @param pTextMetrics [in] 文本度量信息对象
	 * @param usedCachedMetrics [in,out] usedCached度量信息参数
	 * @return HRESULT 返回 执行 ResolveCachedDirectWriteTextMetricsInternal 操作 的结果
	 */
	HRESULT ResolveCachedDirectWriteTextMetricsInternal(
		const RenderD2DInternal::CachedTextLayoutValue& cachedValue,
		DWRITE_TEXT_METRICS* pTextMetrics,
		bool& usedCachedMetrics);
	/**
	 * @brief 获取直接写入文本Format缓存LimitInternal
	 * @details 用于获取直接写入文本Format缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
	 * @return size_t 返回对应的数值结果
	 */
	size_t GetDirectWriteTextFormatCacheLimitInternal();
	/**
	 * @brief 获取直接写入文本布局缓存LimitInternal
	 * @details 用于获取直接写入文本布局缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
	 * @return size_t 返回对应的数值结果
	 */
	size_t GetDirectWriteTextLayoutCacheLimitInternal();
	/**
	 * @brief 获取直接写入Plain文本Normalization缓存LimitInternal
	 * @details 用于获取直接写入Plain文本Normalization缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
	 * @return size_t 返回对应的数值结果
	 */
	size_t GetDirectWritePlainTextNormalizationCacheLimitInternal();
	/**
	 * @brief 获取直接写入HTML 文本解析缓存LimitInternal
	 * @details 用于获取直接写入HTML 文本解析缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
	 * @return size_t 返回对应的数值结果
	 */
	size_t GetDirectWriteHtmlParseCacheLimitInternal();
	/**
	 * @brief 获取直接写入HTML 文本文本布局缓存LimitInternal
	 * @details 用于获取直接写入HTML 文本文本布局缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
	 * @return size_t 返回对应的数值结果
	 */
	size_t GetDirectWriteHtmlTextLayoutCacheLimitInternal();
	/**
	 * @brief 获取直接写入HTML 文本命中测试度量信息缓存LimitInternal
	 * @details 用于获取直接写入HTML 文本命中测试度量信息缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
	 * @return size_t 返回对应的数值结果
	 */
	size_t GetDirectWriteHtmlHitTestMetricsCacheLimitInternal();
	/**
	 * @brief 执行 ResolveDirectWriteTextLayoutBoundsInternal 操作
	 * @details 用于执行 ResolveDirectWriteTextLayoutBoundsInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param rc [in] 矩形区域
	 * @param uStyle [in] 样式标志
	 * @param fontSize [in] 字体尺寸参数
	 * @param minimumFallbackHeight [in] minimumFallback高度参数
	 * @return DirectWriteTextLayoutBoundsInternal 返回 执行 ResolveDirectWriteTextLayoutBoundsInternal 操作 的结果
	 */
	DirectWriteTextLayoutBoundsInternal ResolveDirectWriteTextLayoutBoundsInternal(
		const RECT& rc,
		UINT uStyle,
		float fontSize,
		float minimumFallbackHeight = 0.0f);
	/**
	 * @brief 应用直接写入文本度量信息To矩形Internal
	 * @details 用于应用直接写入文本度量信息To矩形Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param metrics [in] 度量信息参数
	 * @param rc [in,out] 矩形区域
	 */
	void ApplyDirectWriteTextMetricsToRectInternal(const DWRITE_TEXT_METRICS& metrics, RECT& rc);
	/**
	 * @brief 执行 ToDirectWriteTextMetricsSizeInternal 操作
	 * @details 用于执行 ToDirectWriteTextMetricsSizeInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param metrics [in] 度量信息参数
	 * @return 返回对应的几何结果
	 */
	SIZE ToDirectWriteTextMetricsSizeInternal(const DWRITE_TEXT_METRICS& metrics);
}
