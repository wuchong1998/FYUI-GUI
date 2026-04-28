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

} // namespace FYUI

#endif // __UIRENDERTYPES_H__
