#include "pch.h"
#include "..\Control\UIScrollBar.h"
#include "..\Core\UIRenderContext.h"
#include <limits>


namespace FYUI
{
	namespace
	{
		int ClampInt64ToInt(__int64 value)
		{
			if (value < static_cast<__int64>(std::numeric_limits<int>::lowest())) {
				return std::numeric_limits<int>::lowest();
			}
			if (value > static_cast<__int64>((std::numeric_limits<int>::max)())) {
				return (std::numeric_limits<int>::max)();
			}
			return static_cast<int>(value);
		}

		LONG ClampInt64ToLong(__int64 value)
		{
			if (value < static_cast<__int64>(std::numeric_limits<LONG>::lowest())) {
				return std::numeric_limits<LONG>::lowest();
			}
			if (value > static_cast<__int64>((std::numeric_limits<LONG>::max)())) {
				return (std::numeric_limits<LONG>::max)();
			}
			return static_cast<LONG>(value);
		}
	}

	IMPLEMENT_DUICONTROL(CScrollBarUI)

		CScrollBarUI::CScrollBarUI() : m_bHorizontal(false), m_nRange(0), m_nScrollPos(0), m_nLineSize(8),
		m_pOwner(NULL), m_nLastScrollPos(0), m_nLastScrollOffset(0), m_nScrollRepeatDelay(0), m_uButton1State(0), \
		m_uButton2State(0), m_uThumbState(0), m_bShowButton1(true), m_bShowButton2(true), m_bShow(true), m_nSpaceX(0), m_nSpaceY(0)
	{
		m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
		m_ptLastMouse.x = m_ptLastMouse.y = 0;
		::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
		::ZeroMemory(&m_rcButton1, sizeof(m_rcButton1));
		::ZeroMemory(&m_rcButton2, sizeof(m_rcButton2));
	}

	std::wstring_view CScrollBarUI::GetClass() const
	{
		return _T("ScrollBarUI");
	}

