#include "pch.h"
#include "UIDlgBuilder.h"
#include "UIResourceData.h"

#include <mutex>
#include <unordered_map>

namespace FYUI
{
	namespace
	{
		bool EqualsNoCase(std::wstring_view lhs, std::wstring_view rhs)
		{
			return StringUtil::CompareNoCase(lhs, rhs) == 0;
		}

		// ---------------------------------------------------------------------
		// 进程级 XML AST 缓存（CDialogBuilder 全局共享只读）
		// ---------------------------------------------------------------------
		//
		// 缓存的是 CMarkup 文档对象，对应一份已经从磁盘/资源加载并解析完成的 XML AST。
		// CDialogBuilder 在构造控件树时仅读取 CMarkup，不修改其内部数据，所以同一个
		// 共享 CMarkup 可以被多个窗口、多次 Create 反复消费，跳过磁盘 IO 与 XML 解析。
		// 注意：控件实例本身仍然由每次 Create 单独创建，不在缓存范围。
		//
		// 缓存 key 由三段组成：dll instance + xml 来源标识（路径或资源 ID） + type，
		// 避免相同文件名跨 dll、跨资源类型时互相串扰。
		//
		// 内联 XML（首字符 '<'）以及加载失败的输入不会进入缓存。
		// ---------------------------------------------------------------------
		struct DialogBuilderXmlCache
		{
			static DialogBuilderXmlCache& Instance()
			{
				static DialogBuilderXmlCache s;
				return s;
			}

			std::shared_ptr<CMarkup> GetOrLoad(const std::wstring& key,
				STRINGorID xml, std::wstring_view type, HINSTANCE instance)
			{
				if (!key.empty()) {
					std::lock_guard<std::mutex> lock(m_mutex);
					auto it = m_map.find(key);
					if (it != m_map.end()) {
						return it->second;
					}
				}

				auto markup = std::make_shared<CMarkup>();
				if (!LoadMarkupDocument(*markup, xml, type, instance, xml.IsString())) {
					return nullptr;
				}

				if (key.empty()) {
					return markup;
				}

				std::lock_guard<std::mutex> lock(m_mutex);
				// 二次检查：可能已被并发线程写入
				auto it = m_map.find(key);
				if (it != m_map.end()) {
					return it->second;
				}
				m_map.emplace(key, markup);
				return markup;
			}

			void Clear()
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				m_map.clear();
			}

		private:
			std::mutex m_mutex;
			std::unordered_map<std::wstring, std::shared_ptr<CMarkup>> m_map;
		};

		// 构造缓存 key；返回空串表示禁止缓存（内联 XML 或非法输入）
		std::wstring MakeXmlCacheKey(STRINGorID xml, std::wstring_view type, HINSTANCE instance)
		{
			std::wstring key;
			if (xml.IsString()) {
				const std::wstring_view view = xml.view();
				if (view.empty() || view.front() == L'<') {
					return {};
				}
				key.reserve(view.size() + type.size() + 32);
				key.append(L"path:");
				key.append(view);
			}
			else {
				wchar_t buf[32] = {};
				const auto ordinal = reinterpret_cast<ULONG_PTR>(xml.c_str());
				::swprintf_s(buf, L"id:%llu", static_cast<unsigned long long>(ordinal));
				key.append(buf);
			}
			key.append(L"|type:");
			key.append(type);
			wchar_t inst[32] = {};
			::swprintf_s(inst, L"|inst:%llu",
				static_cast<unsigned long long>(reinterpret_cast<ULONG_PTR>(instance)));
			key.append(inst);
			return key;
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
		// 走 ResourceManager 解析后的真实路径，便于跨语言/皮肤切换；缓存 key 同样基于这一最终路径
		std::wstring xmlPathStorage;
		if(xml.IsString() && !xml.view().empty() && xml.view().front() != _T('<')) {
			const std::wstring_view xmlpath = CResourceManager::GetInstance()->GetXmlPath(xml.view());
			if (!xmlpath.empty()) {
				xmlPathStorage.assign(xmlpath);
				xml = STRINGorID(xmlPathStorage);
			}
		}

		HINSTANCE dll_instence = m_instance ? m_instance : CPaintManagerUI::GetResourceDll();

		m_pSharedXml.reset();
		if (m_bUseGlobalXmlCache) {
			const std::wstring key = MakeXmlCacheKey(xml, type, dll_instence);
			std::shared_ptr<CMarkup> shared = DialogBuilderXmlCache::Instance().GetOrLoad(
				key, xml, type, dll_instence);
			if (shared) {
				m_pSharedXml = std::move(shared);
			}
		}

		// 缓存禁用 / 内联 XML / 缓存内部已加载失败：回退到实例自有 m_xml 现场加载
		if (!m_pSharedXml) {
			if (!LoadMarkupDocument(m_xml, xml, type, dll_instence, xml.IsString())) {
				return NULL;
			}
		}

		m_pCallback = pCallback;
		m_typeStorage.assign(type);
		m_pstrtype = m_typeStorage.empty() ? nullptr : m_typeStorage.c_str();

		return Create(pCallback, pManager, pParent);
	}

