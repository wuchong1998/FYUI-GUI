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
		CMarkup(std::wstring_view pstrXML = {});
		~CMarkup();

		bool Load(std::wstring_view pstrXML);
		bool LoadFromMem(BYTE* pByte, DWORD dwSize, int encoding = XMLFILE_ENCODING_UTF8);
		bool LoadFromFile(std::wstring_view pstrFilename, int encoding = XMLFILE_ENCODING_UTF8);
		void Release();
		bool IsValid() const;

		void SetPreserveWhitespace(bool bPreserve = true);
		void GetLastErrorMessage(wchar_t* pstrMessage, SIZE_T cchMax) const;
		void GetLastErrorLocation(wchar_t* pstrSource, SIZE_T cchMax) const;

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
		bool _Parse();
		bool _Parse(wchar_t*& pstrText, ULONG iParent);
		XMLELEMENT* _ReserveElement();
		inline void _SkipWhitespace(wchar_t*& pstr) const;
		inline void _SkipWhitespace(const wchar_t*& pstr) const;
		inline void _SkipIdentifier(wchar_t*& pstr) const;
		inline void _SkipIdentifier(const wchar_t*& pstr) const;
		bool _ParseData(wchar_t*& pstrText, wchar_t*& pstrData, wchar_t cEnd);
		void _ParseMetaChar(wchar_t*& pstrText, wchar_t*& pstrDest);
		bool _ParseAttributes(wchar_t*& pstrText);
		bool _Failed(std::wstring_view pstrError, const wchar_t* pstrLocation = nullptr);
	};


	class FYUI_API CMarkupNode
	{
		friend class CMarkup;
	private:
		CMarkupNode();
		CMarkupNode(CMarkup* pOwner, int iPos);

	public:
		bool IsValid() const;

		CMarkupNode GetParent();
		CMarkupNode GetSibling();
		CMarkupNode GetChild();
		CMarkupNode GetChild(std::wstring_view pstrName);

		bool HasSiblings() const;
		bool HasChildren() const;
		std::wstring_view GetName() const;
		std::wstring_view GetValue() const;

		bool HasAttributes();
		bool HasAttribute(std::wstring_view pstrName);
		int GetAttributeCount();
		std::wstring_view GetAttributeName(int iIndex);
		std::wstring_view GetAttributeValue(int iIndex);
		std::wstring_view GetAttributeValue(std::wstring_view pstrName);
		bool GetAttributeValue(int iIndex, wchar_t* pstrValue, SIZE_T cchMax);
		bool GetAttributeValue(std::wstring_view pstrName, wchar_t* pstrValue, SIZE_T cchMax);

	private:
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


