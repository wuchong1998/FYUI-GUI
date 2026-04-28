#include "pch.h"
#include "UIGifAnim.h"
#include "../Core/UIRenderContext.h"

///////////////////////////////////////////////////////////////////////////////////////
namespace FYUI
{
	IMPLEMENT_DUICONTROL(CGifAnimUI)

	CGifAnimUI::CGifAnimUI(void)
	{
		m_pGifImageInfo		=	NULL;
		m_pPropertyItem		=	NULL;
		m_nFrameCount		=	0;	
		m_nFramePosition	=	0;	
		m_szFrameBitmap.cx	=	0;
		m_szFrameBitmap.cy	=	0;
		m_nCachedFramePosition = static_cast<UINT>(-1);
		m_bFrameCacheValid	=	false;
		m_bIsAutoPlay		=	true;
		m_bIsAutoSize		=	false;
		m_bIsPlaying		=	false;

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
		const RECT& rcPaint = renderContext.GetPaintRect();
		if( !::IntersectRect( &m_rcPaint, &rcPaint, &m_rcItem ) ) return true;
		if (!EnsureGifImageLoaded()) return true;
		DrawFrame(renderContext);
		return true;
	}

	void CGifAnimUI::DoEvent( TEventUI& event )
	{
		if( event.Type == UIEVENT_TIMER )
			OnTimer( (UINT_PTR)event.wParam );
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
		if( StringUtil::CompareNoCase(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
		else if( StringUtil::CompareNoCase(pstrName, _T("autoplay")) == 0 ) {
			SetAutoPlay(StringUtil::CompareNoCase(pstrValue, _T("true")) == 0);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("autosize")) == 0 ) {
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
		return m_pGifImageInfo != NULL && m_pGifImageInfo->pImage != NULL;
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
		return HasGifImage() && m_nFrameCount > 1;
	}

	bool CGifAnimUI::ShouldAutoPlayGif()
	{
		return m_bIsAutoPlay && IsVisible();
	}

	bool CGifAnimUI::CanRunGifPlayback() const
	{
		return m_pManager != NULL && HasPlayableGifFrames();
	}

	bool CGifAnimUI::HasActiveGifPlaybackState() const
	{
		return m_bIsPlaying || m_nFramePosition != 0;
	}

	UINT CGifAnimUI::GetFrameDelayMs(UINT nFramePosition) const
	{
		if (m_pPropertyItem == NULL || nFramePosition >= m_nFrameCount) {
			return 100U;
		}

		long lPause = ((long*)m_pPropertyItem->value)[nFramePosition] * 10;
		if (lPause <= 0) {
			lPause = 100;
		}
		return static_cast<UINT>(lPause);
	}

	void CGifAnimUI::KillGifTimer()
	{
		if (m_pManager != NULL) {
			m_pManager->KillTimer(this, EVENT_TIEM_ID);
		}
	}

	void CGifAnimUI::StopGifPlayback(bool bResetFrame, bool bInvalidate)
	{
		KillGifTimer();
		m_bIsPlaying = false;
		if (bResetFrame) {
			m_nFramePosition = 0;
			InvalidateFrameCache();
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

	void CGifAnimUI::ReleaseGifMetadata()
	{
		if (m_pPropertyItem != NULL)
		{
			free(m_pPropertyItem);
			m_pPropertyItem = NULL;
		}
		m_nFrameCount = 0;
		m_nFramePosition = 0;
	}

	bool CGifAnimUI::AttachGifImageInfo(TImageInfo* pImageInfo)
	{
		if (pImageInfo == NULL || pImageInfo->pImage == NULL) {
			return false;
		}

		Gdiplus::Image* pGifImage = pImageInfo->pImage;
		UINT nCount = pGifImage->GetFrameDimensionsCount();
		if (nCount == 0) {
			return false;
		}

		GUID* pDimensionIDs = new GUID[nCount];
		pGifImage->GetFrameDimensionsList(pDimensionIDs, nCount);
		m_nFrameCount = pGifImage->GetFrameCount(&pDimensionIDs[0]);
		if (m_nFrameCount > 1) {
			int nSize = pGifImage->GetPropertyItemSize(PropertyTagFrameDelay);
			if (nSize > 0) {
				m_pPropertyItem = (Gdiplus::PropertyItem*) malloc(nSize);
				if (m_pPropertyItem != NULL &&
					pGifImage->GetPropertyItem(PropertyTagFrameDelay, nSize, m_pPropertyItem) != Gdiplus::Ok) {
					free(m_pPropertyItem);
					m_pPropertyItem = NULL;
				}
			}
		}
		delete[] pDimensionIDs;

		m_pGifImageInfo = pImageInfo;
		if (m_bIsAutoSize) {
			SetFixedWidth(pGifImage->GetWidth());
			SetFixedHeight(pGifImage->GetHeight());
		}
		SyncGifPlaybackState();
		return true;
	}

	void CGifAnimUI::ReleaseGifImageInfo()
	{
		if (m_pGifImageInfo != NULL)
		{
			CRenderEngine::FreeImage(m_pGifImageInfo);
			m_pGifImageInfo = NULL;
		}
	}

	void CGifAnimUI::ApplyGifReloadResult(TImageInfo* pImageInfo)
	{
		if (AttachGifImageInfo(pImageInfo)) {
			return;
		}

		if (pImageInfo != NULL) {
			CRenderEngine::FreeImage(pImageInfo);
		}
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

	bool CGifAnimUI::SelectActiveGifFrame() const
	{
		if (!HasGifImage()) {
			return false;
		}

		GUID pageGuid = Gdiplus::FrameDimensionTime;
		m_pGifImageInfo->pImage->SelectActiveFrame(&pageGuid, m_nFramePosition);
		return true;
	}

	void CGifAnimUI::ScheduleFrameTimer(UINT nFramePosition)
	{
		if (m_pManager == NULL) {
			return;
		}
		m_pManager->SetTimer(this, EVENT_TIEM_ID, GetFrameDelayMs(nFramePosition));
	}

	void CGifAnimUI::AdvanceGifFrame()
	{
		if (m_nFrameCount == 0) {
			return;
		}

		m_nFramePosition = (++m_nFramePosition) % m_nFrameCount;
		InvalidateFrameCache();
	}

	void CGifAnimUI::PlayGif()
	{
		if (!EnsureGifImageLoaded())
		{
			return;
		}

		if (m_bIsPlaying || !CanRunGifPlayback())
		{
			return;
		}

		ScheduleFrameTimer(m_nFramePosition);
		m_bIsPlaying = true;
	}

	void CGifAnimUI::PauseGif()
	{
		if (!m_bIsPlaying || m_pGifImageInfo == NULL || m_pGifImageInfo->pImage == NULL)
		{
			return;
		}

		StopGifPlayback(false, true);
	}

	void CGifAnimUI::StopGif()
	{
		if (!HasActiveGifPlaybackState())
		{
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
		TImageInfo* pImageInfo = CRenderEngine::GdiplusLoadImage(std::wstring_view(GetBkImage()));
		ApplyGifReloadResult(pImageInfo);
	}

	void CGifAnimUI::InitGifImage()
	{
		ReloadGifImage();
	}

	void CGifAnimUI::DeleteGif()
	{
		ResetGifPlaybackState();
		ReleaseGifMetadata();
		ReleaseGifImageInfo();
	}

	void CGifAnimUI::OnTimer( UINT_PTR idEvent )
	{
		if ( idEvent != EVENT_TIEM_ID )
			return;
		KillGifTimer();
		if (!CanRunGifPlayback()) {
			StopGifPlayback(false, false);
			return;
		}
		this->Invalidate();
		AdvanceGifFrame();
		ScheduleFrameTimer(m_nFramePosition);
	}

	void CGifAnimUI::InvalidateFrameCache()
	{
		m_frameSurface.Reset();
		m_szFrameBitmap.cx = 0;
		m_szFrameBitmap.cy = 0;
		m_nCachedFramePosition = static_cast<UINT>(-1);
		m_bFrameCacheValid = false;
	}

	bool CGifAnimUI::EnsureFrameCache(CPaintRenderContext& renderContext, LONG cx, LONG cy)
	{
		if (m_pGifImageInfo == NULL || m_pGifImageInfo->pImage == NULL || cx <= 0 || cy <= 0) {
			return false;
		}

		if (m_bFrameCacheValid && m_frameSurface.GetBitmap() != NULL &&
			m_nCachedFramePosition == m_nFramePosition &&
			m_szFrameBitmap.cx == cx && m_szFrameBitmap.cy == cy) {
			return true;
		}

		InvalidateFrameCache();

		if (!m_frameSurface.Ensure(renderContext, cx, cy)) {
			return false;
		}

		Gdiplus::Image* pGifImage = m_pGifImageInfo->pImage;
		if (!SelectActiveGifFrame()) {
			return false;
		}

		if (!m_frameSurface.DrawGdiplusImage(pGifImage, cx, cy)) {
			return false;
		}

		m_szFrameBitmap.cx = cx;
		m_szFrameBitmap.cy = cy;
		m_nCachedFramePosition = m_nFramePosition;
		m_bFrameCacheValid = true;
		return true;
	}

	void CGifAnimUI::DrawFrame(CPaintRenderContext& renderContext)
	{
		if (m_pGifImageInfo == NULL || m_pGifImageInfo->pImage == NULL) return;

		const LONG cx = m_rcItem.right - m_rcItem.left;
		const LONG cy = m_rcItem.bottom - m_rcItem.top;
		if (cx <= 0 || cy <= 0) return;

		if (EnsureFrameCache(renderContext, cx, cy) && m_frameSurface.GetBitmap() != NULL) {
			RECT rcBitmap = { 0, 0, m_szFrameBitmap.cx, m_szFrameBitmap.cy };
			RECT rcEmptyCorners = { 0, 0, 0, 0 };
			CRenderEngine::DrawImage(renderContext, m_frameSurface.GetBitmap(), m_rcItem, rcBitmap, rcEmptyCorners, true, 255);
			return;
		}

	}
}


