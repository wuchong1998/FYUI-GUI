#include "pch.h"
#include "UIRing.h"
#include "../Core/Render/UIRenderContext.h"

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CRingUI)

	CRingUI::CRingUI()
		: m_fCurAngle(0.0f)
		, m_pBkimageInfo(NULL)
		, m_bAutoPlay(true)
		, m_bPlaying(true)
		, m_bTimerStarted(false)
		, m_fRotateSpeed(360.0f)
		, m_nIntervalMs(100)
	{
	}

	CRingUI::~CRingUI()
	{
		StopTimer();
		DeleteImage();
	}

	std::wstring_view CRingUI::GetClass() const
	{
		return _T("RingUI");
	}

	LPVOID CRingUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("Ring")) == 0) return static_cast<CRingUI*>(this);
		return CLabelUI::GetInterface(pstrName);
	}

	void CRingUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("bkimage")) == 0) {
			SetBkImage(pstrValue);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("autoplay")) == 0) {
			bool b = !(StringUtil::CompareNoCase(pstrValue, _T("false")) == 0 || pstrValue == _T("0"));
			SetAutoPlay(b);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("playing")) == 0) {
			bool b = !(StringUtil::CompareNoCase(pstrValue, _T("false")) == 0 || pstrValue == _T("0"));
			if (b) Play(); else Pause();
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("speed")) == 0) {
			float f = 360.0f;
			if (swscanf_s(std::wstring(pstrValue).c_str(), L"%f", &f) == 1) SetRotateSpeed(f);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("interval")) == 0) {
			int v = 100;
			if (StringUtil::TryParseInt(pstrValue, v)) SetRotateInterval(v);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("angle")) == 0) {
			float f = 0.0f;
			if (swscanf_s(std::wstring(pstrValue).c_str(), L"%f", &f) == 1) SetAngle(f);
		}
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CRingUI::SetBkImage(std::wstring_view pStrImage)
	{
		if (m_sBkImage == pStrImage) return;
		m_sBkImage = pStrImage;
		DeleteImage();
		Invalidate();
	}

	// ---- 播放控制实现 ----
	void CRingUI::Play()
	{
		m_bPlaying = true;
		StartTimerIfNeeded();
	}

	void CRingUI::Pause()
	{
		m_bPlaying = false;
		StopTimer();
	}

	void CRingUI::Stop()
	{
		m_bPlaying = false;
		StopTimer();
		m_fCurAngle = 0.0f;
		Invalidate();
	}

	bool CRingUI::IsPlaying() const
	{
		return m_bPlaying && m_bTimerStarted;
	}

	void CRingUI::SetAutoPlay(bool bAutoPlay)
	{
		m_bAutoPlay = bAutoPlay;
		if (bAutoPlay) Play();
		else Pause();
	}

	bool CRingUI::IsAutoPlay() const
	{
		return m_bAutoPlay;
	}

	void CRingUI::SetRotateSpeed(float fSpeed)
	{
		m_fRotateSpeed = fSpeed;
	}

	float CRingUI::GetRotateSpeed() const
	{
		return m_fRotateSpeed;
	}

	void CRingUI::SetRotateInterval(int nIntervalMs)
	{
		if (nIntervalMs < 1) nIntervalMs = 1;
		if (nIntervalMs == m_nIntervalMs) return;
		m_nIntervalMs = nIntervalMs;
		// 正在运行中需重启定时器以采用新间隔
		if (m_bTimerStarted) {
			StopTimer();
			StartTimerIfNeeded();
		}
	}

	int CRingUI::GetRotateInterval() const
	{
		return m_nIntervalMs;
	}

	void CRingUI::SetAngle(float fAngle)
	{
		while (fAngle >= 360.0f) fAngle -= 360.0f;
		while (fAngle < 0.0f) fAngle += 360.0f;
		m_fCurAngle = fAngle;
		Invalidate();
	}

	float CRingUI::GetAngle() const
	{
		return m_fCurAngle;
	}

	void CRingUI::StartTimerIfNeeded()
	{
		if (m_bTimerStarted) return;
		if (!m_bPlaying) return;
		if (m_pManager == NULL) return;
		if (m_pBkimageInfo == NULL || m_pBkimageInfo->hBitmap == NULL) return;
		m_pManager->SetTimer(this, RING_TIMERID, static_cast<UINT>(max(1, m_nIntervalMs)));
		m_bTimerStarted = true;
	}

	void CRingUI::StopTimer()
	{
		if (!m_bTimerStarted) return;
		if (m_pManager != NULL) m_pManager->KillTimer(this, RING_TIMERID);
		m_bTimerStarted = false;
	}

	void CRingUI::PaintBkImage(CPaintRenderContext& renderContext)
	{
		if (m_pBkimageInfo == NULL || m_pBkimageInfo->hBitmap == NULL) {
			InitImage();
		}

		if (m_pBkimageInfo != NULL && m_pBkimageInfo->hBitmap != NULL) {
			RECT rcBitmapPart = { 0, 0, m_pBkimageInfo->nX, m_pBkimageInfo->nY };
			CRenderEngine::DrawRotateImage(
				renderContext,
				m_pBkimageInfo->hBitmap,
				m_rcItem,
				rcBitmapPart,
				m_pManager != NULL && m_pManager->IsLayered() ? true : m_pBkimageInfo->bAlpha,
				255,
				static_cast<UINT>(m_fCurAngle));
		}
	}

	CRingUI* CRingUI::Clone()
	{
		CRingUI* pClone = new CRingUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CRingUI::CopyData(CRingUI* pControl)
	{
		m_fCurAngle = pControl->m_fCurAngle;
		m_bAutoPlay = pControl->m_bAutoPlay;
		m_bPlaying = pControl->m_bPlaying;
		m_fRotateSpeed = pControl->m_fRotateSpeed;
		m_nIntervalMs = pControl->m_nIntervalMs;
		__super::CopyData(pControl);
		InitImage();
	}

	void CRingUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER && event.wParam == RING_TIMERID) {
			if (!m_bPlaying) {
				StopTimer();
				return;
			}
			// 按“度/秒 × 间隔”累加角度，保证 speed/interval 任意组合下转速一致
			const float delta = m_fRotateSpeed * (static_cast<float>(m_nIntervalMs) / 1000.0f);
			m_fCurAngle += delta;
			while (m_fCurAngle >= 360.0f) m_fCurAngle -= 360.0f;
			while (m_fCurAngle < 0.0f) m_fCurAngle += 360.0f;
			// Bug 修复：刷新自身即可，不必拖动父容器重布局
			NeedParentUpdate();
			//Invalidate();
			return;
		}
		CLabelUI::DoEvent(event);
	}

	void CRingUI::InitImage()
	{
		if (m_pBkimageInfo != NULL && m_pBkimageInfo->hBitmap != NULL) return;
		const std::wstring bkImage(GetBkImage());
		TImageInfo* pImageInfo = CRenderEngine::GdiplusLoadImage(std::wstring_view(bkImage));
		if (pImageInfo != NULL && pImageInfo->hBitmap != NULL) {
			m_pBkimageInfo = pImageInfo;
			// 图像加载完成后，依据当前播放状态启动定时器
			StartTimerIfNeeded();
		}
		else if (pImageInfo != NULL) {
			CRenderEngine::FreeImage(pImageInfo);
		}
	}

	void CRingUI::DeleteImage()
	{
		StopTimer();
		if (m_pBkimageInfo != NULL)
		{
			CRenderEngine::FreeImage(m_pBkimageInfo);
			m_pBkimageInfo = NULL;
		}
	}
}
