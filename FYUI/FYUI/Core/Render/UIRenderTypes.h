#pragma once
#ifndef __UIRENDERTYPES_H__
#define __UIRENDERTYPES_H__

namespace FYUI {

	enum RenderBackendType
	{
		RenderBackendAuto = 0,
		RenderBackendDirect2D,
	};

	enum Direct2DRenderMode
	{
		Direct2DRenderModeAuto = 0,
		Direct2DRenderModeHardware,
		Direct2DRenderModeSoftware,
	};

	// 透明窗口（bktrans="true"）的呈现模式：
	//  - Auto：根据系统版本自动选择（Win8+ 用 DComp，Win7 退回 GdiThrottled）
	//  - GdiThrottled：现有 UpdateLayeredWindow + 节流 + dirty rect 合并路径，全平台可用
	//  - DComp：DirectComposition + D2D Swap Chain，Win8+ 才有效；Win7 上自动回退为 GdiThrottled
	enum LayeredPresentMode
	{
		LayeredPresentModeAuto = 0,
		LayeredPresentModeGdiThrottled,
		LayeredPresentModeDComp,
	};

} // namespace FYUI

#endif // __UIRENDERTYPES_H__
