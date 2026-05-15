#pragma once
#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    // 扇形与仪表盘展示控件：饼图扇形、环形进度、十字准星、同心圆
    class CPieGaugeUI : public FYUI::CControlUI
    {
    public:
        CPieGaugeUI() = default;
        std::wstring_view GetClass() const override { return L"PieGauge"; }
        bool DoPaint(FYUI::CPaintRenderContext& renderContext, FYUI::CControlUI* pStopControl) override;
    };
}
