#pragma once

#include "OAIdl.h"
#include <locale.h>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
namespace FYUI
{
	class FYUI_API STRINGorID
	{
	public:
		STRINGorID() : m_uId(0), m_bIsId(false) {}
		STRINGorID(std::wstring_view text) : m_text(text), m_uId(0), m_bIsId(false) {}
		STRINGorID(const wchar_t* text) : m_text(text != nullptr ? text : L""), m_uId(0), m_bIsId(false) {}
		STRINGorID(UINT nID) : m_uId(nID), m_bIsId(true) {}

		bool IsId() const { return m_bIsId; }
		bool IsString() const { return !m_bIsId; }
		std::wstring_view view() const { return m_text; }
		const wchar_t* c_str() const { return m_bIsId ? MAKEINTRESOURCE(m_uId) : m_text.c_str(); }

	private:
		std::wstring m_text;
		UINT m_uId;
		bool m_bIsId;
	};

	class FYUI_API CDuiPoint : public tagPOINT
	{
	public:
		CDuiPoint();
		CDuiPoint(const POINT& src);
		CDuiPoint(int x, int y);
		CDuiPoint(LPARAM lParam);
	};

	class FYUI_API CDuiSize : public tagSIZE
	{
	public:
		CDuiSize();
		CDuiSize(const SIZE& src);
		CDuiSize(const RECT rc);
		CDuiSize(int cx, int cy);
	};


	class FYUI_API CDuiRect : public tagRECT
	{
	public:
		CDuiRect();
		CDuiRect(const RECT& src);
		CDuiRect(int iLeft, int iTop, int iRight, int iBottom);

		int GetWidth() const;
		int GetHeight() const;
		void Empty();
		bool IsNull() const;
		void Join(const RECT& rc);
		void ResetOffset();
		void Normalize();
		void Offset(int cx, int cy);
		void Inflate(int cx, int cy);
		void Deflate(int cx, int cy);
		void Union(CDuiRect& rc);
	};

	class FYUI_API CStdPtrArray
	{
	public:
		CStdPtrArray(int iPreallocSize = 0);
		CStdPtrArray(const CStdPtrArray& src);
		~CStdPtrArray();

		void Empty();
		void Resize(int iSize);
		bool IsEmpty() const;
		bool empty() const { return IsEmpty(); }
		int Find(LPVOID iIndex) const;
		bool Add(LPVOID pData);
		bool SetAt(int iIndex, LPVOID pData);
		bool InsertAt(int iIndex, LPVOID pData);
		bool Remove(int iIndex);
		int GetSize() const;
		LPVOID* GetData();
		LPVOID* c_str() { return GetData(); }
		LPVOID* c_str() const { return const_cast<CStdPtrArray*>(this)->GetData(); }

		LPVOID GetAt(int iIndex) const;
		LPVOID operator[] (int nIndex) const;

