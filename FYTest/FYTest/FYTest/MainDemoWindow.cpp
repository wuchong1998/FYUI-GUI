#include "framework.h"
#include "MainDemoWindow.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>

#include "DemoPopupWindow.h"

namespace
{
    constexpr wchar_t kMainSkin[] = L"fytest_main.xml";
    constexpr wchar_t kMenuSkin[] = L"fytest_menu.xml";
    constexpr RECT kPopupCreateRect = { 0, 0, 640, 470 };
    constexpr DWORD kCreateHiddenDialogStyle = UI_WNDSTYLE_DIALOG & ~WS_VISIBLE;
    constexpr UINT_PTR kFpsTimerId = 1001;
    constexpr UINT kFpsTimerMs = 250;
    constexpr auto kScrollFpsUiInterval = std::chrono::milliseconds(500);
    constexpr int kStressTileTargetCount = 200;

    template <typename T>
    T* FindControlAs(FYUI::CPaintManagerUI& manager, const wchar_t* name)
    {
        return static_cast<T*>(manager.FindControl(name));
    }

    std::wstring FormatFixed2(double value)
    {
        const int scaled = static_cast<int>(value * 100.0 + 0.5);
        return std::to_wstring(scaled / 100) + L"." + (scaled % 100 < 10 ? L"0" : L"") + std::to_wstring(scaled % 100);
    }

    std::wstring FormatFpsTitle(UINT framesThisSecond, const FYUI::TRenderDiagnostics& diagnostics)
    {
        return L"FYTest FPS=" + std::to_wstring(framesThisSecond) +
            L"/s paint=" + FormatFixed2(diagnostics.nLastPaintMs) +
            L"ms avg=" + FormatFixed2(diagnostics.nAveragePaintMs) +
            L"ms scroll=" + std::to_wstring(diagnostics.nScrollRenderCacheHits) +
            L"/" + std::to_wstring(diagnostics.nScrollRenderCacheMisses) +
            L" d2d(f/s)=" + std::to_wstring(diagnostics.nDirect2DBatchFlushes) +
            L"/" + std::to_wstring(diagnostics.nDirect2DStandaloneDraws);
    }
}

namespace FYTestApp
{
    std::wstring MainDemoWindow::GetSkinFile()
    {
        return kMainSkin;
    }

    std::wstring_view MainDemoWindow::GetWindowClassName() const
    {
        return L"FYTestMainWindow";
    }

    std::wstring_view MainDemoWindow::GetManagerName()
    {
        return L"FYTestMain";
    }

    void MainDemoWindow::InitWindow()
    {
        PopulateStressTiles();
        ::SetTimer(*this, kFpsTimerId, kFpsTimerMs, nullptr);
        UpdateFpsMeter(true);

        if (auto* diagnostics = FindControlAs<FYUI::CLabelUI>(m_pm, L"diag_status")) {
            diagnostics->SetText(L"Direct2D diagnostics enabled. Interact with tabs, lists, edits, sliders and popups.");
        }
        if (auto* progress = FindControlAs<FYUI::CProgressUI>(m_pm, L"progress_runtime")) {
            progress->SetValue(62);
        }
    }

    void MainDemoWindow::Notify(FYUI::TNotifyUI& msg)
    {
        if (msg.sType == DUI_MSGTYPE_VALUECHANGED || msg.sType == DUI_MSGTYPE_VALUECHANGED_MOVE) {
            UpdateValueStatus(msg.pSender);
        }
        else if (msg.sType == DUI_MSGTYPE_SCROLL || msg.sType == DUI_MSGTYPE_SCROLL_TOOLS) {
            UpdateFpsMeterIfDue();
        }
        else if (msg.sType == DUI_MSGTYPE_TABSELECT || msg.sType == DUI_MSGTYPE_ITEMSELECT ||
            msg.sType == DUI_MSGTYPE_ITEMCLICK || msg.sType == DUI_MSGTYPE_COLORCHANGED) {
            UpdateStatusFromNotify(msg);
        }

        WindowImplBase::Notify(msg);
    }

    void MainDemoWindow::OnClick(FYUI::TNotifyUI& msg)
    {
        if (msg.pSender != nullptr) {
            const std::wstring& name = msg.pSender->GetName();
            if (name == L"open_modal") {
                OpenModalPopup();
                return;
            }
            if (name == L"open_modeless") {
                OpenModelessPopup();
                return;
            }
            if (name == L"open_menu") {
                OpenPopupMenu(msg.pSender);
                return;
            }
            if (name == L"refresh_metrics") {
                RefreshDiagnostics();
                return;
            }
            if (name == L"progress_step") {
                StepProgress();
                return;
            }
        }
        WindowImplBase::OnClick(msg);
    }

