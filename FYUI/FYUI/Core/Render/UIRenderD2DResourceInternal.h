#pragma once

#include <windows.h>
#include <d2d1.h>
#include <cstddef>

namespace FYUI
{
	namespace RenderD2DResourceInternal
	{
		/**
		 * @brief 创建Solid颜色画刷Internal
		 * @details 用于创建Solid颜色画刷Internal。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRenderTarget [in] 渲染目标对象
		 * @param color [in] 颜色参数
		 * @param ppBrush [in] pp画刷参数
		 * @return HRESULT 返回 创建Solid颜色画刷Internal 的结果
		 */
		HRESULT CreateSolidColorBrushInternal(ID2D1RenderTarget* pRenderTarget, DWORD color, ID2D1SolidColorBrush** ppBrush);
		/**
		 * @brief 创建Stroke样式Internal
		 * @details 用于创建Stroke样式Internal。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pFactory [in] Factory对象
		 * @param nStyle [in] 样式数值
		 * @param ppStrokeStyle [in] ppStroke样式参数
		 * @return HRESULT 返回 创建Stroke样式Internal 的结果
		 */
		HRESULT CreateStrokeStyleInternal(ID2D1Factory* pFactory, int nStyle, ID2D1StrokeStyle** ppStrokeStyle);
		/**
		 * @brief 创建渐变StopCollectionInternal
		 * @details 用于创建渐变StopCollectionInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRenderTarget [in] 渲染目标对象
		 * @param dwFirst [in] First数值
		 * @param dwSecond [in] Second数值
		 * @param ppStopCollection [in] ppStopCollection参数
		 * @return HRESULT 返回 创建渐变StopCollectionInternal 的结果
		 */
		HRESULT CreateGradientStopCollectionInternal(
			ID2D1RenderTarget* pRenderTarget,
			DWORD dwFirst,
			DWORD dwSecond,
			ID2D1GradientStopCollection** ppStopCollection);
		/**
		 * @brief 执行 BuildStrokeStyleCacheKeyInternal 操作
		 * @details 用于执行 BuildStrokeStyleCacheKeyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nStyle [in] 样式数值
		 * @return int 返回对应的数值结果
		 */
		int BuildStrokeStyleCacheKeyInternal(int nStyle);
		/**
		 * @brief 执行 ShouldUseD2DStrokeStyleInternal 操作
		 * @details 用于执行 ShouldUseD2DStrokeStyleInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nStyle [in] 样式数值
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ShouldUseD2DStrokeStyleInternal(int nStyle);
		/**
		 * @brief 执行 BuildGradientStopCollectionCacheKeyInternal 操作
		 * @details 用于执行 BuildGradientStopCollectionCacheKeyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwFirst [in] First数值
		 * @param dwSecond [in] Second数值
		 * @return unsigned long long 返回 执行 BuildGradientStopCollectionCacheKeyInternal 操作 的结果
		 */
		unsigned long long BuildGradientStopCollectionCacheKeyInternal(DWORD dwFirst, DWORD dwSecond);
		/**
		 * @brief 获取D2D位图资源缓存LimitInternal
		 * @details 用于获取D2D位图资源缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @return size_t 返回对应的数值结果
		 */
		size_t GetD2DBitmapResourceCacheLimitInternal();
		/**
		 * @brief 获取D2DSolid颜色画刷缓存LimitInternal
		 * @details 用于获取D2DSolid颜色画刷缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @return size_t 返回对应的数值结果
		 */
		size_t GetD2DSolidColorBrushCacheLimitInternal();
		/**
		 * @brief 获取D2DStroke样式缓存LimitInternal
		 * @details 用于获取D2DStroke样式缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @return size_t 返回对应的数值结果
		 */
		size_t GetD2DStrokeStyleCacheLimitInternal();
		/**
		 * @brief 获取D2D渐变StopCollection缓存LimitInternal
		 * @details 用于获取D2D渐变StopCollection缓存LimitInternal。具体行为由当前对象状态以及传入参数共同决定。
		 * @return size_t 返回对应的数值结果
		 */
		size_t GetD2DGradientStopCollectionCacheLimitInternal();
	}
}
