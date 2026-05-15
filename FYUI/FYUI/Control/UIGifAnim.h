#pragma once

#include <vector>

#include "../Core/Render/UIRenderSurface.h"

namespace FYUI
{
	class FYUI_API CGifAnimUI : public CControlUI
	{
		enum
		{
			EVENT_TIMER_ID = 102,
		};

		DECLARE_DUICONTROL(CGifAnimUI)

	public:
		enum GifHideAction
		{
			GifHideAction_Continue = 0,
			GifHideAction_Pause,
			GifHideAction_Stop,
		};
		enum GifPlaybackState
		{
			GifPlaybackState_Stopped = 0,
			GifPlaybackState_Playing,
			GifPlaybackState_Paused,
			GifPlaybackState_Completed,
		};

		CGifAnimUI();
		~CGifAnimUI() override;

		std::wstring_view GetClass() const override;
		LPVOID GetInterface(std::wstring_view pstrName) override;
		void DoInit();
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		void DoEvent(TEventUI& event);
		void SetVisible(bool bVisible = true, bool bSendFocus = true) override;
		void SetInternVisible(bool bVisible = true) override;
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void SetBkImage(std::wstring_view pStrImage);
		std::wstring_view GetBkImage() const;

		bool SetGifData(const void* pData, DWORD dwSize, bool bInvalidate = true);
		void ClearGifData();

		void SetAutoPlay(bool bIsAuto = true);
		bool IsAutoPlay() const;
		void SetAutoSize(bool bIsAuto = true);
		bool IsAutoSize() const;
		void SetLoop(bool bLoop = true);
		bool IsLoop() const;
		void SetHideAction(GifHideAction action);
		GifHideAction GetHideAction() const;

		void PlayGif();
		void StartGif();
		void PauseGif();
		void StopGif();
		void EndGif();
		void RestartGif();
		bool SeekToGifFrame(UINT nFramePosition, bool bInvalidate = true);
		UINT GetGifFrameCount() const;
		UINT GetCurrentGifFrameIndex() const;
		GifPlaybackState GetGifPlaybackState() const;
		bool IsGifPlaying() const;

		virtual CGifAnimUI* Clone();
		virtual void CopyData(CGifAnimUI* pControl);

	private:
		struct GifFrame
		{
			HBITMAP hBitmap = nullptr;
			UINT delayMs = 100;
			bool hasAlpha = false;
		};

		bool HasGifFrames() const;
		bool HasPlayableGifFrames() const;
		bool HasActiveGifPlaybackState() const;
		bool IsGifPlaybackVisible() const;
		bool CanRunGifPlayback() const;
		bool ShouldAutoPlayGif();
		bool EnsureGifLoaded();
		bool ReloadGifImage();
		bool DecodeGifBytes(const BYTE* pData, DWORD dwSize);
		bool UpdateGifSource(std::wstring_view pStrImage, bool bInvalidate);
		UINT GetFrameDelayMs(UINT nFramePosition) const;
		void ReleaseGifFrames();
		void DeleteGif();
		void KillGifTimer();
		void StopGifPlayback(bool bResetFrame, bool bInvalidate);
		void ResetGifPlaybackState();
		void SyncGifPlaybackState();
		void ScheduleFrameTimer(UINT nFramePosition);
		bool AdvanceGifFrame();
		void OnTimer(UINT_PTR idEvent);
		void NotifyGifPlaybackComplete();
		void DrawFrame(CPaintRenderContext& renderContext);

	private:
		std::wstring m_sBkImage;
		std::vector<BYTE> m_gifData;
		std::vector<GifFrame> m_frames;
		SIZE m_szCanvas = { 0, 0 };
		UINT m_nFramePosition = 0;
		bool m_bUseCustomData = false;
		bool m_bIsAutoPlay = true;
		bool m_bIsAutoSize = false;
		bool m_bLoop = true;
		bool m_bIsPlaying = false;
		bool m_bResumePendingOnVisible = false;
		GifHideAction m_hideAction = GifHideAction_Pause;
		GifPlaybackState m_playbackState = GifPlaybackState_Stopped;
	};
}
