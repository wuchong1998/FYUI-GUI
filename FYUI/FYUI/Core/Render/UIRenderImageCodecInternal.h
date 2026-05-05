#pragma once

#include <vector>

#include "../UIManager.h"

namespace FYUI
{
	struct WebpAnimationFrameBitmapInternal
	{
		HBITMAP hBitmap = nullptr;
		UINT delayMs = 100;
		bool hasAlpha = false;
	};

	/**
	 * @brief 从内存数据加载静态 WebP 图像
	 * @details 当通用图片解码链路遇到 WebP 数据时，可调用该内部辅助函数把内存中的 WebP 解码为 `TImageInfo`。
	 * 返回的位图为顶向下 32 位 DIB，颜色通道使用适合 FYUI 渲染链路的预乘 BGRA 布局。
	 * @param pData [in] WebP 二进制数据起始地址
	 * @param dwSize [in] WebP 二进制数据长度，单位为字节
	 * @param nScale [in] 缩放基准，当前静态位图仍按原始像素尺寸解码
	 * @param mask [in] 颜色掩码，命中该颜色的像素会被视为透明
	 * @return TImageInfo* 解码成功返回图像信息对象，失败返回 nullptr
	 */
	TImageInfo* LoadWebpStillImageInfoFromMemoryInternal(const void* pData, DWORD dwSize, int nScale, DWORD mask);

	/**
	 * @brief 从内存数据解码 WebP 动画帧
	 * @details 使用 `libwebp` 的动画解码器把 WebP 数据还原成逐帧位图，每一帧都已经是完整画布，
	 * 可直接用于控件定时播放。调用成功后，`frames` 中的位图所有权转交给调用方。
	 * @param pData [in] WebP 二进制数据起始地址
	 * @param dwSize [in] WebP 二进制数据长度，单位为字节
	 * @param mask [in] 颜色掩码，命中该颜色的像素会被视为透明
	 * @param frames [in,out] 输出帧集合，成功时会写入全部帧数据
	 * @param canvasSize [in,out] 输出动画画布尺寸
	 * @return bool 解码成功返回 true，数据无效或解码失败返回 false
	 */
	bool DecodeWebpAnimationFromMemoryInternal(
		const void* pData,
		DWORD dwSize,
		DWORD mask,
		std::vector<WebpAnimationFrameBitmapInternal>& frames,
		SIZE& canvasSize);

	/**
	 * @brief 释放 WebP 动画帧位图集合
	 * @details 释放 `DecodeWebpAnimationFromMemoryInternal` 产出的所有帧位图，并清空容器。
	 * @param frames [in,out] 动画帧集合
	 */
	void FreeWebpAnimationFramesInternal(std::vector<WebpAnimationFrameBitmapInternal>& frames);
}
