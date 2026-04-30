#pragma once
#pragma once

#include <map>

namespace FYUI 
{
	class FYUI_API IQueryControlText
	{
	public:
		/**
		 * @brief 执行 QueryControlText 操作
		 * @details 用于执行 QueryControlText 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpstrId [in] lpstrId参数
		 * @param lpstrType [in] lpstr类型参数
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring QueryControlText(std::wstring_view lpstrId, std::wstring_view lpstrType) = 0;
	};

	class FYUI_API CResourceManager
	{
	private:
		/**
		 * @brief 构造 CResourceManager 对象
		 * @details 用于构造 CResourceManager 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CResourceManager(void);
		/**
		 * @brief 析构 CResourceManager 对象
		 * @details 用于析构 CResourceManager 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CResourceManager(void);

	public:
		/**
		 * @brief 获取Instance
		 * @details 用于获取Instance。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CResourceManager* 返回结果对象指针，失败时返回 nullptr
		 */
		static CResourceManager* GetInstance()
		{
			static CResourceManager * p = new CResourceManager;
			return p;
		};	
		/**
		 * @brief 执行 Release 操作
		 * @details 用于执行 Release 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Release(void) { delete this; }

	public:
		BOOL LoadResource(STRINGorID xml, std::wstring_view type = {});
		/**
		 * @brief 加载资源
		 * @details 用于加载资源。具体行为由当前对象状态以及传入参数共同决定。
		 * @param xml [in] xml参数
		 * @param type [in] 类型参数
		 * @return BOOL 返回 加载资源 的结果
		 */
		BOOL LoadResource(const std::wstring& xml, std::wstring_view type = {}) { return LoadResource(STRINGorID(xml.c_str()), type); }
		BOOL LoadResource(CMarkupNode Root);
		/**
		 * @brief 重置资源映射
		 * @details 用于重置资源映射。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetResourceMap();
		/**
		 * @brief 获取图像路径
		 * @details 用于获取图像路径。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpstrId [in] lpstrId参数
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetImagePath(std::wstring_view lpstrId);
		/**
		 * @brief 获取Xml路径
		 * @details 用于获取Xml路径。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpstrId [in] lpstrId参数
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetXmlPath(std::wstring_view lpstrId);

	public:
		/**
		 * @brief 设置Language
		 * @details 用于设置Language。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrLanguage [in] Language字符串
		 */
		void SetLanguage(std::wstring_view pstrLanguage) { m_sLauguage = pstrLanguage; }
		void SetLanguage(const std::wstring& language) { m_sLauguage = language; }
		std::wstring_view GetLanguage() { return m_sLauguage; }
		BOOL LoadLanguage(std::wstring_view pstrXml);
		/**
		 * @brief 加载Language
		 * @details 用于加载Language。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrXml [in] Xml字符串
		 * @return BOOL 返回 加载Language 的结果
		 */
		BOOL LoadLanguage(const std::wstring& pstrXml) { return LoadLanguage(std::wstring_view(pstrXml)); }

	public:
		void SetTextQueryInterface(IQueryControlText* pInterface) { m_pQuerypInterface = pInterface; }
		std::wstring GetText(std::wstring_view lpstrId, std::wstring_view lpstrType = {});
		/**
		 * @brief 获取文本
		 * @details 用于获取文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpstrId [in] lpstrId参数
		 * @param lpstrType [in] lpstr类型参数
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetText(const std::wstring& lpstrId, std::wstring_view lpstrType = {}) { return GetText(std::wstring_view(lpstrId), lpstrType); }
		std::wstring GetText(const std::wstring& lpstrId, const std::wstring& lpstrType) { return GetText(std::wstring_view(lpstrId), std::wstring_view(lpstrType)); }
		void ReloadText();
		/**
		 * @brief 重置文本映射
		 * @details 用于重置文本映射。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetTextMap();

	private:
		std::map<std::wstring, std::wstring, std::less<>> m_mTextResourceHashMap;
		IQueryControlText*	m_pQuerypInterface;
		std::map<std::wstring, std::wstring, std::less<>> m_mImageHashMap;
		std::map<std::wstring, std::wstring, std::less<>> m_mXmlHashMap;
		CMarkup m_xml;
		std::wstring m_sLauguage;
		std::map<std::wstring, std::wstring, std::less<>> m_mTextHashMap;
	};

}


