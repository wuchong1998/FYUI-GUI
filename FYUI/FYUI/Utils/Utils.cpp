#include "pch.h"
#include "Utils.h"
#include <algorithm>
#include <array>
#include <cwctype>
#include <limits>

namespace FYUI
{
	CDuiPoint::CDuiPoint()
	{
		x = y = 0;
	}

	CDuiPoint::CDuiPoint(const POINT& src)
	{
		x = src.x;
		y = src.y;
	}

	CDuiPoint::CDuiPoint(int _x, int _y)
	{
		x = _x;
		y = _y;
	}

	CDuiPoint::CDuiPoint(LPARAM lParam)
	{
		x = GET_X_LPARAM(lParam);
		y = GET_Y_LPARAM(lParam);
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CDuiSize::CDuiSize()
	{
		cx = cy = 0;
	}

	CDuiSize::CDuiSize(const SIZE& src)
	{
		cx = src.cx;
		cy = src.cy;
	}

	CDuiSize::CDuiSize(const RECT rc)
	{
		cx = rc.right - rc.left;
		cy = rc.bottom - rc.top;
	}

	CDuiSize::CDuiSize(int _cx, int _cy)
	{
		cx = _cx;
		cy = _cy;
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CDuiRect::CDuiRect()
	{
		left = top = right = bottom = 0;
	}

	CDuiRect::CDuiRect(const RECT& src)
	{
		left = src.left;
		top = src.top;
		right = src.right;
		bottom = src.bottom;
	}

	CDuiRect::CDuiRect(int iLeft, int iTop, int iRight, int iBottom)
	{
		left = iLeft;
		top = iTop;
		right = iRight;
		bottom = iBottom;
	}

	int CDuiRect::GetWidth() const
	{
		return right - left;
	}

	int CDuiRect::GetHeight() const
	{
		return bottom - top;
	}

	void CDuiRect::Empty()
	{
		left = top = right = bottom = 0;
	}

	bool CDuiRect::IsNull() const
	{
		return (left == 0 && right == 0 && top == 0 && bottom == 0); 
	}

	void CDuiRect::Join(const RECT& rc)
	{
		if( rc.left < left ) left = rc.left;
		if( rc.top < top ) top = rc.top;
		if( rc.right > right ) right = rc.right;
		if( rc.bottom > bottom ) bottom = rc.bottom;
	}

	void CDuiRect::ResetOffset()
	{
		::OffsetRect(this, -left, -top);
	}

	void CDuiRect::Normalize()
	{
		if( left > right ) { int iTemp = left; left = right; right = iTemp; }
		if( top > bottom ) { int iTemp = top; top = bottom; bottom = iTemp; }
	}

	void CDuiRect::Offset(int cx, int cy)
	{
		::OffsetRect(this, cx, cy);
	}

	void CDuiRect::Inflate(int cx, int cy)
	{
		::InflateRect(this, cx, cy);
	}

	void CDuiRect::Deflate(int cx, int cy)
	{
		::InflateRect(this, -cx, -cy);
	}

	void CDuiRect::Union(CDuiRect& rc)
	{
		::UnionRect(this, this, &rc);
	}

	CStdPtrArray::CStdPtrArray(int iPreallocSize) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(iPreallocSize)
	{
		if( iPreallocSize > 0 ) m_ppVoid = static_cast<LPVOID*>(malloc(iPreallocSize * sizeof(LPVOID)));
	}

	CStdPtrArray::CStdPtrArray(const CStdPtrArray& src) : m_ppVoid(NULL), m_nCount(0), m_nAllocated(0)
	{
		for(int i=0; i<src.GetSize(); i++)
			Add(src.GetAt(i));
	}

	CStdPtrArray::~CStdPtrArray()
	{
		if( m_ppVoid != NULL ) free(m_ppVoid);
	}

	void CStdPtrArray::Empty()
	{
		if( m_ppVoid != NULL ) free(m_ppVoid);
		m_ppVoid = NULL;
		m_nCount = m_nAllocated = 0;
	}

	void CStdPtrArray::Resize(int iSize)
	{
		Empty();
		m_ppVoid = static_cast<LPVOID*>(malloc(iSize * sizeof(LPVOID)));
		::ZeroMemory(m_ppVoid, iSize * sizeof(LPVOID));
		m_nAllocated = iSize;
		m_nCount = iSize;
	}

	bool CStdPtrArray::IsEmpty() const
	{
		return m_nCount == 0;
	}

	bool CStdPtrArray::Add(LPVOID pData)
	{
		if( ++m_nCount >= m_nAllocated) {
			int nAllocated = m_nAllocated * 2;
			if( nAllocated == 0 ) nAllocated = 11;
			LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
			if( ppVoid != NULL ) {
				m_nAllocated = nAllocated;
				m_ppVoid = ppVoid;
			}
			else {
				--m_nCount;
				return false;
			}
		}
		m_ppVoid[m_nCount - 1] = pData;
		return true;
	}

	bool CStdPtrArray::InsertAt(int iIndex, LPVOID pData)
	{
		if( iIndex == m_nCount ) return Add(pData);
		if( iIndex < 0 || iIndex > m_nCount ) return false;
		if( ++m_nCount >= m_nAllocated) {
			int nAllocated = m_nAllocated * 2;
			if( nAllocated == 0 ) nAllocated = 11;
			LPVOID* ppVoid = static_cast<LPVOID*>(realloc(m_ppVoid, nAllocated * sizeof(LPVOID)));
			if( ppVoid != NULL ) {
				m_nAllocated = nAllocated;
				m_ppVoid = ppVoid;
			}
			else {
				--m_nCount;
				return false;
			}
		}
		memmove(&m_ppVoid[iIndex + 1], &m_ppVoid[iIndex], (m_nCount - iIndex - 1) * sizeof(LPVOID));
		m_ppVoid[iIndex] = pData;
		return true;
	}

	bool CStdPtrArray::SetAt(int iIndex, LPVOID pData)
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return false;
		m_ppVoid[iIndex] = pData;
		return true;
	}

	bool CStdPtrArray::Remove(int iIndex)
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return false;
		if( iIndex < --m_nCount ) ::CopyMemory(m_ppVoid + iIndex, m_ppVoid + iIndex + 1, (m_nCount - iIndex) * sizeof(LPVOID));
		return true;
	}

