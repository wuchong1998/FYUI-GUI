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
		/**
		 * @brief 构造 STRINGorID 对象
		 * @details 用于构造 STRINGorID 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
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
		/**
		 * @brief 构造 CDuiPoint 对象
		 * @details 用于构造 CDuiPoint 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CDuiPoint();
		/**
		 * @brief 构造 CDuiPoint 对象
		 * @details 用于构造 CDuiPoint 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param src [in] src参数
		 */
		CDuiPoint(const POINT& src);
		/**
		 * @brief 构造 CDuiPoint 对象
		 * @details 用于构造 CDuiPoint 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param x [in] x参数
		 * @param y [in] y参数
		 */
		CDuiPoint(int x, int y);
		/**
		 * @brief 构造 CDuiPoint 对象
		 * @details 用于构造 CDuiPoint 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lParam [in] lParam参数
		 */
		CDuiPoint(LPARAM lParam);
	};

	class FYUI_API CDuiSize : public tagSIZE
	{
	public:
		/**
		 * @brief 构造 CDuiSize 对象
		 * @details 用于构造 CDuiSize 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CDuiSize();
		/**
		 * @brief 构造 CDuiSize 对象
		 * @details 用于构造 CDuiSize 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param src [in] src参数
		 */
		CDuiSize(const SIZE& src);
		/**
		 * @brief 构造 CDuiSize 对象
		 * @details 用于构造 CDuiSize 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		CDuiSize(const RECT rc);
		/**
		 * @brief 构造 CDuiSize 对象
		 * @details 用于构造 CDuiSize 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		CDuiSize(int cx, int cy);
	};


	class FYUI_API CDuiRect : public tagRECT
	{
	public:
		/**
		 * @brief 构造 CDuiRect 对象
		 * @details 用于构造 CDuiRect 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CDuiRect();
		/**
		 * @brief 构造 CDuiRect 对象
		 * @details 用于构造 CDuiRect 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param src [in] src参数
		 */
		CDuiRect(const RECT& src);
		/**
		 * @brief 构造 CDuiRect 对象
		 * @details 用于构造 CDuiRect 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iLeft [in] Left值
		 * @param iTop [in] Top值
		 * @param iRight [in] Right值
		 * @param iBottom [in] Bottom值
		 */
		CDuiRect(int iLeft, int iTop, int iRight, int iBottom);

		/**
		 * @brief 获取宽度
		 * @details 用于获取宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetWidth() const;
		/**
		 * @brief 获取高度
		 * @details 用于获取高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetHeight() const;
		/**
		 * @brief 执行 Empty 操作
		 * @details 用于执行 Empty 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Empty();
		/**
		 * @brief 判断是否Null
		 * @details 用于判断是否Null。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsNull() const;
		/**
		 * @brief 执行 Join 操作
		 * @details 用于执行 Join 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void Join(const RECT& rc);
		/**
		 * @brief 重置Offset
		 * @details 用于重置Offset。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetOffset();
		/**
		 * @brief 执行 Normalize 操作
		 * @details 用于执行 Normalize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Normalize();
		/**
		 * @brief 执行 Offset 操作
		 * @details 用于执行 Offset 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void Offset(int cx, int cy);
		/**
		 * @brief 执行 Inflate 操作
		 * @details 用于执行 Inflate 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void Inflate(int cx, int cy);
		/**
		 * @brief 执行 Deflate 操作
		 * @details 用于执行 Deflate 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void Deflate(int cx, int cy);
		/**
		 * @brief 执行 Union 操作
		 * @details 用于执行 Union 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in,out] 矩形区域
		 */
		void Union(CDuiRect& rc);
	};

	class FYUI_API CStdPtrArray
	{
	public:
		/**
		 * @brief 构造 CStdPtrArray 对象
		 * @details 用于构造 CStdPtrArray 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iPreallocSize [in] Prealloc尺寸值
		 */
		CStdPtrArray(int iPreallocSize = 0);
		/**
		 * @brief 构造 CStdPtrArray 对象
		 * @details 用于构造 CStdPtrArray 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param src [in] src参数
		 */
		CStdPtrArray(const CStdPtrArray& src);
		/**
		 * @brief 析构 CStdPtrArray 对象
		 * @details 用于析构 CStdPtrArray 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CStdPtrArray();

		/**
		 * @brief 执行 Empty 操作
		 * @details 用于执行 Empty 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Empty();
		/**
		 * @brief 执行 Resize 操作
		 * @details 用于执行 Resize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iSize [in] 尺寸值
		 */
		void Resize(int iSize);
		/**
		 * @brief 判断是否Empty
		 * @details 用于判断是否Empty。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsEmpty() const;
		/**
		 * @brief 执行 empty 操作
		 * @details 用于执行 empty 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool empty() const { return IsEmpty(); }
		int Find(LPVOID iIndex) const;
		/**
		 * @brief 执行 Add 操作
		 * @details 用于执行 Add 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pData [in] 数据对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Add(LPVOID pData);
		/**
		 * @brief 设置At
		 * @details 用于设置At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param pData [in] 数据对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetAt(int iIndex, LPVOID pData);
		/**
		 * @brief 插入At
		 * @details 用于插入At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param pData [in] 数据对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool InsertAt(int iIndex, LPVOID pData);
		/**
		 * @brief 执行 Remove 操作
		 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Remove(int iIndex);
		/**
		 * @brief 获取尺寸
		 * @details 用于获取尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetSize() const;
		/**
		 * @brief 获取数据
		 * @details 用于获取数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @return LPVOID* 返回结果对象指针，失败时返回 nullptr
		 */
		LPVOID* GetData();
		/**
		 * @brief 执行 c_str 操作
		 * @details 用于执行 c_str 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return LPVOID* 返回结果对象指针，失败时返回 nullptr
		 */
		LPVOID* c_str() { return GetData(); }
		LPVOID* c_str() const { return const_cast<CStdPtrArray*>(this)->GetData(); }

		LPVOID GetAt(int iIndex) const;
		/**
		 * @brief 执行 operator[] 运算
		 * @details 用于执行 operator[] 运算。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nIndex [in] 索引值
		 * @return LPVOID 返回 执行 operator[] 运算 的结果
		 */
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
		/**
		 * @brief 构造 CStdValArray 对象
		 * @details 用于构造 CStdValArray 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iElementSize [in] Element尺寸值
		 * @param iPreallocSize [in] Prealloc尺寸值
		 */
		CStdValArray(int iElementSize, int iPreallocSize = 0);
		/**
		 * @brief 析构 CStdValArray 对象
		 * @details 用于析构 CStdValArray 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CStdValArray();

		/**
		 * @brief 执行 Empty 操作
		 * @details 用于执行 Empty 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Empty();
		/**
		 * @brief 判断是否Empty
		 * @details 用于判断是否Empty。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsEmpty() const;
		/**
		 * @brief 执行 empty 操作
		 * @details 用于执行 empty 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool empty() const { return IsEmpty(); }
		bool Add(LPCVOID pData);
		/**
		 * @brief 执行 Remove 操作
		 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Remove(int iIndex);
		/**
		 * @brief 获取尺寸
		 * @details 用于获取尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetSize() const;
		/**
		 * @brief 获取数据
		 * @details 用于获取数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @return LPVOID 返回 获取数据 的结果
		 */
		LPVOID GetData();
		/**
		 * @brief 执行 c_str 操作
		 * @details 用于执行 c_str 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return LPVOID 返回 执行 c_str 操作 的结果
		 */
		LPVOID c_str() { return GetData(); }
		LPVOID c_str() const { return const_cast<CStdValArray*>(this)->GetData(); }

		LPVOID GetAt(int iIndex) const;
		/**
		 * @brief 执行 operator[] 运算
		 * @details 用于执行 operator[] 运算。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nIndex [in] 索引值
		 * @return LPVOID 返回 执行 operator[] 运算 的结果
		 */
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

		/**
		 * @brief 执行 Assign 操作
		 * @details 用于执行 Assign 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param target [in,out] 目标参数
		 * @param text [in] 文本内容
		 * @param nLength [in] Length数值
		 */
		FYUI_API void Assign(std::wstring& target, std::wstring_view text, int nLength = -1);
		/**
		 * @brief 执行 CompareNoCase 操作
		 * @details 用于执行 CompareNoCase 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lhs [in] lhs参数
		 * @param rhs [in] 用于比较或复制的源对象
		 * @return int 返回对应的数值结果
		 */
		FYUI_API int CompareNoCase(std::wstring_view lhs, std::wstring_view rhs);
		/**
		 * @brief 执行 EqualsNoCase 操作
		 * @details 用于执行 EqualsNoCase 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lhs [in] lhs参数
		 * @param rhs [in] 用于比较或复制的源对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		FYUI_API bool EqualsNoCase(std::wstring_view lhs, std::wstring_view rhs);
		/**
		 * @brief 执行 MakeUpper 操作
		 * @details 用于执行 MakeUpper 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in,out] 文本内容
		 */
		FYUI_API void MakeUpper(std::wstring& text);
		/**
		 * @brief 执行 MakeLower 操作
		 * @details 用于执行 MakeLower 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in,out] 文本内容
		 */
		FYUI_API void MakeLower(std::wstring& text);
		/**
		 * @brief 执行 Left 操作
		 * @details 用于执行 Left 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param nLength [in] Length数值
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::wstring Left(std::wstring_view text, int nLength);
		/**
		 * @brief 执行 Mid 操作
		 * @details 用于执行 Mid 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param iPos [in] 位置值
		 * @param nLength [in] Length数值
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::wstring Mid(std::wstring_view text, int iPos, int nLength = -1);
		/**
		 * @brief 执行 Right 操作
		 * @details 用于执行 Right 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param nLength [in] Length数值
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::wstring Right(std::wstring_view text, int nLength);
		/**
		 * @brief 执行 TrimView 操作
		 * @details 用于执行 TrimView 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::wstring_view TrimView(std::wstring_view text);
		/**
		 * @brief 执行 TrimLeft 操作
		 * @details 用于执行 TrimLeft 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in,out] 文本内容
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::wstring& TrimLeft(std::wstring& text);
		/**
		 * @brief 执行 TrimRight 操作
		 * @details 用于执行 TrimRight 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in,out] 文本内容
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::wstring& TrimRight(std::wstring& text);
		/**
		 * @brief 执行 Trim 操作
		 * @details 用于执行 Trim 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in,out] 文本内容
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::wstring& Trim(std::wstring& text);
		/**
		 * @brief 执行 Find 操作
		 * @details 用于执行 Find 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param ch [in] ch参数
		 * @param iPos [in] 位置值
		 * @return int 返回对应的数值结果
		 */
		FYUI_API int Find(std::wstring_view text, wchar_t ch, int iPos = 0);
		/**
		 * @brief 执行 Find 操作
		 * @details 用于执行 Find 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param sub [in] sub参数
		 * @param iPos [in] 位置值
		 * @return int 返回对应的数值结果
		 */
		FYUI_API int Find(std::wstring_view text, std::wstring_view sub, int iPos = 0);
		/**
		 * @brief 执行 ReverseFind 操作
		 * @details 用于执行 ReverseFind 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param ch [in] ch参数
		 * @return int 返回对应的数值结果
		 */
		FYUI_API int ReverseFind(std::wstring_view text, wchar_t ch);
		/**
		 * @brief 替换All
		 * @details 用于替换All。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in,out] 文本内容
		 * @param from [in] from参数
		 * @param to [in] to参数
		 * @return int 返回对应的数值结果
		 */
		FYUI_API int ReplaceAll(std::wstring& text, std::wstring_view from, std::wstring_view to);
		/**
		 * @brief 执行 TryParseInt 操作
		 * @details 用于执行 TryParseInt 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param value [in,out] 值参数
		 * @param base [in] base参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		FYUI_API bool TryParseInt(std::wstring_view text, int& value, int base = 10);
		/**
		 * @brief 执行 TryParseDword 操作
		 * @details 用于执行 TryParseDword 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param value [in,out] 值参数
		 * @param base [in] base参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		FYUI_API bool TryParseDword(std::wstring_view text, DWORD& value, int base = 10);
		/**
		 * @brief 执行 TryParseDouble 操作
		 * @details 用于执行 TryParseDouble 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param value [in,out] 值参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		FYUI_API bool TryParseDouble(std::wstring_view text, double& value);
		/**
		 * @brief 解析Bool
		 * @details 用于解析Bool。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		FYUI_API bool ParseBool(std::wstring_view text);
		/**
		 * @brief 执行 TryParseSize 操作
		 * @details 用于执行 TryParseSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param value [in,out] 值参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		FYUI_API bool TryParseSize(std::wstring_view text, SIZE& value);
		/**
		 * @brief 执行 TryParseRect 操作
		 * @details 用于执行 TryParseRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param value [in,out] 值参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		FYUI_API bool TryParseRect(std::wstring_view text, RECT& value);
		/**
		 * @brief 执行 TryParseColor 操作
		 * @details 用于执行 TryParseColor 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param value [in,out] 值参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		FYUI_API bool TryParseColor(std::wstring_view text, DWORD& value);
		/**
		 * @brief 执行 SplitView 操作
		 * @details 用于执行 SplitView 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param delimiter [in] delimiter参数
		 * @param trimItems [in] trim子项集合参数
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::vector<std::wstring_view> SplitView(std::wstring_view text, wchar_t delimiter, bool trimItems = false);
		/**
		 * @brief 解析QuotedAttributes
		 * @details 用于解析QuotedAttributes。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 * @param quote [in] quote参数
		 * @return std::vector<KeyValueView> 返回 解析QuotedAttributes 的结果
		 */
		FYUI_API std::vector<KeyValueView> ParseQuotedAttributes(std::wstring_view text, wchar_t quote = L'\'');
		/**
		 * @brief 执行 Utf8ToUtf16 操作
		 * @details 用于执行 Utf8ToUtf16 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param utf8 [in] utf8参数
		 * @param codePage [in] codePage参数
		 * @return 返回对应的字符串内容
		 */
		FYUI_API std::wstring Utf8ToUtf16(std::string_view utf8, UINT codePage = CP_UTF8);
		/**
		 * @brief 执行 Utf16ToUtf8 操作
		 * @details 用于执行 Utf16ToUtf8 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param utf16 [in] utf16参数
		 * @param codePage [in] codePage参数
		 * @return std::string 返回 执行 Utf16ToUtf8 操作 的结果
		 */
		FYUI_API std::string Utf16ToUtf8(std::wstring_view utf16, UINT codePage = CP_UTF8);

		template <typename... TArgs>
		/**
		 * @brief 执行 Format 操作
		 * @details 用于执行 Format 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param fmt [in] fmt参数
		 * @param args [in,out] args参数
		 * @return 返回对应的字符串内容
		 */
		std::wstring Format(std::wformat_string<TArgs...> fmt, TArgs&&... args)
		{
			return std::format(fmt, std::forward<TArgs>(args)...);
		}
	}

	/**
	 * @brief 执行 StrSplit 操作
	 * @details 用于执行 StrSplit 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param text [in] 文本内容
	 * @param sp [in] sp参数
	 * @return 返回对应的字符串内容
	 */
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
		/**
		 * @brief 构造 CWaitCursor 对象
		 * @details 用于构造 CWaitCursor 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CWaitCursor();
		/**
		 * @brief 析构 CWaitCursor 对象
		 * @details 用于析构 CWaitCursor 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CWaitCursor();

	protected:
		HCURSOR m_hOrigCursor;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CDuiVariant : public VARIANT
	{
	public:
		/**
		 * @brief 构造 CDuiVariant 对象
		 * @details 用于构造 CDuiVariant 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CDuiVariant() 
		{ 
			VariantInit(this); 
		}
		/**
		 * @brief 构造 CDuiVariant 对象
		 * @details 用于构造 CDuiVariant 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param i [in] i参数
		 */
		CDuiVariant(int i)
		{
			VariantInit(this);
			this->vt = VT_I4;
			this->intVal = i;
		}
		/**
		 * @brief 构造 CDuiVariant 对象
		 * @details 用于构造 CDuiVariant 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param f [in] f参数
		 */
		CDuiVariant(float f)
		{
			VariantInit(this);
			this->vt = VT_R4;
			this->fltVal = f;
		}
		/**
		 * @brief 构造 CDuiVariant 对象
		 * @details 用于构造 CDuiVariant 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param s [in] s参数
		 */
		CDuiVariant(LPOLESTR s)
		{
			VariantInit(this);
			this->vt = VT_BSTR;
			this->bstrVal = s;
		}
		/**
		 * @brief 构造 CDuiVariant 对象
		 * @details 用于构造 CDuiVariant 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param disp [in] disp参数
		 */
		CDuiVariant(IDispatch *disp)
		{
			VariantInit(this);
			this->vt = VT_DISPATCH;
			this->pdispVal = disp;
		}

		/**
		 * @brief 析构 CDuiVariant 对象
		 * @details 用于析构 CDuiVariant 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CDuiVariant() 
		{ 
			VariantClear(this); 
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////
	//
	/**
	 * @brief 执行 w2a 操作
	 * @details 用于执行 w2a 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param lpszSrc [in] lpszSrc参数
	 * @param CodePage [in] CodePage参数
	 * @return char* 返回结果对象指针，失败时返回 nullptr
	 */
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



	/**
	 * @brief 执行 a2w 操作
	 * @details 用于执行 a2w 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param lpszSrc [in] lpszSrc参数
	 * @param CodePage [in] CodePage参数
	 * @return wchar_t* 返回结果对象指针，失败时返回 nullptr
	 */
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
