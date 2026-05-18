#include "pch.h"
#include "TooltipWnd.h"
#include "Core/UIRender.h"
#include <cmath>

namespace
{
	// 气泡视觉常量（与原 tooltip\*.svg 完全对齐）
	constexpr DWORD kTooltipBubbleColor = 0xFF181818; // 黑色模式：面板与箭头颜色
	constexpr DWORD kTooltipBubbleTextColor = 0xFFFFFFFF; // 黑色模式：文字颜色
	constexpr DWORD kTooltipWhiteBubbleColor = 0xFFFFFFFF; // 白色模式：面板与箭头颜色
	constexpr DWORD kTooltipWhiteBubbleTextColor = 0xFF181818; // 白色模式：文字颜色
	constexpr DWORD kTooltipShadowColor = 0x3c181818; // 白色模式：外阴影颜色（半透明黑）
	constexpr int   kTooltipShadowMargin = 4;         // 白色模式：阴影占用的边距（逻辑像素）
	constexpr int   kTooltipArrowDepth  = 7;          // 箭头突出方向上的厚度（与 svg 宽/高一致）
	constexpr int   kTooltipArrowHalf   = 8;          // 箭头底边一半 (svg 底边 16 / 2)
	constexpr int   kTooltipCornerRadius = 8;         // 面板圆角半径（视觉上与原 svg 接近）
	constexpr int   kTooltipMeasureFontIdBase = 900000;

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

		const int iTooltipFontId = kTooltipMeasureFontIdBase + static_cast<int>(paintManager.GetDPI()) * 100 + nFontSize;
		paintManager.AddFont(iTooltipFontId, fontName, nFontSize, false, false, false, false);
		return iTooltipFontId;
	}

	SIZE CalculateTooltipTextSize(CPaintManagerUI& paintManager, std::wstring_view text, int maxWidth, int nFontSize)
	{
		SIZE size = { 0, 0 };
		if (text.empty())
			return size;

		if (maxWidth <= 0)
			maxWidth = 1;

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
	m_pm.AddFont(17555, L"微软雅黑", 12, false, false, false, false);

	// 拿到内联 XML 中唯一的 Label，挂载背景绘制回调（圆角矩形 + 三角箭头）。
	CLabelUI* pLabel = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_text")));
	if (pLabel != nullptr)
	{
		pLabel->SetPaintCallback(PaintStageBkColor,[this](CControlUI* sender, CPaintRenderContext& ctx) 
		{
			DrawTooltipBubble(sender, ctx);
		},{});
	}
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
				pTooltipInfo->szTooltipGap, pTooltipInfo->nMaxWidth, pTooltipInfo->emToolTipMode);
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
	// 返回内联 XML（首字符 '<'，UIResourceData::LoadMarkupDocument 会直接走 xml.Load 分支，
	// 不再读取外部 tooltip\tooltip.xml，从而摆脱对 4 张 svg 的依赖。
	// 气泡背景与箭头由 InitWindow 中挂载的 PaintStageBkColor 回调绘制。
	return _T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>")
		_T("<Window size=\"100,100\" bktrans=\"true\">")
		_T("<Label name=\"tooltip_text\" align=\"center\" valign=\"vcenter\" font=\"17555\"")
		_T(" textcolor=\"0xffffffff\" mouse=\"false\" wordbreak=\"true\"")
		_T(" textpadding=\"5,5,5,5\"/>")
		_T("</Window>");
}

std::wstring_view TooltipWnd::GetWindowClassName(void) const
{
	return _T("TooltipWnd");
}

