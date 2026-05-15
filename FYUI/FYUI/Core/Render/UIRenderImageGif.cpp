#include "pch.h"
#include "UIRenderImageCodecInternal.h"

#include <algorithm>
#include <wincodec.h>
#include <wrl/client.h>

namespace FYUI
{
	namespace
	{
		using Microsoft::WRL::ComPtr;

		enum GifDisposalMethod
		{
			GifDisposal_Unspecified = 0,
			GifDisposal_None = 1,
			GifDisposal_Background = 2,
			GifDisposal_Previous = 3,
		};

		struct GifFrameInfo
		{
			UINT left = 0;
			UINT top = 0;
			UINT width = 0;
			UINT height = 0;
			UINT delayMs = 100;
			UINT disposal = GifDisposal_None;
		};

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

		HBITMAP CreateGifTopDownDibInternal(int width, int height, BYTE** ppDibBits)
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

		void FreeLocalGifFrames(std::vector<GifAnimationFrameBitmapInternal>& frames)
		{
			for (GifAnimationFrameBitmapInternal& frame : frames) {
				if (frame.hBitmap != nullptr) {
					::DeleteObject(frame.hBitmap);
					frame.hBitmap = nullptr;
				}
			}
			frames.clear();
		}

		bool GetMetadataUInt32(IWICMetadataQueryReader* reader, LPCWSTR name, UINT& value)
		{
			if (reader == nullptr || name == nullptr) {
				return false;
			}

			PROPVARIANT prop;
			::PropVariantInit(&prop);
			HRESULT hr = reader->GetMetadataByName(name, &prop);
			if (FAILED(hr)) {
				::PropVariantClear(&prop);
				return false;
			}

			bool ok = true;
			switch (prop.vt) {
			case VT_UI1:
				value = prop.bVal;
				break;
			case VT_UI2:
				value = prop.uiVal;
				break;
			case VT_UI4:
				value = prop.ulVal;
				break;
			case VT_UINT:
				value = prop.uintVal;
				break;
			case VT_I4:
				value = prop.lVal < 0 ? 0 : static_cast<UINT>(prop.lVal);
				break;
			default:
				ok = false;
				break;
			}

			::PropVariantClear(&prop);
			return ok;
		}

		bool ReadGifLogicalScreenSize(IWICBitmapDecoder* decoder, SIZE& canvasSize)
		{
			canvasSize.cx = 0;
			canvasSize.cy = 0;
			if (decoder == nullptr) {
				return false;
			}

			ComPtr<IWICMetadataQueryReader> reader;
			if (SUCCEEDED(decoder->GetMetadataQueryReader(reader.GetAddressOf())) && reader) {
				UINT width = 0;
				UINT height = 0;
				if (GetMetadataUInt32(reader.Get(), L"/logscrdesc/Width", width) &&
					GetMetadataUInt32(reader.Get(), L"/logscrdesc/Height", height) &&
					width > 0 && height > 0) {
					canvasSize.cx = static_cast<LONG>(width);
					canvasSize.cy = static_cast<LONG>(height);
					return true;
				}
			}
			return false;
		}

		GifFrameInfo ReadGifFrameInfo(IWICBitmapFrameDecode* frame)
		{
			GifFrameInfo info;
			if (frame == nullptr) {
				return info;
			}

			UINT width = 0;
			UINT height = 0;
			if (SUCCEEDED(frame->GetSize(&width, &height))) {
				info.width = width;
				info.height = height;
			}

			ComPtr<IWICMetadataQueryReader> reader;
			if (FAILED(frame->GetMetadataQueryReader(reader.GetAddressOf())) || !reader) {
				return info;
			}

			UINT value = 0;
			if (GetMetadataUInt32(reader.Get(), L"/imgdesc/Left", value)) {
				info.left = value;
			}
			if (GetMetadataUInt32(reader.Get(), L"/imgdesc/Top", value)) {
				info.top = value;
			}
			if (GetMetadataUInt32(reader.Get(), L"/grctlext/Delay", value)) {
				info.delayMs = value == 0 ? 100U : value * 10U;
			}
			if (GetMetadataUInt32(reader.Get(), L"/grctlext/Disposal", value)) {
				info.disposal = value;
			}
			return info;
		}

