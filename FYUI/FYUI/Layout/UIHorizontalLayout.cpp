#include "pch.h"
#include "UIHorizontalLayout.h"
#include "UILayoutLinearUtil.h"
#include <cmath>

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CHorizontalLayoutUI)
		CHorizontalLayoutUI::CHorizontalLayoutUI() : m_iSepWidth(0), m_uButtonState(0), m_bImmMode(false)
	{
		ptLastMouse.x = ptLastMouse.y = 0;
		::ZeroMemory(&m_rcNewPos, sizeof(m_rcNewPos));
	}

	std::wstring_view CHorizontalLayoutUI::GetClass() const
	{
		return _T("HorizontalLayoutUI");
	}

	LPVOID CHorizontalLayoutUI::GetInterface(std::wstring_view pstrName)
	{
		if (StringUtil::CompareNoCase(pstrName, DUI_CTR_HORIZONTALLAYOUT) == 0) return static_cast<CHorizontalLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	UINT CHorizontalLayoutUI::GetControlFlags() const
	{
		if( (IsEnabled() && m_iSepWidth != 0) || m_bCursorMouse ==false) 
			return UIFLAG_SETCURSOR;
		else return 0;
	}

	void CHorizontalLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{

		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = m_rcItem;

		// Adjust for inset
		RECT rcInset = GetInset();
		rc.left += rcInset.left;
		rc.top += rcInset.top;
		rc.right -= rcInset.right;
		rc.bottom -= rcInset.bottom;


		const bool bHasVerticalScroll = m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible();
		const bool bHasHorizontalScroll = m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible();

		if( !m_bScrollFloat && bHasVerticalScroll )
			rc.right -= m_pVerticalScrollBar->GetFixedWidth();
		if( !m_bScrollFloat && bHasHorizontalScroll )
			rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

		const int nItemCount = m_items.GetSize();
		if( nItemCount == 0) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}


		const int iChildPadding = GetChildPadding();
		const UINT iChildAlign = GetChildAlign();
		const UINT iChildVAlign = GetChildVAlign();

		// Determine the minimum size
		SIZE szAvailable = { rc.right - rc.left, rc.bottom - rc.top };
		if( bHasVerticalScroll ) 
			szAvailable.cy += m_pVerticalScrollBar->GetScrollRange();


		std::vector<LinearLayoutInfo> layoutInfos;
		layoutInfos.reserve(nItemCount);  // ✅ 预分配避免动态扩容

		int cyNeeded = 0;
		int nAdjustables = 0;
		int cxFixed = 0;
		int nEstimateNum = 0;


		for( int it1 = 0; it1 < nItemCount; it1++ ) 
		{
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it1]);

			LinearLayoutInfo info;
			CollectLinearLayoutInfo(pControl, szAvailable, LinearLayoutAxis::Horizontal, info, cxFixed, cyNeeded, nAdjustables, nEstimateNum);
			layoutInfos.push_back(info);
		}

		cxFixed += (nEstimateNum - 1) * iChildPadding;

		// Place elements
		int cxNeeded = 0;
		int cxExpand = 0;
		if( nAdjustables > 0 ) 
			cxExpand = MAX(0, (szAvailable.cx - cxFixed) / nAdjustables);

		// Position the elements
		SIZE szRemaining = szAvailable;
		int iPosX = rc.left;


		if( bHasHorizontalScroll ) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
		}
		else {
			// 子控件横向对齐方式
			if(nAdjustables <= 0) {
				if (iChildAlign == DT_CENTER) {
					iPosX += (szAvailable.cx - cxFixed) / 2;
				}
				else if (iChildAlign == DT_RIGHT) {
					iPosX += (szAvailable.cx - cxFixed);
				}
			}
		}


		int iScrollPosY = 0;
		int iScrollRangeY = 0;
		if( bHasVerticalScroll ) {
			iScrollPosY = m_pVerticalScrollBar->GetScrollPos();
			iScrollRangeY = m_pVerticalScrollBar->GetScrollRange();
		}

		int iAdjustable = 0;
		int cxFixedRemaining = cxFixed;


		for( int it2 = 0; it2 < nItemCount; it2++ ) 
		{
			LinearLayoutInfo& info = layoutInfos[it2];

			if( !info.bVisible ) continue;

			if( info.bFloat ) {
				SetFloatPos(it2);
				continue;
			}

			szRemaining.cx -= info.rcPadding.left;

			SIZE szControlAvailable = szRemaining;
			szControlAvailable.cy -= info.rcPadding.top + info.rcPadding.bottom;

			if (szControlAvailable.cx > info.iControlMaxWidth) 
				szControlAvailable.cx = info.iControlMaxWidth;
			if (szControlAvailable.cy > info.iControlMaxHeight) 
				szControlAvailable.cy = info.iControlMaxHeight;

			cxFixedRemaining -= (info.rcPadding.left + info.rcPadding.right);
			if (it2 > 0) cxFixedRemaining -= iChildPadding;


			if( info.sz.cx == 0 ) {
				iAdjustable++;
				info.sz.cx = cxExpand;

				if( iAdjustable == nAdjustables ) {
					info.sz.cx = MAX(0, szRemaining.cx - info.rcPadding.right - cxFixedRemaining);
				}

				const int minWidth = info.pControl->GetMinWidth();
				const int maxWidth = info.pControl->GetMaxWidth();
				if( info.sz.cx < minWidth ) info.sz.cx = minWidth;
				if( info.sz.cx > maxWidth ) info.sz.cx = maxWidth;
			}
			else {
				cxFixedRemaining -= info.sz.cx;
			}

			info.sz.cy = MAX(info.sz.cy, 0);
			if( info.sz.cy == 0 )
				info.sz.cy = szAvailable.cy - info.rcPadding.top - info.rcPadding.bottom;
			if( info.sz.cy < 0 ) info.sz.cy = 0;
			if( info.sz.cy > szControlAvailable.cy )
				info.sz.cy = szControlAvailable.cy;

			const int minHeight = info.pControl->GetMinHeight();
			if( info.sz.cy < minHeight ) info.sz.cy = minHeight;


			RECT rcCtrl;
			switch(iChildVAlign)
			{
			case DT_VCENTER: {
				int iPosY = (rc.bottom + rc.top) / 2;
				if( bHasVerticalScroll ) {
					iPosY += iScrollRangeY / 2;
					iPosY -= iScrollPosY;
				}
				rcCtrl = { 
					iPosX + info.rcPadding.left, 
					iPosY - info.sz.cy/2, 
					iPosX + info.sz.cx + info.rcPadding.left, 
					iPosY + info.sz.cy - info.sz.cy/2 
				};
				break;
			}
			case DT_BOTTOM: {
				int iPosY = rc.bottom;
				if( bHasVerticalScroll ) {
					iPosY += iScrollRangeY;
					iPosY -= iScrollPosY;
				}
				rcCtrl = { 
					iPosX + info.rcPadding.left, 
					iPosY - info.rcPadding.bottom - info.sz.cy, 
					iPosX + info.sz.cx + info.rcPadding.left, 
					iPosY - info.rcPadding.bottom 
				};
				break;
			}
			default: { // DT_TOP
				int iPosY = rc.top;
				if( bHasVerticalScroll ) {
					iPosY -= iScrollPosY;
				}
				rcCtrl = { 
					iPosX + info.rcPadding.left, 
					iPosY + info.rcPadding.top, 
					iPosX + info.sz.cx + info.rcPadding.left, 
					iPosY + info.sz.cy + info.rcPadding.top 
				};
				break;
			}
			}

			info.pControl->SetPos(rcCtrl, false);

			iPosX += info.sz.cx + iChildPadding + info.rcPadding.left + info.rcPadding.right;
			cxNeeded += info.sz.cx + info.rcPadding.left + info.rcPadding.right;
			szRemaining.cx -= info.sz.cx + iChildPadding + info.rcPadding.right;
		}

		cxNeeded += (nEstimateNum - 1) * iChildPadding;

		// Process the scrollbar
		ProcessScrollBar(rc, cxNeeded, cyNeeded);
	}



	void CHorizontalLayoutUI::DoPostPaint(CPaintRenderContext& renderContext)
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode ) {
			RECT rcSeparator = GetThumbRect(true);
			CRenderEngine::DrawColor(renderContext, rcSeparator, 0xAA000000);
		}
	}

	void CHorizontalLayoutUI::SetSepWidth(int iWidth)
	{
		m_iSepWidth = iWidth;
	}

	int CHorizontalLayoutUI::GetSepWidth() const
	{
		return m_iSepWidth;
	}

	void CHorizontalLayoutUI::SetSepImmMode(bool bImmediately)
	{
		if( m_bImmMode == bImmediately ) return;
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && !m_bImmMode && m_pManager != NULL ) {
			m_pManager->RemovePostPaint(this);
		}

		m_bImmMode = bImmediately;
	}

	bool CHorizontalLayoutUI::IsSepImmMode() const
	{
		return m_bImmMode;
	}

	void CHorizontalLayoutUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("sepwidth")) == 0 ) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetSepWidth(value);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("sepimm")) == 0 ) SetSepImmMode(StringUtil::ParseBool(pstrValue));
		else if( StringUtil::CompareNoCase(pstrName, _T("sepimmbordercolor")) == 0 ) 
		{
			DWORD clrColor = 0;
			if (StringUtil::TryParseColor(pstrValue, clrColor)) {
				SetSepImmBorderColor(clrColor);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("sepimmleavebordercolor")) == 0 ) 
		{
			DWORD clrColor = 0;
			if (StringUtil::TryParseColor(pstrValue, clrColor)) {
				SetSepImmLeaveBorderColor(clrColor);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("animation_show_hide")) == 0 )
	{
		if (StringUtil::CompareNoCase(pstrValue, _T("left")) == 0)
			SetShowHideAnimDir(AnimLeft);
		else if (StringUtil::CompareNoCase(pstrValue, _T("right")) == 0)
			SetShowHideAnimDir(AnimRight);
		else
			SetShowHideAnimDir(AnimNone);
	}
	else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	void CHorizontalLayoutUI::DoEvent(TEventUI& event)
	{

		if( m_iSepWidth != 0 ) {
			if( event.Type == UIEVENT_BUTTONDOWN && IsEnabled() )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( ::PtInRect(&rcSeparator, event.ptMouse) ) {
					m_uButtonState |= UISTATE_CAPTURED;
					ptLastMouse = event.ptMouse;
					m_rcNewPos = m_rcItem;
					if( !m_bImmMode && m_pManager ) m_pManager->AddPostPaint(this);
					return;
				}
			}
			if( event.Type == UIEVENT_BUTTONUP )
			{
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					m_uButtonState &= ~UISTATE_CAPTURED;
					m_rcItem = m_rcNewPos;
					if( !m_bImmMode && m_pManager ) m_pManager->RemovePostPaint(this);
					if (m_bImmMode)
					{
						m_pManager->SendNotify(this, DUI_MSGTYPE_SPLITMOVE_UP);
					}
					NeedParentUpdate();
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSEMOVE )
			{
				if (m_bImmMode)
				{
					RECT rcSeparator = GetThumbRect(false);
					if (::PtInRect(&rcSeparator, event.ptMouse))
					{
						::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
						if (GetSepImmBorderColor() != 0)
						{
							SetBorderColor(GetSepImmBorderColor());
						}
					}
				}
				if( (m_uButtonState & UISTATE_CAPTURED) != 0 ) {
					LONG cx = event.ptMouse.x - ptLastMouse.x;
					ptLastMouse = event.ptMouse;
					RECT rc = m_rcNewPos;
					if( m_iSepWidth >= 0 ) {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.right - m_iSepWidth ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.right ) return;
						rc.right += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.right = rc.left + GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.right = rc.left + GetMaxWidth();
						}
					}
					else {
						if( cx > 0 && event.ptMouse.x < m_rcNewPos.left ) return;
						if( cx < 0 && event.ptMouse.x > m_rcNewPos.left - m_iSepWidth ) return;
						rc.left += cx;
						if( rc.right - rc.left <= GetMinWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left <= GetMinWidth() ) return;
							rc.left = rc.right - GetMinWidth();
						}
						if( rc.right - rc.left >= GetMaxWidth() ) {
							if( m_rcNewPos.right - m_rcNewPos.left >= GetMaxWidth() ) return;
							rc.left = rc.right - GetMaxWidth();
						}
					}

					CDuiRect rcInvalidate = GetThumbRect(true);
					m_rcNewPos = rc;
					SetFixedWidthFromPixels(m_rcNewPos.right - m_rcNewPos.left, false);
					//不可以直接设置m_cxyFixed.cx，因为会导致Control 获取 m_cxyFixedScaled不准确 需要SetFixedWidth
					//m_cxyFixed.cx = m_pManager->UnscaleValue(m_rcNewPos.right - m_rcNewPos.left);

					if( m_bImmMode ) {
						m_rcItem = m_rcNewPos;
						NeedParentUpdate();
						m_pManager->SendNotify(this, DUI_MSGTYPE_SPLITMOVE);
					}
					else {
						rcInvalidate.Join(GetThumbRect(true));
						rcInvalidate.Join(GetThumbRect(false));
						if( m_pManager ) m_pManager->Invalidate(rcInvalidate);
					}
					return;
				}
			}
			if( event.Type == UIEVENT_MOUSELEAVE )
			{
				if (m_bImmMode)
				{
					if (GetSepImmLeaveBorderColor() != 0)
					{
						SetBorderColor(GetSepImmLeaveBorderColor());
					}
				}
			}
			if( event.Type == UIEVENT_SETCURSOR )
			{
				RECT rcSeparator = GetThumbRect(false);
				if( IsEnabled() && ::PtInRect(&rcSeparator, event.ptMouse) ) {
					::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
					return;
				}
			}
		}

		// 显隐动画定时器
		if (event.Type == UIEVENT_TIMER && event.wParam == SHOWHIDE_ANIM_TIMERID) {
			AdvanceShowHideAnim();
			return;
		}

		CContainerUI::DoEvent(event);
	}

	RECT CHorizontalLayoutUI::GetThumbRect(bool bUseNew) const
	{
		if( (m_uButtonState & UISTATE_CAPTURED) != 0 && bUseNew) {
			if( m_iSepWidth >= 0 ) return CDuiRect(m_rcNewPos.right - m_iSepWidth, m_rcNewPos.top, m_rcNewPos.right, m_rcNewPos.bottom);
			else return CDuiRect(m_rcNewPos.left, m_rcNewPos.top, m_rcNewPos.left - m_iSepWidth, m_rcNewPos.bottom);
		}
		else {
			if( m_iSepWidth >= 0 ) 
				return CDuiRect(m_rcItem.right - m_iSepWidth, m_rcItem.top, m_rcItem.right, m_rcItem.bottom);
			else 
				return CDuiRect(m_rcItem.left, m_rcItem.top, m_rcItem.left - m_iSepWidth, m_rcItem.bottom);
		}
	}

	CHorizontalLayoutUI* CHorizontalLayoutUI::Clone()
	{
		CHorizontalLayoutUI* pClone = new CHorizontalLayoutUI();
		// 复制基本属性
		pClone->CopyData(this);

		return pClone;
	}

	void CHorizontalLayoutUI::CopyData(CHorizontalLayoutUI* pControl)
	{

		m_iSepWidth = pControl->m_iSepWidth;
		m_uButtonState = 0;
		ptLastMouse = pControl->ptLastMouse;
		m_rcNewPos = pControl->m_rcNewPos;
		m_bImmMode = pControl->m_bImmMode;
	m_animDir = pControl->m_animDir;
	__super::CopyData(pControl);
	}



	// ============================================================
	// 显隐动画实现
	// ============================================================

	void CHorizontalLayoutUI::SetShowHideAnimDir(ShowHideAnimDir dir)
	{
		if (m_animDir == dir) return;
		if (m_bAnimating) StopShowHideAnim();
		m_animDir = dir;
	}

	CHorizontalLayoutUI::ShowHideAnimDir CHorizontalLayoutUI::GetShowHideAnimDir() const
	{
		return m_animDir;
	}

	bool CHorizontalLayoutUI::IsShowHideAnimating() const
	{
		return m_bAnimating;
	}

	void CHorizontalLayoutUI::SetVisible(bool bVisible, bool bSendFocus)
	{
		// 无动画 → 直接走父类逻辑
		if (m_animDir == AnimNone) {
			CContainerUI::SetVisible(bVisible, bSendFocus);
			return;
		}

		// 已经在做相同方向动画，忽略
		if (m_bAnimating && m_bAnimShowing == bVisible)
			return;

		// 正在做反向动画，停止当前动画（保持当前尺寸）
		if (m_bAnimating) {
			StopShowHideAnim();
		}

		// 目标状态和当前状态相同，直接返回
		if (m_bVisible == bVisible)
			return;

		StartShowHideAnim(bVisible);
	}

	void CHorizontalLayoutUI::StartShowHideAnim(bool bShow)
	{
		if (m_pManager == nullptr)
			return;

		m_bAnimShowing = bShow;

		if (bShow) {
			// === 展开动画 ===
			// 1. 确定目标像素宽度
			if (m_nAnimTargetPx > 0) {
				// 沿用上次隐藏时记录的目标宽度
			}
			else if (m_nAnimOrigFixedW > 0) {
				// 有固定宽度，用缩放后的像素值
				m_nAnimTargetPx = m_pManager ? m_pManager->ScaleValue(m_nAnimOrigFixedW) : m_nAnimOrigFixedW;
			}
			else {
				// 默认回退值
				m_nAnimTargetPx = 200;
			}

			// 2. 先设宽度为 1（最小值），然后让控件可见
			m_nAnimCurrentPx = 1;
			SetFixedWidth(1, false);

			// 让控件可见
			CControlUI::SetVisible(true, false);
			for (int it = 0; it < m_items.GetSize(); it++)
				static_cast<CControlUI*>(m_items[it])->SetInternVisible(true);
			NeedParentUpdate();
		}
		else {
			// === 收起动画 ===
			// 1. 记录原始固定宽度（未缩放的逻辑值，0 表示自适应）
			// 重要修复点：GetFixedWidth() 返回的是 DPI 缩放后的像素值。如果我们在下面执行 SetFixedWidth()，
			// 传入缩放后的像素值会导致双重缩放。所以这里必须记录原始逻辑值 m_cxyFixed.cx。
			m_nAnimOrigFixedW = m_cxyFixed.cx;

			// 2. 记录当前实际像素宽度作为动画起点和目标
			m_nAnimCurrentPx = GetWidth();
			m_nAnimTargetPx = m_nAnimCurrentPx;

			if (m_nAnimCurrentPx <= 1) {
				// 控件本身就没宽度，直接隐藏
				CContainerUI::SetVisible(false, true);
				return;
			}
		}

		// 3. 启动定时器
		m_bAnimating = true;
		if (!m_pManager->SetTimer(this, SHOWHIDE_ANIM_TIMERID, SHOWHIDE_ANIM_INTERVAL_MS)) {
			StopShowHideAnim();
			if (!bShow) {
				CContainerUI::SetVisible(false, true);
			}
		}
	}

	void CHorizontalLayoutUI::AdvanceShowHideAnim()
	{
		if (!m_bAnimating || m_nAnimTargetPx <= 0)
			return;

		// 匀速动画：计算每帧步长
		// 动画总时长 200ms，帧间隔 16ms，约 12-13 帧
		const int totalFrames = 120 / SHOWHIDE_ANIM_INTERVAL_MS;
		int step = m_nAnimTargetPx / (totalFrames > 0 ? totalFrames : 1);
		if (step < 1) step = 1;

		// 接近阈值：当剩余距离小于步长的 2 倍时，直接跳到终点
		const int snapThreshold = step * 2;

		if (m_bAnimShowing) {
			// 展开：宽度增大
			const int remaining = m_nAnimTargetPx - m_nAnimCurrentPx;
			if (remaining <= snapThreshold) {
				// 接近目标 → 直接跳到最终宽度
				StopShowHideAnim();
				// 恢复原始固定宽度设置
				SetFixedWidth(m_nAnimOrigFixedW, true);
				return;
			}
			m_nAnimCurrentPx += step;
		}
		else {
			// 收起：宽度减小
			if (m_nAnimCurrentPx <= snapThreshold) {
				// 接近 0 → 直接隐藏
				StopShowHideAnim();
				// 恢复原始固定宽度
				// 因为之前 m_nAnimOrigFixedW 存的是未缩放的 m_cxyFixed.cx，这里直接设置回去就不会出错了
				SetFixedWidth(m_nAnimOrigFixedW, false);
				// 真正隐藏
				CControlUI::SetVisible(false, true);
				for (int it = 0; it < m_items.GetSize(); it++)
					static_cast<CControlUI*>(m_items[it])->SetInternVisible(false);
				NeedParentUpdate();
				return;
			}
			m_nAnimCurrentPx -= step;
			if (m_nAnimCurrentPx < 1) m_nAnimCurrentPx = 1;
		}

		// 这里有一个 DPI 和 自适应 的关键问题：
		// 如果 m_nAnimCurrentPx 只有 1px，如果 DPI>1.0，PixelsToLogical(1) 可能得到 0。
		// 一旦 SetFixedWidthFromPixels 把 m_cxyFixed.cx 设置为 0，布局引擎就会把它当做“自适应”控件。
		// 这个自适应控件将会瓜分父容器（例如外层VBox/HBox）剩余的全部空间！这会导致宽度突然变成非常大的值（例如 260）。
		// 为了防止 m_cxyFixed.cx 被无意中设置为 0，如果我们要设置 1 像素，就必须确保转换后的逻辑宽度至少为 1。
		int logicalW = m_pManager ? m_pManager->UnscaleValue(m_nAnimCurrentPx) : m_nAnimCurrentPx;
		if (logicalW <= 0 && m_nAnimCurrentPx > 0) logicalW = 1; // 强制最小值为 1 逻辑像素，绝不为 0 (自适应)!
		SetFixedWidth(logicalW, true);
	}

	void CHorizontalLayoutUI::StopShowHideAnim()
	{
		if (!m_bAnimating) return;
		m_bAnimating = false;
		if (m_pManager)
			m_pManager->KillTimer(this, SHOWHIDE_ANIM_TIMERID);
	}

}
