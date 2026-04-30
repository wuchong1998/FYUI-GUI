#pragma once

#include "UIRenderContext.h"

#include <string>
#include <string_view>

namespace FYUI
{
	class CPaintManagerUI;
	struct tagTFontInfo;
	typedef tagTFontInfo TFontInfo;

	/**
	 * @brief 执行 ResolveTextFallbackFontInfoInternal 操作
	 * @details 用于执行 ResolveTextFallbackFontInfoInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pManager [in] 管理器对象
	 * @param iFont [in] 字体值
	 * @return TFontInfo* 返回结果对象指针，失败时返回 nullptr
	 */
	TFontInfo* ResolveTextFallbackFontInfoInternal(CPaintManagerUI* pManager, int iFont);
	/**
	 * @brief 执行 ResolveTextFallbackFontFamilyInternal 操作
	 * @details 用于执行 ResolveTextFallbackFontFamilyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param fontFamily [in] 字体Family参数
	 * @return 返回对应的字符串内容
	 */
	std::wstring ResolveTextFallbackFontFamilyInternal(std::wstring_view fontFamily);
	/**
	 * @brief 执行 ResolveTextFallbackFontFamilyInternal 操作
	 * @details 用于执行 ResolveTextFallbackFontFamilyInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pFontInfo [in] 字体信息对象
	 * @return 返回对应的字符串内容
	 */
	std::wstring ResolveTextFallbackFontFamilyInternal(const TFontInfo* pFontInfo);
	/**
	 * @brief 执行 ResolveTextFallbackFontSizeInternal 操作
	 * @details 用于执行 ResolveTextFallbackFontSizeInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param fontSize [in] 字体尺寸参数
	 * @return float 返回对应的数值结果
	 */
	float ResolveTextFallbackFontSizeInternal(float fontSize);
	/**
	 * @brief 执行 ResolveTextColorInternal 操作
	 * @details 用于执行 ResolveTextColorInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pManager [in] 管理器对象
	 * @param dwColor [in] 颜色值
	 * @return DWORD 返回对应的数值结果
	 */
	DWORD ResolveTextColorInternal(CPaintManagerUI* pManager, DWORD dwColor);
	/**
	 * @brief 更新文本Fallback字体度量信息Internal
	 * @details 用于更新文本Fallback字体度量信息Internal。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 * @param pFontInfo [in] 字体信息对象
	 */
	void UpdateTextFallbackFontMetricsInternal(CPaintRenderContext& renderContext, TFontInfo* pFontInfo);
	/**
	 * @brief 执行 ToWideText 操作
	 * @details 用于执行 ToWideText 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param text [in] 文本内容
	 * @return 返回对应的字符串内容
	 */
	std::wstring ToWideText(std::wstring_view text);
	/**
	 * @brief 获取Utf16CodeUnitLength
	 * @details 用于获取Utf16CodeUnitLength。具体行为由当前对象状态以及传入参数共同决定。
	 * @param text [in] 文本内容
	 * @param index [in] 子项下标
	 * @return UINT32 返回 获取Utf16CodeUnitLength 的结果
	 */
	UINT32 GetUtf16CodeUnitLength(const std::wstring& text, size_t index);
	/**
	 * @brief 执行 NormalizePlainTextSharedContent 操作
	 * @details 用于执行 NormalizePlainTextSharedContent 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param sourceText [in] 来源文本参数
	 * @param normalizedText [in,out] normalized文本参数
	 */
	void NormalizePlainTextSharedContent(const std::wstring& sourceText, std::wstring& normalizedText);
	/**
	 * @brief 执行 BuildGdiplusFallbackTextInternal 操作
	 * @details 用于执行 BuildGdiplusFallbackTextInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param text [in] 文本内容
	 * @return 返回对应的字符串内容
	 */
	std::wstring BuildGdiplusFallbackTextInternal(std::wstring_view text);
}
