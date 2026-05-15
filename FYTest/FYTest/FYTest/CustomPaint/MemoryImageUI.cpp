#include "MemoryImageUI.h"
using namespace FYUI;

namespace FYTestApp
{
    // ── 生成更高质量的 128x128 BMP 渐变图（日落晚霞渐变） ──
    static void GenerateDefaultBmp(std::vector<unsigned char>& out)
    {
        const int W = 128, H = 128;
        const int rowBytes = ((W * 3 + 3) & ~3);
        const int pixelDataSize = rowBytes * H;
        const int fileSize = 54 + pixelDataSize;

        out.resize(fileSize, 0);
        unsigned char* p = out.data();

        // ── BITMAPFILEHEADER (14 bytes) ──
        p[0] = 'B'; p[1] = 'M';
        *reinterpret_cast<uint32_t*>(p + 2) = fileSize;
        *reinterpret_cast<uint32_t*>(p + 10) = 54;

        // ── BITMAPINFOHEADER (40 bytes) ──
        *reinterpret_cast<uint32_t*>(p + 14) = 40;
        *reinterpret_cast<int32_t*>(p + 18) = W;
        *reinterpret_cast<int32_t*>(p + 22) = H;
        *reinterpret_cast<uint16_t*>(p + 26) = 1;
        *reinterpret_cast<uint16_t*>(p + 28) = 24;

        // ── Pixel data (Pink to Orange gradient) ──
        for (int row = 0; row < H; ++row) {
            unsigned char* rowPtr = p + 54 + row * rowBytes;
            for (int col = 0; col < W; ++col) {
                float u = (float)col / (W - 1);
                float v = (float)row / (H - 1);
                
                // Color 1: Pink #EC4899 (236, 72, 153)
                // Color 2: Orange #F59E0B (245, 158, 11)
                unsigned char r = static_cast<unsigned char>(236 * (1.0f - v) + 245 * v);
                unsigned char g = static_cast<unsigned char>(72 * (1.0f - v) + 158 * v);
                unsigned char b = static_cast<unsigned char>(153 * (1.0f - u) + 11 * u);
                
                rowPtr[col * 3 + 0] = b;  // B
                rowPtr[col * 3 + 1] = g;  // G
                rowPtr[col * 3 + 2] = r;  // R
            }
        }
    }

    CMemoryImageUI::CMemoryImageUI() = default;

    CMemoryImageUI::~CMemoryImageUI()
    {
        if (m_pImageInfo) {
            CRenderEngine::FreeImage(m_pImageInfo);
            m_pImageInfo = nullptr;
        }
    }

    void CMemoryImageUI::SetImageData(const void* pData, DWORD dwSize)
    {
        if (m_pImageInfo) {
            CRenderEngine::FreeImage(m_pImageInfo);
            m_pImageInfo = nullptr;
        }
        if (pData && dwSize > 0) {
            m_pImageInfo = CRenderEngine::LoadImageFromMemory(pData, dwSize, 100, 0);
        }
        m_bDefaultLoaded = true;
        Invalidate();
    }

    void CMemoryImageUI::EnsureDefaultImage()
    {
        if (m_bDefaultLoaded) return;
        m_bDefaultLoaded = true;

        std::vector<unsigned char> bmpData;
        GenerateDefaultBmp(bmpData);
        m_pImageInfo = CRenderEngine::LoadImageFromMemory(bmpData.data(), static_cast<DWORD>(bmpData.size()), 100, 0);
    }

    bool CMemoryImageUI::DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl)
    {
        if (!CControlUI::DoPaint(renderContext, pStopControl)) return false;

        auto S = [this](int val) { return ScaleValue(val); };

        const RECT& rc = m_rcItem;

        // ── 标题 ──
        RECT rcTitle = { rc.left + S(20), rc.top + S(15), rc.right - S(20), rc.top + S(45) };
        CRenderEngine::DrawText(renderContext, rcTitle, L"内存图片控件：大尺寸高精度渐变 (LoadImageFromMemory)",
            0xFF1E293B, 0, DT_SINGLELINE | DT_LEFT | DT_VCENTER | DT_NOPREFIX);

        EnsureDefaultImage();

        // 绘制照片级相框
        if (m_pImageInfo) {
            RECT rcFrame = { rc.left + S(35), rc.top + S(55), rc.right - S(35), rc.bottom - S(75) };
            RECT rcShadow = { rcFrame.left + S(10), rcFrame.top + S(10), rcFrame.right + S(10), rcFrame.bottom + S(10) };
            CRenderEngine::DrawColor(renderContext, rcShadow, 0x1A000000); // 投影
            CRenderEngine::DrawColor(renderContext, rcFrame, 0xFFFFFFFF);  // 白框

            // 绘制内存图片（保留一定白边）
            RECT rcImg = { rcFrame.left + S(10), rcFrame.top + S(10), rcFrame.right - S(10), rcFrame.bottom - S(10) };
            CRenderEngine::DrawImage(renderContext, m_pImageInfo, rcImg, nullptr, 255);
            
            RECT rcInfo = { rc.left + S(20), rc.bottom - S(55), rc.right - S(20), rc.bottom - S(25) };
            wchar_t info[128];
            swprintf_s(info, L"图片尺寸: %dx%d  |  来源: 内存大号 128x128 BMP 晚霞渐变",
                m_pImageInfo->nX, m_pImageInfo->nY);
            CRenderEngine::DrawText(renderContext, rcInfo, info, 0xFF64748B, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }
        else {
            RECT rcErr = { rc.left + S(20), rc.top + S(50), rc.right - S(20), rc.bottom - S(10) };
            CRenderEngine::DrawText(renderContext, rcErr, L"未加载图片", 0xFFE64A4A, 0,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
        }

        return true;
    }
}
