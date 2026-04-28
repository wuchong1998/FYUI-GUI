#include "pch.h"
#include "UIRing.h"
#include "../Core/UIRenderContext.h"

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CRingUI)

	CRingUI::CRingUI() : m_fCurAngle(0.0f), m_pBkimageInfo(NULL)
	{
	}

	CRingUI::~CRingUI()
	{
		if (m_pManager) m_pManager->KillTimer(this, RING_TIMERID);

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
		if (StringUtil::CompareNoCase(pstrName, _T("bkimage")) == 0) SetBkImage(pstrValue);
		else CLabelUI::SetAttribute(pstrName, pstrValue);
	}

	void CRingUI::SetBkImage(std::wstring_view pStrImage)
	{
		if (m_sBkImage == pStrImage) return;
		m_sBkImage = pStrImage;
		DeleteImage();
		Invalidate();
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
		__super::CopyData(pControl);
		InitImage();
	}

	void CRingUI::DoEvent(TEventUI& event)
	{
		if (event.Type == UIEVENT_TIMER && event.wParam == RING_TIMERID) {
			if (m_fCurAngle > 359) {
				m_fCurAngle = 0;
			}
			m_fCurAngle += 36.0;
			NeedParentUpdate();
		}
		else {
			CLabelUI::DoEvent(event);
		}
	}

	void CRingUI::InitImage()
	{
		const std::wstring bkImage(GetBkImage());
		TImageInfo* pImageInfo = CRenderEngine::GdiplusLoadImage(std::wstring_view(bkImage));
		if (pImageInfo != NULL && pImageInfo->hBitmap != NULL) {
			m_pBkimageInfo = pImageInfo;
			if (m_pManager != NULL) {
				m_pManager->SetTimer(this, RING_TIMERID, 100);
			}
		}
		else if (pImageInfo != NULL) {
			CRenderEngine::FreeImage(pImageInfo);
		}
	}

	void CRingUI::DeleteImage()
	{
		if (m_pBkimageInfo != NULL)
		{
			CRenderEngine::FreeImage(m_pBkimageInfo);
			m_pBkimageInfo = NULL;
		}
	}
}
