#include "pch.h"
#include "UIRenderSurface.h"
#include "UIRender.h"
#include "UIRenderImageLegacyInternal.h"
#include "UIRenderSurfaceInternal.h"
#include "UIManager.h"

namespace FYUI
{
	HBITMAP CreateARGB32BitmapInternal(CPaintRenderContext& renderContext, int cx, int cy, BYTE** pBits);

	struct CPaintRenderSurfaceState
	{
		HDC hNativeDC = NULL;
		HBITMAP hBitmap = NULL;
		HBITMAP hOldBitmap = NULL;
		BYTE* pBits = NULL;
		LONG cx = 0;
		LONG cy = 0;

		bool IsReady() const
		{
			return hNativeDC != NULL && hBitmap != NULL && pBits != NULL;
		}
	};

	class CPaintRenderSurfaceInternalAccess
	{
	public:
		static bool EnsureForContext(CPaintRenderSurface& surface, CPaintRenderContext& renderContext, LONG cx, LONG cy);
		static HDC GetNativeDC(const CPaintRenderSurface& surface);
	};

	namespace
	{
		HDC AcquireLayeredWindowPresentNativeDC(HWND hWnd, HDC hFallbackTargetNativeDC, bool& bReleasePresentDC)
		{
			bReleasePresentDC = false;
			HDC hPresentNativeDC = ::GetDC(hWnd);
			if (hPresentNativeDC != NULL) {
				bReleasePresentDC = true;
				return hPresentNativeDC;
			}
			return hFallbackTargetNativeDC;
		}

		void ReleaseLayeredWindowPresentNativeDC(HWND hWnd, HDC hPresentNativeDC, bool bReleasePresentDC)
		{
			if (bReleasePresentDC && hPresentNativeDC != NULL) {
				::ReleaseDC(hWnd, hPresentNativeDC);
			}
		}

		class CScopedLayeredWindowPresentNativeDC
		{
		public:
			CScopedLayeredWindowPresentNativeDC(HWND hWnd, HDC hFallbackTargetNativeDC)
				: m_hWnd(hWnd)
			{
				m_hNativeDC = AcquireLayeredWindowPresentNativeDC(hWnd, hFallbackTargetNativeDC, m_bReleaseDC);
			}

			~CScopedLayeredWindowPresentNativeDC()
			{
				ReleaseLayeredWindowPresentNativeDC(m_hWnd, m_hNativeDC, m_bReleaseDC);
			}

			HDC GetNativeDC() const
			{
				return m_hNativeDC;
			}

		private:
			HWND m_hWnd = NULL;
			HDC m_hNativeDC = NULL;
			bool m_bReleaseDC = false;
		};
	}

	CPaintRenderSurface::CPaintRenderSurface()
		: m_state(new CPaintRenderSurfaceState())
	{
	}

	CPaintRenderSurface::~CPaintRenderSurface()
	{
		Reset();
		delete m_state;
		m_state = nullptr;
	}

	bool CPaintRenderSurface::Ensure(CPaintRenderContext& renderContext, LONG cx, LONG cy)
	{
		return CPaintRenderSurfaceInternalAccess::EnsureForContext(*this, renderContext, cx, cy);
	}

	bool CPaintRenderSurface::Ensure(CPaintManagerUI* pManager, LONG cx, LONG cy)
	{
		if (pManager == NULL) {
			Reset();
			return false;
		}

		if (const CPaintRenderContext* pCurrentRenderContext = pManager->GetCurrentRenderContext()) {
			if (pCurrentRenderContext->GetDC() != NULL) {
				CPaintRenderContext referenceContext = *pCurrentRenderContext;
				return Ensure(referenceContext, cx, cy);
			}
		}

		const RECT rcMeasure = { 0, 0, cx, cy };
		CPaintRenderContext measureContext = pManager->CreateMeasureRenderContext(rcMeasure);
		return Ensure(measureContext, cx, cy);
	}

