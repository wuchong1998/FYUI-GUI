#include "pch.h"
#include "UIRender.h"
#include "UIResourceData.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\Utils\stb_image.h"

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

		LPBYTE pImage = nullptr;
		std::wstring strFile = bitmap.IsString() ? std::wstring(bitmap.view()) : std::wstring(type);
		strFile = strFile.substr(strFile.find_last_of(L".") + 1);
		int nImageWidth = 1;
		int nImageHeight = 1;
		int n = 0;
		if (strFile != L"svg")
		{
			pImage = stbi_load_from_memory(pData, dwSize, &nImageWidth, &nImageHeight, &n, 4);
		}

		bool bAlphaChannel = false;
		HBITMAP hBitmap = nullptr;
		bool bSvg = false;
		TImageInfo* data = new TImageInfo;

		if (pImage == nullptr)
		{
#ifdef SVG
			GetSvgHandleFunc GetSvgHandle = (GetSvgHandleFunc)g_SDK->GetFunction("GetSvgHandle");
			if (GetSvgHandle == nullptr)
				return nullptr;
			void* pHandle = GetSvgHandle(pData, dwSize, nScale, nImageWidth, nImageHeight, bAlphaChannel, hBitmap);
			if (pHandle != nullptr)
			{
				data->pHandle = pHandle;
				data->nOriWidth = nImageWidth;
				data->nOriHeight = nImageHeight;
				data->fPresent = nScale * 1.0 / 100.0;
				bSvg = true;
			}
#endif
		}
		delete[] pData;
		if ((!pImage) && (!bSvg))
		{
			delete data;
			return NULL;
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


