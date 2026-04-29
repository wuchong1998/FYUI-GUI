#pragma once

namespace FYUI
{
	class FYUI_API CFrameTestUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CFrameTestUI)

	public:
		CFrameTestUI();

		std::wstring_view GetClass() const override;
		LPVOID GetInterface(std::wstring_view pstrName) override;
		void SetVisible(bool bVisible = true);
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue) override;

		void SetRunning(bool bRunning);
		bool IsRunning() const;
		void SetStatsVisible(bool bVisible);
		bool IsStatsVisible() const;
		double GetCurrentFPS() const;
		double GetAverageFPS() const;
		UINT64 GetTotalFrames() const;

		CFrameTestUI* Clone() override;
		void CopyData(CFrameTestUI* pControl);

	private:
		void ResetFrameStats();
		void UpdateFrameStats();
		void PaintAnimation(CPaintRenderContext& renderContext);
		void PaintStats(CPaintRenderContext& renderContext);
		void RequestNextFrame();
		double QpcToMilliseconds(LONGLONG qpcDelta) const;

	private:
		bool m_bRunning = true;
		bool m_bShowStats = true;
		LONGLONG m_nQpcFrequency = 0;
		LONGLONG m_nStartQpc = 0;
		LONGLONG m_nLastFrameQpc = 0;
		LONGLONG m_nSampleStartQpc = 0;
		UINT64 m_nTotalFrames = 0;
		UINT64 m_nSampleFrames = 0;
		double m_nCurrentFPS = 0.0;
		double m_nAverageFPS = 0.0;
		double m_nLastFrameMs = 0.0;
	};
}
