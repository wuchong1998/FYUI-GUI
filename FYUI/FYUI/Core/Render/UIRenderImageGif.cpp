#include "pch.h"
#include "UIRenderImageCodecInternal.h"

#include <algorithm>
#include <cstring>
#include <vector>
#include <wincodec.h>
#include <wrl/client.h>

namespace FYUI
{
	namespace
	{
		using Microsoft::WRL::ComPtr;

		HBITMAP CreateGifCanvasDibInternal(int width, int height, BYTE** ppDibBits)
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

		bool ReadUInt16Metadata(IWICMetadataQueryReader* pReader, const wchar_t* pName, UINT& outValue)
		{
			if (pReader == nullptr || pName == nullptr) {
				return false;
			}

			PROPVARIANT pv;
			::PropVariantInit(&pv);
			HRESULT hr = pReader->GetMetadataByName(pName, &pv);
			bool bOk = false;
			if (SUCCEEDED(hr)) {
				if (pv.vt == VT_UI2) {
					outValue = pv.uiVal;
					bOk = true;
				}
				else if (pv.vt == VT_UI4) {
					outValue = pv.uintVal;
					bOk = true;
				}
			}
			::PropVariantClear(&pv);
			return bOk;
		}

		bool ReadUInt8Metadata(IWICMetadataQueryReader* pReader, const wchar_t* pName, UINT& outValue)
		{
			if (pReader == nullptr || pName == nullptr) {
				return false;
			}

			PROPVARIANT pv;
			::PropVariantInit(&pv);
			HRESULT hr = pReader->GetMetadataByName(pName, &pv);
			bool bOk = false;
			if (SUCCEEDED(hr) && pv.vt == VT_UI1) {
				outValue = pv.bVal;
				bOk = true;
			}
			::PropVariantClear(&pv);
			return bOk;
		}

		void ClearCanvasRectInternal(BYTE* pCanvas, UINT canvasWidth, UINT canvasHeight,
			UINT left, UINT top, UINT width, UINT height)
		{
			if (pCanvas == nullptr || canvasWidth == 0 || canvasHeight == 0) {
				return;
			}

			const UINT clipLeft = (std::min<UINT>)(left, canvasWidth);
			const UINT clipRight = (std::min<UINT>)(left + width, canvasWidth);
			const UINT clipTop = (std::min<UINT>)(top, canvasHeight);
			const UINT clipBottom = (std::min<UINT>)(top + height, canvasHeight);
			if (clipRight <= clipLeft || clipBottom <= clipTop) {
				return;
			}

			const size_t stride = static_cast<size_t>(canvasWidth) * 4;
			for (UINT y = clipTop; y < clipBottom; ++y) {
				BYTE* pRow = pCanvas + static_cast<size_t>(y) * stride;
				std::memset(pRow + clipLeft * 4, 0, static_cast<size_t>(clipRight - clipLeft) * 4);
			}
		}

