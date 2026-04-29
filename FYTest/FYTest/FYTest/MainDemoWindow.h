#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    class MainDemoWindow final : public FYUI::WindowImplBase
    {
    public:
        std::wstring GetSkinFile() override;
        std::wstring_view GetWindowClassName() const override;
        std::wstring_view GetManagerName() override;
        void InitWindow() override;
        void Notify(FYUI::TNotifyUI& msg) override;
        void OnClick(FYUI::TNotifyUI& msg) override;
        LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
        LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;

    private:
        void SetupVirtualListDemo();
        void ApplyVirtualListFixedDemo(std::uint64_t count, int height, std::wstring modeText);
        void ApplyVirtualListVariableDemo(size_t count);
        void MutateVirtualListHeights();
        void MutateVirtualListCount();
        void UpdateVirtualListStatus(const std::wstring& prefix = L"");
        std::vector<int> BuildVirtualListWaveHeights(size_t count) const;
        void SetStatusText(const std::wstring& text);
        void UpdateValueStatus(FYUI::CControlUI* sender);
        void UpdateStatusFromNotify(const FYUI::TNotifyUI& msg);
        void OpenModalPopup();
        void OpenModelessPopup();
        void OpenPopupMenu(FYUI::CControlUI* anchor);
        void PopulateStressTiles();
        void StepProgress();
        void RefreshDiagnostics();
        void UpdateFpsMeter(bool resetSample = false);
        void UpdateFpsMeterIfDue();

        UINT last_fps_frame_count_ = 0;
        UINT last_scroll_cache_hit_count_ = 0;
        UINT last_active_scroll_fps_ = 0;
        std::chrono::steady_clock::time_point last_fps_sample_time_;
        std::chrono::steady_clock::time_point last_active_scroll_sample_time_;
        std::chrono::steady_clock::time_point last_fps_ui_update_time_;
        bool has_fps_sample_ = false;
        bool virtual_list_count_grow_ = true;
        std::wstring virtual_list_mode_text_ = L"fixed";
        std::vector<int> virtual_list_heights_;

        FYUI::CMenuWnd* m_pMenu = nullptr;
    };
}
