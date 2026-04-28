#include "pch.h"
#include "UIRenderImageLegacyInternal.h"

namespace FYUI
{
	void ConfigureImageFallbackGdiplusGraphicsInternal(Gdiplus::Graphics& graphics)
	{
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
		graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
		graphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
		graphics.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
	}
}
