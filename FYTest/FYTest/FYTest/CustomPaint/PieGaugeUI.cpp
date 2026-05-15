#include "PieGaugeUI.h"
using namespace FYUI;

namespace FYTestApp
{
    bool CPieGaugeUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
    {
        if (!CControlUI::DoPaint(renderContext, pStopControl)) return false;

        auto S = [this](int val) { return ScaleValue(val); };

        const RECT& rc = m_rcItem;
        const int x = rc.left + S(15);
        const int y = rc.top + S(50);

        // ── 标题 ──
        RECT rcTitle = { rc.left + S(20), rc.top + S(15), rc.right - S(20), rc.top + S(45) };
        CRenderEngine::DrawText(renderContext, rcTitle, L"扇形与仪表：饼图 / 环形进度 / 雷达网格 / 十字准星 / 同心圆",
            0xFF1E293B, 0, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

        int gapX = S(115);
        int gapY = S(135);

        // 18) 饼图（多扇区组合，带留白间隙）
        {
            int r = S(45);
            RECT rcPie = { x, y, x + r * 2, y + r * 2 };
            RECT rcShadow = { rcPie.left + S(4), rcPie.top + S(4), rcPie.right + S(4), rcPie.bottom + S(4) };
            CRenderEngine::FillPie(renderContext, rcShadow, 0.0f, 360.0f, 0x1A000000);
            
            // 扇区 1：0~120度 (绿色)
            CRenderEngine::FillPie(renderContext, rcPie, 2.0f, 116.0f, 0xFF10B981);
            // 扇区 2：120~210度 (蓝色)
            CRenderEngine::FillPie(renderContext, rcPie, 122.0f, 86.0f, 0xFF3B82F6);
            // 扇区 3：210~360度 (橙色)
            CRenderEngine::FillPie(renderContext, rcPie, 212.0f, 146.0f, 0xFFF59E0B);

            RECT rcL = { x - S(10), y + r * 2 + S(10), x + r * 2 + S(10), y + r * 2 + S(36) };
            CRenderEngine::DrawText(renderContext, rcL, L"大饼图 (间隔)", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        // 19) 环形进度
        {
            int bx = x + gapX;
            int r = S(45);
            RECT rcArc = { bx, y, bx + r * 2, y + r * 2 };
            RECT rcArcShadow = { rcArc.left + S(4), rcArc.top + S(4), rcArc.right + S(4), rcArc.bottom + S(4) };
            
            // 底环
            CRenderEngine::DrawArc(renderContext, rcArcShadow, 0.0f, 360.0f, S(10), 0x15000000);
            CRenderEngine::DrawArc(renderContext, rcArc, 0.0f, 360.0f, S(10), 0xFFE0E7FF); // indigo-100
            // 进度环 (70%)
            CRenderEngine::DrawArc(renderContext, rcArc, -90.0f, 252.0f, S(10), 0xFF6366F1); // indigo-500

            // 中心文字
            RECT rcTxt = { bx, y + r - S(15), bx + r * 2, y + r + S(15) };
            CRenderEngine::DrawText(renderContext, rcTxt, L"70%", 0xFF1E293B, 1,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

            RECT rcL = { bx - S(10), y + r * 2 + S(10), bx + r * 2 + S(10), y + r * 2 + S(36) };
            CRenderEngine::DrawText(renderContext, rcL, L"粗环形进度", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        // 新增) 雷达网格图
        {
            int bx = x + gapX * 2;
            int r = S(45);
            int cx = bx + r, cy = y + r;
            
            // 绘制3个多边形网格
            for(int step = 3; step >= 1; --step) {
                int curR = r * step / 3;
                POINT poly[5];
                for(int i=0; i<5; ++i) {
                    float angle = -3.14159f/2.0f + 3.14159f*2.0f/5.0f * i;
                    poly[i].x = cx + (int)(std::cos(angle)*curR);
                    poly[i].y = cy + (int)(std::sin(angle)*curR);
                }
                CRenderEngine::DrawPolygon(renderContext, poly, 5, S(1), 0xFFCBD5E1); // slate-300
            }
            // 绘制辐条
            for(int i=0; i<5; ++i) {
                float angle = -3.14159f/2.0f + 3.14159f*2.0f/5.0f * i;
                POINT pts[2];
                pts[0] = {cx, cy};
                pts[1] = {cx + (int)(std::cos(angle)*r), cy + (int)(std::sin(angle)*r)};
                CRenderEngine::DrawPolyline(renderContext, pts, 2, S(1), 0xFFCBD5E1);
            }
            // 绘制数据区
            POINT dataPoly[5];
            float data[5] = { 0.9f, 0.6f, 0.8f, 0.4f, 0.7f };
            for(int i=0; i<5; ++i) {
                float angle = -3.14159f/2.0f + 3.14159f*2.0f/5.0f * i;
                int curR = (int)(r * data[i]);
                dataPoly[i].x = cx + (int)(std::cos(angle)*curR);
                dataPoly[i].y = cy + (int)(std::sin(angle)*curR);
            }
            CRenderEngine::FillPolygon(renderContext, dataPoly, 5, 0x44F43F5E); // rose-500 alpha
            CRenderEngine::DrawPolygon(renderContext, dataPoly, 5, S(2), 0xFFF43F5E);

            RECT rcL = { bx - S(10), y + r * 2 + S(10), bx + r * 2 + S(10), y + r * 2 + S(36) };
            CRenderEngine::DrawText(renderContext, rcL, L"雷达数据图", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        // 20) 十字准星
        {
            int y2 = y + gapY;
            int r = S(40);
            int cx = x + r, cy = y2 + r;

            // 绘制泛红光的准星外圈
            RECT rcTarget = { x, y2, x + r * 2, y2 + r * 2 };
            CRenderEngine::DrawEllipse(renderContext, rcTarget, S(2), 0xFFEF4444);
            RECT rcTargetHalo = { rcTarget.left - S(3), rcTarget.top - S(3), rcTarget.right + S(3), rcTarget.bottom + S(3) };
            CRenderEngine::DrawEllipse(renderContext, rcTargetHalo, S(4), 0x33EF4444);

            RECT hRect = { x - S(8), cy, x + r * 2 + S(8), cy };
            CRenderEngine::DrawLine(renderContext, hRect, S(2), 0xFFEF4444);
            RECT vRect = { cx, y2 - S(8), cx, y2 + r * 2 + S(8) };
            CRenderEngine::DrawLine(renderContext, vRect, S(2), 0xFFEF4444);
            
            // 中心红点
            RECT rcCenter = { cx - S(4), cy - S(4), cx + S(4), cy + S(4) };
            CRenderEngine::FillEllipse(renderContext, rcCenter, 0xFFEF4444);

            RECT rcL = { x - S(20), y2 + r * 2 + S(10), x + r * 2 + S(20), y2 + r * 2 + S(36) };
            CRenderEngine::DrawText(renderContext, rcL, L"大尺寸发光准星", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        // 21) 同心多圈圆
        {
            int y2 = y + gapY;
            int bx = x + gapX;
            int r = S(45);
            int cx2 = bx + r, cy2 = y2 + r;

            RECT c4 = { cx2 - S(45), cy2 - S(45), cx2 + S(45), cy2 + S(45) };
            CRenderEngine::DrawEllipse(renderContext, c4, S(4), 0xFF3B82F6);
            RECT c3 = { cx2 - S(32), cy2 - S(32), cx2 + S(32), cy2 + S(32) };
            CRenderEngine::DrawEllipse(renderContext, c3, S(4), 0xFF10B981);
            RECT c2 = { cx2 - S(19), cy2 - S(19), cx2 + S(19), cy2 + S(19) };
            CRenderEngine::DrawEllipse(renderContext, c2, S(4), 0xFFF59E0B);
            RECT c1 = { cx2 - S(6), cy2 - S(6), cx2 + S(6), cy2 + S(6) };
            CRenderEngine::DrawEllipse(renderContext, c1, S(4), 0xFF8B5CF6);

            RECT rcL = { bx - S(20), y2 + r * 2 + S(10), bx + r * 2 + S(20), y2 + r * 2 + S(36) };
            CRenderEngine::DrawText(renderContext, rcL, L"粗同心多色圆环", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        return true;
    }
}