	protected:
		LPVOID* m_ppVoid;
		int m_nCount;
		int m_nAllocated;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CStdValArray
	{
	public:
		CStdValArray(int iElementSize, int iPreallocSize = 0);
		~CStdValArray();

		void Empty();
		bool IsEmpty() const;
		bool empty() const { return IsEmpty(); }
		bool Add(LPCVOID pData);
		bool Remove(int iIndex);
		int GetSize() const;
		LPVOID GetData();
		LPVOID c_str() { return GetData(); }
		LPVOID c_str() const { return const_cast<CStdValArray*>(this)->GetData(); }

		LPVOID GetAt(int iIndex) const;
		LPVOID operator[] (int nIndex) const;

	protected:
		LPBYTE m_pVoid;
		int m_iElementSize;
		int m_nCount;
		int m_nAllocated;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//

	namespace StringUtil
	{
		struct KeyValueView
		{
			std::wstring_view key;
			std::wstring_view value;
		};

		FYUI_API void Assign(std::wstring& target, std::wstring_view text, int nLength = -1);
		FYUI_API int CompareNoCase(std::wstring_view lhs, std::wstring_view rhs);
		FYUI_API bool EqualsNoCase(std::wstring_view lhs, std::wstring_view rhs);
		FYUI_API void MakeUpper(std::wstring& text);
		FYUI_API void MakeLower(std::wstring& text);
		FYUI_API std::wstring Left(std::wstring_view text, int nLength);
		FYUI_API std::wstring Mid(std::wstring_view text, int iPos, int nLength = -1);
		FYUI_API std::wstring Right(std::wstring_view text, int nLength);
		FYUI_API std::wstring_view TrimView(std::wstring_view text);
		FYUI_API std::wstring& TrimLeft(std::wstring& text);
		FYUI_API std::wstring& TrimRight(std::wstring& text);
		FYUI_API std::wstring& Trim(std::wstring& text);
		FYUI_API int Find(std::wstring_view text, wchar_t ch, int iPos = 0);
		FYUI_API int Find(std::wstring_view text, std::wstring_view sub, int iPos = 0);
		FYUI_API int ReverseFind(std::wstring_view text, wchar_t ch);
		FYUI_API int ReplaceAll(std::wstring& text, std::wstring_view from, std::wstring_view to);
		FYUI_API bool TryParseInt(std::wstring_view text, int& value, int base = 10);
		FYUI_API bool TryParseDword(std::wstring_view text, DWORD& value, int base = 10);
		FYUI_API bool TryParseDouble(std::wstring_view text, double& value);
		FYUI_API bool ParseBool(std::wstring_view text);
		FYUI_API bool TryParseSize(std::wstring_view text, SIZE& value);
		FYUI_API bool TryParseRect(std::wstring_view text, RECT& value);
		FYUI_API bool TryParseColor(std::wstring_view text, DWORD& value);
		FYUI_API std::vector<std::wstring_view> SplitView(std::wstring_view text, wchar_t delimiter, bool trimItems = false);
		FYUI_API std::vector<KeyValueView> ParseQuotedAttributes(std::wstring_view text, wchar_t quote = L'\'');
		FYUI_API std::wstring Utf8ToUtf16(std::string_view utf8, UINT codePage = CP_UTF8);
		FYUI_API std::string Utf16ToUtf8(std::wstring_view utf16, UINT codePage = CP_UTF8);

		template <typename... TArgs>
		std::wstring Format(std::wformat_string<TArgs...> fmt, TArgs&&... args)
		{
			return std::format(fmt, std::forward<TArgs>(args)...);
		}
	}

	static std::vector<std::wstring> StrSplit(std::wstring text, std::wstring_view sp)
	{
		std::vector<std::wstring> vResults;
		int pos = StringUtil::Find(text, sp, 0);
		while (pos >= 0)
		{
			std::wstring t = StringUtil::Left(text, pos);
			vResults.push_back(t);
			text = StringUtil::Right(text, static_cast<int>(text.size()) - pos - static_cast<int>(sp.size()));
			pos = StringUtil::Find(text, sp);
		}
		vResults.push_back(text);
		return vResults;
	}
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CWaitCursor
	{
	public:
		CWaitCursor();
		~CWaitCursor();

	protected:
		HCURSOR m_hOrigCursor;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CDuiVariant : public VARIANT
	{
	public:
		CDuiVariant() 
		{ 
			VariantInit(this); 
		}
		CDuiVariant(int i)
		{
			VariantInit(this);
			this->vt = VT_I4;
			this->intVal = i;
		}
		CDuiVariant(float f)
		{
			VariantInit(this);
			this->vt = VT_R4;
			this->fltVal = f;
		}
		CDuiVariant(LPOLESTR s)
		{
			VariantInit(this);
			this->vt = VT_BSTR;
			this->bstrVal = s;
		}
		CDuiVariant(IDispatch *disp)
		{
			VariantInit(this);
			this->vt = VT_DISPATCH;
			this->pdispVal = disp;
		}

		~CDuiVariant() 
		{ 
			VariantClear(this); 
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	//
	static char* w2a(wchar_t* lpszSrc, UINT   CodePage = CP_ACP)
	{
		if (lpszSrc != NULL)
		{
			int  nANSILen = WideCharToMultiByte(CodePage, 0, lpszSrc, -1, NULL, 0, NULL, NULL);
			char* pANSI = new char[nANSILen + 1];
			if (pANSI != NULL)
			{
				ZeroMemory(pANSI, nANSILen + 1);
				WideCharToMultiByte(CodePage, 0, lpszSrc, -1, pANSI, nANSILen, NULL, NULL);
				return pANSI;
			}
		}	
		return NULL;
	}



	static wchar_t* a2w(char* lpszSrc, UINT   CodePage = CP_ACP)
	{
		if (lpszSrc != NULL)
		{
			int nUnicodeLen = MultiByteToWideChar(CodePage, 0, lpszSrc, -1, NULL, 0);
			LPWSTR pUnicode = new WCHAR[nUnicodeLen + 1];
			if (pUnicode != NULL)
			{
				ZeroMemory((void*)pUnicode, (nUnicodeLen + 1) * sizeof(WCHAR));
				MultiByteToWideChar(CodePage, 0, lpszSrc,-1, pUnicode, nUnicodeLen);
				return pUnicode;
			}
		}
		return NULL;
	}

}