	LPVOID CScrollBarUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_SCROLLBAR) == 0) return static_cast<CScrollBarUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	CContainerUI* CScrollBarUI::GetOwner() const
	{
		return m_pOwner;
	}

	void CScrollBarUI::SetOwner(CContainerUI* pOwner)
	{
		m_pOwner = pOwner;
	}

	void CScrollBarUI::SetVisible(bool bVisible)
	{
		if (m_bVisible == bVisible) return;

		bool v = IsVisible();
		m_bVisible = bVisible;
		if (m_bFocused) m_bFocused = false;

	}

	void CScrollBarUI::SetEnabled(bool bEnable)
	{
		CControlUI::SetEnabled(bEnable);
		if (!IsEnabled()) {
			m_uButton1State = 0;
			m_uButton2State = 0;
			m_uThumbState = 0;
		}
	}

	void CScrollBarUI::SetFocus()
	{
		if (m_pOwner != NULL) m_pOwner->SetFocus();
		else CControlUI::SetFocus();
	}

	bool CScrollBarUI::IsHorizontal()
	{
		return m_bHorizontal;
	}

	void CScrollBarUI::SetHorizontal(bool bHorizontal)
	{
		if (m_bHorizontal == bHorizontal) return;

		m_bHorizontal = bHorizontal;
		if (m_bHorizontal) {
			if (m_cxyFixed.cy == 0) {
				m_cxyFixed.cx = 0;
				m_cxyFixed.cy = DEFAULT_SCROLLBAR_SIZE;
			}
		}
		else {
			if (m_cxyFixed.cx == 0) {
				m_cxyFixed.cx = DEFAULT_SCROLLBAR_SIZE;
				m_cxyFixed.cy = 0;
			}
		}

		if (m_pOwner != NULL) m_pOwner->NeedUpdate(); else NeedParentUpdate();
	}

	int CScrollBarUI::GetScrollRange() const
	{
		return ClampInt64ToInt(m_nRange);
	}

	void CScrollBarUI::SetScrollRange(int nRange, bool bIsSetMax)
	{
		if (m_nRange == nRange) return;

		m_nRange = nRange;
		if (m_nRange < 0) m_nRange = 0;
		if (m_nScrollPos > m_nRange && bIsSetMax) m_nScrollPos = m_nRange;
		SetPos(m_rcItem);
	}

	int CScrollBarUI::GetScrollPos() const
	{
		return ClampInt64ToInt(m_nScrollPos);
	}

	void CScrollBarUI::SetScrollPos(int nPos, bool bIsSetMax)
	{
		if (m_nScrollPos == nPos) return;

		m_nScrollPos = nPos;
		if (m_nScrollPos < 0) m_nScrollPos = 0;
		if (m_nScrollPos > m_nRange && bIsSetMax) m_nScrollPos = m_nRange;
		SetPos(m_rcItem);
		if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL);
	}

	int CScrollBarUI::GetLineSize() const
	{
		return m_nLineSize;
	}

	void CScrollBarUI::SetLineSize(int nSize)
	{
		m_nLineSize = nSize;
	}

	bool CScrollBarUI::GetShowButton1()
	{
		return m_bShowButton1;
	}

	void CScrollBarUI::SetShowButton1(bool bShow)
	{
		m_bShowButton1 = bShow;
		SetPos(m_rcItem);
	}

	std::wstring_view CScrollBarUI::GetButton1NormalImage() const
	{
		return m_sButton1NormalImage;
	}

	void CScrollBarUI::SetButton1NormalImage(std::wstring_view pStrImage)
	{
		m_sButton1NormalImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetButton1HotImage() const
	{
		return m_sButton1HotImage;
	}

	void CScrollBarUI::SetButton1HotImage(std::wstring_view pStrImage)
	{
		m_sButton1HotImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetButton1PushedImage() const
	{
		return m_sButton1PushedImage;
	}

	void CScrollBarUI::SetButton1PushedImage(std::wstring_view pStrImage)
	{
		m_sButton1PushedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetButton1DisabledImage() const
	{
		return m_sButton1DisabledImage;
	}

	void CScrollBarUI::SetButton1DisabledImage(std::wstring_view pStrImage)
	{
		m_sButton1DisabledImage.assign(pStrImage);
		Invalidate();
	}

	bool CScrollBarUI::GetShowButton2()
	{
		return m_bShowButton2;
	}

	void CScrollBarUI::SetShowButton2(bool bShow)
	{
		m_bShowButton2 = bShow;
		SetPos(m_rcItem);
	}

	std::wstring_view CScrollBarUI::GetButton2NormalImage() const
	{
		return m_sButton2NormalImage;
	}

	void CScrollBarUI::SetButton2NormalImage(std::wstring_view pStrImage)
	{
		m_sButton2NormalImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetButton2HotImage() const
	{
		return m_sButton2HotImage;
	}

	void CScrollBarUI::SetButton2HotImage(std::wstring_view pStrImage)
	{
		m_sButton2HotImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetButton2PushedImage() const
	{
		return m_sButton2PushedImage;
	}

	void CScrollBarUI::SetButton2PushedImage(std::wstring_view pStrImage)
	{
		m_sButton2PushedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetButton2DisabledImage() const
	{
		return m_sButton2DisabledImage;
	}

	void CScrollBarUI::SetButton2DisabledImage(std::wstring_view pStrImage)
	{
		m_sButton2DisabledImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetThumbNormalImage() const
	{
		return m_sThumbNormalImage;
	}

	void CScrollBarUI::SetThumbNormalImage(std::wstring_view pStrImage)
	{
		m_sThumbNormalImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetThumbHotImage() const
	{
		return m_sThumbHotImage;
	}

	void CScrollBarUI::SetThumbHotImage(std::wstring_view pStrImage)
	{
		m_sThumbHotImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetThumbPushedImage() const
	{
		return m_sThumbPushedImage;
	}

	void CScrollBarUI::SetThumbPushedImage(std::wstring_view pStrImage)
	{
		m_sThumbPushedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetThumbDisabledImage() const
	{
		return m_sThumbDisabledImage;
	}

	void CScrollBarUI::SetThumbDisabledImage(std::wstring_view pStrImage)
	{
		m_sThumbDisabledImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetRailNormalImage() const
	{
		return m_sRailNormalImage;
	}

	void CScrollBarUI::SetRailNormalImage(std::wstring_view pStrImage)
	{
		m_sRailNormalImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetRailHotImage() const
	{
		return m_sRailHotImage;
	}

	void CScrollBarUI::SetRailHotImage(std::wstring_view pStrImage)
	{
		m_sRailHotImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetRailPushedImage() const
	{
		return m_sRailPushedImage;
	}

	void CScrollBarUI::SetRailPushedImage(std::wstring_view pStrImage)
	{
		m_sRailPushedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetRailDisabledImage() const
	{
		return m_sRailDisabledImage;
	}

	void CScrollBarUI::SetRailDisabledImage(std::wstring_view pStrImage)
	{
		m_sRailDisabledImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetBkNormalImage() const
	{
		return m_sBkNormalImage;
	}

	void CScrollBarUI::SetBkNormalImage(std::wstring_view pStrImage)
	{
		m_sBkNormalImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetBkHotImage() const
	{
		return m_sBkHotImage;
	}

	void CScrollBarUI::SetBkHotImage(std::wstring_view pStrImage)
	{
		m_sBkHotImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetBkPushedImage() const
	{
		return m_sBkPushedImage;
	}

	void CScrollBarUI::SetBkPushedImage(std::wstring_view pStrImage)
	{
		m_sBkPushedImage.assign(pStrImage);
		Invalidate();
	}

	std::wstring_view CScrollBarUI::GetBkDisabledImage() const
	{
		return m_sBkDisabledImage;
	}

	void CScrollBarUI::SetBkDisabledImage(std::wstring_view pStrImage)
	{
		m_sBkDisabledImage.assign(pStrImage);
		Invalidate();
	}

	bool CScrollBarUI::GetShow()
	{
		return m_bShow;
	}

	void CScrollBarUI::SetShow(bool bShow)
	{
		m_bShow = bShow;
		Invalidate();
	}

	void CScrollBarUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);

		SIZE cxyFixed = m_cxyFixed;
		if (m_pManager != NULL) {
			cxyFixed = GetManager()->ScaleSize(cxyFixed);
		}
		int nMinSize = 80;
		if (m_pManager != NULL)
		{
			nMinSize = GetManager()->ScaleValue(nMinSize);
		}

		rc = m_rcItem;
		if (m_bHorizontal) {
			int cx = rc.right - rc.left;
			if (m_bShowButton1) cx -= cxyFixed.cy;
			if (m_bShowButton2) cx -= cxyFixed.cy;
			if (cx > cxyFixed.cy) {
				m_rcButton1.left = rc.left;
				m_rcButton1.top = rc.top;
				if (m_bShowButton1) {
					m_rcButton1.right = rc.left + cxyFixed.cy;
					m_rcButton1.bottom = rc.top + cxyFixed.cy;
				}
				else {
					m_rcButton1.right = m_rcButton1.left;
					m_rcButton1.bottom = m_rcButton1.top;
				}

				m_rcButton2.top = rc.top;
				m_rcButton2.right = rc.right;
				if (m_bShowButton2) {
					m_rcButton2.left = rc.right - cxyFixed.cy;
					m_rcButton2.bottom = rc.top + cxyFixed.cy;
				}
				else {
					m_rcButton2.left = m_rcButton2.right;
					m_rcButton2.bottom = m_rcButton2.top;
				}

				m_rcThumb.top = rc.top;
				m_rcThumb.bottom = rc.top + cxyFixed.cy;
				if (m_nRange > 0) {
					const __int64 thumbNumerator = static_cast<__int64>(cx) * (rc.right - rc.left);
					int cxThumb = ClampInt64ToInt(thumbNumerator / (m_nRange + rc.right - rc.left));
					//if( cxThumb < cxyFixed.cy ) cxThumb = cxyFixed.cy;
					if (cxThumb < nMinSize) cxThumb = nMinSize;
					m_rcThumb.left = ClampInt64ToLong(m_nScrollPos * static_cast<__int64>(cx - cxThumb) / m_nRange + m_rcButton1.right);
					m_rcThumb.right = m_rcThumb.left + cxThumb;
					if (m_rcThumb.right > m_rcButton2.left) {
						m_rcThumb.left = m_rcButton2.left - cxThumb;
						m_rcThumb.right = m_rcButton2.left;
					}
				}
				else {
					m_rcThumb.left = m_rcButton1.right;
					m_rcThumb.right = m_rcButton2.left;
				}
			}
			else {
				int cxButton = (rc.right - rc.left) / 2;
				if (cxButton > cxyFixed.cy) cxButton = cxyFixed.cy;
				m_rcButton1.left = rc.left;
				m_rcButton1.top = rc.top;
				if (m_bShowButton1) {
					m_rcButton1.right = rc.left + cxButton;
					m_rcButton1.bottom = rc.top + cxyFixed.cy;
				}
				else {
					m_rcButton1.right = m_rcButton1.left;
					m_rcButton1.bottom = m_rcButton1.top;
				}

				m_rcButton2.top = rc.top;
				m_rcButton2.right = rc.right;
				if (m_bShowButton2) {
					m_rcButton2.left = rc.right - cxButton;
					m_rcButton2.bottom = rc.top + cxyFixed.cy;
				}
				else {
					m_rcButton2.left = m_rcButton2.right;
					m_rcButton2.bottom = m_rcButton2.top;
				}

				::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
			}
		}
		else {
			int cy = rc.bottom - rc.top;
			if (m_bShowButton1) cy -= cxyFixed.cx;
			if (m_bShowButton2) cy -= cxyFixed.cx;
			if (cy > cxyFixed.cx) {
				m_rcButton1.left = rc.left;
				m_rcButton1.top = rc.top;
				if (m_bShowButton1) {
					m_rcButton1.right = rc.left + cxyFixed.cx;
					m_rcButton1.bottom = rc.top + cxyFixed.cx;
				}
				else {
					m_rcButton1.right = m_rcButton1.left;
					m_rcButton1.bottom = m_rcButton1.top;
				}

				m_rcButton2.left = rc.left;
				m_rcButton2.bottom = rc.bottom;
				if (m_bShowButton2) {
					m_rcButton2.top = rc.bottom - cxyFixed.cx;
					m_rcButton2.right = rc.left + cxyFixed.cx;
				}
				else {
					m_rcButton2.top = m_rcButton2.bottom;
					m_rcButton2.right = m_rcButton2.left;
				}

				m_rcThumb.left = rc.left;
				m_rcThumb.right = rc.left + cxyFixed.cx;
				if (m_nRange > 0) {
					const __int64 thumbNumerator = static_cast<__int64>(cy) * (rc.bottom - rc.top);
					int cyThumb = ClampInt64ToInt(thumbNumerator / (m_nRange + rc.bottom - rc.top));
					//if( cyThumb < nMinSize ) cyThumb = cxyFixed.cx;
					if (cyThumb < nMinSize) cyThumb = nMinSize;
					m_rcThumb.top = ClampInt64ToLong(m_nScrollPos * static_cast<__int64>(cy - cyThumb) / m_nRange + m_rcButton1.bottom);
					m_rcThumb.bottom = m_rcThumb.top + cyThumb;
					if (m_rcThumb.bottom > m_rcButton2.top) {
						m_rcThumb.top = m_rcButton2.top - cyThumb;
						m_rcThumb.bottom = m_rcButton2.top;
					}
				}
				else {
					m_rcThumb.top = m_rcButton1.bottom;
					m_rcThumb.bottom = m_rcButton2.top;
				}
			}
			else {
				int cyButton = (rc.bottom - rc.top) / 2;
				if (cyButton > cxyFixed.cx) cyButton = cxyFixed.cx;
				m_rcButton1.left = rc.left;
				m_rcButton1.top = rc.top;
				if (m_bShowButton1) {
					m_rcButton1.right = rc.left + cxyFixed.cx;
					m_rcButton1.bottom = rc.top + cyButton;
				}
				else {
					m_rcButton1.right = m_rcButton1.left;
					m_rcButton1.bottom = m_rcButton1.top;
				}

				m_rcButton2.left = rc.left;
				m_rcButton2.bottom = rc.bottom;
				if (m_bShowButton2) {
					m_rcButton2.top = rc.bottom - cyButton;
					m_rcButton2.right = rc.left + cxyFixed.cx;
				}
				else {
					m_rcButton2.top = m_rcButton2.bottom;
					m_rcButton2.right = m_rcButton2.left;
				}

				::ZeroMemory(&m_rcThumb, sizeof(m_rcThumb));
			}
		}
	}

	void CScrollBarUI::DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pOwner != NULL) m_pOwner->DoEvent(event);
			else CControlUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_SETFOCUS)
		{
			return;
		}
		if (event.Type == UIEVENT_KILLFOCUS)
		{
			return;
		}
		if (event.Type == UIEVENT_BUTTONDOWN || event.Type == UIEVENT_DBLCLICK)
		{
			if (!IsEnabled()) return;

			m_nLastScrollOffset = 0;
			m_nScrollRepeatDelay = 0;
			m_bMouseDown = true;
			if (::PtInRect(&m_rcButton1, event.ptMouse)) {
				m_uButton1State |= UISTATE_PUSHED;
				if (!m_bHorizontal) {
					if (m_pOwner != NULL) m_pOwner->LineUp();
					else SetScrollPos(ClampInt64ToInt(m_nScrollPos - m_nLineSize));
				}
				else {
					if (m_pOwner != NULL) m_pOwner->LineLeft();
					else SetScrollPos(ClampInt64ToInt(m_nScrollPos - m_nLineSize));
				}
			}
			else if (::PtInRect(&m_rcButton2, event.ptMouse)) {
				m_uButton2State |= UISTATE_PUSHED;
				if (!m_bHorizontal) {
					if (m_pOwner != NULL) m_pOwner->LineDown();
					else SetScrollPos(ClampInt64ToInt(m_nScrollPos + m_nLineSize));
				}
				else {
					if (m_pOwner != NULL) m_pOwner->LineRight();
					else SetScrollPos(ClampInt64ToInt(m_nScrollPos + m_nLineSize));
				}
			}
			else if (::PtInRect(&m_rcThumb, event.ptMouse)) {
				m_uThumbState |= UISTATE_CAPTURED | UISTATE_PUSHED;
				m_ptLastMouse = event.ptMouse;
				m_nLastScrollPos = ClampInt64ToInt(m_nScrollPos);

				m_pManager->SetTimer(this, DEFAULT_TIMERID, 50U);
			}
			else {
				if (!m_bHorizontal) {
					if (event.ptMouse.y < m_rcThumb.top) {
						if (m_pOwner != NULL) m_pOwner->PageUp();
						else SetScrollPos(ClampInt64ToInt(m_nScrollPos + m_rcItem.top - m_rcItem.bottom));
					}
					else if (event.ptMouse.y > m_rcThumb.bottom) {
						if (m_pOwner != NULL) m_pOwner->PageDown();
						else SetScrollPos(ClampInt64ToInt(m_nScrollPos - m_rcItem.top + m_rcItem.bottom));
					}
				}
				else {
					if (event.ptMouse.x < m_rcThumb.left) {
						if (m_pOwner != NULL) m_pOwner->PageLeft();
						else SetScrollPos(ClampInt64ToInt(m_nScrollPos + m_rcItem.left - m_rcItem.right));
					}
					else if (event.ptMouse.x > m_rcThumb.right) {
						if (m_pOwner != NULL) m_pOwner->PageRight();
						else SetScrollPos(ClampInt64ToInt(m_nScrollPos - m_rcItem.left + m_rcItem.right));
					}
				}
			}
			if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL);
			return;
		}
		if (event.Type == UIEVENT_BUTTONUP)
		{
			m_bMouseDown = false;
			m_nScrollRepeatDelay = 0;
			m_nLastScrollOffset = 0;
			m_pManager->KillTimer(this, DEFAULT_TIMERID);

			if ((m_uThumbState & UISTATE_CAPTURED) != 0) {
				m_uThumbState &= ~(UISTATE_CAPTURED | UISTATE_PUSHED);
				Invalidate();
			}
			else if ((m_uButton1State & UISTATE_PUSHED) != 0) {
				m_uButton1State &= ~UISTATE_PUSHED;
				Invalidate();
			}
			else if ((m_uButton2State & UISTATE_PUSHED) != 0) {
				m_uButton2State &= ~UISTATE_PUSHED;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSEMOVE)
		{
			if ((m_uThumbState & UISTATE_CAPTURED) != 0)
			{
				if (!m_bHorizontal)
				{
					__int64 fMouseRange = (event.ptMouse.y - m_ptLastMouse.y) * m_nRange;
					int nBtnSize = 0;
					if (GetShowButton1()) nBtnSize += m_cxyFixed.cx;
					if (GetShowButton2()) nBtnSize += m_cxyFixed.cx;
					int vRange = m_rcItem.bottom - m_rcItem.top - (m_rcThumb.bottom - m_rcThumb.top) - nBtnSize;
					if (vRange != 0) {
						m_nLastScrollOffset = ClampInt64ToInt(fMouseRange / abs(vRange));
					}
				}
				else {
					__int64 fMouseRange = (event.ptMouse.x - m_ptLastMouse.x) * m_nRange;
					int nBtnSize = 0;
					if (GetShowButton1()) nBtnSize += m_cxyFixed.cy;
					if (GetShowButton2()) nBtnSize += m_cxyFixed.cy;
					int hRange = m_rcItem.right - m_rcItem.left - m_rcThumb.right + m_rcThumb.left - nBtnSize;
					if (hRange != 0) m_nLastScrollOffset = ClampInt64ToInt(fMouseRange / abs(hRange));
				}
			}
			else {
				if ((m_uThumbState & UISTATE_HOT) != 0) {
					if (!::PtInRect(&m_rcThumb, event.ptMouse)) {
						m_uThumbState &= ~UISTATE_HOT;
						Invalidate();
					}
				}
				else {
					if (!IsEnabled()) return;
					if (::PtInRect(&m_rcThumb, event.ptMouse)) {
						m_uThumbState |= UISTATE_HOT;
						Invalidate();
					}
				}
			}
			return;
		}
		if (event.Type == UIEVENT_CONTEXTMENU)
		{
			return;
		}
		if (event.Type == UIEVENT_TIMER && event.wParam == DEFAULT_TIMERID)
		{
			++m_nScrollRepeatDelay;
			if ((m_uThumbState & UISTATE_CAPTURED) != 0) {
				if (!m_bHorizontal) {
					if (m_pOwner != NULL) m_pOwner->SetScrollPos(CDuiSize(m_pOwner->GetScrollPos().cx, \
						m_nLastScrollPos + m_nLastScrollOffset));
					else SetScrollPos(ClampInt64ToInt(static_cast<__int64>(m_nLastScrollPos) + m_nLastScrollOffset));
				}
				else {
					if (m_pOwner != NULL) m_pOwner->SetScrollPos(CDuiSize(m_nLastScrollPos + m_nLastScrollOffset, \
						m_pOwner->GetScrollPos().cy));
					else SetScrollPos(ClampInt64ToInt(static_cast<__int64>(m_nLastScrollPos) + m_nLastScrollOffset));
				}
				Invalidate();
			}
			else if ((m_uButton1State & UISTATE_PUSHED) != 0) {
				if (m_nScrollRepeatDelay <= 5) return;
				if (!m_bHorizontal) {
					if (m_pOwner != NULL) m_pOwner->LineUp();
					else SetScrollPos(ClampInt64ToInt(m_nScrollPos - m_nLineSize));
				}
				else {
					if (m_pOwner != NULL) m_pOwner->LineLeft();
					else SetScrollPos(ClampInt64ToInt(m_nScrollPos - m_nLineSize));
				}
			}
			else if ((m_uButton2State & UISTATE_PUSHED) != 0) {
				if (m_nScrollRepeatDelay <= 5) return;
				if (!m_bHorizontal) {
					if (m_pOwner != NULL) m_pOwner->LineDown();
					else SetScrollPos(ClampInt64ToInt(m_nScrollPos + m_nLineSize));
				}
				else {
					if (m_pOwner != NULL) m_pOwner->LineRight();
					else SetScrollPos(ClampInt64ToInt(m_nScrollPos + m_nLineSize));
				}
			}
			else {
				if (m_nScrollRepeatDelay <= 5) return;
				POINT pt = { 0 };
				::GetCursorPos(&pt);
				::ScreenToClient(m_pManager->GetPaintWindow(), &pt);
				if (!m_bHorizontal) {
					if (pt.y < m_rcThumb.top) {
						if (m_pOwner != NULL) m_pOwner->PageUp();
						else SetScrollPos(ClampInt64ToInt(m_nScrollPos + m_rcItem.top - m_rcItem.bottom));
					}
					else if (pt.y > m_rcThumb.bottom) {
						if (m_pOwner != NULL) m_pOwner->PageDown();
						else SetScrollPos(ClampInt64ToInt(m_nScrollPos - m_rcItem.top + m_rcItem.bottom));
					}
				}
				else {
					if (pt.x < m_rcThumb.left) {
						if (m_pOwner != NULL) m_pOwner->PageLeft();
						else SetScrollPos(ClampInt64ToInt(m_nScrollPos + m_rcItem.left - m_rcItem.right));
					}
					else if (pt.x > m_rcThumb.right) {
						if (m_pOwner != NULL) m_pOwner->PageRight();
						else SetScrollPos(ClampInt64ToInt(m_nScrollPos - m_rcItem.left + m_rcItem.right));
					}
				}
			}
			if (m_pManager != NULL) m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL);
			return;
		}
		if (event.Type == UIEVENT_MOUSEENTER)
		{
			if (IsEnabled()) {
				m_uButton1State |= UISTATE_HOT;
				m_uButton2State |= UISTATE_HOT;
				if (::PtInRect(&m_rcThumb, event.ptMouse)) m_uThumbState |= UISTATE_HOT;
				Invalidate();
			}
			return;
		}
		if (event.Type == UIEVENT_MOUSELEAVE)
		{
			if (IsEnabled()) {
				m_uButton1State &= ~UISTATE_HOT;
				m_uButton2State &= ~UISTATE_HOT;
				m_uThumbState &= ~UISTATE_HOT;
				Invalidate();
			}
			return;
		}

		if (m_pOwner != NULL) m_pOwner->DoEvent(event); else CControlUI::DoEvent(event);
	}

	void CScrollBarUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
	{
		const std::wstring nameStorage(pstrNameView);
		const std::wstring valueStorage(pstrValueView);
		const wchar_t* pstrName = nameStorage.c_str();
		const wchar_t* pstrValue = valueStorage.c_str();
		if (StringUtil::CompareNoCase(pstrName, _T("button1normalimage")) == 0) SetButton1NormalImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("button1hotimage")) == 0) SetButton1HotImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("button1pushedimage")) == 0) SetButton1PushedImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("button1disabledimage")) == 0) SetButton1DisabledImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("button2normalimage")) == 0) SetButton2NormalImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("button2hotimage")) == 0) SetButton2HotImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("button2pushedimage")) == 0) SetButton2PushedImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("button2disabledimage")) == 0) SetButton2DisabledImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("thumbnormalimage")) == 0) SetThumbNormalImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("thumbhotimage")) == 0) SetThumbHotImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("thumbpushedimage")) == 0) SetThumbPushedImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("thumbdisabledimage")) == 0) SetThumbDisabledImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("railnormalimage")) == 0) SetRailNormalImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("railhotimage")) == 0) SetRailHotImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("railpushedimage")) == 0) SetRailPushedImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("raildisabledimage")) == 0) SetRailDisabledImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("bknormalimage")) == 0) SetBkNormalImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("bkhotimage")) == 0) SetBkHotImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("bkpushedimage")) == 0) SetBkPushedImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("bkdisabledimage")) == 0) SetBkDisabledImage(pstrValue);
		else if (StringUtil::CompareNoCase(pstrName, _T("hor")) == 0) SetHorizontal(StringUtil::ParseBool(pstrValue));
		else if (StringUtil::CompareNoCase(pstrName, _T("linesize")) == 0) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetLineSize(value);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("range")) == 0) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetScrollRange(value);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("value")) == 0) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetScrollPos(value);
		}
		else if (StringUtil::CompareNoCase(pstrName, _T("showbutton1")) == 0) SetShowButton1(StringUtil::ParseBool(pstrValue));
	else if (StringUtil::CompareNoCase(pstrName, _T("showbutton2")) == 0) SetShowButton2(StringUtil::ParseBool(pstrValue));
	else CControlUI::SetAttribute(pstrName, pstrValue);
	}

	bool CScrollBarUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		if (!GetShow()) return true;

		PaintBkColor(renderContext);
		PaintBkImage(renderContext);
		PaintBk(renderContext);
		PaintButton1(renderContext);
		PaintButton2(renderContext);
		PaintThumb(renderContext);
		PaintRail(renderContext);
		PaintBorder(renderContext);
		return true;
	}

	void CScrollBarUI::PaintBk(CPaintRenderContext& renderContext)
	{
		if (!IsEnabled()) m_uThumbState |= UISTATE_DISABLED;
		else m_uThumbState &= ~UISTATE_DISABLED;

		if ((m_uThumbState & UISTATE_DISABLED) != 0) {
			if (!m_sBkDisabledImage.empty()) {
				if (!DrawImage(renderContext, m_sBkDisabledImage)) {}
				else return;
			}
		}
		else if ((m_uThumbState & UISTATE_PUSHED) != 0) {
			if (!m_sBkPushedImage.empty()) {
				if (!DrawImage(renderContext, m_sBkPushedImage)) {}
				else return;
			}
		}
		else if ((m_uThumbState & UISTATE_HOT) != 0) {
			if (!m_sBkHotImage.empty()) {
				if (!DrawImage(renderContext, m_sBkHotImage)) {}
				else return;
			}
		}

		if (!m_sBkNormalImage.empty()) {
			if (!DrawImage(renderContext, m_sBkNormalImage)) {}
			else return;
		}
	}

	void CScrollBarUI::PaintButton1(CPaintRenderContext& renderContext)
	{
		if (!m_bShowButton1) return;

		if (!IsEnabled()) m_uButton1State |= UISTATE_DISABLED;
		else m_uButton1State &= ~UISTATE_DISABLED;

		RECT rcDest = {
			m_rcButton1.left - m_rcItem.left,
			m_rcButton1.top - m_rcItem.top,
			m_rcButton1.right - m_rcItem.left,
			m_rcButton1.bottom - m_rcItem.top
		};
		rcDest = PixelsToLogical(rcDest);
		m_sImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);

		if ((m_uButton1State & UISTATE_DISABLED) != 0) {
			if (!m_sButton1DisabledImage.empty()) {
				if (!DrawImage(renderContext, m_sButton1DisabledImage, m_sImageModify)) {}
				else return;
			}
		}
		else if ((m_uButton1State & UISTATE_PUSHED) != 0) {
			if (!m_sButton1PushedImage.empty()) {
				if (!DrawImage(renderContext, m_sButton1PushedImage, m_sImageModify)) {}
				else return;
			}
		}
		else if ((m_uButton1State & UISTATE_HOT) != 0) {
			if (!m_sButton1HotImage.empty()) {
				if (!DrawImage(renderContext, m_sButton1HotImage, m_sImageModify)) {}
				else return;
			}
		}

		if (!m_sButton1NormalImage.empty()) {
			if (!DrawImage(renderContext, m_sButton1NormalImage, m_sImageModify)) {}
			else return;
		}

		DWORD dwBorderColor = 0xFF85E4FF;
		int nBorderSize = 2;
		CRenderEngine::DrawRect(renderContext, m_rcButton1, nBorderSize, dwBorderColor);
	}

	void CScrollBarUI::PaintButton2(CPaintRenderContext& renderContext)
	{
		if (!m_bShowButton2) return;

		if (!IsEnabled()) m_uButton2State |= UISTATE_DISABLED;
		else m_uButton2State &= ~UISTATE_DISABLED;
		RECT rcDest = {
			m_rcButton2.left - m_rcItem.left,
			m_rcButton2.top - m_rcItem.top,
			m_rcButton2.right - m_rcItem.left,
			m_rcButton2.bottom - m_rcItem.top
		};
		rcDest = PixelsToLogical(rcDest);
		m_sImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);

		if ((m_uButton2State & UISTATE_DISABLED) != 0) {
			if (!m_sButton2DisabledImage.empty()) {
				if (!DrawImage(renderContext, m_sButton2DisabledImage, m_sImageModify)) {}
				else return;
			}
		}
		else if ((m_uButton2State & UISTATE_PUSHED) != 0) {
			if (!m_sButton2PushedImage.empty()) {
				if (!DrawImage(renderContext, m_sButton2PushedImage, m_sImageModify)) {}
				else return;
			}
		}
		else if ((m_uButton2State & UISTATE_HOT) != 0) {
			if (!m_sButton2HotImage.empty()) {
				if (!DrawImage(renderContext, m_sButton2HotImage, m_sImageModify)) {}
				else return;
			}
		}

		if (!m_sButton2NormalImage.empty()) {
			if (!DrawImage(renderContext, m_sButton2NormalImage, m_sImageModify)) {}
			else return;
		}

		DWORD dwBorderColor = 0xFF85E4FF;
		int nBorderSize = 2;
		CRenderEngine::DrawRect(renderContext, m_rcButton2, nBorderSize, dwBorderColor);
	}

	void CScrollBarUI::PaintThumb(CPaintRenderContext& renderContext)
	{
		if (m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0) return;
		if (!IsEnabled()) m_uThumbState |= UISTATE_DISABLED;
		else m_uThumbState &= ~UISTATE_DISABLED;
		RECT rcDest = {
			m_rcThumb.left - m_rcItem.left,
			m_rcThumb.top - m_rcItem.top,
			m_rcThumb.right - m_rcItem.left,
			m_rcThumb.bottom - m_rcItem.top
		};
		rcDest = PixelsToLogical(rcDest);
		m_sImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);

		if ((m_uThumbState & UISTATE_DISABLED) != 0) {
			if (!m_sThumbDisabledImage.empty()) {
				if (!DrawImage(renderContext, m_sThumbDisabledImage, m_sImageModify)) {}
				else return;
			}
		}
		else if ((m_uThumbState & UISTATE_PUSHED) != 0) {
			if (!m_sThumbPushedImage.empty()) {
				if (!DrawImage(renderContext, m_sThumbPushedImage, m_sImageModify)) {}
				else return;
			}
		}
		else if ((m_uThumbState & UISTATE_HOT) != 0) {
			if (!m_sThumbHotImage.empty()) {
				if (!DrawImage(renderContext, m_sThumbHotImage, m_sImageModify)) {}
				else return;
			}
		}

		if (!m_sThumbNormalImage.empty()) {
			if (!DrawImage(renderContext, m_sThumbNormalImage, m_sImageModify)) {}
			else return;
		}

		DWORD dwBorderColor = 0xFF85E4FF;
		int nBorderSize = 2;
		CRenderEngine::DrawRect(renderContext, m_rcThumb, nBorderSize, dwBorderColor);
	}

	void CScrollBarUI::PaintRail(CPaintRenderContext& renderContext)
	{
		if (m_rcThumb.left == 0 && m_rcThumb.top == 0 && m_rcThumb.right == 0 && m_rcThumb.bottom == 0) return;
		if (!IsEnabled()) m_uThumbState |= UISTATE_DISABLED;
		else m_uThumbState &= ~UISTATE_DISABLED;

		m_sImageModify.clear();
		if (!m_bHorizontal) {
			RECT rcDest = {
				m_rcThumb.left - m_rcItem.left,
				(m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top - m_cxyFixed.cx / 2,
				m_rcThumb.right - m_rcItem.left,
				(m_rcThumb.top + m_rcThumb.bottom) / 2 - m_rcItem.top + m_cxyFixed.cx - m_cxyFixed.cx / 2
			};
			rcDest = PixelsToLogical(rcDest);
			m_sImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);
		}
		else {
			RECT rcDest = {
				(m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left - m_cxyFixed.cy / 2,
				m_rcThumb.top - m_rcItem.top,
				(m_rcThumb.left + m_rcThumb.right) / 2 - m_rcItem.left + m_cxyFixed.cy - m_cxyFixed.cy / 2,
				m_rcThumb.bottom - m_rcItem.top
			};
			rcDest = PixelsToLogical(rcDest);
			m_sImageModify = StringUtil::Format(L"dest='{},{},{},{}'", rcDest.left, rcDest.top, rcDest.right, rcDest.bottom);
		}

		if ((m_uThumbState & UISTATE_DISABLED) != 0) {
			if (!m_sRailDisabledImage.empty()) {
				if (!DrawImage(renderContext, m_sRailDisabledImage, m_sImageModify)) {}
				else return;
			}
		}
		else if ((m_uThumbState & UISTATE_PUSHED) != 0) {
			if (!m_sRailPushedImage.empty()) {
				if (!DrawImage(renderContext, m_sRailPushedImage, m_sImageModify)) {}
				else return;
			}
		}
		else if ((m_uThumbState & UISTATE_HOT) != 0) {
			if (!m_sRailHotImage.empty()) {
				if (!DrawImage(renderContext, m_sRailHotImage, m_sImageModify)) {}
				else return;
			}
		}

		if (!m_sRailNormalImage.empty()) {
			if (!DrawImage(renderContext, m_sRailNormalImage, m_sImageModify)) {}
			else return;
		}
	}

	void CScrollBarUI::SetHSpace(int cx)
	{
		if (cx == m_nSpaceX)
			return;
		m_nSpaceX = cx;
		NeedUpdate();
	}
	void CScrollBarUI::SetVSpace(int cy)
	{
		if (cy == m_nSpaceY)
			return;
		m_nSpaceY = cy;
		NeedUpdate();
	}
	int CScrollBarUI::GetHSpace() const
	{
		if (m_pManager) {
			return m_pManager->ScaleValue(m_nSpaceX);
		}
		return m_nSpaceX;
	}
	int CScrollBarUI::GetVSpace() const
	{
		if (m_pManager) {
			return m_pManager->ScaleValue(m_nSpaceY);
		}
		return m_nSpaceY;
	}
}

