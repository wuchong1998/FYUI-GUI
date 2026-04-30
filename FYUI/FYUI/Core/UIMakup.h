#pragma once

#include <array>
#include <string_view>

namespace FYUI
{

	enum
	{
		XMLFILE_ENCODING_UTF8 = 0,
		XMLFILE_ENCODING_UNICODE = 1,
		XMLFILE_ENCODING_ASNI = 2,
	};

	class CMarkup;
	class CMarkupNode;


	class FYUI_API CMarkup
	{
		friend class CMarkupNode;
	public:
		/**
		 * @brief 构造 CMarkup 对象
		 * @details 用于构造 CMarkup 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrXML [in] XML字符串
		 */
		CMarkup(std::wstring_view pstrXML = {});
		/**
		 * @brief 析构 CMarkup 对象
		 * @details 用于析构 CMarkup 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CMarkup();

		/**
		 * @brief 执行 Load 操作
		 * @details 用于执行 Load 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrXML [in] XML字符串
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Load(std::wstring_view pstrXML);
		/**
		 * @brief 加载FromMem
		 * @details 用于加载FromMem。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pByte [in] Byte对象
		 * @param dwSize [in] 尺寸数值
		 * @param encoding [in] encoding参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool LoadFromMem(BYTE* pByte, DWORD dwSize, int encoding = XMLFILE_ENCODING_UTF8);
		/**
		 * @brief 加载From文件
		 * @details 用于加载From文件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrFilename [in] Filename字符串
		 * @param encoding [in] encoding参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool LoadFromFile(std::wstring_view pstrFilename, int encoding = XMLFILE_ENCODING_UTF8);
		/**
		 * @brief 执行 Release 操作
		 * @details 用于执行 Release 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Release();
		/**
		 * @brief 判断是否Valid
		 * @details 用于判断是否Valid。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsValid() const;

		/**
		 * @brief 设置PreserveWhitespace
		 * @details 用于设置PreserveWhitespace。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bPreserve [in] 是否Preserve
		 */
		void SetPreserveWhitespace(bool bPreserve = true);
		/**
		 * @brief 获取LastError消息
		 * @details 用于获取LastError消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrMessage [in] 消息字符串
		 * @param cchMax [in] cch最大参数
		 */
		void GetLastErrorMessage(wchar_t* pstrMessage, SIZE_T cchMax) const;
		/**
		 * @brief 获取LastErrorLocation
		 * @details 用于获取LastErrorLocation。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSource [in] 来源字符串
		 * @param cchMax [in] cch最大参数
		 */
		void GetLastErrorLocation(wchar_t* pstrSource, SIZE_T cchMax) const;

		/**
		 * @brief 获取Root
		 * @details 用于获取Root。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CMarkupNode 返回 获取Root 的结果
		 */
		CMarkupNode GetRoot();

	private:
		typedef struct tagXMLELEMENT
		{
			ULONG iStart;
			ULONG iChild;
			ULONG iNext;
			ULONG iParent;
			ULONG iData;
		} XMLELEMENT;

		wchar_t* m_pstrXML;
		XMLELEMENT* m_pElements;
		ULONG m_nElements;
		ULONG m_nReservedElements;
		std::array<wchar_t, 100> m_szErrorMsg;
		std::array<wchar_t, 50> m_szErrorXML;
		bool m_bPreserveWhitespace;

