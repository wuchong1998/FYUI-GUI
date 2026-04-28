#include "pch.h"
#include "TooltipWnd.h"
#include "Core/UIRender.h"

namespace
{
	constexpr int kTooltipMeasureFontIdBase = 900000;

	int EnsureTooltipMeasureFontIndex(CPaintManagerUI& paintManager, int nFontSize)
	{
		TFontInfo* pDefaultFontInfo = paintManager.GetDefaultFontInfo();
		const std::wstring fontName = (pDefaultFontInfo != nullptr && !pDefaultFontInfo->sFontName.empty())
			? pDefaultFontInfo->sFontName
			: L"Microsoft YaHei";

		int iFont = paintManager.GetFontIndex(fontName, nFontSize, false, false, false, false);
		if (iFont >= 0) {
			return iFont;
		}

		iFont = paintManager.GetFontIndex(fontName, nFontSize, false, false, false, false, true);
		if (iFont >= 0) {
			return iFont;
		}

		const int iTooltipFontId = kTooltipMeasureFontIdBase + static_cast<int>(paintManager.GetDPI()) * 100 + nFontSize;
		paintManager.AddFont(iTooltipFontId, fontName, nFontSize, false, false, false, false);
		return iTooltipFontId;
	}

	SIZE CalculateTooltipTextSize(CPaintManagerUI& paintManager, std::wstring_view text, int maxWidth, int nFontSize)
	{
		SIZE size = { 0, 0 };
		if (text.empty()) {
			return size;
		}

		if (maxWidth <= 0) {
			maxWidth = 1;
		}

		RECT rcMeasure = { 0, 0, maxWidth, 4096 };
		CPaintRenderContext measureContext = paintManager.CreateMeasureRenderContext(rcMeasure);
		const int iFont = EnsureTooltipMeasureFontIndex(paintManager, nFontSize);
		CRenderEngine::DrawText(measureContext, rcMeasure, text, 0xFF000000, iFont, DT_CALCRECT | DT_WORDBREAK | DT_LEFT | DT_TOP);

		size.cx = rcMeasure.right - rcMeasure.left;
		size.cy = rcMeasure.bottom - rcMeasure.top;
		return size;
	}
}

TooltipWnd::TooltipWnd()
{
}

TooltipWnd::~TooltipWnd()
{
}

void TooltipWnd::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

void TooltipWnd::InitWindow()
{
}

void TooltipWnd::Notify(TNotifyUI& msg)
{
	__super::Notify(msg);
}

LRESULT TooltipWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == 11858)
	{
		TooltipInfo* pTooltipInfo = reinterpret_cast<TooltipInfo*>(lParam);
		if (pTooltipInfo)
		{
			if (pTooltipInfo->nDpi > 0 && static_cast<UINT>(pTooltipInfo->nDpi) != m_pm.GetDPI()) {
				m_pm.SetDPI(pTooltipInfo->nDpi);
			}
			SetTooltipInfo(pTooltipInfo->strText, pTooltipInfo->rcPos, pTooltipInfo->emToolTipType,
				pTooltipInfo->szTooltipGap, pTooltipInfo->nMaxWidth);
			delete pTooltipInfo;
			return 0;
		}
	}
	else if (uMsg == WM_DESTROY)
	{
		Close();
	}
	return __super::HandleMessage(uMsg, wParam, lParam);
}

std::wstring TooltipWnd::GetSkinFile()
{
	return _T("tooltip\\tooltip.xml");
}

std::wstring_view TooltipWnd::GetWindowClassName(void) const
{
	return _T("TooltipWnd");
}

