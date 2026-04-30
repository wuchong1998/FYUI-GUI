#pragma once
#ifndef __UIRENDER_H__
#define __UIRENDER_H__

#pragma once

#include "Render/UIRenderContext.h"

namespace FYUI {
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CRenderClip
	{
	public:
		/**
		 * @brief 构造 CRenderClip 对象
		 * @details 用于构造 CRenderClip 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CRenderClip() = default;
		/**
		 * @brief 析构 CRenderClip 对象
		 * @details 用于析构 CRenderClip 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CRenderClip();
		/**
		 * @brief 构造 CRenderClip 对象
		 * @details 用于构造 CRenderClip 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 */
		CRenderClip(const CRenderClip&) = delete;
		/**
		 * @brief 执行 operator= 运算
		 * @details 用于执行 operator= 运算。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return CRenderClip& 返回 执行 operator= 运算 的结果
		 */
		CRenderClip& operator=(const CRenderClip&) = delete;
		/**
		 * @brief 构造 CRenderClip 对象
		 * @details 用于构造 CRenderClip 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in,out] 回调参数
		 */
		CRenderClip(CRenderClip&&) = delete;
		/**
		 * @brief 执行 operator= 运算
		 * @details 用于执行 operator= 运算。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in,out] 回调参数
		 * @return CRenderClip& 返回 执行 operator= 运算 的结果
		 */
		CRenderClip& operator=(CRenderClip&&) = delete;

		/**
		 * @brief 执行 GenerateClip 操作
		 * @details 用于执行 GenerateClip 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in] 矩形区域
		 * @param clip [in,out] 裁剪区域参数
		 */
		static void GenerateClip(CPaintRenderContext& renderContext, RECT rc, CRenderClip& clip);
		/**
		 * @brief 执行 GenerateRoundClip 操作
		 * @details 用于执行 GenerateRoundClip 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in] 矩形区域
		 * @param rcItem [in] 子项矩形区域
		 * @param width [in] 宽度参数
		 * @param height [in] 高度参数
		 * @param clip [in,out] 裁剪区域参数
		 */
		static void GenerateRoundClip(CPaintRenderContext& renderContext, RECT rc, RECT rcItem, int width, int height, CRenderClip& clip);
		/**
		 * @brief 执行 UseOldClipBegin 操作
		 * @details 用于执行 UseOldClipBegin 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param clip [in,out] 裁剪区域参数
		 */
		static void UseOldClipBegin(CPaintRenderContext& renderContext, CRenderClip& clip);
		/**
		 * @brief 执行 UseOldClipEnd 操作
		 * @details 用于执行 UseOldClipEnd 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param clip [in,out] 裁剪区域参数
		 */
		static void UseOldClipEnd(CPaintRenderContext& renderContext, CRenderClip& clip);

	private:
		struct ClipState;
		ClipState* m_pClipState = nullptr;

		/**
		 * @brief 执行 PrepareClipState 操作
		 * @details 用于执行 PrepareClipState 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ClipState& 返回 执行 PrepareClipState 操作 的结果
		 */
		ClipState& PrepareClipState();
		/**
		 * @brief 重置裁剪区域状态
		 * @details 用于重置裁剪区域状态。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetClipState();
		/**
		 * @brief 执行 PushD2DClipForState 操作
		 * @details 用于执行 PushD2DClipForState 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param state [in,out] 状态参数
		 */
		static void PushD2DClipForState(ClipState& state);
		/**
		 * @brief 执行 PopD2DClipForState 操作
		 * @details 用于执行 PopD2DClipForState 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param state [in,out] 状态参数
		 */
		static void PopD2DClipForState(ClipState& state);
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CRenderEngine
	{
	public:
		/**
		 * @brief 设置Preferred渲染渲染后端
		 * @details 用于设置Preferred渲染渲染后端。具体行为由当前对象状态以及传入参数共同决定。
		 * @param backend [in] 渲染后端参数
		 */
		static void SetPreferredRenderBackend(RenderBackendType backend);
		/**
		 * @brief 获取Preferred渲染渲染后端
		 * @details 用于获取Preferred渲染渲染后端。具体行为由当前对象状态以及传入参数共同决定。
		 * @return RenderBackendType 返回 获取Preferred渲染渲染后端 的结果
		 */
		static RenderBackendType GetPreferredRenderBackend();
		/**
		 * @brief 获取Active渲染渲染后端
		 * @details 用于获取Active渲染渲染后端。具体行为由当前对象状态以及传入参数共同决定。
		 * @return RenderBackendType 返回 获取Active渲染渲染后端 的结果
		 */
		static RenderBackendType GetActiveRenderBackend();
		/**
		 * @brief 设置Preferred直接2D渲染Mode
		 * @details 用于设置Preferred直接2D渲染Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @param mode [in] mode参数
		 */
		static void SetPreferredDirect2DRenderMode(Direct2DRenderMode mode);
		/**
		 * @brief 获取Preferred直接2D渲染Mode
		 * @details 用于获取Preferred直接2D渲染Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return Direct2DRenderMode 返回 获取Preferred直接2D渲染Mode 的结果
		 */
		static Direct2DRenderMode GetPreferredDirect2DRenderMode();
		/**
		 * @brief 获取Active直接2D渲染Mode
		 * @details 用于获取Active直接2D渲染Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return Direct2DRenderMode 返回 获取Active直接2D渲染Mode 的结果
		 */
		static Direct2DRenderMode GetActiveDirect2DRenderMode();
		/**
		 * @brief 判断是否直接2DAvailable
		 * @details 用于判断是否直接2DAvailable。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		static bool IsDirect2DAvailable();
		/**
		 * @brief 重置帧统计信息
		 * @details 用于重置帧统计信息。具体行为由当前对象状态以及传入参数共同决定。
		 */
		static void ResetFrameMetrics();
		/**
		 * @brief 提取并清空帧统计信息
		 * @details 用于提取并清空帧统计信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nDirect2DBatchFlushes [in,out] 直接2D批处理Flushes数值
		 * @param nDirect2DStandaloneDraws [in,out] 直接2DStandaloneDraws数值
		 * @param nTextLayoutCacheHits [in,out] 文本布局缓存Hits数值
		 * @param nTextLayoutCacheRefreshes [in,out] 文本布局缓存Refreshes数值
		 * @param nTextMetricsCacheHits [in,out] 文本度量信息缓存Hits数值
		 * @param nTextMetricsCacheRefreshes [in,out] 文本度量信息缓存Refreshes数值
		 * @param nTextNormalizationCacheHits [in,out] 文本Normalization缓存Hits数值
		 * @param nTextNormalizationCacheRefreshes [in,out] 文本Normalization缓存Refreshes数值
		 * @param nHtmlParseCacheHits [in,out] HTML 文本解析缓存Hits数值
		 * @param nHtmlParseCacheRefreshes [in,out] HTML 文本解析缓存Refreshes数值
		 * @param nHtmlLayoutCacheHits [in,out] HTML 文本布局缓存Hits数值
		 * @param nHtmlLayoutCacheRefreshes [in,out] HTML 文本布局缓存Refreshes数值
		 * @param nHtmlMetricsCacheHits [in,out] HTML 文本度量信息缓存Hits数值
		 * @param nHtmlMetricsCacheRefreshes [in,out] HTML 文本度量信息缓存Refreshes数值
		 * @param nHtmlHitTestCacheHits [in,out] HTML 文本命中测试缓存Hits数值
		 * @param nHtmlHitTestCacheRefreshes [in,out] HTML 文本命中测试缓存Refreshes数值
		 * @param nHtmlDrawingEffectCacheHits [in,out] HTML 文本DrawingEffect缓存Hits数值
		 * @param nHtmlDrawingEffectCacheRefreshes [in,out] HTML 文本DrawingEffect缓存Refreshes数值
		 * @param nHtmlDirectWriteParseFailures [in,out] HTML 文本直接写入解析Failures数值
		 * @param nHtmlDirectWriteLayoutFailures [in,out] HTML 文本直接写入布局Failures数值
		 * @param nHtmlDirectWriteRenderFailures [in,out] HTML 文本直接写入渲染Failures数值
		 */
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
		/**
		 * @brief 加载图像
		 * @details 用于加载图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bitmap [in] 位图参数
		 * @param nScale [in] 缩放比例数值
		 * @param type [in] 类型参数
		 * @param mask [in] mask参数
		 * @param instance [in] 模块实例句柄
		 * @return TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		static TImageInfo* LoadImage(STRINGorID bitmap, int nScale, std::wstring_view type = {},
									DWORD mask = 0, HINSTANCE instance = NULL);
		/**
		 * @brief 执行 FreeImage 操作
		 * @details 用于执行 FreeImage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pImageInfo [in] 图像信息对象
		 * @param bDelete [in] 是否删除
		 */
		static void FreeImage(TImageInfo* pImageInfo, bool bDelete = true);
		/**
		 * @brief 执行 FreeBitmap 操作
		 * @details 用于执行 FreeBitmap 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hBitmap [in] h位图参数
		 */
		static void FreeBitmap(HBITMAP hBitmap);
		/**
		 * @brief 加载图像
		 * @details 用于加载图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 * @param nScale [in] 缩放比例数值
		 * @param type [in] 类型参数
		 * @param mask [in] mask参数
		 * @param instance [in] 模块实例句柄
		 * @return TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		static TImageInfo* LoadImage(std::wstring_view pStrImage, int nScale, std::wstring_view type = {},
										DWORD mask = 0, HINSTANCE instance = NULL);
		/**
		 * @brief 加载图像
		 * @details 用于加载图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nID [in] ID数值
		 * @param nScale [in] 缩放比例数值
		 * @param type [in] 类型参数
		 * @param mask [in] mask参数
		 * @param instance [in] 模块实例句柄
		 * @return TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		static TImageInfo* LoadImage(UINT nID, int nScale, std::wstring_view type = {}, DWORD mask = 0, HINSTANCE instance = NULL);

		/**
		 * @brief 绘制图像
		 * @details 用于绘制图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param hBitmap [in] h位图参数
		 * @param rc [in] 矩形区域
		 * @param rcBmpPart [in] BmpPart矩形区域
		 * @param rcCorners [in] Corners矩形区域
		 * @param bAlpha [in] 是否Alpha
		 * @param uFade [in] Fade标志
		 * @param hole [in] hole参数
		 * @param xtiled [in] xtiled参数
		 * @param ytiled [in] ytiled参数
		 */
		static void DrawImage(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcBmpPart,
			const RECT& rcCorners, bool bAlpha, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false);

		/**
		 * @brief 绘制Rotate图像
		 * @details 用于绘制Rotate图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param hBitmap [in] h位图参数
		 * @param rc [in] 矩形区域
		 * @param rcBmpPart [in] BmpPart矩形区域
		 * @param bAlpha [in] 是否Alpha
		 * @param uFade [in] Fade标志
		 * @param uRotate [in] Rotate标志
		 */
		static void DrawRotateImage(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, 
										const RECT& rcBmpPart, bool bAlpha, UINT uFade = 255, UINT uRotate = 0);
		/**
		 * @brief 绘制图像信息
		 * @details 用于绘制图像信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 * @param pDrawInfo [in] 绘制信息对象
		 * @param instance [in] 模块实例句柄
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		static bool DrawImageInfo(CPaintRenderContext& renderContext, const RECT& rcItem, TDrawInfo* pDrawInfo,HINSTANCE instance = NULL);
		/**
		 * @brief 绘制图像String
		 * @details 用于绘制图像String。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 * @param pStrImage [in] 图像描述字符串
		 * @param pStrModify [in] StrModify对象
		 * @param instance [in] 模块实例句柄
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		static bool DrawImageString(CPaintRenderContext& renderContext, const RECT& rcItem, std::wstring_view pStrImage,
									std::wstring_view pStrModify = {}, HINSTANCE instance = NULL);

		// Gdiplus缂佹ê鍩?
		/**
		 * @brief 执行 GdiplusLoadImage 操作
		 * @details 用于执行 GdiplusLoadImage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bitmap [in] 位图参数
		 * @param type [in] 类型参数
		 * @param mask [in] mask参数
		 * @param instance [in] 模块实例句柄
		 * @return TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		static TImageInfo* GdiplusLoadImage(STRINGorID bitmap, std::wstring_view type = {}, DWORD mask = 0, HINSTANCE instance = NULL);
		/**
		 * @brief 执行 GdiplusLoadImage 操作
		 * @details 用于执行 GdiplusLoadImage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 * @param type [in] 类型参数
		 * @param mask [in] mask参数
		 * @param instance [in] 模块实例句柄
		 * @return TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		static TImageInfo* GdiplusLoadImage(std::wstring_view pStrImage, std::wstring_view type = {}, DWORD mask = 0,HINSTANCE instance = NULL);

		// 娴犮儰绗呴崙鑺ユ殶娑擃厾娈戞０婊嗗閸欏倹鏆焌lpha閸婂吋妫ら弫?
		// 閸ユ儳鍘撶紒妯哄煑
		/**
		 * @brief 绘制颜色
		 * @details 用于绘制颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in] 矩形区域
		 * @param color [in] 颜色参数
		 */
		static void DrawColor(CPaintRenderContext& renderContext, const RECT& rc, DWORD color);
		/**
		 * @brief 绘制圆角颜色
		 * @details 用于绘制圆角颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in] 矩形区域
		 * @param width [in] 宽度参数
		 * @param height [in] 高度参数
		 * @param color [in] 颜色参数
		 */
		static void DrawRoundColor(CPaintRenderContext& renderContext, const RECT& rc, int width, int height, DWORD color);
		/**
		 * @brief 绘制渐变
		 * @details 用于绘制渐变。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in] 矩形区域
		 * @param dwFirst [in] First数值
		 * @param dwSecond [in] Second数值
		 * @param bVertical [in] 是否垂直
		 * @param nSteps [in] Steps数值
		 */
		static void DrawGradient(CPaintRenderContext& renderContext, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps);
		/**
		 * @brief 绘制行
		 * @details 用于绘制行。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in] 矩形区域
		 * @param nSize [in] 尺寸数值
		 * @param dwPenColor [in] Pen颜色数值
		 * @param nStyle [in] 样式数值
		 */
		static void DrawLine(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor,int nStyle = PS_SOLID);
		/**
		 * @brief 绘制矩形
		 * @details 用于绘制矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in] 矩形区域
		 * @param nSize [in] 尺寸数值
		 * @param dwPenColor [in] Pen颜色数值
		 * @param nStyle [in] 样式数值
		 */
		static void DrawRect(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor,int nStyle = PS_SOLID);
		/**
		 * @brief 绘制圆角矩形
		 * @details 用于绘制圆角矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in] 矩形区域
		 * @param nSize [in] 尺寸数值
		 * @param width [in] 宽度参数
		 * @param height [in] 高度参数
		 * @param dwPenColor [in] Pen颜色数值
		 * @param nStyle [in] 样式数值
		 */
		static void DrawRoundRect(CPaintRenderContext& renderContext, const RECT& rc, int nSize, int width, 
									int height, DWORD dwPenColor,int nStyle = PS_SOLID);

		// 鐎涙ぞ缍嬬紒妯哄煑
		/**
		 * @brief 绘制文本
		 * @details 用于绘制文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in,out] 矩形区域
		 * @param text [in] 文本内容
		 * @param dwTextColor [in] 文本颜色值
		 * @param iFont [in] 字体值
		 * @param uStyle [in] 样式标志
		 * @param dwTextBKColor [in] 文本背景颜色数值
		 */
		static void DrawText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, 
			int iFont, UINT uStyle, DWORD dwTextBKColor);
		/**
		 * @brief 绘制文本
		 * @details 用于绘制文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in,out] 矩形区域
		 * @param text [in] 文本内容
		 * @param dwTextColor [in] 文本颜色值
		 * @param iFont [in] 字体值
		 * @param uStyle [in] 样式标志
		 */
		static void DrawText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor, 
			int iFont, UINT uStyle);

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
		/**
		 * @brief 绘制HTML 文本文本
		 * @details 用于绘制HTML 文本文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rc [in,out] 矩形区域
		 * @param text [in] 文本内容
		 * @param dwTextColor [in] 文本颜色值
		 * @param pLinks [in] Links对象
		 * @param sLinks [in] sLinks参数
		 * @param nLinkRects [in,out] LinkRects数值
		 * @param iFont [in] 字体值
		 * @param uStyle [in] 样式标志
		 */
		static void DrawHtmlText(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view text, DWORD dwTextColor,
			RECT* pLinks, std::wstring* sLinks, int& nLinkRects, int iFont, UINT uStyle);

