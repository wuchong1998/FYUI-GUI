#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	class CPaintRenderSurface;

	bool PrintWindowToRenderSurfaceInternal(CPaintRenderSurface& surface, HWND hWnd, LPARAM printFlags);
	bool BitBltRenderSurfaceInternal(const CPaintRenderSurface& surface, CPaintRenderContext& targetContext, const RECT& rcDest, const POINT& ptSource, DWORD rop);
	bool UpdateLayeredWindowFromRenderSurfaceInternal(const CPaintRenderSurface& surface, HWND hWnd, CPaintRenderContext& targetContext, const POINT& ptDst, const SIZE& size, BYTE alpha);
}
