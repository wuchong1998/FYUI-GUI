#pragma once

#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>

namespace FYUI
{
	namespace RenderImageD2DInternal
	{
		/**
		 * @brief 绘制位图SegmentWith直接2DInternal
		 * @details 用于绘制位图SegmentWith直接2DInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRenderTarget [in] 渲染目标对象
		 * @param pBitmap [in] 位图对象
		 * @param rcDest [in] Dest矩形区域
		 * @param rcSource [in] 来源矩形区域
		 * @param rcPaint [in] 重绘区域
		 * @param opacity [in] opacity参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DrawBitmapSegmentWithDirect2DInternal(
			ID2D1RenderTarget* pRenderTarget,
			ID2D1Bitmap* pBitmap,
			const RECT& rcDest,
			const RECT& rcSource,
			const RECT& rcPaint,
			float opacity);

		/**
		 * @brief 绘制Tiled位图SegmentWith直接2DInternal
		 * @details 用于绘制Tiled位图SegmentWith直接2DInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRenderTarget [in] 渲染目标对象
		 * @param pBitmap [in] 位图对象
		 * @param rcDest [in] Dest矩形区域
		 * @param rcSource [in] 来源矩形区域
		 * @param rcPaint [in] 重绘区域
		 * @param opacity [in] opacity参数
		 * @param tileX [in] 平铺X参数
		 * @param tileY [in] 平铺Y参数
		 */
		void DrawTiledBitmapSegmentWithDirect2DInternal(
			ID2D1RenderTarget* pRenderTarget,
			ID2D1Bitmap* pBitmap,
			const RECT& rcDest,
			const RECT& rcSource,
			const RECT& rcPaint,
			float opacity,
			bool tileX,
			bool tileY);

		/**
		 * @brief 获取RotatedBoundsInternal
		 * @details 用于获取RotatedBoundsInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param angleDegrees [in] angleDegrees参数
		 * @return 返回对应的几何结果
		 */
		RECT GetRotatedBoundsInternal(const RECT& rc, float angleDegrees);

		/**
		 * @brief 创建D2D位图FromHBITMAPInternal
		 * @details 用于创建D2D位图FromHBITMAPInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRenderTarget [in] 渲染目标对象
		 * @param pWicFactory [in] WicFactory对象
		 * @param hBitmap [in] h位图参数
		 * @param useAlpha [in] useAlpha参数
		 * @param ppBitmap [in] pp位图参数
		 * @return HRESULT 返回 创建D2D位图FromHBITMAPInternal 的结果
		 */
		HRESULT CreateD2DBitmapFromHBITMAPInternal(
			ID2D1RenderTarget* pRenderTarget,
			IWICImagingFactory* pWicFactory,
			HBITMAP hBitmap,
			bool useAlpha,
			ID2D1Bitmap** ppBitmap);
	}
}
