#include "pch.h"
#include "UIRenderHtmlMetricsInternal.h"
#include "UIRenderD2DSharedInternal.h"

#include <algorithm>
#include <cmath>

namespace FYUI
{
	namespace RenderHtmlMetricsInternal
	{
		RECT ToHtmlMetricRectInternal(const DWRITE_HIT_TEST_METRICS& metric)
		{
			RECT rcMetric = {
				static_cast<LONG>(std::floor(metric.left)),
				static_cast<LONG>(std::floor(metric.top)),
				static_cast<LONG>(std::ceil(metric.left + metric.width)),
				static_cast<LONG>(std::ceil(metric.top + metric.height))
			};
			return rcMetric;
		}

		RECT OffsetHtmlMetricRectInternal(const RECT& rcMetric, const RECT& rcOffset)
		{
			RECT rcOffsetMetric = {
				rcMetric.left + rcOffset.left,
				rcMetric.top + rcOffset.top,
				rcMetric.right + rcOffset.left,
				rcMetric.bottom + rcOffset.top
			};
			return rcOffsetMetric;
		}

		bool TryMergeHtmlMetricRectInternal(RECT& rcMerged, const RECT& rcNext)
		{
			if (!RenderD2DInternal::IsRectValid(rcMerged) || !RenderD2DInternal::IsRectValid(rcNext)) {
				return false;
			}

			if (rcMerged.top != rcNext.top || rcMerged.bottom != rcNext.bottom) {
				return false;
			}

			if (rcNext.left > rcMerged.right + 1 || rcNext.right < rcMerged.left - 1) {
				return false;
			}

			rcMerged.left = (std::min)(rcMerged.left, rcNext.left);
			rcMerged.right = (std::max)(rcMerged.right, rcNext.right);
			return true;
		}

		std::vector<RECT> BuildMergedHtmlMetricRectsInternal(IDWriteTextLayout* pTextLayout, UINT32 start, UINT32 length)
		{
			std::vector<RECT> rects;
			if (pTextLayout == nullptr || length == 0) {
				return rects;
			}

			UINT32 metricsCount = 0;
			HRESULT hr = pTextLayout->HitTestTextRange(start, length, 0.0f, 0.0f, nullptr, 0, &metricsCount);
			if (hr != E_NOT_SUFFICIENT_BUFFER && FAILED(hr)) {
				return rects;
			}

			if (metricsCount == 0) {
				return rects;
			}

			std::vector<DWRITE_HIT_TEST_METRICS> metrics(metricsCount);
			hr = pTextLayout->HitTestTextRange(start, length, 0.0f, 0.0f, metrics.data(), metricsCount, &metricsCount);
			if (FAILED(hr)) {
				return rects;
			}
			metrics.resize(metricsCount);

			rects.reserve(metrics.size());
			RECT rcMerged = { 0 };
			bool hasMergedRect = false;
			for (const DWRITE_HIT_TEST_METRICS& metric : metrics) {
				const RECT rcMetric = ToHtmlMetricRectInternal(metric);
				if (!RenderD2DInternal::IsRectValid(rcMetric)) {
					continue;
				}

				if (hasMergedRect && TryMergeHtmlMetricRectInternal(rcMerged, rcMetric)) {
					continue;
				}

				if (hasMergedRect) {
					rects.push_back(rcMerged);
				}

				rcMerged = rcMetric;
				hasMergedRect = true;
			}

			if (hasMergedRect) {
				rects.push_back(rcMerged);
			}

			return rects;
		}
	}
}
