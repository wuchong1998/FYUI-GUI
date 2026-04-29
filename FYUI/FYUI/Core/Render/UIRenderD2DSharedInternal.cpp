#include "pch.h"
#include "UIRenderD2DSharedInternal.h"

#include <d2d1helper.h>
#include <algorithm>

namespace FYUI
{
	namespace RenderD2DInternal
	{
		size_t HashCombine(size_t seed, size_t value)
		{
			const size_t mixConstant = sizeof(size_t) >= 8
				? static_cast<size_t>(0x9e3779b97f4a7c15ULL)
				: static_cast<size_t>(0x9e3779b9UL);
			return seed ^ (value + mixConstant + (seed << 6) + (seed >> 2));
		}

		DWORD NormalizeRenderableColor(DWORD dwColor)
		{
			if (((dwColor >> 24) & 0xFF) == 0 && (dwColor & 0x00FFFFFF) != 0) {
				return dwColor | 0xFF000000;
			}
			return dwColor;
		}

		void SetBitmapAlphaChannel(BYTE* pBits, int width, int height, BYTE alpha)
		{
			if (pBits == nullptr || width <= 0 || height <= 0) {
				return;
			}

			const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
			BYTE* pPixel = pBits;
			for (size_t i = 0; i < pixelCount; ++i, pPixel += 4) {
				pPixel[3] = alpha;
			}
		}

		LPVOID IntValueToPtr(int value)
		{
			return reinterpret_cast<LPVOID>(static_cast<INT_PTR>(value));
		}

		int PtrToIntValue(LPVOID value)
		{
			return static_cast<int>(reinterpret_cast<INT_PTR>(value));
		}

		LPVOID DwordValueToPtr(DWORD value)
		{
			return reinterpret_cast<LPVOID>(static_cast<UINT_PTR>(value));
		}

		DWORD PtrToDwordValue(LPVOID value)
		{
			return static_cast<DWORD>(reinterpret_cast<UINT_PTR>(value));
		}

		D2D1_COLOR_F ToD2DColor(DWORD dwColor, bool normalizeAlpha)
		{
			if (normalizeAlpha) {
				dwColor = NormalizeRenderableColor(dwColor);
			}
			return D2D1::ColorF(
				static_cast<float>(GetBValue(dwColor)) / 255.0f,
				static_cast<float>(GetGValue(dwColor)) / 255.0f,
				static_cast<float>(GetRValue(dwColor)) / 255.0f,
				static_cast<float>((dwColor >> 24) & 0xFF) / 255.0f);
		}

		D2D1_RECT_F ToD2DRectF(const RECT& rc)
		{
			return D2D1::RectF(static_cast<float>(rc.left), static_cast<float>(rc.top), static_cast<float>(rc.right), static_cast<float>(rc.bottom));
		}

		D2D1_RECT_F ToInsetD2DRectF(const RECT& rc, float strokeWidth)
		{
			const float inset = strokeWidth * 0.5f;
			return D2D1::RectF(
				static_cast<float>(rc.left) + inset,
				static_cast<float>(rc.top) + inset,
				static_cast<float>(rc.right) - inset,
				static_cast<float>(rc.bottom) - inset);
		}

		D2D1_DASH_STYLE ToD2DDashStyle(int nStyle)
		{
			switch (nStyle & PS_STYLE_MASK)
			{
			case PS_DASH:
				return D2D1_DASH_STYLE_DASH;
			case PS_DOT:
				return D2D1_DASH_STYLE_DOT;
			case PS_DASHDOT:
				return D2D1_DASH_STYLE_DASH_DOT;
			case PS_DASHDOTDOT:
				return D2D1_DASH_STYLE_DASH_DOT_DOT;
			default:
				return D2D1_DASH_STYLE_SOLID;
			}
		}

		D2D1_RENDER_TARGET_TYPE ToD2DRenderTargetType(Direct2DRenderMode mode)
		{
			switch (mode)
			{
			case Direct2DRenderModeHardware:
				return D2D1_RENDER_TARGET_TYPE_HARDWARE;
			case Direct2DRenderModeSoftware:
				return D2D1_RENDER_TARGET_TYPE_SOFTWARE;
			default:
				return D2D1_RENDER_TARGET_TYPE_DEFAULT;
			}
		}

		bool IsRectValid(const RECT& rc)
		{
			return rc.right > rc.left && rc.bottom > rc.top;
		}

		bool HasVisibleIntersection(const RECT& rcPaint, const RECT& rcDraw)
		{
			if (!IsRectValid(rcPaint)) {
				return true;
			}

			RECT rcTemp = { 0 };
			return ::IntersectRect(&rcTemp, &rcPaint, &rcDraw) == TRUE;
		}
	}
}