	bool CPaintRenderSurfaceInternalAccess::EnsureForContext(CPaintRenderSurface& surface, CPaintRenderContext& renderContext, LONG cx, LONG cy)
	{
		HDC hReferenceNativeDC = renderContext.GetDC();
		CPaintRenderSurfaceState* state = surface.m_state;
		if (hReferenceNativeDC == NULL || cx <= 0 || cy <= 0) {
			surface.Reset();
			return false;
		}

		if (state != nullptr && state->IsReady() && state->cx == cx && state->cy == cy) {
			return true;
		}

		surface.Reset();
		state = surface.m_state;
		if (state == nullptr) {
			return false;
		}

		state->hNativeDC = ::CreateCompatibleDC(hReferenceNativeDC);
		if (state->hNativeDC == NULL) {
			return false;
		}

		BYTE* pBits = NULL;
		state->hBitmap = CreateARGB32BitmapInternal(renderContext, cx, cy, &pBits);
		if (state->hBitmap == NULL || pBits == NULL) {
			surface.Reset();
			return false;
		}

		state->hOldBitmap = (HBITMAP)::SelectObject(state->hNativeDC, state->hBitmap);
		if (state->hOldBitmap == NULL) {
			surface.Reset();
			return false;
		}

		state->pBits = pBits;
		state->cx = cx;
		state->cy = cy;
		return true;
	}

	HDC CPaintRenderSurfaceInternalAccess::GetNativeDC(const CPaintRenderSurface& surface)
	{
		return surface.m_state != nullptr ? surface.m_state->hNativeDC : NULL;
	}

	bool PrintWindowToRenderSurfaceInternal(CPaintRenderSurface& surface, HWND hWnd, LPARAM printFlags)
	{
		HDC hSurfaceNativeDC = CPaintRenderSurfaceInternalAccess::GetNativeDC(surface);
		if (hSurfaceNativeDC == NULL || hWnd == NULL) {
			return false;
		}
		::SendMessage(hWnd, WM_PRINT, reinterpret_cast<WPARAM>(hSurfaceNativeDC), printFlags);
		return true;
	}

	bool BitBltRenderSurfaceInternal(const CPaintRenderSurface& surface, CPaintRenderContext& targetContext, const RECT& rcDest, const POINT& ptSource, DWORD rop)
	{
		HDC hTargetNativeDC = targetContext.GetDC();
		HDC hSurfaceNativeDC = CPaintRenderSurfaceInternalAccess::GetNativeDC(surface);
		if (hTargetNativeDC == NULL || hSurfaceNativeDC == NULL) {
			return false;
		}

		return ::BitBlt(
			hTargetNativeDC,
			rcDest.left,
			rcDest.top,
			rcDest.right - rcDest.left,
			rcDest.bottom - rcDest.top,
			hSurfaceNativeDC,
			ptSource.x,
			ptSource.y,
			rop) != FALSE;
	}

	bool UpdateLayeredWindowFromRenderSurfaceInternal(const CPaintRenderSurface& surface, HWND hWnd, CPaintRenderContext& targetContext, const POINT& ptDst, const SIZE& size, BYTE alpha)
	{
		HDC hSurfaceNativeDC = CPaintRenderSurfaceInternalAccess::GetNativeDC(surface);
		if (hSurfaceNativeDC == NULL || hWnd == NULL) {
			return false;
		}

		POINT ptSrc = { 0, 0 };
		POINT ptDstCopy = ptDst;
		SIZE sizeCopy = size;
		BLENDFUNCTION blendPixelFunction = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
		CScopedLayeredWindowPresentNativeDC presentWindowDC(hWnd, targetContext.GetDC());
		return ::UpdateLayeredWindow(
			hWnd,
			presentWindowDC.GetNativeDC(),
			&ptDstCopy,
			&sizeCopy,
			hSurfaceNativeDC,
			&ptSrc,
			0,
			&blendPixelFunction,
			ULW_ALPHA) != FALSE;
	}

