#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	/**
	 * @brief 绘制颜色Internal
	 * @details 用于绘制颜色Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param rc [in] 矩形区域
	 * @param color [in] 颜色参数
	 */
	void DrawColorInternal(CPaintRenderContext& renderContext, const RECT& rc, DWORD color);
	/**
	 * @brief 绘制圆角颜色Internal
	 * @details 用于绘制圆角颜色Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param rc [in] 矩形区域
	 * @param width [in] 宽度参数
	 * @param height [in] 高度参数
	 * @param color [in] 颜色参数
	 */
	void DrawRoundColorInternal(CPaintRenderContext& renderContext, const RECT& rc, int width, int height, DWORD color);
	/**
	 * @brief 执行 PushD2DRectClipInternal 操作
	 * @details 用于执行 PushD2DRectClipInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param rc [in] 矩形区域
	 */
	void PushD2DRectClipInternal(const RECT& rc);
	/**
	 * @brief 执行 PushD2DRoundClipInternal 操作
	 * @details 用于执行 PushD2DRoundClipInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param rc [in] 矩形区域
	 * @param width [in] 宽度参数
	 * @param height [in] 高度参数
	 */
	void PushD2DRoundClipInternal(const RECT& rc, int width, int height);
	/**
	 * @brief 执行 PopD2DClipInternal 操作
	 * @details 用于执行 PopD2DClipInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 */
	void PopD2DClipInternal();
	/**
	 * @brief 绘制渐变Internal
	 * @details 用于绘制渐变Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param rc [in] 矩形区域
	 * @param dwFirst [in] First数值
	 * @param dwSecond [in] Second数值
	 * @param bVertical [in] 是否垂直
	 * @param nSteps [in] Steps数值
	 */
	void DrawGradientInternal(CPaintRenderContext& renderContext, const RECT& rc, DWORD dwFirst, DWORD dwSecond, bool bVertical, int nSteps);
	/**
	 * @brief 绘制行Internal
	 * @details 用于绘制行Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param rc [in] 矩形区域
	 * @param nSize [in] 尺寸数值
	 * @param dwPenColor [in] Pen颜色数值
	 * @param nStyle [in] 样式数值
	 */
	void DrawLineInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID);
	/**
	 * @brief 绘制矩形Internal
	 * @details 用于绘制矩形Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param rc [in] 矩形区域
	 * @param nSize [in] 尺寸数值
	 * @param dwPenColor [in] Pen颜色数值
	 * @param nStyle [in] 样式数值
	 */
	void DrawRectInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, DWORD dwPenColor, int nStyle = PS_SOLID);
	/**
	 * @brief 绘制圆角矩形Internal
	 * @details 用于绘制圆角矩形Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param rc [in] 矩形区域
	 * @param nSize [in] 尺寸数值
	 * @param width [in] 宽度参数
	 * @param height [in] 高度参数
	 * @param dwPenColor [in] Pen颜色数值
	 * @param nStyle [in] 样式数值
	 */
	void DrawRoundRectInternal(CPaintRenderContext& renderContext, const RECT& rc, int nSize, int width, int height, DWORD dwPenColor, int nStyle = PS_SOLID);
}
