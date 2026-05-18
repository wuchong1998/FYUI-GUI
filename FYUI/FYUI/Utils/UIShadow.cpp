#include "pch.h"
#include "UIShadow.h"
#include "../Core/Render/UIRenderSurface.h"
#include "math.h"
#include "crtdbg.h"

namespace FYUI
{

	const wchar_t* strWndClassName = L"PerryShadowWnd";
	bool CShadowUI::s_bHasInit = FALSE;

	CShadowUI::CShadowUI(void)
		: m_hWnd((HWND)NULL)
		, m_pManager(NULL)
		, m_OriParentProc(NULL)
		, m_Status(0)
		, m_nDarkness(150)
		, m_nSharpness(5)
		, m_nSize(0)
		, m_nxOffset(0)
		, m_nyOffset(0)
		, m_nLogicalSharpness(5)
		, m_nLogicalSize(0)
		, m_nLogicalXOffset(0)
		, m_nLogicalYOffset(0)
		, m_nCornerRadius(0)
		, m_nLogicalCornerRadius(0)
		, m_uAppliedDpiGeneration(0)
		, m_Color(RGB(0, 0, 0))
		, m_WndSize(0)
		, m_bUpdate(false)
		, m_bInSizeMove(false)
		, m_bIsImageMode(false)
		, m_bIsShowShadow(false)
		, m_bIsDisableShadow(false)
		, m_nShadowCacheWidth(0)
		, m_nShadowCacheHeight(0)
		, m_nShadowCacheParentWidth(0)
		, m_nShadowCacheParentHeight(0)
		, m_nShadowCacheSize(0)
		, m_nShadowCacheSharpness(0)
		, m_nShadowCacheXOffset(0)
		, m_nShadowCacheYOffset(0)
		, m_nShadowCacheCornerRadius(0)
		, m_nShadowCacheDarkness(0)
		, m_ShadowCacheColor(0)
		, m_bShadowCacheValid(false)
		, m_bLayeredShadowPresented(false)
		, m_nScaledPreviewWidth(0)
		, m_nScaledPreviewHeight(0)
	{
		::ZeroMemory(&m_rcShadowCorner, sizeof(RECT));
		::ZeroMemory(&m_rcShadowCornerLogical, sizeof(RECT));
	}

