#pragma once

#include "UIRenderContext.h"

#include <string_view>

namespace Gdiplus
{
	class Image;
}

namespace FYUI
{
	class CPaintRenderContext;
	struct tagTDrawInfo;
	typedef tagTDrawInfo TDrawInfo;

	/**
	 * @brief 绘制图像Internal
	 * @details 用于绘制图像Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param hBitmap [in] h位图参数
	 * @param rc [in] 矩形区域
	 * @param rcPaint [in] 重绘区域
	 * @param rcBmpPart [in] BmpPart矩形区域
	 * @param rcCorners [in] Corners矩形区域
	 * @param bAlpha [in] 是否Alpha
	 * @param uFade [in] Fade标志
	 * @param hole [in] hole参数
	 * @param xtiled [in] xtiled参数
	 * @param ytiled [in] ytiled参数
	 */
	void DrawImageInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, const RECT& rcCorners, bool bAlpha, UINT uFade = 255, bool hole = false, bool xtiled = false, bool ytiled = false);
	/**
	 * @brief 绘制Rotate图像Internal
	 * @details 用于绘制Rotate图像Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param hBitmap [in] h位图参数
	 * @param rc [in] 矩形区域
	 * @param rcPaint [in] 重绘区域
	 * @param rcBmpPart [in] BmpPart矩形区域
	 * @param bAlpha [in] 是否Alpha
	 * @param uFade [in] Fade标志
	 * @param uRotate [in] Rotate标志
	 */
	void DrawRotateImageInternal(CPaintRenderContext& renderContext, HBITMAP hBitmap, const RECT& rc, const RECT& rcPaint, const RECT& rcBmpPart, bool bAlpha, UINT uFade = 255, UINT uRotate = 0);
	/**
	 * @brief 绘制图像信息Internal
	 * @details 用于绘制图像信息Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param rcItem [in] 子项矩形区域
	 * @param pDrawInfo [in] 绘制信息对象
	 * @param instance [in] 模块实例句柄
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool DrawImageInfoInternal(CPaintRenderContext& renderContext, const RECT& rcItem, TDrawInfo* pDrawInfo, HINSTANCE instance = NULL);
	/**
	 * @brief 绘制图像StringInternal
	 * @details 用于绘制图像StringInternal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param rcItem [in] 子项矩形区域
	 * @param pStrImage [in] 图像描述字符串
	 * @param pStrModify [in] StrModify对象
	 * @param instance [in] 模块实例句柄
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool DrawImageStringInternal(CPaintRenderContext& renderContext, const RECT& rcItem, std::wstring_view pStrImage, std::wstring_view pStrModify = {}, HINSTANCE instance = NULL);
}
