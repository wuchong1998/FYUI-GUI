#include "pch.h"
#include "UIContainer.h"
#include "Render/UIRenderContext.h"
#include <cmath>
#include <mmsystem.h>
#include <chrono>

#pragma comment(lib, "winmm.lib")

namespace FYUI 
{
    namespace
    {
        constexpr double kSmoothScrollResponse = 20.0;
        constexpr double kSmoothScrollMaxSpeed = 14000.0;
        constexpr double kSmoothScrollSnapDistance = 0.75;
        constexpr int kScrollCacheSafetyInset = 2;

        int ClampScrollDeltaToPixelStep(double distance, int currentPos)
        {
            if (std::fabs(distance) < kSmoothScrollSnapDistance) {
                return currentPos;
            }

            return currentPos + (distance > 0.0 ? 1 : -1);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    //
    //
    IMPLEMENT_DUICONTROL(CContainerUI)



        CContainerUI::CContainerUI()
        : m_iChildPadding(0),
        m_iChildAlign(DT_LEFT),
        m_iChildVAlign(DT_TOP),
        m_bAutoDestroy(true),
        m_bDelayedDestroy(true),
        m_bMouseChildEnabled(true),
        m_pVerticalScrollBar(NULL),
        m_pHorizontalScrollBar(NULL),
        m_nScrollStepSize(0),
        m_bFixedScrollbar(false),
        m_bSmoothScrollbar(true),
        m_bCursorMouse(true),
        m_bShowScrollbar(true) {
        ::ZeroMemory(&m_rcInset, sizeof(m_rcInset));
        m_animDir = AnimNone;
        m_bAnimating = false;
        m_bAnimShowing = false;
        m_nAnimTargetPx = 0;
        m_nAnimCurrentPx = 0;
        m_nAnimOrigFixedSize = 0;
        m_bScrollFloat = true;
        m_smoothScrollLastTick = std::chrono::steady_clock::now();
    }




    CContainerUI::~CContainerUI() {
        StopSmoothScroll(false);
        StopShowHideAnim();
        m_bDelayedDestroy = false;
        RemoveAll();
        if (m_pVerticalScrollBar) {
            delete m_pVerticalScrollBar;
            m_pVerticalScrollBar = NULL;
        }
        if (m_pHorizontalScrollBar) {
            delete m_pHorizontalScrollBar;
            m_pHorizontalScrollBar = NULL;
        }


    }

    std::wstring_view CContainerUI::GetClass() const {
        return _T("ContainerUI");
    }

    LPVOID CContainerUI::GetInterface(std::wstring_view pstrName) {
        if (StringUtil::CompareNoCase(pstrName, _T("IContainer")) == 0) {
            return static_cast<IContainerUI*>(this);
        }
        else if (StringUtil::CompareNoCase(pstrName, DUI_CTR_CONTAINER) == 0) {
            return static_cast<CContainerUI*>(this);
        }
        return CControlUI::GetInterface(pstrName);
    }

    CControlUI* CContainerUI::GetItemAt(int iIndex) const {
        if (iIndex < 0 || iIndex >= m_items.GetSize()) {
            return NULL;
        }
        return static_cast<CControlUI*>(m_items[iIndex]);
    }

    int CContainerUI::GetItemIndex(CControlUI* pControl) const
    {
        const int nCount = m_items.GetSize();
        for (int it = 0; it < m_items.GetSize(); it++)
        {
            if (static_cast<CControlUI*>(m_items[it]) == pControl)
            {
                return it;
            }
        }

        /*  if (nCount == 0) return -1;
        void** ppData = const_cast<CStdPtrArray&>(m_items).c_str();
        int i = 0;

        auto it = std::find(std::execution::par_unseq, ppData, ppData + nCount, pControl);
        if (it != ppData + nCount) {
        return std::distance(ppData, it);
        }*/
        return -1;
    }

    bool CContainerUI::SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate)
    {
        for (int it = 0; it < m_items.GetSize(); it++) {
            if (static_cast<CControlUI*>(m_items[it]) == pControl)
            {
                if (bUpdate)
                    NeedUpdate();
                m_items.Remove(it);
                return m_items.InsertAt(iIndex, pControl);
            }
        }

        return false;
    }

    int CContainerUI::GetCount() const {
        return m_items.GetSize();
    }

    bool CContainerUI::Add(CControlUI* pControl) {
        if (pControl == NULL) {
            return false;
        }

        if (m_pManager != NULL) {
            m_pManager->InitControls(pControl, this);
        }
        if (IsVisible()) {
            NeedUpdate();
        }
        else {
            pControl->SetInternVisible(false);
        }
        return m_items.Add(pControl);
    }

    bool CContainerUI::AddAt(CControlUI* pControl, int iIndex) {
        if (pControl == NULL) {
            return false;
        }

        if (m_pManager != NULL) {
            m_pManager->InitControls(pControl, this);
        }
        if (IsVisible()) {
            NeedUpdate();
        }
        else {
            pControl->SetInternVisible(false);
        }
        return m_items.InsertAt(iIndex, pControl);
    }

    bool CContainerUI::Remove(CControlUI* pControl, bool bChildDelayed) {
        if (pControl == NULL) {
            return false;
        }
        if (m_bAutoDestroy)
        {
            pControl->RemoveAll(bChildDelayed);
        }
        for (int it = 0; it < m_items.GetSize(); it++)
        {
            if (static_cast<CControlUI*>(m_items[it]) == pControl)
            {
                NeedUpdate();
                if (m_bAutoDestroy)
                {
                    if (m_bDelayedDestroy && m_pManager)
                    {
                        m_pManager->AddDelayedCleanup(pControl);
                    }
                    else
                    {
                        delete pControl;
                    }
                }
                return m_items.Remove(it);
            }
        }
        return false;
    }

    bool CContainerUI::RemoveAt(int iIndex, bool bChildDelayed) {
        CControlUI* pControl = GetItemAt(iIndex);
        if (pControl != NULL) {
            return CContainerUI::Remove(pControl, bChildDelayed);
        }

        return false;
    }

    void CContainerUI::RemoveAll(bool bChildDelayed)
    {
        for (int it = 0; m_bAutoDestroy && it < m_items.GetSize(); it++)
        {
            CControlUI* pItem = static_cast<CControlUI*>(m_items[it]);
            if (pItem->IsContainerControl())
            {
                pItem->RemoveAll(bChildDelayed);
            }

            if (bChildDelayed == false)
            {
                delete pItem;
                pItem = NULL;
            }
            else
            {

                if (m_bDelayedDestroy && m_pManager)
                {
                    m_pManager->AddDelayedCleanup(pItem);
                }
                else
                {
                    delete pItem;
                    pItem = NULL;
                }
            }

        }
        m_items.Empty();
        NeedUpdate();
    }

    bool CContainerUI::IsAutoDestroy() const {
        return m_bAutoDestroy;
    }

    void CContainerUI::SetAutoDestroy(bool bAuto) {
        m_bAutoDestroy = bAuto;
    }

    bool CContainerUI::IsDelayedDestroy() const {
        return m_bDelayedDestroy;
    }

    void CContainerUI::SetDelayedDestroy(bool bDelayed) {
        m_bDelayedDestroy = bDelayed;
    }



    RECT CContainerUI::GetInset() const {
        if (m_pManager) {
            return m_pManager->ScaleRect(m_rcInset);
        }
        return m_rcInset;
    }

    void CContainerUI::SetInset(RECT rcInset) {
        m_rcInset = rcInset;
        NeedUpdate();
    }

    int CContainerUI::GetChildPadding() const {
        if (m_pManager) {
            return m_pManager->ScaleValue(m_iChildPadding);
        }
        return m_iChildPadding;
    }

    void CContainerUI::SetChildPadding(int iPadding) {
        m_iChildPadding = iPadding;
        NeedUpdate();
    }

    UINT CContainerUI::GetChildAlign() const {
        return m_iChildAlign;
    }

    void CContainerUI::SetChildAlign(UINT iAlign) {
        m_iChildAlign = iAlign;
        NeedUpdate();
    }

    UINT CContainerUI::GetChildVAlign() const {
        return m_iChildVAlign;
    }

    void CContainerUI::SetChildVAlign(UINT iVAlign) {
        m_iChildVAlign = iVAlign;
        NeedUpdate();
    }

    bool CContainerUI::IsMouseChildEnabled() const {
        return m_bMouseChildEnabled;
    }

    void CContainerUI::SetMouseChildEnabled(bool bEnable) {
        m_bMouseChildEnabled = bEnable;
    }



    CContainerUI* CContainerUI::Clone()
    {
        CContainerUI* pClone = new CContainerUI();
        pClone->CopyData(this);
        return pClone;
    }

    void CContainerUI::CopyData(CContainerUI* pControl)
    {
        __super::CopyData(pControl);
        m_rcInset = pControl->m_rcInset;
        m_iChildPadding = pControl->m_iChildPadding;
        m_iChildAlign = pControl->m_iChildAlign;
        m_iChildVAlign = pControl->m_iChildVAlign;
        m_bAutoDestroy = pControl->m_bAutoDestroy;
        m_bDelayedDestroy = pControl->m_bDelayedDestroy;
        m_bMouseChildEnabled = pControl->m_bMouseChildEnabled;

        m_nScrollStepSize = pControl->m_nScrollStepSize;
        m_bFixedScrollbar = pControl->m_bFixedScrollbar;
        m_bShowScrollbar = pControl->m_bShowScrollbar;
        m_bSmoothScrollbar = pControl->m_bSmoothScrollbar;
        m_bScrollFloat = pControl->m_bScrollFloat;
        m_bCursorMouse = pControl->m_bCursorMouse;
        m_sVerticalScrollBarStyle = pControl->m_sVerticalScrollBarStyle;
        m_sHorizontalScrollBarStyle = pControl->m_sHorizontalScrollBarStyle;

        //SetManager(pControl->m_pManager, nullptr, false);
        for (int it = 0; it < pControl->m_items.GetSize(); it++)
        {
            CControlUI* pCloneControl = static_cast<CControlUI*>(pControl->m_items[it])->Clone();
            std::wstring str(pCloneControl->GetClass());
            Add(pCloneControl);
        }

        SetManager(pControl->m_pManager, nullptr, true);


        EnableScrollBar(pControl->m_pVerticalScrollBar != NULL, pControl->m_pHorizontalScrollBar != NULL);

        if (pControl->m_pVerticalScrollBar && pControl->m_pVerticalScrollBar->IsVisible())
        {
            m_pVerticalScrollBar->SetScrollRange(pControl->m_pVerticalScrollBar->GetScrollRange());
            m_pVerticalScrollBar->SetScrollPos(pControl->m_pVerticalScrollBar->GetScrollPos());
        }
        if (pControl->m_pHorizontalScrollBar && pControl->m_pHorizontalScrollBar->IsVisible())
        {
            m_pHorizontalScrollBar->SetScrollRange(pControl->m_pHorizontalScrollBar->GetScrollRange());
            m_pHorizontalScrollBar->SetScrollPos(pControl->m_pHorizontalScrollBar->GetScrollPos());
        }

    }

