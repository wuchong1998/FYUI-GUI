#pragma once

#include "UIRenderTypes.h"

#include <d2d1.h>
#include <cstddef>

namespace FYUI
{
	namespace RenderD2DInternal
	{
		/**
		 * @brief 判断是否具有hCombine
		 * @details 用于判断是否具有hCombine。具体行为由当前对象状态以及传入参数共同决定。
		 * @param seed [in] seed参数
		 * @param value [in] 值参数
		 * @return size_t 返回对应的数值结果
		 */
		size_t HashCombine(size_t seed, size_t value);
		/**
		 * @brief 执行 NormalizeRenderableColor 操作
		 * @details 用于执行 NormalizeRenderableColor 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD NormalizeRenderableColor(DWORD dwColor);
		/**
		 * @brief 设置位图AlphaChannel
		 * @details 用于设置位图AlphaChannel。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pBits [in] Bits对象
		 * @param width [in] 宽度参数
		 * @param height [in] 高度参数
		 * @param alpha [in] alpha参数
		 */
		void SetBitmapAlphaChannel(BYTE* pBits, int width, int height, BYTE alpha);

		/**
		 * @brief 执行 IntValueToPtr 操作
		 * @details 用于执行 IntValueToPtr 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 * @return LPVOID 返回 执行 IntValueToPtr 操作 的结果
		 */
		LPVOID IntValueToPtr(int value);
		/**
		 * @brief 执行 PtrToIntValue 操作
		 * @details 用于执行 PtrToIntValue 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 * @return int 返回对应的数值结果
		 */
		int PtrToIntValue(LPVOID value);
		/**
		 * @brief 执行 DwordValueToPtr 操作
		 * @details 用于执行 DwordValueToPtr 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 * @return LPVOID 返回 执行 DwordValueToPtr 操作 的结果
		 */
		LPVOID DwordValueToPtr(DWORD value);
		/**
		 * @brief 执行 PtrToDwordValue 操作
		 * @details 用于执行 PtrToDwordValue 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD PtrToDwordValue(LPVOID value);

		/**
		 * @brief 执行 ToD2DColor 操作
		 * @details 用于执行 ToD2DColor 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @param normalizeAlpha [in] normalizeAlpha参数
		 * @return D2D1_COLOR_F 返回 执行 ToD2DColor 操作 的结果
		 */
		D2D1_COLOR_F ToD2DColor(DWORD dwColor, bool normalizeAlpha = false);
		/**
		 * @brief 执行 ToD2DRectF 操作
		 * @details 用于执行 ToD2DRectF 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @return 返回对应的几何结果
		 */
		D2D1_RECT_F ToD2DRectF(const RECT& rc);
		/**
		 * @brief 执行 ToInsetD2DRectF 操作
		 * @details 用于执行 ToInsetD2DRectF 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param strokeWidth [in] stroke宽度参数
		 * @return 返回对应的几何结果
		 */
		D2D1_RECT_F ToInsetD2DRectF(const RECT& rc, float strokeWidth);
		/**
		 * @brief 执行 ToD2DDashStyle 操作
		 * @details 用于执行 ToD2DDashStyle 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nStyle [in] 样式数值
		 * @return D2D1_DASH_STYLE 返回 执行 ToD2DDashStyle 操作 的结果
		 */
		D2D1_DASH_STYLE ToD2DDashStyle(int nStyle);
		/**
		 * @brief 执行 ToD2DRenderTargetType 操作
		 * @details 用于执行 ToD2DRenderTargetType 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param mode [in] mode参数
		 * @return D2D1_RENDER_TARGET_TYPE 返回 执行 ToD2DRenderTargetType 操作 的结果
		 */
		D2D1_RENDER_TARGET_TYPE ToD2DRenderTargetType(Direct2DRenderMode mode);

		/**
		 * @brief 判断是否矩形Valid
		 * @details 用于判断是否矩形Valid。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsRectValid(const RECT& rc);
		/**
		 * @brief 判断是否具有可见状态Intersection
		 * @details 用于判断是否具有可见状态Intersection。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcPaint [in] 重绘区域
		 * @param rcDraw [in] 绘制矩形区域
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HasVisibleIntersection(const RECT& rcPaint, const RECT& rcDraw);
	}
}