		// 鏉堝懎濮崙鑺ユ殶
		/**
		 * @brief 执行 CheckAlphaColor 操作
		 * @details 用于执行 CheckAlphaColor 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in,out] 颜色值
		 */
		static void CheckAlphaColor(DWORD& dwColor);
		/**
		 * @brief 调整颜色
		 * @details 用于调整颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @param H [in] H参数
		 * @param S [in] S参数
		 * @param L [in] L参数
		 * @return DWORD 返回对应的数值结果
		 */
		static DWORD AdjustColor(DWORD dwColor, short H, short S, short L);
		/**
		 * @brief 调整图像
		 * @details 用于调整图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bUseHSL [in] 是否UseHSL 颜色
		 * @param imageInfo [in] 图像信息参数
		 * @param H [in] H参数
		 * @param S [in] S参数
		 * @param L [in] L参数
		 */
		static void AdjustImage(bool bUseHSL, TImageInfo* imageInfo, short H, short S, short L);

		/**
		 * @brief 执行 GenerateBitmap 操作
		 * @details 用于执行 GenerateBitmap 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param rc [in] 矩形区域
		 * @param pStopControl [in] 绘制终止控件
		 * @param dwFilterColor [in] Filter颜色数值
		 * @return HBITMAP 返回 执行 GenerateBitmap 操作 的结果
		 */
		static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, RECT rc, CControlUI* pStopControl = NULL, DWORD dwFilterColor = 0);
		/**
		 * @brief 执行 GenerateBitmap 操作
		 * @details 用于执行 GenerateBitmap 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param pControl [in] 控件对象
		 * @param rc [in] 矩形区域
		 * @param dwFilterColor [in] Filter颜色数值
		 * @return HBITMAP 返回 执行 GenerateBitmap 操作 的结果
		 */
		static HBITMAP GenerateBitmap(CPaintManagerUI* pManager, CControlUI* pControl, RECT rc, DWORD dwFilterColor = 0);
		/**
		 * @brief 获取文本尺寸
		 * @details 用于获取文本尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param text [in] 文本内容
		 * @param iFont [in] 字体值
		 * @param uStyle [in] 样式标志
		 * @return 返回对应的几何结果
		 */
		static SIZE GetTextSize(CPaintRenderContext& renderContext, std::wstring_view text, int iFont, UINT uStyle);


	};

} // namespace DuiLib

#endif // __UIRENDER_H__
