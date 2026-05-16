#include "pch.h"
#include "framework.h"
#include "SVG.h"

#include <algorithm>

HBITMAP CreateARGB32Bitmap(HDC hDC, int cx, int cy, BYTE** pBits, bool bPositive)
{
	BITMAPINFO BmpSrc = {};
	BmpSrc.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BmpSrc.bmiHeader.biWidth = cx;
	BmpSrc.bmiHeader.biHeight = bPositive ? -cy : cy;
	BmpSrc.bmiHeader.biPlanes = 1;
	BmpSrc.bmiHeader.biBitCount = 32;
	BmpSrc.bmiHeader.biCompression = BI_RGB;
	BmpSrc.bmiHeader.biSizeImage = cx * cy * 4;
	BmpSrc.bmiHeader.biXPelsPerMeter = 0;
	BmpSrc.bmiHeader.biYPelsPerMeter = 0;
	BmpSrc.bmiHeader.biClrUsed = 0;
	BmpSrc.bmiHeader.biClrImportant = 0;

	HBITMAP hBitmap = CreateDIBSection(hDC, &BmpSrc, DIB_RGB_COLORS, (void**)pBits, NULL, NULL);

	return hBitmap;
}


HBITMAP SVGDataToHBITMAP(unsigned char* data, int width, int height, int stride)
{
	LPBYTE pDest = NULL;
	HBITMAP hBitmap = CreateARGB32Bitmap(nullptr, width, height, &pDest, true);
	if (hBitmap == nullptr || pDest == nullptr || data == nullptr || stride <= 0) {
		if (hBitmap != nullptr) {
			::DeleteObject(hBitmap);
		}
		return nullptr;
	}

	const int destStride = width * 4;
	const int copyStride = (std::min)(destStride, stride);
	for (int y = 0; y < height; ++y) {
		const LPBYTE pSrcRow = data + static_cast<size_t>(y) * stride;
		LPBYTE pDstRow = pDest + static_cast<size_t>(y) * destStride;
		for (int x = 0; x < width; ++x) {
			const BYTE alpha = pSrcRow[x * 4 + 3];
			pDstRow[x * 4 + 3] = alpha;
			if (alpha == 0) {
				pDstRow[x * 4 + 0] = 0;
				pDstRow[x * 4 + 1] = 0;
				pDstRow[x * 4 + 2] = 0;
			}
			else if (alpha == 255) {
				pDstRow[x * 4 + 0] = pSrcRow[x * 4 + 0];
				pDstRow[x * 4 + 1] = pSrcRow[x * 4 + 1];
				pDstRow[x * 4 + 2] = pSrcRow[x * 4 + 2];
			}
			else {
				pDstRow[x * 4 + 0] = static_cast<BYTE>((std::min)(255, (pSrcRow[x * 4 + 0] * 255 + alpha / 2) / alpha));
				pDstRow[x * 4 + 1] = static_cast<BYTE>((std::min)(255, (pSrcRow[x * 4 + 1] * 255 + alpha / 2) / alpha));
				pDstRow[x * 4 + 2] = static_cast<BYTE>((std::min)(255, (pSrcRow[x * 4 + 2] * 255 + alpha / 2) / alpha));
			}
		}
		if (copyStride < destStride) {
			std::memset(pDstRow + copyStride, 0, destStride - copyStride);
		}
	}
	return hBitmap;
}
HBITMAP RenderSvgToPng(RsvgHandle* handle, int target_w, int target_h, bool& alpha_channel)
{
	if (!handle || target_w <= 0 || target_h <= 0) return nullptr;

	// 获取 SVG 原始逻辑尺寸（像素或用户单位）
	RsvgDimensionData dim;
	rsvg_handle_get_dimensions(handle, &dim);

	if (dim.width <= 0 || dim.height <= 0) return nullptr;

	// 计算缩放（保持长宽比）
	double scale_x = double(target_w) / double(dim.width);
	double scale_y = double(target_h) / double(dim.height);
	double scale = scale_x; // 保持纵横比
	// 如果希望强制填满目标（拉伸），可以使用 scale_x 和 scale_y 分别缩放

	// 创建目标 cairo surface（ARGB32）
	cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, target_w, target_h);
	if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
		cairo_surface_destroy(surface);
		return nullptr;
	}
	cairo_surface_set_device_scale(surface, scale_x, scale_y); // 设置设备缩放为 1.0，防止 HiDPI 下 surface 大小异常
	cairo_t* cr = cairo_create(surface);
	if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		return nullptr;
	}
	if (!rsvg_handle_render_cairo(handle, cr)) {
		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		return nullptr;
	}
	cairo_surface_flush(surface);
	unsigned char* data = cairo_image_surface_get_data(surface);
	const int stride = cairo_image_surface_get_stride(surface);

	alpha_channel = true;
	HBITMAP hBitmap = SVGDataToHBITMAP(data, target_w, target_h, stride);

	// 清理
	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	return hBitmap;
}

SVGDLL_API void* GetSvgHandle(unsigned char* svg_data, DWORD svg_size, int nScale, int& svg_width, int& svg_height, bool& alpha_channel
	, HBITMAP& bitmap)
{
	RsvgHandle* handle = rsvg_handle_new_from_data((const guint8*)svg_data, svg_size, NULL);
	if (handle != nullptr)
	{
		RsvgDimensionData dimension_data;
		rsvg_handle_get_dimensions(handle, &dimension_data);
		svg_width = round(dimension_data.width * (nScale * 1.0 / 100.0));
		svg_height = round(dimension_data.height * (nScale * 1.0 / 100.0));
		bitmap = RenderSvgToPng(handle, svg_width, svg_height, alpha_channel);
	}
	return handle;
}

SVGDLL_API void GetSvgWidthHeight(void* handle, int& svg_width, int& svg_height)
{
	RsvgDimensionData dimension_data;
	rsvg_handle_get_dimensions((RsvgHandle*)handle, &dimension_data);
	svg_width = dimension_data.width;
	svg_height = dimension_data.height;
}

SVGDLL_API void GetSvgHbitmap(void* handle, int svg_width, int svg_height, bool& alpha_channel, HBITMAP& bitmap)
{
	if (handle == nullptr)
		return;
	RsvgDimensionData dimension_data;
	rsvg_handle_get_dimensions((RsvgHandle*)handle, &dimension_data);
	bitmap = RenderSvgToPng((RsvgHandle*)handle, svg_width, svg_height, alpha_channel);
}

SVGDLL_API void FreeSVGBitmap(HBITMAP& bitmap)
{
	if (bitmap)
	{
		::DeleteObject(bitmap);
		bitmap = nullptr;
	}
}

SVGDLL_API void FreeSVGHandle(void* handle)
{
	if (handle)
	{
		g_object_unref(handle);
	}
	handle = nullptr;
}
