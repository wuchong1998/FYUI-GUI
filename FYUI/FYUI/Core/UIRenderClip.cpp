#include "pch.h"
#include "UIRender.h"
#include "UIRenderBatchInternal.h"
#include "UIRenderContext.h"
#include "UIRenderPrimitiveInternal.h"

namespace FYUI
{
	namespace
	{
		bool IsValidClipRect(const RECT& rc)
		{
			return rc.right > rc.left && rc.bottom > rc.top;
		}

		bool ShouldMutateNativeClip(const CPaintRenderContext& renderContext)
		{
			return !renderContext.IsDirect2DBackend();
		}

		void FlushBatchBeforeNativeClipMutation(CPaintRenderContext* pRenderContext)
		{
			if (pRenderContext != nullptr) {
				FlushDirect2DBatchForContextInternal(*pRenderContext);
			}
		}
	}

	struct CRenderClip::ClipState
	{
		CPaintRenderContext* pRenderContext = nullptr;
		HDC hNativeDC = NULL;
		HRGN hClipRgn = NULL;
		HRGN hPreviousClipRgn = NULL;
		bool hasNativeClip = false;
		RECT d2dClipRect = {};
		int d2dRoundWidth = 0;
		int d2dRoundHeight = 0;
		bool hasD2DClip = false;
		bool isD2DRoundClip = false;
		bool isD2DClipPaused = false;
	};

	void CRenderClip::PushD2DClipForState(ClipState& state)
	{
		if (!state.hasD2DClip) {
			return;
		}

		if (state.isD2DRoundClip) {
			PushD2DRoundClipInternal(state.d2dClipRect, state.d2dRoundWidth, state.d2dRoundHeight);
		}
		else {
			PushD2DRectClipInternal(state.d2dClipRect);
		}
		state.isD2DClipPaused = false;
	}

	void CRenderClip::PopD2DClipForState(ClipState& state)
	{
		if (!state.hasD2DClip || state.isD2DClipPaused) {
			return;
		}

		PopD2DClipInternal();
		state.isD2DClipPaused = true;
	}

	CRenderClip::ClipState& CRenderClip::PrepareClipState()
	{
		ResetClipState();
		m_pClipState = new ClipState();
		return *m_pClipState;
	}

	void CRenderClip::ResetClipState()
	{
		if (m_pClipState == nullptr) {
			return;
		}

		if (m_pClipState->hasNativeClip && m_pClipState->hNativeDC != NULL && m_pClipState->hPreviousClipRgn != NULL) {
			FlushBatchBeforeNativeClipMutation(m_pClipState->pRenderContext);
			::SelectClipRgn(m_pClipState->hNativeDC, m_pClipState->hPreviousClipRgn);
		}
		if (m_pClipState->hPreviousClipRgn != NULL) {
			::DeleteObject(m_pClipState->hPreviousClipRgn);
		}
		if (m_pClipState->hClipRgn != NULL) {
			::DeleteObject(m_pClipState->hClipRgn);
		}
		if (m_pClipState->hasD2DClip && !m_pClipState->isD2DClipPaused) {
			PopD2DClipInternal();
		}

		delete m_pClipState;
		m_pClipState = nullptr;
	}

	CRenderClip::~CRenderClip()
	{
		ResetClipState();
	}

	void CRenderClip::GenerateClip(CPaintRenderContext& renderContext, RECT rc, CRenderClip& clip)
	{
		HDC hNativeDC = renderContext.GetDC();
		if (hNativeDC == NULL) {
			clip.ResetClipState();
			return;
		}

		ClipState& state = clip.PrepareClipState();
		state.pRenderContext = &renderContext;
		state.hNativeDC = hNativeDC;
		state.hasNativeClip = ShouldMutateNativeClip(renderContext);
		if (state.hasNativeClip) {
			RECT rcNativeClip = { 0 };
			::GetClipBox(hNativeDC, &rcNativeClip);
			state.hPreviousClipRgn = ::CreateRectRgnIndirect(&rcNativeClip);
			state.hClipRgn = ::CreateRectRgnIndirect(&rc);
			FlushBatchBeforeNativeClipMutation(&renderContext);
			::ExtSelectClipRgn(hNativeDC, state.hClipRgn, RGN_AND);
		}
		state.d2dClipRect = rc;
		state.hasD2DClip = IsValidClipRect(rc);
		state.isD2DRoundClip = false;
		PushD2DClipForState(state);
	}

	void CRenderClip::GenerateRoundClip(CPaintRenderContext& renderContext, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip)
	{
		HDC hNativeDC = renderContext.GetDC();
		if (hNativeDC == NULL) {
			clip.ResetClipState();
			return;
		}

		ClipState& state = clip.PrepareClipState();
		state.pRenderContext = &renderContext;
		const int roundWidth = (std::max)(1, width);
		const int roundHeight = (std::max)(1, height);
		state.hNativeDC = hNativeDC;
		state.hasNativeClip = ShouldMutateNativeClip(renderContext);
		if (state.hasNativeClip) {
			RECT rcNativeClip = { 0 };
			::GetClipBox(hNativeDC, &rcNativeClip);
			state.hPreviousClipRgn = ::CreateRectRgnIndirect(&rcNativeClip);
			state.hClipRgn = ::CreateRectRgnIndirect(&rc);

			HRGN hItemClipRgn = ::CreateRoundRectRgn(rcItem.left, rcItem.top, rcItem.right + 1, rcItem.bottom + 1, roundWidth, roundHeight);
			if (hItemClipRgn != NULL) {
				::CombineRgn(state.hClipRgn, state.hClipRgn, hItemClipRgn, RGN_AND);
				::DeleteObject(hItemClipRgn);
			}

			FlushBatchBeforeNativeClipMutation(&renderContext);
			::ExtSelectClipRgn(hNativeDC, state.hClipRgn, RGN_AND);
		}

		state.d2dClipRect = rcItem;
		state.d2dRoundWidth = roundWidth;
		state.d2dRoundHeight = roundHeight;
		state.hasD2DClip = IsValidClipRect(rcItem) && roundWidth > 0 && roundHeight > 0;
		state.isD2DRoundClip = true;
		PushD2DClipForState(state);
	}

	void CRenderClip::UseOldClipBegin(CPaintRenderContext& renderContext, CRenderClip& clip)
	{
		HDC hNativeDC = renderContext.GetDC();
		if (hNativeDC == NULL || clip.m_pClipState == nullptr) {
			return;
		}

		if (clip.m_pClipState->hasNativeClip && clip.m_pClipState->hPreviousClipRgn != NULL) {
			FlushBatchBeforeNativeClipMutation(&renderContext);
			::SelectClipRgn(hNativeDC, clip.m_pClipState->hPreviousClipRgn);
		}
		PopD2DClipForState(*clip.m_pClipState);
	}

	void CRenderClip::UseOldClipEnd(CPaintRenderContext& renderContext, CRenderClip& clip)
	{
		HDC hNativeDC = renderContext.GetDC();
		if (hNativeDC == NULL || clip.m_pClipState == nullptr) {
			return;
		}

		if (clip.m_pClipState->hasNativeClip && clip.m_pClipState->hClipRgn != NULL) {
			FlushBatchBeforeNativeClipMutation(&renderContext);
			::SelectClipRgn(hNativeDC, clip.m_pClipState->hClipRgn);
		}
		if (clip.m_pClipState->isD2DClipPaused) {
			PushD2DClipForState(*clip.m_pClipState);
		}
	}
}
