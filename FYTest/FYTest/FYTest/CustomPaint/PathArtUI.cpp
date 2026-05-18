#include "PathArtUI.h"
#include <cmath>
using namespace FYUI;

namespace FYTestApp
{
    namespace
    {
        constexpr float kPi = 3.14159265358979323846f;
        constexpr float kDeg2Rad = kPi / 180.0f;

        // 极坐标转直角坐标（围绕 center）
        inline FYUI::POINTF Polar(FYUI::POINTF center, float radius, float angleDeg)
        {
            const float a = angleDeg * kDeg2Rad;
            return FYUI::POINTF{ center.X + radius * ::cosf(a),
                           center.Y + radius * ::sinf(a) };
        }

        // 在指定矩形顶部画 panel 小标题，返回标题底部 Y
        int DrawPanelTitle(CPaintRenderContext& ctx, const RECT& rcPanel,
                           const wchar_t* text, int subTitleH)
        {
            RECT rcTitle = { rcPanel.left + 8, rcPanel.top + 4,
                             rcPanel.right - 8, rcPanel.top + 4 + subTitleH };
            CRenderEngine::DrawText(ctx, rcTitle, text, 0xFF334155, 0,
                                    DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);
            return rcTitle.bottom;
        }

        // ---------- A：五瓣花 ----------
        void DrawPetalFlower(CPaintRenderContext& ctx, const RECT& rcPanel,
                             float scale)
        {
            const float cx = (rcPanel.left + rcPanel.right) * 0.5f;
            const float cy = (rcPanel.top + rcPanel.bottom) * 0.5f + 6.0f * scale;
            const FYUI::POINTF center{ cx, cy };
            const float petalLen = 52.0f * scale;     // 控制点距离 → 花瓣大小
            const float halfAng = 28.0f;              // 每瓣张角的一半（度）

            CRenderPath flower;
            flower.Begin();
            for (int i = 0; i < 5; ++i) {
                const float ang = -90.0f + i * 72.0f; // 顶部第一瓣，5 瓣均分
                const FYUI::POINTF c1 = Polar(center, petalLen, ang - halfAng);
                const FYUI::POINTF c2 = Polar(center, petalLen, ang + halfAng);
                flower.MoveTo(center);
                flower.BezierTo(c1, c2, center);      // 单段三次贝塞尔形成花瓣
                flower.Close();
            }
            flower.End();

            // 阴影：把整朵花向右下偏移 3px 用半透明黑描边一次
            CRenderEngine::StrokePath(ctx, flower, 6.0f * scale, 0x18000000);
            CRenderEngine::FillPath(ctx, flower, 0xFFFBCFE8);   // 粉色填充
            CRenderEngine::StrokePath(ctx, flower, 2.0f * scale, 0xFFDB2777);

            // 花蕊：填充小圆
            const float cr = 8.0f * scale;
            RECT rcCore = { static_cast<LONG>(cx - cr), static_cast<LONG>(cy - cr),
                            static_cast<LONG>(cx + cr), static_cast<LONG>(cy + cr) };
            CRenderEngine::FillEllipse(ctx, rcCore, 0xFFFACC15);
            CRenderEngine::DrawEllipse(ctx, rcCore, 1.0f * scale, 0xFFB45309);
        }

        // ---------- B：彩虹风车（PushTransform Rotation 复制） ----------
        void DrawRotatedPinwheel(CPaintRenderContext& ctx, const RECT& rcPanel,
                                 float scale)
        {
            const float cx = (rcPanel.left + rcPanel.right) * 0.5f;
            const float cy = (rcPanel.top + rcPanel.bottom) * 0.5f + 6.0f * scale;
            const FYUI::POINTF center{ cx, cy };
            const float bladeLen = 56.0f * scale;
            const float bladeFat = 24.0f * scale;

            // 一片叶子：从中心出发 → 右尖 → 内凹曲线回中心
            CRenderPath blade;
            blade.Begin();
            blade.MoveTo(center);
            blade.LineTo(FYUI::POINTF{ cx + bladeLen, cy });
            blade.BezierTo(
                FYUI::POINTF{ cx + bladeLen * 0.85f, cy + bladeFat * 0.85f },
                FYUI::POINTF{ cx + bladeLen * 0.30f, cy + bladeFat },
                center);
            blade.Close();
            blade.End();

            // 12 个彩虹色相
            const DWORD colors[12] = {
                0xFFEF4444, 0xFFF97316, 0xFFF59E0B, 0xFFEAB308,
                0xFF84CC16, 0xFF22C55E, 0xFF10B981, 0xFF06B6D4,
                0xFF3B82F6, 0xFF6366F1, 0xFF8B5CF6, 0xFFEC4899,
            };

            for (int i = 0; i < 12; ++i) {
                CRenderEngine::Matrix2D m = CRenderEngine::Matrix2D::Rotation(
                    i * 30.0f, center);
                CRenderEngine::PushTransform(ctx, m);
                CRenderEngine::FillPath(ctx, blade, colors[i]);
                CRenderEngine::StrokePath(ctx, blade, 1.0f * scale, 0x55000000);
                CRenderEngine::PopTransform(ctx);
            }

            // 中心轴
            const float hr = 10.0f * scale;
            RECT rcHub = { static_cast<LONG>(cx - hr), static_cast<LONG>(cy - hr),
                           static_cast<LONG>(cx + hr), static_cast<LONG>(cy + hr) };
            CRenderEngine::FillEllipse(ctx, rcHub, 0xFFFFFFFF);
            CRenderEngine::DrawEllipse(ctx, rcHub, 1.5f * scale, 0xFF1F2937);
        }

