#include "pch.h"
#include "UIResourceManager.h"
#include "UIResourceData.h"

namespace FYUI
{
	CResourceManager::CResourceManager(void)
	{
		m_pQuerypInterface = NULL;

	}

	CResourceManager::~CResourceManager(void)
	{
		ResetResourceMap();
		ResetTextMap();
	}

	BOOL CResourceManager::LoadResource(STRINGorID xml, std::wstring_view type)
	{
		if( !LoadMarkupDocument(m_xml, xml, type, NULL, xml.IsString()) ) return NULL;

		return LoadResource(m_xml.GetRoot());
	}

	BOOL CResourceManager::LoadResource(CMarkupNode Root)
	{
		if( !Root.IsValid() ) return FALSE;

		std::wstring_view pstrClass;
		int nAttributes = 0;
		std::wstring_view pstrName;
		std::wstring_view pstrValue;

		//鍔犺浇鍥剧墖璧勬簮
		std::wstring_view pstrId;
		std::wstring_view pstrPath;
		for( CMarkupNode node = Root.GetChild() ; node.IsValid(); node = node.GetSibling() ) 
		{
			pstrClass = node.GetName();
			CMarkupNode ChildNode = node.GetChild();
			if(ChildNode.IsValid()) LoadResource(node);
			else if (StringUtil::EqualsNoCase(pstrClass, L"Image") && node.HasAttributes())
			{
				//鍔犺浇鍥剧墖璧勬簮
				nAttributes = node.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) 
				{
					pstrName = node.GetAttributeName(i);
					pstrValue = node.GetAttributeValue(i);
					if( StringUtil::EqualsNoCase(pstrName, L"id") ) 
					{
						pstrId = pstrValue;
					}
					else if( StringUtil::EqualsNoCase(pstrName, L"path") ) 
					{
						pstrPath = pstrValue;
					}
				}
				if( pstrId.empty() || pstrPath.empty()) continue;
				if (m_mImageHashMap.find(pstrId) != m_mImageHashMap.end()) continue;
				m_mImageHashMap.emplace(pstrId, pstrPath);
			}
			else if( StringUtil::EqualsNoCase(pstrClass, L"Xml") && node.HasAttributes()) {
				//鍔犺浇XML閰嶇疆鏂囦欢
				nAttributes = node.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) 
				{
					pstrName = node.GetAttributeName(i);
					pstrValue = node.GetAttributeValue(i);
					if( StringUtil::EqualsNoCase(pstrName, L"id") ) 
					{
						pstrId = pstrValue;
					}
					else if( StringUtil::EqualsNoCase(pstrName, L"path") ) 
					{
						pstrPath = pstrValue;
					}
				}
				if( pstrId.empty() || pstrPath.empty()) continue;
				if (m_mXmlHashMap.find(pstrId) != m_mXmlHashMap.end()) continue;
				m_mXmlHashMap.emplace(pstrId, pstrPath);
			}
			else continue;
		}
		return TRUE;
	}

	std::wstring_view CResourceManager::GetImagePath(std::wstring_view lpstrId)
	{
		const auto it = m_mImageHashMap.find(lpstrId);
		return it == m_mImageHashMap.end() ? std::wstring_view() : std::wstring_view(it->second);
	}

	std::wstring_view CResourceManager::GetXmlPath(std::wstring_view lpstrId)
	{
		const auto it = m_mXmlHashMap.find(lpstrId);
		return it == m_mXmlHashMap.end() ? std::wstring_view() : std::wstring_view(it->second);
	}

	void CResourceManager::ResetResourceMap()
	{
		m_mImageHashMap.clear();
		m_mXmlHashMap.clear();
	}

	BOOL CResourceManager::LoadLanguage(std::wstring_view pstrXml)
	{
		CMarkup xml;
		const std::wstring xmlPath(pstrXml);
		if( !LoadMarkupDocument(xml, STRINGorID(xmlPath.c_str()), {}, NULL, true) ) return FALSE;
		CMarkupNode Root = xml.GetRoot();
		if( !Root.IsValid() ) return FALSE;

		std::wstring_view pstrClass;
		int nAttributes = 0;
		std::wstring_view pstrName;
		std::wstring_view pstrValue;

		//鍔犺浇鍥剧墖璧勬簮
		std::wstring_view pstrId;
		std::wstring_view pstrText;
		for( CMarkupNode node = Root.GetChild() ; node.IsValid(); node = node.GetSibling() ) 
		{
			pstrClass = node.GetName();
			if (StringUtil::EqualsNoCase(pstrClass, L"Text") && node.HasAttributes())
			{
				//鍔犺浇鍥剧墖璧勬簮
				nAttributes = node.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) 
				{
					pstrName = node.GetAttributeName(i);
					pstrValue = node.GetAttributeValue(i);
					if( StringUtil::EqualsNoCase(pstrName, L"id") ) 
					{
						pstrId = pstrValue;
					}
					else if( StringUtil::EqualsNoCase(pstrName, L"value") ) 
					{
						pstrText = pstrValue;
					}
				}
				if( pstrId.empty() || pstrText.empty()) continue;

				m_mTextResourceHashMap[std::wstring(pstrId)] = pstrText;
			}
			else continue;
		}

		return TRUE;
	}

	std::wstring CResourceManager::GetText(std::wstring_view lpstrId, std::wstring_view lpstrType)
	{
		if(lpstrId.empty()) return L"";

		const auto it = m_mTextResourceHashMap.find(lpstrId);
		if (it == m_mTextResourceHashMap.end() && m_pQuerypInterface)
		{
			const std::wstring lpText = m_pQuerypInterface->QueryControlText(lpstrId, lpstrType);
			if(!lpText.empty()) {
				return m_mTextResourceHashMap.emplace(lpstrId, lpText).first->second;
			}
		}
		std::wstring Id(lpstrId);
		return it == m_mTextResourceHashMap.end() ? Id : it->second;
	}

	void CResourceManager::ReloadText()
	{
		if(m_pQuerypInterface == NULL) return;
		//閲嶈浇鏂囧瓧鎻忚堪
		for (auto& entry : m_mTextResourceHashMap)
		{
			const std::wstring lpstrText = m_pQuerypInterface->QueryControlText(entry.first, {});
			if(!lpstrText.empty()) {
				entry.second = lpstrText;
			}
		}
	}
	void CResourceManager::ResetTextMap()
	{
		m_mTextResourceHashMap.clear();
		m_mTextHashMap.clear();
	}


} 

