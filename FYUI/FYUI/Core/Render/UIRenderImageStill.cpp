#include "pch.h"
#include "UIRenderImageCodecInternal.h"

#include <wincodec.h>
#include <wrl/client.h>

namespace FYUI
{
	namespace
	{
		using Microsoft::WRL::ComPtr;

		// 与 UIRenderImageCodec.cpp / UIRenderImageGif.cpp 一致：建立 32bpp top-down DIB 给 D2D pipeline 直接消费
		HBITMAP CreateStillTopDownDibInternal(int width, int height, BYTE** ppDibBits)
		{
			if (ppDibBits == nullptr || width <= 0 || height <= 0) {
				return NULL;
			}

			*ppDibBits = nullptr;
			BITMAPINFO bitmapInfo = {};
			bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bitmapInfo.bmiHeader.biWidth = width;
			bitmapInfo.bmiHeader.biHeight = -height;
			bitmapInfo.bmiHeader.biPlanes = 1;
			bitmapInfo.bmiHeader.biBitCount = 32;
			bitmapInfo.bmiHeader.biCompression = BI_RGB;
			bitmapInfo.bmiHeader.biSizeImage = static_cast<DWORD>(width) * static_cast<DWORD>(height) * 4;

			return ::CreateDIBSection(NULL, &bitmapInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(ppDibBits), NULL, 0);
		}

		void ApplyMaskAndDetectAlphaInternal(int width, int height, DWORD mask, BYTE* pDibBits, bool& bAlphaChannel)
		{
			if (pDibBits == nullptr || width <= 0 || height <= 0) {
				return;
			}

			const int pixelCount = width * height;
			for (int i = 0; i < pixelCount; ++i) {
				BYTE* pPixel = &pDibBits[i * 4];
				if (pPixel[3] < 255) {
					bAlphaChannel = true;
				}

				if (mask != 0 && *reinterpret_cast<DWORD*>(pPixel) == mask) {
					pPixel[0] = 0;
					pPixel[1] = 0;
					pPixel[2] = 0;
					pPixel[3] = 0;
					bAlphaChannel = true;
				}
			}
		}

		// WIC 的 CoInitializeEx 兜底：FYUI 主线程可能已经 STA 初始化过，这里只在尚未初始化时做一次 MTA 初始化
		struct WicComScope
		{
			WicComScope()
			{
				HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
				m_bNeedsUninit = SUCCEEDED(hr);
				if (hr == RPC_E_CHANGED_MODE) {
					m_bNeedsUninit = false;
				}
			}
			~WicComScope()
			{
				if (m_bNeedsUninit) {
					::CoUninitialize();
				}
			}
			bool m_bNeedsUninit = false;
		};
	}

	bool DecodeStillBitmapWithWicInternal(
		const BYTE* pData,
		DWORD dwSize,
		DWORD mask,
		HBITMAP& hBitmap,
		int& outWidth,
		int& outHeight,
		bool& bHasAlpha)
	{
		hBitmap = nullptr;
		outWidth = 0;
		outHeight = 0;
		bHasAlpha = false;

		if (pData == nullptr || dwSize == 0) {
			return false;
		}

		WicComScope comScope;

		ComPtr<IWICImagingFactory> wicFactory;
		HRESULT hr = ::CoCreateInstance(
			CLSID_WICImagingFactory,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(wicFactory.GetAddressOf()));
		if (FAILED(hr) || !wicFactory) {
			return false;
		}

		ComPtr<IWICStream> stream;
		hr = wicFactory->CreateStream(stream.GetAddressOf());
		if (FAILED(hr) || !stream) {
			return false;
		}
		hr = stream->InitializeFromMemory(const_cast<WICInProcPointer>(pData), dwSize);
		if (FAILED(hr)) {
			return false;
		}

		ComPtr<IWICBitmapDecoder> decoder;
		hr = wicFactory->CreateDecoderFromStream(
			stream.Get(),
			nullptr,
			WICDecodeMetadataCacheOnLoad,
			decoder.GetAddressOf());
		if (FAILED(hr) || !decoder) {
			// 数据不是 WIC 识别的格式（含 PSD / PIC / HDR / TGA 等 stb 才能处理的格式）
			return false;
		}

		UINT frameCount = 0;
		if (FAILED(decoder->GetFrameCount(&frameCount)) || frameCount == 0) {
			return false;
		}

		ComPtr<IWICBitmapFrameDecode> frame;
		hr = decoder->GetFrame(0, frame.GetAddressOf());
		if (FAILED(hr) || !frame) {
			return false;
		}

		UINT width = 0;
		UINT height = 0;
		if (FAILED(frame->GetSize(&width, &height)) || width == 0 || height == 0) {
			return false;
		}

		// 把任意源格式（含调色板、灰度、CMYK、16bpc 等）统一转换为 D2D 渲染期望的预乘 BGRA8
		ComPtr<IWICFormatConverter> converter;
		hr = wicFactory->CreateFormatConverter(converter.GetAddressOf());
		if (FAILED(hr) || !converter) {
			return false;
		}
		hr = converter->Initialize(
			frame.Get(),
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			nullptr,
			0.0,
			WICBitmapPaletteTypeMedianCut);
		if (FAILED(hr)) {
			return false;
		}

		BYTE* pDibBits = nullptr;
		HBITMAP hLocalBitmap = CreateStillTopDownDibInternal(
			static_cast<int>(width), static_cast<int>(height), &pDibBits);
		if (hLocalBitmap == NULL || pDibBits == nullptr) {
			if (hLocalBitmap) {
				::DeleteObject(hLocalBitmap);
			}
			return false;
		}

		const UINT stride = width * 4;
		const UINT bufferSize = stride * height;
		hr = converter->CopyPixels(nullptr, stride, bufferSize, pDibBits);
		if (FAILED(hr)) {
			::DeleteObject(hLocalBitmap);
			return false;
		}

		bool localHasAlpha = false;
		ApplyMaskAndDetectAlphaInternal(
			static_cast<int>(width), static_cast<int>(height), mask, pDibBits, localHasAlpha);

		hBitmap = hLocalBitmap;
		outWidth = static_cast<int>(width);
		outHeight = static_cast<int>(height);
		bHasAlpha = localHasAlpha;
		return true;
	}
}
