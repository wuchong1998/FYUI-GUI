#include "ArrowBubbleUI.h"
using namespace FYUI;

namespace FYTestApp
{
    bool CArrowBubbleUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
    {
        if (!CControlUI::DoPaint(renderContext, pStopControl)) return false;

        auto S = [this](int val) { return ScaleValue(val); };

        const RECT& rc = m_rcItem;
        const int x = rc.left + S(15);
        const int y = rc.top + S(50);

        // ── 标题 ──
        RECT rcTitle = { rc.left + S(20), rc.top + S(15), rc.right - S(20), rc.top + S(45) };
        CRenderEngine::DrawText(renderContext, rcTitle, L"箭头与气泡：上箭头 / 右箭头 / 下箭头 / 现代气泡框",
            0xFF1E293B, 0, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

        int gapX = S(115);
        int gapY = S(105);

        // 10) 上箭头
        {
            int ax = x + S(30), ay = y;
            int aw = S(35), ah = S(50);
            POINT arrow[7] = {
                { ax, ay },
                { ax + aw/2, ay + ah/2 },
                { ax + aw/4, ay + ah/2 },
                { ax + aw/4, ay + ah },
                { ax - aw/4, ay + ah },
                { ax - aw/4, ay + ah/2 },
                { ax - aw/2, ay + ah/2 }
            };
            POINT shadow[7];
            for (int i = 0; i < 7; ++i) shadow[i] = { arrow[i].x + S(5), arrow[i].y + S(5) };
            CRenderEngine::FillPolygon(renderContext, shadow, 7, 0x15000000);
            CRenderEngine::FillPolygon(renderContext, arrow, 7, 0xFF60A5FA);
            CRenderEngine::DrawPolygon(renderContext, arrow, 7, S(3), 0xFF2563EB);
            RECT rcL = { ax - S(50), ay + ah + S(10), ax + S(50), ay + ah + S(36) };
            CRenderEngine::DrawText(renderContext, rcL, L"巨型上箭头", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        // 11) 右箭头
        {
            int ax = x + gapX + S(10), ay = y + S(10);
            int aw = S(45), ah = S(35);
            POINT arrow[7] = {
                { ax + aw, ay },
                { ax + aw/2, ay + ah/2 },
                { ax + aw/2, ay + ah/4 },
                { ax,        ay + ah/4 },
                { ax,        ay - ah/4 },
                { ax + aw/2, ay - ah/4 },
                { ax + aw/2, ay - ah/2 }
            };
            POINT shadow[7];
            for (int i = 0; i < 7; ++i) shadow[i] = { arrow[i].x + S(5), arrow[i].y + S(5) };
            CRenderEngine::FillPolygon(renderContext, shadow, 7, 0x15000000);
            CRenderEngine::FillPolygon(renderContext, arrow, 7, 0xFF34D399);
            CRenderEngine::DrawPolygon(renderContext, arrow, 7, S(3), 0xFF059669);
            RECT rcL = { ax, ay + ah/2 + S(10), ax + aw, ay + ah/2 + S(36) };
            CRenderEngine::DrawText(renderContext, rcL, L"胖右箭头", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        // 12) 下箭头
        {
            int ax = x + gapX*2 + S(30), ay = y;
            int aw = S(35), ah = S(50);
            POINT arrow[7] = {
                { ax - aw/4, ay },
                { ax + aw/4, ay },
                { ax + aw/4, ay + ah/2 },
                { ax + aw/2, ay + ah/2 },
                { ax,        ay + ah },
                { ax - aw/2, ay + ah/2 },
                { ax - aw/4, ay + ah/2 }
            };
            POINT shadow[7];
            for (int i = 0; i < 7; ++i) shadow[i] = { arrow[i].x + S(5), arrow[i].y + S(5) };
            CRenderEngine::FillPolygon(renderContext, shadow, 7, 0x15000000);
            CRenderEngine::FillPolygon(renderContext, arrow, 7, 0xFFA78BFA);
            CRenderEngine::DrawPolygon(renderContext, arrow, 7, S(3), 0xFF7C3AED);
            RECT rcL = { ax - S(50), ay + ah + S(10), ax + S(50), ay + ah + S(36) };
            CRenderEngine::DrawText(renderContext, rcL, L"宽下箭头", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        // 13) 现代气泡框
        {
            int y2 = y + gapY;
            int bx = x;
            int bw = S(280), bh = S(85);

            // 气泡阴影
            RECT rcBubbleShadow = { bx + S(6), y2 + S(6), bx + bw + S(6), y2 + bh + S(6) };
            CRenderEngine::DrawRoundColor(renderContext, rcBubbleShadow, S(24), S(24), 0x15000000);
            POINT tipShadow[3] = {
                { bx + S(40), y2 + bh + S(6) },
                { bx + S(70), y2 + bh + S(6) },
                { bx + S(50), y2 + bh + S(26) }
            };
            CRenderEngine::FillPolygon(renderContext, tipShadow, 3, 0x15000000);

            // 圆角矩形主体
            RECT rcBubble = { bx, y2, bx + bw, y2 + bh };
            CRenderEngine::DrawRoundColor(renderContext, rcBubble, S(24), S(24), 0xFFE0E7FF);

            // 底部三角尖角
            POINT tip[3] = {
                { bx + S(40), y2 + bh },
                { bx + S(70), y2 + bh },
                { bx + S(50), y2 + bh + S(20) }
            };
            CRenderEngine::FillPolygon(renderContext, tip, 3, 0xFFE0E7FF);

            // 气泡内文字
            RECT rcTxt = { bx + S(20), y2 + S(15), bx + bw - S(20), y2 + bh - S(10) };
            CRenderEngine::DrawText(renderContext, rcTxt, L"您好！这是一条放大并适配了 DPI 的现代气泡消息，多行文字也不会再被截断了哦～",
                0xFF334155, 0, DT_WORDBREAK | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

            RECT rcL = { bx, y2 + bh + S(26), bx + bw, y2 + bh + S(52) };
            CRenderEngine::DrawText(renderContext, rcL, L"宽体现代卡片气泡", 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        return true;
    }
}
