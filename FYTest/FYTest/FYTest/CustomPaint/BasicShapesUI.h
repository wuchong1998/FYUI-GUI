#pragma once
#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    // 基础图形展示控件：矩形、圆角矩形、椭圆、虚线框、渐变条
    class CBasicShapesUI : public FYUI::CControlUI
    {
    public:
        CBasicShapesUI() = default;
        std::wstring_view GetClass() const override { return L"BasicShapes"; }
        bool DoPaint(FYUI::CPaintRenderContext& renderContext, FYUI::CControlUI* pStopControl) override;
    };
}