        // ---------- C：心形裁剪 + 内部彩色条纹 ----------
        void DrawHeartClip(CPaintRenderContext& ctx, const RECT& rcPanel,
                           float scale)
        {
            const float cx = (rcPanel.left + rcPanel.right) * 0.5f;
            const float cy = (rcPanel.top + rcPanel.bottom) * 0.5f + 6.0f * scale;
            const float w = 50.0f * scale;   // 横向半宽
            const float h = 48.0f * scale;   // 纵向延伸
            const FYUI::POINTF top{ cx, cy - h * 0.40f };

            CRenderPath heart;
            heart.Begin();
            heart.MoveTo(top);
            // 左半边：从顶部凹槽到底部尖
            heart.BezierTo(
                FYUI::POINTF{ cx - w * 1.4f, cy - h * 1.1f },
                FYUI::POINTF{ cx - w * 1.4f, cy + h * 0.20f },
                FYUI::POINTF{ cx,            cy + h * 1.05f });
            // 右半边：从底部尖回到顶部凹槽
            heart.BezierTo(
                FYUI::POINTF{ cx + w * 1.4f, cy + h * 0.20f },
                FYUI::POINTF{ cx + w * 1.4f, cy - h * 1.1f },
                top);
            heart.Close();
            heart.End();

            // 1) 将后续绘制裁剪在心形内
            CRenderEngine::PushPathClip(ctx, heart);

            const RECT rcBounds = heart.GetBoundsRect();
            const int stripeH = static_cast<int>(10.0f * scale);
            const DWORD stripes[5] = {
                0xFFFCA5A5, 0xFFFCD34D, 0xFFA7F3D0, 0xFFBFDBFE, 0xFFC4B5FD,
            };
            int idx = 0;
            for (int y = rcBounds.top - stripeH; y < rcBounds.bottom + stripeH;
                 y += stripeH) {
                RECT rcStripe = { rcBounds.left - stripeH, y,
                                  rcBounds.right + stripeH, y + stripeH };
                CRenderEngine::DrawColor(ctx, rcStripe, stripes[idx % 5]);
                ++idx;
            }
            // 在条纹之上盖一层柔和高光
            CRenderEngine::DrawGradient(ctx, rcBounds,
                0x60FFFFFF, 0x00FFFFFF, false, 64);

            CRenderEngine::PopPathClip(ctx);

            // 2) 心形描边
            CRenderEngine::StrokePath(ctx, heart, 2.5f * scale, 0xFFE11D48);
        }

        // ---------- D：复合变换矩阵螺旋星阵 ----------
        void DrawTransformSpiral(CPaintRenderContext& ctx, const RECT& rcPanel,
                                 float scale)
        {
            // 局部坐标下（围绕原点）的五角星
            CRenderPath star;
            star.Begin();
            const float R1 = 18.0f, R2 = 7.0f; // 局部坐标，未缩放
            for (int i = 0; i < 10; ++i) {
                const float a = (-90.0f + i * 36.0f) * kDeg2Rad;
                const float r = (i % 2 == 0) ? R1 : R2;
                FYUI::POINTF p{ r * ::cosf(a), r * ::sinf(a) };
                if (i == 0) star.MoveTo(p);
                else        star.LineTo(p);
            }
            star.Close();
            star.End();

            const float cx = (rcPanel.left + rcPanel.right) * 0.5f;
            const float cy = (rcPanel.top + rcPanel.bottom) * 0.5f + 6.0f * scale;

            const int N = 18;
            for (int i = 0; i < N; ++i) {
                const float t = (N <= 1) ? 0.0f : i / static_cast<float>(N - 1);
                const float angDeg = t * 540.0f;                // 1.5 圈
                const float radius = (12.0f + t * 58.0f) * scale;
                const float s = (1.05f - t * 0.65f) * scale;    // 越外越小
                const FYUI::POINTF pos = Polar(FYUI::POINTF{ cx, cy }, radius, angDeg - 90.0f);

                // 复合（行向量约定）：P' = P * S * R * T
                // 含义：先在原点缩放、再绕原点旋转、最后平移到 pos
                const CRenderEngine::Matrix2D mS = CRenderEngine::Matrix2D::Scale(s, s);
                const CRenderEngine::Matrix2D mR = CRenderEngine::Matrix2D::Rotation(angDeg * 2.0f);
                const CRenderEngine::Matrix2D mT = CRenderEngine::Matrix2D::Translation(pos.X, pos.Y);
                const CRenderEngine::Matrix2D m = mS * mR * mT;

                // 色相按 t 渐变（青→紫→粉）
                const int hueR = static_cast<int>(96 + t * 159);
                const int hueG = static_cast<int>(165 - t * 100);
                const int hueB = static_cast<int>(250 - t * 50);
                const DWORD fillColor =
                    (0xFFu << 24) | (hueR << 16) | (hueG << 8) | hueB;

                CRenderEngine::PushTransform(ctx, m);
                CRenderEngine::FillPath(ctx, star, fillColor);
                CRenderEngine::StrokePath(ctx, star, 0.8f, 0x66000000);
                CRenderEngine::PopTransform(ctx);
            }

            // 中心点强调
            const float hr = 4.0f * scale;
            RECT rcCenter = { static_cast<LONG>(cx - hr), static_cast<LONG>(cy - hr),
                              static_cast<LONG>(cx + hr), static_cast<LONG>(cy + hr) };
            CRenderEngine::FillEllipse(ctx, rcCenter, 0xFF1F2937);
        }
    } // namespace

