#include "pch.h"
#include "UIFrameTest.h"
#include "../Core/Render/UIRenderContext.h"

#include <algorithm>
#include <cmath>

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CFrameTestUI)

	namespace
	{
		LONGLONG QueryPerfCounterValue()
		{
			LARGE_INTEGER counter = {};
			::QueryPerformanceCounter(&counter);
			return counter.QuadPart;
		}

		LONGLONG QueryPerfFrequencyValue()
		{
			LARGE_INTEGER frequency = {};
			if (!::QueryPerformanceFrequency(&frequency)) {
				return 0;
			}
			return frequency.QuadPart;
		}

		DWORD BlendChannel(DWORD from, DWORD to, double amount)
		{
			return static_cast<DWORD>(from + (to - from) * amount);
		}

		DWORD BlendColor(DWORD from, DWORD to, double amount)
		{
			const DWORD a = BlendChannel((from >> 24) & 0xFF, (to >> 24) & 0xFF, amount);
			const DWORD r = BlendChannel((from >> 16) & 0xFF, (to >> 16) & 0xFF, amount);
			const DWORD g = BlendChannel((from >> 8) & 0xFF, (to >> 8) & 0xFF, amount);
			const DWORD b = BlendChannel(from & 0xFF, to & 0xFF, amount);
			return (a << 24) | (r << 16) | (g << 8) | b;
		}
	}

	CFrameTestUI::CFrameTestUI()
	{
		m_nQpcFrequency = QueryPerfFrequencyValue();
		SetTextStyle(DT_LEFT | DT_TOP | DT_SINGLELINE);
		SetTextColor(0xFFFFFFFF);
	}

	std::wstring_view CFrameTestUI::GetClass() const
	{
		return _T("FrameTestUI");
	}

	LPVOID CFrameTestUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_FRAMETEST) == 0) {
			return static_cast<CFrameTestUI*>(this);
		}
		return CLabelUI::GetInterface(pstrName);
	}

	void CFrameTestUI::SetVisible(bool bVisible)
	{
		const bool bWasVisible = IsVisible();
		CLabelUI::SetVisible(bVisible);
		if (!bWasVisible && IsVisible()) {
			ResetFrameStats();
			RequestNextFrame();
		}
	}

	bool CFrameTestUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		(void)pStopControl;
		UpdateFrameStats();
		PaintBkColor(renderContext);
		PaintAnimation(renderContext);
		if (m_bShowStats) {
			PaintStats(renderContext);
		}
		PaintBorder(renderContext);
		RequestNextFrame();
		return true;
	}

	void CFrameTestUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("running")) == 0 ||
			StringUtil::CompareNoCase(pstrName, _T("autoplay")) == 0) {
			SetRunning(StringUtil::ParseBool(pstrValue));
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("showstats")) == 0) {
			SetStatsVisible(StringUtil::ParseBool(pstrValue));
		}
		else {
			CLabelUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CFrameTestUI::SetRunning(bool bRunning)
	{
		if (m_bRunning == bRunning) {
			return;
		}
		m_bRunning = bRunning;
		ResetFrameStats();
		if (m_bRunning) {
			RequestNextFrame();
		}
	}

	bool CFrameTestUI::IsRunning() const
	{
		return m_bRunning;
	}

	void CFrameTestUI::SetStatsVisible(bool bVisible)
	{
		if (m_bShowStats == bVisible) {
			return;
		}
		m_bShowStats = bVisible;
		Invalidate();
	}

	bool CFrameTestUI::IsStatsVisible() const
	{
		return m_bShowStats;
	}

	double CFrameTestUI::GetCurrentFPS() const
	{
		return m_nCurrentFPS;
	}

	double CFrameTestUI::GetAverageFPS() const
	{
		return m_nAverageFPS;
	}

	UINT64 CFrameTestUI::GetTotalFrames() const
	{
		return m_nTotalFrames;
	}

	void CFrameTestUI::ResetFrameStats()
	{
		m_nStartQpc = 0;
		m_nLastFrameQpc = 0;
		m_nSampleStartQpc = 0;
		m_nTotalFrames = 0;
		m_nSampleFrames = 0;
		m_nCurrentFPS = 0.0;
		m_nAverageFPS = 0.0;
		m_nLastFrameMs = 0.0;
	}

	void CFrameTestUI::UpdateFrameStats()
	{
		const LONGLONG nowQpc = QueryPerfCounterValue();
		if (m_nStartQpc == 0) {
			m_nStartQpc = nowQpc;
			m_nSampleStartQpc = nowQpc;
		}

		if (m_nLastFrameQpc > 0) {
			m_nLastFrameMs = QpcToMilliseconds(nowQpc - m_nLastFrameQpc);
		}
		m_nLastFrameQpc = nowQpc;

		++m_nTotalFrames;
		++m_nSampleFrames;

		const double sampleMs = QpcToMilliseconds(nowQpc - m_nSampleStartQpc);
		if (sampleMs >= 250.0 && m_nSampleFrames > 0) {
			m_nCurrentFPS = static_cast<double>(m_nSampleFrames) * 1000.0 / sampleMs;
			m_nAverageFPS = (m_nAverageFPS <= 0.0) ? m_nCurrentFPS : (m_nAverageFPS * 0.8 + m_nCurrentFPS * 0.2);
			m_nSampleStartQpc = nowQpc;
			m_nSampleFrames = 0;
		}
	}

	void CFrameTestUI::PaintAnimation(CPaintRenderContext& renderContext)
	{
		const RECT rc = GetPos();
		const LONG width = rc.right - rc.left;
		const LONG height = rc.bottom - rc.top;
		if (width <= 0 || height <= 0) {
			return;
		}

		const double elapsedMs = m_nStartQpc > 0 ? QpcToMilliseconds(m_nLastFrameQpc - m_nStartQpc) : 0.0;
		const double phase = std::fmod(elapsedMs / 1400.0, 1.0);
		const double pulse = (std::sin(elapsedMs / 140.0) + 1.0) * 0.5;

		CRenderEngine::DrawGradient(renderContext, rc, 0xFF111827, 0xFF0F766E, false, 16);

		const LONG railInset = 12;
		RECT rcRail = {
			rc.left + railInset,
			rc.bottom - 32,
			rc.right - railInset,
			rc.bottom - 16
		};
		CRenderEngine::DrawRoundColor(renderContext, rcRail, 14, 14, 0x66000000);

		const LONG railWidth = (std::max<LONG>)(1, rcRail.right - rcRail.left);
		const LONG markerWidth = (std::max<LONG>)(24, railWidth / 5);
		const LONG markerTravel = (std::max<LONG>)(1, railWidth - markerWidth);
		const LONG markerLeft = rcRail.left + static_cast<LONG>(markerTravel * phase);
		RECT rcMarker = {
			markerLeft,
			rcRail.top,
			markerLeft + markerWidth,
			rcRail.bottom
		};
		CRenderEngine::DrawRoundColor(renderContext, rcMarker, 14, 14, BlendColor(0xFFFFC857, 0xFF7DD3FC, pulse));

		const LONG orbSize = (std::min<LONG>)(height / 3, width / 5);
		const LONG cx = rc.left + width / 2 + static_cast<LONG>(std::sin(elapsedMs / 260.0) * width * 0.22);
		const LONG cy = rc.top + height / 2 + static_cast<LONG>(std::cos(elapsedMs / 310.0) * height * 0.18);
		RECT rcOrb = {
			cx - orbSize / 2,
			cy - orbSize / 2,
			cx + orbSize / 2,
			cy + orbSize / 2
		};
		CRenderEngine::DrawRoundColor(renderContext, rcOrb, orbSize, orbSize, BlendColor(0xEE22C55E, 0xEEE879F9, 1.0 - pulse));
	}

	void CFrameTestUI::PaintStats(CPaintRenderContext& renderContext)
	{
		RECT rcText = GetPos();
		rcText.left += 12;
		rcText.top += 10;
		rcText.right -= 12;
		rcText.bottom = rcText.top + 28;

		wchar_t text[160] = { 0 };
		swprintf_s(
			text,
			L"FrameTest %.1f FPS  avg %.1f  frame %.2f ms  total %llu",
			m_nCurrentFPS,
			m_nAverageFPS,
			m_nLastFrameMs,
			static_cast<unsigned long long>(m_nTotalFrames));

		const DWORD textColor = GetTextColor() != 0 ? GetTextColor() : (m_pManager != NULL ? m_pManager->GetDefaultFontColor() : 0xFFFFFFFF);
		CRenderEngine::DrawText(renderContext, rcText, text, textColor, GetFont(), DT_LEFT | DT_TOP | DT_SINGLELINE);
	}

	void CFrameTestUI::RequestNextFrame()
	{
		if (m_bRunning && m_pManager != NULL && IsVisible()) {
			m_pManager->RequestAnimationFrame(this);
		}
	}

	double CFrameTestUI::QpcToMilliseconds(LONGLONG qpcDelta) const
	{
		if (m_nQpcFrequency <= 0) {
			return 0.0;
		}
		return static_cast<double>(qpcDelta) * 1000.0 / static_cast<double>(m_nQpcFrequency);
	}

	CFrameTestUI* CFrameTestUI::Clone()
	{
		CFrameTestUI* pClone = new CFrameTestUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CFrameTestUI::CopyData(CFrameTestUI* pControl)
	{
		CLabelUI::CopyData(pControl);
		m_bRunning = pControl->m_bRunning;
		m_bShowStats = pControl->m_bShowStats;
		ResetFrameStats();
	}
}
