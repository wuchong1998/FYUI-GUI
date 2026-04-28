#include "pch.h"
#include "UIColorPalette.h"

namespace FYUI {
#define HSLMAX   255
#define RGBMAX   255
#define HSLUNDEFINED (HSLMAX*2/3)

#define BASE_PALETTE_BUF_W 200
#define BASE_PALETTE_BUF_H 200
#define BAR_GAP 1
#define BASE_HUE_BAR_W 360
#define BASE_BAR_BUF_H 30

	static int ClampColorComponent(int value, int minValue, int maxValue)
	{
		return min(max(value, minValue), maxValue);
	}

	static DWORD HSVToRGB(float h, float s, float v)
	{
		if (s == 0.0f) {
			BYTE gray = static_cast<BYTE>(v * 255.0f);
			return RGB(gray, gray, gray);
		}

		float hh = h;
		if (hh >= 360.0f) hh = 0.0f;
		hh /= 60.0f;

		int i = static_cast<int>(hh);
		float f = hh - i;
		float p = v * (1.0f - s);
		float q = v * (1.0f - s * f);
		float t = v * (1.0f - s * (1.0f - f));

		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		switch (i) {
		case 0: r = v; g = t; b = p; break;
		case 1: r = q; g = v; b = p; break;
		case 2: r = p; g = v; b = t; break;
		case 3: r = p; g = q; b = v; break;
		case 4: r = t; g = p; b = v; break;
		default: r = v; g = p; b = q; break;
		}

		return RGB(
			static_cast<BYTE>(r * 255.0f),
			static_cast<BYTE>(g * 255.0f),
			static_cast<BYTE>(b * 255.0f));
	}

	static void RGBToHSV(DWORD clr, float* pHue, float* pSat, float* pVal)
	{
		float r = GetRValue(clr) / 255.0f;
		float g = GetGValue(clr) / 255.0f;
		float b = GetBValue(clr) / 255.0f;

		float fMax = max(r, max(g, b));
		float fMin = min(r, min(g, b));
		float delta = fMax - fMin;

		*pVal = fMax;
		*pSat = fMax != 0.0f ? delta / fMax : 0.0f;

		if (*pSat == 0.0f) {
			*pHue = 0.0f;
			return;
		}

		if (r == fMax) {
			*pHue = (g - b) / delta;
		}
		else if (g == fMax) {
			*pHue = 2.0f + (b - r) / delta;
		}
		else {
			*pHue = 4.0f + (r - g) / delta;
		}
		*pHue /= 6.0f;
		if (*pHue < 0.0f) *pHue += 1.0f;
	}

#define _HSVToRGB(h,s,v) HSVToRGB(static_cast<float>(h), static_cast<float>(s) / 200.0f, static_cast<float>(v) / 200.0f)

	IMPLEMENT_DUICONTROL(CColorPaletteUI)

	CColorPaletteUI::CColorPaletteUI()
		: m_uButtonState(0)
		, m_activeHandleHueBar(false)
		, m_bIsInBar(false)
		, m_bPaletteDataDirty(true)
		, m_bBarDataDirty(true)
		, m_bIsInPallet(false)
		, m_bSetColor(false)
		, m_nCurH(0)
		, m_nCurS(200)
		, m_nCurB(200)
		, m_nPalletHeight(200)
		, m_nBarHeight(20)
	{
	}

	CColorPaletteUI::~CColorPaletteUI()
	{
	}

	DWORD CColorPaletteUI::GetSelectColor()
	{
		DWORD dwColor = _HSVToRGB(m_nCurH, m_nCurS, m_nCurB);
		return 0xFF000000 |
			(GetRValue(dwColor) << 16) |
			(GetGValue(dwColor) << 8) |
			GetBValue(dwColor);
	}