    bool CPathArtUI::DoPaint(CPaintRenderContext& ctx, CControlUI* pStopControl)
    {
        if (!CControlUI::DoPaint(ctx, pStopControl)) return false;

        auto S = [this](int v) { return ScaleValue(v); };
        const float scale = ScaleValue(1000) / 1000.0f;   // 浮点缩放系数

        const RECT& rc = m_rcItem;

        // ── 主标题 ──
        RECT rcTitle = { rc.left + S(20), rc.top + S(10),
                         rc.right - S(20), rc.top + S(38) };
        CRenderEngine::DrawText(
            ctx, rcTitle,
            L"自由路径与变换栈：FillPath / StrokePath / Push(Pop)PathClip / Push(Pop)Transform",
            0xFF1E293B, 0,
            DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

        // ── 2×2 网格 ──
        const int gridTop = rc.top + S(40);
        const int gridGap = S(10);
        const int gridPad = S(10);
        const int gridW = (rc.right - rc.left - gridPad * 2 - gridGap) / 2;
        const int gridH = (rc.bottom - gridTop - gridGap - S(6)) / 2;

        RECT rcA = { rc.left + gridPad,                   gridTop,
                     rc.left + gridPad + gridW,           gridTop + gridH };
        RECT rcB = { rc.right - gridPad - gridW,          gridTop,
                     rc.right - gridPad,                  gridTop + gridH };
        RECT rcC = { rc.left + gridPad,                   gridTop + gridH + gridGap,
                     rc.left + gridPad + gridW,           gridTop + gridH * 2 + gridGap };
        RECT rcD = { rc.right - gridPad - gridW,          gridTop + gridH + gridGap,
                     rc.right - gridPad,                  gridTop + gridH * 2 + gridGap };

        const DWORD kCellBg     = 0xFFF8FAFC;
        const DWORD kCellBorder = 0xFFE2E8F0;
        const int   kCellRadius = S(8);
        for (const RECT* p : { &rcA, &rcB, &rcC, &rcD }) {
            CRenderEngine::DrawRoundColor(ctx, *p, kCellRadius, kCellRadius, kCellBg);
            CRenderEngine::DrawRoundRect (ctx, *p, 1.0f, kCellRadius, kCellRadius, kCellBorder);
        }

        const int subTitleH = S(20);
        DrawPanelTitle(ctx, rcA, L"A · FillPath + StrokePath（贝塞尔花瓣）",   subTitleH);
        DrawPanelTitle(ctx, rcB, L"B · PushTransform 旋转（彩虹风车）",         subTitleH);
        DrawPanelTitle(ctx, rcC, L"C · Push/PopPathClip（心形裁剪条纹）",       subTitleH);
        DrawPanelTitle(ctx, rcD, L"D · Scale×Rotate×Translate（变换螺旋星）",   subTitleH);

        // 4 段图形（注意把上方小标题区域让开）
        auto Inset = [&](RECT r) {
            r.top += subTitleH + S(4);
            r.left  += S(4); r.right  -= S(4); r.bottom -= S(4);
            return r;
        };
        DrawPetalFlower    (ctx, Inset(rcA), scale);
        DrawRotatedPinwheel(ctx, Inset(rcB), scale);
        DrawHeartClip      (ctx, Inset(rcC), scale);
        DrawTransformSpiral(ctx, Inset(rcD), scale);

        return true;
    }
}
