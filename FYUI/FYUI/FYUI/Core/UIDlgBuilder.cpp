#include "pch.h"
#include "UIDlgBuilder.h"
#include "UIResourceData.h"

namespace FYUI
{
	namespace
	{
		bool EqualsNoCase(std::wstring_view lhs, std::wstring_view rhs)
		{
			return StringUtil::CompareNoCase(lhs, rhs) == 0;
		}

		int ParseInt(std::wstring_view text)
		{
			int value = 0;
			StringUtil::TryParseInt(text, value);
			return value;
		}

		DWORD ParseHex(std::wstring_view text)
		{
			DWORD value = 0;
			StringUtil::TryParseColor(text, value);
			return value;
		}

		void ParseSizePair(std::wstring_view text, int& first, int& second)
		{
			SIZE size = {};
			if (StringUtil::TryParseSize(text, size)) {
				first = size.cx;
				second = size.cy;
			}
			else {
				first = 0;
				second = 0;
			}
		}

		RECT ParseRect(std::wstring_view text)
		{
			RECT rc = { 0 };
			StringUtil::TryParseRect(text, rc);
			return rc;
		}

		RenderBackendType ParseRenderBackend(std::wstring_view pstrValue)
		{
			if (pstrValue.empty()) {
				return RenderBackendAuto;
			}
			if (EqualsNoCase(pstrValue, L"gdi")) {
				return RenderBackendGDI;
			}
			if (EqualsNoCase(pstrValue, L"direct2d") || EqualsNoCase(pstrValue, L"d2d")) {
				return RenderBackendDirect2D;
			}
			return RenderBackendAuto;
		}

		Direct2DRenderMode ParseDirect2DRenderMode(std::wstring_view pstrValue)
		{
			if (pstrValue.empty()) {
				return Direct2DRenderModeAuto;
			}
			if (EqualsNoCase(pstrValue, L"hardware") || EqualsNoCase(pstrValue, L"gpu")) {
				return Direct2DRenderModeHardware;
			}
			if (EqualsNoCase(pstrValue, L"software") || EqualsNoCase(pstrValue, L"cpu")) {
				return Direct2DRenderModeSoftware;
			}
			return Direct2DRenderModeAuto;
		}
	}

	CDialogBuilder::CDialogBuilder() : m_pCallback(NULL), m_pstrtype(NULL)
	{
		m_instance = NULL;
	}

	CControlUI* CDialogBuilder::Create(STRINGorID xml, std::wstring_view type, IDialogBuilderCallback* pCallback, 
		CPaintManagerUI* pManager, CControlUI* pParent)
	{
		//璧勬簮ID涓?-65535锛屼袱涓瓧鑺傦紱瀛楃涓叉寚閽堜负4涓瓧鑺?
		//瀛楃涓蹭互<寮€澶磋涓烘槸XML瀛楃涓诧紝鍚﹀垯璁や负鏄疿ML鏂囦欢
		if(xml.IsString() && !xml.view().empty() && xml.view().front() != _T('<')) {
			const std::wstring_view xmlpath = CResourceManager::GetInstance()->GetXmlPath(xml.view());
			if (!xmlpath.empty()) {
				const std::wstring xmlpathStorage(xmlpath);
				xml = STRINGorID(xmlpathStorage);
			}
		}

		HINSTANCE dll_instence = m_instance ? m_instance : CPaintManagerUI::GetResourceDll();
		if( !LoadMarkupDocument(m_xml, xml, type, dll_instence, xml.IsString()) ) return NULL;
		m_pCallback = pCallback;
		m_typeStorage.assign(type);
		m_pstrtype = m_typeStorage.empty() ? nullptr : m_typeStorage.c_str();

		return Create(pCallback, pManager, pParent);
	}

