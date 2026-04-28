#include "pch.h"
#include "UIResourceData.h"
#include "UIManager.h"
#include "UIMakup.h"

namespace FYUI
{
	namespace
	{
		class CScopedZipHandle
		{
		public:
			CScopedZipHandle(HZIP hz, bool bNeedClose)
				: m_hZip(hz), m_bNeedClose(bNeedClose)
			{
			}

			~CScopedZipHandle()
			{
				if (m_bNeedClose && m_hZip != NULL) {
					CloseZip(m_hZip);
				}
			}

			HZIP Get() const
			{
				return m_hZip;
			}

		private:
			HZIP m_hZip = NULL;
			bool m_bNeedClose = false;
		};

		HZIP OpenResourceZipHandle(const std::wstring& sZipFile, bool& bNeedClose)
		{
			bNeedClose = false;
			if (CPaintManagerUI::IsCachedResourceZip()) {
				return (HZIP)CPaintManagerUI::GetResourceZipHandle();
			}

			bNeedClose = true;
			std::wstring sFilePwd = CPaintManagerUI::GetResourceZipPwd();
#ifdef UNICODE
			char* pwd = w2a((wchar_t*)sFilePwd.c_str());
			HZIP hz = OpenZip(sZipFile.c_str(), pwd);
			if (pwd != NULL) {
				delete[] pwd;
			}
			return hz;
#else
			return OpenZip(sZipFile.c_str(), sFilePwd.c_str());
#endif
		}

	}

	BinaryDataLoadStatus LoadBinaryDataFromFilePath(std::wstring_view pstrFilePath, LPBYTE& pData, DWORD& dwSize)
	{
		pData = NULL;
		dwSize = 0;
		const std::wstring filePath(pstrFilePath);

		HANDLE hFile = ::CreateFile(
			filePath.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return BinaryDataLoadOpenFailed;
		}

		const DWORD dwFileSize = ::GetFileSize(hFile, NULL);
		if (dwFileSize == 0) {
			::CloseHandle(hFile);
			return BinaryDataLoadEmpty;
		}

		LPBYTE pBuffer = new BYTE[dwFileSize];
		DWORD dwRead = 0;
		const BOOL bReadOk = ::ReadFile(hFile, pBuffer, dwFileSize, &dwRead, NULL);
		::CloseHandle(hFile);
		if (!bReadOk || dwRead != dwFileSize) {
			delete[] pBuffer;
			return BinaryDataLoadReadFailed;
		}

		pData = pBuffer;
		dwSize = dwFileSize;
		return BinaryDataLoadSuccess;
	}

	BinaryDataLoadStatus LoadBinaryDataFromResourceZipEntry(std::wstring_view pstrZipRelativePath, LPBYTE& pData, DWORD& dwSize)
	{
		pData = NULL;
		dwSize = 0;

		std::wstring sZipFile = CPaintManagerUI::GetResourcePath();
		sZipFile += CPaintManagerUI::GetResourceZip();

		bool bNeedClose = false;
		CScopedZipHandle zipHandle(OpenResourceZipHandle(sZipFile, bNeedClose), bNeedClose);
		if (zipHandle.Get() == NULL) {
			return BinaryDataLoadZipOpenFailed;
		}

		ZIPENTRY ze = {};
		int iIndex = 0;
		std::wstring sKey = std::wstring(pstrZipRelativePath);
		StringUtil::ReplaceAll(sKey, L"\\", L"/");
		if (FindZipItem(zipHandle.Get(), sKey.c_str(), true, &iIndex, &ze) != 0) {
			return BinaryDataLoadZipEntryNotFound;
		}

		if (ze.unc_size == 0) {
			return BinaryDataLoadEmpty;
		}

		LPBYTE pBuffer = new BYTE[ze.unc_size];
		const int nUnzipResult = UnzipItem(zipHandle.Get(), iIndex, pBuffer, ze.unc_size);
		if (nUnzipResult != 0x00000000 && nUnzipResult != 0x00000600) {
			delete[] pBuffer;
			return BinaryDataLoadUnzipFailed;
		}

		pData = pBuffer;
		dwSize = ze.unc_size;
		return BinaryDataLoadSuccess;
	}

