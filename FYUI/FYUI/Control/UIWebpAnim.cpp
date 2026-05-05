#include "pch.h"
#include "UIWebpAnim.h"

#include "../Core/Render/UIRenderContext.h"
#include "../Core/Render/UIRenderImageCodecInternal.h"
#include "../Core/UIResourceData.h"

namespace FYUI
{
	namespace
	{
		struct WebpSourceDescriptor
		{
			std::wstring location;
			std::wstring resourceType;
			DWORD mask = 0;
			bool useResourceLoader = false;
		};

		WebpSourceDescriptor ParseWebpSourceDescriptor(std::wstring_view source)
		{
			WebpSourceDescriptor descriptor;
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

		bool LoadWebpBytesFromDescriptor(const WebpSourceDescriptor& descriptor, LPBYTE& pData, DWORD& dwSize)
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

		CWebpAnimUI::WebpHideAction ParseWebpHideAction(std::wstring_view value)
		{
			if (StringUtil::CompareNoCase(value, L"continue") == 0) {
				return CWebpAnimUI::WebpHideAction_Continue;
			}
			if (StringUtil::CompareNoCase(value, L"stop") == 0) {
				return CWebpAnimUI::WebpHideAction_Stop;
			}
			return CWebpAnimUI::WebpHideAction_Pause;
		}
	}

	IMPLEMENT_DUICONTROL(CWebpAnimUI)

	CWebpAnimUI::CWebpAnimUI()
	{
	}

	CWebpAnimUI::~CWebpAnimUI()
	{
		DeleteWebp();
	}

	std::wstring_view CWebpAnimUI::GetClass() const
	{
		return _T("WebpAnimUI");
	}

