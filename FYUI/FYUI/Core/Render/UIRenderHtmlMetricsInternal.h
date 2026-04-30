#pragma once

#include <windows.h>
#include <dwrite.h>

#include <vector>

namespace FYUI
{
	namespace RenderHtmlMetricsInternal
	{
		/**
		 * @brief 执行 ToHtmlMetricRectInternal 操作
		 * @details 用于执行 ToHtmlMetricRectInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param metric [in] metric参数
		 * @return 返回对应的几何结果
		 */
		RECT ToHtmlMetricRectInternal(const DWRITE_HIT_TEST_METRICS& metric);
		/**
		 * @brief 执行 OffsetHtmlMetricRectInternal 操作
		 * @details 用于执行 OffsetHtmlMetricRectInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcMetric [in] Metric矩形区域
		 * @param rcOffset [in] Offset矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT OffsetHtmlMetricRectInternal(const RECT& rcMetric, const RECT& rcOffset);
		/**
		 * @brief 执行 TryMergeHtmlMetricRectInternal 操作
		 * @details 用于执行 TryMergeHtmlMetricRectInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcMerged [in,out] Merged矩形区域
		 * @param rcNext [in] Next矩形区域
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool TryMergeHtmlMetricRectInternal(RECT& rcMerged, const RECT& rcNext);
		/**
		 * @brief 执行 BuildMergedHtmlMetricRectsInternal 操作
		 * @details 用于执行 BuildMergedHtmlMetricRectsInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pTextLayout [in] 文本布局对象
		 * @param start [in] start参数
		 * @param length [in] length参数
		 * @return 返回对应的几何结果
		 */
		std::vector<RECT> BuildMergedHtmlMetricRectsInternal(IDWriteTextLayout* pTextLayout, UINT32 start, UINT32 length);
	}
}
