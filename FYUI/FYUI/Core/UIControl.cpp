#include "pch.h"
#include "UIControl.h"
#include "UIRenderContext.h"
#include <cwctype>

namespace FYUI
{
	namespace
	{
		RECT ResolveCompatPaintRect(const RECT& rcPaint, const RECT& rcFallback)
		{
			RECT rcResolved = rcPaint;
			if (rcResolved.right <= rcResolved.left || rcResolved.bottom <= rcResolved.top) {
				rcResolved = rcFallback;
			}
			return rcResolved;
		}

		bool IsAttributeName(std::wstring_view actual, std::wstring_view expected)
		{
			return StringUtil::EqualsNoCase(actual, expected);
		}

		bool HasRoundCorner(SIZE round)
		{
			return round.cx > 0 || round.cy > 0;
		}

		bool RectIntersects(const RECT& lhs, const RECT& rhs)
		{
			RECT rc = { 0 };
			return ::IntersectRect(&rc, &lhs, &rhs) != FALSE;
		}

		bool PaintRectTouchesRoundCorner(const RECT& rcPaint, const RECT& rcItem, SIZE round)
		{
			if (!HasRoundCorner(round)) {
				return false;
			}

			const int rx = round.cx > 0 ? round.cx : round.cy;
			const int ry = round.cy > 0 ? round.cy : round.cx;
			const RECT corners[] = {
				{ rcItem.left, rcItem.top, rcItem.left + rx, rcItem.top + ry },
				{ rcItem.right - rx, rcItem.top, rcItem.right, rcItem.top + ry },
				{ rcItem.left, rcItem.bottom - ry, rcItem.left + rx, rcItem.bottom },
				{ rcItem.right - rx, rcItem.bottom - ry, rcItem.right, rcItem.bottom }
			};

			for (const RECT& corner : corners) {
				if (RectIntersects(rcPaint, corner)) {
					return true;
				}
			}
			return false;
		}

		bool PaintRectTouchesBorderBand(const RECT& rcPaint, const RECT& rcItem, int borderSize)
		{
			if (borderSize <= 0) {
				return false;
			}

			RECT rcPaintInItem = { 0 };
			if (!::IntersectRect(&rcPaintInItem, &rcPaint, &rcItem)) {
				return false;
			}

			RECT rcInner = rcItem;
			::InflateRect(&rcInner, -borderSize, -borderSize);
			if (rcInner.right <= rcInner.left || rcInner.bottom <= rcInner.top) {
				return true;
			}

			RECT rcInnerPaint = { 0 };
			if (!::IntersectRect(&rcInnerPaint, &rcPaintInItem, &rcInner)) {
				return true;
			}
			return !::EqualRect(&rcPaintInItem, &rcInnerPaint);
		}

		void DrawRoundedAwareColor(CPaintRenderContext& renderContext, const RECT& rc, SIZE round, DWORD color)
		{
			if (HasRoundCorner(round)) {
				CRenderEngine::DrawRoundColor(renderContext, rc, round.cx, round.cy, color);
				return;
			}
			CRenderEngine::DrawColor(renderContext, rc, color);
		}

		void DrawRoundedAwareSolidColorForPaint(CPaintRenderContext& renderContext, const RECT& rcItem, const RECT& rcPaint, SIZE round, DWORD color)
		{
			if (HasRoundCorner(round) && PaintRectTouchesRoundCorner(rcPaint, rcItem, round)) {
				CRenderEngine::DrawRoundColor(renderContext, rcItem, round.cx, round.cy, color);
				return;
			}

			CRenderEngine::DrawColor(renderContext, rcPaint, color);
		}

		bool TryParseFloatPercent(std::wstring_view text, TPercentInfo& value)
		{
			const std::vector<std::wstring_view> parts = StringUtil::SplitView(text, L',', true);
			if (parts.size() != 2) {
				return false;
			}

			double left = 0.0;
			double top = 0.0;
			if (!StringUtil::TryParseDouble(parts[0], left) || !StringUtil::TryParseDouble(parts[1], top)) {
				return false;
			}

			value.left = static_cast<float>(left);
			value.top = static_cast<float>(top);
			return true;
		}

		void ApplyFloatAlignToken(UINT& align, std::wstring_view token)
		{
			if (token.empty()) {
				return;
			}

			if (IsAttributeName(token, L"null")) {
				align = 0;
			}
			else if (IsAttributeName(token, L"left")) {
				align &= ~(DT_CENTER | DT_RIGHT);
				align |= DT_LEFT;
			}
			else if (IsAttributeName(token, L"center")) {
				align &= ~(DT_LEFT | DT_RIGHT);
				align |= DT_CENTER;
			}
			else if (IsAttributeName(token, L"right")) {
				align &= ~(DT_LEFT | DT_CENTER);
				align |= DT_RIGHT;
			}
			else if (IsAttributeName(token, L"top")) {
				align &= ~(DT_BOTTOM | DT_VCENTER);
				align |= DT_TOP;
			}
			else if (IsAttributeName(token, L"vcenter")) {
				align &= ~(DT_TOP | DT_BOTTOM);
				align |= DT_VCENTER;
			}
			else if (IsAttributeName(token, L"bottom")) {
				align &= ~(DT_TOP | DT_VCENTER);
				align |= DT_BOTTOM;
			}
		}

		UINT ParseFloatAlign(std::wstring_view text, UINT currentAlign)
		{
			UINT align = currentAlign;
			size_t index = 0;
			while (index < text.size()) {
				while (index < text.size() && (text[index] == L',' || std::iswspace(text[index]) != 0)) {
					++index;
				}

				const size_t tokenStart = index;
				while (index < text.size() && text[index] != L',' && std::iswspace(text[index]) == 0) {
					++index;
				}

				ApplyFloatAlignToken(align, text.substr(tokenStart, index - tokenStart));
			}

			return align;
		}

		RECT ResolveScaledRect(CPaintManagerUI* manager, ULONGLONG currentDpi, const RECT& logicalRect, RECT& scaledRect, ULONGLONG& scaledGeneration)
		{
			if (manager == NULL) {
				return logicalRect;
			}

			if (currentDpi != scaledGeneration) {
				scaledRect = logicalRect;
				manager->ScaleRect(&scaledRect);
				scaledGeneration = currentDpi;
			}

			return scaledRect;
		}

		SIZE ResolveScaledSize(CPaintManagerUI* manager, ULONGLONG currentDpi, const SIZE& logicalSize, SIZE& scaledSize, ULONGLONG& scaledGeneration)
		{
			if (manager == NULL) {
				return logicalSize;
			}

			if (currentDpi != scaledGeneration) {
				scaledSize = logicalSize;
				manager->ScaleSize(&scaledSize);
				scaledGeneration = currentDpi;
			}

			return scaledSize;
		}
	}

	IMPLEMENT_DUICONTROL(CControlUI)

