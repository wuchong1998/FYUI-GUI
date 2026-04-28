#pragma once

#include <windows.h>

namespace FYUI
{
	namespace RenderD2DFrameMetricsInternal
	{
		struct Direct2DFrameMetricsInternal
		{
			UINT frameBatchFlushCount = 0;
			UINT frameStandaloneDrawCount = 0;
			UINT frameTextLayoutCacheHitCount = 0;
			UINT frameTextLayoutCacheRefreshCount = 0;
			UINT frameTextMetricsCacheHitCount = 0;
			UINT frameTextMetricsCacheRefreshCount = 0;
			UINT frameTextNormalizationCacheHitCount = 0;
			UINT frameTextNormalizationCacheRefreshCount = 0;
			UINT frameHtmlParseCacheHitCount = 0;
			UINT frameHtmlParseCacheRefreshCount = 0;
			UINT frameHtmlLayoutCacheHitCount = 0;
			UINT frameHtmlLayoutCacheRefreshCount = 0;
			UINT frameHtmlMetricsCacheHitCount = 0;
			UINT frameHtmlMetricsCacheRefreshCount = 0;
			UINT frameHtmlHitTestCacheHitCount = 0;
			UINT frameHtmlHitTestCacheRefreshCount = 0;
			UINT frameHtmlDrawingEffectCacheHitCount = 0;
			UINT frameHtmlDrawingEffectCacheRefreshCount = 0;
			UINT frameHtmlDirectWriteParseFailureCount = 0;
			UINT frameHtmlDirectWriteLayoutFailureCount = 0;
			UINT frameHtmlDirectWriteRenderFailureCount = 0;
		};

		void ResetDirect2DFrameMetricsInternal(Direct2DFrameMetricsInternal& metrics);
		void ConsumeDirect2DFrameMetricsInternal(
			Direct2DFrameMetricsInternal& metrics,
			UINT& nDirect2DBatchFlushes,
			UINT& nDirect2DStandaloneDraws,
			UINT& nTextLayoutCacheHits,
			UINT& nTextLayoutCacheRefreshes,
			UINT& nTextMetricsCacheHits,
			UINT& nTextMetricsCacheRefreshes,
			UINT& nTextNormalizationCacheHits,
			UINT& nTextNormalizationCacheRefreshes,
			UINT& nHtmlParseCacheHits,
			UINT& nHtmlParseCacheRefreshes,
			UINT& nHtmlLayoutCacheHits,
			UINT& nHtmlLayoutCacheRefreshes,
			UINT& nHtmlMetricsCacheHits,
			UINT& nHtmlMetricsCacheRefreshes,
			UINT& nHtmlHitTestCacheHits,
			UINT& nHtmlHitTestCacheRefreshes,
			UINT& nHtmlDrawingEffectCacheHits,
			UINT& nHtmlDrawingEffectCacheRefreshes,
			UINT& nHtmlDirectWriteParseFailures,
			UINT& nHtmlDirectWriteLayoutFailures,
			UINT& nHtmlDirectWriteRenderFailures);
	}
}
