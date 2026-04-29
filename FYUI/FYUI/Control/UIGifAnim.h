#pragma once

#include "../Core/Render/UIRenderSurface.h"

namespace FYUI
{
	class FYUI_API CGifAnimUI : public CControlUI
	{
		enum
		{ 
			EVENT_TIEM_ID = 100,
		};
		DECLARE_DUICONTROL(CGifAnimUI)
	public:
		CGifAnimUI(void);
		~CGifAnimUI(void);

		std::wstring_view	GetClass() const;
		LPVOID	GetInterface(std::wstring_view pstrName);
		void	DoInit();
		bool	DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		void	DoEvent(TEventUI& event);
		void	SetVisible(bool bVisible = true );
		void	SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void	SetBkImage(std::wstring_view pStrImage);
		std::wstring_view GetBkImage();

		void	SetAutoPlay(bool bIsAuto = true );
		bool	IsAutoPlay() const;
		void	SetAutoSize(bool bIsAuto = true );
		bool	IsAutoSize() const;
		void	PlayGif();
		void	PauseGif();
		void	StopGif();

		virtual CGifAnimUI* Clone();
		virtual void CopyData(CGifAnimUI* pControl) ;


	private:
		void	ReleaseGifMetadata();
		bool	EnsureGifImageLoaded();
		bool	HasGifImage() const;
		bool	HasPlayableGifFrames() const;
		bool	ShouldAutoPlayGif();
		bool	CanRunGifPlayback() const;
		bool	HasActiveGifPlaybackState() const;
		UINT	GetFrameDelayMs(UINT nFramePosition) const;
		void	KillGifTimer();
		void	StopGifPlayback(bool bResetFrame, bool bInvalidate);
		void	ResetGifPlaybackState();
		void	SyncGifPlaybackState();
		bool	AttachGifImageInfo(TImageInfo* pImageInfo);
		void	ReleaseGifImageInfo();
		void	ApplyGifReloadResult(TImageInfo* pImageInfo);
		bool	UpdateGifSource(std::wstring_view pStrImage, bool bInvalidate);
		bool	UpdateGifSource(const std::wstring& pStrImage, bool bInvalidate) { return UpdateGifSource(std::wstring_view(pStrImage), bInvalidate); }
		bool	SelectActiveGifFrame() const;
		void	ScheduleFrameTimer(UINT nFramePosition);
		void	AdvanceGifFrame();
		void	ReloadGifImage();
		void	InitGifImage();
		void	DeleteGif();
		void    OnTimer( UINT_PTR idEvent );
		void	InvalidateFrameCache();
		bool	EnsureFrameCache(CPaintRenderContext& renderContext, LONG cx, LONG cy);
		void	DrawFrame(CPaintRenderContext& renderContext);

	private:
		TImageInfo*		m_pGifImageInfo;
		UINT			m_nFrameCount;				// gif鍥剧墖鎬诲抚鏁?
		UINT			m_nFramePosition;			// 褰撳墠鏀惧埌绗嚑甯?
		Gdiplus::PropertyItem*	m_pPropertyItem;	// 甯т笌甯т箣闂撮棿闅旀椂闂?

		SIZE			m_szFrameBitmap;
		UINT			m_nCachedFramePosition;
		bool			m_bFrameCacheValid;
		CPaintRenderSurface m_frameSurface;
		std::wstring		m_sBkImage;
		bool			m_bIsAutoPlay;				// 鏄惁鑷姩鎾斁gif
		bool			m_bIsAutoSize;				// 鏄惁鑷姩鏍规嵁鍥剧墖璁剧疆澶у皬
		bool			m_bIsPlaying;
	};
}



