#pragma once

#include <windows.h>

#include "UIRenderTypes.h"

namespace FYUI
{
	class CPaintManagerUI;

	class FYUI_API CPaintRenderContext
	{
	public:
		CPaintRenderContext(
			CPaintManagerUI* pManager,
			HDC hDC,
			const RECT& rcPaint,
			RenderBackendType activeBackend = RenderBackendAuto,
			Direct2DRenderMode activeDirect2DMode = Direct2DRenderModeAuto)
			: m_pManager(pManager),
			m_hDC(hDC),
			m_rcPaint(rcPaint),
			m_activeBackend(activeBackend),
			m_activeDirect2DMode(activeDirect2DMode)
		{
		}

		CPaintManagerUI* GetManager() const
		{
			return m_pManager;
		}

		HDC GetDC() const
		{
			return m_hDC;
		}

		const RECT& GetPaintRect() const
		{
			return m_rcPaint;
		}

		RenderBackendType GetActiveBackend() const
		{
			return m_activeBackend;
		}

		Direct2DRenderMode GetActiveDirect2DRenderMode() const
		{
			return m_activeDirect2DMode;
		}

		bool IsDirect2DBackend() const
		{
			return m_activeBackend == RenderBackendDirect2D;
		}

	private:
		CPaintManagerUI* m_pManager = nullptr;
		HDC m_hDC = NULL;
		RECT m_rcPaint = { 0, 0, 0, 0 };
		RenderBackendType m_activeBackend = RenderBackendAuto;
		Direct2DRenderMode m_activeDirect2DMode = Direct2DRenderModeAuto;
	};

}
