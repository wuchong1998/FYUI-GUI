#include "pch.h"
#include "../UIRender.h"
#include "../UIResourceData.h"
#include "UIRenderImageCodecInternal.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\..\Utils\stb_image.h"
#include "src/webp/decode.h"
#include "src/webp/demux.h"

namespace FYUI
{
	namespace
	{
		HBITMAP CreateTopDownImageCodecDibInternal(int width, int height, BYTE** ppDibBits)
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
			bitmapInfo.bmiHeader.biSizeImage = width * height * 4;

			return ::CreateDIBSection(NULL, &bitmapInfo, DIB_RGB_COLORS, reinterpret_cast<void**>(ppDibBits), NULL, 0);
		}

		void CopyStbImageToPremultipliedBgraDibInternal(const BYTE* pImage, int width, int height, DWORD mask, BYTE* pDibBits, bool& bAlphaChannel)
		{
			if (pImage == nullptr || pDibBits == nullptr || width <= 0 || height <= 0) {
				return;
			}

			for (int i = 0; i < width * height; i++)
			{
				pDibBits[i * 4 + 3] = pImage[i * 4 + 3];
				if (pDibBits[i * 4 + 3] < 255)
				{
					pDibBits[i * 4] = (BYTE)(DWORD(pImage[i * 4 + 2]) * pImage[i * 4 + 3] / 255);
					pDibBits[i * 4 + 1] = (BYTE)(DWORD(pImage[i * 4 + 1]) * pImage[i * 4 + 3] / 255);
					pDibBits[i * 4 + 2] = (BYTE)(DWORD(pImage[i * 4]) * pImage[i * 4 + 3] / 255);
					bAlphaChannel = true;
				}
				else
				{
					pDibBits[i * 4] = pImage[i * 4 + 2];
					pDibBits[i * 4 + 1] = pImage[i * 4 + 1];
					pDibBits[i * 4 + 2] = pImage[i * 4];
				}

				if (*(DWORD*)(&pDibBits[i * 4]) == mask) {
					pDibBits[i * 4] = (BYTE)0;
					pDibBits[i * 4 + 1] = (BYTE)0;
					pDibBits[i * 4 + 2] = (BYTE)0;
					pDibBits[i * 4 + 3] = (BYTE)0;
					bAlphaChannel = true;
				}
			}
		}

		void ApplyMaskToPremultipliedBgraDibInternal(int width, int height, DWORD mask, BYTE* pDibBits, bool& bAlphaChannel)
		{
			if (pDibBits == nullptr || width <= 0 || height <= 0) {
				return;
			}

			const int pixelCount = width * height;
			for (int i = 0; i < pixelCount; ++i)
			{
				BYTE* pPixel = &pDibBits[i * 4];
				if (pPixel[3] < 255) {
					bAlphaChannel = true;
				}

				if (mask != 0 && *(DWORD*)pPixel == mask) {
					pPixel[0] = 0;
					pPixel[1] = 0;
					pPixel[2] = 0;
					pPixel[3] = 0;
					bAlphaChannel = true;
				}
			}
		}

		bool CreateBitmapFromPremultipliedBgraBufferInternal(
			const uint8_t* pBuffer,
			int width,
			int height,
			DWORD mask,
			HBITMAP& hBitmap,
			bool& bAlphaChannel)
		{
			hBitmap = nullptr;
			bAlphaChannel = false;
			if (pBuffer == nullptr || width <= 0 || height <= 0) {
				return false;
			}

			BYTE* pDibBits = nullptr;
			hBitmap = CreateTopDownImageCodecDibInternal(width, height, &pDibBits);
			if (hBitmap == nullptr || pDibBits == nullptr) {
				return false;
			}

			const size_t bufferSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
			memcpy(pDibBits, pBuffer, bufferSize);
			ApplyMaskToPremultipliedBgraDibInternal(width, height, mask, pDibBits, bAlphaChannel);
			return true;
		}

		TImageInfo* CreateWebpImageInfoFromDecodedBitmapInternal(
			HBITMAP hBitmap,
			int width,
			int height,
			bool bAlphaChannel)
		{
			if (hBitmap == nullptr || width <= 0 || height <= 0) {
				return nullptr;
			}

			TImageInfo* data = new TImageInfo;
			data->pBits = NULL;
			data->pSrcBits = NULL;
			data->hBitmap = hBitmap;
			data->nX = width;
			data->nY = height;
			data->nDestWidth = width;
			data->nDestHeight = height;
			data->nOriWidth = width;
			data->nOriHeight = height;
			data->fPresent = 1.0f;
			data->bAlpha = bAlphaChannel;
			return data;
		}

