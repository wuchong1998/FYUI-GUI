#pragma once

#include <windows.h>

#include <string>
#include <unordered_map>

namespace FYUI
{
	namespace RenderHtmlParseInternal
	{
		enum class HtmlTagType
		{
			Link,
			Bold,
			Color,
			Font,
			Italic,
			Paragraph,
			Selected,
			Underline,
			Raw
		};

		/**
		 * @brief 执行 TrimWideString 操作
		 * @details 用于执行 TrimWideString 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @return 返回对应的字符串内容
		 */
		std::wstring TrimWideString(const std::wstring& text);
		/**
		 * @brief 执行 ToLowerWideString 操作
		 * @details 用于执行 ToLowerWideString 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @return 返回对应的字符串内容
		 */
		std::wstring ToLowerWideString(std::wstring text);
		/**
		 * @brief 执行 TryParseInteger 操作
		 * @details 用于执行 TryParseInteger 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param value [in,out] 值参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool TryParseInteger(const std::wstring& text, int& value);
		/**
		 * @brief 执行 TryParseHexColor 操作
		 * @details 用于执行 TryParseHexColor 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param color [in,out] 颜色参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool TryParseHexColor(const std::wstring& text, DWORD& color);
		/**
		 * @brief 解析HTML 文本属性映射
		 * @details 用于解析HTML 文本属性映射。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param attributes [in,out] attributes参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ParseHtmlAttributeMap(const std::wstring& text, std::unordered_map<std::wstring, std::wstring>& attributes);
		/**
		 * @brief 执行 TryReadHtmlTag 操作
		 * @details 用于执行 TryReadHtmlTag 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param source [in] 来源参数
		 * @param index [in,out] 子项下标
		 * @param isClosing [in,out] 判断是否Closing参数
		 * @param tagType [in,out] 标记类型参数
		 * @param body [in,out] body参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool TryReadHtmlTag(const std::wstring& source, size_t& index, bool& isClosing, HtmlTagType& tagType, std::wstring& body);
		/**
		 * @brief 执行 TryReadHtmlInlineTagBody 操作
		 * @details 用于执行 TryReadHtmlInlineTagBody 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param source [in] 来源参数
		 * @param index [in] 子项下标
		 * @param expectedTagName [in] expected标记名称参数
		 * @param body [in,out] body参数
		 * @param nextIndex [in,out] next索引参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool TryReadHtmlInlineTagBody(const std::wstring& source, size_t index, wchar_t expectedTagName, std::wstring& body, size_t& nextIndex);
	}
}