	private:
		/**
		 * @brief 执行 _Parse 操作
		 * @details 用于执行 _Parse 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool _Parse();
		/**
		 * @brief 执行 _Parse 操作
		 * @details 用于执行 _Parse 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrText [in,out] 文本字符串
		 * @param iParent [in] 父级控件值
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool _Parse(wchar_t*& pstrText, ULONG iParent);
		/**
		 * @brief 执行 _ReserveElement 操作
		 * @details 用于执行 _ReserveElement 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return XMLELEMENT* 返回结果对象指针，失败时返回 nullptr
		 */
		XMLELEMENT* _ReserveElement();
		/**
		 * @brief 执行 _SkipWhitespace 操作
		 * @details 用于执行 _SkipWhitespace 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstr [in,out] 字符串
		 */
		inline void _SkipWhitespace(wchar_t*& pstr) const;
		/**
		 * @brief 执行 _SkipWhitespace 操作
		 * @details 用于执行 _SkipWhitespace 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstr [in] 字符串
		 */
		inline void _SkipWhitespace(const wchar_t*& pstr) const;
		/**
		 * @brief 执行 _SkipIdentifier 操作
		 * @details 用于执行 _SkipIdentifier 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstr [in,out] 字符串
		 */
		inline void _SkipIdentifier(wchar_t*& pstr) const;
		/**
		 * @brief 执行 _SkipIdentifier 操作
		 * @details 用于执行 _SkipIdentifier 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstr [in] 字符串
		 */
		inline void _SkipIdentifier(const wchar_t*& pstr) const;
		/**
		 * @brief 执行 _ParseData 操作
		 * @details 用于执行 _ParseData 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrText [in,out] 文本字符串
		 * @param pstrData [in,out] 数据字符串
		 * @param cEnd [in] cEnd参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool _ParseData(wchar_t*& pstrText, wchar_t*& pstrData, wchar_t cEnd);
		/**
		 * @brief 执行 _ParseMetaChar 操作
		 * @details 用于执行 _ParseMetaChar 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrText [in,out] 文本字符串
		 * @param pstrDest [in,out] Dest字符串
		 */
		void _ParseMetaChar(wchar_t*& pstrText, wchar_t*& pstrDest);
		/**
		 * @brief 执行 _ParseAttributes 操作
		 * @details 用于执行 _ParseAttributes 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrText [in,out] 文本字符串
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool _ParseAttributes(wchar_t*& pstrText);
		/**
		 * @brief 执行 _Failed 操作
		 * @details 用于执行 _Failed 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrError [in] Error字符串
		 * @param pstrLocation [in] Location字符串
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool _Failed(std::wstring_view pstrError, const wchar_t* pstrLocation = nullptr);
	};


	class FYUI_API CMarkupNode
	{
		friend class CMarkup;
	private:
		/**
		 * @brief 构造 CMarkupNode 对象
		 * @details 用于构造 CMarkupNode 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CMarkupNode();
		/**
		 * @brief 构造 CMarkupNode 对象
		 * @details 用于构造 CMarkupNode 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 * @param iPos [in] 位置值
		 */
		CMarkupNode(CMarkup* pOwner, int iPos);

	public:
		/**
		 * @brief 判断是否Valid
		 * @details 用于判断是否Valid。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsValid() const;

		/**
		 * @brief 获取父级控件
		 * @details 用于获取父级控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CMarkupNode 返回 获取父级控件 的结果
		 */
		CMarkupNode GetParent();
		/**
		 * @brief 获取Sibling
		 * @details 用于获取Sibling。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CMarkupNode 返回 获取Sibling 的结果
		 */
		CMarkupNode GetSibling();
		/**
		 * @brief 获取子控件
		 * @details 用于获取子控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CMarkupNode 返回 获取子控件 的结果
		 */
		CMarkupNode GetChild();
		/**
		 * @brief 获取子控件
		 * @details 用于获取子控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return CMarkupNode 返回 获取子控件 的结果
		 */
		CMarkupNode GetChild(std::wstring_view pstrName);

		/**
		 * @brief 判断是否具有Siblings
		 * @details 用于判断是否具有Siblings。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HasSiblings() const;
		/**
		 * @brief 判断是否具有子控件集合
		 * @details 用于判断是否具有子控件集合。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HasChildren() const;
		/**
		 * @brief 获取名称
		 * @details 用于获取名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetName() const;
		/**
		 * @brief 获取值
		 * @details 用于获取值。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetValue() const;

		/**
		 * @brief 判断是否具有Attributes
		 * @details 用于判断是否具有Attributes。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HasAttributes();
		/**
		 * @brief 判断是否具有属性
		 * @details 用于判断是否具有属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HasAttribute(std::wstring_view pstrName);
		/**
		 * @brief 获取属性数量
		 * @details 用于获取属性数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetAttributeCount();
		/**
		 * @brief 获取属性名称
		 * @details 用于获取属性名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetAttributeName(int iIndex);
		/**
		 * @brief 获取属性值
		 * @details 用于获取属性值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetAttributeValue(int iIndex);
		/**
		 * @brief 获取属性值
		 * @details 用于获取属性值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetAttributeValue(std::wstring_view pstrName);
		/**
		 * @brief 获取属性值
		 * @details 用于获取属性值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param pstrValue [in] 属性值
		 * @param cchMax [in] cch最大参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetAttributeValue(int iIndex, wchar_t* pstrValue, SIZE_T cchMax);
		/**
		 * @brief 获取属性值
		 * @details 用于获取属性值。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 * @param cchMax [in] cch最大参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetAttributeValue(std::wstring_view pstrName, wchar_t* pstrValue, SIZE_T cchMax);

	private:
		/**
		 * @brief 执行 _MapAttributes 操作
		 * @details 用于执行 _MapAttributes 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void _MapAttributes();

		enum { MAX_XML_ATTRIBUTES = 64 };

		typedef struct
		{
			ULONG iName;
			ULONG iValue;
		} XMLATTRIBUTE;

		int m_iPos;
		int m_nAttributes;
		XMLATTRIBUTE m_aAttributes[MAX_XML_ATTRIBUTES];
		CMarkup* m_pOwner;
	};

} 