    void CContainerUI::SetHScrollSpace(int cx)
    {
        if (m_pHorizontalScrollBar != NULL) {
            m_pHorizontalScrollBar->SetVSpace(cx);
        }
    }

    void CContainerUI::SetVScrollSpace(int cy)
    {
        if (m_pVerticalScrollBar != NULL) {
            m_pVerticalScrollBar->SetVSpace(cy);
        }
    }

    void CContainerUI::SetCursorMouse(bool bCursorMouse)
    {
        m_bCursorMouse = bCursorMouse;
    }

    bool CContainerUI::IsFixedScrollbar() {
        return m_bFixedScrollbar;
    }

    void CContainerUI::SetFixedScrollbar(bool bFixed) {
        m_bFixedScrollbar = bFixed;
        Invalidate();
    }

    bool CContainerUI::IsShowScrollbar() {
        return m_bShowScrollbar;
    }

    void CContainerUI::SetShowScrollbar(bool bShow) {
        m_bShowScrollbar = bShow;

        if (m_pVerticalScrollBar != NULL) {
            m_pVerticalScrollBar->SetShow(bShow);
        }
        if (m_pHorizontalScrollBar != NULL) {
            m_pHorizontalScrollBar->SetShow(bShow);
        }
    }

    void CContainerUI::SetSmoothScrollbar(bool bSmooth)
    {
        m_bSmoothScrollbar = bSmooth;
    }

    bool CContainerUI::IsSmoothScrollbar()
    {
        return m_bSmoothScrollbar;
    }

    bool CContainerUI::IsScrollFloat() const
    {
        return m_bScrollFloat;
    }

    void CContainerUI::SetScrollFloat(bool bFloat)
    {
        if (m_bScrollFloat == bFloat) {
            return;
        }
        m_bScrollFloat = bFloat;
        NeedUpdate();
    }

    bool CContainerUI::IsScrollFloatShown() const
    {
        if (!m_bScrollFloat) {
            return true;
        }
        if (m_bScrollFloatHover) {
            return true;
        }
        if (m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() && m_pVerticalScrollBar->IsCaptured()) {
            return true;
        }
        if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() && m_pHorizontalScrollBar->IsCaptured()) {
            return true;
        }
        return false;
    }

    void CContainerUI::SetQuickScrolling(bool bQuickScrolling)
    {
        m_bQuickScrolling = bQuickScrolling;
    }

    bool CContainerUI::GetQuickScrolling()
    {
        return m_bQuickScrolling;
    }

    void CContainerUI::SetVisible(bool bVisible, bool bSendFocus) {
        if (m_animDir == AnimNone) {
            if (m_bVisible == bVisible) return;
            CControlUI::SetVisible(bVisible, bSendFocus);
            for (int it = 0; it < m_items.GetSize(); it++) {
                static_cast<CControlUI*>(m_items[it])->SetInternVisible(IsVisible());
            }
            return;
        }

        // 动画方向与请求一致 → 保持动画继续执行
        if (m_bAnimating && m_bAnimShowing == bVisible)
            return;

        // 动画进行中且方向反转 → 原地反转，保留当前进度/原始尺寸，不做 Stop
        // 旧实现调用 StopShowHideAnim 只停 timer 不恢复尺寸，随后若 m_bVisible==bVisible
        // 直接 return，导致 m_cxyFixed 被冻结在动画中途的小值，并在下一次 hide 中
        // 被错误地当成"原始尺寸"保存，造成宽度逐次衰减。
        if (m_bAnimating) {
            m_bAnimShowing = bVisible;
            if (bVisible && m_nAnimOrigFixedSize > 0 && m_pManager) {
                // Show 方向：把目标重新锚定到原始尺寸
                m_nAnimTargetPx = m_pManager->ScaleValue(m_nAnimOrigFixedSize);
            }
            // Hide 方向：m_nAnimTargetPx 继续当作"收缩基准长度"沿用，current 不变
            return;
        }

        if (m_bVisible == bVisible) {
            // 非动画态但可见状态已一致：若尺寸残留在上一次反转打断的小值，
            // 启动一段补齐动画把控件拉回原始尺寸。
            if (bVisible && m_nAnimOrigFixedSize > 0 && m_pManager) {
                const int curPx = (m_animDir == AnimHor) ? GetWidth() : GetHeight();
                const int targetPx = m_pManager->ScaleValue(m_nAnimOrigFixedSize);
                if (curPx > 0 && curPx < targetPx) {
                    m_bAnimShowing = true;
                    m_nAnimCurrentPx = curPx;
                    m_nAnimTargetPx = targetPx;
                    m_bAnimating = true;
                    if (!m_pManager->SetTimer(this, SHOWHIDE_ANIM_TIMERID, SHOWHIDE_ANIM_INTERVAL_MS)) {
                        m_bAnimating = false;
                        if (m_animDir == AnimHor) SetFixedWidth(m_nAnimOrigFixedSize, true);
                        else SetFixedHeight(m_nAnimOrigFixedSize, true);
                    }
                }
            }
            return;
        }

        StartShowHideAnim(bVisible);
    }

    // 闁槒绶稉濠忕礉鐎甸€涚艾Container閹貉傛娑撳秴鍙曞鈧銈嗘煙濞?
    // 鐠嬪啰鏁ゅ銈嗘煙濞夋洜娈戠紒鎾寸亯閺勵垽绱濋崘鍛村劥鐎涙劖甯舵禒鍫曟閽樺骏绱濋幒褌娆㈤張顒冮煩娓氭繄鍔ч弰鍓с仛閿涘矁鍎楅弲顖滅搼閺佸牊鐏夌€涙ê婀?
    void CContainerUI::SetInternVisible(bool bVisible) {
        CControlUI::SetInternVisible(bVisible);
        if (m_items.empty()) {
            return;
        }
        for (int it = 0; it < m_items.GetSize(); it++) {
            // 閹貉冨煑鐎涙劖甯舵禒鑸垫▔缁€铏瑰Ц閹?
            // InternVisible閻樿埖鈧礁绨查悽鍗炵摍閹貉傛閼奉亜绻侀幒褍鍩?
            static_cast<CControlUI*>(m_items[it])->SetInternVisible(IsVisible());
        }
    }

    void CContainerUI::SetEnabled(bool bEnabled) {
        if (m_bEnabled == bEnabled) {
            return;
        }

        m_bEnabled = bEnabled;

        for (int it = 0; it < m_items.GetSize(); it++) {
            static_cast<CControlUI*>(m_items[it])->SetEnabled(m_bEnabled);
        }

        Invalidate();
    }

    void CContainerUI::SetMouseEnabled(bool bEnabled) {
        if (m_pVerticalScrollBar != NULL) {
            m_pVerticalScrollBar->SetMouseEnabled(bEnabled);
        }
        if (m_pHorizontalScrollBar != NULL) {
            m_pHorizontalScrollBar->SetMouseEnabled(bEnabled);
        }
        CControlUI::SetMouseEnabled(bEnabled);
    }




    void CContainerUI::DoEvent(TEventUI& event)
    {
        if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN &&
            event.Type < UIEVENT__MOUSEEND) {
            if (m_pParent != NULL) {
                m_pParent->DoEvent(event);
            }
            else {
                CControlUI::DoEvent(event);
            }
            return;
        }


        if (event.Type == UIEVENT_TIMER && event.wParam == SHOWHIDE_ANIM_TIMERID) {
            AdvanceShowHideAnim();
            return;
        }

        if (event.Type == UIEVENT_SETFOCUS) {
            m_bFocused = true;
            return;
        }
        if (event.Type == UIEVENT_KILLFOCUS) {
            m_bFocused = false;
            return;
        }

        if (event.Type == UIEVENT_TIMER && event.wParam == SMOOTH_SCROLL_TIMERID)
        {
            if (AdvanceSmoothScroll()) {
                return;
            }
        }

        // 姒х姵鐖ｅ姘崇枂閸旂娀鈧喎瀹虫径鍕倞閿涘牆鐎惄杈剧礆
        if (event.Type == UIEVENT_SCROLLWHEEL) {
            if ((event.wKeyState & MK_CONTROL) == 0)
            {
                const int code = LOWORD(event.wParam);
                const int wheelDelta = static_cast<short>(HIWORD(event.wParam));
                const bool bShiftHorizontal = (event.wKeyState & MK_SHIFT) != 0;
                const bool bHasVertical = m_pVerticalScrollBar && m_pVerticalScrollBar->IsEnabled() && m_pVerticalScrollBar->IsVisible();
                const bool bHasHorizontal = m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsEnabled() && m_pHorizontalScrollBar->IsVisible();

                if ((code == SB_LINELEFT || code == SB_LINERIGHT) && bHasHorizontal)
                {
                    const int cxStep = GetWheelScrollStep(true);
                    const int cxDelta = (wheelDelta >= 0 ? 1 : -1) * (std::max)(1, static_cast<int>(std::lround(std::fabs(static_cast<double>(wheelDelta)) * cxStep / WHEEL_DELTA)));
                    ScrollByDelta(cxDelta, 0, true, true, true);
                    return;
                }

                if (bHasVertical && (code == SB_LINEUP || code == SB_LINEDOWN) && !bShiftHorizontal)
                {
                    const int cyStep = GetWheelScrollStep(false);
                    const int cyDelta = (wheelDelta <= 0 ? 1 : -1) * (std::max)(1, static_cast<int>(std::lround(std::fabs(static_cast<double>(wheelDelta)) * cyStep / WHEEL_DELTA)));
                    ScrollByDelta(0, cyDelta, true, true, true);
                    return;
                }

                if (bHasHorizontal && (code == SB_LINEUP || code == SB_LINEDOWN) && bShiftHorizontal)
                {
                    const int cxStep = GetWheelScrollStep(true);
                    const int cxDelta = (wheelDelta <= 0 ? 1 : -1) * (std::max)(1, static_cast<int>(std::lround(std::fabs(static_cast<double>(wheelDelta)) * cxStep / WHEEL_DELTA)));
                    ScrollByDelta(cxDelta, 0, true, true, true);
                    return;
                }
            }
        }

