#include "PolygonShapesUI.h"
#include <cmath>
using namespace FYUI;

namespace FYTestApp
{
    static constexpr float kPi = 3.14159265358979323846f;

    void CPolygonShapesUI::DrawStar(CPaintRenderContext& ctx, int cx, int cy, int outerR, int innerR, DWORD fillColor, DWORD strokeColor, int shadowOffset, int lineWidth)
    {
        POINT pts[10];
        POINT shadowPts[10];
        for (int i = 0; i < 10; ++i) {
            float angle = -kPi / 2.0f + kPi / 5.0f * i;
            int r = (i % 2 == 0) ? outerR : innerR;
            pts[i].x = cx + static_cast<LONG>(std::cos(angle) * r);
            pts[i].y = cy + static_cast<LONG>(std::sin(angle) * r);
            shadowPts[i].x = pts[i].x + shadowOffset;
            shadowPts[i].y = pts[i].y + shadowOffset;
        }
        CRenderEngine::FillPolygon(ctx, shadowPts, 10, 0x15000000);
        CRenderEngine::FillPolygon(ctx, pts, 10, fillColor);
        CRenderEngine::DrawPolygon(ctx, pts, 10, lineWidth, strokeColor);
    }

    bool CPolygonShapesUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
    {
        if (!CControlUI::DoPaint(renderContext, pStopControl)) return false;

        auto S = [this](int val) { return ScaleValue(val); };

        const RECT& rc = m_rcItem;
        const int x = rc.left + S(15);
        const int y = rc.top + S(50);

        // ── 标题 ──
        RECT rcTitle = { rc.left + S(20), rc.top + S(15), rc.right - S(20), rc.top + S(45) };
        CRenderEngine::DrawText(renderContext, rcTitle, L"多边形：五角星 / 三角形 / 六边形 / 菱形 / 八边形",
            0xFF1E293B, 0, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

        int gapX = S(115);
        int gapY = S(115);

        // 6) 五角星
        int cx1 = x + S(45), cy1 = y + S(35);
        DrawStar(renderContext, cx1, cy1, S(35), S(15), 0xFFFDE047, 0xFFEAB308, S(6), S(3));
        RECT rcL6 = { x - S(20), cy1 + S(45), x + S(110), cy1 + S(70) };
        CRenderEngine::DrawText(renderContext, rcL6, L"大五角星", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // 7) 等边三角形
        int cx2 = x + gapX + S(45), cy2 = y + S(5);
        int triR = S(35);
        POINT tri[3] = {
            { cx2, cy2 },
            { cx2 - static_cast<LONG>(triR * 0.866f), cy2 + static_cast<LONG>(triR * 1.5f) },
            { cx2 + static_cast<LONG>(triR * 0.866f), cy2 + static_cast<LONG>(triR * 1.5f) }
        };
        POINT triShadow[3];
        for (int i = 0; i < 3; ++i) { triShadow[i] = { tri[i].x + S(6), tri[i].y + S(6) }; }
        CRenderEngine::FillPolygon(renderContext, triShadow, 3, 0x15000000);
        CRenderEngine::FillPolygon(renderContext, tri, 3, 0xFF6EE7B7);
        CRenderEngine::DrawPolygon(renderContext, tri, 3, S(3), 0xFF059669);
        RECT rcL7 = { cx2 - S(50), cy1 + S(45), cx2 + S(50), cy1 + S(70) };
        CRenderEngine::DrawText(renderContext, rcL7, L"等边三角形", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // 8) 正六边形
        int cx3 = x + gapX * 2 + S(45), cy3 = y + S(35);
        int hexR = S(35);
        POINT hex[6];
        POINT hexShadow[6];
        for (int i = 0; i < 6; ++i) {
            float angle = kPi / 6.0f + kPi / 3.0f * i;
            hex[i].x = cx3 + static_cast<LONG>(std::cos(angle) * hexR);
            hex[i].y = cy3 + static_cast<LONG>(std::sin(angle) * hexR);
            hexShadow[i] = { hex[i].x + S(6), hex[i].y + S(6) };
        }
        CRenderEngine::FillPolygon(renderContext, hexShadow, 6, 0x15000000);
        CRenderEngine::FillPolygon(renderContext, hex, 6, 0xFFBFDBFE);
        CRenderEngine::DrawPolygon(renderContext, hex, 6, S(3), 0xFF2563EB);
        RECT rcL8 = { cx3 - S(50), cy1 + S(45), cx3 + S(50), cy1 + S(70) };
        CRenderEngine::DrawText(renderContext, rcL8, L"正六边形", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // 9) 菱形
        int y2 = y + gapY;
        int cx4 = x + S(45), cy4 = y2 + S(35);
        int diaWx = S(45), diaWy = S(35);
        POINT diamond[4] = {
            { cx4, cy4 - diaWy },
            { cx4 + diaWx, cy4 },
            { cx4, cy4 + diaWy },
            { cx4 - diaWx, cy4 }
        };
        POINT diamondShadow[4];
        for (int i = 0; i < 4; ++i) { diamondShadow[i] = { diamond[i].x + S(6), diamond[i].y + S(6) }; }
        CRenderEngine::FillPolygon(renderContext, diamondShadow, 4, 0x15000000);
        CRenderEngine::FillPolygon(renderContext, diamond, 4, 0xFFFCA5A5);
        CRenderEngine::DrawPolygon(renderContext, diamond, 4, S(3), 0xFFDC2626);
        RECT rcL9 = { cx4 - S(50), cy4 + S(45), cx4 + S(50), cy4 + S(70) };
        CRenderEngine::DrawText(renderContext, rcL9, L"宽体菱形", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // 10) 新增：正八边形
        int cx5 = x + gapX + S(45), cy5 = y2 + S(35);
        int octR = S(35);
        POINT oct[8];
        POINT octShadow[8];
        for (int i = 0; i < 8; ++i) {
            float angle = kPi / 8.0f + kPi / 4.0f * i;
            oct[i].x = cx5 + static_cast<LONG>(std::cos(angle) * octR);
            oct[i].y = cy5 + static_cast<LONG>(std::sin(angle) * octR);
            octShadow[i] = { oct[i].x + S(6), oct[i].y + S(6) };
        }
        CRenderEngine::FillPolygon(renderContext, octShadow, 8, 0x15000000);
        CRenderEngine::FillPolygon(renderContext, oct, 8, 0xFFD8B4FE); // purple-300
        CRenderEngine::DrawPolygon(renderContext, oct, 8, S(3), 0xFF9333EA); // purple-600
        RECT rcL10 = { cx5 - S(50), cy5 + S(45), cx5 + S(50), cy5 + S(70) };
        CRenderEngine::DrawText(renderContext, rcL10, L"正八边形", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        return true;
    }
}