	int CStdPtrArray::Find(LPVOID pData) const
	{
		for( int i = 0; i < m_nCount; i++ ) if( m_ppVoid[i] == pData ) return i;
		return -1;
	}

	int CStdPtrArray::GetSize() const
	{
		return m_nCount;
	}

	LPVOID* CStdPtrArray::GetData()
	{
		return m_ppVoid;
	}

	LPVOID CStdPtrArray::GetAt(int iIndex) const
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return NULL;
		return m_ppVoid[iIndex];
	}

	LPVOID CStdPtrArray::operator[] (int iIndex) const
	{
		
		return m_ppVoid[iIndex];
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CStdValArray::CStdValArray(int iElementSize, int iPreallocSize /*= 0*/) : 
		m_pVoid(NULL), 
		m_nCount(0), 
		m_iElementSize(iElementSize), 
		m_nAllocated(iPreallocSize)
	{
	
		if( iPreallocSize > 0 ) m_pVoid = static_cast<LPBYTE>(malloc(iPreallocSize * m_iElementSize));
	}

	CStdValArray::~CStdValArray()
	{
		if( m_pVoid != NULL ) free(m_pVoid);
	}

	void CStdValArray::Empty()
	{   
		m_nCount = 0;  // NOTE: We keep the memory in place
	}

	bool CStdValArray::IsEmpty() const
	{
		return m_nCount == 0;
	}

	bool CStdValArray::Add(LPCVOID pData)
	{
		if( ++m_nCount >= m_nAllocated) {
			int nAllocated = m_nAllocated * 2;
			if( nAllocated == 0 ) nAllocated = 11;
			LPBYTE pVoid = static_cast<LPBYTE>(realloc(m_pVoid, nAllocated * m_iElementSize));
			if( pVoid != NULL ) {
				m_nAllocated = nAllocated;
				m_pVoid = pVoid;
			}
			else {
				--m_nCount;
				return false;
			}
		}
		::CopyMemory(m_pVoid + ((m_nCount - 1) * m_iElementSize), pData, m_iElementSize);
		return true;
	}

	bool CStdValArray::Remove(int iIndex)
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return false;
		if( iIndex < --m_nCount ) ::CopyMemory(m_pVoid + (iIndex * m_iElementSize), m_pVoid + ((iIndex + 1) * m_iElementSize), (m_nCount - iIndex) * m_iElementSize);
		return true;
	}

	int CStdValArray::GetSize() const
	{
		return m_nCount;
	}

	LPVOID CStdValArray::GetData()
	{
		return static_cast<LPVOID>(m_pVoid);
	}

	LPVOID CStdValArray::GetAt(int iIndex) const
	{
		if( iIndex < 0 || iIndex >= m_nCount ) return NULL;
		return m_pVoid + (iIndex * m_iElementSize);
	}

	LPVOID CStdValArray::operator[] (int iIndex) const
	{
		
		return m_pVoid + (iIndex * m_iElementSize);
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	namespace
	{
		int NposToInt(std::wstring_view::size_type pos)
		{
			return pos == std::wstring_view::npos ? -1 : static_cast<int>(pos);
		}

		int HexDigitToInt(wchar_t ch)
		{
			if (ch >= L'0' && ch <= L'9') return ch - L'0';
			if (ch >= L'a' && ch <= L'f') return ch - L'a' + 10;
			if (ch >= L'A' && ch <= L'F') return ch - L'A' + 10;
			return -1;
		}

		bool TryParseUnsignedInteger(std::wstring_view text, unsigned long long& value, int base)
		{
			if (base < 2 || base > 16) {
				return false;
			}

			text = StringUtil::TrimView(text);
			if (text.empty()) {
				return false;
			}

			if (base == 16 && text.size() > 2 && text[0] == L'0' && (text[1] == L'x' || text[1] == L'X')) {
				text.remove_prefix(2);
			}

			if (text.empty()) {
				return false;
			}

			unsigned long long parsedValue = 0;
			for (wchar_t ch : text) {
				const int digit = HexDigitToInt(ch);
				if (digit < 0 || digit >= base) {
					return false;
				}

				if (parsedValue > (((std::numeric_limits<unsigned long long>::max)()) - static_cast<unsigned long long>(digit)) / static_cast<unsigned long long>(base)) {
					return false;
				}

				parsedValue = (parsedValue * static_cast<unsigned long long>(base)) + static_cast<unsigned long long>(digit);
			}

			value = parsedValue;
			return true;
		}

		template <size_t Count>
		bool TryParseIntList(std::wstring_view text, std::array<int, Count>& values)
		{
			const std::vector<std::wstring_view> tokens = StringUtil::SplitView(text, L',', true);
			if (tokens.size() != Count) {
				return false;
			}

			for (size_t i = 0; i < Count; ++i) {
				if (!StringUtil::TryParseInt(tokens[i], values[i])) {
					return false;
				}
			}

			return true;
		}
	}

	namespace StringUtil
	{
		void Assign(std::wstring& target, std::wstring_view text, int nLength)
		{
			if (nLength < 0) {
				target.assign(text);
				return;
			}

			target.assign(text.data(), text.data() + (std::min)(static_cast<std::wstring_view::size_type>(nLength), text.size()));
		}

		int CompareNoCase(std::wstring_view lhs, std::wstring_view rhs)
		{
			const size_t sharedLength = (std::min)(lhs.size(), rhs.size());
			for (size_t i = 0; i < sharedLength; ++i) {
				const wchar_t left = static_cast<wchar_t>(std::towlower(lhs[i]));
				const wchar_t right = static_cast<wchar_t>(std::towlower(rhs[i]));
				if (left < right) return -1;
				if (left > right) return 1;
			}

			if (lhs.size() < rhs.size()) return -1;
			if (lhs.size() > rhs.size()) return 1;
			return 0;
		}

		bool EqualsNoCase(std::wstring_view lhs, std::wstring_view rhs)
		{
			return CompareNoCase(lhs, rhs) == 0;
		}

		void MakeUpper(std::wstring& text)
		{
			for (wchar_t& ch : text) {
				ch = static_cast<wchar_t>(std::towupper(ch));
			}
		}

		void MakeLower(std::wstring& text)
		{
			for (wchar_t& ch : text) {
				ch = static_cast<wchar_t>(std::towlower(ch));
			}
		}

		std::wstring Left(std::wstring_view text, int nLength)
		{
			if (nLength <= 0) {
				return std::wstring();
			}
			const size_t count = (std::min)(text.size(), static_cast<size_t>(nLength));
			return std::wstring(text.substr(0, count));
		}

		std::wstring Mid(std::wstring_view text, int iPos, int nLength)
		{
			if (iPos < 0 || static_cast<size_t>(iPos) >= text.size()) {
				return std::wstring();
			}

			const size_t start = static_cast<size_t>(iPos);
			size_t count = text.size() - start;
			if (nLength >= 0) {
				count = (std::min)(count, static_cast<size_t>(nLength));
			}
			return std::wstring(text.substr(start, count));
		}

		std::wstring Right(std::wstring_view text, int nLength)
		{
			if (nLength <= 0) {
				return std::wstring();
			}
			const size_t count = (std::min)(text.size(), static_cast<size_t>(nLength));
			return std::wstring(text.substr(text.size() - count, count));
		}

		std::wstring_view TrimView(std::wstring_view text)
		{
			size_t start = 0;
			while (start < text.size() && std::iswspace(text[start]) != 0) {
				++start;
			}

			size_t end = text.size();
			while (end > start && std::iswspace(text[end - 1]) != 0) {
				--end;
			}

			return text.substr(start, end - start);
		}

		std::wstring& TrimLeft(std::wstring& text)
		{
			const auto it = std::find_if_not(text.begin(), text.end(), [](wchar_t ch) {
				return std::iswspace(ch) != 0;
			});
			text.erase(text.begin(), it);
			return text;
		}

		std::wstring& TrimRight(std::wstring& text)
		{
			const auto it = std::find_if_not(text.rbegin(), text.rend(), [](wchar_t ch) {
				return std::iswspace(ch) != 0;
			});
			text.erase(it.base(), text.end());
			return text;
		}

		std::wstring& Trim(std::wstring& text)
		{
			return TrimRight(TrimLeft(text));
		}

		int Find(std::wstring_view text, wchar_t ch, int iPos)
		{
			if (iPos < 0 || static_cast<size_t>(iPos) > text.size()) {
				return -1;
			}
			return NposToInt(text.find(ch, static_cast<size_t>(iPos)));
		}

		int Find(std::wstring_view text, std::wstring_view sub, int iPos)
		{
			if (iPos < 0 || static_cast<size_t>(iPos) > text.size()) {
				return -1;
			}
			return NposToInt(text.find(sub, static_cast<size_t>(iPos)));
		}

		int ReverseFind(std::wstring_view text, wchar_t ch)
		{
			return NposToInt(text.rfind(ch));
		}

		int ReplaceAll(std::wstring& text, std::wstring_view from, std::wstring_view to)
		{
			if (from.empty()) {
				return 0;
			}

			int count = 0;
			size_t pos = 0;
			while ((pos = text.find(from, pos)) != std::wstring::npos) {
				text.replace(pos, from.size(), to);
				pos += to.size();
				++count;
			}
			return count;
		}

		bool TryParseInt(std::wstring_view text, int& value, int base)
		{
			text = TrimView(text);
			if (text.empty()) {
				return false;
			}

			bool negative = false;
			if (text.front() == L'+' || text.front() == L'-') {
				negative = text.front() == L'-';
				text.remove_prefix(1);
			}

			unsigned long long parsedValue = 0;
			if (!TryParseUnsignedInteger(text, parsedValue, base)) {
				return false;
			}

			if (negative) {
				const unsigned long long absMin = static_cast<unsigned long long>((std::numeric_limits<int>::max)()) + 1ULL;
				if (parsedValue > absMin) {
					return false;
				}

				value = parsedValue == absMin ? (std::numeric_limits<int>::min)() : -static_cast<int>(parsedValue);
				return true;
			}

			if (parsedValue > static_cast<unsigned long long>((std::numeric_limits<int>::max)())) {
				return false;
			}

			value = static_cast<int>(parsedValue);
			return true;
		}

		bool TryParseDword(std::wstring_view text, DWORD& value, int base)
		{
			unsigned long long parsedValue = 0;
			if (!TryParseUnsignedInteger(text, parsedValue, base)) {
				return false;
			}

			if (parsedValue > static_cast<unsigned long long>((std::numeric_limits<DWORD>::max)())) {
				return false;
			}

			value = static_cast<DWORD>(parsedValue);
			return true;
		}

		bool TryParseDouble(std::wstring_view text, double& value)
		{
			const std::wstring trimmedText(TrimView(text));
			if (trimmedText.empty()) {
				return false;
			}

			size_t processedLength = 0;
			try {
				value = std::stod(trimmedText, &processedLength);
			}
			catch (...) {
				return false;
			}

			return processedLength == trimmedText.size();
		}

		bool ParseBool(std::wstring_view text)
		{
			text = TrimView(text);
			return EqualsNoCase(text, L"true")
				|| EqualsNoCase(text, L"yes")
				|| EqualsNoCase(text, L"on")
				|| text == L"1";
		}

		bool TryParseSize(std::wstring_view text, SIZE& value)
		{
			std::array<int, 2> values = { 0, 0 };
			if (!TryParseIntList(text, values)) {
				return false;
			}

			value.cx = values[0];
			value.cy = values[1];
			return true;
		}

		bool TryParseRect(std::wstring_view text, RECT& value)
		{
			std::array<int, 4> values = { 0, 0, 0, 0 };
			if (!TryParseIntList(text, values)) {
				return false;
			}

			value.left = values[0];
			value.top = values[1];
			value.right = values[2];
			value.bottom = values[3];
			return true;
		}

		bool TryParseColor(std::wstring_view text, DWORD& value)
		{
			text = TrimView(text);
			if (!text.empty() && text.front() == L'#') {
				text.remove_prefix(1);
			}

			return TryParseDword(text, value, 16);
		}

		std::vector<std::wstring_view> SplitView(std::wstring_view text, wchar_t delimiter, bool trimItems)
		{
			std::vector<std::wstring_view> parts;
			size_t start = 0;
			while (start <= text.size()) {
				const size_t end = text.find(delimiter, start);
				std::wstring_view part = end == std::wstring_view::npos
					? text.substr(start)
					: text.substr(start, end - start);
				if (trimItems) {
					part = TrimView(part);
				}
				parts.push_back(part);

				if (end == std::wstring_view::npos) {
					break;
				}

				start = end + 1;
			}

			return parts;
		}

		std::vector<KeyValueView> ParseQuotedAttributes(std::wstring_view text, wchar_t quote)
		{
			std::vector<KeyValueView> attributes;
			size_t position = 0;
			while (position < text.size()) {
				while (position < text.size() && std::iswspace(text[position]) != 0) {
					++position;
				}
				if (position >= text.size()) {
					break;
				}

				const size_t keyStart = position;
				while (position < text.size() && text[position] > L' ' && text[position] != L'=') {
					++position;
				}

				const std::wstring_view key = TrimView(text.substr(keyStart, position - keyStart));
				if (key.empty()) {
					break;
				}

				while (position < text.size() && std::iswspace(text[position]) != 0) {
					++position;
				}
				if (position >= text.size() || text[position] != L'=') {
					break;
				}
				++position;

				while (position < text.size() && std::iswspace(text[position]) != 0) {
					++position;
				}
				if (position >= text.size() || text[position] != quote) {
					break;
				}
				++position;

				const size_t valueStart = position;
				while (position < text.size() && text[position] != quote) {
					++position;
				}

				attributes.push_back({ key, text.substr(valueStart, position - valueStart) });

				if (position >= text.size()) {
					break;
				}
				++position;
			}

			return attributes;
		}

		std::wstring Utf8ToUtf16(std::string_view utf8, UINT codePage)
		{
			if (utf8.empty()) {
				return std::wstring();
			}

			const int length = ::MultiByteToWideChar(
				codePage,
				0,
				utf8.data(),
				static_cast<int>(utf8.size()),
				nullptr,
				0);
			if (length <= 0) {
				return std::wstring();
			}

			std::wstring utf16(static_cast<size_t>(length), L'\0');
			::MultiByteToWideChar(
				codePage,
				0,
				utf8.data(),
				static_cast<int>(utf8.size()),
				utf16.data(),
				length);
			return utf16;
		}

		std::string Utf16ToUtf8(std::wstring_view utf16, UINT codePage)
		{
			if (utf16.empty()) {
				return std::string();
			}

			const int length = ::WideCharToMultiByte(
				codePage,
				0,
				utf16.data(),
				static_cast<int>(utf16.size()),
				nullptr,
				0,
				nullptr,
				nullptr);
			if (length <= 0) {
				return std::string();
			}

			std::string utf8(static_cast<size_t>(length), '\0');
			::WideCharToMultiByte(
				codePage,
				0,
				utf16.data(),
				static_cast<int>(utf16.size()),
				utf8.data(),
				length,
				nullptr,
				nullptr);
			return utf8;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	CWaitCursor::CWaitCursor()
	{
		m_hOrigCursor = ::SetCursor(::LoadCursor(NULL, IDC_WAIT));
	}

	CWaitCursor::~CWaitCursor()
	{
		::SetCursor(m_hOrigCursor);
	}


}


