#pragma once
#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    // 多边形与星形展示控件：五角星、三角形、六边形、菱形
    class CPolygonShapesUI : public FYUI::CControlUI
    {
    public:
        CPolygonShapesUI() = default;
        std::wstring_view GetClass() const override { return L"PolygonShapes"; }
        bool DoPaint(FYUI::CPaintRenderContext& renderContext, FYUI::CControlUI* pStopControl) override;
    private:
        void DrawStar(FYUI::CPaintRenderContext& ctx, int cx, int cy, int outerR, int innerR, DWORD fillColor, DWORD strokeColor, int shadowOffset, int lineWidth);
    };
}
