#pragma once

#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    class DemoPopupWindow final : public FYUI::WindowImplBase
    {
    public:
        explicit DemoPopupWindow(bool autoDestroy);

        std::wstring GetSkinFile() override;
        std::wstring_view GetWindowClassName() const override;
        void InitWindow() override;
        void OnClick(FYUI::TNotifyUI& msg) override;
        void OnFinalMessage(HWND hWnd) override;

    private:
        bool autoDestroy_ = false;
    };
}