		bool ConvertGifFramePixels(IWICImagingFactory* wicFactory, IWICBitmapFrameDecode* frame, std::vector<BYTE>& pixels, UINT width, UINT height)
		{
			if (wicFactory == nullptr || frame == nullptr || width == 0 || height == 0) {
				return false;
			}

			ComPtr<IWICFormatConverter> converter;
			HRESULT hr = wicFactory->CreateFormatConverter(converter.GetAddressOf());
			if (FAILED(hr) || !converter) {
				return false;
			}

			hr = converter->Initialize(
				frame,
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				nullptr,
				0.0,
				WICBitmapPaletteTypeMedianCut);
			if (FAILED(hr)) {
				return false;
			}

			const UINT stride = width * 4;
			const UINT bufferSize = stride * height;
			pixels.assign(bufferSize, 0);
			hr = converter->CopyPixels(nullptr, stride, bufferSize, pixels.data());
			return SUCCEEDED(hr);
		}

		void ClearCanvasRect(std::vector<BYTE>& canvas, UINT canvasWidth, UINT canvasHeight, const RECT& rc)
		{
			if (canvas.empty() || canvasWidth == 0 || canvasHeight == 0) {
				return;
			}

			const LONG left = rc.left < 0 ? 0 : rc.left;
			const LONG top = rc.top < 0 ? 0 : rc.top;
			const LONG rightLimit = static_cast<LONG>(canvasWidth);
			const LONG bottomLimit = static_cast<LONG>(canvasHeight);
			const LONG right = rc.right < rightLimit ? rc.right : rightLimit;
			const LONG bottom = rc.bottom < bottomLimit ? rc.bottom : bottomLimit;
			if (right <= left || bottom <= top) {
				return;
			}

			for (LONG y = top; y < bottom; ++y) {
				BYTE* row = canvas.data() + (static_cast<size_t>(y) * canvasWidth + left) * 4;
				ZeroMemory(row, static_cast<size_t>(right - left) * 4);
			}
		}

		void BlendPremultipliedPixel(BYTE* dst, const BYTE* src)
		{
			const UINT srcA = src[3];
			if (srcA == 0) {
				return;
			}
			if (srcA == 255) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = 255;
				return;
			}

			const UINT invA = 255 - srcA;
			dst[0] = static_cast<BYTE>(src[0] + (dst[0] * invA + 127) / 255);
			dst[1] = static_cast<BYTE>(src[1] + (dst[1] * invA + 127) / 255);
			dst[2] = static_cast<BYTE>(src[2] + (dst[2] * invA + 127) / 255);
			dst[3] = static_cast<BYTE>(srcA + (dst[3] * invA + 127) / 255);
		}

		bool BlendFrameToCanvas(std::vector<BYTE>& canvas, UINT canvasWidth, UINT canvasHeight, const std::vector<BYTE>& framePixels, const GifFrameInfo& info)
		{
			if (canvas.empty() || framePixels.empty() || canvasWidth == 0 || canvasHeight == 0 || info.width == 0 || info.height == 0) {
				return false;
			}

			const UINT right = (std::min)(canvasWidth, info.left + info.width);
			const UINT bottom = (std::min)(canvasHeight, info.top + info.height);
			if (right <= info.left || bottom <= info.top) {
				return false;
			}

			for (UINT y = info.top; y < bottom; ++y) {
				const UINT frameY = y - info.top;
				for (UINT x = info.left; x < right; ++x) {
					const UINT frameX = x - info.left;
					BYTE* dst = canvas.data() + (static_cast<size_t>(y) * canvasWidth + x) * 4;
					const BYTE* src = framePixels.data() + (static_cast<size_t>(frameY) * info.width + frameX) * 4;
					BlendPremultipliedPixel(dst, src);
				}
			}
			return true;
		}

		bool CopyCanvasToBitmap(const std::vector<BYTE>& canvas, UINT width, UINT height, HBITMAP& hBitmap)
		{
			hBitmap = nullptr;
			BYTE* pDibBits = nullptr;
			HBITMAP hLocalBitmap = CreateGifTopDownDibInternal(static_cast<int>(width), static_cast<int>(height), &pDibBits);
			if (hLocalBitmap == NULL || pDibBits == nullptr) {
				if (hLocalBitmap != NULL) {
					::DeleteObject(hLocalBitmap);
				}
				return false;
			}

			CopyMemory(pDibBits, canvas.data(), canvas.size());
			hBitmap = hLocalBitmap;
			return true;
		}

