#include "pch.h"
#include "UIMakup.h"
#include "UIResourceData.h"
#include <cwctype>

namespace FYUI 
{
    namespace
    {
        inline ULONG ClampSizeToULong(SIZE_T value)
        {
            const SIZE_T maxValue = static_cast<SIZE_T>(static_cast<ULONG>(-1));
            if (value > maxValue) {
                return static_cast<ULONG>(-1);
            }
            return static_cast<ULONG>(value);
        }

        inline int ClampDwordToInt(DWORD value)
        {
            if (value > static_cast<DWORD>(INT_MAX)) {
                return INT_MAX;
            }
            return static_cast<int>(value);
        }

        std::wstring_view MakeXmlView(const wchar_t* pstrText)
        {
            return pstrText != nullptr ? std::wstring_view(pstrText) : std::wstring_view();
        }

        std::wstring_view ResolveMarkupLoadErrorMessage(BinaryDataLoadStatus status, bool bFromZip)
        {
            if (!bFromZip) {
                switch (status)
                {
                case BinaryDataLoadEmpty:
                    return _T("File is empty");
                case BinaryDataLoadReadFailed:
                    return _T("Could not read file");
                default:
                    return _T("Error opening file");
                }
            }

            switch (status)
            {
            case BinaryDataLoadZipEntryNotFound:
                return _T("Could not find ziped file");
            case BinaryDataLoadEmpty:
                return _T("File is empty");
            case BinaryDataLoadUnzipFailed:
                return _T("Could not unzip file");
            default:
                return _T("Error opening zip file");
            }
        }