	LPVOID CWebpAnimUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_WEBPANIM) == 0) {
			return static_cast<CWebpAnimUI*>(this);
		}
		return CControlUI::GetInterface(pstrName);
	}

	void CWebpAnimUI::DoInit()
	{
		ReloadWebpImage();
	}

	bool CWebpAnimUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		(void)pStopControl;
		const RECT& rcPaint = renderContext.GetPaintRect();
		if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) {
			return true;
		}
		if (!EnsureWebpLoaded()) {
			return true;
		}
		DrawFrame(renderContext);
		return true;
	}

	void CWebpAnimUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER) {
			OnTimer((UINT_PTR)event.wParam);
		}
	}

	void CWebpAnimUI::SetVisible(bool bVisible, bool bSendFocus)
	{
		const bool wasVisible = IsVisible();
		CControlUI::SetVisible(bVisible, bSendFocus);
		if (wasVisible == IsVisible()) {
			return;
		}
		SyncWebpPlaybackState();
	}

	void CWebpAnimUI::SetInternVisible(bool bVisible)
	{
		const bool wasVisible = IsVisible();
		CControlUI::SetInternVisible(bVisible);
		if (wasVisible == IsVisible()) {
			return;
		}
		SyncWebpPlaybackState();
	}

	void CWebpAnimUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
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
			SetHideAction(ParseBoolAttribute(pstrValue) ? WebpHideAction_Pause : WebpHideAction_Continue);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("hideaction")) == 0) {
			SetHideAction(ParseWebpHideAction(pstrValue));
		}
		else {
			CControlUI::SetAttribute(pstrName, pstrValue);
		}
	}

	void CWebpAnimUI::SetBkImage(std::wstring_view pStrImage)
	{
		UpdateWebpSource(pStrImage, true);
	}

	std::wstring_view CWebpAnimUI::GetBkImage() const
	{
		return m_sBkImage;
	}

	bool CWebpAnimUI::SetWebpData(const void* pData, DWORD dwSize, bool bInvalidate)
	{
		if (pData == nullptr || dwSize == 0) {
			return false;
		}

		m_bUseCustomData = true;
		m_sBkImage.clear();
		m_webpData.assign(reinterpret_cast<const BYTE*>(pData), reinterpret_cast<const BYTE*>(pData) + dwSize);
		const bool bLoaded = ReloadWebpImage();
		if (bLoaded && bInvalidate) {
			Invalidate();
		}
		return bLoaded;
	}

	void CWebpAnimUI::ClearWebpData()
	{
		m_bUseCustomData = false;
		m_sBkImage.clear();
		m_webpData.clear();
		DeleteWebp();
		Invalidate();
	}

	void CWebpAnimUI::SetAutoPlay(bool bIsAuto)
	{
		if (m_bIsAutoPlay == bIsAuto) {
			return;
		}
		m_bIsAutoPlay = bIsAuto;
		if (!m_bIsAutoPlay) {
			m_bResumePendingOnVisible = false;
		}
		SyncWebpPlaybackState();
	}

	bool CWebpAnimUI::IsAutoPlay() const
	{
		return m_bIsAutoPlay;
	}

	void CWebpAnimUI::SetAutoSize(bool bIsAuto)
	{
		m_bIsAutoSize = bIsAuto;
	}

	bool CWebpAnimUI::IsAutoSize() const
	{
		return m_bIsAutoSize;
	}

	void CWebpAnimUI::SetLoop(bool bLoop)
	{
		m_bLoop = bLoop;
	}

	bool CWebpAnimUI::IsLoop() const
	{
		return m_bLoop;
	}

	void CWebpAnimUI::SetHideAction(WebpHideAction action)
	{
		if (m_hideAction == action) {
			return;
		}
		m_hideAction = action;
		SyncWebpPlaybackState();
	}

	CWebpAnimUI::WebpHideAction CWebpAnimUI::GetHideAction() const
	{
		return m_hideAction;
	}

	void CWebpAnimUI::PlayWebp()
	{
		if (!EnsureWebpLoaded()) {
			return;
		}
		if (m_bIsPlaying || !CanRunWebpPlayback()) {
			return;
		}
		if (!m_bLoop && !m_frames.empty() && m_nFramePosition >= m_frames.size() - 1) {
			m_nFramePosition = 0;
		}

		ScheduleFrameTimer(m_nFramePosition);
		m_bIsPlaying = true;
		m_bResumePendingOnVisible = false;
		m_playbackState = WebpPlaybackState_Playing;
	}

	void CWebpAnimUI::PauseWebp()
	{
		if (!m_bIsPlaying) {
			return;
		}
		m_bResumePendingOnVisible = false;
		StopWebpPlayback(false, true);
		m_playbackState = WebpPlaybackState_Paused;
	}

	void CWebpAnimUI::StopWebp()
	{
		if (!HasActiveWebpPlaybackState()) {
			return;
		}
		m_bResumePendingOnVisible = false;
		StopWebpPlayback(true, true);
		m_playbackState = WebpPlaybackState_Stopped;
	}

	void CWebpAnimUI::RestartWebp()
	{
		if (!EnsureWebpLoaded()) {
			return;
		}
		StopWebpPlayback(true, false);
		m_nFramePosition = 0;
		m_playbackState = WebpPlaybackState_Stopped;
		Invalidate();
		PlayWebp();
	}

	bool CWebpAnimUI::SeekToWebpFrame(UINT nFramePosition, bool bInvalidate)
	{
		if (!EnsureWebpLoaded()) {
			return false;
		}
		if (nFramePosition >= m_frames.size()) {
			return false;
		}

		m_nFramePosition = nFramePosition;
		if (m_bIsPlaying) {
			KillWebpTimer();
			ScheduleFrameTimer(m_nFramePosition);
			m_playbackState = WebpPlaybackState_Playing;
		}
		else if (m_playbackState == WebpPlaybackState_Completed) {
			m_playbackState = WebpPlaybackState_Paused;
		}
		if (bInvalidate) {
			Invalidate();
		}
		return true;
	}

	UINT CWebpAnimUI::GetWebpFrameCount() const
	{
		return static_cast<UINT>(m_frames.size());
	}

	UINT CWebpAnimUI::GetCurrentWebpFrameIndex() const
	{
		return m_nFramePosition;
	}

	CWebpAnimUI::WebpPlaybackState CWebpAnimUI::GetWebpPlaybackState() const
	{
		return m_playbackState;
	}

	bool CWebpAnimUI::IsWebpPlaying() const
	{
		return m_bIsPlaying;
	}

	CWebpAnimUI* CWebpAnimUI::Clone()
	{
		CWebpAnimUI* pClone = new CWebpAnimUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CWebpAnimUI::CopyData(CWebpAnimUI* pControl)
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
		m_webpData = pControl->m_webpData;
		__super::CopyData(pControl);
		ReloadWebpImage();
	}

	bool CWebpAnimUI::HasWebpFrames() const
	{
		return !m_frames.empty() && m_nFramePosition < m_frames.size() && m_frames[m_nFramePosition].hBitmap != nullptr;
	}

	bool CWebpAnimUI::HasPlayableWebpFrames() const
	{
		return m_frames.size() > 1;
	}

	bool CWebpAnimUI::HasActiveWebpPlaybackState() const
	{
		return m_bIsPlaying || m_nFramePosition != 0;
	}

	bool CWebpAnimUI::IsWebpPlaybackVisible() const
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

	bool CWebpAnimUI::CanRunWebpPlayback() const
	{
		return m_pManager != nullptr && HasPlayableWebpFrames() && IsWebpPlaybackVisible();
	}

	bool CWebpAnimUI::ShouldAutoPlayWebp()
	{
		return m_bIsAutoPlay && IsWebpPlaybackVisible();
	}

	bool CWebpAnimUI::EnsureWebpLoaded()
	{
		if (!HasWebpFrames()) {
			return ReloadWebpImage();
		}
		return true;
	}

	bool CWebpAnimUI::ReloadWebpImage()
	{
		DeleteWebp();

		if (m_bUseCustomData) {
			if (m_webpData.empty()) {
				return false;
			}
			return DecodeWebpBytes(m_webpData.data(), static_cast<DWORD>(m_webpData.size()));
		}

		if (m_sBkImage.empty()) {
			return false;
		}

		const WebpSourceDescriptor descriptor = ParseWebpSourceDescriptor(m_sBkImage);
		if (descriptor.location.empty()) {
			return false;
		}

		LPBYTE pData = nullptr;
		DWORD dwSize = 0;
		if (!LoadWebpBytesFromDescriptor(descriptor, pData, dwSize)) {
			return false;
		}

		m_webpData.assign(pData, pData + dwSize);
		delete[] pData;
		return DecodeWebpBytes(m_webpData.data(), static_cast<DWORD>(m_webpData.size()));
	}

	bool CWebpAnimUI::DecodeWebpBytes(const BYTE* pData, DWORD dwSize)
	{
		if (pData == nullptr || dwSize == 0) {
			return false;
		}

		std::vector<WebpAnimationFrameBitmapInternal> decodedFrames;
		SIZE canvasSize = { 0, 0 };
		DWORD mask = 0;
		if (!m_bUseCustomData && !m_sBkImage.empty()) {
			mask = ParseWebpSourceDescriptor(m_sBkImage).mask;
		}
		if (!DecodeWebpAnimationFromMemoryInternal(pData, dwSize, mask, decodedFrames, canvasSize)) {
			return false;
		}

		m_szCanvas = canvasSize;
		m_frames.reserve(decodedFrames.size());
		for (WebpAnimationFrameBitmapInternal& frame : decodedFrames) {
			WebpFrame webpFrame;
			webpFrame.hBitmap = frame.hBitmap;
			webpFrame.delayMs = frame.delayMs;
			webpFrame.hasAlpha = frame.hasAlpha;
			frame.hBitmap = nullptr;
			m_frames.push_back(webpFrame);
		}
		decodedFrames.clear();

		m_nFramePosition = 0;
		m_playbackState = WebpPlaybackState_Stopped;
		if (m_bIsAutoSize && m_szCanvas.cx > 0 && m_szCanvas.cy > 0) {
			SetFixedWidth(m_szCanvas.cx);
			SetFixedHeight(m_szCanvas.cy);
		}

		SyncWebpPlaybackState();
		return HasWebpFrames();
	}

	bool CWebpAnimUI::UpdateWebpSource(std::wstring_view pStrImage, bool bInvalidate)
	{
		if (pStrImage.empty() || m_sBkImage == pStrImage) {
			return false;
		}

		m_bUseCustomData = false;
		m_webpData.clear();
		m_sBkImage = pStrImage;
		const bool bLoaded = ReloadWebpImage();
		if (bInvalidate) {
			Invalidate();
		}
		return bLoaded;
	}

	UINT CWebpAnimUI::GetFrameDelayMs(UINT nFramePosition) const
	{
		if (nFramePosition >= m_frames.size()) {
			return 100U;
		}

		const UINT delayMs = m_frames[nFramePosition].delayMs;
		return delayMs == 0 ? 100U : delayMs;
	}

	void CWebpAnimUI::ReleaseWebpFrames()
	{
		for (WebpFrame& frame : m_frames) {
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

	void CWebpAnimUI::DeleteWebp()
	{
		ResetWebpPlaybackState();
		ReleaseWebpFrames();
	}

	void CWebpAnimUI::KillWebpTimer()
	{
		if (m_pManager != nullptr) {
			m_pManager->KillTimer(this, EVENT_TIMER_ID);
		}
	}

	void CWebpAnimUI::StopWebpPlayback(bool bResetFrame, bool bInvalidate)
	{
		KillWebpTimer();
		m_bIsPlaying = false;
		if (bResetFrame) {
			m_nFramePosition = 0;
		}
		if (bInvalidate) {
			Invalidate();
		}
	}

	void CWebpAnimUI::ResetWebpPlaybackState()
	{
		m_bResumePendingOnVisible = false;
		StopWebpPlayback(true, false);
		m_playbackState = WebpPlaybackState_Stopped;
	}

	void CWebpAnimUI::SyncWebpPlaybackState()
	{
		if (!IsWebpPlaybackVisible()) {
			if (m_bIsPlaying) {
				m_bResumePendingOnVisible = true;
				StopWebpPlayback(false, true);
				m_playbackState = WebpPlaybackState_Paused;
			}
			return;
		}

		if (m_bResumePendingOnVisible) {
			PlayWebp();
			return;
		}
		if (ShouldAutoPlayWebp()) {
			PlayWebp();
			return;
		}
		if (m_bIsPlaying) {
			m_bResumePendingOnVisible = false;
			StopWebpPlayback(false, true);
		}
	}

	void CWebpAnimUI::ScheduleFrameTimer(UINT nFramePosition)
	{
		if (m_pManager == nullptr) {
			return;
		}
		m_pManager->SetTimer(this, EVENT_TIMER_ID, GetFrameDelayMs(nFramePosition));
	}

	bool CWebpAnimUI::AdvanceWebpFrame()
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

	void CWebpAnimUI::OnTimer(UINT_PTR idEvent)
	{
		if (idEvent != EVENT_TIMER_ID) {
			return;
		}

		const bool wasPlaying = m_bIsPlaying;
		KillWebpTimer();
		if (!CanRunWebpPlayback()) {
			if (wasPlaying && !IsWebpPlaybackVisible()) {
				m_bResumePendingOnVisible = true;
			}
			StopWebpPlayback(false, false);
			m_playbackState = WebpPlaybackState_Paused;
			return;
		}

		Invalidate();
		if (!AdvanceWebpFrame()) {
			StopWebpPlayback(false, true);
			m_playbackState = WebpPlaybackState_Completed;
			NotifyWebpPlaybackComplete();
			return;
		}
		ScheduleFrameTimer(m_nFramePosition);
	}

	void CWebpAnimUI::NotifyWebpPlaybackComplete()
	{
		m_bResumePendingOnVisible = false;
		if (m_pManager != nullptr) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_WEBPPLAYCOMPLETE, static_cast<WPARAM>(m_frames.size()), static_cast<LPARAM>(m_nFramePosition));
		}
	}

	void CWebpAnimUI::DrawFrame(CPaintRenderContext& renderContext)
	{
		if (!HasWebpFrames()) {
			return;
		}

		const WebpFrame& frame = m_frames[m_nFramePosition];
		if (frame.hBitmap == nullptr) {
			return;
		}

		RECT rcBitmap = { 0, 0, m_szCanvas.cx, m_szCanvas.cy };
		RECT rcEmptyCorners = { 0, 0, 0, 0 };
		CRenderEngine::DrawImage(renderContext, frame.hBitmap, m_rcItem, rcBitmap, rcEmptyCorners, frame.hasAlpha, 255);
	}
}