	CControlUI* CDialogBuilder::Create(IDialogBuilderCallback* pCallback, CPaintManagerUI* pManager, CControlUI* pParent)
	{
		m_pCallback = pCallback;
		CMarkupNode root = m_xml.GetRoot();
		if( !root.IsValid() ) return NULL;

		if( pManager ) {
			std::wstring_view pstrClass;
			int nAttributes = 0;
			std::wstring_view pstrName;
			std::wstring_view pstrValue;
			for( CMarkupNode node = root.GetChild() ; node.IsValid(); node = node.GetSibling() ) {
				pstrClass = node.GetName();
				if( EqualsNoCase(pstrClass, L"Image") ) {
					nAttributes = node.GetAttributeCount();
					std::wstring_view pImageName;
					std::wstring_view pImageResType;
					bool shared = false;
					DWORD mask = 0;
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if( EqualsNoCase(pstrName, L"name") ) {
							pImageName = pstrValue;
						}
						else if( EqualsNoCase(pstrName, L"restype") ) {
							pImageResType = pstrValue;
						}
						else if( EqualsNoCase(pstrName, L"mask") ) {
							mask = ParseHex(pstrValue);
						}
						else if( EqualsNoCase(pstrName, L"shared") ) {
							shared = EqualsNoCase(pstrValue, L"true");
						}
					}
					if( !pImageName.empty() ) pManager->AddImage(pImageName, pImageResType, mask, false, shared);
				}
				else if( EqualsNoCase(pstrClass, L"Font") ) {
					nAttributes = node.GetAttributeCount();
					int id = -1;
					std::wstring_view pFontName;
					int size = 12;
					bool bold = false;
					bool underline = false;
					bool italic = false;
					bool defaultfont = false;
					bool shared = false;
					bool strikeout = false;
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if( EqualsNoCase(pstrName, L"id") ) {
							id = ParseInt(pstrValue);
						}
						else if( EqualsNoCase(pstrName, L"name") ) {
							pFontName = pstrValue;
						}
						else if( EqualsNoCase(pstrName, L"size") ) {
							size = ParseInt(pstrValue);
						}
						else if( EqualsNoCase(pstrName, L"bold") ) {
							bold = EqualsNoCase(pstrValue, L"true");
						}
						else if( EqualsNoCase(pstrName, L"underline") ) {
							underline = EqualsNoCase(pstrValue, L"true");
						}
						else if( EqualsNoCase(pstrName, L"italic") ) {
							italic = EqualsNoCase(pstrValue, L"true");
						}
						else if (EqualsNoCase(pstrName, L"strikeout")) {
							strikeout = EqualsNoCase(pstrValue, L"true");
						}
						else if( EqualsNoCase(pstrName, L"default") ) {
							defaultfont = EqualsNoCase(pstrValue, L"true");
						}
						else if( EqualsNoCase(pstrName, L"shared") ) {
							shared = EqualsNoCase(pstrValue, L"true");
						}
					}
					if( id >= 0 ) {
						pManager->AddFont(id, pFontName, size, bold, underline, italic, strikeout, shared);
						if( defaultfont ) pManager->SetDefaultFont(pFontName, size, bold, underline, italic, strikeout, shared);
					}
				}
				else if( EqualsNoCase(pstrClass, L"Default") ) {
					nAttributes = node.GetAttributeCount();
					std::wstring_view pControlName;
					std::wstring_view pControlValue;
					bool shared = false;
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if( EqualsNoCase(pstrName, L"name") ) {
							pControlName = pstrValue;
						}
						else if( EqualsNoCase(pstrName, L"value") ) {
							pControlValue = pstrValue;
						}
						else if( EqualsNoCase(pstrName, L"shared") ) {
							shared = EqualsNoCase(pstrValue, L"true");
						}
					}
					if( !pControlName.empty() ) {
						pManager->AddDefaultAttributeList(pControlName, pControlValue, shared);
					}
				}
				else if( EqualsNoCase(pstrClass, L"Style") ) {
					nAttributes = node.GetAttributeCount();
					std::wstring_view pName;
					std::wstring_view pStyle;
					bool shared = false;
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if( EqualsNoCase(pstrName, L"name") ) {
							pName = pstrValue;
						}
						else if( EqualsNoCase(pstrName, L"value") ) {
							pStyle = pstrValue;
						}
						else if( EqualsNoCase(pstrName, L"shared") ) {
							shared = EqualsNoCase(pstrValue, L"true");
						}
					}
					if( !pName.empty() ) {
						pManager->AddStyle(pName, pStyle, shared);
					}
				}
				else if (EqualsNoCase(pstrClass, L"Import")) {
					nAttributes = node.GetAttributeCount();
					std::wstring_view pstrPath;
					for (int i = 0; i < nAttributes; i++) {
						pstrName = node.GetAttributeName(i);
						pstrValue = node.GetAttributeValue(i);
						if (EqualsNoCase(pstrName, L"fontfile")) {
							pstrPath = pstrValue;
						}
					}
					if (!pstrPath.empty()) {
						pManager->AddFontArray(pstrPath);
					}
				}
			}

			pstrClass = root.GetName();
			if( EqualsNoCase(pstrClass, L"Window") ) {
				if( pManager->GetPaintWindow() ) {
					int nAttributes = root.GetAttributeCount();
					for( int i = 0; i < nAttributes; i++ ) {
						pstrName = root.GetAttributeName(i);
						pstrValue = root.GetAttributeValue(i);
						if( EqualsNoCase(pstrName, L"size") ) {
							int cx = 0;
							int cy = 0;
							ParseSizePair(pstrValue, cx, cy);
							pManager->SetInitSize(pManager->ScaleValue(cx), pManager->ScaleValue(cy));
						} 
						else if( EqualsNoCase(pstrName, L"sizebox") ) {
							RECT rcSizeBox = ParseRect(pstrValue);
							pManager->SetSizeBox(rcSizeBox);
						}
						else if( EqualsNoCase(pstrName, L"caption") ) {
							RECT rcCaption = ParseRect(pstrValue);
							pManager->SetCaptionRect(rcCaption);
						}
						else if( EqualsNoCase(pstrName, L"roundcorner") ) {
							int cx = 0;
							int cy = 0;
							ParseSizePair(pstrValue, cx, cy);
							pManager->SetRoundCorner(cx, cy);
						} 
						else if( EqualsNoCase(pstrName, L"mininfo") ) {
							int cx = 0;
							int cy = 0;
							ParseSizePair(pstrValue, cx, cy);
							pManager->SetMinInfo(cx, cy);
						}
						else if( EqualsNoCase(pstrName, L"maxinfo") ) {
							int cx = 0;
							int cy = 0;
							ParseSizePair(pstrValue, cx, cy);
							pManager->SetMaxInfo(cx, cy);
						}
						else if( EqualsNoCase(pstrName, L"showdirty") ) {
							pManager->SetShowUpdateRect(EqualsNoCase(pstrValue, L"true"));
						} 
						else if( EqualsNoCase(pstrName, L"opacity") || EqualsNoCase(pstrName, L"alpha") ) {
							pManager->SetOpacity(ParseInt(pstrValue));
						} 
						else if( EqualsNoCase(pstrName, L"layeredopacity") ) {
							pManager->SetLayeredOpacity(ParseInt(pstrValue));
						} 
						else if( EqualsNoCase(pstrName, L"layered") || EqualsNoCase(pstrName, L"bktrans")) {
							pManager->SetLayered(EqualsNoCase(pstrValue, L"true"));
						}
						else if( EqualsNoCase(pstrName, L"layeredimage") ) {
							pManager->SetLayered(true);
							pManager->SetLayeredImage(pstrValue);
						} 
						else if( EqualsNoCase(pstrName, L"noactivate") ) {
							pManager->SetNoActivate(EqualsNoCase(pstrValue, L"true"));
						}
						else if( EqualsNoCase(pstrName, L"disabledfontcolor") ) {
							pManager->SetDefaultDisabledColor(ParseHex(pstrValue));
						} 
						else if( EqualsNoCase(pstrName, L"defaultfontcolor") ) {
							pManager->SetDefaultFontColor(ParseHex(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"linkfontcolor") ) {
							pManager->SetDefaultLinkFontColor(ParseHex(pstrValue));
						} 
						else if( EqualsNoCase(pstrName, L"linkhoverfontcolor") ) {
							pManager->SetDefaultLinkHoverFontColor(ParseHex(pstrValue));
						} 
						else if( EqualsNoCase(pstrName, L"selectedcolor") ) {
							pManager->SetDefaultSelectedBkColor(ParseHex(pstrValue));
						} 
						else if( EqualsNoCase(pstrName, L"shadowsize") ) {
							pManager->GetShadow()->SetSize(ParseInt(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"shadowsharpness") ) {
							pManager->GetShadow()->SetSharpness(ParseInt(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"shadowdarkness") ) {
							pManager->GetShadow()->SetDarkness(ParseInt(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"shadowposition") ) {
							int cx = 0;
							int cy = 0;
							ParseSizePair(pstrValue, cx, cy);
							pManager->GetShadow()->SetPosition(cx, cy);
						}
						else if( EqualsNoCase(pstrName, L"shadowcolor") ) {
							pManager->GetShadow()->SetColor(ParseHex(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"shadowcorner") ) {
							pManager->GetShadow()->SetShadowCorner(ParseRect(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"shadowimage") ) {
							pManager->GetShadow()->SetImage(std::wstring(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"showshadow") ) {
							pManager->GetShadow()->ShowShadow(EqualsNoCase(pstrValue, L"true"));
						} 
						else if( EqualsNoCase(pstrName, L"gdiplustext") ) {
							pManager->SetUseGdiplusText(EqualsNoCase(pstrValue, L"true"));
						} 
						else if( EqualsNoCase(pstrName, L"renderbackend") ) {
							pManager->SetRenderBackend(ParseRenderBackend(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"direct2drendermode") || EqualsNoCase(pstrName, L"d2drendermode") ) {
							pManager->SetDirect2DRenderMode(ParseDirect2DRenderMode(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"renderdiagnostics") ) {
							pManager->SetRenderDiagnosticsEnabled(EqualsNoCase(pstrValue, L"true"), pManager->IsRenderDiagnosticsDebugTraceEnabled());
						}
						else if( EqualsNoCase(pstrName, L"renderdebugtrace") ) {
							const bool bTrace = EqualsNoCase(pstrValue, L"true");
							pManager->SetRenderDiagnosticsEnabled(pManager->IsRenderDiagnosticsEnabled() || bTrace, bTrace);
						}
						else if( EqualsNoCase(pstrName, L"textrenderinghint") ) {
							pManager->SetGdiplusTextRenderingHint(ParseInt(pstrValue));
						} 
						else if( EqualsNoCase(pstrName, L"tooltiphovertime") ) {
							pManager->SetHoverTime(ParseInt(pstrValue));
						} 
					}
				}
			}
		}
		return _Parse(&root, pParent, pManager);
	}

	CMarkup* CDialogBuilder::GetMarkup()
	{
		return &m_xml;
	}

	void CDialogBuilder::GetLastErrorMessage(wchar_t* pstrMessage, SIZE_T cchMax) const
	{
		return m_xml.GetLastErrorMessage(pstrMessage, cchMax);
	}

	void CDialogBuilder::GetLastErrorLocation(wchar_t* pstrSource, SIZE_T cchMax) const
	{
		return m_xml.GetLastErrorLocation(pstrSource, cchMax);
	}

	CControlUI* CDialogBuilder::_Parse(CMarkupNode* pRoot, CControlUI* pParent, CPaintManagerUI* pManager)
	{

		IContainerUI* pContainer = NULL;
		CControlUI* pReturn = NULL;
		for( CMarkupNode node = pRoot->GetChild() ; node.IsValid(); node = node.GetSibling() ) {
			const std::wstring_view pstrClass = node.GetName();
			if( EqualsNoCase(pstrClass, L"Image") || EqualsNoCase(pstrClass, L"Font") \
				|| EqualsNoCase(pstrClass, L"Default") || EqualsNoCase(pstrClass, L"Style") ) continue;

			CControlUI* pControl = NULL;
			if (EqualsNoCase(pstrClass, L"Import")) continue;
			if( EqualsNoCase(pstrClass, L"Include") ) {
				if( !node.HasAttributes() ) continue;
				int count = 1;
				wchar_t szValue[500] = { 0 };
				SIZE_T cchLen = lengthof(szValue) - 1;
				if ( node.GetAttributeValue(L"count", szValue, cchLen) )
					count = ParseInt(szValue);
				cchLen = lengthof(szValue) - 1;
				if ( !node.GetAttributeValue(L"source", szValue, cchLen) ) continue;
				for ( int i = 0; i < count; i++ ) {
					CDialogBuilder builder;
					if( m_pstrtype != NULL ) { // 浣跨敤璧勬簮dll锛屼粠璧勬簮涓鍙?
						WORD id = static_cast<WORD>(ParseInt(szValue)); 
						pControl = builder.Create((UINT)id, m_pstrtype, m_pCallback, pManager, pParent);
					}
					else {
						pControl = builder.Create(szValue, {}, m_pCallback, pManager, pParent);
					}
				}
				continue;
			}
			else {
				std::wstring strClass;
				strClass = StringUtil::Format(L"C{}UI", pstrClass);
				pControl = dynamic_cast<CControlUI*>(CControlFactory::GetInstance()->CreateControl(strClass));

				// 妫€鏌ユ彃浠?
				if( pControl == NULL ) {
					CStdPtrArray* pPlugins = CPaintManagerUI::GetPlugins();
					LPCREATECONTROL lpCreateControl = NULL;
					for( int i = 0; i < pPlugins->GetSize(); ++i ) {
						lpCreateControl = (LPCREATECONTROL)pPlugins->GetAt(i);
						if( lpCreateControl != NULL ) {
							pControl = lpCreateControl(pstrClass.data());
							if( pControl != NULL ) break;
						}
					}
				}
				// 鍥炴帀鍒涘缓
				if( pControl == NULL && m_pCallback != NULL ) {
					pControl = m_pCallback->CreateControl(pstrClass);
				}
			}

			if( pControl == NULL ) {
#ifdef _DEBUG
				DUITRACE(_T("鏈煡鎺т欢:%s"), pstrClass);
#else
				continue;
#endif
			}

			// Add children
			if( node.HasChildren() ) {
				_Parse(&node, pControl, pManager);
			}
			// Attach to parent
			// 鍥犱负鏌愪簺灞炴€у拰鐖剁獥鍙ｇ浉鍏筹紝姣斿selected锛屽繀椤诲厛Add鍒扮埗绐楀彛
			CTreeViewUI* pTreeView = NULL;
			if( pParent != NULL && pControl != NULL ) {
				CTreeNodeUI* pParentTreeNode = static_cast<CTreeNodeUI*>(pParent->GetInterface(_T("TreeNode")));
				CTreeNodeUI* pTreeNode = static_cast<CTreeNodeUI*>(pControl->GetInterface(_T("TreeNode")));
				pTreeView = static_cast<CTreeViewUI*>(pParent->GetInterface(_T("TreeView")));
				// TreeNode瀛愯妭鐐?
				if(pTreeNode != NULL) {
					if(pParentTreeNode) {
						pTreeView = pParentTreeNode->GetTreeView();
						if(!pParentTreeNode->Add(pTreeNode)) {
							delete pTreeNode;
							pTreeNode = NULL;
							continue;
						}
					}
					else {
						if(pTreeView != NULL) {
							if(!pTreeView->Add(pTreeNode)) {
								delete pTreeNode;
								pTreeNode = NULL;
								continue;
							}
						}
					}
				}
				// TreeNode瀛愭帶浠?
				else if(pParentTreeNode != NULL) {
					pParentTreeNode->GetTreeNodeHoriznotal()->Add(pControl);
				}
				// 鏅€氭帶浠?
				else {
					if( pContainer == NULL ) pContainer = static_cast<IContainerUI*>(pParent->GetInterface(_T("IContainer")));
					
					if( pContainer == NULL ) return NULL;
					if( !pContainer->Add(pControl) ) {
						delete pControl;
						continue;
					}
				}
			}
			
			
			if( pControl == NULL ) continue;

			// Init default attributes
			if( pManager ) {
				if(pTreeView != NULL) {
					pControl->SetManager(pManager, pTreeView, true);
				}
				else {
					pControl->SetManager(pManager, NULL, false);
				}
				const std::wstring_view pDefaultAttributes = pManager->GetDefaultAttributeList(pstrClass);
				if( !pDefaultAttributes.empty() ) {
					pControl->ApplyAttributeList(pDefaultAttributes);
				}
			}
			// Process attributes
			if( node.HasAttributes() ) {
				// Set ordinary attributes
				int nAttributes = node.GetAttributeCount();
				for( int i = 0; i < nAttributes; i++ ) {
					pControl->SetAttribute(node.GetAttributeName(i), node.GetAttributeValue(i));
				}
			}
			if( pManager ) {
				if(pTreeView == NULL) {
					pControl->SetManager(NULL, NULL, false);
				}
			}
			// Return first item
			if( pReturn == NULL ) pReturn = pControl;
		}
		return pReturn;
	}

}

