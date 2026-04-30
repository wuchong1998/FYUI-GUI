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

		/**
		 * @brief 重置直接2D帧度量信息Internal
		 * @details 用于重置直接2D帧度量信息Internal。具体行为由当前对象状态以及传入参数共同决定。
		 * @param metrics [in,out] 度量信息参数
		 */
		void ResetDirect2DFrameMetricsInternal(Direct2DFrameMetricsInternal& metrics);
		/**
		 * @brief 执行 ConsumeDirect2DFrameMetricsInternal 操作
		 * @details 用于执行 ConsumeDirect2DFrameMetricsInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param metrics [in,out] 度量信息参数
		 * @param nDirect2DBatchFlushes [in,out] 直接2D批处理Flushes数值
		 * @param nDirect2DStandaloneDraws [in,out] 直接2DStandaloneDraws数值
		 * @param nTextLayoutCacheHits [in,out] 文本布局缓存Hits数值
		 * @param nTextLayoutCacheRefreshes [in,out] 文本布局缓存Refreshes数值
		 * @param nTextMetricsCacheHits [in,out] 文本度量信息缓存Hits数值
		 * @param nTextMetricsCacheRefreshes [in,out] 文本度量信息缓存Refreshes数值
		 * @param nTextNormalizationCacheHits [in,out] 文本Normalization缓存Hits数值
		 * @param nTextNormalizationCacheRefreshes [in,out] 文本Normalization缓存Refreshes数值
		 * @param nHtmlParseCacheHits [in,out] HTML 文本解析缓存Hits数值
		 * @param nHtmlParseCacheRefreshes [in,out] HTML 文本解析缓存Refreshes数值
		 * @param nHtmlLayoutCacheHits [in,out] HTML 文本布局缓存Hits数值
		 * @param nHtmlLayoutCacheRefreshes [in,out] HTML 文本布局缓存Refreshes数值
		 * @param nHtmlMetricsCacheHits [in,out] HTML 文本度量信息缓存Hits数值
		 * @param nHtmlMetricsCacheRefreshes [in,out] HTML 文本度量信息缓存Refreshes数值
		 * @param nHtmlHitTestCacheHits [in,out] HTML 文本命中测试缓存Hits数值
		 * @param nHtmlHitTestCacheRefreshes [in,out] HTML 文本命中测试缓存Refreshes数值
		 * @param nHtmlDrawingEffectCacheHits [in,out] HTML 文本DrawingEffect缓存Hits数值
		 * @param nHtmlDrawingEffectCacheRefreshes [in,out] HTML 文本DrawingEffect缓存Refreshes数值
		 * @param nHtmlDirectWriteParseFailures [in,out] HTML 文本直接写入解析Failures数值
		 * @param nHtmlDirectWriteLayoutFailures [in,out] HTML 文本直接写入布局Failures数值
		 * @param nHtmlDirectWriteRenderFailures [in,out] HTML 文本直接写入渲染Failures数值
		 */
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
