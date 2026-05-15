#include "pch.h"
#include "UIGifAnim.h"

#include "../Core/Render/UIRenderContext.h"
#include "../Core/Render/UIRenderImageCodecInternal.h"
#include "../Core/UIResourceData.h"

namespace FYUI
{
	namespace
	{
		struct GifSourceDescriptor
		{
			std::wstring location;
			std::wstring resourceType;
			DWORD mask = 0;
			bool useResourceLoader = false;
		};

		GifSourceDescriptor ParseGifSourceDescriptor(std::wstring_view source)
		{
			GifSourceDescriptor descriptor;
			bool hasStructuredAttribute = false;
			for (const auto& attribute : StringUtil::ParseQuotedAttributes(source, L'\'')) {
				if (attribute.value.empty()) {
					continue;
				}

				if (StringUtil::EqualsNoCase(attribute.key, L"file")) {
					descriptor.location = attribute.value;
					descriptor.useResourceLoader = false;
					hasStructuredAttribute = true;
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"res")) {
					descriptor.location = attribute.value;
					descriptor.useResourceLoader = true;
					hasStructuredAttribute = true;
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"restype")) {
					descriptor.resourceType = attribute.value;
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"mask")) {
					StringUtil::TryParseColor(attribute.value, descriptor.mask);
				}
			}

			if (!hasStructuredAttribute) {
				descriptor.location = source;
			}
			if (!descriptor.resourceType.empty()) {
				descriptor.useResourceLoader = true;
			}
			return descriptor;
		}

		bool LoadGifBytesFromDescriptor(const GifSourceDescriptor& descriptor, LPBYTE& pData, DWORD& dwSize)
		{
			if (descriptor.location.empty()) {
				pData = nullptr;
				dwSize = 0;
				return false;
			}

			if (descriptor.useResourceLoader) {
				return LoadBinaryResourceData(STRINGorID(descriptor.location.c_str()), pData, dwSize, descriptor.resourceType);
			}
			return LoadBinaryDataFromConfiguredPath(descriptor.location, pData, dwSize) == BinaryDataLoadSuccess;
		}

		bool ParseBoolAttribute(std::wstring_view value)
		{
			return StringUtil::CompareNoCase(value, L"true") == 0 || value == L"1";
		}

		CGifAnimUI::GifHideAction ParseGifHideAction(std::wstring_view value)
		{
			if (StringUtil::CompareNoCase(value, L"continue") == 0) {
				return CGifAnimUI::GifHideAction_Continue;
			}
			if (StringUtil::CompareNoCase(value, L"stop") == 0) {
				return CGifAnimUI::GifHideAction_Stop;
			}
			return CGifAnimUI::GifHideAction_Pause;
		}
	}

	IMPLEMENT_DUICONTROL(CGifAnimUI)

	CGifAnimUI::CGifAnimUI()
	{
	}

	CGifAnimUI::~CGifAnimUI()
	{
		DeleteGif();
	}

	std::wstring_view CGifAnimUI::GetClass() const
	{
		return _T("GifAnimUI");
	}

	LPVOID CGifAnimUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_GIFANIM) == 0) {
			return static_cast<CGifAnimUI*>(this);
		}
		return CControlUI::GetInterface(pstrName);
	}

	void CGifAnimUI::DoInit()
	{
		ReloadGifImage();
	}

	bool CGifAnimUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		(void)pStopControl;
		const RECT& rcPaint = renderContext.GetPaintRect();
		if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) {
			return true;
		}
		if (!EnsureGifLoaded()) {
			return true;
		}
		DrawFrame(renderContext);
		return true;
	}

	void CGifAnimUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER) {
			OnTimer((UINT_PTR)event.wParam);
		}
	}

	void CGifAnimUI::SetVisible(bool bVisible, bool bSendFocus)
	{
		const bool wasVisible = IsVisible();
		CControlUI::SetVisible(bVisible, bSendFocus);
		if (wasVisible == IsVisible()) {
			return;
		}
		SyncGifPlaybackState();
	}

	void CGifAnimUI::SetInternVisible(bool bVisible)
	{
		const bool wasVisible = IsVisible();
		CControlUI::SetInternVisible(bVisible);
		if (wasVisible == IsVisible()) {
			return;
		}
		SyncGifPlaybackState();
	}

	void CGifAnimUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("bkimage")) == 0) {
			SetBkImage(pstrValue);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("autoplay")) == 0) {
			SetAutoPlay(ParseBoolAttribute(pstrValue));
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("autosize")) == 0) {
			SetAutoSize(ParseBoolAttribute(pstrValue));
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("loop")) == 0) {
			SetLoop(ParseBoolAttribute(pstrValue));
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("pauseonhide")) == 0) {
			SetHideAction(ParseBoolAttribute(pstrValue) ? GifHideAction_Pause : GifHideAction_Continue);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("hideaction")) == 0) {
			SetHideAction(ParseGifHideAction(pstrValue));
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CGifAnimUI::SetBkImage(std::wstring_view pStrImage)
	{
		UpdateGifSource(pStrImage, true);
	}

	std::wstring_view CGifAnimUI::GetBkImage() const
	{
		return m_sBkImage;
	}

	bool CGifAnimUI::SetGifData(const void* pData, DWORD dwSize, bool bInvalidate)
	{
		if (pData == nullptr || dwSize == 0) {
			return false;
		}

		m_bUseCustomData = true;
		m_sBkImage.clear();
		m_gifData.assign(reinterpret_cast<const BYTE*>(pData), reinterpret_cast<const BYTE*>(pData) + dwSize);
		const bool bLoaded = ReloadGifImage();
		if (bLoaded && bInvalidate) {
			Invalidate();
		}
		return bLoaded;
	}

	void CGifAnimUI::ClearGifData()
	{
		m_bUseCustomData = false;
		m_sBkImage.clear();
		m_gifData.clear();
		DeleteGif();
		Invalidate();
	}

	void CGifAnimUI::SetAutoPlay(bool bIsAuto)
	{
		if (m_bIsAutoPlay == bIsAuto) {
			return;
		}
		m_bIsAutoPlay = bIsAuto;
		if (!m_bIsAutoPlay) {
			m_bResumePendingOnVisible = false;
		}
		SyncGifPlaybackState();
	}

	bool CGifAnimUI::IsAutoPlay() const
	{
		return m_bIsAutoPlay;
	}

	void CGifAnimUI::SetAutoSize(bool bIsAuto)
	{
		m_bIsAutoSize = bIsAuto;
	}

	bool CGifAnimUI::IsAutoSize() const
	{
		return m_bIsAutoSize;
	}

	void CGifAnimUI::SetLoop(bool bLoop)
	{
		m_bLoop = bLoop;
	}

	bool CGifAnimUI::IsLoop() const
	{
		return m_bLoop;
	}

	void CGifAnimUI::SetHideAction(GifHideAction action)
	{
		if (m_hideAction == action) {
			return;
		}
		m_hideAction = action;
		SyncGifPlaybackState();
	}

	CGifAnimUI::GifHideAction CGifAnimUI::GetHideAction() const
	{
		return m_hideAction;
	}

	void CGifAnimUI::PlayGif()
	{
		if (!EnsureGifLoaded()) {
			return;
		}
		if (m_bIsPlaying || !CanRunGifPlayback()) {
			return;
		}
		if (!m_bLoop && !m_frames.empty() && m_nFramePosition >= m_frames.size() - 1) {
			m_nFramePosition = 0;
		}

		ScheduleFrameTimer(m_nFramePosition);
		m_bIsPlaying = true;
		m_bResumePendingOnVisible = false;
		m_playbackState = GifPlaybackState_Playing;
	}

	void CGifAnimUI::StartGif()
	{
		PlayGif();
	}

	void CGifAnimUI::PauseGif()
	{
		if (!m_bIsPlaying) {
			return;
		}
		m_bResumePendingOnVisible = false;
		StopGifPlayback(false, true);
		m_playbackState = GifPlaybackState_Paused;
	}

	void CGifAnimUI::StopGif()
	{
		if (!HasActiveGifPlaybackState()) {
			return;
		}
		m_bResumePendingOnVisible = false;
		StopGifPlayback(true, true);
		m_playbackState = GifPlaybackState_Stopped;
	}

	void CGifAnimUI::EndGif()
	{
		if (!EnsureGifLoaded() || m_frames.empty()) {
			return;
		}
		m_bResumePendingOnVisible = false;
		StopGifPlayback(false, false);
		m_nFramePosition = static_cast<UINT>(m_frames.size() - 1);
		m_playbackState = GifPlaybackState_Completed;
		Invalidate();
	}

	void CGifAnimUI::RestartGif()
	{
		if (!EnsureGifLoaded()) {
			return;
		}
		StopGifPlayback(true, false);
		m_nFramePosition = 0;
		m_playbackState = GifPlaybackState_Stopped;
		Invalidate();
		PlayGif();
	}

	bool CGifAnimUI::SeekToGifFrame(UINT nFramePosition, bool bInvalidate)
	{
		if (!EnsureGifLoaded()) {
			return false;
		}
		if (nFramePosition >= m_frames.size()) {
			return false;
		}

		m_nFramePosition = nFramePosition;
		if (m_bIsPlaying) {
			KillGifTimer();
			ScheduleFrameTimer(m_nFramePosition);
			m_playbackState = GifPlaybackState_Playing;
		}
		else if (m_playbackState == GifPlaybackState_Completed) {
			m_playbackState = GifPlaybackState_Paused;
		}
		if (bInvalidate) {
			Invalidate();
		}
		return true;
	}

	UINT CGifAnimUI::GetGifFrameCount() const
	{
		return static_cast<UINT>(m_frames.size());
	}

	UINT CGifAnimUI::GetCurrentGifFrameIndex() const
	{
		return m_nFramePosition;
	}

	CGifAnimUI::GifPlaybackState CGifAnimUI::GetGifPlaybackState() const
	{
		return m_playbackState;
	}

	bool CGifAnimUI::IsGifPlaying() const
	{
		return m_bIsPlaying;
	}

	CGifAnimUI* CGifAnimUI::Clone()
	{
		CGifAnimUI* pClone = new CGifAnimUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CGifAnimUI::CopyData(CGifAnimUI* pControl)
	{
		if (pControl == nullptr) {
			return;
		}

		m_bIsAutoPlay = pControl->m_bIsAutoPlay;
		m_bIsAutoSize = pControl->m_bIsAutoSize;
		m_bLoop = pControl->m_bLoop;
		m_hideAction = pControl->m_hideAction;
		m_bUseCustomData = pControl->m_bUseCustomData;
		m_sBkImage = pControl->m_sBkImage;
		m_gifData = pControl->m_gifData;
		__super::CopyData(pControl);
		ReloadGifImage();
	}

	bool CGifAnimUI::HasGifFrames() const
	{
		return !m_frames.empty() && m_nFramePosition < m_frames.size() && m_frames[m_nFramePosition].hBitmap != nullptr;
	}

	bool CGifAnimUI::HasPlayableGifFrames() const
	{
		return m_frames.size() > 1;
	}

	bool CGifAnimUI::HasActiveGifPlaybackState() const
	{
		return m_bIsPlaying || m_nFramePosition != 0 || m_playbackState == GifPlaybackState_Completed || m_playbackState == GifPlaybackState_Paused;
	}

	bool CGifAnimUI::IsGifPlaybackVisible() const
	{
		if (!(m_bVisible && m_bInternVisible)) {
			return false;
		}

		for (CControlUI* pParent = m_pParent; pParent != nullptr; pParent = pParent->GetParent()) {
			if (!pParent->GetVisible()) {
				return false;
			}
		}
		return true;
	}

	bool CGifAnimUI::CanRunGifPlayback() const
	{
		if (m_pManager == nullptr || !HasPlayableGifFrames()) {
			return false;
		}
		if (m_hideAction == GifHideAction_Continue) {
			return true;
		}
		return IsGifPlaybackVisible();
	}

	bool CGifAnimUI::ShouldAutoPlayGif()
	{
		if (!m_bIsAutoPlay) {
			return false;
		}
		if (m_hideAction == GifHideAction_Continue) {
			return true;
		}
		return IsGifPlaybackVisible();
	}

	bool CGifAnimUI::EnsureGifLoaded()
	{
		if (!HasGifFrames()) {
			return ReloadGifImage();
		}
		return true;
	}

	bool CGifAnimUI::ReloadGifImage()
	{
		DeleteGif();

		if (m_bUseCustomData) {
			if (m_gifData.empty()) {
				return false;
			}
			return DecodeGifBytes(m_gifData.data(), static_cast<DWORD>(m_gifData.size()));
		}

		if (m_sBkImage.empty()) {
			return false;
		}

		const GifSourceDescriptor descriptor = ParseGifSourceDescriptor(m_sBkImage);
		if (descriptor.location.empty()) {
			return false;
		}

		LPBYTE pData = nullptr;
		DWORD dwSize = 0;
		if (!LoadGifBytesFromDescriptor(descriptor, pData, dwSize)) {
			return false;
		}

		m_gifData.assign(pData, pData + dwSize);
		delete[] pData;
		return DecodeGifBytes(m_gifData.data(), static_cast<DWORD>(m_gifData.size()));
	}

	bool CGifAnimUI::DecodeGifBytes(const BYTE* pData, DWORD dwSize)
	{
		if (pData == nullptr || dwSize == 0) {
			return false;
		}

		std::vector<GifAnimationFrameBitmapInternal> decodedFrames;
		SIZE canvasSize = { 0, 0 };
		DWORD mask = 0;
		if (!m_bUseCustomData && !m_sBkImage.empty()) {
			mask = ParseGifSourceDescriptor(m_sBkImage).mask;
		}
		if (!DecodeGifAnimationFromMemoryInternal(pData, dwSize, mask, decodedFrames, canvasSize)) {
			return false;
		}

		m_szCanvas = canvasSize;
		m_frames.reserve(decodedFrames.size());
		for (GifAnimationFrameBitmapInternal& frame : decodedFrames) {
			GifFrame gifFrame;
			gifFrame.hBitmap = frame.hBitmap;
			gifFrame.delayMs = frame.delayMs;
			gifFrame.hasAlpha = frame.hasAlpha;
			frame.hBitmap = nullptr;
			m_frames.push_back(gifFrame);
		}
		FreeGifAnimationFramesInternal(decodedFrames);

		m_nFramePosition = 0;
		m_playbackState = GifPlaybackState_Stopped;
		if (m_bIsAutoSize && m_szCanvas.cx > 0 && m_szCanvas.cy > 0) {
			SetFixedWidth(m_szCanvas.cx);
			SetFixedHeight(m_szCanvas.cy);
		}

		SyncGifPlaybackState();
		return HasGifFrames();
	}

	bool CGifAnimUI::UpdateGifSource(std::wstring_view pStrImage, bool bInvalidate)
	{
		if (pStrImage.empty() || m_sBkImage == pStrImage) {
			return false;
		}

		m_bUseCustomData = false;
		m_gifData.clear();
		m_sBkImage = pStrImage;
		const bool bLoaded = ReloadGifImage();
		if (bInvalidate) {
			Invalidate();
		}
		return bLoaded;
	}

	UINT CGifAnimUI::GetFrameDelayMs(UINT nFramePosition) const
	{
		if (nFramePosition >= m_frames.size()) {
			return 100U;
		}

		const UINT delayMs = m_frames[nFramePosition].delayMs;
		return delayMs == 0 ? 100U : delayMs;
	}

	void CGifAnimUI::ReleaseGifFrames()
	{
		for (GifFrame& frame : m_frames) {
			if (frame.hBitmap != nullptr) {
				CRenderEngine::FreeBitmap(frame.hBitmap);
				frame.hBitmap = nullptr;
			}
		}
		m_frames.clear();
		m_szCanvas.cx = 0;
		m_szCanvas.cy = 0;
		m_nFramePosition = 0;
	}

	void CGifAnimUI::DeleteGif()
	{
		ResetGifPlaybackState();
		ReleaseGifFrames();
	}

	void CGifAnimUI::KillGifTimer()
	{
		if (m_pManager != nullptr) {
			m_pManager->KillTimer(this, EVENT_TIMER_ID);
		}
	}

	void CGifAnimUI::StopGifPlayback(bool bResetFrame, bool bInvalidate)
	{
		KillGifTimer();
		m_bIsPlaying = false;
		if (bResetFrame) {
			m_nFramePosition = 0;
		}
		if (bInvalidate) {
			Invalidate();
		}
	}

	void CGifAnimUI::ResetGifPlaybackState()
	{
		m_bResumePendingOnVisible = false;
		StopGifPlayback(true, false);
		m_playbackState = GifPlaybackState_Stopped;
	}

	void CGifAnimUI::SyncGifPlaybackState()
	{
		if (!IsGifPlaybackVisible()) {
			switch (m_hideAction) {
			case GifHideAction_Continue:
				return;
			case GifHideAction_Stop:
				m_bResumePendingOnVisible = false;
				StopGifPlayback(true, false);
				m_playbackState = GifPlaybackState_Stopped;
				return;
			case GifHideAction_Pause:
			default:
				if (m_bIsPlaying) {
					m_bResumePendingOnVisible = true;
					StopGifPlayback(false, true);
					m_playbackState = GifPlaybackState_Paused;
				}
				return;
			}
		}

		if (m_bResumePendingOnVisible) {
			PlayGif();
			return;
		}
		if (ShouldAutoPlayGif()) {
			PlayGif();
			return;
		}
		if (m_bIsPlaying) {
			m_bResumePendingOnVisible = false;
			StopGifPlayback(false, true);
		}
	}

	void CGifAnimUI::ScheduleFrameTimer(UINT nFramePosition)
	{
		if (m_pManager == nullptr) {
			return;
		}
		m_pManager->SetTimer(this, EVENT_TIMER_ID, GetFrameDelayMs(nFramePosition));
	}

	bool CGifAnimUI::AdvanceGifFrame()
	{
		if (m_frames.empty()) {
			return false;
		}

		const UINT lastFrame = static_cast<UINT>(m_frames.size() - 1);
		if (m_nFramePosition >= lastFrame) {
			if (!m_bLoop) {
				return false;
			}
			m_nFramePosition = 0;
			return true;
		}

		++m_nFramePosition;
		return true;
	}

	void CGifAnimUI::OnTimer(UINT_PTR idEvent)
	{
		if (idEvent != EVENT_TIMER_ID) {
			return;
		}

		const bool wasPlaying = m_bIsPlaying;
		KillGifTimer();
		if (!CanRunGifPlayback()) {
			if (wasPlaying && !IsGifPlaybackVisible()) {
				m_bResumePendingOnVisible = true;
			}
			StopGifPlayback(false, false);
			m_playbackState = GifPlaybackState_Paused;
			return;
		}

		if (!AdvanceGifFrame()) {
			StopGifPlayback(false, true);
			m_playbackState = GifPlaybackState_Completed;
			NotifyGifPlaybackComplete();
			return;
		}
		Invalidate();
		ScheduleFrameTimer(m_nFramePosition);
	}

	void CGifAnimUI::NotifyGifPlaybackComplete()
	{
		m_bResumePendingOnVisible = false;
		if (m_pManager != nullptr) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_GIFPLAYCOMPLETE, static_cast<WPARAM>(m_frames.size()), static_cast<LPARAM>(m_nFramePosition));
		}
	}

	void CGifAnimUI::DrawFrame(CPaintRenderContext& renderContext)
	{
		if (!HasGifFrames()) {
			return;
		}

		const GifFrame& frame = m_frames[m_nFramePosition];
		if (frame.hBitmap == nullptr) {
			return;
		}

		RECT rcBitmap = { 0, 0, m_szCanvas.cx, m_szCanvas.cy };
		RECT rcEmptyCorners = { 0, 0, 0, 0 };
		CRenderEngine::DrawImage(renderContext, frame.hBitmap, m_rcItem, rcBitmap, rcEmptyCorners, frame.hasAlpha, 255);
	}
}