	        bool LoadMarkupBytes(std::wstring_view pstrFilename, LPBYTE& pByte, DWORD& dwSize, std::wstring_view& pstrErrorMessage)
	        {
	            pByte = NULL;
	            dwSize = 0;
	            pstrErrorMessage = {};

	            const bool bFromZip = !CPaintManagerUI::GetResourceZip().empty();
	            const std::wstring filename(pstrFilename);
	            const BinaryDataLoadStatus status = LoadBinaryDataFromConfiguredPath(filename.c_str(), pByte, dwSize, false);
	            if (status != BinaryDataLoadSuccess) {
	                pstrErrorMessage = ResolveMarkupLoadErrorMessage(status, bFromZip);
	                return false;
	            }
	            return true;
	        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //
    CMarkupNode::CMarkupNode() : m_pOwner(NULL)
    {
    }

    CMarkupNode::CMarkupNode(CMarkup* pOwner, int iPos) : m_pOwner(pOwner), m_iPos(iPos), m_nAttributes(0)
    {
    }

    CMarkupNode CMarkupNode::GetSibling()
    {
        if( m_pOwner == NULL ) return CMarkupNode();
        ULONG iPos = m_pOwner->m_pElements[m_iPos].iNext;
        if( iPos == 0 ) return CMarkupNode();
        return CMarkupNode(m_pOwner, iPos);
    }

    bool CMarkupNode::HasSiblings() const
    {
        if( m_pOwner == NULL ) return false;
        ULONG iPos = m_pOwner->m_pElements[m_iPos].iNext;
        return iPos > 0;
    }

    CMarkupNode CMarkupNode::GetChild()
    {
        if( m_pOwner == NULL ) return CMarkupNode();
        ULONG iPos = m_pOwner->m_pElements[m_iPos].iChild;
        if( iPos == 0 ) return CMarkupNode();
        return CMarkupNode(m_pOwner, iPos);
    }

    CMarkupNode CMarkupNode::GetChild(std::wstring_view pstrName)
    {
        if( m_pOwner == NULL ) return CMarkupNode();
        ULONG iPos = m_pOwner->m_pElements[m_iPos].iChild;
        while( iPos != 0 ) {
            if( StringUtil::CompareNoCase(MakeXmlView(m_pOwner->m_pstrXML + m_pOwner->m_pElements[iPos].iStart), pstrName) == 0 ) {
                return CMarkupNode(m_pOwner, iPos);
            }
            iPos = m_pOwner->m_pElements[iPos].iNext;
        }
        return CMarkupNode();
    }

    bool CMarkupNode::HasChildren() const
    {
        if( m_pOwner == NULL ) return false;
        return m_pOwner->m_pElements[m_iPos].iChild != 0;
    }

    CMarkupNode CMarkupNode::GetParent()
    {
        if( m_pOwner == NULL ) return CMarkupNode();
        ULONG iPos = m_pOwner->m_pElements[m_iPos].iParent;
        if( iPos == 0 ) return CMarkupNode();
        return CMarkupNode(m_pOwner, iPos);
    }

    bool CMarkupNode::IsValid() const
    {
        return m_pOwner != NULL;
    }

    std::wstring_view CMarkupNode::GetName() const
    {
        if( m_pOwner == NULL ) return {};
        return MakeXmlView(m_pOwner->m_pstrXML + m_pOwner->m_pElements[m_iPos].iStart);
    }

    std::wstring_view CMarkupNode::GetValue() const
    {
        if( m_pOwner == NULL ) return {};
        return MakeXmlView(m_pOwner->m_pstrXML + m_pOwner->m_pElements[m_iPos].iData);
    }

    std::wstring_view CMarkupNode::GetAttributeName(int iIndex)
    {
        if( m_pOwner == NULL ) return {};
        if( m_nAttributes == 0 ) _MapAttributes();
        if( iIndex < 0 || iIndex >= m_nAttributes ) return {};
        return MakeXmlView(m_pOwner->m_pstrXML + m_aAttributes[iIndex].iName);
    }

    std::wstring_view CMarkupNode::GetAttributeValue(int iIndex)
    {
        if( m_pOwner == NULL ) return {};
        if( m_nAttributes == 0 ) _MapAttributes();
        if( iIndex < 0 || iIndex >= m_nAttributes ) return {};
        return MakeXmlView(m_pOwner->m_pstrXML + m_aAttributes[iIndex].iValue);
    }

    std::wstring_view CMarkupNode::GetAttributeValue(std::wstring_view pstrName)
    {
        if( m_pOwner == NULL ) return {};
        if( m_nAttributes == 0 ) _MapAttributes();
        for( int i = 0; i < m_nAttributes; i++ ) {
            if( StringUtil::CompareNoCase(MakeXmlView(m_pOwner->m_pstrXML + m_aAttributes[i].iName), pstrName) == 0 ) {
                return MakeXmlView(m_pOwner->m_pstrXML + m_aAttributes[i].iValue);
            }
        }
        return {};
    }

    bool CMarkupNode::GetAttributeValue(int iIndex, wchar_t* pstrValue, SIZE_T cchMax)
    {
        if( m_pOwner == NULL ) return false;
        if( m_nAttributes == 0 ) _MapAttributes();
        if( iIndex < 0 || iIndex >= m_nAttributes ) return false;
        wcsncpy_s(pstrValue, cchMax, m_pOwner->m_pstrXML + m_aAttributes[iIndex].iValue, _TRUNCATE);
        return true;
    }

    bool CMarkupNode::GetAttributeValue(std::wstring_view pstrName, wchar_t* pstrValue, SIZE_T cchMax)
    {
        if( m_pOwner == NULL ) return false;
        if( m_nAttributes == 0 ) _MapAttributes();
        for( int i = 0; i < m_nAttributes; i++ ) {
            if( StringUtil::CompareNoCase(MakeXmlView(m_pOwner->m_pstrXML + m_aAttributes[i].iName), pstrName) == 0 ) {
                wcsncpy_s(pstrValue, cchMax, m_pOwner->m_pstrXML + m_aAttributes[i].iValue, _TRUNCATE);
                return true;
            }
        }
        return false;
    }

    int CMarkupNode::GetAttributeCount()
    {
        if( m_pOwner == NULL ) return 0;
        if( m_nAttributes == 0 ) _MapAttributes();
        return m_nAttributes;
    }

    bool CMarkupNode::HasAttributes()
    {
        if( m_pOwner == NULL ) return false;
        if( m_nAttributes == 0 ) _MapAttributes();
        return m_nAttributes > 0;
    }

    bool CMarkupNode::HasAttribute(std::wstring_view pstrName)
    {
        if( m_pOwner == NULL ) return false;
        if( m_nAttributes == 0 ) _MapAttributes();
        for( int i = 0; i < m_nAttributes; i++ ) {
            if( StringUtil::CompareNoCase(MakeXmlView(m_pOwner->m_pstrXML + m_aAttributes[i].iName), pstrName) == 0 ) return true;
        }
        return false;
    }

    void CMarkupNode::_MapAttributes()
    {
        m_nAttributes = 0;
        const wchar_t* pstr = m_pOwner->m_pstrXML + m_pOwner->m_pElements[m_iPos].iStart;
        const wchar_t* pstrEnd = m_pOwner->m_pstrXML + m_pOwner->m_pElements[m_iPos].iData;
        pstr += wcslen(pstr) + 1;
        while( pstr < pstrEnd ) {
            m_pOwner->_SkipWhitespace(pstr);
            m_aAttributes[m_nAttributes].iName = ClampSizeToULong(static_cast<SIZE_T>(pstr - m_pOwner->m_pstrXML));
            pstr += wcslen(pstr) + 1;
            m_pOwner->_SkipWhitespace(pstr);
            if( *pstr++ != _T('\"') ) return;

            m_aAttributes[m_nAttributes++].iValue = ClampSizeToULong(static_cast<SIZE_T>(pstr - m_pOwner->m_pstrXML));
            if( m_nAttributes >= MAX_XML_ATTRIBUTES ) return;
            pstr += wcslen(pstr) + 1;
        }
    }


    ///////////////////////////////////////////////////////////////////////////////////////
    //
    //
    //

    CMarkup::CMarkup(std::wstring_view pstrXML)
    {
        m_pstrXML = NULL;
        m_pElements = NULL;
        m_nElements = 0;
        m_bPreserveWhitespace = true;
        if( !pstrXML.empty() ) Load(pstrXML);
    }

    CMarkup::~CMarkup()
    {
        Release();
    }

    bool CMarkup::IsValid() const
    {
        return m_pElements != NULL;
    }

    void CMarkup::SetPreserveWhitespace(bool bPreserve)
    {
        m_bPreserveWhitespace = bPreserve;
    }

    bool CMarkup::Load(std::wstring_view pstrXML)
    {
        Release();
        const SIZE_T cchLen = pstrXML.size() + 1;
        m_pstrXML = static_cast<wchar_t*>(malloc(cchLen * sizeof(wchar_t)));
        if (!pstrXML.empty()) {
            ::CopyMemory(m_pstrXML, pstrXML.data(), pstrXML.size() * sizeof(wchar_t));
        }
        m_pstrXML[pstrXML.size()] = _T('\0');
        bool bRes = _Parse();
        if( !bRes ) Release();
        return bRes;
    }

    bool CMarkup::LoadFromMem(BYTE* pByte, DWORD dwSize, int encoding)
    {
#ifdef _UNICODE
        if (encoding == XMLFILE_ENCODING_UTF8)
        {
            if( dwSize >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF ) 
            {
                pByte += 3; dwSize -= 3;
            }
            const int cbSize = ClampDwordToInt(dwSize);
            const int nWide = ::MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pByte, cbSize, NULL, 0 );

            m_pstrXML = static_cast<wchar_t*>(malloc((static_cast<SIZE_T>(nWide) + 1) * sizeof(wchar_t)));
            ::MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pByte, cbSize, m_pstrXML, nWide );
            m_pstrXML[nWide] = _T('\0');
        }
        else if (encoding == XMLFILE_ENCODING_ASNI)
        {
            const int cbSize = ClampDwordToInt(dwSize);
            const int nWide = ::MultiByteToWideChar( CP_ACP, 0, (LPCSTR)pByte, cbSize, NULL, 0 );

            m_pstrXML = static_cast<wchar_t*>(malloc((static_cast<SIZE_T>(nWide) + 1) * sizeof(wchar_t)));
            ::MultiByteToWideChar( CP_ACP, 0, (LPCSTR)pByte, cbSize, m_pstrXML, nWide );
            m_pstrXML[nWide] = _T('\0');
        }
        else
        {
            if ( dwSize >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
            {
                dwSize = dwSize / 2 - 1;

                if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
                {
                    pByte += 2;

                    for ( DWORD nSwap = 0 ; nSwap < dwSize ; nSwap ++ )
                    {
						CHAR nTemp = pByte[ ( nSwap << 1 ) + 0 ];
                        pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
                        pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
                    }
                }
                else
                {
                    pByte += 2;
                }

                m_pstrXML = static_cast<wchar_t*>(malloc((dwSize + 1)*sizeof(wchar_t)));
                ::CopyMemory( m_pstrXML, pByte, dwSize * sizeof(wchar_t) );
                m_pstrXML[dwSize] = _T('\0');

                pByte -= 2;
            }
        }
#else // !_UNICODE
        if (encoding == XMLFILE_ENCODING_UTF8)
        {
            if( dwSize >= 3 && pByte[0] == 0xEF && pByte[1] == 0xBB && pByte[2] == 0xBF ) 
            {
                pByte += 3; dwSize -= 3;
            }
            const int cbSize = ClampDwordToInt(dwSize);
            const int nWide = ::MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pByte, cbSize, NULL, 0 );

            LPWSTR w_str = static_cast<LPWSTR>(malloc((static_cast<SIZE_T>(nWide) + 1) * sizeof(WCHAR)));
            ::MultiByteToWideChar( CP_UTF8, 0, (LPCSTR)pByte, cbSize, w_str, nWide );
            w_str[nWide] = L'\0';

            const int wide = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)w_str, nWide, NULL, 0, NULL, NULL);