		void BlitFrameOntoCanvasInternal(BYTE* pCanvas, UINT canvasWidth, UINT canvasHeight,
			const BYTE* pFrame, UINT frameWidth, UINT frameHeight,
			UINT offsetLeft, UINT offsetTop, bool& bAlphaSeen)
		{
			if (pCanvas == nullptr || pFrame == nullptr || frameWidth == 0 || frameHeight == 0) {
				return;
			}

			const size_t canvasStride = static_cast<size_t>(canvasWidth) * 4;
			const size_t frameStride = static_cast<size_t>(frameWidth) * 4;
			for (UINT y = 0; y < frameHeight; ++y) {
				const UINT canvasY = offsetTop + y;
				if (canvasY >= canvasHeight) {
					break;
				}

				BYTE* pDstRow = pCanvas + static_cast<size_t>(canvasY) * canvasStride;
				const BYTE* pSrcRow = pFrame + static_cast<size_t>(y) * frameStride;
				for (UINT x = 0; x < frameWidth; ++x) {
					const UINT canvasX = offsetLeft + x;
					if (canvasX >= canvasWidth) {
						break;
					}

					BYTE* pDst = pDstRow + static_cast<size_t>(canvasX) * 4;
					const BYTE* pSrc = pSrcRow + static_cast<size_t>(x) * 4;
					const BYTE srcAlpha = pSrc[3];
					if (srcAlpha == 255) {
						pDst[0] = pSrc[0];
						pDst[1] = pSrc[1];
						pDst[2] = pSrc[2];
						pDst[3] = pSrc[3];
					}
					else if (srcAlpha == 0) {
						bAlphaSeen = bAlphaSeen || pDst[3] < 255;
					}
					else {
						// PBGRA source-over：dst = src + dst * (1 - srcAlpha/255)
						const UINT inv = 255 - srcAlpha;
						pDst[0] = static_cast<BYTE>(pSrc[0] + (pDst[0] * inv) / 255);
						pDst[1] = static_cast<BYTE>(pSrc[1] + (pDst[1] * inv) / 255);
						pDst[2] = static_cast<BYTE>(pSrc[2] + (pDst[2] * inv) / 255);
						pDst[3] = static_cast<BYTE>(srcAlpha + (pDst[3] * inv) / 255);
						bAlphaSeen = true;
					}
				}
			}
		}

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