	BinaryDataLoadStatus LoadBinaryDataFromConfiguredPath(
		std::wstring_view pstrRelativePath,
		LPBYTE& pData,
		DWORD& dwSize,
		bool bAllowRawPathFallback)
	{
		pData = NULL;
		dwSize = 0;

		if (CPaintManagerUI::GetResourceZip().empty()) {
			std::wstring sFile = CPaintManagerUI::GetResourcePath();
			sFile += pstrRelativePath;
			BinaryDataLoadStatus status = LoadBinaryDataFromFilePath(sFile.c_str(), pData, dwSize);
			if (status == BinaryDataLoadSuccess || !bAllowRawPathFallback) {
				return status;
			}
		}
		else {
			BinaryDataLoadStatus status = LoadBinaryDataFromResourceZipEntry(pstrRelativePath, pData, dwSize);
			if (status == BinaryDataLoadSuccess || !bAllowRawPathFallback) {
				return status;
			}
		}

		return LoadBinaryDataFromFilePath(pstrRelativePath, pData, dwSize);
	}

	BinaryDataLoadStatus LoadBinaryDataFromDllResource(STRINGorID source, std::wstring_view type, HINSTANCE instance, LPBYTE& pData, DWORD& dwSize)
	{
		pData = NULL;
		dwSize = 0;

		HINSTANCE hResourceInstance = instance != NULL ? instance : CPaintManagerUI::GetResourceDll();
		const std::wstring typeStorage(type);
		HRSRC hResource = ::FindResource(hResourceInstance, source.c_str(), typeStorage.empty() ? nullptr : typeStorage.c_str());
		if (hResource == NULL) {
			return BinaryDataLoadResourceNotFound;
		}

		HGLOBAL hGlobal = ::LoadResource(hResourceInstance, hResource);
		if (hGlobal == NULL) {
			::FreeResource(hResource);
			return BinaryDataLoadResourceLoadFailed;
		}

		const DWORD dwResourceSize = ::SizeofResource(hResourceInstance, hResource);
		if (dwResourceSize == 0) {
			::FreeResource(hGlobal);
			return BinaryDataLoadResourceEmpty;
		}

		LPBYTE pBuffer = new BYTE[dwResourceSize];
		::CopyMemory(pBuffer, (LPBYTE)::LockResource(hGlobal), dwResourceSize);
		::FreeResource(hGlobal);

		pData = pBuffer;
		dwSize = dwResourceSize;
		return BinaryDataLoadSuccess;
	}

	bool LoadBinaryResourceData(
		STRINGorID source,
		LPBYTE& pData,
		DWORD& dwSize,
		std::wstring_view type,
		HINSTANCE instance,
		bool bAllowRawPathFallback)
	{
		pData = NULL;
		dwSize = 0;

		if (type.empty() && source.IsString()) {
			if (LoadBinaryDataFromConfiguredPath(source.view(), pData, dwSize, bAllowRawPathFallback) == BinaryDataLoadSuccess) {
				return true;
			}
		}
		else if (LoadBinaryDataFromDllResource(source, type, instance, pData, dwSize) == BinaryDataLoadSuccess) {
			return true;
		}
		return false;
	}

	bool LoadMarkupDocument(
		CMarkup& xml,
		STRINGorID source,
		std::wstring_view type,
		HINSTANCE instance,
		bool bAllowRawPathFallback,
		DWORD dwMaxSize)
	{
		if (source.IsString() && !source.view().empty() && source.view().front() == _T('<')) {
			return xml.Load(source.c_str());
		}

		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		if (!LoadBinaryResourceData(source, pData, dwSize, type, instance, bAllowRawPathFallback)) {
			return false;
		}
		if (dwSize > dwMaxSize) {
			delete[] pData;
			return false;
		}

		const bool bLoadOk = xml.LoadFromMem(pData, dwSize);
		delete[] pData;
		return bLoadOk;
	}
}