void TooltipWnd::SetTooltipInfo(const std::wstring& strText, RECT rcPos, ToolTipType emToolTipType, SIZE szTooltipGap, int nMaxWidth,
	ToolTipMode emToolTipMode)
{
	POINT ptAnchor = { rcPos.left + (rcPos.right - rcPos.left) / 2, rcPos.top + (rcPos.bottom - rcPos.top) / 2 };

	if (nMaxWidth < m_pm.ScaleValue(20)) {
		nMaxWidth = 9999;
	}

	// 文本面板尺寸（仅文字 + 内边距，不含箭头突出部分），与旧版保持一致。
	SIZE szText = CalculateTooltipTextSize(m_pm, strText, nMaxWidth, 13);
	if (szText.cx < m_pm.ScaleValue(30)) {
		szText.cx = m_pm.ScaleValue(30);
	}
	szText.cx += m_pm.ScaleValue(10);
	if (szText.cy < m_pm.ScaleValue(23)) {
		szText.cy = m_pm.ScaleValue(23);
	}
	szText.cy += m_pm.ScaleValue(10);

	// 整窗口尺寸 = 文本面板 + 箭头突出方向上的 7px；白色模式额外四面预留 4px 阴影空间。
	const int nArrowDepth = m_pm.ScaleValue(kTooltipArrowDepth);
	const int nShadowMargin = (emToolTipMode == WhiteBubbles) ? m_pm.ScaleValue(kTooltipShadowMargin) : 0;
	SIZE szWindow = { 0, 0 };
	if (emToolTipType == Tool_Top || emToolTipType == Tool_Bottom) 
	{
		szWindow.cx = szText.cx + nShadowMargin * 2;
		szWindow.cy = szText.cy + nArrowDepth + nShadowMargin * 2;
	}
	else 
	{ // Tool_Left / Tool_Right
		szWindow.cx = szText.cx + nArrowDepth + nShadowMargin * 2;
		szWindow.cy = szText.cy + nShadowMargin * 2;
	}

	// 保存箭头方向与气泡模式，供 PaintBkColor 回调使用。
	m_emToolTipType = emToolTipType;
	m_emToolTipMode = emToolTipMode;

	CLabelUI* pTooltipText = static_cast<CLabelUI*>(m_pm.FindControl(_T("tooltip_text")));
	if (pTooltipText == nullptr) {
		return;
	}

	// Label 作为 PaintManager 的根控件，会在 WM_SIZE / Layout 阶段被 m_pRoot->SetPos
	// 强制铺满 client，因此无需 SetFixedWidth/Height；通过 textpadding 把文字挤到
	// 面板内部，让出箭头侧的 7px（白色模式再加 4px 阴影边距）。
	pTooltipText->SetText(strText);
	pTooltipText->SetTextColor(emToolTipMode == WhiteBubbles ? kTooltipWhiteBubbleTextColor : kTooltipBubbleTextColor);

	const int kBaseInset = 5; // 与原 xml 中 textpadding="5,5,5,5" 一致的逻辑像素
	RECT rcTextPadding = { kBaseInset, kBaseInset, kBaseInset, kBaseInset };
	if (emToolTipMode == WhiteBubbles) {
		rcTextPadding.left   += kTooltipShadowMargin;
		rcTextPadding.top    += kTooltipShadowMargin;
		rcTextPadding.right  += kTooltipShadowMargin;
		rcTextPadding.bottom += kTooltipShadowMargin;
	}
	switch (emToolTipType) {
	case Tool_Bottom: // 提示在锚点下方 → 箭头位于面板顶部
		rcTextPadding.top += kTooltipArrowDepth;
		break;
	case Tool_Top:    // 提示在锚点上方 → 箭头位于面板底部
		rcTextPadding.bottom += kTooltipArrowDepth;
		break;
	case Tool_Right:  // 提示在锚点右侧 → 箭头位于面板左侧
		rcTextPadding.left += kTooltipArrowDepth;
		break;
	case Tool_Left:   // 提示在锚点左侧 → 箭头位于面板右侧
		rcTextPadding.right += kTooltipArrowDepth;
		break;
	default:
		break;
	}
	pTooltipText->SetTextPadding(rcTextPadding);

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
	pTooltipText->NeedUpdate();
}

