#include "CurveWaveUI.h"
#include <cmath>
using namespace FYUI;

namespace FYTestApp {
static constexpr float kPi = 3.14159265358979323846f;

    bool CCurveWaveUI::DoPaint(CPaintRenderContext &renderContext,
                               CControlUI *pStopControl) {
      if (!CControlUI::DoPaint(renderContext, pStopControl))
        return false;

      auto S = [this](int val) { return ScaleValue(val); };

      const RECT &rc = m_rcItem;
      const int x = rc.left + S(15);
      const int y = rc.top + S(50);

      // ── 标题 ──
      RECT rcTitle = {rc.left + S(20), rc.top + S(15), rc.right - S(20), rc.top + S(45)};
      CRenderEngine::DrawText(
          renderContext, rcTitle, L"曲线：平滑贝塞尔 / 多重海浪 / 面积折线图 / 圆弧",
          0xFF1E293B, 0, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

      int gapX = S(170);
      int gapY = S(135);

      // 14) S 形贝塞尔曲线
      {
        POINT p0 = {x, y + S(50)};
        POINT c1 = {x + S(40), y};
        POINT c2 = {x + S(80), y + S(80)};
        POINT p3 = {x + S(130), y + S(20)};
        POINT p0s = {p0.x + S(3), p0.y + S(3)}, c1s = {c1.x + S(3), c1.y + S(3)},
              c2s = {c2.x + S(3), c2.y + S(3)}, p3s = {p3.x + S(3), p3.y + S(3)};
        CRenderEngine::DrawBezier(renderContext, p0s, c1s, c2s, p3s, S(5), 0x1A000000);
        CRenderEngine::DrawBezier(renderContext, p0, c1, c2, p3, S(5), 0xFFF59E0B);
        RECT rcL = {x, y + S(90), x + S(130), y + S(116)};
        CRenderEngine::DrawText(renderContext, rcL, L"加粗平滑贝塞尔", 0xFF64748B, 0,
                                DT_SINGLELINE | DT_CENTER | DT_VCENTER |
                                    DT_NOPREFIX);
      }

      // 15) 多重波浪线（模拟海浪交织）
      {
        int wx = x + gapX;
        const int segments = 3;
        const int segW = S(50);
        const int amp = S(25);
        for(int waveIdx = 0; waveIdx < 3; ++waveIdx) {
            int waveOffsetY = waveIdx * S(15);
            DWORD waveColors[3] = { 0x4460A5FA, 0x883B82F6, 0xFF2563EB };
            for (int i = 0; i < segments; ++i) {
              int sx = wx + i * segW;
              POINT p0 = {sx, y + S(50) + waveOffsetY};
              POINT c1 = {sx + segW / 3, y + S(50) + waveOffsetY - ((i % 2 == 0) ? amp : -amp)};
              POINT c2 = {sx + segW * 2 / 3, y + S(50) + waveOffsetY + ((i % 2 == 0) ? amp : -amp)};
              POINT p3 = {sx + segW, y + S(50) + waveOffsetY};
              CRenderEngine::DrawBezier(renderContext, p0, c1, c2, p3, S(3), waveColors[waveIdx]);
            }
        }
        RECT rcL = {wx, y + S(105), wx + segW * segments, y + S(131)};
        CRenderEngine::DrawText(renderContext, rcL, L"多重海浪波浪", 0xFF64748B, 0,
                                DT_SINGLELINE | DT_CENTER | DT_VCENTER |
                                    DT_NOPREFIX);
      }

      // 16) 折线图
      {
        const int y2 = y + gapY;
        POINT pts[8] = {{x, y2 + S(50)},       {x + S(35), y2 + S(25)},  {x + S(70), y2 + S(40)},
                        {x + S(105), y2 + S(10)},  {x + S(140), y2 + S(30)}, {x + S(175), y2 + S(10)},
                        {x + S(210), y2 + S(25)}, {x + S(245), y2 + S(5)}};
        // 绘制底下面积填充
        POINT fillPts[10];
        for (int i = 0; i < 8; ++i)
          fillPts[i] = pts[i];
        fillPts[8] = {pts[7].x, y2 + S(70)};
        fillPts[9] = {pts[0].x, y2 + S(70)};
        CRenderEngine::FillPolygon(renderContext, fillPts, 10, 0x3310B981);

        CRenderEngine::DrawPolyline(renderContext, pts, 8, S(4), 0xFF10B981);
        // 数据点圆点（带白色内圈）
        for (int i = 0; i < 8; ++i) {
          RECT dotOuter = {pts[i].x - S(6), pts[i].y - S(6), pts[i].x + S(6), pts[i].y + S(6)};
          CRenderEngine::FillEllipse(renderContext, dotOuter, 0xFF10B981);
          RECT dotInner = {pts[i].x - S(3), pts[i].y - S(3), pts[i].x + S(3), pts[i].y + S(3)};
          CRenderEngine::FillEllipse(renderContext, dotInner, 0xFFFFFFFF);
        }
        RECT rcL = {x, y2 + S(80), x + S(245), y2 + S(106)};
        CRenderEngine::DrawText(renderContext, rcL, L"大尺度面积折线图", 0xFF64748B, 0,
                                DT_SINGLELINE | DT_CENTER | DT_VCENTER |
                                    DT_NOPREFIX);
      }

      // 17) 圆弧
      {
        const int y2 = y + gapY;
        int arcCx = x + gapX + S(100);
        RECT rcArc = {arcCx - S(50), y2 + S(10), arcCx + S(50), y2 + S(60)};
        RECT rcArcShadow = {rcArc.left + S(3), rcArc.top + S(3), rcArc.right + S(3), rcArc.bottom + S(3)};
        CRenderEngine::DrawArc(renderContext, rcArcShadow, -30.0f, 240.0f, S(6),
                               0x1A000000);
        CRenderEngine::DrawArc(renderContext, rcArc, -30.0f, 240.0f, S(6), 0xFF8B5CF6);
        RECT rcL = {rcArc.left, y2 + S(80), rcArc.right, y2 + S(106)};
        CRenderEngine::DrawText(renderContext, rcL, L"粗圆弧", 0xFF64748B, 0,
                                DT_SINGLELINE | DT_CENTER | DT_VCENTER |
                                    DT_NOPREFIX);
      }

      return true;
    }
} // namespace FYTestApp