		CControlUI::CControlUI()
		:m_pManager(NULL), 
		m_pParent(NULL), 
		m_bUpdateNeeded(true),
		m_bMenuUsed(false),
		m_bVisible(true), 
		m_bInternVisible(true),
		m_bFocused(false),
		m_bEnabled(true),
		m_bMouseEnabled(true),
		m_bKeyboardEnabled(true),
		m_bFloat(false),
		m_uFloatAlign(0),
		m_bSetPos(false),
		m_bRichEvent(false),
		m_bDragEnabled(false),
		m_bDropEnabled(false),
		m_bResourceText(false),
		m_chShortcut('\0'),
		m_pTag(NULL),
		m_dwBackColor(0),
		m_dwBackColor2(0),
		m_dwBackColor3(0),
		m_dwForeColor(0),
		m_dwBorderColor(0),
		m_dwFocusBorderColor(0),
		m_dwFocusBkColor(0),
		m_bColorHSL(false),
		m_nBorderSize(0),
		m_nBorderStyle(PS_SOLID),
		m_nTooltipWidth(300),
		m_wCursor(0),
		m_instance(NULL)
	{
		m_cXY.cx = m_cXY.cy = 0;
		m_cxyFixed.cx = m_cxyFixed.cy = 0;
		m_cxyMin.cx = m_cxyMin.cy = 0;
		m_cxyMax.cx = m_cxyMax.cy = 9999999;
		m_cxyBorderRound.cx = m_cxyBorderRound.cy = 0;

		m_cXYScaled.cx = m_cXYScaled.cy = 0;
		m_cxyFixedScaled.cx = m_cxyFixedScaled.cy = 0;
		m_cxyMinScaled.cx = m_cxyMinScaled.cy =0;
		m_cxyMaxScaled.cx = m_cxyMaxScaled.cy=0;

		::ZeroMemory(&m_rcPadding, sizeof(RECT));
		::ZeroMemory(&m_rcItem, sizeof(RECT));
		::ZeroMemory(&m_rcPaint, sizeof(RECT));
		::ZeroMemory(&m_rcBorderSize,sizeof(RECT));
		::ZeroMemory(&m_rcPaddingScaled, sizeof(RECT)); // 鉁?鍒濆鍖栫紦瀛?
		m_piFloatPercent.left = m_piFloatPercent.top = m_piFloatPercent.right = m_piFloatPercent.bottom = 0.0f;
	}

	CControlUI::~CControlUI()
	{
		if( OnDestroy ) OnDestroy(this);
		RemoveAllCustomAttribute();	
		if( m_pManager != NULL ) m_pManager->ReapObjects(this);
	}

	const std::wstring& CControlUI::GetName() const
	{
		return m_sName;
	}

	const std::wstring& CControlUI::GetNameRef() const
	{
		return m_sName;
	}

	std::wstring_view CControlUI::GetNameView() const
	{
		return m_sName;
	}

	void CControlUI::SetName(std::wstring_view name)
	{
		m_sName.assign(name);
	}

