#pragma once

#include "UIRenderContext.h"

#include <string>

namespace FYUI
{
	class CPaintManagerUI;
	struct tagTDrawInfo;
	typedef tagTDrawInfo TDrawInfo;
	struct tagTImageInfo;
	typedef tagTImageInfo TImageInfo;
}

namespace FYUI
{
	/**
	 * @brief 检查是否可以Use图像渲染上下文Internal
	 * @details 用于检查是否可以Use图像渲染上下文Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in] 绘制上下文
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool CanUseImageRenderContextInternal(const CPaintRenderContext& renderContext);
	/**
	 * @brief 绘制ResolvedGdiplus位图直接2D路径
	 * @details 用于绘制ResolvedGdiplus位图直接2D路径。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param data [in] 数据参数
	 * @param rcItem [in] 子项矩形区域
	 * @param rcBmpPart [in] BmpPart矩形区域
	 * @param uFade [in] Fade标志
	 * @param uRotate [in] Rotate标志
	 * @param bHasPaintRect [in] 是否判断是否具有绘制矩形
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool DrawResolvedGdiplusBitmapDirect2DPath(CPaintRenderContext& renderContext, const TImageInfo* data, const RECT& rcItem, const RECT& rcBmpPart, UINT uFade, UINT uRotate, bool bHasPaintRect);
	/**
	 * @brief 执行 MakeImageDest 操作
	 * @details 用于执行 MakeImageDest 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param rcControl [in] 控件矩形区域
	 * @param szImage [in] 图像尺寸参数
	 * @param sAlign [in] s对齐方式参数
	 * @param rcPadding [in] 内边距矩形
	 * @param rcDest [in,out] Dest矩形区域
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool MakeImageDest(const RECT& rcControl, const CDuiSize& szImage, const std::wstring& sAlign, const RECT& rcPadding, RECT& rcDest);
	/**
	 * @brief 获取绘制信息
	 * @details 用于获取绘制信息。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pDrawInfo [in] 绘制信息对象
	 * @param bUseRes [in] 是否UseRes
	 * @param bUseHSL [in] 是否UseHSL 颜色
	 * @param dwMask [in,out] Mask数值
	 * @param sImageResType [in,out] s图像Res类型参数
	 * @param pManager [in] 管理器对象
	 */
	void GetDrawInfo(TDrawInfo** pDrawInfo, bool bUseRes, bool bUseHSL, DWORD& dwMask, std::wstring& sImageResType, CPaintManagerUI* pManager);
	/**
	 * @brief 获取绘制信息2
	 * @details 用于获取绘制信息2。具体行为由当前对象状态以及传入参数共同决定。
	 * @param sDrawString [in,out] s绘制String参数
	 * @param rcCorner [in,out] Corner矩形区域
	 * @param rcDest [in,out] Dest矩形区域
	 * @param pManager [in] 管理器对象
	 */
	void GetDrawInfo2(std::wstring& sDrawString, RECT& rcCorner, RECT& rcDest, CPaintManagerUI* pManager);
	/**
	 * @brief 判断是否绘制图像绘制矩形Empty
	 * @details 用于判断是否绘制图像绘制矩形Empty。具体行为由当前对象状态以及传入参数共同决定。
	 * @param rcPaint [in] 重绘区域
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool IsDrawImagePaintRectEmpty(const RECT& rcPaint);
	/**
	 * @brief 执行 NormalizeDrawImageBitmapPart 操作
	 * @details 用于执行 NormalizeDrawImageBitmapPart 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param data [in] 数据参数
	 * @param rcBmpPart [in,out] BmpPart矩形区域
	 */
	void NormalizeDrawImageBitmapPart(const TImageInfo* data, RECT& rcBmpPart);
	/**
	 * @brief 执行 ShouldSkipDrawImageByClip 操作
	 * @details 用于执行 ShouldSkipDrawImageByClip 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param rc [in] 矩形区域
	 * @param rcPaint [in] 重绘区域
	 * @param rcItem [in] 子项矩形区域
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	bool ShouldSkipDrawImageByClip(const RECT& rc, const RECT& rcPaint, const RECT& rcItem);
	/**
	 * @brief 绘制Resolved位图图像路径
	 * @details 用于绘制Resolved位图图像路径。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param data [in] 数据参数
	 * @param rcItem [in] 子项矩形区域
	 * @param rcBmpPart [in] BmpPart矩形区域
	 * @param rcCorner [in] Corner矩形区域
	 * @param uFade [in] Fade标志
	 * @param bHole [in] 是否Hole
	 * @param bTiledX [in] 是否TiledX
	 * @param bTiledY [in] 是否TiledY
	 * @param pDrawInfoCache [in] 绘制信息缓存对象
	 * @param bHasPaintRect [in] 是否判断是否具有绘制矩形
	 */
	void DrawResolvedBitmapImagePath(CPaintRenderContext& renderContext, const TImageInfo* data, const RECT& rcItem, const RECT& rcBmpPart, const RECT& rcCorner, UINT uFade, bool bHole, bool bTiledX, bool bTiledY, TDrawInfo* pDrawInfoCache, bool bHasPaintRect);
}
