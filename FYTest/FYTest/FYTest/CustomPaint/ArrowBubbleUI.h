#pragma once
#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    // 箭头与气泡展示控件：上/右/下箭头 + 气泡框
    class CArrowBubbleUI : public FYUI::CControlUI
    {
    public:
        CArrowBubbleUI() = default;
        std::wstring_view GetClass() const override { return L"ArrowBubble"; }
        bool DoPaint(FYUI::CPaintRenderContext& renderContext, FYUI::CControlUI* pStopControl) override;
    };
}