	LPVOID CControlUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_CONTROL) == 0) return this;
		return NULL;
	}

	std::wstring_view CControlUI::GetClass() const
	{
		return _T("ControlUI");
	}

	UINT CControlUI::GetControlFlags() const
	{
		return 0;
	}

	bool CControlUI::Activate()
	{
		//if( !IsVisible() ) return false;
		if( !IsEnabled() ) return false;
		return true;
	}

	CPaintManagerUI* CControlUI::GetManager() const
	{
		return m_pManager;
	}

	void CControlUI::SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit)
	{
		m_pManager = pManager;
		m_pParent = pParent;
		m_uPaddingDpiGeneration = 0;
		m_uXYDpiGeneration = 0;
		m_uFixedDpiGeneration = 0;
		m_uMinDpiGeneration = 0;
		m_uMaxDpiGeneration = 0;
		if( bInit && m_pParent ) Init();
	}

	CControlUI* CControlUI::GetParent() const
	{
		return m_pParent;
	}

	void CControlUI::setInstance(HINSTANCE instance)
	{
		m_instance = instance;
	}

	bool CControlUI::IsContainerControl() const
	{
		return false;
	}


	void CControlUI::RemoveAll (bool bChildDelayed)
	{
	}

	bool CControlUI::SetTimer(UINT nTimerID, UINT nElapse)
	{
		if(m_pManager == NULL) return false;

		return m_pManager->SetTimer(this, nTimerID, nElapse);
	}

	void CControlUI::KillTimer(UINT nTimerID)
	{
		if(m_pManager == NULL) return;

		m_pManager->KillTimer(this, nTimerID);
	}

	std::wstring CControlUI::GetText() const
	{
		if (!IsResourceText()) return m_sText;
		return CResourceManager::GetInstance()->GetText(m_sText);
	}

	void CControlUI::SetText(std::wstring_view text)
	{
		if (m_sText == text) return;

		m_sText.assign(text);
		// 瑙ｆ瀽xml灞炴€?
		StringUtil::ReplaceAll(m_sText, _T("{\\n}"), _T("\n"));
		Invalidate();
	}
	std::wstring CControlUI::GetHotText() const
	{
		if (!IsResourceText()) return m_sHotText.empty() ? GetText() : m_sHotText;
		return CResourceManager::GetInstance()->GetText(m_sHotText);
		/*auto temp_text = m_sHotText.empty() ? GetText() : m_sHotText;
		CPaintManagerUI::ProcessMultiLanguageTokens(temp_text);
		return temp_text;*/
	}


	void CControlUI::SetHotText(std::wstring_view text)
	{
		if (m_sHotText == text) return;

		m_sHotText.assign(text);
		Invalidate();
	}

	bool CControlUI::IsResourceText() const
	{
		return m_bResourceText;
	}

	void CControlUI::SetResourceText(bool bResource)
	{
		if( m_bResourceText == bResource ) return;
		m_bResourceText = bResource;
		Invalidate();
	}

	bool CControlUI::IsDragEnabled() const
	{
		return m_bDragEnabled;
	}

	void CControlUI::SetDragEnable(bool bDrag)
	{
		m_bDragEnabled = bDrag;
	}

	bool CControlUI::IsDropEnabled() const
	{
		return m_bDropEnabled;
	}

	void CControlUI::SetDropEnable(bool bDrop)
	{
		m_bDropEnabled = bDrop;
	}


	bool CControlUI::IsRichEvent() const
	{
		return m_bRichEvent;
	}

	void CControlUI::SetRichEvent(bool bEnable)
	{
		m_bRichEvent = bEnable;
	}

	void CControlUI::SetGradient(std::wstring_view image)
	{
		if (m_sGradient == image) return;

		m_sGradient.assign(image);
		Invalidate();
	}

	std::wstring_view CControlUI::GetGradient() const
	{
		return m_sGradient;
	}

	DWORD CControlUI::GetBkColor() const
	{
		return m_dwBackColor;
	}

	void CControlUI::SetBkColor(DWORD dwBackColor, bool bIsUpdate)
	{
		if( m_dwBackColor == dwBackColor ) return;

		m_dwBackColor = dwBackColor;
		if(bIsUpdate)
			Invalidate();
	}

	DWORD CControlUI::GetBkColor2() const
	{
		return m_dwBackColor2;
	}

	void CControlUI::SetBkColor2(DWORD dwBackColor)
	{
		if( m_dwBackColor2 == dwBackColor ) return;

		m_dwBackColor2 = dwBackColor;
		Invalidate();
	}

	DWORD CControlUI::GetBkColor3() const
	{
		return m_dwBackColor3;
	}

	void CControlUI::SetBkColor3(DWORD dwBackColor)
	{
		if( m_dwBackColor3 == dwBackColor ) return;

		m_dwBackColor3 = dwBackColor;
		Invalidate();
	}

	void CControlUI::SetDisableBkColor(DWORD dwDisableBkColor)
	{
		m_dwDisableBkColor = dwDisableBkColor;
	}

	DWORD CControlUI::GetDisableBkColor() const
	{
		return m_dwDisableBkColor;
	}

	DWORD CControlUI::GetForeColor() const
	{
		return m_dwForeColor;
	}

	void CControlUI::SetForeColor(DWORD dwForeColor)
	{
		if( m_dwForeColor == dwForeColor ) return;

		m_dwForeColor = dwForeColor;
		Invalidate();
	}

	void CControlUI::SetBkImage(std::wstring_view image)
	{
		if (m_sBkImage == image) return;

		m_sBkImage.assign(image);
		Invalidate();
	}

	std::wstring_view CControlUI::GetBkImage() const
	{
		return m_sBkImage;
	}

	void CControlUI::SetForeImage(std::wstring_view image)
	{
		if (m_sForeImage == image) return;

		m_sForeImage.assign(image);
		Invalidate();
	}

	std::wstring_view CControlUI::GetForeImage() const
	{
		return m_sForeImage;
	}


	DWORD CControlUI::GetBorderColor() const
	{
		return m_dwBorderColor;
	}

	void CControlUI::SetBorderColor(DWORD dwBorderColor)
	{
		if( m_dwBorderColor == dwBorderColor ) return;

		m_dwBorderColor = dwBorderColor;
		Invalidate();
	}

	DWORD CControlUI::GetFocusBorderColor() const
	{
		return m_dwFocusBorderColor;
	}

	void CControlUI::SetFocusBorderColor(DWORD dwBorderColor)
	{
		if( m_dwFocusBorderColor == dwBorderColor ) return;

		m_dwFocusBorderColor = dwBorderColor;
		Invalidate();
	}

	DWORD CControlUI::GetFocusBKColor() const
	{
		return m_dwFocusBkColor;
	}

	void CControlUI::SetFocusBKColor(DWORD dwFocusBkColor)
	{
		m_dwFocusBkColor= dwFocusBkColor;
	}

	bool CControlUI::IsColorHSL() const
	{
		return m_bColorHSL;
	}

	void CControlUI::SetColorHSL(bool bColorHSL)
	{
		if( m_bColorHSL == bColorHSL ) return;

		m_bColorHSL = bColorHSL;
		Invalidate();
	}

	int CControlUI::GetBorderSize() const
	{
		if(m_pManager != NULL) return m_pManager->ScaleValue(m_nBorderSize);
		return m_nBorderSize;
	}

	void CControlUI::SetBorderSize(int nSize)
	{
		if( m_nBorderSize == nSize ) return;

		m_nBorderSize = nSize;
		Invalidate();
	}

	RECT CControlUI::GetBorderRectSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcBorderSize);
		return rcBorderSize;
	}

	void CControlUI::SetBorderSize( RECT rc )
	{
		m_rcBorderSize = rc;
		Invalidate();
	}

	SIZE CControlUI::GetBorderRound() const
	{
		SIZE cxyBorderRound = m_cxyBorderRound;
		if(m_pManager != NULL) m_pManager->ScaleSize(&cxyBorderRound);
		return cxyBorderRound;
	}

	void CControlUI::SetBorderRound(SIZE cxyRound)
	{
		m_cxyBorderRound = cxyRound;
		Invalidate();
	}

	bool CControlUI::DrawImage(CPaintRenderContext& renderContext, std::wstring_view pStrImage, std::wstring_view pStrModify)
	{
		return CRenderEngine::DrawImageString(
			renderContext,
			m_rcItem,
			std::wstring(pStrImage),
			pStrModify.empty() ? std::wstring() : std::wstring(pStrModify),
			m_instance);
	}

	RECT CControlUI::GetRelativePos() const
	{
		CControlUI* pParent = GetParent();
		if( pParent != NULL ) {
			RECT rcParentPos = pParent->GetPos();
			CDuiRect rcRelativePos(m_rcItem);
			rcRelativePos.Offset(-rcParentPos.left, -rcParentPos.top);
			return rcRelativePos;
		}
		else {
			return CDuiRect(0, 0, 0, 0);
		}
	}

	RECT CControlUI::GetClientPos() const 
	{
		return m_rcItem;
	}
	const RECT& CControlUI::GetPos() const
	{
		return m_rcItem;
	}

	void CControlUI::SetOnlyPos(RECT& rc)
	{
		m_rcItem = rc;
	}


	void CControlUI::SetPos(RECT rc, bool bNeedInvalidate)
	{

		if( rc.right < rc.left ) rc.right = rc.left;
		if( rc.bottom < rc.top ) rc.bottom = rc.top;

		CDuiRect invalidateRc={0,0,0,0};
		if (bNeedInvalidate)
		{
			invalidateRc = m_rcItem;
			if( ::IsRectEmpty(&invalidateRc) ) invalidateRc = rc;
		}

		m_rcItem = rc;
		if (m_rcItem.right - m_rcItem.left< GetFixedWidth())
		{
			m_rcItem.right= m_rcItem.left+ GetFixedWidth();
		}
		if( m_pManager == NULL ) return;

		if( !m_bSetPos ) {
			m_bSetPos = true;
			if( OnSize ) OnSize(this);
			m_bSetPos = false;
		}

		m_bUpdateNeeded = false;

		if( bNeedInvalidate && IsVisible() ) {
			invalidateRc.Join(m_rcItem);
			CControlUI* pParent = this;
			RECT rcTemp;
			RECT rcParent;
			while( pParent = pParent->GetParent() ) {
				if( !pParent->IsVisible() ) return;
				rcTemp = invalidateRc;
				rcParent = pParent->GetPos();
				if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) return;
			}
			m_pManager->Invalidate(invalidateRc);
		}
	}


	void CControlUI::Move(SIZE szOffset, bool bNeedInvalidate)
	{
		m_cXY.cx += szOffset.cx;
		m_cXY.cy += szOffset.cy;
		NeedParentUpdate();
	}

	int CControlUI::GetWidth() const
	{
		return m_rcItem.right - m_rcItem.left;
	}

	int CControlUI::GetHeight() const
	{
		return m_rcItem.bottom - m_rcItem.top;
	}

	int CControlUI::GetX() const
	{
		return m_rcItem.left;
	}

	int CControlUI::GetY() const
	{
		return m_rcItem.top;
	}

	RECT CControlUI::GetPadding() const
	{
		return ResolveScaledRect(
			m_pManager,
			GetCurrentDPIGeneration(),
			m_rcPadding,
			m_rcPaddingScaled,
			m_uPaddingDpiGeneration);
	}

	SIZE CControlUI::GetFixedXY() const
	{
		return ResolveScaledSize(
			m_pManager,
			GetCurrentDPIGeneration(),
			m_cXY,
			m_cXYScaled,
			m_uXYDpiGeneration);
	}

	SIZE CControlUI::GetFixedSize() const
	{
		return ResolveScaledSize(
			m_pManager,
			GetCurrentDPIGeneration(),
			m_cxyFixed,
			m_cxyFixedScaled,
			m_uFixedDpiGeneration);
	}

	int CControlUI::GetFixedWidth() const
	{
		return GetFixedSize().cx;
	}

	int CControlUI::GetFixedHeight() const
	{
		return GetFixedSize().cy;
	}

	int CControlUI::GetMinWidth() const
	{
		return ResolveScaledSize(
			m_pManager,
			GetCurrentDPIGeneration(),
			m_cxyMin,
			m_cxyMinScaled,
			m_uMinDpiGeneration).cx;
	}

	int CControlUI::GetMinHeight() const
	{
		return ResolveScaledSize(
			m_pManager,
			GetCurrentDPIGeneration(),
			m_cxyMin,
			m_cxyMinScaled,
			m_uMinDpiGeneration).cy;
	}

	int CControlUI::GetMaxWidth() const
	{
		return ResolveScaledSize(
			m_pManager,
			GetCurrentDPIGeneration(),
			m_cxyMax,
			m_cxyMaxScaled,
			m_uMaxDpiGeneration).cx;
	}

	int CControlUI::GetMaxHeight() const
	{
		return ResolveScaledSize(
			m_pManager,
			GetCurrentDPIGeneration(),
			m_cxyMax,
			m_cxyMaxScaled,
			m_uMaxDpiGeneration).cy;
	}

	void CControlUI::SetPadding(RECT rcPadding, bool NeedUpdate)
	{
		if(rcPadding.left == m_rcPadding.left && rcPadding.top == m_rcPadding.top &&
			rcPadding.right == m_rcPadding.right && rcPadding.bottom == m_rcPadding.bottom) 
			return;

		m_rcPadding = rcPadding;
		m_uPaddingDpiGeneration = 0;
		if (NeedUpdate)
		{
			NeedParentUpdate();
		}
	}

	void CControlUI::SetFixedXY(SIZE szXY)
	{
		if (szXY.cx == m_cXY.cx && szXY.cy == m_cXY.cy)
			return;
		m_cXY.cx = szXY.cx;
		m_cXY.cy = szXY.cy;
		m_uXYDpiGeneration = 0;
		NeedParentUpdate();
	}



	void CControlUI::SetFixedWidth(int cx, bool NeedUpdate)
	{
		if( cx < 0 || m_cxyFixed.cx == cx) return;
		m_cxyFixed.cx = cx;
		m_uFixedDpiGeneration = 0;
		if (NeedUpdate)
		{
			NeedParentUpdate();
		}
	}



	void CControlUI::SetFixedHeight(int cy, bool NeedUpdate)
	{
		if( cy < 0 || m_cxyFixed.cy == cy) return;
		m_cxyFixed.cy = cy;
		m_uFixedDpiGeneration = 0;
		if (NeedUpdate)
		{
			NeedParentUpdate();
		}
	}

	int CControlUI::ScaleValue(int value) const
	{
		if (m_pManager != NULL) {
			return m_pManager->ScaleValue(value);
		}
		return value;
	}

	RECT CControlUI::ScaleRect(RECT rc) const
	{
		if (m_pManager != NULL) {
			return m_pManager->ScaleRect(rc);
		}
		return rc;
	}

	SIZE CControlUI::ScaleSize(SIZE sz) const
	{
		if (m_pManager != NULL) {
			return m_pManager->ScaleSize(sz);
		}
		return sz;
	}

	POINT CControlUI::ScalePoint(POINT pt) const
	{
		if (m_pManager != NULL) {
			return m_pManager->ScalePoint(pt);
		}
		return pt;
	}

	ULONGLONG CControlUI::GetCurrentDPIGeneration() const
	{
		if (m_pManager != NULL) {
			return m_pManager->GetDPIGeneration();
		}
		return 0;
	}

	int CControlUI::PixelsToLogical(int value) const
	{
		if (m_pManager != NULL) {
			return m_pManager->UnscaleValue(value);
		}
		return value;
	}

	RECT CControlUI::PixelsToLogical(RECT rc) const
	{
		if (m_pManager != NULL) {
			return m_pManager->UnscaleRect(rc);
		}
		return rc;
	}

	SIZE CControlUI::PixelsToLogical(SIZE sz) const
	{
		if (m_pManager != NULL) {
			return m_pManager->UnscaleSize(sz);
		}
		return sz;
	}

	POINT CControlUI::PixelsToLogical(POINT pt) const
	{
		if (m_pManager != NULL) {
			return m_pManager->UnscalePoint(pt);
		}
		return pt;
	}

	void CControlUI::SetPaddingFromPixels(RECT rcPadding, bool NeedUpdate)
	{
		SetPadding(PixelsToLogical(rcPadding), NeedUpdate);
	}

	void CControlUI::SetFixedWidthFromPixels(int cx, bool NeedUpdate)
	{
		SetFixedWidth(PixelsToLogical(cx), NeedUpdate);
	}

	void CControlUI::SetFixedHeightFromPixels(int cy, bool NeedUpdate)
	{
		SetFixedHeight(PixelsToLogical(cy), NeedUpdate);
	}



	void CControlUI::SetMinWidth(int cx)
	{
		if( m_cxyMin.cx == cx ) return;
		if( cx < 0 ) return; 
		m_uMinDpiGeneration = 0;
		m_cxyMin.cx = cx;
		NeedParentUpdate();
	}



	void CControlUI::SetMaxWidth(int cx)
	{
		if( m_cxyMax.cx == cx ) return;
		if( cx < 0 ) return; 
		m_uMaxDpiGeneration = 0;
		m_cxyMax.cx = cx;
		NeedParentUpdate();
	}



	void CControlUI::SetMinHeight(int cy)
	{
		if( m_cxyMin.cy == cy ) return;
		if( cy < 0 ) return; 
		m_uMinDpiGeneration = 0;
		m_cxyMin.cy = cy;
		NeedParentUpdate();
	}



	void CControlUI::SetMaxHeight(int cy)
	{
		if( m_cxyMax.cy == cy ) return;
		if( cy < 0 ) return; 
		m_uMaxDpiGeneration = 0;
		m_cxyMax.cy = cy;
		NeedParentUpdate();
	}

	TPercentInfo CControlUI::GetFloatPercent() const
	{
		return m_piFloatPercent;
	}

	void CControlUI::SetFloatPercent(TPercentInfo piFloatPercent)
	{
		m_piFloatPercent = piFloatPercent;
		NeedParentUpdate();
	}

	void CControlUI::SetFloatAlign(UINT uAlign)
	{
		m_uFloatAlign = uAlign;
		NeedParentUpdate();
	}

	UINT CControlUI::GetFloatAlign() const
	{
		return m_uFloatAlign;
	}

	std::wstring CControlUI::GetToolTip() const
	{
		if (!IsResourceText()) return m_sToolTip;
		return CResourceManager::GetInstance()->GetText(m_sToolTip);
	}

	void CControlUI::SetToolTip(std::wstring_view text)
	{
		std::wstring strTemp(text);
		StringUtil::ReplaceAll(strTemp, _T("<n>"), _T("\r\n"));
		m_sToolTip = std::move(strTemp);
	}

	void CControlUI::SetToolTipWidth( int nWidth )
	{
		m_nTooltipWidth = nWidth;
	}

	int CControlUI::GetToolTipWidth( void )
	{
		if(m_pManager != NULL) return m_pManager->ScaleValue(m_nTooltipWidth);
		return m_nTooltipWidth;
	}

	void CControlUI::SetToolTipGap(SIZE szGap)
	{
		m_szGap = szGap;
	}

	SIZE CControlUI::GetToolTipGap()
	{
		SIZE szGap = m_szGap;
		if (m_pManager != NULL)
		{
			m_pManager->ScaleSize(&szGap);
			return szGap;
		}
		return m_szGap;
	}

	void CControlUI::SetToolTipShowMode(ToolTipType emToolType)
	{
		m_emToolTipType = emToolType;
	}

	ToolTipType CControlUI::GetToolTipShowMode()
	{
		return m_emToolTipType;
	}

	WORD CControlUI::GetCursor()
	{
		return m_wCursor;
	}

	HCURSOR CControlUI::GetHCursor()
	{
		return m_Cursor;
	}

	void CControlUI::SetCursor(WORD wCursor)
	{
		if(m_wCursor== wCursor)
			return;
		m_wCursor = wCursor;
		m_Cursor = nullptr;
		NeedUpdate();
	}

	void CControlUI::SetCursor(HCURSOR hCursor)
	{
		if (hCursor == m_Cursor)
			return;
		m_Cursor = hCursor;
		NeedUpdate();
	}

	wchar_t CControlUI::GetShortcut() const
	{
		return m_chShortcut;
	}

	void CControlUI::SetShortcut(wchar_t ch)
	{
		m_chShortcut = ch;
	}

	bool CControlUI::IsContextMenuUsed() const
	{
		return m_bMenuUsed;
	}

	void CControlUI::SetContextMenuUsed(bool bMenuUsed)
	{
		m_bMenuUsed = bMenuUsed;
	}

	const std::wstring& CControlUI::GetUserData() const
	{
		return m_sUserData;
	}

	std::wstring_view CControlUI::GetUserDataView() const
	{
		return m_sUserData;
	}

	void CControlUI::SetUserData(std::wstring_view text)
	{
		m_sUserData.assign(text);
	}

	void CControlUI::SetMapUserData(const std::wstring& strTextFirst, const std::wstring& strTextSecond)
	{
		if (strTextFirst.empty() || strTextSecond.empty()) return;
		m_mapUserData[strTextFirst] = strTextSecond;
	}

	std::wstring& CControlUI::GetMapUserData(const std::wstring& strTextFirst)
	{
		auto it = m_mapUserData.find(strTextFirst);
		if (it != m_mapUserData.end()) 
			return it->second;
	
		static std::wstring empty;
		return empty;
	}

	void CControlUI::ClearMapUserData()
	{
		m_mapUserData.clear();
	}

	void CControlUI::EraseMapUserData(const std::wstring& strTextFirst)
	{
		auto it = m_mapUserData.find(strTextFirst);
		if (it != m_mapUserData.end()) {
			m_mapUserData.erase(it);
		}
	}



	UINT_PTR CControlUI::GetTag() const
	{
		return m_pTag;
	}

	void CControlUI::SetTag(UINT_PTR pTag)
	{
		m_pTag = pTag;
	}



	bool CControlUI::IsVisible()
	{
		return m_bVisible && m_bInternVisible;
	}

	bool CControlUI::GetVisible() const
	{
		return m_bVisible;
	}

	void CControlUI::SetVisible(bool bVisible, bool bSendFocus)
	{

		if( m_bVisible == bVisible ) return;

		bool v = IsVisible();
		m_bVisible = bVisible;
		if( m_bFocused ) m_bFocused = false;
		if (!bVisible && m_pManager && m_pManager->GetFocus() == this && bSendFocus) {
			m_pManager->SetFocus(NULL) ;
		}
		if( IsVisible() != v ) {
			NeedParentUpdate();
		}
	}

	void CControlUI::SetInternVisible(bool bVisible)
	{
		m_bInternVisible = bVisible;
		if (!bVisible && m_pManager && m_pManager->GetFocus() == this) {
			m_pManager->SetFocus(NULL) ;
		}
	}

	bool CControlUI::IsEnabled() const
	{
		return m_bEnabled;
	}

	void CControlUI::SetEnabled(bool bEnabled)
	{
		if( m_bEnabled == bEnabled ) return;

		m_bEnabled = bEnabled;
		Invalidate();
	}

	bool CControlUI::IsMouseEnabled() const
	{
		return m_bMouseEnabled;
	}

	void CControlUI::SetMouseEnabled(bool bEnabled)
	{
		m_bMouseEnabled = bEnabled;
	}

	bool CControlUI::IsKeyboardEnabled() const
	{
		return m_bKeyboardEnabled ;
	}
	void CControlUI::SetKeyboardEnabled(bool bEnabled)
	{
		m_bKeyboardEnabled = bEnabled ; 
	}

	bool CControlUI::IsFocused() const
	{
		return m_bFocused;
	}

	void CControlUI::SetFocus()
	{
		if( m_pManager != NULL ) m_pManager->SetFocus(this);
	}


	void CControlUI::SetFloat(bool bFloat)
	{
		if( m_bFloat == bFloat ) return;

		m_bFloat = bFloat;
		NeedParentUpdate();
	}

	bool CControlUI::IsFloat() const
	{
		return m_bFloat;
	}

	CControlUI* CControlUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
	{
		if( (uFlags & UIFIND_VISIBLE) != 0 && !IsVisible() ) return NULL;
		if( (uFlags & UIFIND_ENABLED) != 0 && !IsEnabled() ) return NULL;
		if( (uFlags & UIFIND_HITTEST) != 0 && (!m_bMouseEnabled || !::PtInRect(&m_rcItem, * static_cast<LPPOINT>(pData))) ) return NULL;
		return Proc(this, pData);
	}

	CControlUI* CControlUI::Clone() 
	{
		CControlUI* pClone = new CControlUI();
		// 澶嶅埗鍩烘湰灞炴€?
		pClone->CopyData(this);

		return pClone;
	}

	void CControlUI::CopyData(CControlUI* pControl) 
	{
		SetName(pControl->m_sName);
		SetText(pControl->m_sText);
		SetHotText(pControl->m_sHotText);
		SetToolTip(pControl->m_sToolTip);
		SetUserData(pControl->m_sUserData);
		SetTag(pControl->m_pTag);

		// 澶嶅埗浣嶇疆鍜屽ぇ灏?
		SetFixedXY(pControl->m_cXY);
		SetFixedWidth(pControl->m_cxyFixed.cx, false);
		SetFixedHeight(pControl->m_cxyFixed.cy, false);
		SetMinWidth(pControl->m_cxyMin.cx);
		SetMinHeight(pControl->m_cxyMin.cy);
		SetMaxWidth(pControl->m_cxyMax.cx);
		SetMaxHeight(pControl->m_cxyMax.cy);
		SetPadding(pControl->m_rcPadding, false);

		m_rcPaddingScaled = pControl->m_rcPaddingScaled ;
		m_cXYScaled = pControl->m_cXYScaled;
		m_cxyFixedScaled = pControl->m_cxyFixedScaled ;
		m_cxyMinScaled = pControl->m_cxyMinScaled;
		m_cxyMaxScaled = pControl->m_cxyMaxScaled;
		m_uPaddingDpiGeneration = 0;
		m_uXYDpiGeneration = 0;
		m_uFixedDpiGeneration = 0;
		m_uMinDpiGeneration = 0;
		m_uMaxDpiGeneration = 0;
		m_bUpdateNeeded = pControl->m_bUpdateNeeded;
		m_rcItem = pControl->m_rcItem;
		m_instance= pControl->m_instance;
		m_wCursor = pControl->m_wCursor;
		m_Cursor = pControl->m_Cursor;

		// 澶嶅埗棰滆壊鍜屽浘鍍?
		SetBkColor(pControl->m_dwBackColor, false);
		SetBkColor2(pControl->m_dwBackColor2);
		SetBkColor3(pControl->m_dwBackColor3);
		SetForeColor(pControl->m_dwForeColor);
		SetBkImage(pControl->m_sBkImage);
		SetForeImage(pControl->m_sForeImage);
		SetGradient(pControl->m_sGradient);

		// 澶嶅埗杈规灞炴€?
		SetBorderColor(pControl->m_dwBorderColor);
		SetFocusBorderColor(pControl->m_dwFocusBorderColor);
		SetFocusBKColor(pControl->m_dwFocusBkColor);
		SetBorderSize(pControl->m_rcBorderSize);
		SetBorderSize(pControl->m_nBorderSize);
		SetBorderRound(pControl->m_cxyBorderRound);
		SetBorderStyle(pControl->m_nBorderStyle);


		// 澶嶅埗鐘舵€佹爣蹇?
		SetVisible(pControl->m_bVisible, false);
		m_bVisible = pControl->m_bVisible;
		SetEnabled(pControl->m_bEnabled);
		SetMouseEnabled(pControl->m_bMouseEnabled);
		SetKeyboardEnabled(pControl->m_bKeyboardEnabled);
		SetFloat(pControl->m_bFloat);
		SetFloatPercent(pControl->m_piFloatPercent);
		SetFloatAlign(pControl->m_uFloatAlign);

		// 澶嶅埗鍏朵粬灞炴€?
		SetResourceText(pControl->m_bResourceText);
		SetDragEnable(pControl->m_bDragEnabled);
		SetDropEnable(pControl->m_bDropEnabled);
		SetRichEvent(pControl->m_bRichEvent);
		SetShortcut(pControl->m_chShortcut);

		SetToolTipWidth(pControl->m_nTooltipWidth);
		SetContextMenuUsed(pControl->m_bMenuUsed);
		SetVirtualWnd(pControl->m_sVirtualWnd);
		SetToolTipGap(pControl->m_szGap);
		SetToolTipShowMode(pControl->m_emToolTipType);

		// 澶嶅埗鑷畾涔夊睘鎬?
		for (auto& pair : pControl->m_mapUserData) {
			SetMapUserData(pair.first, pair.second);
		}

		// 澶嶅埗鑷畾涔夊睘鎬у搱甯岃〃
		for (const auto& pair : pControl->m_mCustomAttrHash) {
			AddCustomAttribute(pair.first, pair.second);
		}
	}

	void CControlUI::Invalidate()
	{
		if( !IsVisible() ) return;

		RECT invalidateRc = m_rcItem;

		CControlUI* pParent = this;
		RECT rcTemp;
		RECT rcParent;
		while( pParent = pParent->GetParent() )
		{
			rcTemp = invalidateRc;
			rcParent = pParent->GetPos();
			if( !::IntersectRect(&invalidateRc, &rcTemp, &rcParent) ) 
			{
				return;
			}
		}

		if( m_pManager != NULL ) m_pManager->Invalidate(invalidateRc);
	}

	bool CControlUI::IsUpdateNeeded() const
	{
		return m_bUpdateNeeded;
	}

	void CControlUI::NeedUpdate()
	{
		if( !IsVisible() ) return;
		m_bUpdateNeeded = true;
		Invalidate();

		if( m_pManager != NULL ) m_pManager->NeedUpdate();
	}

	void CControlUI::NeedParentUpdate()
	{
		if( GetParent() ) {
			GetParent()->NeedUpdate();
			GetParent()->Invalidate();
		}
		else {
			NeedUpdate();
		}

		if( m_pManager != NULL ) m_pManager->NeedUpdate();
	}

	DWORD CControlUI::GetAdjustColor(DWORD dwColor)
	{
		if( !m_bColorHSL ) return dwColor;
		short H, S, L;
		CPaintManagerUI::GetHSL(&H, &S, &L);
		return CRenderEngine::AdjustColor(dwColor, H, S, L);
	}

	void CControlUI::Init()
	{
		DoInit();
		if( OnInit ) OnInit(this);
	}

	void CControlUI::DoInit()
	{

	}

	void CControlUI::Event(TEventUI& event)
	{
		if( OnEvent(&event) ) DoEvent(event);
	}

	void CControlUI::DoEvent(TEventUI& event)
	{
		if( event.Type == UIEVENT_SETCURSOR )
		{
			if (GetHCursor())
			{
				::SetCursor(GetHCursor());
			}
			else
			{
				if (GetCursor())
				{
					::SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(GetCursor())));
				}
				else
				{
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
				}
			}

			return;
		}

		if( event.Type == UIEVENT_SETFOCUS ) 
		{
			m_bFocused = true;
			Invalidate();
			return;
		}
		if( event.Type == UIEVENT_KILLFOCUS ) 
		{
			m_bFocused = false;
			Invalidate();
			return;
		}
		if( event.Type == UIEVENT_TIMER )
		{
			m_pManager->SendNotify(this, DUI_MSGTYPE_TIMER, event.wParam, event.lParam);
			return;
		}
		if( event.Type == UIEVENT_CONTEXTMENU )
		{
			if( IsContextMenuUsed() ) {
				m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
				return;
			}
		}

		if( m_pParent != NULL ) m_pParent->DoEvent(event);
	}


	void CControlUI::SetVirtualWnd(std::wstring_view value)
	{
		m_sVirtualWnd.assign(value);
		if(m_pManager!=nullptr)
			m_pManager->UsedVirtualWnd(true);
	}

	std::wstring_view CControlUI::GetVirtualWnd() const
	{
		if (!m_sVirtualWnd.empty()) {
			return m_sVirtualWnd;
		}

		CControlUI* pParent = GetParent();
		if (pParent != NULL) {
			return pParent->GetVirtualWnd();
		}

		return std::wstring_view();
	}

	void CControlUI::AddCustomAttribute(std::wstring_view name, std::wstring_view attr)
	{
		if (name.empty() || attr.empty()) return;

		m_mCustomAttrHash.try_emplace(std::wstring(name), attr);
	}

	std::wstring_view CControlUI::GetCustomAttribute(std::wstring_view name) const
	{
		if (name.empty()) return std::wstring_view();
		const auto it = m_mCustomAttrHash.find(name);
		if (it != m_mCustomAttrHash.end()) {
			return it->second;
		}
		return std::wstring_view();
	}

	bool CControlUI::RemoveCustomAttribute(std::wstring_view name)
	{
		if (name.empty()) return false;
		const auto it = m_mCustomAttrHash.find(name);
		if (it == m_mCustomAttrHash.end()) return false;
		m_mCustomAttrHash.erase(it);
		return true;
	}

	void CControlUI::RemoveAllCustomAttribute()
	{
		m_mCustomAttrHash.clear();
	}

		void CControlUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView)
	{
		const std::wstring_view name = StringUtil::TrimView(pstrNameView);
		if (name.empty()) {
			return;
		}

		if (m_pManager != NULL && IsAttributeName(name, L"style")) {
			const std::wstring_view style = m_pManager->GetStyle(pstrValueView);
			if (!style.empty()) {
				ApplyAttributeList(style);
				return;
			}
		}

		if (IsAttributeName(name, L"innerstyle")) {
			ApplyAttributeList(pstrValueView);
		}
		else if (IsAttributeName(name, L"pos")) {
			RECT position = { 0 };
			if (StringUtil::TryParseRect(pstrValueView, position)) {
				SetFixedXY({ position.left, position.top });
				SetFixedWidth(abs(position.right - position.left));
				SetFixedHeight(abs(position.bottom - position.top));
			}
		}
		else if (IsAttributeName(name, L"xy")) {
			SIZE position = { 0, 0 };
			if (StringUtil::TryParseSize(pstrValueView, position)) {
				SetFixedXY(position);
			}
		}
		else if (IsAttributeName(name, L"float")) {
			if (StringUtil::Find(pstrValueView, L',') < 0) {
				SetFloat(StringUtil::ParseBool(pstrValueView));
			}
			else {
				TPercentInfo floatPercent = { 0 };
				if (TryParseFloatPercent(pstrValueView, floatPercent)) {
					SetFloatPercent(floatPercent);
					SetFloat(true);
				}
			}
		}
		else if (IsAttributeName(name, L"floatalign")) {
			SetFloatAlign(ParseFloatAlign(pstrValueView, GetFloatAlign()));
		}
		else if (IsAttributeName(name, L"padding")) {
			RECT padding = { 0 };
			if (StringUtil::TryParseRect(pstrValueView, padding)) {
				SetPadding(padding);
			}
		}
		else if (IsAttributeName(name, L"gradient")) SetGradient(pstrValueView);
		else if (IsAttributeName(name, L"tooltipgap")) {
			SIZE gap = { 0, 0 };
			if (StringUtil::TryParseSize(pstrValueView, gap)) {
				SetToolTipGap(gap);
			}
		}
		else if (IsAttributeName(name, L"tooltipmode")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) {
				if (value == 0) SetToolTipShowMode(Tool_Left);
				else if (value == 1) SetToolTipShowMode(Tool_Top);
				else if (value == 2) SetToolTipShowMode(Tool_Right);
				else if (value == 3) SetToolTipShowMode(Tool_Bottom);
			}
		}
		else if (IsAttributeName(name, L"bkcolor") || IsAttributeName(name, L"bkcolor1")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetBkColor(color);
		}
		else if (IsAttributeName(name, L"bkcolor2")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetBkColor2(color);
		}
		else if (IsAttributeName(name, L"bkcolor3")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetBkColor3(color);
		}
		else if (IsAttributeName(name, L"disabledbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetDisableBkColor(color);
		}
		else if (IsAttributeName(name, L"focusbkcolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetFocusBKColor(color);
		}
		else if (IsAttributeName(name, L"forecolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetForeColor(color);
		}
		else if (IsAttributeName(name, L"bordercolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetBorderColor(color);
		}
		else if (IsAttributeName(name, L"focusbordercolor")) {
			DWORD color = 0;
			if (StringUtil::TryParseColor(pstrValueView, color)) SetFocusBorderColor(color);
		}
		else if (IsAttributeName(name, L"colorhsl")) SetColorHSL(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"bordersize")) {
			if (StringUtil::Find(pstrValueView, L',') < 0) {
				int borderSize = 0;
				if (StringUtil::TryParseInt(pstrValueView, borderSize)) {
					SetBorderSize(borderSize);
					SetBorderSize(RECT{ 0, 0, 0, 0 });
				}
			}
			else {
				RECT borderSize = { 0 };
				if (StringUtil::TryParseRect(pstrValueView, borderSize)) {
					SetBorderSize(borderSize);
				}
			}
		}
		else if (IsAttributeName(name, L"leftbordersize")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetLeftBorderSize(value);
		}
		else if (IsAttributeName(name, L"topbordersize")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetTopBorderSize(value);
		}
		else if (IsAttributeName(name, L"rightbordersize")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetRightBorderSize(value);
		}
		else if (IsAttributeName(name, L"bottombordersize")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetBottomBorderSize(value);
		}
		else if (IsAttributeName(name, L"borderstyle")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetBorderStyle(value);
		}
		else if (IsAttributeName(name, L"borderround")) {
			SIZE round = { 0, 0 };
			if (StringUtil::TryParseSize(pstrValueView, round)) {
				SetBorderRound(round);
			}
		}
		else if (IsAttributeName(name, L"bkimage")) SetBkImage(pstrValueView);
		else if (IsAttributeName(name, L"foreimage")) SetForeImage(pstrValueView);
		else if (IsAttributeName(name, L"width")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetFixedWidth(value);
		}
		else if (IsAttributeName(name, L"height")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetFixedHeight(value);
		}
		else if (IsAttributeName(name, L"minwidth")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetMinWidth(value);
		}
		else if (IsAttributeName(name, L"minheight")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetMinHeight(value);
		}
		else if (IsAttributeName(name, L"maxwidth")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetMaxWidth(value);
		}
		else if (IsAttributeName(name, L"maxheight")) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValueView, value)) SetMaxHeight(value);
		}
		else if (IsAttributeName(name, L"name")) SetName(pstrValueView);
		else if (IsAttributeName(name, L"drag")) SetDragEnable(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"drop")) SetDropEnable(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"resourcetext")) SetResourceText(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"richevent")) SetRichEvent(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"text")) SetText(pstrValueView);
		else if (IsAttributeName(name, L"tooltip")) SetToolTip(pstrValueView);
		else if (IsAttributeName(name, L"userdata")) SetUserData(pstrValueView);
		else if (IsAttributeName(name, L"enabled")) SetEnabled(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"mouse")) SetMouseEnabled(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"keyboard")) SetKeyboardEnabled(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"visible")) SetVisible(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"shortcut")) {
			if (!pstrValueView.empty()) {
				SetShortcut(pstrValueView.front());
			}
		}
		else if (IsAttributeName(name, L"menu")) SetContextMenuUsed(StringUtil::ParseBool(pstrValueView));
		else if (IsAttributeName(name, L"cursor") && !pstrValueView.empty()) {
			if (IsAttributeName(pstrValueView, L"arrow")) SetCursor(DUI_ARROW);
			else if (IsAttributeName(pstrValueView, L"ibeam")) SetCursor(DUI_IBEAM);
			else if (IsAttributeName(pstrValueView, L"wait")) SetCursor(DUI_WAIT);
			else if (IsAttributeName(pstrValueView, L"cross")) SetCursor(DUI_CROSS);
			else if (IsAttributeName(pstrValueView, L"uparrow")) SetCursor(DUI_UPARROW);
			else if (IsAttributeName(pstrValueView, L"size")) SetCursor(DUI_SIZE);
			else if (IsAttributeName(pstrValueView, L"icon")) SetCursor(DUI_ICON);
			else if (IsAttributeName(pstrValueView, L"sizenwse")) SetCursor(DUI_SIZENWSE);
			else if (IsAttributeName(pstrValueView, L"sizenesw")) SetCursor(DUI_SIZENESW);
			else if (IsAttributeName(pstrValueView, L"sizewe")) SetCursor(DUI_SIZEWE);
			else if (IsAttributeName(pstrValueView, L"sizens")) SetCursor(DUI_SIZENS);
			else if (IsAttributeName(pstrValueView, L"sizeall")) SetCursor(DUI_SIZEALL);
			else if (IsAttributeName(pstrValueView, L"no")) SetCursor(DUI_NO);
			else if (IsAttributeName(pstrValueView, L"hand")) SetCursor(DUI_HAND);
		}
		else if (IsAttributeName(name, L"virtualwnd")) SetVirtualWnd(pstrValueView);
		else {
			AddCustomAttribute(name, pstrValueView);
		}
	}

	CControlUI* CControlUI::ApplyAttributeList(std::wstring_view pstrValue)
	{
		// 閻熸瑱绲鹃悗浠嬪冀瀹勬壆纭€閻?
		if(m_pManager != NULL) {
			const std::wstring_view pStyle = m_pManager->GetStyle(pstrValue);
			if (!pStyle.empty()) {
				return ApplyAttributeList(pStyle);
			}
		}
		std::wstring sXmlData(pstrValue);
		StringUtil::ReplaceAll(sXmlData, _T("&quot;"), _T("\""));
		StringUtil::ReplaceAll(sXmlData, _T("\r"), _T(" "));
		StringUtil::ReplaceAll(sXmlData, _T("\n"), _T(" "));
		StringUtil::ReplaceAll(sXmlData, _T("\t"), _T(" "));
		StringUtil::Trim(sXmlData);

		std::wstring_view attributes = sXmlData;
		size_t cursor = 0;
		while (cursor < attributes.size()) {
			while (cursor < attributes.size() && std::iswspace(attributes[cursor]) != 0) {
				++cursor;
			}
			if (cursor >= attributes.size()) {
				break;
			}

			const size_t nameStart = cursor;
			while (cursor < attributes.size() && attributes[cursor] != L'=' && std::iswspace(attributes[cursor]) == 0) {
				++cursor;
			}
			const size_t nameEnd = cursor;

			while (cursor < attributes.size() && std::iswspace(attributes[cursor]) != 0) {
				++cursor;
			}
			if (cursor >= attributes.size() || attributes[cursor] != L'=') {
				return this;
			}
			++cursor;

			while (cursor < attributes.size() && std::iswspace(attributes[cursor]) != 0) {
				++cursor;
			}
			if (cursor >= attributes.size() || attributes[cursor] != L'\"') {
				return this;
			}
			++cursor;

			const size_t valueStart = cursor;
			const size_t valueEnd = attributes.find(L'\"', cursor);
			if (valueEnd == std::wstring_view::npos) {
				return this;
			}

			SetAttribute(StringUtil::TrimView(attributes.substr(nameStart, nameEnd - nameStart)),
				attributes.substr(valueStart, valueEnd - valueStart));
			cursor = valueEnd + 1;

			if (cursor < attributes.size() && attributes[cursor] != L' ' && attributes[cursor] != L',') 
				return this;
			while (cursor < attributes.size() && (attributes[cursor] == L' ' || attributes[cursor] == L',')) 
			{
				++cursor;
			}
		}
		return this;
	}

	SIZE CControlUI::EstimateSize(SIZE szAvailable)
	{
		if(m_pManager != NULL)
			return m_pManager->ScaleSize(m_cxyFixed);
		return m_cxyFixed;
	}

	bool CControlUI::Paint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		if (pStopControl == this) return false;
		if (!::IntersectRect(&m_rcPaint, &renderContext.GetPaintRect(), &m_rcItem)) return true;
		if (!DoPaint(renderContext, pStopControl)) return false;
		return true;
	}

	bool CControlUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
	{
		SIZE cxyBorderRound = GetBorderRound();
		RECT rcBorderSize = GetBorderRectSize();
		(void)rcBorderSize;

		if( cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0 ) {
			PaintBkColor(renderContext);
			{
				CRenderClip roundClip;
				CRenderClip::GenerateRoundClip(renderContext, m_rcPaint,  m_rcItem, cxyBorderRound.cx, cxyBorderRound.cy, roundClip);
				PaintBkImage(renderContext);
				PaintStatusImage(renderContext);
				PaintForeColor(renderContext);
				PaintForeImage(renderContext);
				PaintText(renderContext);
			}
			PaintBorder(renderContext);
		}
		else {
			PaintBkColor(renderContext);
			PaintBkImage(renderContext);
			PaintStatusImage(renderContext);
			PaintForeColor(renderContext);
			PaintForeImage(renderContext);
			PaintText(renderContext);
			PaintBorder(renderContext);
		}
		return true;
	}

	void CControlUI::PaintBkColor(CPaintRenderContext& renderContext)
	{
		SIZE cxyBorderRound = GetBorderRound();
		if (IsEnabled() == false)
		{
			if (m_dwDisableBkColor != 0)
			{
				DrawRoundedAwareSolidColorForPaint(renderContext, m_rcItem, m_rcPaint, cxyBorderRound, GetAdjustColor(m_dwDisableBkColor));
				return;
			}
		}
		if (IsFocused() && GetFocusBKColor() != 0)
		{
			DrawRoundedAwareSolidColorForPaint(renderContext, m_rcItem, m_rcPaint, cxyBorderRound, GetAdjustColor(GetFocusBKColor()));
		}
		else
		{
			if (m_dwBackColor != 0)
			{
				bool bVer = (m_sGradient.compare(_T("hor")) != 0);
				if (m_dwBackColor2 != 0)
				{
					CRenderClip roundClip;
					if (HasRoundCorner(cxyBorderRound)) {
						CRenderClip::GenerateRoundClip(renderContext, m_rcPaint, m_rcItem, cxyBorderRound.cx, cxyBorderRound.cy, roundClip);
					}
					if (m_dwBackColor3 != 0)
					{
						RECT rc = m_rcItem;
						rc.bottom = (rc.bottom + rc.top) / 2;
						CRenderEngine::DrawGradient(renderContext, rc, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 8);
						rc.top = rc.bottom;
						rc.bottom = m_rcItem.bottom;
						CRenderEngine::DrawGradient(renderContext, rc, GetAdjustColor(m_dwBackColor2), GetAdjustColor(m_dwBackColor3), bVer, 8);
					}
					else
					{
						CRenderEngine::DrawGradient(renderContext, m_rcItem, GetAdjustColor(m_dwBackColor), GetAdjustColor(m_dwBackColor2), bVer, 16);
					}
				}
				else if (m_dwBackColor >= 0xFF000000 && m_rcPaint.right - m_rcPaint.left != 0)
					DrawRoundedAwareSolidColorForPaint(renderContext, m_rcItem, m_rcPaint, cxyBorderRound, GetAdjustColor(m_dwBackColor));
				else
					DrawRoundedAwareColor(renderContext, m_rcItem, cxyBorderRound, GetAdjustColor(m_dwBackColor));
			}
		}
	}

	void CControlUI::PaintBkImage(CPaintRenderContext& renderContext)
	{
		if( m_sBkImage.empty() ) return;
		if( !DrawImage(renderContext, m_sBkImage) ) {}
	}

	void CControlUI::PaintStatusImage(CPaintRenderContext& renderContext)
	{
		return;
	}

	void CControlUI::PaintForeColor(CPaintRenderContext& renderContext)
	{
		CRenderEngine::DrawColor(renderContext, m_rcItem, GetAdjustColor(m_dwForeColor));
	}

	void CControlUI::PaintForeImage(CPaintRenderContext& renderContext)
	{
		if( m_sForeImage.empty() ) return;
		DrawImage(renderContext, m_sForeImage);
	}

	void CControlUI::PaintText(CPaintRenderContext& renderContext)
	{
		return;
	}

	void CControlUI::PaintBorder(CPaintRenderContext& renderContext)
	{
		int nBorderSize = GetBorderSize();
		SIZE cxyBorderRound = GetBorderRound();
		RECT rcBorderSize = GetBorderRectSize();
		int nBorderBand = nBorderSize;
		if (rcBorderSize.left > nBorderBand) nBorderBand = rcBorderSize.left;
		if (rcBorderSize.top > nBorderBand) nBorderBand = rcBorderSize.top;
		if (rcBorderSize.right > nBorderBand) nBorderBand = rcBorderSize.right;
		if (rcBorderSize.bottom > nBorderBand) nBorderBand = rcBorderSize.bottom;

		if(m_dwBorderColor != 0 || m_dwFocusBorderColor != 0) {
			if (!PaintRectTouchesBorderBand(m_rcPaint, m_rcItem, nBorderBand + 1)) {
				return;
			}
			if(nBorderSize > 0 && ( cxyBorderRound.cx > 0 || cxyBorderRound.cy > 0 )) {
				if (IsFocused() && m_dwFocusBorderColor != 0)
					CRenderEngine::DrawRoundRect(renderContext, m_rcItem, nBorderSize, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(m_dwFocusBorderColor), m_nBorderStyle);
				else
					CRenderEngine::DrawRoundRect(renderContext, m_rcItem, nBorderSize, cxyBorderRound.cx, cxyBorderRound.cy, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
			}
			else {
				if (IsFocused() && m_dwFocusBorderColor != 0 && nBorderSize > 0) {
					CRenderEngine::DrawRect(renderContext, m_rcItem, nBorderSize, GetAdjustColor(m_dwFocusBorderColor), m_nBorderStyle);
				}
				else if(rcBorderSize.left > 0 || rcBorderSize.top > 0 || rcBorderSize.right > 0 || rcBorderSize.bottom > 0) {
					RECT rcBorder;

					if(rcBorderSize.left > 0){
						rcBorder = m_rcItem;
						rcBorder.right = rcBorder.left;
						CRenderEngine::DrawLine(renderContext, rcBorder, rcBorderSize.left, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
					}
					if(rcBorderSize.top > 0){
						rcBorder = m_rcItem;
						rcBorder.bottom = rcBorder.top;
						CRenderEngine::DrawLine(renderContext, rcBorder, rcBorderSize.top, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
					}
					if(rcBorderSize.right > 0){
						rcBorder = m_rcItem;
						rcBorder.right -= 1;
						rcBorder.left = rcBorder.right;
						CRenderEngine::DrawLine(renderContext, rcBorder, rcBorderSize.right, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
					}
					if(rcBorderSize.bottom > 0){
						rcBorder = m_rcItem;
						rcBorder.bottom -= 1;
						rcBorder.top = rcBorder.bottom;
						CRenderEngine::DrawLine(renderContext, rcBorder, rcBorderSize.bottom, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
					}
				}
				else if(nBorderSize > 0) {
					CRenderEngine::DrawRect(renderContext, m_rcItem, nBorderSize, GetAdjustColor(m_dwBorderColor), m_nBorderStyle);
				}
			}
		}
	}

	void CControlUI::DoPostPaint(CPaintRenderContext& renderContext)
	{
		(void)renderContext;
		return;
	}

	int CControlUI::GetLeftBorderSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcBorderSize);

		return rcBorderSize.left;
	}

	void CControlUI::SetLeftBorderSize( int nSize )
	{
		m_rcBorderSize.left = nSize;
		Invalidate();
	}

	int CControlUI::GetTopBorderSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcBorderSize);

		return rcBorderSize.top;
	}

	void CControlUI::SetTopBorderSize( int nSize )
	{
		m_rcBorderSize.top = nSize;
		Invalidate();
	}

	int CControlUI::GetRightBorderSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcBorderSize);

		return rcBorderSize.right;
	}

	void CControlUI::SetRightBorderSize( int nSize )
	{
		m_rcBorderSize.right = nSize;
		Invalidate();
	}

	int CControlUI::GetBottomBorderSize() const
	{
		RECT rcBorderSize = m_rcBorderSize;
		if(m_pManager != NULL) m_pManager->ScaleRect(&rcBorderSize);

		return rcBorderSize.bottom;
	}

	void CControlUI::SetBottomBorderSize( int nSize )
	{
		m_rcBorderSize.bottom = nSize;
		Invalidate();
	}

	int CControlUI::GetBorderStyle() const
	{
		return m_nBorderStyle;
	}

	void CControlUI::SetBorderStyle( int nStyle )
	{
		m_nBorderStyle = nStyle;
		Invalidate();
	}

} // namespace DuiLib


