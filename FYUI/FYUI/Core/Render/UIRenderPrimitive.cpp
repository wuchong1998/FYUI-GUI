#include "pch.h"
#include "../UIRender.h"
#include "UIRenderPrimitiveInternal.h"

namespace FYUI
{
	void CRenderEngine::DrawColor(CPaintRenderContext& renderContext, const RECT& rc, DWORD color)
	{
		DrawColorInternal(renderContext, rc, color);
	}

	void CRenderEngine::DrawRoundColor(CPaintRenderContext& renderContext, const RECT& rc, int radiusX, int radiusY, DWORD color)
	{
		DrawRoundColorInternal(renderContext, rc, radiusX, radiusY, color);
	}

	void CRenderEngine::DrawGradient(CPaintRenderContext& renderContext, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps)
	{
		DrawGradientInternal(renderContext, rc, dwFirst, dwSecond, bVertical, nSteps);
	}

	void CRenderEngine::DrawLine(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle)
	{
		DrawLineInternal(renderContext, rc, nSize, dwPenColor, nStyle);
	}

	void CRenderEngine::DrawRect(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle)
	{
		DrawRectInternal(renderContext, rc, nSize, dwPenColor, nStyle);
	}

	void CRenderEngine::DrawRoundRect(CPaintRenderContext& renderContext, const RECT& rc, int nSize, int radiusX, int radiusY, DWORD dwPenColor, int nStyle)
	{
		DrawRoundRectInternal(renderContext, rc, nSize, radiusX, radiusY, dwPenColor, nStyle);
	}
}