    LRESULT MainDemoWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        (void)uMsg;
        (void)wParam;
        (void)lParam;
        ::KillTimer(*this, kFpsTimerId);
        FYUI::ContextMenuParam closeMenus = { 1, NULL };
        FYUI::CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(closeMenus);
        FYUI::CMenuWnd::DestroyMenu();
        bHandled = FALSE;
        ::PostQuitMessage(0);
        return 0;
    }

    LRESULT MainDemoWindow::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        (void)lParam;
        if (uMsg == WM_TIMER && wParam == kFpsTimerId) {
            UpdateFpsMeter();
            bHandled = TRUE;
            return 0;
        }

        bHandled = FALSE;
        return 0;
    }

    void MainDemoWindow::SetStatusText(const std::wstring& text)
    {
        if (auto* status = FindControlAs<FYUI::CLabelUI>(m_pm, L"event_status")) {
            status->SetText(text);
        }
    }

    void MainDemoWindow::UpdateValueStatus(FYUI::CControlUI* sender)
    {
        if (sender == nullptr) {
            return;
        }

        std::wstring text = L"Value changed: ";
        text += sender->GetName();
        if (auto* slider = static_cast<FYUI::CSliderUI*>(sender->GetInterface(DUI_CTR_SLIDER))) {
            text += L" = ";
            text += std::to_wstring(slider->GetValue());
        }
        else if (auto* progress = static_cast<FYUI::CProgressUI*>(sender->GetInterface(DUI_CTR_PROGRESS))) {
            text += L" = ";
            text += std::to_wstring(progress->GetValue());
        }
        SetStatusText(text);
    }

    void MainDemoWindow::UpdateStatusFromNotify(const FYUI::TNotifyUI& msg)
    {
        std::wstring text = L"Notify: ";
        text += msg.sType;
        if (msg.pSender != nullptr) {
            text += L" from ";
            text += msg.pSender->GetName();
        }
        SetStatusText(text);
    }

    void MainDemoWindow::OpenModalPopup()
    {
        auto popup = std::make_unique<DemoPopupWindow>(false);
        if (popup->Create(*this, L"FYUI modal popup", kCreateHiddenDialogStyle, UI_WNDSTYLE_EX_DIALOG, kPopupCreateRect) == nullptr) {
            SetStatusText(L"Modal popup creation failed.");
            return;
        }
        popup->CenterWindow();
        const UINT result = popup->ShowModal();
        SetStatusText(result == IDOK ? L"Modal popup closed with OK." : L"Modal popup closed.");
    }

    void MainDemoWindow::OpenModelessPopup()
    {
        auto* popup = new DemoPopupWindow(true);
        if (popup->Create(*this, L"FYUI modeless popup", kCreateHiddenDialogStyle, UI_WNDSTYLE_EX_DIALOG, kPopupCreateRect) == nullptr) {
            delete popup;
            SetStatusText(L"Modeless popup creation failed.");
            return;
        }
        popup->CenterWindow();
        popup->ShowWindow(true);
        SetStatusText(L"Modeless popup opened. Main window remains interactive.");
    }

    void MainDemoWindow::OpenPopupMenu(FYUI::CControlUI* anchor)
    {
        POINT pt = { 0, 0 };
        if (anchor != nullptr) {
            const RECT rcAnchor = anchor->GetPos();
            pt.x = rcAnchor.left;
            pt.y = rcAnchor.bottom;
        }
        ::ClientToScreen(m_pm.GetPaintWindow(), &pt);

        FYUI::CMenuWnd::CreateMenu(
            nullptr,
            kMenuSkin,
            pt,
            &m_pm,
            nullptr,
            FYUI::eMenuAlignment_Left | FYUI::eMenuAlignment_Top);
        SetStatusText(L"Popup menu opened from standalone button.");
    }

    void MainDemoWindow::PopulateStressTiles()
    {
        auto* tiles = FindControlAs<FYUI::CTileLayoutUI>(m_pm, L"stress_tiles");
        if (tiles == nullptr) {
            return;
        }

        for (int index = tiles->GetCount() + 1; index <= kStressTileTargetCount; ++index) {
            auto* button = new FYUI::CButtonUI();
            wchar_t text[32] = {};
            swprintf_s(text, L"Button %03d", index);
            button->SetText(text);
            button->SetFixedWidth(120);
            if (!tiles->Add(button)) {
                delete button;
                break;
            }
            button->ApplyAttributeList(L"ActionButton");
        }
        tiles->NeedUpdate();
    }

    void MainDemoWindow::StepProgress()
    {
        if (auto* progress = FindControlAs<FYUI::CProgressUI>(m_pm, L"progress_runtime")) {
            int value = progress->GetValue() + 9;
            if (value > progress->GetMaxValue()) {
                value = progress->GetMinValue();
            }
            progress->SetValue(value);
            SetStatusText(L"Runtime progress advanced to " + std::to_wstring(value) + L"%.");
        }
    }

    void MainDemoWindow::RefreshDiagnostics()
    {
        FYUI::TRenderDiagnostics diagnostics = {};
        m_pm.GetRenderDiagnostics(diagnostics);

        std::wstring backend = L"direct2d";

        std::wstring mode = L"auto";
        if (diagnostics.activeDirect2DMode == FYUI::Direct2DRenderModeHardware) {
            mode = L"hardware";
        }
        else if (diagnostics.activeDirect2DMode == FYUI::Direct2DRenderModeSoftware) {
            mode = L"software";
        }

        std::wstring text = L"Render: " + backend + L" / " + mode +
            L", frames=" + std::to_wstring(diagnostics.nTotalFrames) +
            L", avg=" + std::to_wstring(static_cast<int>(diagnostics.nAveragePaintMs * 100.0) / 100.0) + L" ms" +
            L", fps=" + std::to_wstring(static_cast<int>(diagnostics.nAverageFPS));

        if (auto* label = FindControlAs<FYUI::CLabelUI>(m_pm, L"diag_status")) {
            label->SetText(text);
        }
        SetStatusText(L"Diagnostics refreshed.");
    }

    void MainDemoWindow::UpdateFpsMeter(bool resetSample)
    {
        const auto now = std::chrono::steady_clock::now();
        FYUI::TRenderDiagnostics diagnostics = {};
        m_pm.GetRenderDiagnostics(diagnostics);

        UINT paintFramesThisSecond = static_cast<UINT>(diagnostics.nAverageFPS);
        UINT scrollFramesThisSecond = 0;
        if (!resetSample && has_fps_sample_) {
            const double elapsedSeconds = (std::max)(
                std::chrono::duration<double>(now - last_fps_sample_time_).count(),
                0.001);
            const UINT paintFrameDelta = diagnostics.nTotalFrames >= last_fps_frame_count_
                ? diagnostics.nTotalFrames - last_fps_frame_count_
                : diagnostics.nTotalFrames;
            const UINT scrollFrameDelta = diagnostics.nScrollRenderCacheHits >= last_scroll_cache_hit_count_
                ? diagnostics.nScrollRenderCacheHits - last_scroll_cache_hit_count_
                : diagnostics.nScrollRenderCacheHits;
            paintFramesThisSecond = static_cast<UINT>(std::lround(paintFrameDelta / elapsedSeconds));
            scrollFramesThisSecond = static_cast<UINT>(std::lround(scrollFrameDelta / elapsedSeconds));
            if (scrollFrameDelta > 0) {
                last_active_scroll_fps_ = scrollFramesThisSecond;
                last_active_scroll_sample_time_ = now;
            }
        }

        last_fps_frame_count_ = diagnostics.nTotalFrames;
        last_scroll_cache_hit_count_ = diagnostics.nScrollRenderCacheHits;
        last_fps_sample_time_ = now;
        has_fps_sample_ = true;

        const bool hasRecentScrollSample =
            last_active_scroll_sample_time_.time_since_epoch().count() != 0 &&
            now - last_active_scroll_sample_time_ < std::chrono::milliseconds(1500);
        const UINT activeScrollFramesThisSecond = hasRecentScrollSample ? last_active_scroll_fps_ : scrollFramesThisSecond;
        const UINT framesThisSecond = (std::max)(paintFramesThisSecond, activeScrollFramesThisSecond);
        if (auto* fps = FindControlAs<FYUI::CButtonUI>(m_pm, L"fps_meter")) {
            fps->SetText(L"FPS " + std::to_wstring(framesThisSecond) + L"/s");
        }

        const std::wstring title = FormatFpsTitle(framesThisSecond, diagnostics);
        ::SetWindowText(*this, title.c_str());
    }

    void MainDemoWindow::UpdateFpsMeterIfDue()
    {
        const auto now = std::chrono::steady_clock::now();
        if (last_fps_ui_update_time_.time_since_epoch().count() != 0 &&
            now - last_fps_ui_update_time_ < kScrollFpsUiInterval) {
            return;
        }

        last_fps_ui_update_time_ = now;
        UpdateFpsMeter();
    }
}
