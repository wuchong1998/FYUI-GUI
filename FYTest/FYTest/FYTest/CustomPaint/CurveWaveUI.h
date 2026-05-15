#pragma once
#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    // 曲线与波浪展示控件：贝塞尔S曲线、正弦波浪线、折线图、圆弧
    class CCurveWaveUI : public FYUI::CControlUI
    {
    public:
        CCurveWaveUI() = default;
        std::wstring_view GetClass() const override { return L"CurveWave"; }
        bool DoPaint(FYUI::CPaintRenderContext& renderContext, FYUI::CControlUI* pStopControl) override;
    };
}