		TImageInfo* LoadImageInfoFromMemoryInternal(const BYTE* pData, DWORD dwSize, int nScale, DWORD mask)
		{
			if (pData == nullptr || dwSize == 0) {
				return NULL;
			}

			LPBYTE pImage = nullptr;
			int nImageWidth = 1;
			int nImageHeight = 1;
			int n = 0;
			pImage = stbi_load_from_memory(pData, static_cast<int>(dwSize), &nImageWidth, &nImageHeight, &n, 4);

			bool bAlphaChannel = false;
			HBITMAP hBitmap = nullptr;
			bool bSvg = false;
			TImageInfo* data = new TImageInfo;

			if (pImage == nullptr)
			{
#ifdef SVG
				GetSvgHandleFunc GetSvgHandle = (GetSvgHandleFunc)g_SDK->GetFunction("GetSvgHandle");
				if (GetSvgHandle != nullptr) {
					void* pHandle = GetSvgHandle(const_cast<BYTE*>(pData), dwSize, nScale, nImageWidth, nImageHeight, bAlphaChannel, hBitmap);
					if (pHandle != nullptr)
					{
						data->pHandle = pHandle;
						data->nOriWidth = nImageWidth;
						data->nOriHeight = nImageHeight;
						data->fPresent = nScale * 1.0f / 100.0f;
						bSvg = true;
					}
				}
#endif
			}

			if ((!pImage) && (!bSvg))
			{
				delete data;
				return LoadWebpStillImageInfoFromMemoryInternal(pData, dwSize, nScale, mask);
			}

			if (bSvg == false)
			{
				BYTE* pDibBits = NULL;
				hBitmap = CreateTopDownImageCodecDibInternal(nImageWidth, nImageHeight, &pDibBits);
				if (!hBitmap) {
					stbi_image_free(pImage);
					delete data;
					return NULL;
				}

				CopyStbImageToPremultipliedBgraDibInternal(pImage, nImageWidth, nImageHeight, mask, pDibBits, bAlphaChannel);
				stbi_image_free(pImage);
			}

			data->pBits = NULL;
			data->pSrcBits = NULL;
			data->hBitmap = hBitmap;
			data->nX = nImageWidth;
			data->nY = nImageHeight;
			data->nDestWidth = nImageWidth;
			data->nDestHeight = nImageHeight;
			data->bAlpha = bAlphaChannel;
			return data;
		}
	}

	TImageInfo* LoadWebpStillImageInfoFromMemoryInternal(const void* pData, DWORD dwSize, int nScale, DWORD mask)
	{
		(void)nScale;
		if (pData == nullptr || dwSize == 0) {
			return nullptr;
		}

		WebPBitstreamFeatures features = {};
		if (WebPGetFeatures(reinterpret_cast<const uint8_t*>(pData), static_cast<size_t>(dwSize), &features) != VP8_STATUS_OK) {
			return nullptr;
		}
		if (features.width <= 0 || features.height <= 0) {
			return nullptr;
		}

		BYTE* pDibBits = nullptr;
		HBITMAP hBitmap = CreateTopDownImageCodecDibInternal(features.width, features.height, &pDibBits);
		if (hBitmap == nullptr || pDibBits == nullptr) {
			return nullptr;
		}

		WebPDecoderConfig config = {};
		if (!WebPInitDecoderConfig(&config)) {
			::DeleteObject(hBitmap);
			return nullptr;
		}
		if (WebPGetFeatures(reinterpret_cast<const uint8_t*>(pData), static_cast<size_t>(dwSize), &config.input) != VP8_STATUS_OK) {
			::DeleteObject(hBitmap);
			return nullptr;
		}

		config.output.colorspace = MODE_bgrA;
		config.output.is_external_memory = 1;
		config.output.u.RGBA.rgba = pDibBits;
		config.output.u.RGBA.stride = features.width * 4;
		config.output.u.RGBA.size = static_cast<size_t>(features.width) * static_cast<size_t>(features.height) * 4;

		const VP8StatusCode status = WebPDecode(reinterpret_cast<const uint8_t*>(pData), static_cast<size_t>(dwSize), &config);
		WebPFreeDecBuffer(&config.output);
		if (status != VP8_STATUS_OK) {
			::DeleteObject(hBitmap);
			return nullptr;
		}

		bool bAlphaChannel = (features.has_alpha != 0);
		ApplyMaskToPremultipliedBgraDibInternal(features.width, features.height, mask, pDibBits, bAlphaChannel);
		return CreateWebpImageInfoFromDecodedBitmapInternal(hBitmap, features.width, features.height, bAlphaChannel);
	}

	bool DecodeWebpAnimationFromMemoryInternal(
		const void* pData,
		DWORD dwSize,
		DWORD mask,
		std::vector<WebpAnimationFrameBitmapInternal>& frames,
		SIZE& canvasSize)
	{
		frames.clear();
		canvasSize.cx = 0;
		canvasSize.cy = 0;
		if (pData == nullptr || dwSize == 0) {
			return false;
		}

		WebPData webpData;
		WebPDataInit(&webpData);
		webpData.bytes = reinterpret_cast<const uint8_t*>(pData);
		webpData.size = static_cast<size_t>(dwSize);

		WebPAnimDecoderOptions options = {};
		if (!WebPAnimDecoderOptionsInit(&options)) {
			return false;
		}
		options.color_mode = MODE_bgrA;
		options.use_threads = 1;

		WebPAnimDecoder* pDecoder = WebPAnimDecoderNew(&webpData, &options);
		if (pDecoder == nullptr) {
			return false;
		}

		bool bDecodeOk = false;
		WebPIterator frameIterator = {};
		bool bIteratorAcquired = false;
		bool bHasIterator = false;
		do {
			WebPAnimInfo animInfo = {};
			if (!WebPAnimDecoderGetInfo(pDecoder, &animInfo) ||
				animInfo.canvas_width == 0 ||
				animInfo.canvas_height == 0 ||
				animInfo.frame_count == 0) {
				break;
			}

			canvasSize.cx = static_cast<LONG>(animInfo.canvas_width);
			canvasSize.cy = static_cast<LONG>(animInfo.canvas_height);

			const WebPDemuxer* pDemuxer = WebPAnimDecoderGetDemuxer(pDecoder);
			if (pDemuxer != nullptr) {
				bHasIterator = WebPDemuxGetFrame(pDemuxer, 1, &frameIterator) != 0;
				bIteratorAcquired = bHasIterator;
			}

			while (WebPAnimDecoderHasMoreFrames(pDecoder)) {
				uint8_t* pFrameBuffer = nullptr;
				int nTimestampMs = 0;
				if (!WebPAnimDecoderGetNext(pDecoder, &pFrameBuffer, &nTimestampMs)) {
					FreeWebpAnimationFramesInternal(frames);
					break;
				}

				(void)nTimestampMs;
				WebpAnimationFrameBitmapInternal frame;
				frame.delayMs = 100U;
				if (bHasIterator) {
					frame.delayMs = frameIterator.duration > 0 ? static_cast<UINT>(frameIterator.duration) : 100U;
				}

				if (!CreateBitmapFromPremultipliedBgraBufferInternal(
					pFrameBuffer,
					static_cast<int>(animInfo.canvas_width),
					static_cast<int>(animInfo.canvas_height),
					mask,
					frame.hBitmap,
					frame.hasAlpha)) {
					FreeWebpAnimationFramesInternal(frames);
					break;
				}

				frames.push_back(frame);
				if (bHasIterator) {
					bHasIterator = WebPDemuxNextFrame(&frameIterator) != 0;
				}
			}

			bDecodeOk = !frames.empty();
		} while (false);

		if (bIteratorAcquired) {
			WebPDemuxReleaseIterator(&frameIterator);
		}
		WebPAnimDecoderDelete(pDecoder);

		if (!bDecodeOk) {
			FreeWebpAnimationFramesInternal(frames);
			canvasSize.cx = 0;
			canvasSize.cy = 0;
		}
		return bDecodeOk;
	}

	void FreeWebpAnimationFramesInternal(std::vector<WebpAnimationFrameBitmapInternal>& frames)
	{
		for (WebpAnimationFrameBitmapInternal& frame : frames) {
			if (frame.hBitmap != nullptr) {
				::DeleteObject(frame.hBitmap);
				frame.hBitmap = nullptr;
			}
		}
		frames.clear();
	}

	bool SaveHBITMAPToPNGAInfo(HBITMAP hBitmap, const wchar_t* filename)
	{
		auto GetEncoderClsid = [&](const WCHAR* format, CLSID* pClsid) -> int {
			UINT num = 0;
			UINT size = 0;
			Gdiplus::GetImageEncodersSize(&num, &size);
			if (size == 0) return -1;

			Gdiplus::ImageCodecInfo* pCodecInfo = (Gdiplus::ImageCodecInfo*)malloc(size);
			Gdiplus::GetImageEncoders(num, size, pCodecInfo);

			for (UINT i = 0; i < num; i++)
			{
				if (wcscmp(pCodecInfo[i].MimeType, format) == 0)
				{
					*pClsid = pCodecInfo[i].Clsid;
					free(pCodecInfo);
					return i;
				}
			}

			free(pCodecInfo);
			return -1;
		};

		Gdiplus::Bitmap bitmap(hBitmap, NULL);

		CLSID pngClsid;
		GetEncoderClsid(L"image/png", &pngClsid);

		Gdiplus::Status status = bitmap.Save(filename, &pngClsid, NULL);
		return (status == Gdiplus::Ok);
	}

	TImageInfo* CRenderEngine::LoadImage(STRINGorID bitmap, int nScale, std::wstring_view type, DWORD mask, HINSTANCE instance)
	{
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		if (!LoadBinaryResourceData(bitmap, pData, dwSize, type, instance))
		{
			return NULL;
		}

		TImageInfo* data = LoadImageInfoFromMemoryInternal(pData, dwSize, nScale, mask);
		delete[] pData;
		return data;
	}

	TImageInfo* CRenderEngine::LoadImage(std::wstring_view pStrImage, int nScale, std::wstring_view type, DWORD mask, HINSTANCE instance)
	{
		if (pStrImage.empty()) return NULL;

		std::wstring sStrPath(pStrImage);
		if (type.empty()) {
			sStrPath = CResourceManager::GetInstance()->GetImagePath(pStrImage);
			if (sStrPath.empty()) {
				sStrPath.assign(pStrImage);
			}
		}
		return LoadImage(STRINGorID(sStrPath.c_str()), nScale, type, mask, instance);
	}

	TImageInfo* CRenderEngine::LoadImage(UINT nID, int nScale, std::wstring_view type, DWORD mask, HINSTANCE instance)
	{
		return LoadImage(STRINGorID(nID), nScale, type, mask, instance);
	}

	TImageInfo* CRenderEngine::LoadImageFromMemory(const void* pData, DWORD dwSize, int nScale, DWORD mask)
	{
		return LoadImageInfoFromMemoryInternal(reinterpret_cast<const BYTE*>(pData), dwSize, nScale, mask);
	}

	TImageInfo* CRenderEngine::GdiplusLoadImage(STRINGorID bitmap, std::wstring_view type, DWORD mask, HINSTANCE instance)
	{
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		if (!LoadBinaryResourceData(bitmap, pData, dwSize, type, instance)) {
			return NULL;
		}

		bool bAlphaChannel = false;
		HBITMAP hBitmap = NULL;
		int x = 0;
		int y = 0;
		int n = 0;
		LPBYTE pImage = stbi_load_from_memory(pData, dwSize, &x, &y, &n, 4);
		if (pImage != NULL) {
			BYTE* pDibBits = NULL;
			hBitmap = CreateTopDownImageCodecDibInternal(x, y, &pDibBits);
			if (hBitmap != NULL) {
				CopyStbImageToPremultipliedBgraDibInternal(pImage, x, y, mask, pDibBits, bAlphaChannel);
			}
			stbi_image_free(pImage);
		}

		HGLOBAL hMem = ::GlobalAlloc(GMEM_FIXED, dwSize);
		BYTE* pMem = (BYTE*)::GlobalLock(hMem);
		memcpy(pMem, pData, dwSize);
		IStream* pStm = NULL;
		::CreateStreamOnHGlobal(hMem, TRUE, &pStm);

		Gdiplus::Image* pGdiplusImage = Gdiplus::Image::FromStream(pStm);
		if (!pGdiplusImage || pGdiplusImage->GetLastStatus() != Gdiplus::Ok)
		{
			pGdiplusImage = NULL;

			if (pStm) pStm->Release();
			::GlobalUnlock(hMem);
		}

		delete[] pData;
		pData = NULL;

		if (hBitmap == NULL && pGdiplusImage == NULL) {
			if (pStm) pStm->Release();
			return NULL;
		}

		TImageInfo* data = new TImageInfo;
		data->pBits = NULL;
		data->pSrcBits = NULL;
		data->pImage = pGdiplusImage;
		data->hBitmap = hBitmap;
		data->nX = x;
		data->nY = y;
		data->bAlpha = bAlphaChannel;
		if (pStm) pStm->Release();
		return data;
	}

	TImageInfo* CRenderEngine::GdiplusLoadImage(std::wstring_view pStrImage, std::wstring_view type, DWORD mask, HINSTANCE instance)
	{
		if (pStrImage.empty()) {
			return NULL;
		}

		std::wstring sStrPath(pStrImage);
		if (type.empty()) {
			sStrPath = CResourceManager::GetInstance()->GetImagePath(pStrImage);
			if (sStrPath.empty()) {
				sStrPath.assign(pStrImage);
			}
		}
		return GdiplusLoadImage(STRINGorID(sStrPath.c_str()), type, mask, instance);
	}
}