void TooltipWnd::SetTooltipInfo(const std::wstring& strText, RECT rcPos, ToolTipType emToolTipType, SIZE szTooltipGap, int nMaxWidth)
{
	POINT ptAnchor = { rcPos.left + (rcPos.right - rcPos.left) / 2, rcPos.top + (rcPos.bottom - rcPos.top) / 2 };

	if (nMaxWidth < m_pm.ScaleValue(20)) {
		nMaxWidth = 9999;
	}

	SIZE szText = CalculateTooltipTextSize(m_pm, strText, nMaxWidth, 13);
	if (szText.cx < m_pm.ScaleValue(30)) {
		szText.cx = m_pm.ScaleValue(30);
	}
	szText.cx += m_pm.ScaleValue(10);

	if (szText.cy < m_pm.ScaleValue(23)) {
		szText.cy = m_pm.ScaleValue(23);
	}
	szText.cy += m_pm.ScaleValue(10);

	SIZE szWindow = { 0, 0 };
	if (emToolTipType == Tool_Top || emToolTipType == Tool_Bottom)
	{
		szWindow.cx = szText.cx;
		szWindow.cy += szText.cy + m_pm.ScaleValue(7);
	}
	else if (emToolTipType == Tool_Right || emToolTipType == Tool_Left)
	{
		szWindow.cx = szText.cx + m_pm.ScaleValue(7);
		szWindow.cy += szText.cy;
	}

	auto selectem = [](CHorizontalLayoutUI* ptab_layout, int nIndex)
	{
		if (ptab_layout == nullptr) {
			return;
		}

		for (int i = 0; i < ptab_layout->GetCount(); i++)
		{
			CHorizontalLayoutUI* pHor = static_cast<CHorizontalLayoutUI*>(ptab_layout->GetItemAt(i));
			if (i == nIndex) {
				pHor->SetVisible(true, false);
			}
			else {
				pHor->SetVisible(false, false);
			}
		}
	};

	CHorizontalLayoutUI* ptab_layout = static_cast<CHorizontalLayoutUI*>(m_pm.FindControl(_T("tab_layout")));
	CLabelUI* pTootipText = nullptr;
	CLabelUI* pTootip = nullptr;
	if (emToolTipType == Tool_Left)
	{
		pTootipText = static_cast<CLabelUI*>(m_pm.FindControl(_T("tootip_left_text")));
		pTootip = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_left")));
		selectem(ptab_layout, 0);
	}
	else if (emToolTipType == Tool_Top)
	{
		pTootipText = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_top_text")));
		pTootip = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_top")));
		selectem(ptab_layout, 1);
	}
	else if (emToolTipType == Tool_Right)
	{
		pTootipText = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_right_text")));
		pTootip = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_right")));
		selectem(ptab_layout, 2);
	}
	else if (emToolTipType == Tool_Bottom)
	{
		pTootipText = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_bottom_text")));
		pTootip = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_bottom")));
		selectem(ptab_layout, 3);
	}

	if (ptab_layout == nullptr || pTootipText == nullptr || pTootip == nullptr) {
		return;
	}

	pTootipText->SetText(strText);
	pTootipText->SetFixedWidth(m_pm.UnscaleValue(szText.cx));
	pTootipText->SetFixedHeight(m_pm.UnscaleValue(szText.cy));

	RECT rcPadding = { 0, 0, 0, 0 };
	if (emToolTipType == Tool_Left || emToolTipType == Tool_Right)
	{
		rcPadding.top = (szWindow.cy - m_pm.ScaleValue(16)) / 2;
		rcPadding = m_pm.UnscaleRect(rcPadding);
		pTootip->SetPadding(rcPadding);
	}
	else if (emToolTipType == Tool_Top || emToolTipType == Tool_Bottom)
	{
		rcPadding.left = (szWindow.cx - m_pm.ScaleValue(16)) / 2;
		rcPadding = m_pm.UnscaleRect(rcPadding);
		pTootip->SetPadding(rcPadding);
	}

	int nX = 0;
	int nY = 0;

	switch (emToolTipType)
	{
	case Tool_Top:
		nX = rcPos.left + (rcPos.right - rcPos.left - szWindow.cx) / 2 + szTooltipGap.cx;
		nY = rcPos.top - szWindow.cy - szTooltipGap.cy;
		break;
	case Tool_Bottom:
		nX = rcPos.left + (rcPos.right - rcPos.left - szWindow.cx) / 2 + szTooltipGap.cx;
		nY = rcPos.bottom + szTooltipGap.cy;
		break;
	case Tool_Left:
		nX = rcPos.left - szWindow.cx - szTooltipGap.cx;
		nY = rcPos.top + (rcPos.bottom - rcPos.top - szWindow.cy) / 2 + szTooltipGap.cy;
		break;
	case Tool_Right:
		nX = rcPos.right + szTooltipGap.cx;
		nY = rcPos.top + (rcPos.bottom - rcPos.top - szWindow.cy) / 2 + szTooltipGap.cy;
		break;
	default:
		nX = rcPos.left + (rcPos.right - rcPos.left - szWindow.cx) / 2;
		nY = rcPos.bottom + szTooltipGap.cy;
		break;
	}

	RECT rcWork = { 0 };
	MONITORINFO monitorInfo = {};
	monitorInfo.cbSize = sizeof(monitorInfo);
	if (::GetMonitorInfo(::MonitorFromPoint(ptAnchor, MONITOR_DEFAULTTONEAREST), &monitorInfo)) {
		rcWork = monitorInfo.rcWork;
	}
	else {
		::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
	}

	if (nX + szWindow.cx > rcWork.right) {
		nX = rcWork.right - szWindow.cx;
	}
	if (nX < rcWork.left) {
		nX = rcWork.left;
	}
	if (nY + szWindow.cy > rcWork.bottom) {
		nY = rcWork.bottom - szWindow.cy;
	}
	if (nY < rcWork.top) {
		nY = rcWork.top;
	}

	::SetWindowPos(m_hWnd, HWND_TOPMOST, nX, nY, szWindow.cx, szWindow.cy,
		SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER);
	ptab_layout->NeedUpdate();
}
