#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	void DrawColorInternal(CPaintRenderContext& renderContext, const RECT& rc, DWORD color);
	void DrawRoundColorInternal(CPaintRenderContext& renderContext, const RECT& rc, int width, int height, DWORD color);
	void PushD2DRectClipInternal(const RECT& rc);
	void PushD2DRoundClipInternal(const RECT& rc, int width, int height);
	void PopD2DClipInternal();
	void DrawGradientInternal(CPaintRenderContext& renderContext, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps);
	void DrawLineInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID);
	void DrawRectInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID);
	void DrawRoundRectInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle = PS_SOLID);
}