        if (event.Type == UIEVENT_SETCURSOR)
        {
            if (m_bCursorMouse == false)
                return;
        }

        // 闁款喚娲忔稉搴″従鐎瑰啴鈧槒绶穱婵囧瘮
        if (m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() &&
            m_pVerticalScrollBar->IsEnabled()) {
            if (event.Type == UIEVENT_KEYDOWN) {
                switch (event.chKey) {
                case VK_DOWN: LineDown(); return;
                case VK_UP:   LineUp(); return;
                case VK_NEXT: PageDown(); return;
                case VK_PRIOR: PageUp(); return;
                case VK_HOME: HomeUp(); return;
                case VK_END:  EndDown(); return;
                }
            }
        }
        if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() &&
            m_pHorizontalScrollBar->IsEnabled()) {
            if (event.Type == UIEVENT_KEYDOWN) {
                switch (event.chKey) {
                case VK_DOWN: LineRight(); return;
                case VK_UP:   LineLeft(); return;
                case VK_NEXT: PageRight(); return;
                case VK_PRIOR: PageLeft(); return;
                case VK_HOME: HomeLeft(); return;
                case VK_END:  EndRight(); return;
                }
            }
        }
        CControlUI::DoEvent(event);
    }

    CControlUI* CContainerUI::FindItmeControl(std::wstring_view pstrChildControlName)
    {
        for (int index = 0; index < m_items.GetSize(); index++)
        {
            CControlUI* pControl = static_cast<CControlUI*>(m_items[index]);
            if (pControl->GetNameView() == pstrChildControlName)
            {
                return pControl;
            }
        }
        return nullptr;
    }

    SIZE CContainerUI::GetScrollPos() const {
        SIZE sz = { 0, 0 };
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            sz.cy = m_pVerticalScrollBar->GetScrollPos();
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            sz.cx = m_pHorizontalScrollBar->GetScrollPos();
        }
        return sz;
    }

    SIZE CContainerUI::GetScrollRange() const {
        SIZE sz = { 0, 0 };
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            sz.cy = m_pVerticalScrollBar->GetScrollRange();
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            sz.cx = m_pHorizontalScrollBar->GetScrollRange();
        }
        return sz;
    }

    RECT CContainerUI::GetScrollViewRect() const
    {
        RECT rcView = m_rcItem;
        RECT rcInset = GetInset();
        rcView.left += rcInset.left;
        rcView.top += rcInset.top;
        rcView.right -= rcInset.right;
        rcView.bottom -= rcInset.bottom;
        if (!m_bScrollFloat && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            rcView.right -= m_pVerticalScrollBar->GetFixedWidth();
        }
        if (!m_bScrollFloat && m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            rcView.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
        }
        if (rcView.right < rcView.left) rcView.right = rcView.left;
        if (rcView.bottom < rcView.top) rcView.bottom = rcView.top;
        return rcView;
    }

    SIZE CContainerUI::ClampScrollPos(SIZE szPos) const
    {
        SIZE clamped = szPos;
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            const LONG nRange = static_cast<LONG>(m_pVerticalScrollBar->GetScrollRange());
            clamped.cy = (std::max)(0L, (std::min)(clamped.cy, nRange));
        }
        else {
            clamped.cy = 0;
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            const LONG nRange = static_cast<LONG>(m_pHorizontalScrollBar->GetScrollRange());
            clamped.cx = (std::max)(0L, (std::min)(clamped.cx, nRange));
        }
        else {
            clamped.cx = 0;
        }
        return clamped;
    }

    int CContainerUI::GetWheelScrollStep(bool bHorizontal) const
    {
        const int nExplicitStep = GetScrollStepSize();
        if (nExplicitStep > 0) {
            return nExplicitStep;
        }

        UINT wheelUnits = 0;
        if (!::SystemParametersInfo(bHorizontal ? SPI_GETWHEELSCROLLCHARS : SPI_GETWHEELSCROLLLINES, 0, &wheelUnits, 0)) {
            wheelUnits = 3;
        }
        if (wheelUnits == WHEEL_PAGESCROLL) {
            RECT rcView = GetScrollViewRect();
            return bHorizontal ? (rcView.right - rcView.left) : (rcView.bottom - rcView.top);
        }

        int nStep = 8;
        if (m_pManager) {
            nStep = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
        }
        return (std::max)(1, nStep * static_cast<int>((std::max)(1u, wheelUnits)));
    }

    bool CContainerUI::CanUseScrollRenderCache(const RECT& rcScrollView) const
    {
        if (!m_bQuickScrolling || m_pManager == NULL || m_pManager->IsUpdateNeeded()) {
            return false;
        }

        if (m_pManager->GetActiveRenderBackend() == RenderBackendDirect2D) {
            return false;
        }

        RECT rcTest = rcScrollView;
        ::InflateRect(&rcTest, kScrollCacheSafetyInset, kScrollCacheSafetyInset);
        for (int i = 0; i < m_items.GetSize(); ++i) {
            CControlUI* pControl = static_cast<CControlUI*>(m_items[i]);
            if (pControl == NULL || !pControl->IsVisible() || !pControl->IsFloat()) {
                continue;
            }

            RECT rcIntersect = { 0 };
            if (::IntersectRect(&rcIntersect, &rcTest, &pControl->GetPos())) {
                return false;
            }
        }

        return true;
    }

    void CContainerUI::ApplyScrollPos(SIZE szPos, bool bMsg, bool bScroolVisible)
    {
        int cx = 0;
        int cy = 0;
        const SIZE szOriPos = GetScrollPos();
        if (szOriPos.cy == szPos.cy && szOriPos.cx == szPos.cx) {
            return;
        }

        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
        {
            const int iLastScrollPos = m_pVerticalScrollBar->GetScrollPos();
            m_pVerticalScrollBar->SetScrollPos(szPos.cy);
            cy = m_pVerticalScrollBar->GetScrollPos() - iLastScrollPos;
        }

        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            const int iLastScrollPos = m_pHorizontalScrollBar->GetScrollPos();
            m_pHorizontalScrollBar->SetScrollPos(szPos.cx);
            cx = m_pHorizontalScrollBar->GetScrollPos() - iLastScrollPos;
        }

        if (!bScroolVisible && m_pManager != NULL)
        {
            m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL_TOOLS, (WPARAM)szPos.cy, (LPARAM)cy);
        }

        if (cx == 0 && cy == 0) {
            if (bScroolVisible && m_pManager != NULL) {
                m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL_TOOLS, (WPARAM)szPos.cy, (LPARAM)cy);
            }
            return;
        }

        const RECT rcView = GetScrollViewRect();
        const bool bUsedScrollCache = CanUseScrollRenderCache(rcView) &&
            m_pManager->ScrollRenderCacheRect(rcView, -cx, -cy);

        RECT rcQuickView = rcView;
        const int quickMargin = (std::max)(GetScrollStepSize() * 2, 96);
        ::InflateRect(&rcQuickView, quickMargin, quickMargin);

        for (int it2 = 0; it2 < m_items.GetSize(); it2++)
        {
            CControlUI* pControl = static_cast<CControlUI*>(m_items[it2]);
            if (pControl == NULL || !pControl->IsVisible() || pControl->IsFloat()) {
                continue;
            }

            RECT rcPos = pControl->GetPos();
            ::OffsetRect(&rcPos, -cx, -cy);
            if (m_bQuickScrolling && !pControl->IsContainerControl()) {
                RECT rcIntersect = { 0 };
                if (!::IntersectRect(&rcIntersect, &rcQuickView, &rcPos)) {
                    pControl->SetOnlyPos(rcPos);
                    continue;
                }
            }

            pControl->SetPos(rcPos, false);
        }

        if (m_pManager != NULL) {
            if (bUsedScrollCache) {
                if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
                    RECT rcScrollBar = m_pVerticalScrollBar->GetPos();
                    m_pManager->Invalidate(rcScrollBar);
                }
                if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
                    RECT rcScrollBar = m_pHorizontalScrollBar->GetPos();
                    m_pManager->Invalidate(rcScrollBar);
                }
            }
            else {
                Invalidate();
            }
        }

        if (m_pVerticalScrollBar && m_pManager != NULL && bMsg) {
            const int nPage = (m_pVerticalScrollBar->GetScrollPos() + m_pVerticalScrollBar->GetLineSize());
            m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL, (WPARAM)nPage, (LPARAM)cy);
            m_pManager->SendNotify(this, DUI_MSGTYPE_SCROLL_TOOLS, (WPARAM)szPos.cy, (LPARAM)cy);
        }
    }

    void CContainerUI::QueueSmoothScroll(SIZE szTarget, bool bMsg, bool bScroolVisible)
    {
        if (m_pManager == NULL || !m_bSmoothScrollbar) {
            ApplyScrollPos(ClampScrollPos(szTarget), bMsg, bScroolVisible);
            return;
        }

        const SIZE clampedTarget = ClampScrollPos(szTarget);
        const SIZE current = GetScrollPos();
        if (current.cx == clampedTarget.cx && current.cy == clampedTarget.cy) {
            StopSmoothScroll(false);
            return;
        }

        m_szSmoothScrollTarget = clampedTarget;
        m_bSmoothScrollNotify = bMsg;
        m_bSmoothScrollVisible = bScroolVisible;
        m_fSmoothScrollPosX = static_cast<double>(current.cx);
        m_fSmoothScrollPosY = static_cast<double>(current.cy);
        m_smoothScrollLastTick = std::chrono::steady_clock::now();

        if (!m_bSmoothScrollAnimating) {
            if (!m_bSmoothScrollPeriodRaised) {
                timeBeginPeriod(1);
                m_bSmoothScrollPeriodRaised = true;
            }
            m_bSmoothScrollAnimating = m_pManager->SetTimer(this, SMOOTH_SCROLL_TIMERID, SMOOTH_SCROLL_INTERVAL_MS);
        }

        if (!m_bSmoothScrollAnimating) {
            if (m_bSmoothScrollPeriodRaised) {
                timeEndPeriod(1);
                m_bSmoothScrollPeriodRaised = false;
            }
            ApplyScrollPos(clampedTarget, bMsg, bScroolVisible);
            return;
        }

        AdvanceSmoothScroll();
    }

    void CContainerUI::ScrollByDelta(int cxDelta, int cyDelta, bool bMsg, bool bScroolVisible, bool bPreferSmooth)
    {
        if (cxDelta == 0 && cyDelta == 0) {
            return;
        }

        SIZE szTarget = m_bSmoothScrollAnimating ? m_szSmoothScrollTarget : GetScrollPos();
        szTarget.cx += cxDelta;
        szTarget.cy += cyDelta;
        szTarget = ClampScrollPos(szTarget);
        if (bPreferSmooth && m_bSmoothScrollbar) {
            QueueSmoothScroll(szTarget, bMsg, bScroolVisible);
        }
        else {
            StopSmoothScroll(false);
            ApplyScrollPos(szTarget, bMsg, bScroolVisible);
            m_szSmoothScrollTarget = ClampScrollPos(GetScrollPos());
            m_fSmoothScrollPosX = static_cast<double>(m_szSmoothScrollTarget.cx);
            m_fSmoothScrollPosY = static_cast<double>(m_szSmoothScrollTarget.cy);
        }
    }

    bool CContainerUI::AdvanceSmoothScroll()
    {
        if (!m_bSmoothScrollAnimating) {
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - m_smoothScrollLastTick).count();
        m_smoothScrollLastTick = now;
        if (dt <= 0.0) {
            dt = 1.0 / 240.0;
        }
        else if (dt > 0.05) {
            dt = 0.05;
        }

        const SIZE currentPos = GetScrollPos();
        const SIZE clampedTarget = ClampScrollPos(m_szSmoothScrollTarget);
        m_szSmoothScrollTarget = clampedTarget;

        const auto advanceAxis = [dt](double current, int currentInt, int target) {
            const double distance = static_cast<double>(target) - current;
            if (std::fabs(distance) < kSmoothScrollSnapDistance) {
                return static_cast<double>(target);
            }

            const double smoothFactor = 1.0 - std::exp(-kSmoothScrollResponse * dt);
            double step = distance * smoothFactor;
            const double maxStep = (std::max)(1.0, kSmoothScrollMaxSpeed * dt);
            if (std::fabs(step) > maxStep) {
                step = std::copysign(maxStep, step);
            }
            current += step;

            const int rounded = static_cast<int>(std::lround(current));
            if (rounded == currentInt) {
                current = static_cast<double>(ClampScrollDeltaToPixelStep(distance, currentInt));
            }
            return current;
            };

        m_fSmoothScrollPosX = advanceAxis(m_fSmoothScrollPosX, currentPos.cx, clampedTarget.cx);
        m_fSmoothScrollPosY = advanceAxis(m_fSmoothScrollPosY, currentPos.cy, clampedTarget.cy);

        SIZE szNext = {
            static_cast<int>(std::lround(m_fSmoothScrollPosX)),
            static_cast<int>(std::lround(m_fSmoothScrollPosY))
        };
        szNext = ClampScrollPos(szNext);

        m_bApplyingSmoothScroll = true;
        ApplyScrollPos(szNext, m_bSmoothScrollNotify, m_bSmoothScrollVisible);
        m_bApplyingSmoothScroll = false;

        const SIZE applied = GetScrollPos();
        m_fSmoothScrollPosX = static_cast<double>(applied.cx);
        m_fSmoothScrollPosY = static_cast<double>(applied.cy);

        if (applied.cx == clampedTarget.cx && applied.cy == clampedTarget.cy) {
            StopSmoothScroll(false);
            return true;
        }

        return true;
    }

    void CContainerUI::StopSmoothScroll(bool bSnapToTarget)
    {
        const bool bWasAnimating = m_bSmoothScrollAnimating;
        m_bSmoothScrollAnimating = false;

        if (m_pManager != NULL) {
            m_pManager->KillTimer(this, SMOOTH_SCROLL_TIMERID);
        }

        if (bSnapToTarget) {
            m_bApplyingSmoothScroll = true;
            ApplyScrollPos(ClampScrollPos(m_szSmoothScrollTarget), m_bSmoothScrollNotify, m_bSmoothScrollVisible);
            m_bApplyingSmoothScroll = false;
        }

        if (m_bSmoothScrollPeriodRaised) {
            timeEndPeriod(1);
            m_bSmoothScrollPeriodRaised = false;
        }

        if (bWasAnimating || bSnapToTarget) {
            const SIZE applied = GetScrollPos();
            m_szSmoothScrollTarget = ClampScrollPos(applied);
            m_fSmoothScrollPosX = static_cast<double>(m_szSmoothScrollTarget.cx);
            m_fSmoothScrollPosY = static_cast<double>(m_szSmoothScrollTarget.cy);
        }
    }

    void CContainerUI::SetScrollPos(SIZE szPos, bool bMsg, bool bScroolVisible) {
        if (!m_bApplyingSmoothScroll) {
            StopSmoothScroll(false);
        }

        ApplyScrollPos(ClampScrollPos(szPos), bMsg, bScroolVisible);

        if (!m_bApplyingSmoothScroll) {
            const SIZE applied = GetScrollPos();
            m_szSmoothScrollTarget = ClampScrollPos(applied);
            m_fSmoothScrollPosX = static_cast<double>(m_szSmoothScrollTarget.cx);
            m_fSmoothScrollPosY = static_cast<double>(m_szSmoothScrollTarget.cy);
        }
    }

    void CContainerUI::SetScrollStepSize(int nSize) {
        if (nSize > 0) {
            m_nScrollStepSize = nSize;
        }
    }

    int CContainerUI::GetScrollStepSize() const {
        if (m_pManager) {
            return m_pManager->ScaleValue(m_nScrollStepSize);
        }

        return m_nScrollStepSize;
    }

    void CContainerUI::LineUp(bool bScroolVisible) {
        int cyLine = GetScrollStepSize();
        if (cyLine == 0) {
            cyLine = 8;
            if (m_pManager) {
                cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
            }
        }
        ScrollByDelta(0, -cyLine, true, bScroolVisible, true);
    }

    void CContainerUI::LineDown(bool bScroolVisible) {
        int cyLine = GetScrollStepSize();
        if (cyLine == 0) {
            cyLine = 8;
            if (m_pManager) {
                cyLine = m_pManager->GetDefaultFontInfo()->tm.tmHeight + 8;
            }
        }

        ScrollByDelta(0, cyLine, true, bScroolVisible, true);
    }

    void CContainerUI::PageUp() {
        int iOffset =
            m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
        if (!m_bScrollFloat && m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
        }
        ScrollByDelta(0, -iOffset, true, true, true);
    }

    void CContainerUI::PageDown() {
        int iOffset =
            m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
        if (!m_bScrollFloat && m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
        }
        ScrollByDelta(0, iOffset, true, true, true);
    }

    void CContainerUI::HomeUp() {
        SIZE sz = GetScrollPos();
        sz.cy = 0;
        QueueSmoothScroll(sz);
    }

    void CContainerUI::EndDown() {
        if (m_pManager) {
            ::UpdateWindow(m_pManager->GetPaintWindow());
        }
        SIZE sz = GetScrollPos();
        sz.cy = GetScrollRange().cy;
        QueueSmoothScroll(sz);
    }

    void CContainerUI::LineLeft() {
        int nScrollStepSize = GetScrollStepSize();
        int cxLine = nScrollStepSize == 0 ? 8 : nScrollStepSize;

        ScrollByDelta(-cxLine, 0, true, true, true);
    }

    void CContainerUI::LineRight() {
        int nScrollStepSize = GetScrollStepSize();
        int cxLine = nScrollStepSize == 0 ? 8 : nScrollStepSize;

        ScrollByDelta(cxLine, 0, true, true, true);
    }

    void CContainerUI::PageLeft() {
        RECT rcInset = GetInset();
        int iOffset = m_rcItem.right - m_rcItem.left - rcInset.left - rcInset.right;
        if (!m_bScrollFloat && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            iOffset -= m_pVerticalScrollBar->GetFixedWidth();
        }
        ScrollByDelta(-iOffset, 0, true, true, true);
    }

    void CContainerUI::PageRight() {
        RECT rcInset = GetInset();
        int iOffset = m_rcItem.right - m_rcItem.left - rcInset.left - rcInset.right;
        if (!m_bScrollFloat && m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            iOffset -= m_pVerticalScrollBar->GetFixedWidth();
        }
        ScrollByDelta(iOffset, 0, true, true, true);
    }

    void CContainerUI::HomeLeft() {
        SIZE sz = GetScrollPos();
        sz.cx = 0;
        QueueSmoothScroll(sz);
    }

    void CContainerUI::EndRight() {
        if (m_pManager) {
            ::UpdateWindow(m_pManager->GetPaintWindow());
        }
        SIZE sz = GetScrollPos();
        sz.cx = GetScrollRange().cx;
        QueueSmoothScroll(sz);
    }

    void CContainerUI::EnableScrollBar(bool bEnableVertical,
        bool bEnableHorizontal) {
        if (bEnableVertical && !m_pVerticalScrollBar) {
            m_pVerticalScrollBar = new CScrollBarUI;
            m_pVerticalScrollBar->SetOwner(this);
            m_pVerticalScrollBar->SetManager(m_pManager, NULL, false);
            if (m_pManager) {
                const std::wstring_view pDefaultAttributes =
                    m_pManager->GetDefaultAttributeList(L"VScrollBar");
                if (!pDefaultAttributes.empty()) {
                    m_pVerticalScrollBar->ApplyAttributeList(pDefaultAttributes);
                }
                else {
                    m_pVerticalScrollBar->ApplyDefaultStyle();
                }

                m_pVerticalScrollBar->SetShow(m_bShowScrollbar);
            }
        }
        else if (!bEnableVertical && m_pVerticalScrollBar) {
            delete m_pVerticalScrollBar;
            m_pVerticalScrollBar = NULL;
        }

        if (bEnableHorizontal && !m_pHorizontalScrollBar) {
            m_pHorizontalScrollBar = new CScrollBarUI;
            m_pHorizontalScrollBar->SetHorizontal(true);
            m_pHorizontalScrollBar->SetOwner(this);
            m_pHorizontalScrollBar->SetManager(m_pManager, NULL, false);

            if (m_pManager) {
                const std::wstring_view pDefaultAttributes =
                    m_pManager->GetDefaultAttributeList(L"HScrollBar");
                if (!pDefaultAttributes.empty()) {
                    m_pHorizontalScrollBar->ApplyAttributeList(pDefaultAttributes);
                }
                else {
                    m_pHorizontalScrollBar->ApplyDefaultStyle();
                }

                m_pHorizontalScrollBar->SetShow(m_bShowScrollbar);
            }
        }
        else if (!bEnableHorizontal && m_pHorizontalScrollBar) {
            delete m_pHorizontalScrollBar;
            m_pHorizontalScrollBar = NULL;
        }

        NeedUpdate();
    }

    CScrollBarUI* CContainerUI::GetVerticalScrollBar() const {
        return m_pVerticalScrollBar;
    }

    CScrollBarUI* CContainerUI::GetHorizontalScrollBar() const {
        return m_pHorizontalScrollBar;
    }

    DWORD CContainerUI::GetNowTime()
    {
        SYSTEMTIME st;
        memset(&st, 0, sizeof(SYSTEMTIME));
        GetLocalTime(&st);
        DWORD dTime = st.wHour * 3600 * 1000 + st.wMinute * 60 * 1000 + st.wSecond * 1000 + st.wMilliseconds;
        return dTime;
    }

    int CContainerUI::FindSelectable(int iIndex, bool bForward /*= true*/) const {
        // NOTE: This is actually a helper-function for the list/combo/ect controls
        //       that allow them to find the next enabled/available selectable item
        if (GetCount() == 0) {
            return -1;
        }
        iIndex = CLAMP(iIndex, 0, GetCount() - 1);
        if (bForward) {
            for (int i = iIndex; i < GetCount(); i++) {
                if (GetItemAt(i)->GetInterface(_T("ListItem")) != NULL &&
                    GetItemAt(i)->IsVisible() && GetItemAt(i)->IsEnabled()) {
                    return i;
                }
            }
            return -1;
        }
        else {
            for (int i = iIndex; i >= 0; --i) {
                if (GetItemAt(i)->GetInterface(_T("ListItem")) != NULL &&
                    GetItemAt(i)->IsVisible() && GetItemAt(i)->IsEnabled()) {
                    return i;
                }
            }
            return FindSelectable(0, true);
        }
    }

    RECT CContainerUI::GetClientPos() const {
        RECT rc = m_rcItem;

        RECT rcInset = GetInset();
        rc.left += rcInset.left;
        rc.top += rcInset.top;
        rc.right -= rcInset.right;
        rc.bottom -= rcInset.bottom;

        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            rc.top -= m_pVerticalScrollBar->GetScrollPos();
            rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
            rc.bottom += m_pVerticalScrollBar->GetScrollRange();
            if (!m_bScrollFloat) {
                rc.right -= m_pVerticalScrollBar->GetFixedWidth();
            }
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            rc.left -= m_pHorizontalScrollBar->GetScrollPos();
            rc.right -= m_pHorizontalScrollBar->GetScrollPos();
            rc.right += m_pHorizontalScrollBar->GetScrollRange();
            if (!m_bScrollFloat) {
                rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
            }
        }
        return rc;
    }

    void CContainerUI::Move(SIZE szOffset, bool bNeedInvalidate) {
        CControlUI::Move(szOffset, bNeedInvalidate);
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            m_pVerticalScrollBar->Move(szOffset, false);
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            m_pHorizontalScrollBar->Move(szOffset, false);
        }
        for (int it = 0; it < m_items.GetSize(); it++) {
            CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
            if (pControl != NULL && pControl->IsVisible()) {
                pControl->Move(szOffset, false);
            }
        }
    }

    void CContainerUI::SetPos(RECT rc, bool bNeedInvalidate) {
        CControlUI::SetPos(rc, bNeedInvalidate);
        if (m_items.empty()) {
            return;
        }

        rc = m_rcItem;
        RECT rcInset = GetInset();

        rc.left += rcInset.left;
        rc.top += rcInset.top;
        rc.right -= rcInset.right;
        rc.bottom -= rcInset.bottom;

        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            rc.top -= m_pVerticalScrollBar->GetScrollPos();
            rc.bottom -= m_pVerticalScrollBar->GetScrollPos();
            rc.bottom += m_pVerticalScrollBar->GetScrollRange();
            if (!m_bScrollFloat) {
                rc.right -= m_pVerticalScrollBar->GetFixedWidth();
            }
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            rc.left -= m_pHorizontalScrollBar->GetScrollPos();
            rc.right -= m_pHorizontalScrollBar->GetScrollPos();
            rc.right += m_pHorizontalScrollBar->GetScrollRange();
            if (!m_bScrollFloat) {
                rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
            }
        }

        for (int it = 0; it < m_items.GetSize(); it++) {
            CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
            if (!pControl->IsVisible()) {
                continue;
            }
            if (pControl->IsFloat()) {
                SetFloatPos(it);
            }
            else {
                SIZE sz = { rc.right - rc.left, rc.bottom - rc.top };
                if (sz.cx < pControl->GetMinWidth()) {
                    sz.cx = pControl->GetMinWidth();
                }
                if (sz.cx > pControl->GetMaxWidth()) {
                    sz.cx = pControl->GetMaxWidth();
                }
                if (sz.cy < pControl->GetMinHeight()) {
                    sz.cy = pControl->GetMinHeight();
                }
                if (sz.cy > pControl->GetMaxHeight()) {
                    sz.cy = pControl->GetMaxHeight();
                }
                RECT rcCtrl = { rc.left, rc.top, rc.left + sz.cx, rc.top + sz.cy };
                pControl->SetPos(rcCtrl, false);
            }
        }
    }

    void CContainerUI::SetAttribute(std::wstring_view pstrNameView, std::wstring_view pstrValueView) {
        const std::wstring_view name = StringUtil::TrimView(pstrNameView);
        if (name.empty()) {
            return;
        }

        if (StringUtil::EqualsNoCase(name, L"inset")) {
            RECT inset = { 0 };
            if (StringUtil::TryParseRect(pstrValueView, inset)) {
                SetInset(inset);
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"mousechild")) {
            SetMouseChildEnabled(StringUtil::ParseBool(pstrValueView));
        }
        else if (StringUtil::EqualsNoCase(name, L"animation_show_hide")) {
            if (StringUtil::EqualsNoCase(pstrValueView, L"hor")) SetShowHideAnimDir(AnimHor);
            else if (StringUtil::EqualsNoCase(pstrValueView, L"ver")) SetShowHideAnimDir(AnimVer);
            else SetShowHideAnimDir(AnimNone);
        }
        else if (StringUtil::EqualsNoCase(name, L"vscrollbar"))
        {
            EnableScrollBar(StringUtil::ParseBool(pstrValueView), GetHorizontalScrollBar() != NULL);
        }
        else if (StringUtil::EqualsNoCase(name, L"vspace"))
        {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) {
                SetVScrollSpace(value);
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"hspace"))
        {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) {
                SetHScrollSpace(value);
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"cursormouse"))
        {
            SetCursorMouse(StringUtil::ParseBool(pstrValueView));
        }

        else if (StringUtil::EqualsNoCase(name, L"vscrollbarstyle")) {
            m_sVerticalScrollBarStyle = pstrValueView;
            EnableScrollBar(TRUE, GetHorizontalScrollBar() != NULL);
            if (GetVerticalScrollBar()) {
                const std::wstring_view pStyle = m_pManager->GetStyle(m_sVerticalScrollBarStyle);
                if (!pStyle.empty()) {
                    GetVerticalScrollBar()->ApplyAttributeList(pStyle);
                }
                else {
                    GetVerticalScrollBar()->ApplyAttributeList(pstrValueView);
                }
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"hscrollbar")) {
            EnableScrollBar(GetVerticalScrollBar() != NULL,
                StringUtil::ParseBool(pstrValueView));
        }
        else if (StringUtil::EqualsNoCase(name, L"hscrollbarstyle")) {
            m_sHorizontalScrollBarStyle = pstrValueView;
            EnableScrollBar(TRUE, GetHorizontalScrollBar() != NULL);
            if (GetHorizontalScrollBar()) {
                const std::wstring_view pStyle = m_pManager->GetStyle(m_sHorizontalScrollBarStyle);
                if (!pStyle.empty()) {
                    GetHorizontalScrollBar()->ApplyAttributeList(pStyle);
                }
                else {
                    GetHorizontalScrollBar()->ApplyAttributeList(pstrValueView);
                }
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"childpadding")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) {
                SetChildPadding(value);
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"childalign")) {
            if (StringUtil::EqualsNoCase(pstrValueView, L"left")) {
                m_iChildAlign = DT_LEFT;
            }
            else if (StringUtil::EqualsNoCase(pstrValueView, L"center")) {
                m_iChildAlign = DT_CENTER;
            }
            else if (StringUtil::EqualsNoCase(pstrValueView, L"right")) {
                m_iChildAlign = DT_RIGHT;
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"childvalign")) {
            if (StringUtil::EqualsNoCase(pstrValueView, L"top")) {
                m_iChildVAlign = DT_TOP;
            }
            else if (StringUtil::EqualsNoCase(pstrValueView, L"vcenter")) {
                m_iChildVAlign = DT_VCENTER;
            }
            else if (StringUtil::EqualsNoCase(pstrValueView, L"bottom")) {
                m_iChildVAlign = DT_BOTTOM;
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"scrollstepsize")) {
            int value = 0;
            if (StringUtil::TryParseInt(pstrValueView, value)) {
                SetScrollStepSize(value);
            }
        }
        else if (StringUtil::EqualsNoCase(name, L"fixedscrollbar")) {
            SetFixedScrollbar(StringUtil::ParseBool(pstrValueView));
        }
        else if (StringUtil::EqualsNoCase(name, L"showscrollbar")) {
            SetShowScrollbar(StringUtil::ParseBool(pstrValueView));
        }
        else if (StringUtil::EqualsNoCase(name, L"smooth"))
        {
            SetSmoothScrollbar(StringUtil::ParseBool(pstrValueView));
        }
        else if (StringUtil::EqualsNoCase(name, L"scrollfloat"))
        {
            SetScrollFloat(StringUtil::ParseBool(pstrValueView));
        }
        else if (StringUtil::EqualsNoCase(name, L"quick_scroll"))
        {
            SetQuickScrolling(StringUtil::ParseBool(pstrValueView));
        }
        else {
            CControlUI::SetAttribute(pstrNameView, pstrValueView);
        }
    }

    void CContainerUI::SetManager(CPaintManagerUI* pManager,
        CControlUI* pParent,
        bool bInit) {
        if (m_pManager != pManager) {
            StopSmoothScroll(false);
        }
        for (int it = 0; it < m_items.GetSize(); it++) {
            static_cast<CControlUI*>(m_items[it])->SetManager(pManager, this, bInit);
        }

        if (m_pVerticalScrollBar != NULL) {
            m_pVerticalScrollBar->SetManager(pManager, this, bInit);
        }
        if (m_pHorizontalScrollBar != NULL) {
            m_pHorizontalScrollBar->SetManager(pManager, this, bInit);
        }
        CControlUI::SetManager(pManager, pParent, bInit);
        const SIZE szPos = GetScrollPos();
        m_szSmoothScrollTarget = ClampScrollPos(szPos);
        m_fSmoothScrollPosX = static_cast<double>(m_szSmoothScrollTarget.cx);
        m_fSmoothScrollPosY = static_cast<double>(m_szSmoothScrollTarget.cy);
    }


    CControlUI* CContainerUI::FindSubControl(CControlUI* pRoot, std::wstring_view strSubControlName)
    {
        if (!pRoot) return nullptr;
        if (strSubControlName.empty()) return nullptr;

        std::vector<CControlUI*> stack;
        stack.reserve(1024);
        stack.push_back(pRoot);

        while (!stack.empty())
        {
            CControlUI* pItem = stack.back();
            stack.pop_back();
            const std::wstring_view strName = pItem->GetNameView();
            if (!strName.empty() && strName == strSubControlName)
                return pItem;

            if (pItem->IsContainerControl())
            {
                CContainerUI* pContainer = static_cast<CContainerUI*>(pItem);
                const int count = pContainer->m_items.GetSize();
                void** ppData = pContainer->m_items.c_str();
                for (int i = count - 1; i >= 0; --i)
                {
                    if (CControlUI* pChild = static_cast<CControlUI*>(ppData[i]))
                        stack.push_back(pChild);
                }
            }
        }

        return nullptr;
    }


    CControlUI* CContainerUI::FindSubControl(CControlUI* pRoot, std::wstring_view strSubControlName, std::wstring_view strEliminateControlName)
    {
        if (!pRoot) return nullptr;
        if (strSubControlName.empty()) return nullptr;

        std::vector<CControlUI*> stack;
        stack.reserve(1024);
        stack.push_back(pRoot);

        while (!stack.empty())
        {
            CControlUI* pItem = stack.back();
            stack.pop_back();
            const std::wstring_view strName = pItem->GetNameView();
            if (!strName.empty() && strName == strSubControlName)
                return pItem;
            if (!strEliminateControlName.empty() && strName == strEliminateControlName)
                continue;

            if (pItem->IsContainerControl())
            {
                CContainerUI* pContainer = static_cast<CContainerUI*>(pItem);
                const int count = pContainer->m_items.GetSize();
                void** ppData = pContainer->m_items.c_str();
                for (int i = count - 1; i >= 0; --i)
                {
                    if (CControlUI* pChild = static_cast<CControlUI*>(ppData[i]))
                        stack.push_back(pChild);
                }
            }
        }

        return nullptr;
    }

    CControlUI* CContainerUI::FindControl(FINDCONTROLPROC Proc,
        LPVOID pData,
        UINT uFlags) {
        if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
            return NULL;
        }
        if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
            return NULL;
        }
        if ((uFlags & UIFIND_HITTEST) != 0) {
            const bool bInside = ::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData)));
            if (m_bScrollFloat && m_bScrollFloatHover != bInside) {
                m_bScrollFloatHover = bInside;
                if (m_pManager != NULL) {
                    if (m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()) {
                        RECT rcSB = m_pVerticalScrollBar->GetPos();
                        m_pManager->Invalidate(rcSB);
                    }
                    if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible()) {
                        RECT rcSB = m_pHorizontalScrollBar->GetPos();
                        m_pManager->Invalidate(rcSB);
                    }
                }
            }
            if (!bInside) {
                // Recurse once into children so nested containers also clear their ScrollFloat hover.
                for (int it = 0; it < m_items.GetSize(); ++it) {
                    CControlUI* pChild = static_cast<CControlUI*>(m_items[it]);
                    if (pChild != NULL) pChild->FindControl(Proc, pData, uFlags);
                }
                return NULL;
            }
        }
        if ((uFlags & UIFIND_UPDATETEST) != 0 && Proc(this, pData) != NULL) {
            return NULL;
        }

        CControlUI* pResult = NULL;
        if ((uFlags & UIFIND_ME_FIRST) != 0) {
            if ((uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled()) {
                pResult = Proc(this, pData);
            }
        }
        if (pResult == NULL && m_pVerticalScrollBar != NULL) {
            if ((uFlags & UIFIND_HITTEST) == 0 || (IsMouseEnabled() && IsScrollFloatShown())) {
                pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
            }
        }
        if (pResult == NULL && m_pHorizontalScrollBar != NULL) {
            if ((uFlags & UIFIND_HITTEST) == 0 || (IsMouseEnabled() && IsScrollFloatShown())) {
                pResult = m_pHorizontalScrollBar->FindControl(Proc, pData, uFlags);
            }
        }
        if (pResult != NULL) {
            return pResult;
        }

        if ((uFlags & UIFIND_HITTEST) == 0 || IsMouseChildEnabled()) 
        {
            RECT rc = m_rcItem;

            RECT rcInset = GetInset();

            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;

            if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
                if (!m_bScrollFloat) {
                    rc.right -= m_pVerticalScrollBar->GetFixedWidth();
                }
            }
            if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
                if (!m_bScrollFloat) {
                    rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
                }
            }
            if ((uFlags & UIFIND_TOP_FIRST) != 0)
            {

                for (int it = m_items.GetSize() - 1; it >= 0; it--) 
                {
                    pResult = static_cast<CControlUI*>(m_items[it])->FindControl(Proc, pData, uFlags);
                    if (pResult != NULL)
                    {
                        if ((uFlags & UIFIND_HITTEST) != 0 && !pResult->IsFloat() &&
                            !::PtInRect(&rc, *(static_cast<LPPOINT>(pData)))) 
                        {
                            continue;
                        } 
                        else
                        {
                            return pResult;
                        }
                    }
                }
            }
            else 
            {
                for (int it = 0; it < m_items.GetSize(); it++) 
                {
                    pResult = static_cast<CControlUI*>(m_items[it])->FindControl(Proc, pData, uFlags);
                    if (pResult != NULL) 
                    {
                        if ((uFlags & UIFIND_HITTEST) != 0 && !pResult->IsFloat() &&
                            !::PtInRect(&rc, *(static_cast<LPPOINT>(pData)))) {
                            continue;
                        }
                        else
                        {
                            return pResult;
                        }
                    }
                }
            }
        }

        pResult = NULL;
        if (pResult == NULL && (uFlags & UIFIND_ME_FIRST) == 0)
        {
            if ((uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled()) 
            {
                pResult = Proc(this, pData);
            }
        }
        return pResult;
    }

    bool CContainerUI::DoPaint(CPaintRenderContext& renderContext,
        CControlUI* pStopControl) {
        const RECT& rcPaint = renderContext.GetPaintRect();

        RECT rcTemp = { 0 };
        if (!::IntersectRect(&rcTemp, &rcPaint, &m_rcItem)) {
            return true;
        }

        CRenderClip clip;
        CRenderClip::GenerateClip(renderContext, rcTemp, clip);
        CControlUI::DoPaint(renderContext, pStopControl);

        if (m_items.GetSize() > 0) {
            RECT rcInset = GetInset();
            //娴滃窋dfium 閻ㄥ嫯顔曠純?
            rcInset.top = 0;
            RECT rc = m_rcItem;
            rc.left += rcInset.left;
            rc.top += rcInset.top;
            rc.right -= rcInset.right;
            rc.bottom -= rcInset.bottom;
            if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
                if (!m_bScrollFloat) {
                    rc.right -= m_pVerticalScrollBar->GetFixedWidth();
                }
            }
            if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
                if (!m_bScrollFloat) {
                    rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
                }
            }

            if (!::IntersectRect(&rcTemp, &rcPaint, &rc))
            {
                for (int it = 0; it < m_items.GetSize(); it++)
                {
                    CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
                    if (pControl == pStopControl) {
                        return false;
                    }
                    if (!pControl->IsVisible()) {
                        continue;
                    }
                    if (!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos())) {
                        continue;
                    }
                    if (pControl->IsFloat()) {
                        if (!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos())) {
                            continue;
                        }
                        if (!pControl->Paint(renderContext, pStopControl)) {
                            return false;
                        }
                    }
                }
            }
            else
            {
                CRenderClip childClip;
                CRenderClip::GenerateClip(renderContext, rcTemp, childClip);
                for (int it = 0; it < m_items.GetSize(); it++)
                {
                    CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
                    if (!::IntersectRect(&rcTemp, &rcPaint, &pControl->GetPos())) {
                        continue;
                    }
                    if (pControl == pStopControl) {
                        return false;
                    }
                    if (!pControl->IsVisible()) {
                        continue;
                    }

                    if (pControl->IsFloat()) {
                        if (!::IntersectRect(&rcTemp, &m_rcItem, &pControl->GetPos())) {
                            continue;
                        }
                        CRenderClip::UseOldClipBegin(renderContext, childClip);
                        if (!pControl->Paint(renderContext, pStopControl)) {
                            return false;
                        }
                        CRenderClip::UseOldClipEnd(renderContext, childClip);
                    }
                    else {
                        if (!::IntersectRect(&rcTemp, &rc, &pControl->GetPos())) {
                            continue;
                        }
                        if (!pControl->Paint(renderContext, pStopControl)) {
                            return false;
                        }
                    }
                }
            }
        }

        if (m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible() && IsScrollFloatShown()) {
            if (m_pVerticalScrollBar == pStopControl) {
                return false;
            }
            if (::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos())) {
                if (!m_pVerticalScrollBar->Paint(renderContext, pStopControl)) {
                    return false;
                }
            }
        }

        if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible() && IsScrollFloatShown()) {
            if (m_pHorizontalScrollBar == pStopControl) {
                return false;
            }
            if (::IntersectRect(&rcTemp, &rcPaint, &m_pHorizontalScrollBar->GetPos())) {
                if (!m_pHorizontalScrollBar->Paint(renderContext, pStopControl)) {
                    return false;
                }
            }
        }
        return true;
    }



    void CContainerUI::SetFloatPos(int iIndex)
    {
       
        if (iIndex < 0 || iIndex >= m_items.GetSize()) {
            return;
        }

        CControlUI* pControl = static_cast<CControlUI*>(m_items[iIndex]);

        if (!pControl->IsVisible()) {
            return;
        }
        if (!pControl->IsFloat()) {
            return;
        }
       
        SIZE szXY = pControl->GetFixedXY();
        SIZE sz = { pControl->GetFixedWidth(), pControl->GetFixedHeight() };

        int nParentWidth = m_rcItem.right - m_rcItem.left;
        int nParentHeight = m_rcItem.bottom - m_rcItem.top;

        UINT uAlign = pControl->GetFloatAlign();
        if (uAlign != 0) 
        {
            RECT rcCtrl = { 0, 0, sz.cx, sz.cy };
            if ((uAlign & DT_CENTER) != 0) {
                ::OffsetRect(&rcCtrl, (nParentWidth - sz.cx) / 2, 0);
            }
            else if ((uAlign & DT_RIGHT) != 0) {
                ::OffsetRect(&rcCtrl, nParentWidth - sz.cx, 0);
            }
            else {
                ::OffsetRect(&rcCtrl, szXY.cx, 0);
            }

            if ((uAlign & DT_VCENTER) != 0) {
                ::OffsetRect(&rcCtrl, 0, (nParentHeight - sz.cy) / 2);
            }
            else if ((uAlign & DT_BOTTOM) != 0) {
                ::OffsetRect(&rcCtrl, 0, nParentHeight - sz.cy);
            }
            else {
                ::OffsetRect(&rcCtrl, 0, szXY.cy);
            }

            ::OffsetRect(&rcCtrl, m_rcItem.left, m_rcItem.top);
            pControl->SetPos(rcCtrl, false);
        }
        else if (pControl->HasFloatingRatio())
        {
            // floating_ratio：控件中心定位于父 (rx*W, ry*H) 处
            sz = pControl->EstimateSize(sz);
            if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
            if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
            if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
            if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

            const double rx = pControl->GetFloatingRatioX();
            const double ry = pControl->GetFloatingRatioY();
            const LONG cxCenter = m_rcItem.left + static_cast<LONG>(nParentWidth * rx);
            const LONG cyCenter = m_rcItem.top + static_cast<LONG>(nParentHeight * ry);

            RECT rcCtrl = { 0 };
            rcCtrl.left = cxCenter - sz.cx / 2;
            rcCtrl.top = cyCenter - sz.cy / 2;
            rcCtrl.right = rcCtrl.left + sz.cx;
            rcCtrl.bottom = rcCtrl.top + sz.cy;
            pControl->SetPos(rcCtrl, true);
        }
        else if (pControl->HasFloatRBPadding())
        {
            // float_right_bottom_padding：控件右下角距父右下角 (px, py) 逻辑像素
            sz = pControl->EstimateSize(sz);
            if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
            if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
            if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
            if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

            // 逻辑像素 → 设备像素（考虑 DPI 缩放）
            const int padX = (m_pManager != NULL) ? m_pManager->ScaleValue(pControl->GetFloatRBPaddingX()) : pControl->GetFloatRBPaddingX();
            const int padY = (m_pManager != NULL) ? m_pManager->ScaleValue(pControl->GetFloatRBPaddingY()) : pControl->GetFloatRBPaddingY();

            RECT rcCtrl = { 0 };
            rcCtrl.right = m_rcItem.right - padX;
            rcCtrl.bottom = m_rcItem.bottom - padY;
            rcCtrl.left = rcCtrl.right - sz.cx;
            rcCtrl.top = rcCtrl.bottom - sz.cy;
            pControl->SetPos(rcCtrl, true);
        }
        else
        {
            sz = pControl->EstimateSize(sz);
            if (sz.cx < pControl->GetMinWidth()) sz.cx = pControl->GetMinWidth();
            if (sz.cx > pControl->GetMaxWidth()) sz.cx = pControl->GetMaxWidth();
            if (sz.cy < pControl->GetMinHeight()) sz.cy = pControl->GetMinHeight();
            if (sz.cy > pControl->GetMaxHeight()) sz.cy = pControl->GetMaxHeight();

            TPercentInfo rcPercent = pControl->GetFloatPercent();
            LONG width = m_rcItem.right - m_rcItem.left;
            LONG height = m_rcItem.bottom - m_rcItem.top;
            LONG left = m_rcItem.left;
            LONG top = m_rcItem.top;
            RECT rcCtrl = { 0 };

            rcCtrl.left = (LONG)((width - sz.cx) * rcPercent.left) + szXY.cx + left;
            rcCtrl.top = (LONG)((height - sz.cy) * rcPercent.top) + szXY.cy + top;
            rcCtrl.right = (LONG)((width - sz.cx) * rcPercent.left) + szXY.cx + sz.cx + left;
            rcCtrl.bottom = (LONG)((height - sz.cy) * rcPercent.top) + szXY.cy + sz.cy + top;
            pControl->SetPos(rcCtrl, true);
        }
    }

    void CContainerUI::ProcessScrollBar(RECT rc, int cxRequired, int cyRequired) {
        while (m_pHorizontalScrollBar) {
            // Scroll needed
            if (cxRequired > (rc.right - rc.left) &&
                !m_pHorizontalScrollBar->IsVisible()) {
                m_pHorizontalScrollBar->SetVisible(true);
                m_pHorizontalScrollBar->SetScrollRange(cxRequired - (rc.right - rc.left));
                m_pHorizontalScrollBar->SetScrollPos(0);
                SetPos(m_rcItem);
                break;
            }

            // No scrollbar required
            if (!m_pHorizontalScrollBar->IsVisible()) {
                break;
            }

            // Scroll not needed anymore?
            int cxScroll = cxRequired - (rc.right - rc.left);
            if (cxScroll <= 0) {
                m_pHorizontalScrollBar->SetVisible(false);
                m_pHorizontalScrollBar->SetScrollPos(0);
                m_pHorizontalScrollBar->SetScrollRange(0);
                SetPos(m_rcItem);
            }
            else {
                RECT rcScrollBarPos;
                int nHSpace = m_pHorizontalScrollBar->GetHSpace();
                if (m_bScrollFloat) {
                    rcScrollBarPos = {
                        rc.left,
                        rc.bottom - m_pHorizontalScrollBar->GetFixedHeight() - nHSpace,
                        rc.right,
                        rc.bottom - nHSpace };
                }
                else {
                    rcScrollBarPos = {
                        rc.left, rc.bottom, rc.right,
                        rc.bottom + m_pHorizontalScrollBar->GetFixedHeight() };
                }
                m_pHorizontalScrollBar->SetPos(rcScrollBarPos);

                if (m_pHorizontalScrollBar->GetScrollRange() != cxScroll) {
                    int iScrollPos = m_pHorizontalScrollBar->GetScrollPos();
                    m_pHorizontalScrollBar->SetScrollRange(
                        ::abs(cxScroll));  // if scrollpos>range then scrollpos=range
                    if (iScrollPos > m_pHorizontalScrollBar->GetScrollPos()) {
                        SetPos(m_rcItem);
                    }
                }
            }
            break;
        }

        while (m_pVerticalScrollBar) {
            // Scroll needed
            if (cyRequired > rc.bottom - rc.top && !m_pVerticalScrollBar->IsVisible()) {
                m_pVerticalScrollBar->SetVisible(true);
                m_pVerticalScrollBar->SetScrollRange(cyRequired - (rc.bottom - rc.top));
                m_pVerticalScrollBar->SetScrollPos(0);
                SetPos(m_rcItem);
                break;
            }

            // No scrollbar required
            if (!m_pVerticalScrollBar->IsVisible()) {
                break;
            }

            // Scroll not needed anymore?
            int cyScroll = cyRequired - (rc.bottom - rc.top);
            if (cyScroll <= 0) {
                m_pVerticalScrollBar->SetVisible(false);
                m_pVerticalScrollBar->SetScrollPos(0);
                m_pVerticalScrollBar->SetScrollRange(0);
                SetPos(m_rcItem);
                break;
            }

            int nVSpace = m_pVerticalScrollBar->GetVSpace();
            RECT rcScrollBarPos;
            if (m_bScrollFloat) {
                rcScrollBarPos = {
                    rc.right - m_pVerticalScrollBar->GetFixedWidth() - nVSpace,
                    rc.top,
                    rc.right - nVSpace,
                    rc.bottom };
            }
            else {
                rcScrollBarPos = { (rc.right - nVSpace), rc.top,
                    (rc.right + m_pVerticalScrollBar->GetFixedWidth() - nVSpace),
                    rc.bottom };
            }


            m_pVerticalScrollBar->SetPos(rcScrollBarPos);

            if (m_pVerticalScrollBar->GetScrollRange() != cyScroll) {
                int iScrollPos = m_pVerticalScrollBar->GetScrollPos();
                m_pVerticalScrollBar->SetScrollRange(
                    ::abs(cyScroll));  // if scrollpos>range then scrollpos=range
                if (iScrollPos > m_pVerticalScrollBar->GetScrollPos()) {
                    SetPos(m_rcItem);
                }
            }
            break;
        }
    }

    bool CContainerUI::SetSubControlText(std::wstring_view pstrSubControlName,
        std::wstring_view pstrText) {
        CControlUI* pSubControl = NULL;
        pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl != NULL) {
            pSubControl->SetText(pstrText);
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

    bool CContainerUI::SetSubControlFixedHeight(std::wstring_view pstrSubControlName,
        int cy) {
        CControlUI* pSubControl = NULL;
        pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl != NULL) {
            pSubControl->SetFixedHeight(cy);
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

    bool CContainerUI::SetSubControlFixedWdith(std::wstring_view pstrSubControlName, int cx)
    {
        CControlUI* pSubControl = nullptr;
        pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl != nullptr)
        {
            pSubControl->SetFixedWidth(cx);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool CContainerUI::SetSubControlUserData(std::wstring_view pstrSubControlName,
        std::wstring_view pstrText) {
        CControlUI* pSubControl = NULL;
        pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl != NULL) {
            pSubControl->SetUserData(pstrText);
            return TRUE;
        }
        else {
            return FALSE;
        }
    }

    bool CContainerUI::SetSubControlAttribute(std::wstring_view pstrSubControlName, std::wstring_view pstrName, std::wstring_view pstrValue)
    {
        CControlUI* pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl != nullptr)
            pSubControl->SetAttribute(pstrName, pstrValue);
        else
            return false;
        return true;
    }

    std::wstring CContainerUI::GetSubControlText(std::wstring_view pstrSubControlName) {
        CControlUI* pSubControl = NULL;
        pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl == NULL) {
            return _T("");
        }
        else {
            return pSubControl->GetText();
        }
    }

    int CContainerUI::GetSubControlFixedHeight(std::wstring_view pstrSubControlName) {
        CControlUI* pSubControl = NULL;
        pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl == NULL) {
            return -1;
        }
        else {
            return pSubControl->GetFixedHeight();
        }
    }

    int CContainerUI::GetSubControlFixedWdith(std::wstring_view pstrSubControlName) {
        CControlUI* pSubControl = NULL;
        pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl == NULL) {
            return -1;
        }
        else {
            return pSubControl->GetFixedWidth();
        }
    }

    std::wstring CContainerUI::GetSubControlUserData(
        std::wstring_view pstrSubControlName) {
        CControlUI* pSubControl = NULL;
        pSubControl = this->FindSubControl(pstrSubControlName);
        if (pSubControl == NULL) {
            return _T("");
        }
        else {
            return pSubControl->GetUserData();
        }
    }

    
    CControlUI* CContainerUI::FindSubControl(std::wstring_view pstrSubControlName, std::wstring_view strEliminateControlName)
    {
        CControlUI* pSubControl = nullptr;
        if (strEliminateControlName.empty())
            pSubControl = FindSubControl(this, pstrSubControlName);
        else
            pSubControl = FindSubControl(this, pstrSubControlName, strEliminateControlName);
        return pSubControl;
    }

    // ============================================================
    // 显隐动画实现
    // ============================================================

    void CContainerUI::SetShowHideAnimDir(ShowHideAnimDir dir)
    {
        if (m_animDir == dir) return;
        if (m_bAnimating) StopShowHideAnim();
        m_animDir = dir;
    }

    CContainerUI::ShowHideAnimDir CContainerUI::GetShowHideAnimDir() const
    {
        return m_animDir;
    }

    bool CContainerUI::IsShowHideAnimating() const
    {
        return m_bAnimating;
    }

    void CContainerUI::StartShowHideAnim(bool bShow)
    {
        if (m_pManager == nullptr) return;

        m_bAnimShowing = bShow;

        if (bShow) {
            // 若此前从未记录过原始尺寸（例如 XML 设置 visible=false 后首次 Show），
            // 使用当前的 FixedWidth/Height 作为原始值；一旦有值就保持稳定。
            if (m_nAnimOrigFixedSize <= 0) {
                const int currentFixed = (m_animDir == AnimHor) ? m_cxyFixed.cx : m_cxyFixed.cy;
                if (currentFixed > 0) {
                    m_nAnimOrigFixedSize = currentFixed;
                }
            }

            // 强制以 m_nAnimOrigFixedSize 为目标（原实现仅在 <=0 时赋值，
            // 会错误地沿用上一轮 hide 路径残留的 m_nAnimTargetPx）。
            const int defaultLogical = 200;
            const int targetLogical = (m_nAnimOrigFixedSize > 0) ? m_nAnimOrigFixedSize : defaultLogical;
            m_nAnimTargetPx = m_pManager->ScaleValue(targetLogical);

            m_nAnimCurrentPx = 1;
            if (m_animDir == AnimHor) SetFixedWidth(1, false);
            else SetFixedHeight(1, false);

            CControlUI::SetVisible(true, false);
            for (int it = 0; it < m_items.GetSize(); it++)
                static_cast<CControlUI*>(m_items[it])->SetInternVisible(true);
            NeedParentUpdate();
        }
        else {
            // 原始固定尺寸仅 Lazy-init 一次；否则反复 Hide 会把已经被动画
            // 改小的 m_cxyFixed 当作"原始尺寸"保存，导致尺寸逐次衰减。
            if (m_nAnimOrigFixedSize <= 0) {
                m_nAnimOrigFixedSize = (m_animDir == AnimHor) ? m_cxyFixed.cx : m_cxyFixed.cy;
            }
            m_nAnimCurrentPx = (m_animDir == AnimHor) ? GetWidth() : GetHeight();
            m_nAnimTargetPx = m_nAnimCurrentPx;

            if (m_nAnimCurrentPx <= 1) {
                CControlUI::SetVisible(false, true);
                for (int it = 0; it < m_items.GetSize(); it++)
                    static_cast<CControlUI*>(m_items[it])->SetInternVisible(false);
                NeedParentUpdate();
                return;
            }
        }

        m_bAnimating = true;
        if (!m_pManager->SetTimer(this, SHOWHIDE_ANIM_TIMERID, SHOWHIDE_ANIM_INTERVAL_MS)) {
            StopShowHideAnim();
            if (!bShow) {
                CControlUI::SetVisible(false, true);
                for (int it = 0; it < m_items.GetSize(); it++)
                    static_cast<CControlUI*>(m_items[it])->SetInternVisible(false);
                NeedParentUpdate();
            }
        }
    }

    void CContainerUI::AdvanceShowHideAnim()
    {
        if (!m_bAnimating || m_nAnimTargetPx <= 0) return;

        const int totalFrames = 100 / SHOWHIDE_ANIM_INTERVAL_MS;
        int step = m_nAnimTargetPx / (totalFrames > 0 ? totalFrames : 1);
        if (step < 1) step = 1;
        const int snapThreshold = step * 2;

        if (m_bAnimShowing) {
            const int remaining = m_nAnimTargetPx - m_nAnimCurrentPx;
            if (remaining <= snapThreshold) {
                StopShowHideAnim();
                if (m_animDir == AnimHor) SetFixedWidth(m_nAnimOrigFixedSize, true);
                else SetFixedHeight(m_nAnimOrigFixedSize, true);
                return;
            }
            m_nAnimCurrentPx += step;
        }
        else {
            if (m_nAnimCurrentPx <= snapThreshold) {
                StopShowHideAnim();
                if (m_animDir == AnimHor) SetFixedWidth(m_nAnimOrigFixedSize, false);
                else SetFixedHeight(m_nAnimOrigFixedSize, false);

                CControlUI::SetVisible(false, true);
                for (int it = 0; it < m_items.GetSize(); it++)
                    static_cast<CControlUI*>(m_items[it])->SetInternVisible(false);
                NeedParentUpdate();
                return;
            }
            m_nAnimCurrentPx -= step;
            if (m_nAnimCurrentPx < 1) m_nAnimCurrentPx = 1;
        }

        int logicalSize = m_pManager ? m_pManager->UnscaleValue(m_nAnimCurrentPx) : m_nAnimCurrentPx;
        if (logicalSize <= 0 && m_nAnimCurrentPx > 0) 
            logicalSize = 1;

        if (m_animDir == AnimHor) 
            SetFixedWidth(logicalSize, true);
        else 
            SetFixedHeight(logicalSize, true);
    }

    void CContainerUI::StopShowHideAnim()
    {
        if (!m_bAnimating) return;
        m_bAnimating = false;
        if (m_pManager)
            m_pManager->KillTimer(this, SHOWHIDE_ANIM_TIMERID);
    }
}  // namespace FYUI


