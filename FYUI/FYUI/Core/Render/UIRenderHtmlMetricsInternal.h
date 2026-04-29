#pragma once

#include <windows.h>
#include <dwrite.h>

#include <vector>

namespace FYUI
{
	namespace RenderHtmlMetricsInternal
	{
		RECT ToHtmlMetricRectInternal(const DWRITE_HIT_TEST_METRICS& metric);
		RECT OffsetHtmlMetricRectInternal(const RECT& rcMetric, const RECT& rcOffset);
		bool TryMergeHtmlMetricRectInternal(RECT& rcMerged, const RECT& rcNext);
		std::vector<RECT> BuildMergedHtmlMetricRectsInternal(IDWriteTextLayout* pTextLayout, UINT32 start, UINT32 length);
	}
}
