#pragma once
#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    // CRenderPath + Push/PopTransform + Push/PopPathClip 综合演示控件
    // 在一个 2x2 网格内分别展示：
    //   A: FillPath / StrokePath 绘制五瓣花
    //   B: PushTransform(Rotation) 复制叶片形成彩虹风车
    //   C: PushPathClip / PopPathClip 心形裁剪 + 内部彩色条纹
    //   D: Scale*Rotation*Translation 复合矩阵分布的螺旋星阵
    class CPathArtUI : public FYUI::CControlUI
    {
    public:
        CPathArtUI() = default;
        std::wstring_view GetClass() const override { return L"PathArt"; }
        bool DoPaint(FYUI::CPaintRenderContext& renderContext, FYUI::CControlUI* pStopControl) override;
    };
}
