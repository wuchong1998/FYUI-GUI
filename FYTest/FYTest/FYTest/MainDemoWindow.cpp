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
    constexpr wchar_t kMenuSkin[] = L"right_title_pop_wnd.xml";
    constexpr RECT kPopupCreateRect = { 0, 0, 640, 470 };
    constexpr DWORD kCreateHiddenDialogStyle = UI_WNDSTYLE_DIALOG & ~WS_VISIBLE;
    constexpr auto kScrollFpsUiInterval = std::chrono::milliseconds(500);
    constexpr int kStressTileTargetCount = 200;

    template <typename T>
    T* FindControlAs(CPaintManagerUI& manager, const wchar_t* name)
    {
        return static_cast<T*>(manager.FindControl(name));
    }

    std::wstring FormatFixed2(double value)
    {
        const int scaled = static_cast<int>(value * 100.0 + 0.5);
        return std::to_wstring(scaled / 100) + L"." + (scaled % 100 < 10 ? L"0" : L"") + std::to_wstring(scaled % 100);
    }

    std::wstring FormatFpsTitle(UINT framesThisSecond, const TRenderDiagnostics& diagnostics)
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
        auto* stress_tiles = FindControlAs<CTileLayoutUI>(m_pm, L"stress_tiles");
        for (int index = stress_tiles->GetCount() + 1; index <= 200; ++index)
        {
            auto* button = new CButtonUI();
            //xml 中 ActionButton风格
            button->SetText(format(L"Button{}", index).c_str());
            button->ApplyAttributeList(L"ActionButton");
            stress_tiles->Add(button);
        }

        SetupVirtualListDemo();
        ::SetTimer(*this, 1001, 250, nullptr);

        auto* diag_status = FindControlAs<CLabelUI>(m_pm, L"diag_status");
        diag_status->SetText(L"Direct2D 诊断已启用。请与选项卡、列表、编辑框、滑块及弹出窗口进行交互。");
        auto* progress = FindControlAs<CProgressUI>(m_pm, L"progress_runtime");
        progress->SetValue(60);

        // WebP 测试控件：初始化状态显示
        UpdateWebpStatus();
        // Ring 测试控件：初始化状态显示
        UpdateRingStatus();
    }

    void MainDemoWindow::Notify(TNotifyUI& msg)
    {
        if (msg.sType == DUI_MSGTYPE_VALUECHANGED || msg.sType == DUI_MSGTYPE_VALUECHANGED_MOVE) {
            UpdateValueStatus(msg.pSender);
        }
        else if (msg.sType == DUI_MSGTYPE_SCROLL || msg.sType == DUI_MSGTYPE_SCROLL_TOOLS) {
            if (msg.pSender == FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo")) {
                UpdateVirtualListStatus(L"VirtualList scrolled.");
            }
            UpdateFpsMeterIfDue();
        }
        else if (msg.sType == DUI_MSGTYPE_TABSELECT || msg.sType == DUI_MSGTYPE_ITEMSELECT ||
            msg.sType == DUI_MSGTYPE_ITEMCLICK || msg.sType == DUI_MSGTYPE_COLORCHANGED) {
            if (msg.pSender == FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo")) {
                UpdateVirtualListStatus(L"VirtualList notify: " + msg.sType);
            }
            if (msg.sType == DUI_MSGTYPE_COLORCHANGED) {
                UpdatePaletteColorPreview(msg);
            }
            UpdateStatusFromNotify(msg);
        }

        WindowImplBase::Notify(msg);
    }

    void MainDemoWindow::OnClick(TNotifyUI& msg)
    {
        const std::wstring& name = msg.pSender->GetName();
        ToggleHideAnimationDemo(name);
        if (name == L"open_modal")
            OpenModalPopup();
        else if (name == L"open_modeless")
            OpenModelessPopup();
        else if (name == L"open_menu")
            OpenPopupMenu(msg.pSender);
        else if (name == L"refresh_metrics")
            RefreshDiagnostics();
        else if (name == L"progress_step") 
            StepProgress();
        if (name == L"virt_apply_fixed_demo")
            ApplyVirtualListFixedDemo(240, 32, L"预设 240 个子项");
        if (name == L"virt_apply_large_demo") 
            ApplyVirtualListFixedDemo(10000000ULL, 24, L"预设 10,000,000 个子项");
        if (name == L"virt_apply_variable_demo")
            ApplyVirtualListVariableDemo(1800000);
        if (name == L"virt_mutate_height")
            MutateVirtualListHeights();
        if (name == L"virt_change_count") 
            MutateVirtualListCount();
        if (name == L"virt_clear_selection")
        {
            if (auto* list = FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo")) {
                list->ClearSelection(true);
                UpdateVirtualListStatus(L"VirtualList 选定状态已清除。");
            }
            return;
        }

        // Ring 测试控件按钮处理
        if (auto* ring = FindControlAs<CRingUI>(m_pm, L"ring_demo")) {
            bool handled = true;
            if (name == L"ring_play")        ring->Play();
            else if (name == L"ring_pause")  ring->Pause();
            else if (name == L"ring_stop")   ring->Stop();
            else if (name == L"ring_slow")    ring->SetRotateSpeed(90.0f);    // 4 秒一圈
            else if (name == L"ring_normal")  ring->SetRotateSpeed(360.0f);   // 1 秒一圈
            else if (name == L"ring_fast")    ring->SetRotateSpeed(1080.0f);  // 3 圈/秒
            else if (name == L"ring_reverse") ring->SetRotateSpeed(-ring->GetRotateSpeed());
            else if (name == L"ring_autoplay") {
                if (auto* opt = static_cast<COptionUI*>(msg.pSender->GetInterface(DUI_CTR_CHECKBOX))) {
                    ring->SetAutoPlay(opt->IsSelected());
                }
            }
            else handled = false;
            if (handled) UpdateRingStatus();
        }

        // WebP 测试控件按钮处理
        if (auto* webp = FindControlAs<CWebpAnimUI>(m_pm, L"webp_demo")) {
            if (name == L"webp_play") {
                webp->PlayWebp();
                UpdateWebpStatus();
            }
            else if (name == L"webp_pause") {
                webp->PauseWebp();
                UpdateWebpStatus();
            }
            else if (name == L"webp_stop") {
                webp->StopWebp();
                UpdateWebpStatus();
            }
            else if (name == L"webp_restart") {
                webp->RestartWebp();
                UpdateWebpStatus();
            }
            else if (name == L"webp_step") {
                const UINT total = webp->GetWebpFrameCount();
                if (total > 0) {
                    const UINT next = (webp->GetCurrentWebpFrameIndex() + 1) % total;
                    webp->SeekToWebpFrame(next, true);
                }
                UpdateWebpStatus();
            }
            else if (name == L"webp_autoplay") {
                if (auto* opt = static_cast<COptionUI*>(msg.pSender->GetInterface(DUI_CTR_CHECKBOX))) {
                    webp->SetAutoPlay(opt->IsSelected());
                }
                UpdateWebpStatus();
            }
            else if (name == L"webp_loop") {
                if (auto* opt = static_cast<COptionUI*>(msg.pSender->GetInterface(DUI_CTR_CHECKBOX))) {
                    webp->SetLoop(opt->IsSelected());
                }
                UpdateWebpStatus();
            }
            else if (name == L"webp_hide_pause" || name == L"webp_hide_continue" || name == L"webp_hide_stop") {
                ApplyWebpHideActionFromOptions();
                UpdateWebpStatus();
            }
        }

        WindowImplBase::OnClick(msg);
    }

    void MainDemoWindow::UpdateRingStatus()
    {
        auto* ring = FindControlAs<CRingUI>(m_pm, L"ring_demo");
        auto* status = FindControlAs<CLabelUI>(m_pm, L"ring_status");
        if (ring == nullptr || status == nullptr) return;

        wchar_t buf[160] = {};
        swprintf_s(buf, L"状态: %s  速度: %.0f°/s  间隔: %dms  角度: %.0f°  AutoPlay: %s",
            ring->IsPlaying() ? L"播放中" : L"已停止",
            ring->GetRotateSpeed(),
            ring->GetRotateInterval(),
            ring->GetAngle(),
            ring->IsAutoPlay() ? L"on" : L"off");
        status->SetText(buf);
    }

    void MainDemoWindow::UpdateWebpStatus()
    {
        auto* webp = FindControlAs<CWebpAnimUI>(m_pm, L"webp_demo");
        auto* status = FindControlAs<CLabelUI>(m_pm, L"webp_status");
        if (webp == nullptr || status == nullptr) {
            return;
        }
        const UINT total = webp->GetWebpFrameCount();
        const UINT index = webp->GetCurrentWebpFrameIndex();
        std::wstring stateText;
        switch (webp->GetWebpPlaybackState()) {
        case CWebpAnimUI::WebpPlaybackState_Playing:   stateText = L"播放中"; break;
        case CWebpAnimUI::WebpPlaybackState_Paused:    stateText = L"已暂停"; break;
        case CWebpAnimUI::WebpPlaybackState_Completed: stateText = L"已完成"; break;
        case CWebpAnimUI::WebpPlaybackState_Stopped:
        default:                                       stateText = L"已停止"; break;
        }
        std::wstring text = L"状态：" + stateText
            + L"   帧 " + std::to_wstring(index + (total == 0 ? 0 : 1))
            + L"/" + std::to_wstring(total)
            + L"   AutoPlay=" + (webp->IsAutoPlay() ? L"on" : L"off")
            + L"   Loop=" + (webp->IsLoop() ? L"on" : L"off");
        status->SetText(text);
    }

    void MainDemoWindow::ApplyWebpHideActionFromOptions()
    {
        auto* webp = FindControlAs<CWebpAnimUI>(m_pm, L"webp_demo");
        if (webp == nullptr) {
            return;
        }
        auto isSelected = [this](const wchar_t* name) -> bool {
            auto* opt = FindControlAs<COptionUI>(m_pm, name);
            return opt != nullptr && opt->IsSelected();
        };
        if (isSelected(L"webp_hide_continue")) {
            webp->SetHideAction(CWebpAnimUI::WebpHideAction_Continue);
        }
        else if (isSelected(L"webp_hide_stop")) {
            webp->SetHideAction(CWebpAnimUI::WebpHideAction_Stop);
        }
        else {
            webp->SetHideAction(CWebpAnimUI::WebpHideAction_Pause);
        }
    }

    bool MainDemoWindow::ToggleHideAnimationDemo(const std::wstring& buttonName)
    {
        if (buttonName == L"hor_left_to_right")
        {
            auto* box = FindControlAs<CContainerUI>(m_pm, L"left_to_right_test1");
            box->SetVisible(!box->GetVisible());
        }
        else if (buttonName == L"hor_right_to_left")
        {
            auto* box = FindControlAs<CContainerUI>(m_pm, L"right_to_left1");
            box->SetVisible(!box->GetVisible());
        }
        else if (buttonName == L"hor_top_to_bottom")
        {
            auto* box = FindControlAs<CContainerUI>(m_pm, L"top_to_bottom1");
            box->SetVisible(!box->GetVisible());
        }
        else if (buttonName == L"hor_bottom_to_top")
        {
            auto* box = FindControlAs<CContainerUI>(m_pm, L"bottom_to_top1");
            box->SetVisible(!box->GetVisible());
        }
        return true;
    }

    void MainDemoWindow::SetupVirtualListDemo()
    {
        auto* virtual_list_demo = FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo");
        virtual_list_demo->SetOverscanItemCount(8);
        virtual_list_demo->SetAllowSelectionCancel(true);
        virtual_list_demo->SetItemBkColor(0xFFF7FAFD);
        virtual_list_demo->SetItemHotBkColor(0xFFEAF2FA);
        virtual_list_demo->SetItemSelectedBkColor(0xFFDCEBFA);
        //虚拟列表创建子列表回调
        virtual_list_demo->SetCreateItemCallback([](CVirtualListUI*) -> CControlUI*
        {
            auto* label = new CLabelUI();
            RECT padding = { 12, 0, 12, 0 };
            label->SetTextPadding(padding);
            label->SetTextColor(0xFF243042);
            label->SetBkColor(0x00FFFFFF);
            return label;
        });

        //虚拟列表设置刷新子项时的回调
        virtual_list_demo->SetBindItemCallback([this](CVirtualListUI* owner, CControlUI* control, CVirtualListUI::ItemIndex index)
        {
            auto* label = dynamic_cast<CLabelUI*>(control);
            if (label == nullptr)
                return;

            std::wstring text = L"#" + std::to_wstring(index);
            text += L"  height=" + std::to_wstring(owner->GetItemHeight(index));
            if (virtual_list_mode_text_.find(L"variable") != std::wstring::npos)
                text += L"  variable";
            else
                text += L"  fixed";

            if (owner->IsItemSelected(index))
                text += L"  [selected]";
            else if (owner->HasHotItem() && owner->GetHotItemIndex() == index)
                text += L"  [hot]";
            label->SetText(text);
        });
       
        virtual_list_demo->SetItemClickCallback([this](CVirtualListUI*, CVirtualListUI::ItemIndex index, TEventUI&) 
        {
            UpdateVirtualListStatus(L"点击虚拟列表子项 " + std::to_wstring(index) + L".");
        });
        virtual_list_demo->SetItemDoubleClickCallback([this](CVirtualListUI*, CVirtualListUI::ItemIndex index, TEventUI&) 
        {
            UpdateVirtualListStatus(L"双击虚拟列表子项 " + std::to_wstring(index) + L".");
        });

        ApplyVirtualListFixedDemo(240, 32, L"fixed 240 items");
    }

    void MainDemoWindow::ApplyVirtualListFixedDemo(std::uint64_t count, int height, std::wstring modeText)
    {
        auto* virtual_list_demo = FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo");

        virtual_list_heights_.clear();
        virtual_list_mode_text_ = std::move(modeText);
        virtual_list_demo->SetFixedItemHeight(height);
        virtual_list_demo->SetItemCount(count);
        virtual_list_demo->SetScrollOffset(0, false);
        virtual_list_demo->ClearSelection(false);
        UpdateVirtualListStatus(L"虚拟列表切换模式-固定子项高度 " + virtual_list_mode_text_ + L".");
    }

    void MainDemoWindow::ApplyVirtualListVariableDemo(size_t count)
    {
        auto* virtual_list_demo = FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo");
       
        virtual_list_heights_ = BuildVirtualListWaveHeights(count);
        virtual_list_mode_text_ = L"可变高度";
        virtual_list_demo->SetItemHeights(virtual_list_heights_);
        virtual_list_demo->SetScrollOffset(0, false);
        virtual_list_demo->ClearSelection(false);
       
        UpdateVirtualListStatus(format(L"虚拟列表切换可变高度模式.子项{}",count));
        int A = 0;
    }

    void MainDemoWindow::MutateVirtualListHeights()
    {
        auto* virtual_list_demo = FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo");
      
        if (virtual_list_demo->IsVariableHeightMode())
        {
            if (virtual_list_heights_.empty())  
                virtual_list_heights_ = BuildVirtualListWaveHeights(static_cast<size_t>(virtual_list_demo->GetItemCount()));
           
            for (size_t i = 0; i < virtual_list_heights_.size(); ++i) 
                if (i % 5 == 0)
                    virtual_list_heights_[i] = 26 + static_cast<int>((i * 11) % 46);


            virtual_list_demo->SetItemHeights(virtual_list_heights_);
            UpdateVirtualListStatus(L"VirtualList 改变了多个可变高度的列表项.");
            return;
        }

        const int nextHeight = virtual_list_demo->GetFixedItemHeight() >= 56 ? 24 : virtual_list_demo->GetFixedItemHeight() + 8;
        virtual_list_demo->SetFixedItemHeight(nextHeight);
        UpdateVirtualListStatus(L"VirtualList 已将固定项目高度更改为 " + std::to_wstring(nextHeight) + L".");
    }

    void MainDemoWindow::MutateVirtualListCount()
    {
        auto* virtual_list_demo = FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo");
        if (virtual_list_demo->IsVariableHeightMode())
        {
            size_t nextCount = virtual_list_heights_.size();
            if (virtual_list_count_grow_) 
                nextCount += 37;
            else  
                nextCount = nextCount > 60 ? nextCount - 41 : 96;

            virtual_list_count_grow_ = !virtual_list_count_grow_;
            virtual_list_heights_ = BuildVirtualListWaveHeights(nextCount);
            virtual_list_demo->SetItemHeights(virtual_list_heights_);
            UpdateVirtualListStatus(L"VirtualList 将变量项计数更改为 " + std::to_wstring(nextCount) + L".");
            return;
        }

        const std::uint64_t currentCount = virtual_list_demo->GetItemCount();
        const std::uint64_t nextCount = currentCount >= 10000000ULL ? 512ULL : currentCount + 128ULL;
        virtual_list_demo->SetItemCount(nextCount);
        UpdateVirtualListStatus(L"VirtualList 已将固定项目数更改为 " + std::to_wstring(nextCount) + L".");
    }

    void MainDemoWindow::UpdateVirtualListStatus(const std::wstring& prefix)
    {
        auto* virtual_list_demo = FindControlAs<CVirtualListUI>(m_pm, L"virtual_list_demo");
        auto* virtual_list_status = FindControlAs<CLabelUI>(m_pm, L"virtual_list_status");
      
        std::wstring text;
        if (!prefix.empty()) {
            text += prefix;
            text += L" ";
        }
        text += L"模式=" + virtual_list_mode_text_;
        text += L", 个数=" + std::to_wstring(virtual_list_demo->GetItemCount());
        text += L", 滚动距离=" + std::to_wstring(virtual_list_demo->GetScrollOffset());
        text += L", 选择下标=";
        text += (virtual_list_demo->HasSelection() ? std::to_wstring(virtual_list_demo->GetSelectedIndex()) : L"none");
        text += L", 鼠标放上去的控件下标=";
        text += (virtual_list_demo->HasHotItem() ? std::to_wstring(virtual_list_demo->GetHotItemIndex()) : L"none");
        text += L", 是否允许取消选择=";
        text += (virtual_list_demo->IsAllowSelectionCancel() ? L"true" : L"false");
        virtual_list_status->SetText(text);
    }

    std::vector<int> MainDemoWindow::BuildVirtualListWaveHeights(size_t count) const
    {
        std::vector<int> heights;
        heights.reserve(count);
        for (size_t i = 0; i < count; ++i)
        {
            heights.push_back(24 + static_cast<int>((i * 9 + (i % 3) * 7) % 40));
        }
        return heights;
    }

    LRESULT MainDemoWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        (void)uMsg;
        (void)wParam;
        (void)lParam;
        ::KillTimer(*this, 1001);
        ContextMenuParam closeMenus = { 1, NULL };
        CMenuWnd::GetGlobalContextMenuObserver().RBroadcast(closeMenus);
        CMenuWnd::DestroyMenu();
        bHandled = FALSE;
        ::PostQuitMessage(0);
        return 0;
    }

    LRESULT MainDemoWindow::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        (void)lParam;
        if (uMsg == WM_TIMER && wParam == 1001) {
            UpdateFpsMeter();
            bHandled = TRUE;
            return 0;
        }

        bHandled = FALSE;
        return 0;
    }

    void MainDemoWindow::SetStatusText(const std::wstring& text)
    {
        if (auto* status = FindControlAs<CLabelUI>(m_pm, L"event_status")) {
            status->SetText(text);
        }
    }

    void MainDemoWindow::UpdateValueStatus(CControlUI* sender)
    {
        if (sender == nullptr) {
            return;
        }

        std::wstring text = L"Value changed: ";
        text += sender->GetName();
        if (auto* slider = static_cast<CSliderUI*>(sender->GetInterface(DUI_CTR_SLIDER))) {
            text += L" = ";
            text += std::to_wstring(slider->GetValue());
        }
        else if (auto* progress = static_cast<CProgressUI*>(sender->GetInterface(DUI_CTR_PROGRESS))) {
            text += L" = ";
            text += std::to_wstring(progress->GetValue());
        }
        SetStatusText(text);
    }

    void MainDemoWindow::UpdateStatusFromNotify(const TNotifyUI& msg)
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

    void MainDemoWindow::UpdatePaletteColorPreview(const TNotifyUI& msg)
    {
        if (msg.pSender == nullptr) return;
        auto* palette = static_cast<CColorPaletteUI*>(msg.pSender->GetInterface(DUI_CTR_COLORPALETTE));
        if (palette == nullptr) return;

        const DWORD color = palette->GetSelectColor(); // 0xFFRRGGBB
        auto* preview = FindControlAs<CButtonUI>(m_pm, L"palette_color_preview");
        if (preview == nullptr) return;

        preview->SetBkColor(color);
        preview->SetHotBkColor(color);
        preview->SetPushedBkColor(color);

        // 根据亮度自动选择前景色，保证文本可读
        const int r = (color >> 16) & 0xFF;
        const int g = (color >> 8) & 0xFF;
        const int b = color & 0xFF;
        const int luma = (r * 299 + g * 587 + b * 114) / 1000;
        preview->SetTextColor(luma > 140 ? 0xFF243042 : 0xFFFFFFFF);

        wchar_t buf[32] = {};
        swprintf_s(buf, L"#%02X%02X%02X", r, g, b);
        preview->SetText(buf);
    }

    void MainDemoWindow::OpenPopupMenu(CControlUI* anchor)
    {
        POINT pt = { 0, 0 };
        if (anchor != nullptr) {
            const RECT rcAnchor = anchor->GetPos();
            pt.x = rcAnchor.left;
            pt.y = rcAnchor.bottom;
        }
        ::ClientToScreen(m_pm.GetPaintWindow(), &pt);

        m_pMenu = CMenuWnd::CreateMenu(
            nullptr,
            kMenuSkin,
            pt,
            &m_pm,
            nullptr,
            eMenuAlignment_Left | eMenuAlignment_Top);
        SetStatusText(L"Popup menu opened from standalone button.");
    }

  

    void MainDemoWindow::StepProgress()
    {
        if (auto* progress = FindControlAs<CProgressUI>(m_pm, L"progress_runtime")) {
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
        TRenderDiagnostics diagnostics = {};
        m_pm.GetRenderDiagnostics(diagnostics);

        std::wstring backend = L"direct2d";

        std::wstring mode = L"auto";
        if (diagnostics.activeDirect2DMode == Direct2DRenderModeHardware) {
            mode = L"hardware";
        }
        else if (diagnostics.activeDirect2DMode == Direct2DRenderModeSoftware) {
            mode = L"software";
        }

        std::wstring text = L"Render: " + backend + L" / " + mode +
            L", frames=" + std::to_wstring(diagnostics.nTotalFrames) +
            L", avg=" + std::to_wstring(static_cast<int>(diagnostics.nAveragePaintMs * 100.0) / 100.0) + L" ms" +
            L", fps=" + std::to_wstring(static_cast<int>(diagnostics.nAverageFPS));

        if (auto* label = FindControlAs<CLabelUI>(m_pm, L"diag_status")) {
            label->SetText(text);
        }
        SetStatusText(L"Diagnostics refreshed.");
    }

    void MainDemoWindow::UpdateFpsMeter(bool resetSample)
    {
        const auto now = std::chrono::steady_clock::now();
        TRenderDiagnostics diagnostics = {};
        m_pm.GetRenderDiagnostics(diagnostics);

        UINT paintFramesThisSecond = static_cast<UINT>(diagnostics.nAverageFPS);
        UINT scrollFramesThisSecond = 0;
        if (!resetSample && has_fps_sample_) 
        {
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
        if (auto* fps = FindControlAs<CButtonUI>(m_pm, L"fps_meter")) {
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
