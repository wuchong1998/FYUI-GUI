#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	class CPaintRenderSurfaceInternalAccess;
	struct CPaintRenderSurfaceState;

	class FYUI_API CPaintRenderSurface
	{
	public:
		CPaintRenderSurface();
		~CPaintRenderSurface();
		CPaintRenderSurface(const CPaintRenderSurface&) = delete;
		CPaintRenderSurface& operator=(const CPaintRenderSurface&) = delete;
		CPaintRenderSurface(CPaintRenderSurface&&) = delete;
		CPaintRenderSurface& operator=(CPaintRenderSurface&&) = delete;

		bool Ensure(CPaintRenderContext& renderContext, LONG cx, LONG cy);
		bool Ensure(CPaintManagerUI* pManager, LONG cx, LONG cy);
		bool ClearTransparent();
		bool DrawGdiplusImage(Gdiplus::Image* image, LONG cx, LONG cy);
		bool PresentLayeredWindow(HWND hWnd, const POINT& ptDst, BYTE alpha = 255) const;
		CPaintRenderContext CreateRenderContext(
			CPaintManagerUI* pManager,
			const RECT& rcPaint,
			RenderBackendType activeBackend = RenderBackendAuto,
			Direct2DRenderMode activeDirect2DMode = Direct2DRenderModeAuto) const;
		HBITMAP DetachBitmap();
		void Reset();

		HBITMAP GetBitmap() const;
		BYTE* GetBits() const;
		COLORREF* GetColorBits() const;
		SIZE GetSize() const;
		bool IsReady() const;

	private:
		friend class CPaintRenderSurfaceInternalAccess;

		CPaintRenderSurfaceState* m_state = nullptr;
	};
}