void TooltipWnd::DrawTooltipBubble(CControlUI* sender, CPaintRenderContext& ctx)
{
	if (sender == nullptr)
		return;

	const RECT rcCtrl = sender->GetPos();
	if (rcCtrl.right <= rcCtrl.left || rcCtrl.bottom <= rcCtrl.top) 
		return;

	const int nArrowDepth   = m_pm.ScaleValue(kTooltipArrowDepth);
	const int nArrowHalf    = m_pm.ScaleValue(kTooltipArrowHalf);
	const int nRadius       = m_pm.ScaleValue(kTooltipCornerRadius);
	const int nShadowMargin = (m_emToolTipMode == WhiteBubbles) ? m_pm.ScaleValue(kTooltipShadowMargin) : 0;

	// 白色模式下，气泡本体向内收缩 nShadowMargin，让出四周阴影空间。
	RECT rcBubble = rcCtrl;
	if (nShadowMargin > 0) {
		::InflateRect(&rcBubble, -nShadowMargin, -nShadowMargin);
	}

	// 计算面板矩形（让出箭头侧的 7px）与三角形 3 个顶点（顶点指向锚点）。
	RECT rcPanel = rcBubble;
	POINT pts[3] = {};

	switch (m_emToolTipType) 
	{
	case Tool_Bottom: 
	{ // 提示在锚点下方，箭头位于面板顶部、向上指
		rcPanel.top += nArrowDepth;
		const LONG cx = (rcPanel.left + rcPanel.right) / 2;
		pts[0] = { cx - nArrowHalf, rcPanel.top };
		pts[1] = { cx + nArrowHalf, rcPanel.top };
		pts[2] = { cx,              rcBubble.top };
		break;
	}
	case Tool_Top:
	{ // 提示在锚点上方，箭头位于面板底部、向下指
		rcPanel.bottom -= nArrowDepth;
		const LONG cx = (rcPanel.left + rcPanel.right) / 2;
		pts[0] = { cx - nArrowHalf, rcPanel.bottom };
		pts[1] = { cx + nArrowHalf, rcPanel.bottom };
		pts[2] = { cx,              rcBubble.bottom };
		break;
	}
	case Tool_Right:
	{ // 提示在锚点右侧，箭头位于面板左侧、向左指
		rcPanel.left += nArrowDepth;
		const LONG cy = (rcPanel.top + rcPanel.bottom) / 2;
		pts[0] = { rcPanel.left, cy - nArrowHalf };
		pts[1] = { rcPanel.left, cy + nArrowHalf };
		pts[2] = { rcBubble.left, cy };
		break;
	}
	case Tool_Left:
	{ // 提示在锚点左侧，箭头位于面板右侧、向右指
		rcPanel.right -= nArrowDepth;
		const LONG cy = (rcPanel.top + rcPanel.bottom) / 2;
		pts[0] = { rcPanel.right, cy - nArrowHalf };
		pts[1] = { rcPanel.right, cy + nArrowHalf };
		pts[2] = { rcBubble.right, cy };
		break;
	}
	default:
		break;
	}

	if (m_emToolTipMode == WhiteBubbles)
	{
		// 1) 面板矩形：先用 DrawShadow 在外圈绘制渐变阴影，避免在纯白背景中融为一体。
		CRenderEngine::DrawShadow(ctx, rcPanel, nRadius, nRadius, kTooltipShadowColor, kTooltipShadowMargin);

		// 2) 三角箭头：DrawShadow 仅作用于矩形，需要手动给三角形外扩一圈半透明黑作"硬阴影"。
		POINT ptsShadow[3] = {};
		LONG cx = 0, cy = 0;
		for (int i = 0; i < 3; ++i) { cx += pts[i].x; cy += pts[i].y; }
		cx /= 3; cy /= 3;
		for (int i = 0; i < 3; ++i) {
			const double dx = pts[i].x - cx;
			const double dy = pts[i].y - cy;
			const double len = std::sqrt(dx * dx + dy * dy);
			if (len < 1e-6) { ptsShadow[i] = pts[i]; continue; }
			ptsShadow[i].x = static_cast<LONG>(pts[i].x + dx / len * nShadowMargin);
			ptsShadow[i].y = static_cast<LONG>(pts[i].y + dy / len * nShadowMargin);
		}
		CRenderEngine::FillPolygon(ctx, ptsShadow, 3, kTooltipShadowColor);

		// 3) 主气泡：白色面板 + 白色三角形覆盖在阴影之上。
		CRenderEngine::DrawRoundColor(ctx, rcPanel, nRadius, nRadius, kTooltipWhiteBubbleColor);
		CRenderEngine::FillPolygon(ctx, pts, 3, kTooltipWhiteBubbleColor);
	}
	else {
		// 黑色气泡：保持原 svg 视觉，直接画黑色面板 + 黑色三角。
		CRenderEngine::DrawRoundColor(ctx, rcPanel, nRadius, nRadius, kTooltipBubbleColor);
		CRenderEngine::FillPolygon(ctx, pts, 3, kTooltipBubbleColor);
	}
}
