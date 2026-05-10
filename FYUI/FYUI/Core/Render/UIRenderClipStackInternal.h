#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	// Internal clip-stack ops used by CRenderClip.
	//
	// The stack itself is thread-local state owned by UIRender.cpp (it lives
	// next to the D2D draw scope because it has to be pushed/popped in sync
	// with the active render target). These three functions are the only
	// out-of-file callers, so UIRenderClip.cpp talks to them through this
	// header.

	void PushD2DRectClipInternal(const RECT& rc);

	// radiusX / radiusY are corner radii (not diameters) — see the 2026-05-09
	// semantic flip in UIRender.cpp.
	void PushD2DRoundClipInternal(const RECT& rc, int radiusX, int radiusY);

	void PopD2DClipInternal();
}