	void CColorPaletteUI::SetSelectColor(DWORD dwColor)
	{
		float H = 0.0f;
		float S = 0.0f;
		float V = 0.0f;
		COLORREF dwBkClr = RGB(GetBValue(dwColor), GetGValue(dwColor), GetRValue(dwColor));
		RGBToHSV(dwBkClr, &H, &S, &V);
		m_nCurH = ClampColorComponent(static_cast<int>(H * 360.0f), 0, 359);
		m_nCurS = ClampColorComponent(static_cast<int>(S * 200.0f), 0, 200);
		m_nCurB = ClampColorComponent(static_cast<int>(V * 200.0f), 0, 200);

		if (m_pManager != NULL && m_rcItem.right > m_rcItem.left && GetPalletHeight() > 0) {
			const int width = max(1, m_rcItem.right - m_rcItem.left);
			m_ptLastPalletMouse.x = m_rcItem.left + m_nCurS * width / 200;
			m_ptLastPalletMouse.y = m_rcItem.top + (200 - m_nCurB) * GetPalletHeight() / 200;
			m_ptLastPalletMouse.x = ClampColorComponent(m_ptLastPalletMouse.x, m_rcItem.left, m_rcItem.right);
			m_ptLastPalletMouse.y = ClampColorComponent(m_ptLastPalletMouse.y, m_rcItem.top, m_rcItem.top + GetPalletHeight());
		}

		m_bPaletteDataDirty = true;
		m_bBarDataDirty = true;
		NeedUpdate();
	}

	std::wstring_view CColorPaletteUI::GetClass() const
	{
		return _T("ColorPaletteUI");
	}

