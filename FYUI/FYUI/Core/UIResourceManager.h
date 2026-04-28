#pragma once
#pragma once

#include <map>

namespace FYUI 
{
	class FYUI_API IQueryControlText
	{
	public:
		virtual std::wstring QueryControlText(std::wstring_view lpstrId, std::wstring_view lpstrType) = 0;
	};

	class FYUI_API CResourceManager
	{
	private:
		CResourceManager(void);
		~CResourceManager(void);

	public:
		static CResourceManager* GetInstance()
		{
			static CResourceManager * p = new CResourceManager;
			return p;
		};	
		void Release(void) { delete this; }

	public:
		BOOL LoadResource(STRINGorID xml, std::wstring_view type = {});
		BOOL LoadResource(const std::wstring& xml, std::wstring_view type = {}) { return LoadResource(STRINGorID(xml.c_str()), type); }
		BOOL LoadResource(CMarkupNode Root);
		void ResetResourceMap();
		std::wstring_view GetImagePath(std::wstring_view lpstrId);
		std::wstring_view GetXmlPath(std::wstring_view lpstrId);

	public:
		void SetLanguage(std::wstring_view pstrLanguage) { m_sLauguage = pstrLanguage; }
		void SetLanguage(const std::wstring& language) { m_sLauguage = language; }
		std::wstring_view GetLanguage() { return m_sLauguage; }
		BOOL LoadLanguage(std::wstring_view pstrXml);
		BOOL LoadLanguage(const std::wstring& pstrXml) { return LoadLanguage(std::wstring_view(pstrXml)); }

	public:
		void SetTextQueryInterface(IQueryControlText* pInterface) { m_pQuerypInterface = pInterface; }
		std::wstring GetText(std::wstring_view lpstrId, std::wstring_view lpstrType = {});
		std::wstring GetText(const std::wstring& lpstrId, std::wstring_view lpstrType = {}) { return GetText(std::wstring_view(lpstrId), lpstrType); }
		std::wstring GetText(const std::wstring& lpstrId, const std::wstring& lpstrType) { return GetText(std::wstring_view(lpstrId), std::wstring_view(lpstrType)); }
		void ReloadText();
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