		bool CanvasHasAlpha(const std::vector<BYTE>& canvas)
		{
			for (size_t i = 3; i < canvas.size(); i += 4) {
				if (canvas[i] < 255) {
					return true;
				}
			}
			return false;
		}
	}

	bool DecodeGifAnimationFromMemoryInternal(
		const void* pData,
		DWORD dwSize,
		DWORD mask,
		std::vector<GifAnimationFrameBitmapInternal>& frames,
		SIZE& canvasSize)
	{
		frames.clear();
		canvasSize.cx = 0;
		canvasSize.cy = 0;

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
		hr = stream->InitializeFromMemory(const_cast<WICInProcPointer>(reinterpret_cast<const BYTE*>(pData)), dwSize);
		if (FAILED(hr)) {
			return false;
		}

		ComPtr<IWICBitmapDecoder> decoder;
		hr = wicFactory->CreateDecoderFromStream(stream.Get(), nullptr, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf());
		if (FAILED(hr) || !decoder) {
			return false;
		}

		GUID containerFormat = {};
		if (FAILED(decoder->GetContainerFormat(&containerFormat)) || !::IsEqualGUID(containerFormat, GUID_ContainerFormatGif)) {
			return false;
		}

		UINT frameCount = 0;
		if (FAILED(decoder->GetFrameCount(&frameCount)) || frameCount == 0) {
			return false;
		}

		SIZE logicalSize = { 0, 0 };
		ReadGifLogicalScreenSize(decoder.Get(), logicalSize);
		if (logicalSize.cx <= 0 || logicalSize.cy <= 0) {
			ComPtr<IWICBitmapFrameDecode> firstFrame;
			if (FAILED(decoder->GetFrame(0, firstFrame.GetAddressOf())) || !firstFrame) {
				return false;
			}
			UINT firstWidth = 0;
			UINT firstHeight = 0;
			if (FAILED(firstFrame->GetSize(&firstWidth, &firstHeight)) || firstWidth == 0 || firstHeight == 0) {
				return false;
			}
			logicalSize.cx = static_cast<LONG>(firstWidth);
			logicalSize.cy = static_cast<LONG>(firstHeight);
		}

		const UINT canvasWidth = static_cast<UINT>(logicalSize.cx);
		const UINT canvasHeight = static_cast<UINT>(logicalSize.cy);
		std::vector<BYTE> canvas(static_cast<size_t>(canvasWidth) * canvasHeight * 4, 0);
		std::vector<BYTE> restoreCanvas;
		RECT previousRect = { 0, 0, 0, 0 };
		UINT previousDisposal = GifDisposal_None;

		for (UINT frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
			if (previousDisposal == GifDisposal_Background) {
				ClearCanvasRect(canvas, canvasWidth, canvasHeight, previousRect);
			}
			else if (previousDisposal == GifDisposal_Previous && !restoreCanvas.empty()) {
				canvas = restoreCanvas;
			}
			restoreCanvas.clear();

			ComPtr<IWICBitmapFrameDecode> frame;
			if (FAILED(decoder->GetFrame(frameIndex, frame.GetAddressOf())) || !frame) {
				FreeLocalGifFrames(frames);
				return false;
			}

			GifFrameInfo info = ReadGifFrameInfo(frame.Get());
			if (info.width == 0 || info.height == 0) {
				FreeLocalGifFrames(frames);
				return false;
			}

			std::vector<BYTE> framePixels;
			if (!ConvertGifFramePixels(wicFactory.Get(), frame.Get(), framePixels, info.width, info.height)) {
				FreeLocalGifFrames(frames);
				return false;
			}

			if (info.disposal == GifDisposal_Previous) {
				restoreCanvas = canvas;
			}

			BlendFrameToCanvas(canvas, canvasWidth, canvasHeight, framePixels, info);

			if (mask != 0) {
				const size_t pixelCount = canvas.size() / 4;
				for (size_t i = 0; i < pixelCount; ++i) {
					BYTE* pixel = canvas.data() + i * 4;
					if (*reinterpret_cast<DWORD*>(pixel) == mask) {
						pixel[0] = 0;
						pixel[1] = 0;
						pixel[2] = 0;
						pixel[3] = 0;
					}
				}
			}

			HBITMAP hFrameBitmap = nullptr;
			if (!CopyCanvasToBitmap(canvas, canvasWidth, canvasHeight, hFrameBitmap)) {
				FreeLocalGifFrames(frames);
				return false;
			}

			GifAnimationFrameBitmapInternal outFrame;
			outFrame.hBitmap = hFrameBitmap;
			outFrame.delayMs = info.delayMs == 0 ? 100U : info.delayMs;
			outFrame.hasAlpha = CanvasHasAlpha(canvas);
			frames.push_back(outFrame);

			previousDisposal = info.disposal;
			previousRect.left = static_cast<LONG>(info.left);
			previousRect.top = static_cast<LONG>(info.top);
			previousRect.right = static_cast<LONG>((std::min)(canvasWidth, info.left + info.width));
			previousRect.bottom = static_cast<LONG>((std::min)(canvasHeight, info.top + info.height));
		}

		canvasSize = logicalSize;
		return !frames.empty();
	}

	void FreeGifAnimationFramesInternal(std::vector<GifAnimationFrameBitmapInternal>& frames)
	{
		FreeLocalGifFrames(frames);
	}
}