	CMarkup& CDialogBuilder::_ActiveMarkup()
	{
		return m_pSharedXml ? *m_pSharedXml : m_xml;
	}

	CControlUI* CDialogBuilder::CreateControlsFromXml(STRINGorID xml, std::wstring_view type,
		IDialogBuilderCallback* pCallback, CPaintManagerUI* pManager, CControlUI* pParent,
		HINSTANCE instance)
	{
		CDialogBuilder builder;
		if (instance != NULL) {
			builder.SetInstance(instance);
		}
		// 全局接口默认启用缓存（与构造默认值一致），显式写出便于阅读
		builder.SetUseGlobalXmlCache(true);
		return builder.Create(xml, type, pCallback, pManager, pParent);
	}

	void CDialogBuilder::ClearXmlCache()
	{
		DialogBuilderXmlCache::Instance().Clear();
	}

	CControlUI* CDialogBuilder::Create(IDialogBuilderCallback* pCallback, CPaintManagerUI* pManager, CControlUI* pParent)
	{
		m_pCallback = pCallback;
		CMarkupNode root = _ActiveMarkup().GetRoot();
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
						pManager->AddFont(id, pFontName, size, bold, underline, italic, strikeout);
						if( defaultfont ) pManager->SetDefaultFont(pFontName, size, bold, underline, italic, strikeout, shared);
					}
				}
				else if( EqualsNoCase(pstrClass, L"Default") ) {
					nAttributes = node.GetAttributeCount();
					std::wstring_view pControlName;
					std::wstring_view pControlValue;
					std::wstring inlineAttrs;
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
						else {
							// New short form: <Default name="X" attr1="v1" attr2="v2" .../>
							// Collect every non-reserved attribute into a single
							// `key="val" key="val"` declaration list. Quotes inside
							// values are escaped as &quot; so they survive the later
							// ApplyAttributeList unescape step.
							if (!inlineAttrs.empty()) inlineAttrs.push_back(L' ');
							inlineAttrs.append(pstrName);
							inlineAttrs.append(L"=\"");
							for (wchar_t ch : pstrValue) {
								if (ch == L'"') inlineAttrs.append(L"&quot;");
								else inlineAttrs.push_back(ch);
							}
							inlineAttrs.push_back(L'"');
						}
					}
					if( !pControlName.empty() ) {
						const std::wstring_view declarations = pControlValue.empty()
							? std::wstring_view(inlineAttrs)
							: pControlValue;
						pManager->AddDefaultAttributeList(pControlName, declarations, shared);
					}
				}
				else if( EqualsNoCase(pstrClass, L"Style") ) {
					nAttributes = node.GetAttributeCount();
					std::wstring_view pName;
					std::wstring_view pStyle;
					std::wstring inlineAttrs;
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
						else {
							// New short form: <Style name="X" attr1="v1" attr2="v2" .../>
							// See <Default> branch above for rationale.
							if (!inlineAttrs.empty()) inlineAttrs.push_back(L' ');
							inlineAttrs.append(pstrName);
							inlineAttrs.append(L"=\"");
							for (wchar_t ch : pstrValue) {
								if (ch == L'"') inlineAttrs.append(L"&quot;");
								else inlineAttrs.push_back(ch);
							}
							inlineAttrs.push_back(L'"');
						}
					}
					if( !pName.empty() ) {
						const std::wstring_view declarations = pStyle.empty()
							? std::wstring_view(inlineAttrs)
							: pStyle;
						pManager->AddStyle(pName, declarations);
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
						else if( EqualsNoCase(pstrName, L"bktrans_min_interval_ms") || EqualsNoCase(pstrName, L"layered_present_min_interval_ms")) {
							const int nInterval = ParseInt(pstrValue);
							if (nInterval >= 0) {
								pManager->SetLayeredPresentMinIntervalMs(static_cast<UINT>(nInterval));
							}
						}
						else if( EqualsNoCase(pstrName, L"bktrans_mode") || EqualsNoCase(pstrName, L"layered_present_mode")) {
							// auto / gdi / dcomp，三选一；其余值视为 auto
							if (EqualsNoCase(pstrValue, L"gdi") || EqualsNoCase(pstrValue, L"throttled") || EqualsNoCase(pstrValue, L"gdi_throttled")) {
								pManager->SetLayeredPresentMode(LayeredPresentModeGdiThrottled);
							}
							else if (EqualsNoCase(pstrValue, L"dcomp") || EqualsNoCase(pstrValue, L"directcomposition")) {
								pManager->SetLayeredPresentMode(LayeredPresentModeDComp);
							}
							else {
								pManager->SetLayeredPresentMode(LayeredPresentModeAuto);
							}
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
						else if( EqualsNoCase(pstrName, L"shadowcornerradius") ) {
							pManager->GetShadow()->SetCornerRadius(ParseInt(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"shadowimage") ) {
							pManager->GetShadow()->SetImage(std::wstring(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"showshadow") ) {
							pManager->GetShadow()->ShowShadow(EqualsNoCase(pstrValue, L"true"));
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
						else if( EqualsNoCase(pstrName, L"tooltiphovertime") ) {
							pManager->SetHoverTime(ParseInt(pstrValue));
						}
						else if( EqualsNoCase(pstrName, L"tool_tip_mode") ) {
							// 值为 white_bubbles / blcak_bubbles，默认 blcak_bubbles
							if (EqualsNoCase(pstrValue, L"white_bubbles")) {
								pManager->SetToolTipMode(WhiteBubbles);
							}
							else {
								pManager->SetToolTipMode(BlcakBubbles);
							}
						}
					}
				}
			}
		}
		const std::wstring_view rootClass = root.GetName();
		if (!EqualsNoCase(rootClass, L"Window")) {
			CMarkupNode rootNode = root;
			return _ParseControlNode(rootNode, pParent, pManager);
		}

		return _Parse(&root, pParent, pManager);
	}

	CMarkup* CDialogBuilder::GetMarkup()
	{
		return &_ActiveMarkup();
	}

	void CDialogBuilder::GetLastErrorMessage(wchar_t* pstrMessage, SIZE_T cchMax) const
	{
		// 错误信息在共享 CMarkup 上同样可读取（仅读访问，不会修改 CMarkup）
		CMarkup& xml = const_cast<CDialogBuilder*>(this)->_ActiveMarkup();
		return xml.GetLastErrorMessage(pstrMessage, cchMax);
	}

	void CDialogBuilder::GetLastErrorLocation(wchar_t* pstrSource, SIZE_T cchMax) const
	{
		CMarkup& xml = const_cast<CDialogBuilder*>(this)->_ActiveMarkup();
		return xml.GetLastErrorLocation(pstrSource, cchMax);
	}

	CControlUI* CDialogBuilder::_ParseControlNode(CMarkupNode& node, CControlUI* pParent, CPaintManagerUI* pManager)
	{
		const std::wstring_view pstrClass = node.GetName();
		if( EqualsNoCase(pstrClass, L"Image") || EqualsNoCase(pstrClass, L"Font")
			|| EqualsNoCase(pstrClass, L"Default") || EqualsNoCase(pstrClass, L"Style")
			|| EqualsNoCase(pstrClass, L"Import") ) {
			return NULL;
		}

		CControlUI* pControl = NULL;
		if( EqualsNoCase(pstrClass, L"Include") ) {
			if( !node.HasAttributes() ) return NULL;
			int count = 1;
			wchar_t szValue[500] = { 0 };
			SIZE_T cchLen = lengthof(szValue) - 1;
			if ( node.GetAttributeValue(L"count", szValue, cchLen) )
				count = ParseInt(szValue);
			cchLen = lengthof(szValue) - 1;
			if ( !node.GetAttributeValue(L"source", szValue, cchLen) ) return NULL;
			for ( int i = 0; i < count; i++ ) {
				CDialogBuilder builder;
				if( m_pstrtype != NULL ) {
					WORD id = static_cast<WORD>(ParseInt(szValue));
					pControl = builder.Create((UINT)id, m_pstrtype, m_pCallback, pManager, pParent);
				}
				else {
					pControl = builder.Create(szValue, {}, m_pCallback, pManager, pParent);
				}
			}
			return NULL;
		}

		std::wstring strClass = StringUtil::Format(L"C{}UI", pstrClass);
		pControl = dynamic_cast<CControlUI*>(CControlFactory::GetInstance()->CreateControl(strClass));

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

		if( pControl == NULL && m_pCallback != NULL ) {
			pControl = m_pCallback->CreateControl(pstrClass);
		}

		if( pControl == NULL ) {
#ifdef _DEBUG
			DUITRACE(_T("鏈煡鎺т欢:%s"), pstrClass);
#else
			return NULL;
#endif
		}

		if( node.HasChildren() ) {
			_Parse(&node, pControl, pManager);
		}

		CTreeViewUI* pTreeView = NULL;
		if( pParent != NULL && pControl != NULL ) {
			CTreeNodeUI* pParentTreeNode = static_cast<CTreeNodeUI*>(pParent->GetInterface(_T("TreeNode")));
			CTreeNodeUI* pTreeNode = static_cast<CTreeNodeUI*>(pControl->GetInterface(_T("TreeNode")));
			pTreeView = static_cast<CTreeViewUI*>(pParent->GetInterface(_T("TreeView")));
			if(pTreeNode != NULL) {
				if(pParentTreeNode) {
					pTreeView = pParentTreeNode->GetTreeView();
					if(!pParentTreeNode->Add(pTreeNode)) {
						delete pTreeNode;
						return NULL;
					}
				}
				else {
					if(pTreeView != NULL) {
						if(!pTreeView->Add(pTreeNode)) {
							delete pTreeNode;
							return NULL;
						}
					}
				}
			}
			else if(pParentTreeNode != NULL) {
				pParentTreeNode->GetTreeNodeHoriznotal()->Add(pControl);
			}
			else {
				IContainerUI* pContainer = static_cast<IContainerUI*>(pParent->GetInterface(_T("IContainer")));
				if( pContainer == NULL ) return NULL;
				if( !pContainer->Add(pControl) ) {
					delete pControl;
					return NULL;
				}
			}
		}

		if( pControl == NULL ) return NULL;

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

		if( node.HasAttributes() ) {
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

		return pControl;
	}

	CControlUI* CDialogBuilder::_Parse(CMarkupNode* pRoot, CControlUI* pParent, CPaintManagerUI* pManager)
	{
		CControlUI* pReturn = NULL;
		for( CMarkupNode node = pRoot->GetChild() ; node.IsValid(); node = node.GetSibling() ) {
			CControlUI* pControl = _ParseControlNode(node, pParent, pManager);
			if( pControl == NULL ) continue;
			if( pReturn == NULL ) pReturn = pControl;
		}
		return pReturn;
	}

}

