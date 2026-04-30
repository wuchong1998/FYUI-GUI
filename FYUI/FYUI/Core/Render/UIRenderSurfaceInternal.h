#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	class CPaintRenderSurface;

	/**
	 * @brief 执行 PrintWindowToRenderSurfaceInternal 操作
	 * @details 用于执行 PrintWindowToRenderSurfaceInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param surface [in,out] 绘制表面参数
	 * @param hWnd [in] 窗口句柄
	 * @param printFlags [in] printFlags参数
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool PrintWindowToRenderSurfaceInternal(CPaintRenderSurface& surface, HWND hWnd, LPARAM printFlags);
	/**
	 * @brief 执行 BitBltRenderSurfaceInternal 操作
	 * @details 用于执行 BitBltRenderSurfaceInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param surface [in] 绘制表面参数
	 * @param targetContext [in,out] 目标上下文参数
	 * @param rcDest [in] Dest矩形区域
	 * @param ptSource [in] 来源坐标点
	 * @param rop [in] rop参数
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool BitBltRenderSurfaceInternal(const CPaintRenderSurface& surface, CPaintRenderContext& targetContext, const RECT& rcDest, const POINT& ptSource, DWORD rop);
	/**
	 * @brief 更新Layered窗口From渲染绘制表面Internal
	 * @details 用于更新Layered窗口From渲染绘制表面Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param surface [in] 绘制表面参数
	 * @param hWnd [in] 窗口句柄
	 * @param targetContext [in,out] 目标上下文参数
	 * @param ptDst [in] Dst坐标点
	 * @param size [in] 尺寸参数
	 * @param alpha [in] alpha参数
	 * @param pDirtyRect [in] Dirty矩形对象
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool UpdateLayeredWindowFromRenderSurfaceInternal(
		const CPaintRenderSurface& surface,
		HWND hWnd,
		CPaintRenderContext& targetContext,
		const POINT& ptDst,
		const SIZE& size,
		BYTE alpha,
		const RECT* pDirtyRect = nullptr);
}
