#pragma once
#include "framework.h"
#include "FYUILib.h"

namespace FYTestApp
{
    // 内存图片控件：演示从内存加载 PNG/JPG 等图片数据并绘制
    class CMemoryImageUI : public FYUI::CControlUI
    {
    public:
        CMemoryImageUI();
        ~CMemoryImageUI();

        std::wstring_view GetClass() const override { return L"MemoryImage"; }
        bool DoPaint(FYUI::CPaintRenderContext& renderContext, FYUI::CControlUI* pStopControl) override;

        /// 外部设置图片数据（控件内部会拷贝一份并通过 LoadImageFromMemory 解码缓存）
        void SetImageData(const void* pData, DWORD dwSize);

    private:
        void EnsureDefaultImage();
        FYUI::TImageInfo* m_pImageInfo = nullptr;
        bool m_bDefaultLoaded = false;
    };
}