	CShadowUI::~CShadowUI(void)
	{
		// 析构时把父窗口 WndProc 还原、影子窗口销毁、ShadowMap 解除，
		// 否则父窗口仍指向 ParentProc 但 CShadowUI 实例已失效，
		// 后续任何消息都会访问悬挂指针。
		if (m_OriParentProc != 0) {
			HWND hParentWnd = m_hWnd != NULL ? ::GetParent(m_hWnd) : NULL;
			if (hParentWnd == NULL && m_pManager != NULL) {
				hParentWnd = m_pManager->GetPaintWindow();
			}
			if (hParentWnd != NULL && ::IsWindow(hParentWnd)) {
#pragma warning(disable: 4311 4312)
				::SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, m_OriParentProc);
#pragma warning(default: 4311 4312)
				GetShadowMap().erase(hParentWnd);
			}
			m_OriParentProc = 0;
		}
		if (m_hWnd != NULL && ::IsWindow(m_hWnd)) {
			::DestroyWindow(m_hWnd);
		}
		m_hWnd = NULL;
	}

	int CShadowUI::ScaleMetric(int value) const
	{
		if (m_pManager != NULL) {
			return m_pManager->ScaleValue(value);
		}
		return value;
	}

	RECT CShadowUI::ScaleMetric(RECT rcValue) const
	{
		if (m_pManager != NULL) {
			return m_pManager->ScaleRect(rcValue);
		}
		return rcValue;
	}

	void CShadowUI::RefreshScaledMetrics()
	{
		const int oldSize = m_nSize;
		const int oldSharpness = m_nSharpness;
		const int oldXOffset = m_nxOffset;
		const int oldYOffset = m_nyOffset;
		const int oldCornerRadius = m_nCornerRadius;
		m_nSharpness = max(0, ScaleMetric(m_nLogicalSharpness));
		m_nSize = ScaleMetric(m_nLogicalSize);
		m_nxOffset = ScaleMetric(m_nLogicalXOffset);
		m_nyOffset = ScaleMetric(m_nLogicalYOffset);
		m_nCornerRadius = max(0, ScaleMetric(m_nLogicalCornerRadius));
		m_rcShadowCorner = ScaleMetric(m_rcShadowCornerLogical);
		m_uAppliedDpiGeneration = m_pManager != NULL ? m_pManager->GetDPIGeneration() : 0;
		if (oldSize != m_nSize || oldSharpness != m_nSharpness || oldXOffset != m_nxOffset ||
			oldYOffset != m_nyOffset || oldCornerRadius != m_nCornerRadius) {
			InvalidateShadowCache();
		}
	}

	void CShadowUI::InvalidateShadowCache()
	{
		m_bShadowCacheValid = false;
		m_bLayeredShadowPresented = false;
		m_nScaledPreviewWidth = 0;
		m_nScaledPreviewHeight = 0;
	}

	void CShadowUI::EnsureScaledMetricsUpToDate()
	{
		const ULONGLONG generation = m_pManager != NULL ? m_pManager->GetDPIGeneration() : 0;
		if (generation != m_uAppliedDpiGeneration) {
			RefreshScaledMetrics();
		}
	}

	bool CShadowUI::Initialize(HINSTANCE hInstance)
	{
		if (hInstance == NULL)
		{
			hInstance = GetModuleHandleW(NULL);
		}

		WNDCLASSEXW wc = { 0 };
		wc.cbSize = sizeof(wc);

		if (GetClassInfoExW(hInstance, strWndClassName, &wc))
		{
			s_bHasInit = true;
			return true;
		}

		WNDCLASSEXW wcex;
		memset(&wcex, 0, sizeof(wcex));

		wcex.cbSize = sizeof(WNDCLASSEXW);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = CShadowUI::ShadowProc;
		wcex.hInstance = hInstance;
		wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszClassName = strWndClassName;

		const ATOM atom = RegisterClassExW(&wcex);
		if (atom == 0)
		{
			const DWORD err = GetLastError();
			(void)err;
		
			if (err == ERROR_CLASS_ALREADY_EXISTS)
			{
				s_bHasInit = true;
				return true;
			}
			return false;
		}

		s_bHasInit = true;
		return true;
	}

	LRESULT CALLBACK CShadowUI::ShadowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_NCHITTEST:
			return HTTRANSPARENT;
		case WM_MOUSEACTIVATE:
			return MA_NOACTIVATE;
		}
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	void CShadowUI::Create(CPaintManagerUI* pPaintManager)
	{
		if (!m_bIsShowShadow)
			return;

		if (pPaintManager == NULL) {
			return;
		}
		// 重复 AttachDialog 时不要再次挂钩父窗口 WndProc，否则会形成
		// ParentProc -> ParentProc 的循环调用链。
		if (m_hWnd != NULL) {
			return;
		}
		m_pManager = pPaintManager;
		RefreshScaledMetrics();

		HWND hParentWnd = m_pManager->GetPaintWindow();
		if (hParentWnd == NULL) {
			return;
		}
		// 父窗口已经被其它 CShadowUI 占用时，先把旧映射解除，
		// 避免 ParentProc 拿到错误的 this。
		auto itShadow = GetShadowMap().find(hParentWnd);
		if (itShadow != GetShadowMap().end()) {
			GetShadowMap().erase(itShadow);
		}
		GetShadowMap()[hParentWnd] = this;

		LONG_PTR lParentStyle = GetWindowLongPtr(hParentWnd, GWL_STYLE);

		DWORD styleValue = static_cast<DWORD>(lParentStyle & WS_CAPTION);
		m_hWnd = CreateWindowExW(
			WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
			strWndClassName,
			NULL,
			/*WS_VISIBLE | */styleValue | WS_POPUPWINDOW,
			CW_USEDEFAULT, 0, 0, 0,
			hParentWnd,
			NULL,
			CPaintManagerUI::GetInstance(),
			NULL);

		if (m_hWnd == NULL)
		{
			const DWORD err = GetLastError();
			(void)err;
			return;
		}

#pragma warning(disable: 4311)
		SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, (LONG_PTR)ShadowProc);