	bool CPaintRenderSurface::ClearTransparent()
	{
		if (!IsReady()) {
			return false;
		}
		::ZeroMemory(m_state->pBits, static_cast<size_t>(m_state->cx) * static_cast<size_t>(m_state->cy) * 4);
		return true;
	}

	bool CPaintRenderSurface::DrawGdiplusImage(Gdiplus::Image* image, LONG cx, LONG cy)
	{
		if (!IsReady() || image == NULL || cx <= 0 || cy <= 0) {
			return false;
		}

		ClearTransparent();
		Gdiplus::Graphics graphics(m_state->hNativeDC);
		ConfigureImageFallbackGdiplusGraphicsInternal(graphics);
		graphics.Clear(Gdiplus::Color(0, 0, 0, 0));
		return graphics.DrawImage(image, 0, 0, static_cast<INT>(cx), static_cast<INT>(cy)) == Gdiplus::Ok;
	}

	bool CPaintRenderSurface::PresentLayeredWindow(HWND hWnd, const POINT& ptDst, BYTE alpha) const
	{
		if (!IsReady() || hWnd == NULL) {
			return false;
		}

		POINT ptSrc = { 0, 0 };
		SIZE size = { m_state->cx, m_state->cy };
		BLENDFUNCTION blendPixelFunction = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
		return ::UpdateLayeredWindow(
			hWnd,
			NULL,
			const_cast<POINT*>(&ptDst),
			&size,
			m_state->hNativeDC,
			&ptSrc,
			0,
			&blendPixelFunction,
			ULW_ALPHA) != FALSE;
	}

	CPaintRenderContext CPaintRenderSurface::CreateRenderContext(
		CPaintManagerUI* pManager,
		const RECT& rcPaint,
		RenderBackendType activeBackend,
		Direct2DRenderMode activeDirect2DMode) const
	{
		return CPaintRenderContext(pManager, CPaintRenderSurfaceInternalAccess::GetNativeDC(*this), rcPaint, activeBackend, activeDirect2DMode);
	}

	HBITMAP CPaintRenderSurface::DetachBitmap()
	{
		if (m_state == nullptr) {
			return NULL;
		}

		if (m_state->hNativeDC != NULL && m_state->hOldBitmap != NULL) {
			::SelectObject(m_state->hNativeDC, m_state->hOldBitmap);
		}
		m_state->hOldBitmap = NULL;

		HBITMAP hBitmap = m_state->hBitmap;
		m_state->hBitmap = NULL;
		m_state->pBits = NULL;
		m_state->cx = 0;
		m_state->cy = 0;
		return hBitmap;
	}

	void CPaintRenderSurface::Reset()
	{
		if (m_state == nullptr) {
			return;
		}

		if (m_state->hNativeDC != NULL && m_state->hOldBitmap != NULL) {
			::SelectObject(m_state->hNativeDC, m_state->hOldBitmap);
		}
		m_state->hOldBitmap = NULL;

		if (m_state->hBitmap != NULL) {
			CRenderEngine::FreeBitmap(m_state->hBitmap);
			m_state->hBitmap = NULL;
		}
		if (m_state->hNativeDC != NULL) {
			::DeleteDC(m_state->hNativeDC);
			m_state->hNativeDC = NULL;
		}

		m_state->pBits = NULL;
		m_state->cx = 0;
		m_state->cy = 0;
	}

	HBITMAP CPaintRenderSurface::GetBitmap() const
	{
		return m_state != nullptr ? m_state->hBitmap : NULL;
	}

	BYTE* CPaintRenderSurface::GetBits() const
	{
		return m_state != nullptr ? m_state->pBits : nullptr;
	}

	COLORREF* CPaintRenderSurface::GetColorBits() const
	{
		return reinterpret_cast<COLORREF*>(GetBits());
	}

	SIZE CPaintRenderSurface::GetSize() const
	{
		return m_state != nullptr ? SIZE{ m_state->cx, m_state->cy } : SIZE{ 0, 0 };
	}

	bool CPaintRenderSurface::IsReady() const
	{
		return m_state != nullptr && m_state->IsReady();
	}
}
