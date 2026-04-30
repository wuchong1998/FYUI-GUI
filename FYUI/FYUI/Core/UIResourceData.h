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

	/**
	 * @brief 加载Binary数据From文件路径
	 * @details 用于加载Binary数据From文件路径。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pstrFilePath [in] 文件路径字符串
	 * @param pData [in,out] 数据对象
	 * @param dwSize [in,out] 尺寸数值
	 * @return BinaryDataLoadStatus 返回 加载Binary数据From文件路径 的结果
	 */
	FYUI_API BinaryDataLoadStatus LoadBinaryDataFromFilePath(
		std::wstring_view pstrFilePath,
		LPBYTE& pData,
		DWORD& dwSize);

	/**
	 * @brief 加载Binary数据From资源压缩包Entry
	 * @details 用于加载Binary数据From资源压缩包Entry。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pstrZipRelativePath [in] 压缩包Relative路径字符串
	 * @param pData [in,out] 数据对象
	 * @param dwSize [in,out] 尺寸数值
	 * @return BinaryDataLoadStatus 返回 加载Binary数据From资源压缩包Entry 的结果
	 */
	FYUI_API BinaryDataLoadStatus LoadBinaryDataFromResourceZipEntry(
		std::wstring_view pstrZipRelativePath,
		LPBYTE& pData,
		DWORD& dwSize);

	/**
	 * @brief 加载Binary数据FromConfigured路径
	 * @details 用于加载Binary数据FromConfigured路径。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pstrRelativePath [in] Relative路径字符串
	 * @param pData [in,out] 数据对象
	 * @param dwSize [in,out] 尺寸数值
	 * @param bAllowRawPathFallback [in] 是否AllowRaw路径Fallback
	 * @return BinaryDataLoadStatus 返回 加载Binary数据FromConfigured路径 的结果
	 */
	FYUI_API BinaryDataLoadStatus LoadBinaryDataFromConfiguredPath(
		std::wstring_view pstrRelativePath,
		LPBYTE& pData,
		DWORD& dwSize,
		bool bAllowRawPathFallback = true);

	/**
	 * @brief 加载Binary数据From动态库资源
	 * @details 用于加载Binary数据From动态库资源。具体行为由当前对象状态以及传入参数共同决定。
	 * @param source [in] 来源参数
	 * @param type [in] 类型参数
	 * @param instance [in] 模块实例句柄
	 * @param pData [in,out] 数据对象
	 * @param dwSize [in,out] 尺寸数值
	 * @return BinaryDataLoadStatus 返回 加载Binary数据From动态库资源 的结果
	 */
	FYUI_API BinaryDataLoadStatus LoadBinaryDataFromDllResource(
		STRINGorID source,
		std::wstring_view type,
		HINSTANCE instance,
		LPBYTE& pData,
		DWORD& dwSize);

	/**
	 * @brief 加载Binary资源数据
	 * @details 用于加载Binary资源数据。具体行为由当前对象状态以及传入参数共同决定。
	 * @param source [in] 来源参数
	 * @param pData [in,out] 数据对象
	 * @param dwSize [in,out] 尺寸数值
	 * @param type [in] 类型参数
	 * @param instance [in] 模块实例句柄
	 * @param bAllowRawPathFallback [in] 是否AllowRaw路径Fallback
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	FYUI_API bool LoadBinaryResourceData(
		STRINGorID source,
		LPBYTE& pData,
		DWORD& dwSize,
		std::wstring_view type = {},
		HINSTANCE instance = NULL,
		bool bAllowRawPathFallback = true);

	/**
	 * @brief 加载标记文档Document
	 * @details 用于加载标记文档Document。具体行为由当前对象状态以及传入参数共同决定。
	 * @param xml [in,out] xml参数
	 * @param source [in] 来源参数
	 * @param type [in] 类型参数
	 * @param instance [in] 模块实例句柄
	 * @param bAllowRawPathFallback [in] 是否AllowRaw路径Fallback
	 * @param dwMaxSize [in] 最大尺寸数值
	 * @return bool 操作成功返回 true，否则返回 false
	 */
	FYUI_API bool LoadMarkupDocument(
		CMarkup& xml,
		STRINGorID source,
		std::wstring_view type = {},
		HINSTANCE instance = NULL,
		bool bAllowRawPathFallback = true,
		DWORD dwMaxSize = 4096 * 1024);
}