	bool DecodeGifAnimationFromFileInternal(
		const wchar_t* pStrFilename,
		std::vector<GifAnimationFrameBitmapInternal>& frames,
		SIZE& canvasSize)
	{
		FreeGifAnimationFramesInternal(frames);
		canvasSize.cx = 0;
		canvasSize.cy = 0;

		if (pStrFilename == nullptr || *pStrFilename == L'\0') {
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

		ComPtr<IWICBitmapDecoder> decoder;
		hr = wicFactory->CreateDecoderFromFilename(
			pStrFilename,
			nullptr,
			GENERIC_READ,
			WICDecodeMetadataCacheOnLoad,
			decoder.GetAddressOf());
		if (FAILED(hr) || !decoder) {
			return false;
		}

		UINT frameCount = 0;
		if (FAILED(decoder->GetFrameCount(&frameCount)) || frameCount == 0) {
			return false;
		}

		UINT canvasWidth = 0;
		UINT canvasHeight = 0;
		ComPtr<IWICMetadataQueryReader> globalReader;
		if (SUCCEEDED(decoder->GetMetadataQueryReader(globalReader.GetAddressOf()))) {
			ReadUInt16Metadata(globalReader.Get(), L"/logscrdesc/Width", canvasWidth);
			ReadUInt16Metadata(globalReader.Get(), L"/logscrdesc/Height", canvasHeight);
		}

		if (canvasWidth == 0 || canvasHeight == 0) {
			ComPtr<IWICBitmapFrameDecode> probe;
			if (FAILED(decoder->GetFrame(0, probe.GetAddressOf())) || !probe) {
				return false;
			}
			probe->GetSize(&canvasWidth, &canvasHeight);
		}
		if (canvasWidth == 0 || canvasHeight == 0) {
			return false;
		}

		const size_t canvasStride = static_cast<size_t>(canvasWidth) * 4;
		const size_t canvasBytes = canvasStride * canvasHeight;
		std::vector<BYTE> canvas(canvasBytes, 0);
		std::vector<BYTE> previousCanvas;

		UINT prevDisposal = 0;
		UINT prevLeft = 0;
		UINT prevTop = 0;
		UINT prevWidth = 0;
		UINT prevHeight = 0;
		bool bAnyAlpha = false;

		frames.reserve(frameCount);
		for (UINT i = 0; i < frameCount; ++i) {
			ComPtr<IWICBitmapFrameDecode> frameDecode;
			hr = decoder->GetFrame(i, frameDecode.GetAddressOf());
			if (FAILED(hr) || !frameDecode) {
				FreeGifAnimationFramesInternal(frames);
				return false;
			}

			UINT frameWidth = 0;
			UINT frameHeight = 0;
			frameDecode->GetSize(&frameWidth, &frameHeight);
			if (frameWidth == 0 || frameHeight == 0) {
				FreeGifAnimationFramesInternal(frames);
				return false;
			}

			UINT frameLeft = 0;
			UINT frameTop = 0;
			UINT delayCenti = 10;
			UINT disposal = 0;
			ComPtr<IWICMetadataQueryReader> frameReader;
			if (SUCCEEDED(frameDecode->GetMetadataQueryReader(frameReader.GetAddressOf()))) {
				ReadUInt16Metadata(frameReader.Get(), L"/imgdesc/Left", frameLeft);
				ReadUInt16Metadata(frameReader.Get(), L"/imgdesc/Top", frameTop);
				ReadUInt16Metadata(frameReader.Get(), L"/grctlext/Delay", delayCenti);
				ReadUInt8Metadata(frameReader.Get(), L"/grctlext/Disposal", disposal);
			}

			// 处理上一帧 disposal
			if (i > 0) {
				if (prevDisposal == 2) {
					ClearCanvasRectInternal(canvas.data(), canvasWidth, canvasHeight,
						prevLeft, prevTop, prevWidth, prevHeight);
				}
				else if (prevDisposal == 3 && !previousCanvas.empty()) {
					canvas = previousCanvas;
				}
			}

			if (disposal == 3) {
				previousCanvas = canvas;
			}
			else {
				previousCanvas.clear();
			}

			ComPtr<IWICFormatConverter> converter;
			hr = wicFactory->CreateFormatConverter(converter.GetAddressOf());
			if (FAILED(hr) || !converter) {
				FreeGifAnimationFramesInternal(frames);
				return false;
			}
			hr = converter->Initialize(
				frameDecode.Get(),
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				nullptr,
				0.0,
				WICBitmapPaletteTypeMedianCut);
			if (FAILED(hr)) {
				FreeGifAnimationFramesInternal(frames);
				return false;
			}

			std::vector<BYTE> framePixels(static_cast<size_t>(frameWidth) * frameHeight * 4, 0);
			hr = converter->CopyPixels(
				nullptr,
				frameWidth * 4,
				static_cast<UINT>(framePixels.size()),
				framePixels.data());
			if (FAILED(hr)) {
				FreeGifAnimationFramesInternal(frames);
				return false;
			}

			BlitFrameOntoCanvasInternal(canvas.data(), canvasWidth, canvasHeight,
				framePixels.data(), frameWidth, frameHeight,
				frameLeft, frameTop, bAnyAlpha);

			BYTE* pDibBits = nullptr;
			HBITMAP hBitmap = CreateGifCanvasDibInternal(static_cast<int>(canvasWidth),
				static_cast<int>(canvasHeight), &pDibBits);
			if (hBitmap == NULL || pDibBits == nullptr) {
				if (hBitmap) ::DeleteObject(hBitmap);
				FreeGifAnimationFramesInternal(frames);
				return false;
			}

			std::memcpy(pDibBits, canvas.data(), canvasBytes);

			GifAnimationFrameBitmapInternal info;
			info.hBitmap = hBitmap;
			info.delayMs = (delayCenti == 0) ? 100U : delayCenti * 10U;
			info.hasAlpha = true;
			frames.push_back(info);

			prevDisposal = disposal;
			prevLeft = frameLeft;
			prevTop = frameTop;
			prevWidth = frameWidth;
			prevHeight = frameHeight;
		}

		(void)bAnyAlpha;
		canvasSize.cx = static_cast<LONG>(canvasWidth);
		canvasSize.cy = static_cast<LONG>(canvasHeight);
		return !frames.empty();
	}

	void FreeGifAnimationFramesInternal(std::vector<GifAnimationFrameBitmapInternal>& frames)
	{
		for (GifAnimationFrameBitmapInternal& frame : frames) {
			if (frame.hBitmap != nullptr) {
				::DeleteObject(frame.hBitmap);
				frame.hBitmap = nullptr;
			}
		}
		frames.clear();
	}
}
