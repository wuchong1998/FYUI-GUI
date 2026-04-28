#pragma once

#include "UIRenderD2DSharedInternal.h"

#include <dwrite.h>
#include <wrl/client.h>
#include <functional>
#include <string>
#include <vector>

namespace FYUI
{
	namespace RenderD2DInternal
	{
		struct D2DBitmapCacheKey
		{
			HBITMAP bitmap = NULL;
			bool useAlpha = false;

			bool operator==(const D2DBitmapCacheKey& other) const
			{
				return bitmap == other.bitmap && useAlpha == other.useAlpha;
			}
		};

		struct D2DBitmapCacheKeyHasher
		{
			size_t operator()(const D2DBitmapCacheKey& key) const
			{
				const UINT_PTR handleValue = reinterpret_cast<UINT_PTR>(key.bitmap);
				return std::hash<UINT_PTR>()(handleValue ^ (key.useAlpha ? static_cast<UINT_PTR>(0x9E3779B9u) : 0u));
			}
		};

		struct D2DTextFormatCacheKey
		{
			std::wstring fontFamily;
			LONG fontSize100 = 0;
			LONG lineSpacing100 = 0;
			LONG baseline100 = 0;
			UINT fontWeight = 0;
			UINT fontStyle = 0;
			UINT textAlignment = 0;
			UINT paragraphAlignment = 0;
			UINT wordWrapping = 0;

			bool operator==(const D2DTextFormatCacheKey& other) const
			{
				return fontFamily == other.fontFamily
					&& fontSize100 == other.fontSize100
					&& lineSpacing100 == other.lineSpacing100
					&& baseline100 == other.baseline100
					&& fontWeight == other.fontWeight
					&& fontStyle == other.fontStyle
					&& textAlignment == other.textAlignment
					&& paragraphAlignment == other.paragraphAlignment
					&& wordWrapping == other.wordWrapping;
			}
		};

		struct D2DTextFormatCacheKeyHasher
		{
			size_t operator()(const D2DTextFormatCacheKey& key) const
			{
				size_t hash = std::hash<std::wstring>()(key.fontFamily);
				hash = HashCombine(hash, std::hash<LONG>()(key.fontSize100));
				hash = HashCombine(hash, std::hash<LONG>()(key.lineSpacing100));
				hash = HashCombine(hash, std::hash<LONG>()(key.baseline100));
				hash = HashCombine(hash, std::hash<UINT>()(key.fontWeight));
				hash = HashCombine(hash, std::hash<UINT>()(key.fontStyle));
				hash = HashCombine(hash, std::hash<UINT>()(key.textAlignment));
				hash = HashCombine(hash, std::hash<UINT>()(key.paragraphAlignment));
				hash = HashCombine(hash, std::hash<UINT>()(key.wordWrapping));
				return hash;
			}
		};

		struct D2DTextLayoutCacheKey
		{
			D2DTextFormatCacheKey formatKey;
			std::wstring text;
			LONG maxWidth100 = 0;
			LONG maxHeight100 = 0;
			bool endEllipsis = false;
			bool underline = false;
			bool strikeout = false;
			bool noPrefix = false;

			bool operator==(const D2DTextLayoutCacheKey& other) const
			{
				return formatKey == other.formatKey
					&& text == other.text
					&& maxWidth100 == other.maxWidth100
					&& maxHeight100 == other.maxHeight100
					&& endEllipsis == other.endEllipsis
					&& underline == other.underline
					&& strikeout == other.strikeout
					&& noPrefix == other.noPrefix;
			}
		};

		struct D2DTextLayoutCacheKeyHasher
		{
			size_t operator()(const D2DTextLayoutCacheKey& key) const
			{
				size_t hash = D2DTextFormatCacheKeyHasher()(key.formatKey);
				hash = HashCombine(hash, std::hash<std::wstring>()(key.text));
				hash = HashCombine(hash, std::hash<LONG>()(key.maxWidth100));
				hash = HashCombine(hash, std::hash<LONG>()(key.maxHeight100));
				hash = HashCombine(hash, std::hash<bool>()(key.endEllipsis));
				hash = HashCombine(hash, std::hash<bool>()(key.underline));
				hash = HashCombine(hash, std::hash<bool>()(key.strikeout));
				hash = HashCombine(hash, std::hash<bool>()(key.noPrefix));
				return hash;
			}
		};

