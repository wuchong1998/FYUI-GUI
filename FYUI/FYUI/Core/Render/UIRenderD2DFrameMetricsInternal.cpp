#include "pch.h"
#include "UIRenderD2DFrameMetricsInternal.h"

namespace FYUI
{
	namespace RenderD2DFrameMetricsInternal
	{
		void ResetDirect2DFrameMetricsInternal(Direct2DFrameMetricsInternal& metrics)
		{
			metrics = Direct2DFrameMetricsInternal();
		}

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
			UINT& nHtmlDirectWriteRenderFailures)
		{
			nDirect2DBatchFlushes = metrics.frameBatchFlushCount;
			nDirect2DStandaloneDraws = metrics.frameStandaloneDrawCount;
			nTextLayoutCacheHits = metrics.frameTextLayoutCacheHitCount;
			nTextLayoutCacheRefreshes = metrics.frameTextLayoutCacheRefreshCount;
			nTextMetricsCacheHits = metrics.frameTextMetricsCacheHitCount;
			nTextMetricsCacheRefreshes = metrics.frameTextMetricsCacheRefreshCount;
			nTextNormalizationCacheHits = metrics.frameTextNormalizationCacheHitCount;
			nTextNormalizationCacheRefreshes = metrics.frameTextNormalizationCacheRefreshCount;
			nHtmlParseCacheHits = metrics.frameHtmlParseCacheHitCount;
			nHtmlParseCacheRefreshes = metrics.frameHtmlParseCacheRefreshCount;
			nHtmlLayoutCacheHits = metrics.frameHtmlLayoutCacheHitCount;
			nHtmlLayoutCacheRefreshes = metrics.frameHtmlLayoutCacheRefreshCount;
			nHtmlMetricsCacheHits = metrics.frameHtmlMetricsCacheHitCount;
			nHtmlMetricsCacheRefreshes = metrics.frameHtmlMetricsCacheRefreshCount;
			nHtmlHitTestCacheHits = metrics.frameHtmlHitTestCacheHitCount;
			nHtmlHitTestCacheRefreshes = metrics.frameHtmlHitTestCacheRefreshCount;
			nHtmlDrawingEffectCacheHits = metrics.frameHtmlDrawingEffectCacheHitCount;
			nHtmlDrawingEffectCacheRefreshes = metrics.frameHtmlDrawingEffectCacheRefreshCount;
			nHtmlDirectWriteParseFailures = metrics.frameHtmlDirectWriteParseFailureCount;
			nHtmlDirectWriteLayoutFailures = metrics.frameHtmlDirectWriteLayoutFailureCount;
			nHtmlDirectWriteRenderFailures = metrics.frameHtmlDirectWriteRenderFailureCount;
			ResetDirect2DFrameMetricsInternal(metrics);
		}
	}
}