	LPVOID CColorPaletteUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_COLORPALETTE) == 0) return static_cast<CColorPaletteUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CColorPaletteUI::SetPalletHeight(int nHeight)
	{
		m_nPalletHeight = nHeight;
		m_bPaletteDataDirty = true;
	}

	int CColorPaletteUI::GetPalletHeight() const
	{
		int nFixHeight = m_nPalletHeight;
		if (m_pManager != NULL) nFixHeight = m_pManager->ScaleValue(nFixHeight);
		return nFixHeight;
	}

	void CColorPaletteUI::SetBarHeight(int nHeight)
	{
		const int MAX_BAR_BUF_H = BASE_BAR_BUF_H;
		m_nBarHeight = min(nHeight, MAX_BAR_BUF_H);
	}

	int CColorPaletteUI::GetBarHeight() const
	{
		int nFixHeight = m_nBarHeight;
		if (m_pManager != NULL) nFixHeight = m_pManager->ScaleValue(nFixHeight);
		return nFixHeight;
	}

	void CColorPaletteUI::SetPalletImage(std::wstring_view pszImage)
	{
		if (m_strPalletImage != pszImage) {
			m_strPalletImage = pszImage;
			NeedUpdate();
		}
	}

	std::wstring_view CColorPaletteUI::GetPalletImage() const
	{
		return m_strPalletImage;
	}

	void CColorPaletteUI::SetBarImage(std::wstring_view pszImage)
	{
		if (m_strBarImage != pszImage) {
			m_strBarImage = pszImage;
			NeedUpdate();
		}
	}

	std::wstring_view CColorPaletteUI::GetBarImage() const
	{
		return m_strBarImage;
	}

	void CColorPaletteUI::SetColorBarImage(std::wstring_view pszImage)
	{
		if (m_strColorBarImage != pszImage) {
			m_strColorBarImage = pszImage;
			NeedUpdate();
		}
	}

	std::wstring_view CColorPaletteUI::GetColorBarImage() const
	{
		return m_strColorBarImage;
	}

	void CColorPaletteUI::SetPalletImageSize(SIZE szPalletImage)
	{
		m_szPalletImage = szPalletImage;
	}

	void CColorPaletteUI::SetBarImageSize(SIZE szBarImage)
	{
		m_szBarImage = szBarImage;
	}

	SIZE CColorPaletteUI::GetPalletImageSize()
	{
		SIZE szImage = m_szPalletImage;
		if (m_pManager != NULL) szImage = m_pManager->ScaleSize(szImage);
		return szImage;
	}

	SIZE CColorPaletteUI::GetBarImageSize()
	{
		SIZE szImage = m_szBarImage;
		if (m_pManager != NULL) szImage = m_pManager->ScaleSize(szImage);
		return szImage;
	}

	void CColorPaletteUI::SetBarHorMove(int move)
	{
		if (move == 0) return;

		if (m_activeHandleHueBar) {
			int h = ClampColorComponent(m_nCurH + move, 0, 359);
			if (h == m_nCurH) return;
			m_nCurH = h;
			m_bPaletteDataDirty = true;
			NotifyColorChanged();
			Invalidate();
			return;
		}

		int s = ClampColorComponent(m_nCurS + move, 0, 200);
		if (s == m_nCurS) return;
		m_nCurS = s;

		const int width = max(1, m_rcItem.right - m_rcItem.left);
		m_ptLastPalletMouse.x = m_rcItem.left + m_nCurS * width / 200;
		m_ptLastPalletMouse.y = m_rcItem.top + (200 - m_nCurB) * GetPalletHeight() / 200;
		m_ptLastPalletMouse.x = ClampColorComponent(m_ptLastPalletMouse.x, m_rcItem.left, m_rcItem.right);
		m_ptLastPalletMouse.y = ClampColorComponent(m_ptLastPalletMouse.y, m_rcItem.top, m_rcItem.top + GetPalletHeight());
		NotifyColorChanged();
		Invalidate();
	}

	void CColorPaletteUI::SetBarVerMove(int move)
	{
		if (move == 0 || m_activeHandleHueBar) return;

		int v = ClampColorComponent(m_nCurB + move, 0, 200);
		if (v == m_nCurB) return;
		m_nCurB = v;

		const int width = max(1, m_rcItem.right - m_rcItem.left);
		m_ptLastPalletMouse.x = m_rcItem.left + m_nCurS * width / 200;
		m_ptLastPalletMouse.y = m_rcItem.top + (200 - m_nCurB) * GetPalletHeight() / 200;
		m_ptLastPalletMouse.x = ClampColorComponent(m_ptLastPalletMouse.x, m_rcItem.left, m_rcItem.right);
		m_ptLastPalletMouse.y = ClampColorComponent(m_ptLastPalletMouse.y, m_rcItem.top, m_rcItem.top + GetPalletHeight());
		NotifyColorChanged();
		Invalidate();
	}

	void CColorPaletteUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if (StringUtil::CompareNoCase(pstrName, _T("palletheight")) == 0) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetPalletHeight(value);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("barheight")) == 0) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetBarHeight(value);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("palletimage")) == 0) SetPalletImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("barimage")) == 0) SetBarImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("colorbarimage")) == 0) SetColorBarImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("palletsize")) == 0) {
			SIZE szXY = { 0, 0 };
			if (StringUtil::TryParseSize(pstrValue, szXY)) SetPalletImageSize(szXY);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("barsize")) == 0) {
			SIZE szXY = { 0, 0 };
			if (StringUtil::TryParseSize(pstrValue, szXY)) SetBarImageSize(szXY);
		}
		else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CColorPaletteUI::DoInit()
	{
	}

	void CColorPaletteUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		if (m_pManager == NULL) return;
		if (!m_bSetColor) {
			SetSelectColor(0xffe91313);
			m_bSetColor = true;
		}
	}

	void CColorPaletteUI::DoEvent(TEventUI& event)
	{
		CControlUI::DoEvent(event);

		const int width = max(1, m_rcItem.right - m_rcItem.left);
		const int palletHeight = max(1, GetPalletHeight());
		const int barHeight = max(1, GetBarHeight());

		if (event.Type == UIEVENT_BUTTONDOWN) {
			if (event.ptMouse.x >= m_rcItem.left && event.ptMouse.y >= m_rcItem.top &&
				event.ptMouse.x < m_rcItem.right && event.ptMouse.y < m_rcItem.top + palletHeight) {
				m_nCurS = ClampColorComponent((event.ptMouse.x - m_rcItem.left) * 200 / width, 0, 200);
				m_nCurB = ClampColorComponent(200 - (event.ptMouse.y - m_rcItem.top) * 200 / palletHeight, 0, 200);
				m_ptLastPalletMouse = event.ptMouse;
				m_ptLastPalletMouse.x = ClampColorComponent(m_ptLastPalletMouse.x, m_rcItem.left, m_rcItem.right);
				m_ptLastPalletMouse.y = ClampColorComponent(m_ptLastPalletMouse.y, m_rcItem.top, m_rcItem.top + palletHeight);
				m_uButtonState |= UISTATE_PUSHED;
				m_bIsInPallet = true;
				m_bIsInBar = false;
				m_activeHandleHueBar = false;
			}

			if (event.ptMouse.x >= m_rcItem.left && event.ptMouse.y >= m_rcItem.bottom - barHeight &&
				event.ptMouse.x < m_rcItem.right && event.ptMouse.y < m_rcItem.bottom) {
				m_nCurH = ClampColorComponent((event.ptMouse.x - m_rcItem.left) * 360 / width, 0, 359);
				m_uButtonState |= UISTATE_PUSHED;
				m_bIsInBar = true;
				m_bIsInPallet = false;
				m_activeHandleHueBar = true;
				m_bPaletteDataDirty = true;
			}

			Invalidate();
			return;
		}

		if (event.Type == UIEVENT_BUTTONUP) {
			if ((m_uButtonState & UISTATE_PUSHED) != 0 && IsEnabled() && m_pManager != NULL) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_COLORCHANGED, GetSelectColor(), 0);
			}

			m_uButtonState &= ~UISTATE_PUSHED;
			m_bIsInPallet = false;
			m_bIsInBar = false;
			Invalidate();
			return;
		}

		if (event.Type == UIEVENT_MOUSEMOVE) {
			if ((m_uButtonState & UISTATE_PUSHED) == 0) {
				m_bIsInBar = false;
				m_bIsInPallet = false;
			}

			if (m_bIsInPallet) {
				POINT pt = event.ptMouse;
				pt.x -= m_rcItem.left;
				pt.y -= m_rcItem.top;
				m_nCurS = ClampColorComponent(pt.x * 200 / width, 0, 200);
				m_nCurB = ClampColorComponent(200 - pt.y * 200 / palletHeight, 0, 200);
				m_ptLastPalletMouse = event.ptMouse;
				m_ptLastPalletMouse.x = ClampColorComponent(m_ptLastPalletMouse.x, m_rcItem.left, m_rcItem.right);
				m_ptLastPalletMouse.y = ClampColorComponent(m_ptLastPalletMouse.y, m_rcItem.top, m_rcItem.top + palletHeight - 1);
			}
			else if (m_bIsInBar) {
				m_nCurH = ClampColorComponent((event.ptMouse.x - m_rcItem.left) * 360 / width, 0, 359);
				m_bPaletteDataDirty = true;
			}

			if ((m_bIsInPallet || m_bIsInBar) && (m_uButtonState & UISTATE_PUSHED) != 0 && IsEnabled() && m_pManager != NULL) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_COLORCHANGED, GetSelectColor(), 0);
			}

			NeedParentUpdate();
			return;
		}
	}

	void CColorPaletteUI::PaintForeImage(CPaintRenderContext& renderContext)
	{
		if (!EnsurePaletteSurface()) return;
		if (m_bPaletteDataDirty) UpdatePalletData();
		if (m_bBarDataDirty) UpdateBarData();
		PaintPallet(renderContext);
	}

	bool CColorPaletteUI::EnsurePaletteSurface()
	{
		if (m_pManager == NULL) return false;

		const int nScaledPaletteW = m_pManager->ScaleValue(BASE_PALETTE_BUF_W);
		const int nScaledPaletteH = m_pManager->ScaleValue(BASE_PALETTE_BUF_H);
		const int nScaledBarW = m_pManager->ScaleValue(BASE_HUE_BAR_W);
		const int nScaledBarH = m_pManager->ScaleValue(BASE_BAR_BUF_H);

		const LONG cx = max(nScaledPaletteW, nScaledBarW);
		const LONG cy = nScaledPaletteH + BAR_GAP + nScaledBarH;
		const SIZE oldSize = m_paletteSurface.GetSize();
		if (!m_paletteSurface.Ensure(m_pManager, cx, cy)) {
			return false;
		}
		if (oldSize.cx != cx || oldSize.cy != cy) {
			m_bPaletteDataDirty = true;
			m_bBarDataDirty = true;
		}
		return true;
	}

	void CColorPaletteUI::NotifyColorChanged()
	{
		if (m_pManager != NULL && IsEnabled()) {
			m_pManager->SendNotify(this, DUI_MSGTYPE_COLORCHANGED, GetSelectColor(), 0);
		}
	}

	void CColorPaletteUI::PaintPallet(CPaintRenderContext& renderContext)
	{
		HBITMAP hPaletteBitmap = m_paletteSurface.GetBitmap();
		if (hPaletteBitmap == NULL) return;

		const int nScaledPaletteW = m_pManager->ScaleValue(BASE_PALETTE_BUF_W);
		const int nScaledPaletteH = m_pManager->ScaleValue(BASE_PALETTE_BUF_H);
		const int nScaledBarW = m_pManager->ScaleValue(BASE_HUE_BAR_W);
		const int nScaledBarH = m_pManager->ScaleValue(BASE_BAR_BUF_H);
		RECT rcEmptyCorners = { 0, 0, 0, 0 };

		RECT rcPaletteDest = {
			m_rcItem.left,
			m_rcItem.top,
			m_rcItem.right,
			m_rcItem.top + GetPalletHeight()
		};
		RECT rcPaletteSource = { 0, 0, nScaledPaletteW, nScaledPaletteH };
		CRenderEngine::DrawImage(renderContext, hPaletteBitmap, rcPaletteDest, rcPaletteSource, rcEmptyCorners, true, 255);

		RECT rcBarDest = {
			m_rcItem.left,
			m_rcItem.bottom - GetBarHeight(),
			m_rcItem.right,
			m_rcItem.bottom
		};
		RECT rcBarSource = {
			0,
			nScaledPaletteH + BAR_GAP,
			nScaledBarW,
			nScaledPaletteH + BAR_GAP + nScaledBarH
		};
		CRenderEngine::DrawImage(renderContext, hPaletteBitmap, rcBarDest, rcBarSource, rcEmptyCorners, true, 255);

		SIZE szPalletImage = GetPalletImageSize();
		int nPalletImageLeft = szPalletImage.cx / 2;
		int nPalletImageRight = szPalletImage.cx - nPalletImageLeft;
		int nPalletImageTop = szPalletImage.cy / 2;
		int nPalletImageBottom = szPalletImage.cy - nPalletImageTop;

		SIZE szBarImage = GetBarImageSize();
		int nBarImageLeft = szBarImage.cx / 2;
		int nBarImageRight = szBarImage.cx - nBarImageLeft;
		int nBarImageTop = szBarImage.cy / 2;
		int nBarImageBottom = szBarImage.cy - nBarImageTop;

		RECT rcCursorPaint = {
			m_ptLastPalletMouse.x - nPalletImageLeft,
			m_ptLastPalletMouse.y - nPalletImageTop,
			m_ptLastPalletMouse.x + nPalletImageRight,
			m_ptLastPalletMouse.y + nPalletImageBottom
		};
		CRenderEngine::DrawImageString(renderContext, rcCursorPaint, m_strPalletImage);

		const int width = max(1, m_rcItem.right - m_rcItem.left);
		const int hueX = m_rcItem.left + m_nCurH * width / 360;
		rcCursorPaint.left = hueX - nBarImageLeft;
		rcCursorPaint.right = hueX + nBarImageRight;
		rcCursorPaint.top = m_rcItem.bottom - GetBarHeight() / 2 - nBarImageTop;
		rcCursorPaint.bottom = m_rcItem.bottom - GetBarHeight() / 2 + nBarImageBottom;
		CRenderEngine::DrawImageString(renderContext, rcCursorPaint, m_strBarImage);

		rcCursorPaint.left = m_rcItem.left;
		rcCursorPaint.right = m_rcItem.right;
		rcCursorPaint.top = m_rcItem.bottom - GetBarHeight();
		rcCursorPaint.bottom = m_rcItem.bottom;
		CRenderEngine::DrawImageString(renderContext, rcCursorPaint, m_strColorBarImage);
	}

	void CColorPaletteUI::UpdatePalletData()
	{
		if (m_pManager == NULL || !m_paletteSurface.IsReady()) return;

		BYTE* pBits = m_paletteSurface.GetBits();
		SIZE surfaceSize = m_paletteSurface.GetSize();
		if (pBits == NULL || surfaceSize.cx <= 0 || surfaceSize.cy <= 0) return;

		int nScaledW = min(m_pManager->ScaleValue(BASE_PALETTE_BUF_W), surfaceSize.cx);
		int nScaledH = min(m_pManager->ScaleValue(BASE_PALETTE_BUF_H), surfaceSize.cy);
		const int rowBytes = surfaceSize.cx * 4;

		for (int y = 0; y < nScaledH; ++y) {
			int logicalV = ((nScaledH - 1 - y) * 200) / max(1, nScaledH - 1);
			BYTE* row = pBits + (surfaceSize.cy - 1 - y) * rowBytes;

			for (int x = 0; x < nScaledW; ++x) {
				int logicalS = (x * 200) / max(1, nScaledW - 1);
				DWORD dwColor = _HSVToRGB(m_nCurH, logicalS, logicalV);
				BYTE* pPixel = row + x * 4;
				pPixel[0] = GetBValue(dwColor);
				pPixel[1] = GetGValue(dwColor);
				pPixel[2] = GetRValue(dwColor);
				pPixel[3] = 0xFF;
			}
		}
		m_bPaletteDataDirty = false;
	}

	void CColorPaletteUI::UpdateBarData()
	{
		if (m_pManager == NULL || !m_paletteSurface.IsReady()) return;

		BYTE* pBits = m_paletteSurface.GetBits();
		SIZE surfaceSize = m_paletteSurface.GetSize();
		if (pBits == NULL || surfaceSize.cx <= 0 || surfaceSize.cy <= 0) return;

		const int nScaledPaletteH = m_pManager->ScaleValue(BASE_PALETTE_BUF_H);
		int nScaledBarW = min(m_pManager->ScaleValue(BASE_HUE_BAR_W), surfaceSize.cx);
		int nScaledBarH = min(m_pManager->ScaleValue(BASE_BAR_BUF_H), surfaceSize.cy - nScaledPaletteH - BAR_GAP);
		const int rowBytes = surfaceSize.cx * 4;

		for (int y = 0; y < nScaledBarH; ++y) {
			const int pixelY = nScaledPaletteH + BAR_GAP + y;
			if (pixelY >= surfaceSize.cy) break;
			BYTE* row = pBits + (surfaceSize.cy - 1 - pixelY) * rowBytes;

			for (int x = 0; x < nScaledBarW; ++x) {
				int hue = (x * 360) / max(1, nScaledBarW - 1);
				DWORD dwColor = _HSVToRGB(hue, 200, 200);
				BYTE* pPixel = row + x * 4;
				pPixel[0] = GetBValue(dwColor);
				pPixel[1] = GetGValue(dwColor);
				pPixel[2] = GetRValue(dwColor);
				pPixel[3] = 0xFF;
			}
		}
		m_bBarDataDirty = false;
	}

}
