#pragma once
#ifndef __UIRENDER_H__
#define __UIRENDER_H__

#pragma once

#include "UIRenderContext.h"

namespace FYUI {
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CRenderClip
	{
	public:
		CRenderClip() = default;
		~CRenderClip();
		CRenderClip(const CRenderClip&) = delete;
		CRenderClip& operator=(const CRenderClip&) = delete;
		CRenderClip(CRenderClip&&) = delete;
		CRenderClip& operator=(CRenderClip&&) = delete;

		static void GenerateClip(CPaintRenderContext& renderContext, RECT rc, CRenderClip& clip);
		static void GenerateRoundClip(CPaintRenderContext& renderContext, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip);
		static void UseOldClipBegin(CPaintRenderContext& renderContext, CRenderClip& clip);
		static void UseOldClipEnd(CPaintRenderContext& renderContext, CRenderClip& clip);

	private:
		struct ClipState;
		ClipState* m_pClipState = nullptr;

		ClipState& PrepareClipState();
		void ResetClipState();
		static void PushD2DClipForState(ClipState& state);
		static void PopD2DClipForState(ClipState& state);
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CRenderEngine
	{
	public:
		static void SetPreferredRenderBackend(RenderBackendType backend);
		static RenderBackendType GetPreferredRenderBackend();
		static RenderBackendType GetActiveRenderBackend();
		static void SetPreferredDirect2DRenderMode(Direct2DRenderMode mode);
		static Direct2DRenderMode GetPreferredDirect2DRenderMode();
		static Direct2DRenderMode GetActiveDirect2DRenderMode();
		static bool IsDirect2DAvailable();
		static void ResetFrameMetrics();
		static void ConsumeFrameMetrics(
			UINT& nDirect2DBatchFlushes,
			UINT& nDirect2DStandaloneDraws,
			UINT& nTextLayoutCacheHits,
			UINT& nTextLayoutCacheRefreshes,
			UINT& nTextMetricsCacheHits,
			UINT& nTextMetricsCacheRefreshes,
			UINT& nTextNormalizationCacheHits,
			UINT& nTextNormalizationCacheRefreshes,
			UINT& nHtmlParseCacheHits,
			UINT& nHtmlParseCacheRefreshes,
			UINT& nHtmlLayoutCacheHits,
			UINT& nHtmlLayoutCacheRefreshes,
			UINT& nHtmlMetricsCacheHits,
			UINT& nHtmlMetricsCacheRefreshes,
			UINT& nHtmlHitTestCacheHits,
			UINT& nHtmlHitTestCacheRefreshes,
			UINT& nHtmlDrawingEffectCacheHits,
			UINT& nHtmlDrawingEffectCacheRefreshes,
			UINT& nHtmlDirectWriteParseFailures,
			UINT& nHtmlDirectWriteLayoutFailures,
			UINT& nHtmlDirectWriteRenderFailures);

		// 閸ュ墽澧栭崝鐘烘祰
		static TImageInfo* LoadImage(STRINGorID bitmap, int nScale, std::wstring_view type = {},
									DWORD mask = 0, HINSTANCE instance = NULL);
		static void FreeImage(TImageInfo* pImageInfo, bool bDelete = true);
		static void FreeBitmap(HBITMAP hBitmap);
		static TImageInfo* LoadImage(std::wstring_view pStrImage, int nScale, std::wstring_view type = {}, DWORD mask = 0, HINSTANCE instance = NULL);
		static TImageInfo* LoadImage(UINT nID, int nScale, std::wstring_view type = {}, DWORD mask = 0, HINSTANCE instance = NULL);

		static void DrawImage(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcBmpPart,
			const RECT& rcCorners, bool bAlpha, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false);

		static void DrawRotateImage(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcBmpPart, bool bAlpha, UINT uFade = 255, UINT uRotate = 0);
		static bool DrawImageInfo(CPaintRenderContext& renderContext, const RECT& rcItem, TDrawInfo* pDrawInfo, HINSTANCE instance = NULL);
		static bool DrawImageString(CPaintRenderContext& renderContext, const RECT& rcItem, std::wstring_view pStrImage, std::wstring_view pStrModify = {}, HINSTANCE instance = NULL);

		// Gdiplus缂佹ê鍩?
		static TImageInfo* GdiplusLoadImage(STRINGorID bitmap, std::wstring_view type = {}, DWORD mask = 0, HINSTANCE instance = NULL);
		static TImageInfo* GdiplusLoadImage(std::wstring_view pStrImage, std::wstring_view type = {}, DWORD mask = 0, HINSTANCE instance = NULL);

		// 娴犮儰绗呴崙鑺ユ殶娑擃厾娈戞０婊嗗閸欏倹鏆焌lpha閸婂吋妫ら弫?
		// 閸ユ儳鍘撶紒妯哄煑
		static void DrawColor(CPaintRenderContext& renderContext, const RECT& rc, DWORD color);
		static void DrawRoundColor(CPaintRenderContext& renderContext, const RECT& rc, int width, int height, DWORD color);
		static void DrawGradient(CPaintRenderContext& renderContext, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps);
		static void DrawLine(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor,int nStyle = PS_SOLID);
		static void DrawRect(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor,int nStyle = PS_SOLID);
		static void DrawRoundRect(CPaintRenderContext& renderContext, const RECT& rc, int nSize, int width, int height, DWORD dwPenColor,int nStyle = PS_SOLID);

		// 鐎涙ぞ缍嬬紒妯哄煑
		static void DrawText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, int iFont, UINT uStyle, DWORD dwTextBKColor);
		static void DrawText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, int iFont, UINT uStyle);

