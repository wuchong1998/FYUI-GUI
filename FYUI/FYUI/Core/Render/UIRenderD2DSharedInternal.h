#pragma once

#include "UIRenderTypes.h"

#include <d2d1.h>
#include <cstddef>

namespace FYUI
{
	namespace RenderD2DInternal
	{
		size_t HashCombine(size_t seed, size_t value);
		DWORD NormalizeRenderableColor(DWORD dwColor);
		void SetBitmapAlphaChannel(BYTE* pBits, int width, int height, BYTE alpha);

		LPVOID IntValueToPtr(int value);
		int PtrToIntValue(LPVOID value);
		LPVOID DwordValueToPtr(DWORD value);
		DWORD PtrToDwordValue(LPVOID value);

		D2D1_COLOR_F ToD2DColor(DWORD dwColor, bool normalizeAlpha = false);
		D2D1_RECT_F ToD2DRectF(const RECT& rc);
		D2D1_RECT_F ToInsetD2DRectF(const RECT& rc, float strokeWidth);
		D2D1_DASH_STYLE ToD2DDashStyle(int nStyle);
		D2D1_RENDER_TARGET_TYPE ToD2DRenderTargetType(Direct2DRenderMode mode);

		bool IsRectValid(const RECT& rc);
		bool HasVisibleIntersection(const RECT& rcPaint, const RECT& rcDraw);
	}
}