            m_pstrXML = static_cast<wchar_t*>(malloc((static_cast<SIZE_T>(wide) + 1) * sizeof(wchar_t)));
            ::WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)w_str, nWide, m_pstrXML, wide, NULL, NULL);
            m_pstrXML[wide] = _T('\0');

            free(w_str);
        }
        else if (encoding == XMLFILE_ENCODING_UNICODE)
        {
            if ( dwSize >= 2 && ( ( pByte[0] == 0xFE && pByte[1] == 0xFF ) || ( pByte[0] == 0xFF && pByte[1] == 0xFE ) ) )
            {
                dwSize = dwSize / 2 - 1;

                if ( pByte[0] == 0xFE && pByte[1] == 0xFF )
                {
                    pByte += 2;

                    for ( DWORD nSwap = 0 ; nSwap < dwSize ; nSwap ++ )
                    {
						CHAR nTemp = pByte[ ( nSwap << 1 ) + 0 ];
                        pByte[ ( nSwap << 1 ) + 0 ] = pByte[ ( nSwap << 1 ) + 1 ];
                        pByte[ ( nSwap << 1 ) + 1 ] = nTemp;
                    }
                }
                else
                {
                    pByte += 2;
                }

                const int cchWide = ClampDwordToInt(dwSize);
                const int nWide = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)pByte, cchWide, NULL, 0, NULL, NULL);
                m_pstrXML = static_cast<wchar_t*>(malloc((static_cast<SIZE_T>(nWide) + 1) * sizeof(wchar_t)));
                ::WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)pByte, cchWide, m_pstrXML, nWide, NULL, NULL);
                m_pstrXML[nWide] = _T('\0');

                pByte -= 2;
            }
        }
        else
        {
            m_pstrXML = static_cast<wchar_t*>(malloc((dwSize + 1)*sizeof(wchar_t)));
            ::CopyMemory( m_pstrXML, pByte, dwSize * sizeof(wchar_t) );
            m_pstrXML[dwSize] = _T('\0');
        }
