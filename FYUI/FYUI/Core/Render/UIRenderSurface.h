#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	class CPaintRenderSurfaceInternalAccess;
	struct CPaintRenderSurfaceState;

	class FYUI_API CPaintRenderSurface
	{
	public:
		/**
		 * @brief 构造 CPaintRenderSurface 对象
		 * @details 用于构造 CPaintRenderSurface 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CPaintRenderSurface();
		/**
		 * @brief 析构 CPaintRenderSurface 对象
		 * @details 用于析构 CPaintRenderSurface 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CPaintRenderSurface();
		/**
		 * @brief 构造 CPaintRenderSurface 对象
		 * @details 用于构造 CPaintRenderSurface 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 */
		CPaintRenderSurface(const CPaintRenderSurface&) = delete;
		/**
		 * @brief 执行 operator= 运算
		 * @details 用于执行 operator= 运算。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return CPaintRenderSurface& 返回 执行 operator= 运算 的结果
		 */
		CPaintRenderSurface& operator=(const CPaintRenderSurface&) = delete;
		/**
		 * @brief 构造 CPaintRenderSurface 对象
		 * @details 用于构造 CPaintRenderSurface 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in,out] 回调参数
		 */
		CPaintRenderSurface(CPaintRenderSurface&&) = delete;
		/**
		 * @brief 执行 operator= 运算
		 * @details 用于执行 operator= 运算。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in,out] 回调参数
		 * @return CPaintRenderSurface& 返回 执行 operator= 运算 的结果
		 */
		CPaintRenderSurface& operator=(CPaintRenderSurface&&) = delete;

		/**
		 * @brief 执行 Ensure 操作
		 * @details 用于执行 Ensure 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Ensure(CPaintRenderContext& renderContext, LONG cx, LONG cy);
		/**
		 * @brief 执行 Ensure 操作
		 * @details 用于执行 Ensure 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Ensure(CPaintManagerUI* pManager, LONG cx, LONG cy);
		/**
		 * @brief 清空Transparent
		 * @details 用于清空Transparent。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ClearTransparent();
		/**
		 * @brief 执行 PresentLayeredWindow 操作
		 * @details 用于执行 PresentLayeredWindow 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 * @param ptDst [in] Dst坐标点
		 * @param alpha [in] alpha参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PresentLayeredWindow(HWND hWnd, const POINT& ptDst, BYTE alpha = 255) const;
		/**
		 * @brief 创建渲染上下文
		 * @details 用于创建渲染上下文。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param rcPaint [in] 重绘区域
		 * @param activeBackend [in] active渲染后端参数
		 * @param activeDirect2DMode [in] active直接2DMode参数
		 * @return CPaintRenderContext 返回 创建渲染上下文 的结果
		 */
		CPaintRenderContext CreateRenderContext(
			CPaintManagerUI* pManager,
			const RECT& rcPaint,
			RenderBackendType activeBackend = RenderBackendAuto,
			Direct2DRenderMode activeDirect2DMode = Direct2DRenderModeAuto) const;
		/**
		 * @brief 执行 DetachBitmap 操作
		 * @details 用于执行 DetachBitmap 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HBITMAP 返回 执行 DetachBitmap 操作 的结果
		 */
		HBITMAP DetachBitmap();
		/**
		 * @brief 执行 Reset 操作
		 * @details 用于执行 Reset 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Reset();

		/**
		 * @brief 获取位图
		 * @details 用于获取位图。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HBITMAP 返回 获取位图 的结果
		 */
		HBITMAP GetBitmap() const;
		/**
		 * @brief 获取Bits
		 * @details 用于获取Bits。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BYTE* 返回结果对象指针，失败时返回 nullptr
		 */
		BYTE* GetBits() const;
		/**
		 * @brief 获取颜色Bits
		 * @details 用于获取颜色Bits。具体行为由当前对象状态以及传入参数共同决定。
		 * @return COLORREF* 返回结果对象指针，失败时返回 nullptr
		 */
		COLORREF* GetColorBits() const;
		/**
		 * @brief 获取尺寸
		 * @details 用于获取尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetSize() const;
		/**
		 * @brief 判断是否Ready
		 * @details 用于判断是否Ready。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsReady() const;

	private:
		friend class CPaintRenderSurfaceInternalAccess;

		CPaintRenderSurfaceState* m_state = nullptr;
	};
}
