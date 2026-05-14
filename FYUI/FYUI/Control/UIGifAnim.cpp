#include "pch.h"
#include "UIGifAnim.h"
#include "../Core/Render/UIRenderContext.h"
#include "../Core/Render/UIRenderImageCodecInternal.h"

///////////////////////////////////////////////////////////////////////////////////////
namespace FYUI
{
	IMPLEMENT_DUICONTROL(CGifAnimUI)

	CGifAnimUI::CGifAnimUI(void)
	{
		m_szCanvas.cx = 0;
		m_szCanvas.cy = 0;
		m_nFramePosition = 0;
		m_bIsAutoPlay = true;
		m_bIsAutoSize = false;
		m_bIsPlaying = false;
	}

	CGifAnimUI::~CGifAnimUI(void)
	{
		DeleteGif();
	}

	std::wstring_view CGifAnimUI::GetClass() const
	{
		return _T("GifAnimUI");
	}

	LPVOID CGifAnimUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_GIFANIM) == 0) return static_cast<CGifAnimUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CGifAnimUI::DoInit()
	{
		InitGifImage();
	}

	bool CGifAnimUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		(void)pStopControl;
		const RECT& rcPaint = renderContext.GetPaintRect();
		if (!::IntersectRect(&m_rcPaint, &rcPaint, &m_rcItem)) return true;
		if (!EnsureGifImageLoaded()) return true;
		DrawFrame(renderContext);
		return true;
	}

	void CGifAnimUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER)
			OnTimer(static_cast<UINT_PTR>(event.wParam));
	}

	void CGifAnimUI::SetVisible(bool bVisible /* = true */)
	{
		if (IsVisible() == bVisible) {
			return;
		}
		CControlUI::SetVisible(bVisible);
		SyncGifPlaybackState();
	}

	void CGifAnimUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("bkimage")) == 0) SetBkImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("autoplay")) == 0) {
			SetAutoPlay(StringUtil::CompareNoCase(pstrValue, _T("true")) == 0);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("autosize")) == 0) {
			SetAutoSize(StringUtil::CompareNoCase(pstrValue, _T("true")) == 0);
		}
		else
			CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CGifAnimUI::SetBkImage(std::wstring_view pStrImage)
	{
		UpdateGifSource(pStrImage, true);
	}

	std::wstring_view CGifAnimUI::GetBkImage()
	{
		return m_sBkImage;
	}

	void CGifAnimUI::SetAutoPlay(bool bIsAuto)
	{
		if (m_bIsAutoPlay == bIsAuto) {
			return;
		}
		m_bIsAutoPlay = bIsAuto;
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

	bool CGifAnimUI::HasGifImage() const
	{
		return !m_frames.empty() && m_frames.front().hBitmap != nullptr;
	}

	bool CGifAnimUI::EnsureGifImageLoaded()
	{
		if (!HasGifImage()) {
			ReloadGifImage();
		}
		return HasGifImage();
	}

	bool CGifAnimUI::HasPlayableGifFrames() const
	{
		return HasGifImage() && m_frames.size() > 1;
	}

	bool CGifAnimUI::ShouldAutoPlayGif()
	{
		return m_bIsAutoPlay && IsVisible();
	}

	bool CGifAnimUI::CanRunGifPlayback() const
	{
		return m_pManager != nullptr && HasPlayableGifFrames();
	}

	bool CGifAnimUI::HasActiveGifPlaybackState() const
	{
		return m_bIsPlaying || m_nFramePosition != 0;
	}

	UINT CGifAnimUI::GetFrameDelayMs(UINT nFramePosition) const
	{
		if (nFramePosition >= m_frames.size()) {
			return 100U;
		}
		const UINT delayMs = m_frames[nFramePosition].delayMs;
		return delayMs == 0 ? 100U : delayMs;
	}

	void CGifAnimUI::KillGifTimer()
	{
		if (m_pManager != nullptr) {
			m_pManager->KillTimer(this, EVENT_TIEM_ID);
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
		StopGifPlayback(true, false);
	}

	void CGifAnimUI::SyncGifPlaybackState()
	{
		if (ShouldAutoPlayGif()) {
			PlayGif();
			return;
		}

		if (HasActiveGifPlaybackState()) {
			StopGif();
		}
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

	bool CGifAnimUI::UpdateGifSource(std::wstring_view pStrImage, bool bInvalidate)
	{
		if (pStrImage.empty() || m_sBkImage == pStrImage) {
			return false;
		}

		m_sBkImage = pStrImage;
		ReloadGifImage();
		if (bInvalidate) {
			Invalidate();
		}
		return true;
	}

	void CGifAnimUI::ScheduleFrameTimer(UINT nFramePosition)
	{
		if (m_pManager == nullptr) {
			return;
		}
		m_pManager->SetTimer(this, EVENT_TIEM_ID, GetFrameDelayMs(nFramePosition));
	}

	void CGifAnimUI::AdvanceGifFrame()
	{
		if (m_frames.empty()) {
			return;
		}
		m_nFramePosition = (m_nFramePosition + 1) % static_cast<UINT>(m_frames.size());
	}

	void CGifAnimUI::PlayGif()
	{
		if (!EnsureGifImageLoaded()) {
			return;
		}
		if (m_bIsPlaying || !CanRunGifPlayback()) {
			return;
		}
		ScheduleFrameTimer(m_nFramePosition);
		m_bIsPlaying = true;
	}

	void CGifAnimUI::PauseGif()
	{
		if (!m_bIsPlaying) {
			return;
		}
		StopGifPlayback(false, true);
	}

	void CGifAnimUI::StopGif()
	{
		if (!HasActiveGifPlaybackState()) {
			return;
		}
		StopGifPlayback(true, true);
	}

	CGifAnimUI* CGifAnimUI::Clone()
	{
		CGifAnimUI* pClone = new CGifAnimUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CGifAnimUI::CopyData(CGifAnimUI* pControl)
	{
		m_bIsAutoPlay = pControl->m_bIsAutoPlay;
		m_bIsAutoSize = pControl->m_bIsAutoSize;
		__super::CopyData(pControl);
		if (!UpdateGifSource(pControl->m_sBkImage, false)) {
			ReloadGifImage();
		}
	}

	void CGifAnimUI::ReloadGifImage()
	{
		DeleteGif();
		if (m_sBkImage.empty()) {
			return;
		}

		// 解析 bkimage 字符串：支持 "file='xxx.gif'" 风格，也兼容裸路径 "xxx.gif"
		std::wstring sImagePath;
		bool bHasStructuredAttribute = false;
		for (const auto& attribute : StringUtil::ParseQuotedAttributes(m_sBkImage, L'\'')) {
			if (attribute.value.empty()) continue;
			if (StringUtil::EqualsNoCase(attribute.key, L"file")) {
				sImagePath.assign(attribute.value);
				bHasStructuredAttribute = true;
			}
		}
		if (!bHasStructuredAttribute) {
			sImagePath = m_sBkImage;
		}
		if (sImagePath.empty()) {
			return;
		}

		// 拼接资源根路径
		std::wstring sFullPath;
		const std::wstring& sResourceRoot = CPaintManagerUI::GetResourcePath();
		if (!sResourceRoot.empty()) {
			sFullPath = sResourceRoot;
			sFullPath += sImagePath;
		}
		else {
			sFullPath = sImagePath;
		}

		std::vector<GifAnimationFrameBitmapInternal> decodedFrames;
		SIZE canvasSize = { 0, 0 };
		if (!DecodeGifAnimationFromFileInternal(sFullPath.c_str(), decodedFrames, canvasSize)) {
			decodedFrames.clear();
			if (!DecodeGifAnimationFromFileInternal(sImagePath.c_str(), decodedFrames, canvasSize)) {
				return;
			}
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
		decodedFrames.clear();

		m_nFramePosition = 0;

		if (m_bIsAutoSize && m_szCanvas.cx > 0 && m_szCanvas.cy > 0) {
			SetFixedWidth(m_szCanvas.cx);
			SetFixedHeight(m_szCanvas.cy);
		}

		SyncGifPlaybackState();
	}

	void CGifAnimUI::InitGifImage()
	{
		ReloadGifImage();
	}

	void CGifAnimUI::DeleteGif()
	{
		ResetGifPlaybackState();
		ReleaseGifFrames();
	}

	void CGifAnimUI::OnTimer(UINT_PTR idEvent)
	{
		if (idEvent != EVENT_TIEM_ID)
			return;

		KillGifTimer();
		if (!CanRunGifPlayback()) {
			StopGifPlayback(false, false);
			return;
		}

		AdvanceGifFrame();
		Invalidate();
		ScheduleFrameTimer(m_nFramePosition);
	}

	void CGifAnimUI::DrawFrame(CPaintRenderContext& renderContext)
	{
		if (m_frames.empty() || m_nFramePosition >= m_frames.size()) {
			return;
		}
		const GifFrame& frame = m_frames[m_nFramePosition];
		if (frame.hBitmap == nullptr) {
			return;
		}

		const LONG cx = m_rcItem.right - m_rcItem.left;
		const LONG cy = m_rcItem.bottom - m_rcItem.top;
		if (cx <= 0 || cy <= 0) return;

		RECT rcBitmap = { 0, 0, m_szCanvas.cx, m_szCanvas.cy };
		RECT rcEmptyCorners = { 0, 0, 0, 0 };
		CRenderEngine::DrawImage(renderContext, frame.hBitmap, m_rcItem, rcBitmap, rcEmptyCorners, frame.hasAlpha, 255);
	}
}