		// DrawHtmlText 的 text 不是浏览器完整 HTML，而是 FYUI 内置的轻量富文本标记。
		// text 是 UTF-16 std::wstring_view：可传 std::wstring、std::wstring_view 或 L"..."。
		// std::wstring_view 不持有内存，传入文本只需要在本次同步绘制调用期间有效。
		// 本工程源文件按 UTF-8 保存，并在 C/C++ 命令行启用 /utf-8，因此源码中可直接写：
		//   L"普通文本 <b>加粗</b> <c #FF3355>红色</c> <a user://42>链接</a>"
		//
		// 标记可以使用 <tag>...</tag>，也可以使用 {tag}...{/tag}；打开和关闭分隔符要配对。
		// 支持的样式标记：
		//   <b>...</b>                     加粗。
		//   <u>...</u>                     下划线。
		//   <i>...</i>                     斜体；如果 <i ...> 带内容，则表示内联图片，见下方。
		//   <s>...</s>                     使用默认选中背景色绘制文本背景。
		//   <c #RRGGBB>...</c>             设置文字颜色；也支持 #AARRGGBB。
		//   <f 2>...</f>                   使用 PaintManager 字体表中的第 2 号字体。
		//   <f Microsoft YaHei 14 bold>...</f>
		//                                  使用字体名、字号和可选属性 bold/italic/underline/strikeout。
		//   <a target>...</a>              链接；target 会写入 sLinks，文本区域会写入 pLinks。
		//   <p 8>...</p>                   段落；数字表示额外段落/行间距，DT_SINGLELINE 下不换段。
		//   <r>...</r>                     原样文本；其中的 <b> 等标记不会继续解析。
		//
		// 支持的内联标记：
		//   <n>                            换行；DT_SINGLELINE 下会被忽略。
		//   <x 12>                         插入 12 个当前绘制单位的水平空白。
		//   <y 8>                          插入 8 个当前绘制单位的垂直空白。
		//   <i file='icons.png'>           插入图片文件。
		//   <i res='IDB_ICON' restype='PNG'>插入资源图片。
		//   <i imageName 5 2>              插入已注册图片 imageName 的横向切片，5 为总数，2 为从 0 开始的索引。
		//
		// 转义写法：{<} 输出 <，{>} 输出 >，<{> 输出 {，<}> 输出 }。
		// 链接输出：调用前 nLinkRects 表示 pLinks/sLinks 的容量；返回后表示实际写入的链接矩形数量。
		// 标签必须正确嵌套和闭合；无法解析的标记会导致本次富文本绘制失败。
		static void DrawHtmlText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, RECT* pLinks, std::wstring* sLinks, int& nLinkRects, int iFont, UINT uStyle);

		// 鏉堝懎濮崙鑺ユ殶
		static void CheckAlphaColor(DWORD& dwColor);
		static DWORD AdjustColor(DWORD dwColor, short H, short S, short L);
		static void AdjustImage(bool bUseHSL, TImageInfo* imageInfo, short H, short S, short L);

		static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, RECT rc, CControlUI* pStopControl = NULL, DWORD dwFilterColor = 0);
		static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc, DWORD dwFilterColor = 0);
		static SIZE GetTextSize(CPaintRenderContext& renderContext, std::wstring_view text, int iFont, UINT uStyle);
	};

} // namespace DuiLib

#endif // __UIRENDER_H__