		struct PlainTextNormalizationCacheKey
		{
			std::wstring text;
			bool noPrefix = false;

			bool operator==(const PlainTextNormalizationCacheKey& other) const
			{
				return text == other.text && noPrefix == other.noPrefix;
			}
		};

		struct PlainTextNormalizationCacheKeyHasher
		{
			size_t operator()(const PlainTextNormalizationCacheKey& key) const
			{
				size_t hash = std::hash<std::wstring>()(key.text);
				hash = HashCombine(hash, std::hash<bool>()(key.noPrefix));
				return hash;
			}
		};

		struct PlainTextNormalizationCacheValue
		{
			std::wstring normalizedText;
			std::vector<DWRITE_TEXT_RANGE> hotkeyRanges;
		};

		struct HtmlParseCacheKey
		{
			UINT_PTR managerId = 0;
			std::wstring text;
			std::wstring hoverLink;
			DWORD textColor = 0;
			int fontId = 0;
			UINT uStyle = 0;

			bool operator==(const HtmlParseCacheKey& other) const
			{
				return managerId == other.managerId
					&& text == other.text
					&& hoverLink == other.hoverLink
					&& textColor == other.textColor
					&& fontId == other.fontId
					&& uStyle == other.uStyle;
			}
		};

		struct HtmlParseCacheKeyHasher
		{
			size_t operator()(const HtmlParseCacheKey& key) const
			{
				size_t hash = std::hash<UINT_PTR>()(key.managerId);
				hash = HashCombine(hash, std::hash<std::wstring>()(key.text));
				hash = HashCombine(hash, std::hash<std::wstring>()(key.hoverLink));
				hash = HashCombine(hash, std::hash<DWORD>()(key.textColor));
				hash = HashCombine(hash, std::hash<int>()(key.fontId));
				hash = HashCombine(hash, std::hash<UINT>()(key.uStyle));
				return hash;
			}
		};

		struct HtmlTextLayoutCacheKey
		{
			D2DTextFormatCacheKey formatKey;
			std::wstring text;
			unsigned long long structureSignature = 0;
			LONG maxWidth100 = 0;
			LONG maxHeight100 = 0;
			bool endEllipsis = false;

			bool operator==(const HtmlTextLayoutCacheKey& other) const
			{
				return formatKey == other.formatKey
					&& text == other.text
					&& structureSignature == other.structureSignature
					&& maxWidth100 == other.maxWidth100
					&& maxHeight100 == other.maxHeight100
					&& endEllipsis == other.endEllipsis;
			}
		};

		struct HtmlTextLayoutCacheKeyHasher
		{
			size_t operator()(const HtmlTextLayoutCacheKey& key) const
			{
				size_t hash = D2DTextFormatCacheKeyHasher()(key.formatKey);
				hash = HashCombine(hash, std::hash<std::wstring>()(key.text));
				hash = HashCombine(hash, std::hash<unsigned long long>()(key.structureSignature));
				hash = HashCombine(hash, std::hash<LONG>()(key.maxWidth100));
				hash = HashCombine(hash, std::hash<LONG>()(key.maxHeight100));
				hash = HashCombine(hash, std::hash<bool>()(key.endEllipsis));
				return hash;
			}
		};

		struct HtmlDrawingEffectState
		{
			unsigned long long signature = 0;
			bool initialized = false;
			bool hasCustomForeground = false;
		};

		struct HtmlHitTestMetricsCacheKey
		{
			UINT_PTR layoutId = 0;
			UINT32 start = 0;
			UINT32 length = 0;

			bool operator==(const HtmlHitTestMetricsCacheKey& other) const
			{
				return layoutId == other.layoutId
					&& start == other.start
					&& length == other.length;
			}
		};

		struct HtmlHitTestMetricsCacheKeyHasher
		{
			size_t operator()(const HtmlHitTestMetricsCacheKey& key) const
			{
				size_t hash = std::hash<UINT_PTR>()(key.layoutId);
				hash = HashCombine(hash, std::hash<UINT32>()(key.start));
				hash = HashCombine(hash, std::hash<UINT32>()(key.length));
				return hash;
			}
		};

		struct CachedTextLayoutValue
		{
			Microsoft::WRL::ComPtr<IDWriteTextLayout> layout;
			DWRITE_TEXT_METRICS metrics = {};
			bool hasMetrics = false;
		};
	}
}
