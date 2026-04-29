#pragma once

#include <windows.h>
#include <d2d1.h>
#include <cstddef>

namespace FYUI
{
	namespace RenderD2DResourceInternal
	{
		HRESULT CreateSolidColorBrushInternal(ID2D1RenderTarget* pRenderTarget, DWORD color, ID2D1SolidColorBrush** ppBrush);
		HRESULT CreateStrokeStyleInternal(ID2D1Factory* pFactory, int nStyle, ID2D1StrokeStyle** ppStrokeStyle);
		HRESULT CreateGradientStopCollectionInternal(
			ID2D1RenderTarget* pRenderTarget,
			DWORD dwFirst,
			DWORD dwSecond,
			ID2D1GradientStopCollection** ppStopCollection);
		int BuildStrokeStyleCacheKeyInternal(int nStyle);
		bool ShouldUseD2DStrokeStyleInternal(int nStyle);
		unsigned long long BuildGradientStopCollectionCacheKeyInternal(DWORD dwFirst, DWORD dwSecond);
		size_t GetD2DBitmapResourceCacheLimitInternal();
		size_t GetD2DSolidColorBrushCacheLimitInternal();
		size_t GetD2DStrokeStyleCacheLimitInternal();
		size_t GetD2DGradientStopCollectionCacheLimitInternal();
	}
}
