#pragma once

#include "../Utils/Utils.h"

namespace FYUI
{
	class CMarkup;

	enum BinaryDataLoadStatus
	{
		BinaryDataLoadSuccess = 0,
		BinaryDataLoadOpenFailed,
		BinaryDataLoadEmpty,
		BinaryDataLoadReadFailed,
		BinaryDataLoadZipOpenFailed,
		BinaryDataLoadZipEntryNotFound,
		BinaryDataLoadUnzipFailed,
		BinaryDataLoadResourceNotFound,
		BinaryDataLoadResourceLoadFailed,
		BinaryDataLoadResourceEmpty,
	};

	FYUI_API BinaryDataLoadStatus LoadBinaryDataFromFilePath(
		std::wstring_view pstrFilePath,
		LPBYTE& pData,
		DWORD& dwSize);

	FYUI_API BinaryDataLoadStatus LoadBinaryDataFromResourceZipEntry(
		std::wstring_view pstrZipRelativePath,
		LPBYTE& pData,
		DWORD& dwSize);

	FYUI_API BinaryDataLoadStatus LoadBinaryDataFromConfiguredPath(
		std::wstring_view pstrRelativePath,
		LPBYTE& pData,
		DWORD& dwSize,
		bool bAllowRawPathFallback = true);

	FYUI_API BinaryDataLoadStatus LoadBinaryDataFromDllResource(
		STRINGorID source,
		std::wstring_view type,
		HINSTANCE instance,
		LPBYTE& pData,
		DWORD& dwSize);

	FYUI_API bool LoadBinaryResourceData(
		STRINGorID source,
		LPBYTE& pData,
		DWORD& dwSize,
		std::wstring_view type = {},
		HINSTANCE instance = NULL,
		bool bAllowRawPathFallback = true);

	FYUI_API bool LoadMarkupDocument(
		CMarkup& xml,
		STRINGorID source,
		std::wstring_view type = {},
		HINSTANCE instance = NULL,
		bool bAllowRawPathFallback = true,
		DWORD dwMaxSize = 4096 * 1024);
}