#endif // _UNICODE

        bool bRes = _Parse();
        if( !bRes ) Release();
        return bRes;
    }

    bool CMarkup::LoadFromFile(std::wstring_view pstrFilename, int encoding)
    {
        Release();
        BYTE* pByte = NULL;
        DWORD dwSize = 0;
        std::wstring_view pstrErrorMessage;
        if (!LoadMarkupBytes(pstrFilename, pByte, dwSize, pstrErrorMessage)) {
            return _Failed(pstrErrorMessage);
        }
        if (dwSize > 4096 * 1024) {
            delete[] pByte;
            return _Failed(_T("File too large"));
        }
        bool ret = LoadFromMem(pByte, dwSize, encoding);
        delete[] pByte;
        pByte = NULL;
        return ret;
    }

    void CMarkup::Release()
    {
        if( m_pstrXML != NULL ) free(m_pstrXML);
        if( m_pElements != NULL ) free(m_pElements);
        m_pstrXML = NULL;
        m_pElements = NULL;
        m_nElements = 0;
    }

    void CMarkup::GetLastErrorMessage(wchar_t* pstrMessage, SIZE_T cchMax) const
    {
        wcsncpy_s(pstrMessage, cchMax, m_szErrorMsg.data(), _TRUNCATE);
    }

    void CMarkup::GetLastErrorLocation(wchar_t* pstrSource, SIZE_T cchMax) const
    {
        wcsncpy_s(pstrSource, cchMax, m_szErrorXML.data(), _TRUNCATE);
    }

    CMarkupNode CMarkup::GetRoot()
    {
        if( m_nElements == 0 ) return CMarkupNode();
        return CMarkupNode(this, 1);
    }

    bool CMarkup::_Parse()
    {
        _ReserveElement(); // Reserve index 0 for errors
        ::ZeroMemory(m_szErrorMsg.data(), sizeof(m_szErrorMsg));
        ::ZeroMemory(m_szErrorXML.data(), sizeof(m_szErrorXML));
        wchar_t* pstrXML = m_pstrXML;
        return _Parse(pstrXML, 0);
    }

    bool CMarkup::_Parse(wchar_t*& pstrText, ULONG iParent)
    {
        _SkipWhitespace(pstrText);
        ULONG iPrevious = 0;
        for( ; ; ) 
        {
            if( *pstrText == _T('\0') && iParent <= 1 ) return true;
            _SkipWhitespace(pstrText);
            if( *pstrText != _T('<') ) return _Failed(_T("Expected start tag"), pstrText);
            if( pstrText[1] == _T('/') ) return true;
            *pstrText++ = _T('\0');
            _SkipWhitespace(pstrText);
            // Skip comment or processing directive
            if( *pstrText == _T('!') || *pstrText == _T('?') ) {
                wchar_t ch = *pstrText;
                if( *pstrText == _T('!') ) ch = _T('-');
                while( *pstrText != _T('\0') && !(*pstrText == ch && *(pstrText + 1) == _T('>')) ) ++pstrText;
                if( *pstrText != _T('\0') ) pstrText += 2;
                _SkipWhitespace(pstrText);
                continue;
            }
            _SkipWhitespace(pstrText);
            // Fill out element structure
            XMLELEMENT* pEl = _ReserveElement();
            ULONG iPos = ClampSizeToULong(static_cast<SIZE_T>(pEl - m_pElements));
            pEl->iStart = ClampSizeToULong(static_cast<SIZE_T>(pstrText - m_pstrXML));
            pEl->iParent = iParent;
            pEl->iNext = pEl->iChild = 0;
            if( iPrevious != 0 ) m_pElements[iPrevious].iNext = iPos;
            else if( iParent > 0 ) m_pElements[iParent].iChild = iPos;
            iPrevious = iPos;
            // Parse name
            const wchar_t* pstrName = pstrText;
            _SkipIdentifier(pstrText);
            wchar_t* pstrNameEnd = pstrText;
            if( *pstrText == _T('\0') ) return _Failed(_T("Error parsing element name"), pstrText);
            // Parse attributes
            if( !_ParseAttributes(pstrText) ) return false;
            _SkipWhitespace(pstrText);
            if( pstrText[0] == _T('/') && pstrText[1] == _T('>') )
            {
                pEl->iData = ClampSizeToULong(static_cast<SIZE_T>(pstrText - m_pstrXML));
                *pstrText = _T('\0');
                pstrText += 2;
            }
            else
            {
                if( *pstrText != _T('>') ) return _Failed(_T("Expected start-tag closing"), pstrText);
                // Parse node data
                pEl->iData = ClampSizeToULong(static_cast<SIZE_T>(++pstrText - m_pstrXML));
                wchar_t* pstrDest = pstrText;
                if( !_ParseData(pstrText, pstrDest, _T('<')) ) return false;
                // Determine type of next element
                if( *pstrText == _T('\0') && iParent <= 1 ) return true;
                if( *pstrText != _T('<') ) return _Failed(_T("Expected end-tag start"), pstrText);
                if( pstrText[0] == _T('<') && pstrText[1] != _T('/') ) 
                {
                    if( !_Parse(pstrText, iPos) ) return false;
                }
                if( pstrText[0] == _T('<') && pstrText[1] == _T('/') ) 
                {
                    *pstrDest = _T('\0');
                    *pstrText = _T('\0');
                    pstrText += 2;
                    _SkipWhitespace(pstrText);
                    SIZE_T cchName = pstrNameEnd - pstrName;
                    if( wcsncmp(pstrText, pstrName, cchName) != 0 ) return _Failed(_T("Unmatched closing tag"), pstrText);
                    pstrText += cchName;
                    _SkipWhitespace(pstrText);
                    if( *pstrText++ != _T('>') ) return _Failed(_T("Unmatched closing tag"), pstrText);
                }
            }
            *pstrNameEnd = _T('\0');
            _SkipWhitespace(pstrText);
        }
    }

    CMarkup::XMLELEMENT* CMarkup::_ReserveElement()
    {
        if( m_nElements == 0 ) m_nReservedElements = 0;
        if( m_nElements >= m_nReservedElements ) {
            m_nReservedElements += (m_nReservedElements / 2) + 500;
            m_pElements = static_cast<XMLELEMENT*>(realloc(m_pElements, m_nReservedElements * sizeof(XMLELEMENT)));
        }
        return &m_pElements[m_nElements++];
    }

    void CMarkup::_SkipWhitespace(const wchar_t*& pstr) const
    {
        while( *pstr > _T('\0') && *pstr <= _T(' ') ) ++pstr;
    }

    void CMarkup::_SkipWhitespace(wchar_t*& pstr) const
    {
        while( *pstr > _T('\0') && *pstr <= _T(' ') ) ++pstr;
    }

    void CMarkup::_SkipIdentifier(const wchar_t*& pstr) const
    {
        // 灞炴€у彧鑳界敤鑻辨枃锛屾墍浠ヨ繖鏍峰鐞嗘病鏈夐棶棰?
        while( *pstr != _T('\0') && (*pstr == _T('_') || *pstr == _T(':') || std::iswalnum(*pstr) != 0) ) ++pstr;
    }

    void CMarkup::_SkipIdentifier(wchar_t*& pstr) const
    {
        // 灞炴€у彧鑳界敤鑻辨枃锛屾墍浠ヨ繖鏍峰鐞嗘病鏈夐棶棰?
        while( *pstr != _T('\0') && (*pstr == _T('_') || *pstr == _T(':') || std::iswalnum(*pstr) != 0) ) ++pstr;
    }

    bool CMarkup::_ParseAttributes(wchar_t*& pstrText)
    {   
        // 鏃犲睘鎬?
        wchar_t* pstrIdentifier = pstrText;
        if( *pstrIdentifier == _T('/') && *++pstrIdentifier == _T('>') ) return true;
        if( *pstrText == _T('>') ) return true;
        *pstrText++ = _T('\0');
        _SkipWhitespace(pstrText);
        while( *pstrText != _T('\0') && *pstrText != _T('>') && *pstrText != _T('/') ) {
            _SkipIdentifier(pstrText);
            wchar_t* pstrIdentifierEnd = pstrText;
            _SkipWhitespace(pstrText);
            if( *pstrText != _T('=') ) return _Failed(_T("Error while parsing attributes"), pstrText);
            *pstrText++ = _T(' ');
            *pstrIdentifierEnd = _T('\0');
            _SkipWhitespace(pstrText);
            if( *pstrText++ != _T('\"') ) return _Failed(_T("Expected attribute value"), pstrText);
            wchar_t* pstrDest = pstrText;
            if( !_ParseData(pstrText, pstrDest, _T('\"')) ) return false;
            if( *pstrText == _T('\0') ) return _Failed(_T("Error while parsing attribute string"), pstrText);
            *pstrDest = _T('\0');
            if( pstrText != pstrDest ) *pstrText = _T(' ');
            pstrText++;
            _SkipWhitespace(pstrText);
        }
        return true;
    }

    bool CMarkup::_ParseData(wchar_t*& pstrText, wchar_t*& pstrDest, wchar_t cEnd)
    {
        while( *pstrText != _T('\0') && *pstrText != cEnd ) {
            if( *pstrText == _T('&') ) {
                while( *pstrText == _T('&') ) {
                    _ParseMetaChar(++pstrText, pstrDest);
                }
                if (*pstrText == cEnd)
                    break;
            }

            if( *pstrText == _T(' ') ) {
                *pstrDest++ = *pstrText++;
                if( !m_bPreserveWhitespace ) _SkipWhitespace(pstrText);
            }
            else {
                wchar_t* pstrTemp = pstrText + 1;
                while( pstrText < pstrTemp) {
                    *pstrDest++ = *pstrText++;
                }
            }
        }
        // Make sure that MapAttributes() works correctly when it parses
        // over a value that has been transformed.
        wchar_t* pstrFill = pstrDest + 1;
        while( pstrFill < pstrText ) *pstrFill++ = _T(' ');
        return true;
    }

    void CMarkup::_ParseMetaChar(wchar_t*& pstrText, wchar_t*& pstrDest)
    {
        if( pstrText[0] == _T('a') && pstrText[1] == _T('m') && pstrText[2] == _T('p') && pstrText[3] == _T(';') ) {
            *pstrDest++ = _T('&');
            pstrText += 4;
        }
        else if( pstrText[0] == _T('l') && pstrText[1] == _T('t') && pstrText[2] == _T(';') ) {
            *pstrDest++ = _T('<');
            pstrText += 3;
        }
        else if( pstrText[0] == _T('g') && pstrText[1] == _T('t') && pstrText[2] == _T(';') ) {
            *pstrDest++ = _T('>');
            pstrText += 3;
        }
        else if( pstrText[0] == _T('q') && pstrText[1] == _T('u') && pstrText[2] == _T('o') && pstrText[3] == _T('t') && pstrText[4] == _T(';') ) {
            *pstrDest++ = _T('\"');
            pstrText += 5;
        }
        else if( pstrText[0] == _T('a') && pstrText[1] == _T('p') && pstrText[2] == _T('o') && pstrText[3] == _T('s') && pstrText[4] == _T(';') ) {
            *pstrDest++ = _T('\'');
            pstrText += 5;
        }
        else {
            *pstrDest++ = _T('&');
        }
    }

    bool CMarkup::_Failed(std::wstring_view pstrError, const wchar_t* pstrLocation)
    {
        // 淇绗竴琛岋細閿欒娑堟伅鎷疯礉
        // _TRUNCATE 鍛婅瘔鍑芥暟锛氬鏋?pstrError 澶暱锛屽氨鎴柇骞惰嚜鍔ㄨˉ \0
        const size_t cchError = min(pstrError.size(), m_szErrorMsg.size() - 1);
        if (cchError > 0) {
            wmemcpy(m_szErrorMsg.data(), pstrError.data(), cchError);
        }
        m_szErrorMsg[cchError] = _T('\0');

        // 淇绗簩琛岋細閿欒浣嶇疆鎷疯礉
        const wchar_t* pSrcXML = (pstrLocation != NULL) ? pstrLocation : _T("");
        wcsncpy_s(m_szErrorXML.data(), m_szErrorXML.size(), pSrcXML, _TRUNCATE);
        return false; // Always return 'false'
    }

} // namespace DuiLib