#pragma warning(default: 4311)

		if (!(WS_VISIBLE & lParentStyle))
			m_Status = SS_ENABLED;
		else if ((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle)
			m_Status = SS_ENABLED | SS_PARENTVISIBLE;
		else
		{
			m_Status = SS_ENABLED | SS_VISABLE | SS_PARENTVISIBLE;
			m_bUpdate = true;
		}

		m_OriParentProc = GetWindowLongPtr(hParentWnd, GWLP_WNDPROC);

#pragma warning(disable: 4311)
		SetWindowLongPtr(hParentWnd, GWLP_WNDPROC, (LONG_PTR)ParentProc);
#pragma warning(default: 4311)
	}
	std::map<HWND, CShadowUI *>& CShadowUI::GetShadowMap()
	{
		static std::map<HWND, CShadowUI *> s_Shadowmap;
		return s_Shadowmap;
	}

	LRESULT CALLBACK CShadowUI::ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		_ASSERT(GetShadowMap().find(hwnd) != GetShadowMap().end());	// Shadow must have been attached

		CShadowUI *pThis = GetShadowMap()[hwnd];
		if (pThis->IsDisableShadow()) {
			return CallWindowProc((WNDPROC)pThis->m_OriParentProc, hwnd, uMsg, wParam, lParam);
		}
		pThis->EnsureScaledMetricsUpToDate();

		switch(uMsg)
		{
		case WM_ACTIVATEAPP:
		{
			if(!IsWindowEnabled(hwnd)) break;
			::SetWindowPos(pThis->m_hWnd, hwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
			break;
		}
		case WM_NCACTIVATE:
		{
			if(!IsWindowEnabled(hwnd)) break;
			::SetWindowPos(pThis->m_hWnd, hwnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREDRAW);
			break;
		}
		case WM_WINDOWPOSCHANGED:
		{
			RECT WndRect;
			LPWINDOWPOS pWndPos;
			pWndPos = (LPWINDOWPOS)lParam;
			GetWindowRect(hwnd, &WndRect);
			if (pThis->m_bIsImageMode) {
				SetWindowPos(pThis->m_hWnd, hwnd, WndRect.left - pThis->m_nSize, WndRect.top - pThis->m_nSize, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
			}
			else {
				SetWindowPos(pThis->m_hWnd, hwnd, WndRect.left + pThis->m_nxOffset - pThis->m_nSize, WndRect.top + pThis->m_nyOffset - pThis->m_nSize, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
			}

			if (pWndPos->flags & SWP_SHOWWINDOW) {
				if (pThis->m_Status & SS_ENABLED && !(pThis->m_Status & SS_PARENTVISIBLE))
				{
					pThis->m_bUpdate = true;
					::ShowWindow(pThis->m_hWnd, SW_SHOWNOACTIVATE);
					pThis->m_Status |= SS_VISABLE | SS_PARENTVISIBLE;
				}
			}
			else if (pWndPos->flags & SWP_HIDEWINDOW) {
				if (pThis->m_Status & SS_ENABLED)
				{
					::ShowWindow(pThis->m_hWnd, SW_HIDE);
					pThis->m_Status &= ~(SS_VISABLE | SS_PARENTVISIBLE);
				}
			}
			break;
		}

		case WM_MOVE:
			if(pThis->m_Status & SS_VISABLE) {
				RECT WndRect;
				GetWindowRect(hwnd, &WndRect);
				if (pThis->m_bIsImageMode) {
					SetWindowPos(pThis->m_hWnd, hwnd, WndRect.left - pThis->m_nSize, WndRect.top - pThis->m_nSize, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
				}
				else {
					SetWindowPos(pThis->m_hWnd, hwnd, WndRect.left + pThis->m_nxOffset - pThis->m_nSize, WndRect.top + pThis->m_nyOffset - pThis->m_nSize, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
				}
			}
			break;

		case WM_ENTERSIZEMOVE:
			pThis->m_bInSizeMove = true;
			break;

		case WM_SIZE:
			if(pThis->m_Status & SS_ENABLED) {
				if(SIZE_MAXIMIZED == wParam || SIZE_MINIMIZED == wParam) {
					::ShowWindow(pThis->m_hWnd, SW_HIDE);
					pThis->m_Status &= ~SS_VISABLE;
				}
				// Parent maybe resized even if invisible
				else if(pThis->m_Status & SS_PARENTVISIBLE) {
					pThis->m_bUpdate = true;
					if (pThis->m_bInSizeMove) {
						if (!pThis->PresentScaledCachedShadow(hwnd) && (pThis->m_Status & SS_VISABLE)) {
							::ShowWindow(pThis->m_hWnd, SW_HIDE);
							pThis->m_Status &= ~SS_VISABLE;
						}
					}
					if (!pThis->m_bInSizeMove && !(LOWORD(lParam) != LOWORD(pThis->m_WndSize) || HIWORD(lParam) != HIWORD(pThis->m_WndSize))) {
						pThis->Update(hwnd);
						pThis->m_bUpdate = false;
					}

					if(!pThis->m_bInSizeMove && !(pThis->m_Status & SS_VISABLE)) {
						::ShowWindow(pThis->m_hWnd, SW_SHOWNOACTIVATE);
						pThis->m_Status |= SS_VISABLE;
					}
				}
				pThis->m_WndSize = lParam;
			}
			break;

		case WM_PAINT:
		{
			if(pThis->m_bUpdate && !pThis->m_bInSizeMove) {
				pThis->Update(hwnd);
				pThis->m_bUpdate = false;
			}
			break;
		}

		// In some cases of sizing, the up-right corner of the parent window region would not be properly updated
		// Update() again when sizing is finished
		case WM_EXITSIZEMOVE:
			pThis->m_bInSizeMove = false;
			if(pThis->m_Status & SS_PARENTVISIBLE) {
				if(!(pThis->m_Status & SS_VISABLE)) {
					::ShowWindow(pThis->m_hWnd, SW_SHOWNOACTIVATE);
					pThis->m_Status |= SS_VISABLE;
				}
				pThis->Update(hwnd);
				pThis->m_bUpdate = false;
			}
			break;

		case WM_SHOWWINDOW:
			if(pThis->m_Status & SS_ENABLED) {
				// the window is being hidden
				if(!wParam) {
					::ShowWindow(pThis->m_hWnd, SW_HIDE);
					pThis->m_Status &= ~(SS_VISABLE | SS_PARENTVISIBLE);
				}
				else if(!(pThis->m_Status & SS_PARENTVISIBLE)) {
					//pThis->Update(hwnd);
					pThis->m_bUpdate = true;
					::ShowWindow(pThis->m_hWnd, SW_SHOWNOACTIVATE);
					pThis->m_Status |= SS_VISABLE | SS_PARENTVISIBLE;
				}
			}
			break;

		case WM_DESTROY:
			DestroyWindow(pThis->m_hWnd);	// Destroy the shadow
			break;
		case WM_NCDESTROY:
			GetShadowMap().erase(hwnd);	// Remove this window and shadow from the map
			break;
		}

		return CallWindowProc((WNDPROC)pThis->m_OriParentProc, hwnd, uMsg, wParam, lParam);
	}

	//Formats GetLastError() value.
	void GetLastErrorMessage() 
	{          
		LPVOID lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(wchar_t*)&lpMsgBuf, 0, NULL
		);

		// Display the string.
		//MessageBox(NULL, (const wchar_t*)lpMsgBuf, L"GetLastError", MB_OK | MB_ICONINFORMATION);

		// Free the buffer.
		LocalFree(lpMsgBuf);
	}

	void CShadowUI::Update(HWND hParent)
	{
		if(!m_bIsShowShadow || !(m_Status & SS_VISABLE)) return;
		EnsureScaledMetricsUpToDate();

		// 非图片模式：方案 C —— 模板 + 九宫格常态化。
		// 模板只在参数变化或首次时生成一次，后续任意尺寸/位置都仅做九宫格映射。
		if (!m_bIsImageMode) {
			if (m_nSize == 0) return;
			BuildShadowTemplate();
			PresentScaledCachedShadow(hParent);
			return;
		}

		// 图片模式：保留原有"按真实尺寸贴图"路径，因为图片本身不需要卷积，
		// 直接 DrawImage 已经很便宜，且九宫格逻辑已在 CRenderEngine::DrawImage 里。
		if (m_sShadowImage.empty()) return;

		RECT WndRect;
		GetWindowRect(hParent, &WndRect);
		const int nShadWndWid = WndRect.right - WndRect.left + m_nSize * 2;
		const int nShadWndHei = WndRect.bottom - WndRect.top + m_nSize * 2;
		if (nShadWndWid <= 0 || nShadWndHei <= 0) return;

		POINT ptDst;
		ptDst.x = WndRect.left - m_nSize;
		ptDst.y = WndRect.top - m_nSize;

		CPaintRenderSurface shadowSurface;
		if (!shadowSurface.Ensure(m_pManager, nShadWndWid, nShadWndHei)) {
			return;
		}
		if (!shadowSurface.ClearTransparent()) {
			return;
		}

		RECT rcPaint = {0, 0, nShadWndWid, nShadWndHei};
		const TDrawInfo* pDrawInfo = m_pManager->GetDrawInfo(m_sShadowImage, _T(""));
		if(pDrawInfo != NULL) {
			const TImageInfo* data = m_pManager->GetImageEx(pDrawInfo->sImageName, pDrawInfo->sResType, pDrawInfo->dwMask, pDrawInfo->bHSL, pDrawInfo->bGdiplus);
			if( !data ) return;
			RECT rcBmpPart = {0};
			rcBmpPart.right = data->nX;
			rcBmpPart.bottom = data->nY;
			RECT corner = m_rcShadowCorner;
			CPaintRenderContext renderContext = shadowSurface.CreateRenderContext(
				m_pManager,
				rcPaint,
				m_pManager->GetActiveRenderBackend(),
				m_pManager->GetActiveDirect2DRenderMode());
			CRenderEngine::DrawImage(renderContext, data->hBitmap, rcPaint, rcBmpPart, corner, data->bAlpha, 0xFF, true, false, false);
		}

		// UpdateLayeredWindow 内部就会原子地把 ptDst/size/bitmap 一并提交，
		// 在它之前再 MoveWindow 会先合成出一帧"新尺寸 + 旧位图"的中间状态，
		// 拖拽时表现为"主窗口右侧像被绘制到阴影区"。这里直接交给 PresentLayeredWindow。
		BOOL bRet = shadowSurface.PresentLayeredWindow(m_hWnd, ptDst);
		m_bLayeredShadowPresented = bRet != FALSE;
		_ASSERT(bRet); // something was wrong....
	}

	void CShadowUI::BuildShadowTemplate()
	{
		// 仅非图片模式生成模板。
		if (m_bIsImageMode) {
			return;
		}
		if (m_nSize == 0) {
			InvalidateShadowCache();
			return;
		}

		// 模板的"虚拟父矩形"必须能完整容纳一个圆角和高斯衰减带，
		// 让九宫格中央块永远落在"父窗口内部清空区"，从而中央拉伸不引入视觉差。
		const int nMargin = max(m_nCornerRadius, m_nSharpness);
		const int nTplParent = max(8, 2 * nMargin + 4);
		const int nTplW = nTplParent + 2 * m_nSize;
		const int nTplH = nTplParent + 2 * m_nSize;
		const size_t nPixelCount = static_cast<size_t>(nTplW) * static_cast<size_t>(nTplH);

		// 命中缓存就直接返回，避免重复跑一次卷积。
		if (m_bShadowCacheValid &&
			m_nShadowCacheWidth == nTplW &&
			m_nShadowCacheHeight == nTplH &&
			m_nShadowCacheParentWidth == nTplParent &&
			m_nShadowCacheParentHeight == nTplParent &&
			m_nShadowCacheSize == m_nSize &&
			m_nShadowCacheSharpness == m_nSharpness &&
			m_nShadowCacheXOffset == m_nxOffset &&
			m_nShadowCacheYOffset == m_nyOffset &&
			m_nShadowCacheCornerRadius == m_nCornerRadius &&
			m_nShadowCacheDarkness == m_nDarkness &&
			m_ShadowCacheColor == m_Color &&
			m_ShadowCacheBits.size() == nPixelCount) {
			return;
		}

		m_ShadowCacheBits.assign(nPixelCount, 0);
		RECT rcTpl = { 0, 0, nTplParent, nTplParent };
		MakeShadow(&m_ShadowCacheBits[0], NULL, &rcTpl);

		m_nShadowCacheWidth = nTplW;
		m_nShadowCacheHeight = nTplH;
		m_nShadowCacheParentWidth = nTplParent;
		m_nShadowCacheParentHeight = nTplParent;
		m_nShadowCacheSize = m_nSize;
		m_nShadowCacheSharpness = m_nSharpness;
		m_nShadowCacheXOffset = m_nxOffset;
		m_nShadowCacheYOffset = m_nyOffset;
		m_nShadowCacheCornerRadius = m_nCornerRadius;
		m_nShadowCacheDarkness = m_nDarkness;
		m_ShadowCacheColor = m_Color;
		m_bShadowCacheValid = true;
		m_bLayeredShadowPresented = false;
		m_nScaledPreviewWidth = 0;
		m_nScaledPreviewHeight = 0;
	}

	bool CShadowUI::PresentScaledCachedShadow(HWND hParent)
	{
		if (m_bIsImageMode || !m_bShadowCacheValid || m_ShadowCacheBits.empty() ||
			m_nShadowCacheWidth <= 0 || m_nShadowCacheHeight <= 0 ||
			m_nShadowCacheParentWidth <= 0 || m_nShadowCacheParentHeight <= 0) {
			return false;
		}
		EnsureScaledMetricsUpToDate();
		RECT WndRect;
		GetWindowRect(hParent, &WndRect);
		const int nParentW = WndRect.right - WndRect.left;
		const int nParentH = WndRect.bottom - WndRect.top;
		const int nShadWndWid = nParentW + m_nSize * 2;
		const int nShadWndHei = nParentH + m_nSize * 2;
		if (nParentW <= 0 || nParentH <= 0 || nShadWndWid <= 0 || nShadWndHei <= 0) {
			return false;
		}

		POINT ptDst;
		ptDst.x = WndRect.left + m_nxOffset - m_nSize;
		ptDst.y = WndRect.top + m_nyOffset - m_nSize;
		if (m_bLayeredShadowPresented &&
			m_nScaledPreviewWidth == nShadWndWid &&
			m_nScaledPreviewHeight == nShadWndHei) {
			SetWindowPos(m_hWnd, hParent, ptDst.x, ptDst.y, nShadWndWid, nShadWndHei, SWP_NOACTIVATE | SWP_NOREDRAW);
			return true;
		}

		CPaintRenderSurface shadowSurface;
		if (!shadowSurface.Ensure(m_pManager, nShadWndWid, nShadWndHei)) {
			return false;
		}
		UINT32* pShadowBits = reinterpret_cast<UINT32*>(shadowSurface.GetBits());
		if (pShadowBits == nullptr) {
			return false;
		}

		StretchShadowCache(pShadowBits, nShadWndWid, nShadWndHei, nParentW, nParentH);

		// 必须先 UpdateLayeredWindow（原子提交 ptDst/size/bitmap），再 ShowWindow，
		// 否则首次显示瞬间会出现"窗口已可见但位图还没刷"的空白帧，
		// 拖拽 resize 时会被持续放大成"主窗口右侧绘制到阴影区"的视觉错位。
		BOOL bRet = shadowSurface.PresentLayeredWindow(m_hWnd, ptDst);
		m_bLayeredShadowPresented = bRet != FALSE;
		if (bRet != FALSE) {
			m_nScaledPreviewWidth = nShadWndWid;
			m_nScaledPreviewHeight = nShadWndHei;
			if (!(m_Status & SS_VISABLE)) {
				::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
				m_Status |= SS_VISABLE;
			}
		}
		return bRet != FALSE;
	}

	void CShadowUI::StretchShadowCache(UINT32* pDstBits, int nDstWidth, int nDstHeight, int nDstParentWidth, int nDstParentHeight)
	{
		if (pDstBits == nullptr) {
			return;
		}
		const int nSrcWidth = m_nShadowCacheWidth;
		const int nSrcHeight = m_nShadowCacheHeight;
		const int nSrcParentWidth = m_nShadowCacheParentWidth;
		const int nSrcParentHeight = m_nShadowCacheParentHeight;
		const int nSrcLeft = max(1, min(m_nShadowCacheSize, nSrcWidth / 2));
		const int nSrcRight = max(1, min(m_nShadowCacheSize, nSrcWidth / 2));
		const int nSrcTop = max(1, min(m_nShadowCacheSize, nSrcHeight / 2));
		const int nSrcBottom = max(1, min(m_nShadowCacheSize, nSrcHeight / 2));
		const int nDstLeft = max(1, min(m_nSize, nDstWidth / 2));
		const int nDstRight = max(1, min(m_nSize, nDstWidth / 2));
		const int nDstTop = max(1, min(m_nSize, nDstHeight / 2));
		const int nDstBottom = max(1, min(m_nSize, nDstHeight / 2));
		const int nSrcCenterW = max(1, nSrcParentWidth);
		const int nSrcCenterH = max(1, nSrcParentHeight);
		const int nDstCenterW = max(1, nDstParentWidth);
		const int nDstCenterH = max(1, nDstParentHeight);

		for (int y = 0; y < nDstHeight; ++y) {
			int nSrcY = 0;
			if (y < nDstTop) {
				nSrcY = y;
			}
			else if (y >= nDstHeight - nDstBottom) {
				nSrcY = nSrcHeight - (nDstHeight - y);
			}
			else {
				nSrcY = nSrcTop + (int)(((long long)(y - nDstTop) * nSrcCenterH) / nDstCenterH);
			}
			nSrcY = max(0, min(nSrcY, nSrcHeight - 1));
			for (int x = 0; x < nDstWidth; ++x) {
				int nSrcX = 0;
				if (x < nDstLeft) {
					nSrcX = x;
				}
				else if (x >= nDstWidth - nDstRight) {
					nSrcX = nSrcWidth - (nDstWidth - x);
				}
				else {
					nSrcX = nSrcLeft + (int)(((long long)(x - nDstLeft) * nSrcCenterW) / nDstCenterW);
				}
				nSrcX = max(0, min(nSrcX, nSrcWidth - 1));
				pDstBits[y * nDstWidth + x] = m_ShadowCacheBits[nSrcY * nSrcWidth + nSrcX];
			}
		}
	}

	void CShadowUI::MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent)
	{
		const int nParentW = abs(rcParent->right - rcParent->left);
		const int nParentH = abs(rcParent->bottom - rcParent->top);
		SIZE szParent = { nParentW, nParentH };
		SIZE szShadow = { szParent.cx + 2 * m_nSize, szParent.cy + 2 * m_nSize };
		if (szParent.cx <= 0 || szParent.cy <= 0 || szShadow.cx <= 0 || szShadow.cy <= 0) {
			return;
		}

		const int nParentX = m_nSize - m_nxOffset;
		const int nParentY = m_nSize - m_nyOffset;
		const int nPixelCount = szShadow.cx * szShadow.cy;
		unsigned char* pMask = new unsigned char[nPixelCount];
		unsigned char* pTmp = new unsigned char[nPixelCount];
		::ZeroMemory(pMask, nPixelCount);
		::ZeroMemory(pTmp, nPixelCount);

		if (m_nCornerRadius > 0) {
			const int nMaxR = (min)(szParent.cx, szParent.cy) / 2;
			const int nR = (min)(m_nCornerRadius, nMaxR);
			const int nRSq = nR * nR;
			for (int y = 0; y < szParent.cy; ++y) {
				const int nDstY = nParentY + y;
				if (nDstY < 0 || nDstY >= szShadow.cy) {
					continue;
				}
				unsigned char* pLine = pMask + nDstY * szShadow.cx;
				for (int x = 0; x < szParent.cx; ++x) {
					const int nDstX = nParentX + x;
					if (nDstX < 0 || nDstX >= szShadow.cx) {
						continue;
					}
					bool bInside = true;
					if (x < nR && y < nR) {
						const int dx = x - nR;
						const int dy = y - nR;
						bInside = dx * dx + dy * dy <= nRSq;
					}
					else if (x >= szParent.cx - nR && y < nR) {
						const int dx = x - (szParent.cx - nR - 1);
						const int dy = y - nR;
						bInside = dx * dx + dy * dy <= nRSq;
					}
					else if (x >= szParent.cx - nR && y >= szParent.cy - nR) {
						const int dx = x - (szParent.cx - nR - 1);
						const int dy = y - (szParent.cy - nR - 1);
						bInside = dx * dx + dy * dy <= nRSq;
					}
					else if (x < nR && y >= szParent.cy - nR) {
						const int dx = x - nR;
						const int dy = y - (szParent.cy - nR - 1);
						bInside = dx * dx + dy * dy <= nRSq;
					}
					if (bInside) {
						pLine[nDstX] = 255;
					}
				}
			}
		}
		else {
			// hParent 为 NULL 时（模板生成路径）按纯矩形填充，不依赖真实窗口形状。
			// 真实窗口路径仍尝试 GetWindowRgn 拿到自定义形状；失败时降级为矩形。
			HRGN hParentRgn = NULL;
			if (hParent != NULL) {
				hParentRgn = CreateRectRgn(0, 0, nParentW, nParentH);
				if (hParentRgn != NULL) {
					if (GetWindowRgn(hParent, hParentRgn) == ERROR) {
						DeleteObject(hParentRgn);
						hParentRgn = NULL;
					}
				}
			}
			for (int y = 0; y < szParent.cy; ++y) {
				const int nDstY = nParentY + y;
				if (nDstY < 0 || nDstY >= szShadow.cy) {
					continue;
				}
				unsigned char* pLine = pMask + nDstY * szShadow.cx;
				for (int x = 0; x < szParent.cx; ++x) {
					const int nDstX = nParentX + x;
					if (nDstX < 0 || nDstX >= szShadow.cx) {
						continue;
					}
					const bool bInside = (hParentRgn == NULL) || PtInRegion(hParentRgn, x, y);
					if (bInside) {
						pLine[nDstX] = 255;
					}
				}
			}
			if (hParentRgn != NULL) {
				DeleteObject(hParentRgn);
			}
		}

		const int nRadius = max(1, max(m_nSize, m_nSharpness));
		const double dSigma = max(1.0, (double)m_nSharpness * 0.50);
		int* pKernel = new int[nRadius * 2 + 1];
		double dSum = 0.0;
		for (int i = -nRadius; i <= nRadius; ++i) {
			dSum += exp(-(double)(i * i) / (2.0 * dSigma * dSigma));
		}
		int nKernelSum = 0;
		const int nKernelScale = 4096;
		for (int i = -nRadius; i <= nRadius; ++i) {
			const double dValue = exp(-(double)(i * i) / (2.0 * dSigma * dSigma));
			const int nWeight = (int)(dValue * (double)nKernelScale / dSum + 0.5);
			pKernel[i + nRadius] = nWeight;
			nKernelSum += nWeight;
		}
		if (nKernelSum <= 0) {
			pKernel[nRadius] = 1;
			nKernelSum = 1;
		}

		for (int y = 0; y < szShadow.cy; ++y) {
			const int nLineOffset = y * szShadow.cx;
			for (int x = 0; x < szShadow.cx; ++x) {
				int nAlpha = 0;
				for (int k = -nRadius; k <= nRadius; ++k) {
					const int nSampleX = x + k;
					if (nSampleX >= 0 && nSampleX < szShadow.cx) {
						nAlpha += (int)pMask[nLineOffset + nSampleX] * pKernel[k + nRadius];
					}
				}
				pTmp[nLineOffset + x] = (unsigned char)((nAlpha + nKernelSum / 2) / nKernelSum);
			}
		}

		const BYTE nColorAlpha = (BYTE)((m_Color >> 24) & 0xFF);
		const unsigned int nMaxAlpha = nColorAlpha > 0 ? ((unsigned int)m_nDarkness * nColorAlpha + 127) / 255 : m_nDarkness;
		for (int y = 0; y < szShadow.cy; ++y) {
			for (int x = 0; x < szShadow.cx; ++x) {
				const int nIndex = y * szShadow.cx + x;
				int nBlurAlpha = 0;
				for (int k = -nRadius; k <= nRadius; ++k) {
					const int nSampleY = y + k;
					if (nSampleY >= 0 && nSampleY < szShadow.cy) {
						nBlurAlpha += (int)pTmp[nSampleY * szShadow.cx + x] * pKernel[k + nRadius];
					}
				}
				unsigned int nAlpha = ((unsigned int)((nBlurAlpha + nKernelSum / 2) / nKernelSum) * nMaxAlpha + 127) / 255;
				if (pMask[nIndex] != 0) {
					nAlpha = 0;
				}
				if (nAlpha > 255) {
					nAlpha = 255;
				}
				pShadBits[(szShadow.cy - y - 1) * szShadow.cx + x] = (nAlpha << 24) | PreMultiply(m_Color, (unsigned char)nAlpha);
			}
		}

		delete[] pKernel;
		delete[] pTmp;
		delete[] pMask;
	}

	void CShadowUI::ShowShadow(bool bShow)
	{
		m_bIsShowShadow = bShow;
	}

	bool CShadowUI::IsShowShadow() const
	{
		return m_bIsShowShadow;
	}


	void CShadowUI::DisableShadow(bool bDisable)
	{
		m_bIsDisableShadow = bDisable;
		if (m_hWnd != NULL) {
			if (m_bIsDisableShadow) {
				::ShowWindow(m_hWnd, SW_HIDE);
			}
			else {
				// Determine the initial show state of shadow according to parent window's state
				LONG_PTR lParentStyle = GetWindowLongPtr(GetParent(m_hWnd), GWL_STYLE);

				if (!(WS_VISIBLE & lParentStyle))	// Parent invisible
					m_Status = SS_ENABLED;
				else if ((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle)	// Parent visible but does not need shadow
					m_Status = SS_ENABLED | SS_PARENTVISIBLE;
				else {
					m_Status = SS_ENABLED | SS_VISABLE | SS_PARENTVISIBLE;
				}

				if ((WS_VISIBLE & lParentStyle) && !((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle)) {
					::ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);
					Update(GetParent(m_hWnd));
				}
			}
		}
	}

	////TODO shadow disnable fix////
	bool CShadowUI::IsDisableShadow() const 
	{
		return m_bIsDisableShadow;
	}

	bool CShadowUI::SetSize(int NewSize)
	{
		if(NewSize > 35 || NewSize < -35)
			return false;

		m_nLogicalSize = NewSize;
		RefreshScaledMetrics();
		if(m_hWnd != NULL && (SS_VISABLE & m_Status))
			Update(GetParent(m_hWnd));
		return true;
	}

	bool CShadowUI::SetSharpness(unsigned int NewSharpness)
	{
		if(NewSharpness > 35)
			return false;

		m_nLogicalSharpness = static_cast<int>(NewSharpness);
		RefreshScaledMetrics();
		if(m_hWnd != NULL && (SS_VISABLE & m_Status))
			Update(GetParent(m_hWnd));
		return true;
	}

	bool CShadowUI::SetDarkness(unsigned int NewDarkness)
	{
		if(NewDarkness > 255)
			return false;

		m_nDarkness = (unsigned char)NewDarkness;
		InvalidateShadowCache();
		if(m_hWnd != NULL && (SS_VISABLE & m_Status))
			Update(GetParent(m_hWnd));
		return true;
	}

	bool CShadowUI::SetPosition(int NewXOffset, int NewYOffset)
	{
		if(NewXOffset > 35 || NewXOffset < -35 ||
			NewYOffset > 35 || NewYOffset < -35)
			return false;

		m_nLogicalXOffset = NewXOffset;
		m_nLogicalYOffset = NewYOffset;
		RefreshScaledMetrics();
		if(m_hWnd != NULL && (SS_VISABLE & m_Status))
			Update(GetParent(m_hWnd));

		return true;
	}

	bool CShadowUI::SetColor(COLORREF NewColor)
	{
		m_Color = NewColor;
		InvalidateShadowCache();
		if(m_hWnd != NULL && (SS_VISABLE & m_Status))
			Update(GetParent(m_hWnd));

		return true;
	}

	bool CShadowUI::SetImage(std::wstring_view szImage)
	{
		m_bIsImageMode = true;
		m_sShadowImage = szImage;
		InvalidateShadowCache();
		if(m_hWnd != NULL && (SS_VISABLE & m_Status))
			Update(GetParent(m_hWnd));

		return true;
	}

	bool CShadowUI::SetShadowCorner(RECT rcCorner)
	{
		if (rcCorner.left < 0 || rcCorner.top < 0 || rcCorner.right < 0 || rcCorner.bottom < 0) return false;

		m_rcShadowCornerLogical = rcCorner;
		RefreshScaledMetrics();
		if(m_hWnd != NULL && (SS_VISABLE & m_Status)) {
			Update(GetParent(m_hWnd));
		}

		return true;
	}

	bool CShadowUI::SetCornerRadius(int nRadius)
	{
		if (nRadius < 0) {
			return false;
		}
		m_nLogicalCornerRadius = nRadius;
		RefreshScaledMetrics();
		if (m_hWnd != NULL && (SS_VISABLE & m_Status)) {
			Update(GetParent(m_hWnd));
		}
		return true;
	}

	bool CShadowUI::CopyShadow(CShadowUI* pShadow)
	{
		if (m_bIsImageMode) {
			pShadow->SetImage(m_sShadowImage);
			pShadow->SetShadowCorner(m_rcShadowCornerLogical);
			pShadow->SetSize(m_nLogicalSize);
		}
		else {
			pShadow->SetSize(m_nLogicalSize);
			pShadow->SetSharpness((unsigned int)m_nLogicalSharpness);
			pShadow->SetDarkness((unsigned int)m_nDarkness);
			pShadow->SetColor(m_Color);
			pShadow->SetPosition(m_nLogicalXOffset, m_nLogicalYOffset);
			pShadow->SetCornerRadius(m_nLogicalCornerRadius);
		}

		pShadow->DisableShadow(m_bIsDisableShadow);
		pShadow->ShowShadow(m_bIsShowShadow);
		return true;
	}
} //namespace DuiLib

