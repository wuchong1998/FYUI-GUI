#include "BasicShapesUI.h"
using namespace FYUI;

namespace FYTestApp
{
    bool CBasicShapesUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
    {
        if (!CControlUI::DoPaint(renderContext, pStopControl)) return false;

        auto S = [this](int val) { return ScaleValue(val); };

        const RECT& rc = m_rcItem;
        const int x = rc.left + S(15);
        const int y = rc.top + S(50);

        // ── 标题 ──
        RECT rcTitle = { rc.left + S(20), rc.top + S(15), rc.right - S(20), rc.top + S(45) };
        CRenderEngine::DrawText(renderContext, rcTitle, L"基础图形：矩形 / 圆角矩形 / 椭圆 / 虚线框 / 柔和渐变",
            0xFF1E293B, 0, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

        int w = S(95);
        int h = S(65);
        int gapX = S(115);
        int gapY = S(110);

        // 1) 填充矩形
        RECT rc1 = { x, y, x + w, y + h };
        RECT rc1Shadow = { rc1.left + S(6), rc1.top + S(6), rc1.right + S(6), rc1.bottom + S(6) };
        CRenderEngine::DrawColor(renderContext, rc1Shadow, 0x15000000);
        CRenderEngine::DrawColor(renderContext, rc1, 0xFF3B82F6);
        RECT rcLabel1 = { rc1.left, rc1.bottom + S(10), rc1.right, rc1.bottom + S(36) };
        CRenderEngine::DrawText(renderContext, rcLabel1, L"明快填充矩形", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // 2) 超大圆角矩形（填充+描边）
        RECT rc2 = { x + gapX, y, x + gapX + w, y + h };
        RECT rc2Shadow = { rc2.left + S(6), rc2.top + S(6), rc2.right + S(6), rc2.bottom + S(6) };
        int rRound = S(24);
        CRenderEngine::DrawRoundColor(renderContext, rc2Shadow, rRound, rRound, 0x15000000);
        CRenderEngine::DrawRoundColor(renderContext, rc2, rRound, rRound, 0xFFD1FAE5);
        CRenderEngine::DrawRoundRect(renderContext, rc2, S(3), rRound, rRound, 0xFF10B981);
        RECT rcLabel2 = { rc2.left, rc2.bottom + S(10), rc2.right, rc2.bottom + S(36) };
        CRenderEngine::DrawText(renderContext, rcLabel2, L"大圆角矩形", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // 3) 椭圆（填充+描边）
        RECT rc3 = { x + gapX * 2, y, x + gapX * 2 + w, y + h };
        RECT rc3Shadow = { rc3.left + S(6), rc3.top + S(6), rc3.right + S(6), rc3.bottom + S(6) };
        CRenderEngine::FillEllipse(renderContext, rc3Shadow, 0x15000000);
        CRenderEngine::FillEllipse(renderContext, rc3, 0xFFEDE9FE);
        CRenderEngine::DrawEllipse(renderContext, rc3, S(3), 0xFF8B5CF6);
        RECT rcLabel3 = { rc3.left, rc3.bottom + S(10), rc3.right, rc3.bottom + S(36) };
        CRenderEngine::DrawText(renderContext, rcLabel3, L"精致椭圆", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // 4) 虚线矩形边框
        int y2 = y + gapY;
        RECT rc4 = { x, y2, x + w, y2 + h };
        CRenderEngine::DrawRect(renderContext, rc4, S(2), 0xFFF59E0B, PS_DASH);
        RECT rcLabel4 = { rc4.left, rc4.bottom + S(10), rc4.right, rc4.bottom + S(36) };
        CRenderEngine::DrawText(renderContext, rcLabel4, L"粗虚线框", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        // 5) 长渐变条 (带圆角和横向长发光感)
        RECT rc5 = { x + gapX, y2, x + gapX * 2 + w, y2 + h };
        RECT rc5Shadow = { rc5.left, rc5.top + S(8), rc5.right, rc5.bottom + S(8) };
        CRenderEngine::DrawRoundColor(renderContext, rc5Shadow, rRound, rRound, 0x10000000);
        CRenderEngine::DrawGradient(renderContext, rc5, 0xFF60A5FA, 0xFFF472B6, true, 256); // Blue to Pink horizontal
        CRenderEngine::DrawRoundRect(renderContext, rc5, S(2), rRound, rRound, 0x33FFFFFF);
        RECT rcLabel5 = { rc5.left, rc5.bottom + S(10), rc5.right, rc5.bottom + S(36) };
        CRenderEngine::DrawText(renderContext, rcLabel5, L"柔和横向双色长渐变条", 0xFF64748B, 0,
            DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

        return true;
    }
}
