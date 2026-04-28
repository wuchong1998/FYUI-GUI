#include "pch.h"
#include "UIContainer.h"
#include "UIRenderContext.h"
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
        m_smoothScrollLastTick = std::chrono::steady_clock::now();
    }

      
    

    CContainerUI::~CContainerUI() {
        StopSmoothScroll(false);
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
            //鐠恒劎鐛ラ崣锝嗗珛閹疯姤妞傞崐?婵″倹鐏夐弰鐥砳chedit 閹貉傛闁库偓濮ｄ椒绱板畷鈺傜皾
            std::wstring strClass(pItem->GetClass());
            if (strClass == L"RichEditUI")
                continue;
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
        m_bCursorMouse = pControl->m_bCursorMouse;
        m_sVerticalScrollBarStyle = pControl->m_sVerticalScrollBarStyle;
        m_sHorizontalScrollBarStyle = pControl->m_sHorizontalScrollBarStyle;
        m_dwSepImmBorderColor = pControl->m_dwSepImmBorderColor;
        m_dwLeaveSepImmBorderColor = pControl->m_dwLeaveSepImmBorderColor;
 
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

    void CContainerUI::SetQuickScrolling(bool bQuickScrolling)
    {
        m_bQuickScrolling = bQuickScrolling;
    }

    bool CContainerUI::GetQuickScrolling()
    {
        return m_bQuickScrolling;
    }

    void CContainerUI::SetVisible(bool bVisible, bool bSendFocus) {
        if (m_bVisible == bVisible) {
            return;
        }
        CControlUI::SetVisible(bVisible, bSendFocus);
        for (int it = 0; it < m_items.GetSize(); it++) {
            static_cast<CControlUI*>(m_items[it])->SetInternVisible(IsVisible());
        }
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
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            rcView.right -= m_pVerticalScrollBar->GetFixedWidth();
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
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
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            iOffset -= m_pHorizontalScrollBar->GetFixedHeight();
        }
        ScrollByDelta(0, -iOffset, true, true, true);
    }

    void CContainerUI::PageDown() {
        int iOffset =
            m_rcItem.bottom - m_rcItem.top - m_rcInset.top - m_rcInset.bottom;
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
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
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
            iOffset -= m_pVerticalScrollBar->GetFixedWidth();
        }
        ScrollByDelta(-iOffset, 0, true, true, true);
    }

    void CContainerUI::PageRight() {
        RECT rcInset = GetInset();
        int iOffset = m_rcItem.right - m_rcItem.left - rcInset.left - rcInset.right;
        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
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
            rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            rc.left -= m_pHorizontalScrollBar->GetScrollPos();
            rc.right -= m_pHorizontalScrollBar->GetScrollPos();
            rc.right += m_pHorizontalScrollBar->GetScrollRange();
            rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
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
            rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
            rc.left -= m_pHorizontalScrollBar->GetScrollPos();
            rc.right -= m_pHorizontalScrollBar->GetScrollPos();
            rc.right += m_pHorizontalScrollBar->GetScrollRange();
            rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
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
        else if (StringUtil::EqualsNoCase(name, L"quick_scroll"))
        {
            SetQuickScrolling(StringUtil::ParseBool(pstrValueView));
        }
        else {
            CControlUI::SetAttribute(pstrNameView, pstrValueView);
        }
    }

    void CContainerUI::SetSepImmBorderColor(DWORD dwSepImmBorderColor)
    {
        m_dwSepImmBorderColor = dwSepImmBorderColor;
    }

    DWORD CContainerUI::GetSepImmBorderColor() const
    {
        return m_dwSepImmBorderColor;
    }

    void CContainerUI::SetSepImmLeaveBorderColor(DWORD dwSepImmBorderColor)
    {
        m_dwLeaveSepImmBorderColor = dwSepImmBorderColor;
    }

    DWORD CContainerUI::GetSepImmLeaveBorderColor() const
    {
        return m_dwLeaveSepImmBorderColor;
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

    //CControlUI* CContainerUI::FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags)
    //{

    //    if ((uFlags & UIFIND_VISIBLE) && !IsVisible()) return NULL;
    //    if ((uFlags & UIFIND_ENABLED) && !IsEnabled()) return NULL;

    //    POINT* pPt = NULL;
    //    const bool bHitTest = (uFlags & UIFIND_HITTEST) != 0;
    //    if (bHitTest)
    //    {
    //        pPt = static_cast<LPPOINT>(pData);
    //        if (!::PtInRect(&m_rcItem, *pPt)) return NULL;
    //    }

    //    if ((uFlags & UIFIND_UPDATETEST) && Proc(this, pData))
    //        return NULL;

    //    if ((uFlags & UIFIND_ME_FIRST))
    //    {
    //        if (!bHitTest || IsMouseEnabled())
    //        {
    //            CControlUI* pResult = Proc(this, pData);
    //            if (pResult) return pResult;
    //        }
    //    }


    //    auto CheckScrollBar = [&](CControlUI* pBar) -> CControlUI*
    //        {
    //            if (pBar && (!bHitTest || IsMouseEnabled()))
    //            {
    //                return pBar->FindControl(Proc, pData, uFlags);
    //            }
    //            return NULL;
    //        };

    //    if (CControlUI* pRet = CheckScrollBar(m_pVerticalScrollBar)) return pRet;
    //    if (CControlUI* pRet = CheckScrollBar(m_pHorizontalScrollBar)) return pRet;


    //    if (!bHitTest || IsMouseChildEnabled())
    //    {

    //        RECT rc = m_rcItem;
    //        const RECT rcInset = GetInset();
    //        rc.left += rcInset.left;
    //        rc.top += rcInset.top;
    //        rc.right -= rcInset.right;
    //        rc.bottom -= rcInset.bottom;

    //        if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible())
    //            rc.right -= m_pVerticalScrollBar->GetFixedWidth();
    //        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible())
    //            rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();

    //        const int nCount = m_items.GetSize();
    //        void** ppItems = m_items.c_str();


    //        const bool bCheckVisible = (uFlags & UIFIND_VISIBLE) != 0;
    //        const bool bTopFirst = (uFlags & UIFIND_TOP_FIRST) != 0;

    //        // 瀵邦亞骞嗘担鎾烩偓鏄忕帆鐏忎浇顥?
    //        auto FindInChild = [&](int index) -> CControlUI*
    //            {
    //                CControlUI* pItem = static_cast<CControlUI*>(ppItems[index]);

    //                if (bHitTest)
    //                {
    //                    // 婵″倹鐏夌憰浣圭湴閸欘垵顫嗛幀褝绱濇稉鏃€甯舵禒鏈电瑝閸欘垵顫嗛敍宀冪儲鏉?
    //                    if (bCheckVisible && !pItem->IsVisible())
    //                        return NULL;
    //                    if (!::PtInRect(&pItem->GetPos(), *pPt))
    //                        return NULL;
    //                }

    //                CControlUI* pResult = pItem->FindControl(Proc, pData, uFlags);
    //                if (pResult)
    //                {
    //                    if (bHitTest && !pResult->IsFloat() && !::PtInRect(&rc, *pPt)) {
    //                        return NULL;
    //                    }
    //                    return pResult;
    //                }
    //                return NULL;
    //            };

    //        // 閺嶈宓侀柆宥呭坊妞ゅ搫绨幍褑顢戝顏嗗箚
    //        if (bTopFirst)
    //        {
    //            for (int i = nCount - 1; i >= 0; --i)
    //            {
    //                if (CControlUI* pRet = FindInChild(i)) return pRet;
    //            }
    //        }
    //        else
    //        {
    //            for (int i = 0; i < nCount; ++i)
    //            {
    //                if (CControlUI* pRet = FindInChild(i)) return pRet;
    //            }
    //        }
    //    }


    //    if (!(uFlags & UIFIND_ME_FIRST)) {
    //        if (!bHitTest || IsMouseEnabled()) {
    //            return Proc(this, pData);
    //        }
    //    }

    //    return NULL;
    //}


    CControlUI* CContainerUI::FindControl(FINDCONTROLPROC Proc,
                                          LPVOID pData,
                                          UINT uFlags) {
      if ((uFlags & UIFIND_VISIBLE) != 0 && !IsVisible()) {
        return NULL;
      }
      if ((uFlags & UIFIND_ENABLED) != 0 && !IsEnabled()) {
        return NULL;
      }
      if ((uFlags & UIFIND_HITTEST) != 0 &&
          !::PtInRect(&m_rcItem, *(static_cast<LPPOINT>(pData)))) {
        return NULL;
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
        if ((uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled()) {
          pResult = m_pVerticalScrollBar->FindControl(Proc, pData, uFlags);
        }
      }
      if (pResult == NULL && m_pHorizontalScrollBar != NULL) {
        if ((uFlags & UIFIND_HITTEST) == 0 || IsMouseEnabled()) {
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
          rc.right -= m_pVerticalScrollBar->GetFixedWidth();
        }
        if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
          rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
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
                rc.right -= m_pVerticalScrollBar->GetFixedWidth();
            }
            if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
                rc.bottom -= m_pHorizontalScrollBar->GetFixedHeight();
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

        if (m_pVerticalScrollBar != NULL && m_pVerticalScrollBar->IsVisible()) {
            if (m_pVerticalScrollBar == pStopControl) {
                return false;
            }
            if (::IntersectRect(&rcTemp, &rcPaint, &m_pVerticalScrollBar->GetPos())) {
                if (!m_pVerticalScrollBar->Paint(renderContext, pStopControl)) {
                    return false;
                }
            }
        }

        if (m_pHorizontalScrollBar != NULL && m_pHorizontalScrollBar->IsVisible()) {
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


 
    void CContainerUI::SetFloatPos(int iIndex) {
        // 閸ョ姳璐烠ControlUI::SetPos鐎电loat閻ㄥ嫭鎼锋担婊冨閸濆稄绱濇潻娆撳櫡娑撳秷鍏樼€电loat缂佸嫪娆㈠ǎ璇插濠婃艾濮╅弶锛勬畱瑜板崬鎼?
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
        //std::wstring strName= pControl->GetName();

        SIZE szXY = pControl->GetFixedXY();
        SIZE sz = { pControl->GetFixedWidth(), pControl->GetFixedHeight() };

        int nParentWidth = m_rcItem.right - m_rcItem.left;
        int nParentHeight = m_rcItem.bottom - m_rcItem.top;

        UINT uAlign = pControl->GetFloatAlign();
        if (uAlign != 0) {
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
                RECT rcScrollBarPos = {
                    rc.left, rc.bottom, rc.right,
                    rc.bottom + m_pHorizontalScrollBar->GetFixedHeight() };
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
            RECT rcScrollBarPos = { (rc.right - nVSpace), rc.top,
                                   (rc.right + m_pVerticalScrollBar->GetFixedWidth() - nVSpace),
                                   rc.bottom };


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

    /*
    ** 閺嶈宓侀崥宥呯摟閺屻儲澹樼€涙劖甯舵禒璁圭礉鏉╂柨娲栭幍鎯у煂閻ㄥ嫮顑囨稉鈧稉顏呭付娴犺埖瀵氶柦?
    *  param pstrSubControlName: 鐎涙劖甯舵禒璺烘倳鐎?
    *  param strEliminateControlName: 閹烘帡娅庨弻鎰嚋閸氬秴鐡ч惃鍕付娴?娑撳秳绱伴崷銊嚉閸氬秶袨閹貉傛鏉╂稖顢戠€涙劖甯舵禒鑸电叀閹?
    *  return CControlUI*: 閹垫儳鍩岄惃鍕摍閹貉傛閹稿洭鎷￠敍灞炬弓閹垫儳鍩屾潻鏂挎礀nullptr
    */
    CControlUI* CContainerUI::FindSubControl(std::wstring_view pstrSubControlName, std::wstring_view strEliminateControlName)
    {
        CControlUI* pSubControl = nullptr;
        if (strEliminateControlName.empty())
            pSubControl = FindSubControl(this, pstrSubControlName);
        else
            pSubControl = FindSubControl(this, pstrSubControlName, strEliminateControlName);
        return pSubControl;
    }
}  // namespace DuiLib



