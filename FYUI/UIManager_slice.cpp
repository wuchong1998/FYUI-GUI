#include "pch.h"
#include "UIManager.h"
#include "UIRenderContext.h"
#include "UIRenderLegacyHdc.h"
#include "UIResourceData.h"
#include <zmouse.h>
#include <cwctype>

#include "../TooltipWnd.h"

#pragma warning(push)
#pragma warning(disable:4838 4244)


namespace FYUI {

	/////////////////////////////////////////////////////////////////////////////////////
	//
	//

	static void GetChildWndRect(HWND hWnd, HWND hChildWnd, RECT& rcChildWnd)
	{
		::GetWindowRect(hChildWnd, &rcChildWnd);

		POINT pt;
		pt.x = rcChildWnd.left;
		pt.y = rcChildWnd.top;
		::ScreenToClient(hWnd, &pt);
		rcChildWnd.left = pt.x;
		rcChildWnd.top = pt.y;

		pt.x = rcChildWnd.right;
		pt.y = rcChildWnd.bottom;
		::ScreenToClient(hWnd, &pt);
		rcChildWnd.right = pt.x;
		rcChildWnd.bottom = pt.y;
	}

	static UINT MapKeyState()
	{
		UINT uState = 0;
		if (::GetKeyState(VK_CONTROL) < 0) uState |= MK_CONTROL;
		if (::GetKeyState(VK_LBUTTON) < 0) uState |= MK_LBUTTON;
		if (::GetKeyState(VK_RBUTTON) < 0) uState |= MK_RBUTTON;
		if (::GetKeyState(VK_SHIFT) < 0) uState |= MK_SHIFT;
		if (::GetKeyState(VK_MENU) < 0) uState |= MK_ALT;
		return uState;
	}

	struct TStringMatchViewData
	{
		std::wstring_view value;
	};

	static bool EqualsInsensitive(std::wstring_view lhs, std::wstring_view rhs)
	{
		if (lhs.size() != rhs.size()) {
			return false;
		}

		for (size_t i = 0; i < lhs.size(); ++i) {
			if (towlower(lhs[i]) != towlower(rhs[i])) {
				return false;
			}
		}

		return true;
	}

	static void InvalidateConfiguredResourceMetadata()
	{
		CResourceManager::GetInstance()->ResetResourceMap();
	}

	class CScopedCurrentRenderContext
	{
	public:
		CScopedCurrentRenderContext(const CPaintRenderContext*& pCurrentRenderContext, const CPaintRenderContext& renderContext)
			: m_pCurrentRenderContext(pCurrentRenderContext)
			, m_pPreviousRenderContext(pCurrentRenderContext)
		{
			m_pCurrentRenderContext = &renderContext;
		}

		~CScopedCurrentRenderContext()
		{
			m_pCurrentRenderContext = m_pPreviousRenderContext;
		}

	private:
		const CPaintRenderContext*& m_pCurrentRenderContext;
		const CPaintRenderContext* m_pPreviousRenderContext;
	};

	class CScopedPaintingState
	{
	public:
		CScopedPaintingState(CPaintManagerUI& manager, bool bResetFrameMetrics)
			: m_manager(manager)
		{
			m_manager.SetPainting(true);
			if (bResetFrameMetrics) {
				CRenderEngine::ResetFrameMetrics();
			}
		}

		~CScopedPaintingState()
		{
			if (m_bActive) {
				m_manager.SetPainting(false);
			}
		}

		void Release()
		{
			m_bActive = false;
		}

	private:
		CPaintManagerUI& m_manager;
		bool m_bActive = true;
	};

	class CScopedWindowPaint
	{
	public:
		explicit CScopedWindowPaint(HWND hWnd)
			: m_hWnd(hWnd)
		{
			::ZeroMemory(&m_ps, sizeof(m_ps));
			if (m_hWnd != NULL) {
				m_hDC = ::BeginPaint(m_hWnd, &m_ps);
			}
		}

		~CScopedWindowPaint()
		{
			if (m_hWnd != NULL && m_hDC != NULL) {
				::EndPaint(m_hWnd, &m_ps);
			}
		}

		HDC GetDC() const
		{
			return m_hDC;
		}

		const RECT& GetPaintRect() const
		{
			return m_ps.rcPaint;
		}

	private:
		HWND m_hWnd = NULL;
		HDC m_hDC = NULL;
		PAINTSTRUCT m_ps = {};
	};

	template <typename TRenderBody>
	static bool ExecuteBatchedPaintScope(HDC hTargetDC, const RECT& rcPaint, TRenderBody&& renderBody)
	{
		if (hTargetDC == NULL) {
			return false;
		}

		int iSaveDC = ::SaveDC(hTargetDC);
		CScopedLegacyDirect2DBatch batchScope(hTargetDC, rcPaint);
		const bool bSuccess = renderBody();
		::RestoreDC(hTargetDC, iSaveDC);
		return bSuccess;
	}

	static void ClearBottomUpBitsRect(BYTE* pBits, LONG width, const RECT& rcClient, const RECT& rcPaint)
	{
		if (pBits == nullptr || width <= 0 || rcPaint.right <= rcPaint.left || rcPaint.bottom <= rcPaint.top) {
			return;
		}

		const LONG height = rcClient.bottom - rcClient.top;
		const LONG rowWidth = rcPaint.right - rcPaint.left;
		DWORD* pPixels = reinterpret_cast<DWORD*>(pBits);
		for (LONG y = rcPaint.top; y < rcPaint.bottom; ++y) {
			const LONG rowIndex = height - 1 - (y - rcClient.top);
			if (rowIndex < 0 || rowIndex >= height) {
				continue;
			}
			DWORD* pRow = pPixels + static_cast<size_t>(rowIndex) * static_cast<size_t>(width) + rcPaint.left;
			::ZeroMemory(pRow, static_cast<size_t>(rowWidth) * sizeof(DWORD));
		}
	}

	static void NormalizeBottomUpBitsRectAlpha(BYTE* pBits, LONG width, const RECT& rcClient, const RECT& rcPaint)
	{
		if (pBits == nullptr || width <= 0 || rcPaint.right <= rcPaint.left || rcPaint.bottom <= rcPaint.top) {
			return;
		}

		const LONG height = rcClient.bottom - rcClient.top;
		DWORD* pPixels = reinterpret_cast<DWORD*>(pBits);
		for (LONG y = rcPaint.top; y < rcPaint.bottom; ++y) {
			const LONG rowIndex = height - 1 - (y - rcClient.top);
			if (rowIndex < 0 || rowIndex >= height) {
				continue;
			}
			DWORD* pRow = pPixels + static_cast<size_t>(rowIndex) * static_cast<size_t>(width) + rcPaint.left;
			for (LONG x = rcPaint.left; x < rcPaint.right; ++x, ++pRow) {
				if (((*pRow & 0xFF000000) == 0) && ((*pRow & 0x00FFFFFF) != 0)) {
					*pRow |= 0xFF000000;
				}
			}
		}
	}

	static void ApplyBottomUpAlphaMaskRect(BYTE* pColorBits, const COLORREF* pMaskBits, LONG width, const RECT& rcClient, const RECT& rcPaint)
	{
		if (pColorBits == nullptr || pMaskBits == nullptr || width <= 0 || rcPaint.right <= rcPaint.left || rcPaint.bottom <= rcPaint.top) {
			return;
		}

		const LONG height = rcClient.bottom - rcClient.top;
		DWORD* pColorPixels = reinterpret_cast<DWORD*>(pColorBits);
		for (LONG y = rcPaint.top; y < rcPaint.bottom; ++y) {
			const LONG rowIndex = height - 1 - (y - rcClient.top);
			if (rowIndex < 0 || rowIndex >= height) {
				continue;
			}
			DWORD* pColorRow = pColorPixels + static_cast<size_t>(rowIndex) * static_cast<size_t>(width) + rcPaint.left;
			const COLORREF* pMaskRow = pMaskBits + static_cast<size_t>(rowIndex) * static_cast<size_t>(width) + rcPaint.left;
			for (LONG x = rcPaint.left; x < rcPaint.right; ++x, ++pColorRow, ++pMaskRow) {
				const BYTE alpha = static_cast<BYTE>((*pMaskRow) >> 24);
				const BYTE red = static_cast<BYTE>((*pColorRow) >> 16);
				const BYTE green = static_cast<BYTE>((*pColorRow) >> 8);
				const BYTE blue = static_cast<BYTE>(*pColorRow);
				*pColorRow =
					(static_cast<DWORD>(blue * alpha / 255) << 16) |
					(static_cast<DWORD>(green * alpha / 255) << 8) |
					static_cast<DWORD>(red * alpha / 255) |
					(static_cast<DWORD>(alpha) << 24);
			}
		}
	}

	static void NormalizeOpaquePixels(COLORREF* pBits, LONG width, LONG height)
	{
		if (pBits == nullptr || width <= 0 || height <= 0) {
			return;
		}

		const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
		for (size_t i = 0; i < pixelCount; ++i) {
			if ((pBits[i] & 0x00FFFFFF) != 0) {
				pBits[i] |= 0xFF000000;
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	typedef struct tagFINDTABINFO
	{
		CControlUI* pFocus;
		CControlUI* pLast;
		bool bForward;
		bool bNextIsIt;
	} FINDTABINFO;

	typedef struct tagFINDSHORTCUT
	{
		TCHAR ch;
		bool bPickNext;
	} FINDSHORTCUT;

	typedef struct tagTIMERINFO
	{
		CControlUI* pSender;
		UINT nLocalID;
		HWND hWnd;
		UINT uWinTimer;
		bool bKilled;
	} TIMERINFO;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	tagTImageInfo::tagTImageInfo()
	{
		pImage = NULL;
		hBitmap = NULL;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///
	tagTDrawInfo::tagTDrawInfo()
	{
		Clear();
	}

	tagTDrawInfo::~tagTDrawInfo()
	{
		ClearCachedBitmap();
	}

	void tagTDrawInfo::ClearCachedBitmap()
	{
		if (hCachedScaledBitmap != NULL) {
			CRenderEngine::FreeBitmap(hCachedScaledBitmap);
			hCachedScaledBitmap = NULL;
		}
		hCachedScaledBitmapSource = NULL;
		::ZeroMemory(&rcCachedScaledSource, sizeof(rcCachedScaledSource));
		szCachedScaledBitmap.cx = 0;
		szCachedScaledBitmap.cy = 0;
	}

	void tagTDrawInfo::Parse(std::wstring_view pStrImageView, std::wstring_view pStrModifyView, CPaintManagerUI* pManager)
	{
		const std::wstring imageStorage(pStrImageView);
		const std::wstring modifyStorage(pStrModifyView);
		const wchar_t* pStrImage = imageStorage.c_str();
		const wchar_t* pStrModify = modifyStorage.empty() ? nullptr : modifyStorage.c_str();
		// 1闁靛棔宸a.jpg
		// 2闁靛棔鍏琲le='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0' corner='0,0,0,0' 
		// mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false'
		sDrawString = pStrImage;
		sDrawModify = pStrModify;
		sImageName = pStrImage;

		std::wstring sItem;
		std::wstring sValue;
		LPTSTR pstr = NULL;
		for (int i = 0; i < 2; ++i) {
			if (i == 1) pStrImage = pStrModify;
			if (!pStrImage) continue;
			while (*pStrImage != _T('\0')) {
				sItem.clear();
				sValue.clear();
				while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
				while (*pStrImage != _T('\0') && *pStrImage != _T('=') && *pStrImage > _T(' ')) {
					const wchar_t* pstrTemp = ::CharNext(pStrImage);
					while (pStrImage < pstrTemp) {
						sItem += *pStrImage++;
					}
				}
				while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
				if (*pStrImage++ != _T('=')) break;
				while (*pStrImage > _T('\0') && *pStrImage <= _T(' ')) pStrImage = ::CharNext(pStrImage);
				if (*pStrImage++ != _T('\'')) break;
				while (*pStrImage != _T('\0') && *pStrImage != _T('\'')) {
					const wchar_t* pstrTemp = ::CharNext(pStrImage);
					while (pStrImage < pstrTemp) {
						sValue += *pStrImage++;
					}
				}
				if (*pStrImage++ != _T('\'')) break;
				if (!sValue.empty()) {
					if (sItem == _T("file") || sItem == _T("res")) {
						sImageName = sValue;
					}
					else if (sItem == _T("restype")) {
						sResType = sValue;
					}
					else if (sItem == _T("dest")) {
						rcDest.left = _tcstol(sValue.c_str(), &pstr, 10);
						rcDest.top = _tcstol(pstr + 1, &pstr, 10);
						rcDest.right = _tcstol(pstr + 1, &pstr, 10);
						rcDest.bottom = _tcstol(pstr + 1, &pstr, 10);
						if (pManager != NULL) pManager->ScaleRect(&rcDest);
					}
					else if (sItem == _T("source")) {
						rcSource.left = _tcstol(sValue.c_str(), &pstr, 10);
						rcSource.top = _tcstol(pstr + 1, &pstr, 10);
						rcSource.right = _tcstol(pstr + 1, &pstr, 10);
						rcSource.bottom = _tcstol(pstr + 1, &pstr, 10);
						if (pManager != NULL) pManager->ScaleRect(&rcSource);
					}
					else if (sItem == _T("corner")) {
						rcCorner.left = _tcstol(sValue.c_str(), &pstr, 10);
						rcCorner.top = _tcstol(pstr + 1, &pstr, 10);
						rcCorner.right = _tcstol(pstr + 1, &pstr, 10);
						rcCorner.bottom = _tcstol(pstr + 1, &pstr, 10);
						if (pManager != NULL) pManager->ScaleRect(&rcCorner);
					}
					else if (sItem == _T("mask")) {
						if (sValue[0] == _T('#')) dwMask = _tcstoul(sValue.c_str() + 1, &pstr, 16);
						else dwMask = _tcstoul(sValue.c_str(), &pstr, 16);
					}
					else if (sItem == _T("fade")) {
						uFade = (UINT)_tcstoul(sValue.c_str(), &pstr, 10);
					}
					else if (sItem == _T("rotate")) {
						uRotate = (UINT)_tcstoul(sValue.c_str(), &pstr, 10);
						bGdiplus = true;
					}
					else if (sItem == _T("gdiplus")) {
						bGdiplus = (_tcsicmp(sValue.c_str(), _T("true")) == 0);
					}
					else if (sItem == _T("hole")) {
						bHole = (_tcsicmp(sValue.c_str(), _T("true")) == 0);
					}
					else if (sItem == _T("xtiled")) {
						bTiledX = (_tcsicmp(sValue.c_str(), _T("true")) == 0);
					}
					else if (sItem == _T("ytiled")) {
						bTiledY = (_tcsicmp(sValue.c_str(), _T("true")) == 0);
					}
					else if (sItem == _T("hsl")) {
						bHSL = (_tcsicmp(sValue.c_str(), _T("true")) == 0);
					}
					else if (sItem == _T("size")) {
						szImage.cx = _tcstol(sValue.c_str(), &pstr, 10);
						szImage.cy = _tcstol(pstr + 1, &pstr, 10);
					}
					else if (sItem == _T("align")) {
						sAlign = sValue;
					}
					else if (sItem == _T("padding")) {
						rcPadding.left = _tcstol(sValue.c_str(), &pstr, 10);
						rcPadding.top = _tcstol(pstr + 1, &pstr, 10);
						rcPadding.right = _tcstol(pstr + 1, &pstr, 10);
						rcPadding.bottom = _tcstol(pstr + 1, &pstr, 10);
						//if(pManager != NULL) pManager->ScaleRect(&rcPadding);
					}
				}
				if (*pStrImage++ != _T(' ')) break;
			}
		}

		// 閻犲鍟弳顤奝I閻犙冨缁?		if (pManager != NULL && pManager->GetScale() != 100) {
			std::wstring strName = sImageName;
			StringUtil::MakeLower(strName);
			if (StringUtil::Find(strName, L".svg") == -1)
			{
				std::wstring sScale;
				sScale = StringUtil::Format(L"@{}.", pManager->GetScale());
				StringUtil::ReplaceAll(sImageName, _T("."), sScale);
			}
		}
		//rcDest = pManager->ScaleRect(rcDest);

		//rcCorner = pManager->ScaleRect(rcCorner);
	}
	void tagTDrawInfo::Clear()
	{
		ClearCachedBitmap();
		sDrawString.clear();
		sDrawModify.clear();
		sImageName.clear();
		sResType.clear();

		memset(&rcDest, 0, sizeof(RECT));
		memset(&rcSource, 0, sizeof(RECT));
		memset(&rcCorner, 0, sizeof(RECT));
		memset(&rcPicDest, 0, sizeof(RECT));
		dwMask = 0;
		uFade = 255;
		uRotate = 0;
		bHole = false;
		bTiledX = false;
		bTiledY = false;
		bHSL = false;
		bGdiplus = false;
		bLoaded = false;
		bSvg = false;
		pImageInfo = nullptr;
		szImage.cx = szImage.cy = 0;
		sAlign.clear();
		memset(&rcPadding, 0, sizeof(RECT));
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///


	HPEN m_hUpdateRectPen = NULL;

	HINSTANCE CPaintManagerUI::m_hResourceInstance = NULL;
	std::wstring CPaintManagerUI::m_pStrResourcePath;
	std::wstring CPaintManagerUI::m_pStrResourceZip;
	std::wstring CPaintManagerUI::m_pStrResourceZipPwd;  //Garfield 20160325 閻㈩垽绠戦惁鎴︽儘娴滃儷p闁告牕鎳撹閻?
	HANDLE CPaintManagerUI::m_hResourceZip = NULL;
	bool CPaintManagerUI::m_bCachedResourceZip = true;
	BYTE* CPaintManagerUI::m_cbZipBuf = nullptr;
	int CPaintManagerUI::m_nResType = UILIB_FILE;
	TResInfo CPaintManagerUI::m_SharedResInfo;
	static bool g_bPendingResourceEnvironmentReload = false;
	HINSTANCE CPaintManagerUI::m_hInstance = NULL;
	bool CPaintManagerUI::m_bUseHSL = false;
	short CPaintManagerUI::m_H = 180;
	short CPaintManagerUI::m_S = 100;
	short CPaintManagerUI::m_L = 100;
	CStdPtrArray CPaintManagerUI::m_aPreMessages;
	CStdPtrArray CPaintManagerUI::m_aPlugins;

	namespace
	{
		double QpcDeltaToMilliseconds(LONGLONG nStartQpc, LONGLONG nEndQpc, LONGLONG nFrequency)
		{
			if (nFrequency <= 0 || nEndQpc <= nStartQpc) {
				return 0.0;
			}
			return static_cast<double>(nEndQpc - nStartQpc) * 1000.0 / static_cast<double>(nFrequency);
		}

		struct TNativeWindowBitmapCache
		{
			HWND hWnd = NULL;
			HBITMAP hBitmap = NULL;
			COLORREF* pBits = NULL;
			SIZE szBitmap = { 0, 0 };
			bool bHasContent = false;
		};

		void ReleaseNativeWindowBitmapCacheBitmap(TNativeWindowBitmapCache& cache)
		{
			if (cache.hBitmap != NULL) {
				CRenderEngine::FreeBitmap(cache.hBitmap);
				cache.hBitmap = NULL;
			}
			cache.pBits = NULL;
			cache.szBitmap.cx = 0;
			cache.szBitmap.cy = 0;
			cache.bHasContent = false;
		}

		void DestroyNativeWindowBitmapCache(TNativeWindowBitmapCache*& pCache)
		{
			if (pCache == NULL) {
				return;
			}
			ReleaseNativeWindowBitmapCacheBitmap(*pCache);
			delete pCache;
			pCache = NULL;
		}

		bool EnsureNativeWindowBitmapCacheSlot(CStdPtrArray& aCache, int iIndex, HWND hWnd, TNativeWindowBitmapCache*& pCache)
		{
			while (aCache.GetSize() <= iIndex) {
				if (!aCache.Add(NULL)) {
					pCache = NULL;
					return false;
				}
			}

			pCache = static_cast<TNativeWindowBitmapCache*>(aCache.GetAt(iIndex));
			if (pCache == NULL) {
				pCache = new TNativeWindowBitmapCache;
				if (pCache == NULL || !aCache.SetAt(iIndex, pCache)) {
					delete pCache;
					pCache = NULL;
					return false;
				}
			}

			if (pCache->hWnd != hWnd) {
				ReleaseNativeWindowBitmapCacheBitmap(*pCache);
				pCache->hWnd = hWnd;
			}
			return true;
		}

		bool EnsureNativeWindowBitmapCacheBitmap(TNativeWindowBitmapCache& cache, HDC hReferenceDC, LONG nWidth, LONG nHeight)
		{
			if (nWidth <= 0 || nHeight <= 0 || hReferenceDC == NULL) {
				ReleaseNativeWindowBitmapCacheBitmap(cache);
				return false;
			}
			if (cache.hBitmap != NULL && cache.pBits != NULL &&
				cache.szBitmap.cx == nWidth && cache.szBitmap.cy == nHeight) {
				return true;
			}

			ReleaseNativeWindowBitmapCacheBitmap(cache);
		cache.hBitmap = CreateLegacyARGB32Bitmap(hReferenceDC, nWidth, nHeight, reinterpret_cast<BYTE**>(&cache.pBits));
			if (cache.hBitmap == NULL || cache.pBits == NULL) {
				ReleaseNativeWindowBitmapCacheBitmap(cache);
				return false;
			}

			cache.szBitmap.cx = nWidth;
			cache.szBitmap.cy = nHeight;
			return true;
		}

		bool EnsureCompatibleMemoryDC(HDC hReferenceDC, HDC& hMemoryDC)
		{
			if (hMemoryDC != NULL) {
				return true;
			}
			if (hReferenceDC == NULL) {
				return false;
			}
			hMemoryDC = ::CreateCompatibleDC(hReferenceDC);
			return hMemoryDC != NULL;
		}

		bool NeedsRefreshNativeWindowBitmapCache(const TNativeWindowBitmapCache& cache, HWND hWnd, LONG nWidth, LONG nHeight)
		{
			if (!cache.bHasContent || cache.szBitmap.cx != nWidth || cache.szBitmap.cy != nHeight) {
				return true;
			}
			return ::GetUpdateRect(hWnd, NULL, FALSE) != FALSE;
		}

		bool RefreshNativeWindowBitmapCache(HDC hReferenceDC, HDC& hMemoryDC, HWND hChildWnd, TNativeWindowBitmapCache& cache, LONG nWidth, LONG nHeight)
		{
			if (!EnsureCompatibleMemoryDC(hReferenceDC, hMemoryDC)) {
				return false;
			}
			if (!EnsureNativeWindowBitmapCacheBitmap(cache, hReferenceDC, nWidth, nHeight)) {
				return false;
			}
			if (cache.pBits != NULL) {
				::ZeroMemory(cache.pBits, static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight) * 4);
			}
			HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemoryDC, cache.hBitmap);
			::SendMessage(hChildWnd, WM_PRINT, reinterpret_cast<WPARAM>(hMemoryDC), static_cast<LPARAM>(PRF_CHECKVISIBLE | PRF_CHILDREN | PRF_CLIENT | PRF_OWNED));
			NormalizeOpaquePixels(cache.pBits, nWidth, nHeight);
			::SelectObject(hMemoryDC, hOldBitmap);
			cache.bHasContent = true;
			::ValidateRect(hChildWnd, NULL);
			return true;
		}

		void DrawNativeWindowBitmapCache(HDC hTargetDC, const RECT& rcChildWnd, const RECT& rcPaint, const TNativeWindowBitmapCache& cache)
		{
			if (hTargetDC == NULL || cache.hBitmap == NULL || !cache.bHasContent) {
				return;
			}

			const RECT rcEmptyCorners = { 0, 0, 0, 0 };
			RECT rcChildBitmap = { 0, 0, cache.szBitmap.cx, cache.szBitmap.cy };
		LegacyDrawImage(hTargetDC, cache.hBitmap, rcChildWnd, rcPaint, rcChildBitmap, rcEmptyCorners, true, 255);
		}

		HDC AcquirePresentWindowDC(HWND hWnd, HDC hFallbackTargetDC, bool& bReleasePresentDC)
		{
			bReleasePresentDC = false;
			HDC hPresentDC = ::GetDC(hWnd);
			if (hPresentDC != NULL) {
				bReleasePresentDC = true;
				return hPresentDC;
			}
			return hFallbackTargetDC;
		}

		void ReleasePresentWindowDC(HWND hWnd, HDC hPresentDC, bool bReleasePresentDC)
		{
			if (bReleasePresentDC && hPresentDC != NULL) {
				::ReleaseDC(hWnd, hPresentDC);
			}
		}

		class CScopedPresentWindowDC
		{
		public:
			CScopedPresentWindowDC(HWND hWnd, HDC hFallbackTargetDC)
				: m_hWnd(hWnd)
			{
				m_hDC = AcquirePresentWindowDC(hWnd, hFallbackTargetDC, m_bReleaseDC);
			}

			~CScopedPresentWindowDC()
			{
				ReleasePresentWindowDC(m_hWnd, m_hDC, m_bReleaseDC);
			}

			HDC Get() const
			{
				return m_hDC;
			}

		private:
			HWND m_hWnd = NULL;
			HDC m_hDC = NULL;
			bool m_bReleaseDC = false;
		};

		bool TryGetWindowRectInClient(HWND hRelativeWnd, HWND hWnd, RECT& rcWindow)
		{
			::SetRectEmpty(&rcWindow);
			if (hRelativeWnd == NULL || hWnd == NULL) {
				return false;
			}
			if (!::GetWindowRect(hWnd, &rcWindow)) {
				return false;
			}
			::MapWindowPoints(HWND_DESKTOP, hRelativeWnd, reinterpret_cast<LPPOINT>(&rcWindow), 2);
			return true;
		}

		enum class NativeWindowRenderResult
		{
			Skipped,
			CacheHit,
			CacheRefresh,
			Failed,
		};

		NativeWindowRenderResult RenderNativeWindowBitmapCacheEntry(
			CStdPtrArray& aCache,
			HWND hParentWnd,
			HDC hOffscreenDC,
			HDC& hChildMemDC,
			const RECT& rcPaint,
			int nCacheIndex,
			HWND hChildWnd)
		{
			if (hOffscreenDC == NULL || hParentWnd == NULL || hChildWnd == NULL || !::IsWindowVisible(hChildWnd)) {
				return NativeWindowRenderResult::Skipped;
			}

			RECT rcChildWnd = { 0 };
			if (!TryGetWindowRectInClient(hParentWnd, hChildWnd, rcChildWnd)) {
				return NativeWindowRenderResult::Skipped;
			}

			RECT rcTemp = { 0 };
			if (!::IntersectRect(&rcTemp, &rcPaint, &rcChildWnd)) {
				return NativeWindowRenderResult::Skipped;
			}

			const LONG childWidth = rcChildWnd.right - rcChildWnd.left;
			const LONG childHeight = rcChildWnd.bottom - rcChildWnd.top;
			TNativeWindowBitmapCache* pChildCache = NULL;
			if (!EnsureNativeWindowBitmapCacheSlot(aCache, nCacheIndex, hChildWnd, pChildCache) || pChildCache == NULL) {
				return NativeWindowRenderResult::Failed;
			}

			const bool bNeedsRefresh = NeedsRefreshNativeWindowBitmapCache(*pChildCache, hChildWnd, childWidth, childHeight);
			if (bNeedsRefresh) {
				if (!RefreshNativeWindowBitmapCache(hOffscreenDC, hChildMemDC, hChildWnd, *pChildCache, childWidth, childHeight)) {
					return NativeWindowRenderResult::Failed;
				}
			}

			DrawNativeWindowBitmapCache(hOffscreenDC, rcChildWnd, rcPaint, *pChildCache);
			return bNeedsRefresh ? NativeWindowRenderResult::CacheRefresh : NativeWindowRenderResult::CacheHit;
		}

		void UpdateFontTextMetrics(HDC hDC, TFontInfo* pFontInfo)
		{
			if (hDC == NULL || pFontInfo == NULL || pFontInfo->hFont == NULL) {
				return;
			}

			HFONT hOldFont = (HFONT)::SelectObject(hDC, pFontInfo->hFont);
			::GetTextMetrics(hDC, &pFontInfo->tm);
			::SelectObject(hDC, hOldFont);
		}
	}

	HDC CPaintManagerUI::EnsureMeasureDC()
	{
		if (m_hDcMeasure != NULL && m_hBmpMeasure != NULL) {
			return m_hDcMeasure;
		}

		HWND hReferenceWnd = ::IsWindow(m_hWndPaint) ? m_hWndPaint : NULL;
		HDC hReferenceDC = ::GetDC(hReferenceWnd);
		if (hReferenceDC == NULL) {
			return NULL;
		}

		m_hDcMeasure = ::CreateCompatibleDC(hReferenceDC);
		if (m_hDcMeasure == NULL) {
			::ReleaseDC(hReferenceWnd, hReferenceDC);
			return NULL;
		}

		BYTE* pBits = NULL;
		m_hBmpMeasure = CreateLegacyARGB32Bitmap(hReferenceDC, 1, 1, &pBits);
		if (m_hBmpMeasure == NULL || pBits == NULL) {
			ResetMeasureDC();
			::ReleaseDC(hReferenceWnd, hReferenceDC);
			return NULL;
		}

		m_hOldBmpMeasure = (HBITMAP)::SelectObject(m_hDcMeasure, m_hBmpMeasure);
		if (m_hOldBmpMeasure == NULL) {
			ResetMeasureDC();
			::ReleaseDC(hReferenceWnd, hReferenceDC);
			return NULL;
		}

		::ReleaseDC(hReferenceWnd, hReferenceDC);
		return m_hDcMeasure;
	}

	void CPaintManagerUI::ResetMeasureDC()
	{
		if (m_hDcMeasure != NULL && m_hOldBmpMeasure != NULL) {
			::SelectObject(m_hDcMeasure, m_hOldBmpMeasure);
		}
		m_hOldBmpMeasure = NULL;

		if (m_hBmpMeasure != NULL) {
			CRenderEngine::FreeBitmap(m_hBmpMeasure);
			m_hBmpMeasure = NULL;
		}
		if (m_hDcMeasure != NULL) {
			::DeleteDC(m_hDcMeasure);
			m_hDcMeasure = NULL;
		}
	}

	bool CPaintManagerUI::EnsureMainRenderSurface(HDC hReferenceDC, LONG cx, LONG cy)
	{
		if (!m_bOffscreenPaint) {
			return true;
		}
		if (hReferenceDC == NULL) {
			hReferenceDC = EnsureMeasureDC();
		}
		return m_offscreenSurface.Ensure(hReferenceDC, cx, cy);
	}

	bool CPaintManagerUI::EnsureLayeredBackgroundSurface(HDC hReferenceDC, LONG cx, LONG cy)
	{
		if (hReferenceDC == NULL) {
			hReferenceDC = EnsureMeasureDC();
		}
		return m_backgroundSurface.Ensure(hReferenceDC, cx, cy);
	}

	RECT CPaintManagerUI::GetRootLayoutRect(const RECT& rcClient) const
	{
		RECT rcRoot = rcClient;
		if (m_bLayered) {
			rcRoot.left += m_rcLayeredInset.left;
			rcRoot.top += m_rcLayeredInset.top;
			rcRoot.right -= m_rcLayeredInset.right;
			rcRoot.bottom -= m_rcLayeredInset.bottom;
		}
		return rcRoot;
	}

	void CPaintManagerUI::ResetRenderSurfaces()
	{
		m_offscreenSurface.Reset();
		m_backgroundSurface.Reset();
	}

	void CPaintManagerUI::UpdateControlsNeedingLayout()
	{
		CControlUI* pControl = NULL;
		m_aFoundControls.Empty();
		m_pRoot->FindControl(__FindControlsFromUpdate, NULL, UIFIND_VISIBLE | UIFIND_ME_FIRST | UIFIND_UPDATETEST);
		for (int it = 0; it < m_aFoundControls.GetSize(); it++) {
			pControl = static_cast<CControlUI*>(m_aFoundControls[it]);
			pControl->SetPos(pControl->GetPos(), true);
		}
	}

	bool CPaintManagerUI::HandleLayoutUpdatesForPaint(const RECT& rcClient, bool& bNeedSizeMsg)
	{
		if (m_bUpdateNeeded) {
			m_bUpdateNeeded = false;
			if (!::IsRectEmpty(&rcClient) && !::IsIconic(m_hWndPaint)) {
				if (m_pRoot->IsUpdateNeeded()) {
					ResetRenderSurfaces();
					RECT rcRoot = GetRootLayoutRect(rcClient);
					m_pRoot->SetPos(rcRoot, true);
					bNeedSizeMsg = true;
				}
				else {
					UpdateControlsNeedingLayout();
					bNeedSizeMsg = true;
				}

				if (m_bFirstLayout) {
					m_bFirstLayout = false;
					SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWINIT, 0, 0, false);
					if (m_bLayered && m_bLayeredChanged) {
						Invalidate();
						return false;
					}
					m_shadow.Update(m_hWndPaint);
				}
			}
		}
		else if (m_bLayered && m_bLayeredChanged) {
			if (m_offscreenSurface.GetBits()) {
				const LONG cx = rcClient.right - rcClient.left;
				const LONG cy = rcClient.bottom - rcClient.top;
				::ZeroMemory(m_offscreenSurface.GetBits(), static_cast<size_t>(cx) * static_cast<size_t>(cy) * 4);
			}
			RECT rcRoot = GetRootLayoutRect(rcClient);
			m_pRoot->SetPos(rcRoot, true);
		}
		return true;
	}

	void CPaintManagerUI::AccumulateLayeredUpdateRect(const RECT& rcDirty)
	{
		::UnionRect(&m_rcLayeredUpdate, &m_rcLayeredUpdate, &rcDirty);
	}

	void CPaintManagerUI::ConsumeLayeredUpdateRectForPaint(RECT& rcPaint, const RECT& rcClient)
	{
		AccumulateLayeredUpdateRect(rcPaint);
		if (rcPaint.right > rcClient.right) rcPaint.right = rcClient.right;
		if (rcPaint.bottom > rcClient.bottom) rcPaint.bottom = rcClient.bottom;
		::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
	}

	void CPaintManagerUI::PrepareLayeredPaintRect(RECT& rcPaint, const RECT& rcClient)
	{
		if (!m_bLayered) {
			return;
		}

		DWORD dwExStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
		DWORD dwNewExStyle = dwExStyle | WS_EX_LAYERED;
		if (dwExStyle != dwNewExStyle) {
			::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewExStyle);
		}
		m_bOffscreenPaint = true;
		ConsumeLayeredUpdateRectForPaint(rcPaint, rcClient);
	}

	bool CPaintManagerUI::ExecutePaintFrame(HDC hTargetDC, const TPaintFrameState& paintFrame)
	{
		if (m_bOffscreenPaint) {
			return RenderFrameToMainSurface(hTargetDC, paintFrame)
				&& PresentMainRenderSurface(hTargetDC, paintFrame);
		}

		return RenderFrameDirectToWindow(hTargetDC, paintFrame.rcPaint);
	}

	void CPaintManagerUI::PrepareRenderClientRect(RECT& rcClient)
	{
		ApplyRenderPreferences();
		::GetClientRect(m_hWndPaint, &rcClient);
	}

	LONGLONG CPaintManagerUI::BeginRenderDiagnosticsFrame() const
	{
		if (!m_bRenderDiagnosticsEnabled || m_nRenderPerfFrequency <= 0) {
			return 0;
		}

		LARGE_INTEGER counter = {};
		::QueryPerformanceCounter(&counter);
		return counter.QuadPart;
	}

	bool CPaintManagerUI::PaintEmptyRootFrame()
	{
		CScopedWindowPaint paintScope(m_hWndPaint);
		if (paintScope.GetDC() != NULL) {
		LegacyDrawColor(paintScope.GetDC(), paintScope.GetPaintRect(), 0xFF000000);
		}
		return true;
	}

	bool CPaintManagerUI::PreparePaintFrameRects(TPaintFrameState& paintFrame)
	{
		PrepareRenderClientRect(paintFrame.rcClient);
		return ::GetUpdateRect(m_hWndPaint, &paintFrame.rcPaint, FALSE) != FALSE;
	}

	void CPaintManagerUI::HandlePendingFocusForPaint()
	{
		if (m_bFocusNeeded) {
			SetNextTabControl();
		}
	}

	bool CPaintManagerUI::PreparePaintFrameLayout(TPaintFrameState& paintFrame)
	{
		paintFrame.dwWidth = paintFrame.rcClient.right - paintFrame.rcClient.left;
		paintFrame.dwHeight = paintFrame.rcClient.bottom - paintFrame.rcClient.top;
		if (!HandleLayoutUpdatesForPaint(paintFrame.rcClient, paintFrame.bNeedSizeMsg)) {
			return false;
		}
		PrepareLayeredPaintRect(paintFrame.rcPaint, paintFrame.rcClient);
		return true;
	}

	bool CPaintManagerUI::PreparePaintFrameExecution(TPaintFrameState& paintFrame)
	{
		HandlePendingFocusForPaint();
		paintFrame.nPaintStartQpc = BeginRenderDiagnosticsFrame();
		return PreparePaintFrameLayout(paintFrame);
	}

	bool CPaintManagerUI::ExecutePaintWindowFrame(HDC hTargetDC, const TPaintFrameState& paintFrame) const
	{
		if (hTargetDC == NULL || ShouldSkipPaintExecution(paintFrame)) {
			return false;
		}
		return const_cast<CPaintManagerUI*>(this)->ExecutePaintFrame(hTargetDC, paintFrame);
	}

	bool CPaintManagerUI::PaintPreparedWindowFrame(const TPaintFrameState& paintFrame)
	{
		CScopedWindowPaint paintScope(m_hWndPaint);
		return ExecutePaintWindowFrame(paintScope.GetDC(), paintFrame);
	}

	bool CPaintManagerUI::ShouldSkipPaintExecution(const TPaintFrameState& paintFrame) const
	{
		return ::IsRectEmpty(&paintFrame.rcPaint) && !m_bLayeredChanged && !m_bUpdateNeeded;
	}

	bool CPaintManagerUI::RenderPrintClientFrame(HDC hTargetDC, LPARAM lParam)
	{
		if (m_pRoot == NULL || hTargetDC == NULL) {
			return false;
		}

		RECT rcClient = { 0 };
		PrepareRenderClientRect(rcClient);
		if (!RenderFrameDirectToWindow(hTargetDC, rcClient, false)) {
			return false;
		}
		PrintChildWindows(hTargetDC, lParam);
		return true;
	}

	CPaintRenderContext CPaintManagerUI::CreateWindowRenderContext(HDC hDC, const RECT& rcPaint)
	{
		return CreateCompatRenderContext(this, hDC, rcPaint);
	}

	CPaintRenderContext CPaintManagerUI::CreateMeasureRenderContext(const RECT& rcPaint)
	{
		RenderBackendType activeBackend = GetActiveRenderBackend();
		Direct2DRenderMode activeDirect2DMode = GetActiveDirect2DRenderMode();
		if (const CPaintRenderContext* pCurrentRenderContext = GetCurrentRenderContext()) {
			activeBackend = pCurrentRenderContext->GetActiveBackend();
			activeDirect2DMode = pCurrentRenderContext->GetActiveDirect2DRenderMode();
		}

		return CPaintRenderContext(this, GetMeasureDC(), rcPaint, activeBackend, activeDirect2DMode);
	}

	bool CPaintManagerUI::ExecuteBatchedControlRenderPass(HDC hTargetDC, const RECT& rcPaint, CPaintRenderContext& renderContext, bool bIncludeNativeWindows, bool bIncludePostPaint)
	{
		return ExecuteBatchedPaintScope(hTargetDC, rcPaint, [&]() -> bool {
			RenderControlTree(renderContext, NULL, false);
			if (bIncludeNativeWindows && !RenderNativeWindowsToSurface(hTargetDC, rcPaint)) {
				return false;
			}
			if (bIncludePostPaint) {
				RenderPostPaintControls(renderContext);
			}
			return true;
		});
	}

	bool CPaintManagerUI::ExecuteBatchedDrawInfoPass(HDC hTargetDC, const RECT& rcPaint, TDrawInfo* pDrawInfo)
	{
		if (pDrawInfo == NULL) {
			return false;
		}

		return ExecuteBatchedPaintScope(hTargetDC, rcPaint, [&]() -> bool {
			CRenderClip clip;
			CRenderClipHdc::GenerateClip(hTargetDC, rcPaint, clip);
		LegacyDrawImageInfo(hTargetDC, this, rcPaint, rcPaint, pDrawInfo);
			return true;
		});
	}

	void CPaintManagerUI::RenderControlTree(CPaintRenderContext& renderContext, CControlUI* pStopControl, bool bIncludePostPaint)
	{
		CScopedCurrentRenderContext scopedCurrentRenderContext(m_pCurrentRenderContext, renderContext);
		m_pRoot->Paint(renderContext, pStopControl);

		if (bIncludePostPaint) {
			RenderPostPaintControls(renderContext);
		}
	}

	void CPaintManagerUI::RenderPostPaintControls(CPaintRenderContext& renderContext)
	{
		CScopedCurrentRenderContext scopedCurrentRenderContext(m_pCurrentRenderContext, renderContext);
		for (auto iter : m_mapPostPaintControls)
		{
			CControlUI* pPostPaintControl = static_cast<CControlUI*>(iter.first);
			if (pPostPaintControl != nullptr)
			{
				pPostPaintControl->DoPostPaint(renderContext);
			}
		}
	}

	bool CPaintManagerUI::RenderNativeWindowsToSurface(HDC hOffscreenDC, const RECT& rcPaint)
	{
		if (!m_bLayered || m_aNativeWindow.GetSize() <= 0 || hOffscreenDC == NULL) {
			return true;
		}

		HDC hChildMemDC = NULL;
		for (int i = 0; i < m_aNativeWindow.GetSize(); ) {
			const int nCacheIndex = i;
			HWND hChildWnd = static_cast<HWND>(m_aNativeWindow[nCacheIndex]);
			if (!::IsWindow(hChildWnd)) {
				RemoveNativeWindowEntryAt(nCacheIndex);
				continue;
			}
			++i;
			switch (RenderNativeWindowBitmapCacheEntry(
				m_aNativeWindowCache,
				m_hWndPaint,
				hOffscreenDC,
				hChildMemDC,
				rcPaint,
				nCacheIndex,
				hChildWnd)) {
			case NativeWindowRenderResult::CacheRefresh:
				++m_nNativeWindowCacheRefreshes;
				++m_nSampleNativeWindowCacheRefreshes;
				break;
			case NativeWindowRenderResult::CacheHit:
				++m_nNativeWindowCacheHits;
				++m_nSampleNativeWindowCacheHits;
				break;
			case NativeWindowRenderResult::Skipped:
			case NativeWindowRenderResult::Failed:
			default:
				break;
			}
		}

		if (hChildMemDC != NULL) {
			::DeleteDC(hChildMemDC);
		}
		return true;
	}

	bool CPaintManagerUI::RenderLayeredBackgroundToSurface(HDC hReferenceDC, const TPaintFrameState& paintFrame)
	{
		if (!m_diLayered.sDrawString.empty()) {
			RECT rcLayeredClient = paintFrame.rcClient;
			rcLayeredClient.left += m_rcLayeredInset.left;
			rcLayeredClient.top += m_rcLayeredInset.top;
			rcLayeredClient.right -= m_rcLayeredInset.right;
			rcLayeredClient.bottom -= m_rcLayeredInset.bottom;
			const bool bHadBackgroundSurface = m_backgroundSurface.IsReady();
			if (!EnsureLayeredBackgroundSurface(hReferenceDC, static_cast<LONG>(paintFrame.dwWidth), static_cast<LONG>(paintFrame.dwHeight))) {
				return false;
			}
			if (!bHadBackgroundSurface || m_bLayeredChanged) {
				::ZeroMemory(m_backgroundSurface.GetBits(), paintFrame.dwWidth * paintFrame.dwHeight * 4);
				if (!ExecuteBatchedDrawInfoPass(m_backgroundSurface.GetDC(), rcLayeredClient, &m_diLayered)) {
					return false;
				}
			}
			ApplyBottomUpAlphaMaskRect(
				m_offscreenSurface.GetBits(),
				m_backgroundSurface.GetColorBits(),
				static_cast<LONG>(paintFrame.dwWidth),
				paintFrame.rcClient,
				paintFrame.rcPaint);
		}
		else {
			NormalizeBottomUpBitsRectAlpha(
				m_offscreenSurface.GetBits(),
				static_cast<LONG>(paintFrame.dwWidth),
				paintFrame.rcClient,
				paintFrame.rcPaint);
		}
		return true;
	}

	bool CPaintManagerUI::RenderFrameToMainSurface(HDC hReferenceDC, const TPaintFrameState& paintFrame)
	{
		if (!EnsureMainRenderSurface(hReferenceDC, static_cast<LONG>(paintFrame.dwWidth), static_cast<LONG>(paintFrame.dwHeight))) {
			return false;
		}

		HDC hOffscreenDC = m_offscreenSurface.GetDC();
		if (m_bLayered) {
			ClearBottomUpBitsRect(
				m_offscreenSurface.GetBits(),
				static_cast<LONG>(paintFrame.dwWidth),
				paintFrame.rcClient,
				paintFrame.rcPaint);
		}
		CPaintRenderContext renderContext = m_offscreenSurface.CreateRenderContext(
			this,
			paintFrame.rcPaint,
			GetActiveRenderBackend(),
			GetActiveDirect2DRenderMode());
		return ExecuteBatchedControlRenderPass(hOffscreenDC, paintFrame.rcPaint, renderContext, true, true);
	}

	bool CPaintManagerUI::RenderFrameDirectToWindow(HDC hTargetDC, const RECT& rcPaint, bool bIncludePostPaint)
	{
		CPaintRenderContext renderContext = CreateWindowRenderContext(hTargetDC, rcPaint);
		return ExecuteBatchedControlRenderPass(hTargetDC, rcPaint, renderContext, false, bIncludePostPaint);
	}

	void CPaintManagerUI::PrintChildWindows(HDC hTargetDC, LPARAM lParam)
	{
		if (hTargetDC == NULL || (lParam & PRF_CHILDREN) == 0) {
			return;
		}

		HWND hWndChild = ::GetWindow(m_hWndPaint, GW_CHILD);
		while (hWndChild != NULL) {
			RECT rcPos = { 0 };
			::GetWindowRect(hWndChild, &rcPos);
			::MapWindowPoints(HWND_DESKTOP, m_hWndPaint, reinterpret_cast<LPPOINT>(&rcPos), 2);
			int iSaveChildDC = ::SaveDC(hTargetDC);
			::SetWindowOrgEx(hTargetDC, -rcPos.left, -rcPos.top, NULL);
			::SendMessage(hWndChild, WM_PRINT, reinterpret_cast<WPARAM>(hTargetDC), lParam | PRF_NONCLIENT);
			::RestoreDC(hTargetDC, iSaveChildDC);
			hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
		}
	}

	bool CPaintManagerUI::PresentLayeredSurface(HDC hTargetDC, const TPaintFrameState& paintFrame)
	{
		RECT rcWnd = { 0 };
		::GetWindowRect(m_hWndPaint, &rcWnd);

		BLENDFUNCTION bf = { AC_SRC_OVER, 0, m_nOpacity, AC_SRC_ALPHA };
		POINT ptPos = { rcWnd.left, rcWnd.top };
		SIZE sizeWnd = { static_cast<LONG>(paintFrame.dwWidth), static_cast<LONG>(paintFrame.dwHeight) };
		POINT ptSrc = { 0, 0 };
		CScopedPresentWindowDC presentWindowDC(m_hWndPaint, hTargetDC);

		BOOL bUpdated = UpdateLayeredWindow(
			m_hWndPaint,
			presentWindowDC.Get(),
			&ptPos,
			&sizeWnd,
			m_offscreenSurface.GetDC(),
			&ptSrc,
			0,
			&bf,
			ULW_ALPHA);
		return bUpdated != FALSE;
	}

	void CPaintManagerUI::PresentWindowSurface(HDC hTargetDC, const TPaintFrameState& paintFrame)
	{
		if (hTargetDC == NULL) {
			return;
		}

		if (CRenderEngine::GetActiveRenderBackend() == RenderBackendDirect2D) {
			RECT rcEmptyCorners = { 0, 0, 0, 0 };
		LegacyDrawImage(
				hTargetDC,
				m_offscreenSurface.GetBitmap(),
				paintFrame.rcPaint,
				paintFrame.rcPaint,
				paintFrame.rcPaint,
				rcEmptyCorners,
				false,
				255);
		}
		else {
			::BitBlt(
				hTargetDC,
				paintFrame.rcPaint.left,
				paintFrame.rcPaint.top,
				paintFrame.rcPaint.right - paintFrame.rcPaint.left,
				paintFrame.rcPaint.bottom - paintFrame.rcPaint.top,
				m_offscreenSurface.GetDC(),
				paintFrame.rcPaint.left,
				paintFrame.rcPaint.top,
				SRCCOPY);
		}
	}

	bool CPaintManagerUI::PresentMainRenderSurface(HDC hTargetDC, const TPaintFrameState& paintFrame)
	{
		if (m_bLayered) {
			if (!RenderLayeredBackgroundToSurface(m_offscreenSurface.GetDC(), paintFrame)) {
				return false;
			}
			if (!PresentLayeredSurface(hTargetDC, paintFrame)) {
				return false;
			}
		}
		else {
			PresentWindowSurface(hTargetDC, paintFrame);
		}

		if (m_bShowUpdateRect && !m_bLayered && hTargetDC != NULL) {
			HPEN hOldPen = (HPEN)::SelectObject(hTargetDC, m_hUpdateRectPen);
			::SelectObject(hTargetDC, ::GetStockObject(HOLLOW_BRUSH));
			::Rectangle(
				hTargetDC,
				paintFrame.rcPaint.left,
				paintFrame.rcPaint.top,
				paintFrame.rcPaint.right,
				paintFrame.rcPaint.bottom);
			::SelectObject(hTargetDC, hOldPen);
		}
		return true;
	}

	void CPaintManagerUI::ConsumeRenderFrameMetrics()
	{
		if (!m_bRenderDiagnosticsEnabled) {
			return;
		}

		UINT nDirect2DBatchFlushes = 0;
		UINT nDirect2DStandaloneDraws = 0;
		UINT nLegacyImageFallbackDraws = 0;
		UINT nLegacyTextFallbackDraws = 0;
		UINT nTextLayoutCacheHits = 0;
		UINT nTextLayoutCacheRefreshes = 0;
		UINT nTextMetricsCacheHits = 0;
		UINT nTextMetricsCacheRefreshes = 0;
		UINT nTextNormalizationCacheHits = 0;
		UINT nTextNormalizationCacheRefreshes = 0;
		UINT nHtmlParseCacheHits = 0;
		UINT nHtmlParseCacheRefreshes = 0;
		UINT nHtmlLayoutCacheHits = 0;
		UINT nHtmlLayoutCacheRefreshes = 0;
		UINT nHtmlMetricsCacheHits = 0;
		UINT nHtmlMetricsCacheRefreshes = 0;
		UINT nHtmlHitTestCacheHits = 0;
		UINT nHtmlHitTestCacheRefreshes = 0;
		UINT nHtmlDrawingEffectCacheHits = 0;
		UINT nHtmlDrawingEffectCacheRefreshes = 0;
		UINT nHtmlDirectWriteParseFailures = 0;
		UINT nHtmlDirectWriteLayoutFailures = 0;
		UINT nHtmlDirectWriteRenderFailures = 0;
		CRenderEngine::ConsumeFrameMetrics(
			nDirect2DBatchFlushes,
			nDirect2DStandaloneDraws,
			nLegacyImageFallbackDraws,
			nLegacyTextFallbackDraws,
			nTextLayoutCacheHits,
			nTextLayoutCacheRefreshes,
			nTextMetricsCacheHits,
			nTextMetricsCacheRefreshes,
			nTextNormalizationCacheHits,
			nTextNormalizationCacheRefreshes,
			nHtmlParseCacheHits,
			nHtmlParseCacheRefreshes,
			nHtmlLayoutCacheHits,
			nHtmlLayoutCacheRefreshes,
			nHtmlMetricsCacheHits,
			nHtmlMetricsCacheRefreshes,
			nHtmlHitTestCacheHits,
			nHtmlHitTestCacheRefreshes,
			nHtmlDrawingEffectCacheHits,
			nHtmlDrawingEffectCacheRefreshes,
			nHtmlDirectWriteParseFailures,
			nHtmlDirectWriteLayoutFailures,
			nHtmlDirectWriteRenderFailures);
		m_nDirect2DBatchFlushes += nDirect2DBatchFlushes;
		m_nDirect2DStandaloneDraws += nDirect2DStandaloneDraws;
		m_nSampleDirect2DBatchFlushes += nDirect2DBatchFlushes;
		m_nSampleDirect2DStandaloneDraws += nDirect2DStandaloneDraws;
		m_nLegacyImageFallbackDraws += nLegacyImageFallbackDraws;
		m_nSampleLegacyImageFallbackDraws += nLegacyImageFallbackDraws;
		m_nLegacyTextFallbackDraws += nLegacyTextFallbackDraws;
		m_nSampleLegacyTextFallbackDraws += nLegacyTextFallbackDraws;
		m_nTextNormalizationCacheHits += nTextNormalizationCacheHits;
		m_nTextNormalizationCacheRefreshes += nTextNormalizationCacheRefreshes;
		m_nSampleTextNormalizationCacheHits += nTextNormalizationCacheHits;
		m_nSampleTextNormalizationCacheRefreshes += nTextNormalizationCacheRefreshes;
		m_nTextLayoutCacheHits += nTextLayoutCacheHits;
		m_nTextLayoutCacheRefreshes += nTextLayoutCacheRefreshes;
		m_nSampleTextLayoutCacheHits += nTextLayoutCacheHits;
		m_nSampleTextLayoutCacheRefreshes += nTextLayoutCacheRefreshes;
		m_nTextMetricsCacheHits += nTextMetricsCacheHits;
		m_nTextMetricsCacheRefreshes += nTextMetricsCacheRefreshes;
		m_nSampleTextMetricsCacheHits += nTextMetricsCacheHits;
		m_nSampleTextMetricsCacheRefreshes += nTextMetricsCacheRefreshes;
		m_nHtmlParseCacheHits += nHtmlParseCacheHits;
		m_nHtmlParseCacheRefreshes += nHtmlParseCacheRefreshes;
		m_nSampleHtmlParseCacheHits += nHtmlParseCacheHits;
		m_nSampleHtmlParseCacheRefreshes += nHtmlParseCacheRefreshes;
		m_nHtmlLayoutCacheHits += nHtmlLayoutCacheHits;
		m_nHtmlLayoutCacheRefreshes += nHtmlLayoutCacheRefreshes;
		m_nSampleHtmlLayoutCacheHits += nHtmlLayoutCacheHits;
		m_nSampleHtmlLayoutCacheRefreshes += nHtmlLayoutCacheRefreshes;
		m_nHtmlMetricsCacheHits += nHtmlMetricsCacheHits;
		m_nHtmlMetricsCacheRefreshes += nHtmlMetricsCacheRefreshes;
		m_nSampleHtmlMetricsCacheHits += nHtmlMetricsCacheHits;
		m_nSampleHtmlMetricsCacheRefreshes += nHtmlMetricsCacheRefreshes;
		m_nHtmlHitTestCacheHits += nHtmlHitTestCacheHits;
		m_nHtmlHitTestCacheRefreshes += nHtmlHitTestCacheRefreshes;
		m_nSampleHtmlHitTestCacheHits += nHtmlHitTestCacheHits;
		m_nSampleHtmlHitTestCacheRefreshes += nHtmlHitTestCacheRefreshes;
		m_nHtmlDrawingEffectCacheHits += nHtmlDrawingEffectCacheHits;
		m_nHtmlDrawingEffectCacheRefreshes += nHtmlDrawingEffectCacheRefreshes;
		m_nSampleHtmlDrawingEffectCacheHits += nHtmlDrawingEffectCacheHits;
		m_nSampleHtmlDrawingEffectCacheRefreshes += nHtmlDrawingEffectCacheRefreshes;
		m_nHtmlDirectWriteParseFailures += nHtmlDirectWriteParseFailures;
		m_nHtmlDirectWriteLayoutFailures += nHtmlDirectWriteLayoutFailures;
		m_nHtmlDirectWriteRenderFailures += nHtmlDirectWriteRenderFailures;
		m_nSampleHtmlDirectWriteParseFailures += nHtmlDirectWriteParseFailures;
		m_nSampleHtmlDirectWriteLayoutFailures += nHtmlDirectWriteLayoutFailures;
		m_nSampleHtmlDirectWriteRenderFailures += nHtmlDirectWriteRenderFailures;
	}

	void CPaintManagerUI::FinalizeRenderDiagnostics(const TPaintFrameState& paintFrame)
	{
		ConsumeRenderFrameMetrics();
		if (paintFrame.nPaintStartQpc > 0) {
			LARGE_INTEGER counter = {};
			::QueryPerformanceCounter(&counter);
			UpdateRenderDiagnostics(paintFrame.nPaintStartQpc, counter.QuadPart);
		}
	}

	void CPaintManagerUI::FinalizePaintState(const TPaintFrameState& paintFrame)
	{
		SetPainting(false);
		m_bLayeredChanged = false;
		if (m_bUpdateNeeded) {
			Invalidate();
		}

		if (paintFrame.bNeedSizeMsg) {
			this->SendNotify(m_pRoot, DUI_MSGTYPE_WINDOWSIZE, 0, 0, true);
		}
	}

	void CPaintManagerUI::FinalizePaintFrame(const TPaintFrameState& paintFrame)
	{
		FinalizeRenderDiagnostics(paintFrame);
		FinalizePaintState(paintFrame);
	}

	bool CPaintManagerUI::HandlePaintMessage()
	{
		if (m_pRoot == NULL) {
			return PaintEmptyRootFrame();
		}

		TPaintFrameState paintFrame;
		if (!PreparePaintFrameRects(paintFrame)) {
			return true;
		}

		CScopedPaintingState paintingState(*this, m_bRenderDiagnosticsEnabled);

		if (!PreparePaintFrameExecution(paintFrame)) {
			return true;
		}

		if (!PaintPreparedWindowFrame(paintFrame)) {
			return true;
		}

		FinalizePaintFrame(paintFrame);
		paintingState.Release();
		return true;
	}

	bool CPaintManagerUI::HandlePrintClientMessage(HDC hTargetDC, LPARAM lParam)
	{
		return RenderPrintClientFrame(hTargetDC, lParam);
	}

	CPaintManagerUI::CPaintManagerUI() :
		m_hWndPaint(NULL),
		m_bOffscreenPaint(true),
		m_hwndTooltip(NULL),
		m_uTimerID(0x1000),
		m_pRoot(NULL),
		m_pFocus(NULL),
		m_pEventHover(NULL),
		m_pEventClick(NULL),
		m_pEventRClick(NULL),
		m_pEventKey(NULL),
		m_bFirstLayout(true),
		m_bFocusNeeded(false),
		m_bUpdateNeeded(false),
		m_bMouseTracking(false),
		m_bMouseCapture(false),
		m_bAsyncNotifyPosted(false),
		m_bUsedVirtualWnd(false),
		m_bForceUseSharedRes(false),
		m_nOpacity(0xFF),
		m_bLayered(false),
		m_bLayeredChanged(false),
		m_bShowUpdateRect(false),
		m_renderBackend(RenderBackendAuto),
		m_direct2DRenderMode(Direct2DRenderModeAuto),
		m_bRenderDiagnosticsEnabled(false),
		m_bRenderDiagnosticsDebugTrace(false),
		m_nRenderPerfFrequency(0),
		m_nLastPaintEndQpc(0),
		m_nDiagnosticsSampleStartQpc(0),
		m_nLastPaintMs(0.0),
		m_nAveragePaintMs(0.0),
		m_nWorstPaintMs(0.0),
		m_nLastFrameIntervalMs(0.0),
		m_nAverageFrameIntervalMs(0.0),
		m_nCurrentFPS(0.0),
		m_nAverageFPS(0.0),
		m_nSamplePaintTotalMs(0.0),
		m_nRenderedFrameCount(0),
		m_nOverBudgetFrameCount(0),
		m_nSampleFrameCount(0),
		m_nNativeWindowCacheHits(0),
		m_nNativeWindowCacheRefreshes(0),
		m_nSampleNativeWindowCacheHits(0),
		m_nSampleNativeWindowCacheRefreshes(0),
		m_nImageScaleCacheHits(0),
		m_nImageScaleCacheRefreshes(0),
		m_nSampleImageScaleCacheHits(0),
		m_nSampleImageScaleCacheRefreshes(0),
		m_nDirect2DBatchFlushes(0),
		m_nDirect2DStandaloneDraws(0),
		m_nSampleDirect2DBatchFlushes(0),
		m_nSampleDirect2DStandaloneDraws(0),
		m_nLegacyImageFallbackDraws(0),
		m_nSampleLegacyImageFallbackDraws(0),
		m_nLegacyTextFallbackDraws(0),
		m_nSampleLegacyTextFallbackDraws(0),
		m_bUseGdiplusText(false),
		m_trh(0),
		m_bDragDrop(false),
		m_bDragMode(false),
		m_hDragBitmap(NULL),
		m_pDPI(NULL),
		m_uDPIGeneration(1),
		m_iHoverTime(100UL)
	{
		ResetRenderDiagnostics();

#ifdef SVG
		g_SDK = SDK::GetInstance();
		bool bIsLoadSucceed = g_SDK->LoadSdkDll(nullptr, ".\\svg.dll");
		if (bIsLoadSucceed == false)
			bIsLoadSucceed = g_SDK->LoadSdkDll(nullptr, ".\\bin\\svg.dll");

#endif // SVG

	

		if (m_SharedResInfo.m_DefaultFontInfo.sFontName.empty())
		{
			m_SharedResInfo.m_dwDefaultDisabledColor = 0xFFA7A6AA;
			m_SharedResInfo.m_dwDefaultFontColor = 0xFF000000;
			m_SharedResInfo.m_dwDefaultLinkFontColor = 0xFF0000FF;
			m_SharedResInfo.m_dwDefaultLinkHoverFontColor = 0xFFD3215F;
			m_SharedResInfo.m_dwDefaultSelectedBkColor = 0xFFBAE4FF;

			LOGFONT lf = { 0 };
			::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
			lf.lfCharSet = DEFAULT_CHARSET;
			HFONT hDefaultFont = ::CreateFontIndirect(&lf);
			m_SharedResInfo.m_DefaultFontInfo.hFont = hDefaultFont;
			m_SharedResInfo.m_DefaultFontInfo.sFontName = lf.lfFaceName;
			m_SharedResInfo.m_DefaultFontInfo.iSize = -lf.lfHeight;
			m_SharedResInfo.m_DefaultFontInfo.bBold = (lf.lfWeight >= FW_BOLD);
			m_SharedResInfo.m_DefaultFontInfo.bUnderline = (lf.lfUnderline == TRUE);
			m_SharedResInfo.m_DefaultFontInfo.bItalic = (lf.lfItalic == TRUE);
			::ZeroMemory(&m_SharedResInfo.m_DefaultFontInfo.tm, sizeof(m_SharedResInfo.m_DefaultFontInfo.tm));
		}

		m_ResInfo.m_dwDefaultDisabledColor = m_SharedResInfo.m_dwDefaultDisabledColor;
		m_ResInfo.m_dwDefaultFontColor = m_SharedResInfo.m_dwDefaultFontColor;
		m_ResInfo.m_dwDefaultLinkFontColor = m_SharedResInfo.m_dwDefaultLinkFontColor;
		m_ResInfo.m_dwDefaultLinkHoverFontColor = m_SharedResInfo.m_dwDefaultLinkHoverFontColor;
		m_ResInfo.m_dwDefaultSelectedBkColor = m_SharedResInfo.m_dwDefaultSelectedBkColor;

		if (m_hUpdateRectPen == NULL) {
			m_hUpdateRectPen = ::CreatePen(PS_SOLID, 1, RGB(220, 0, 0));
			// Boot Windows Common Controls (for the ToolTip control)
			::InitCommonControls();
			::LoadLibrary(_T("msimg32.dll"));
		}

		m_szMinWindow.cx = 0;
		m_szMinWindow.cy = 0;
		m_szMaxWindow.cx = 0;
		m_szMaxWindow.cy = 0;
		m_szInitWindowSize.cx = 0;
		m_szInitWindowSize.cy = 0;
		m_szRoundCorner.cx = m_szRoundCorner.cy = 0;
		::ZeroMemory(&m_rcSizeBox, sizeof(m_rcSizeBox));
		::ZeroMemory(&m_rcCaption, sizeof(m_rcCaption));
		::ZeroMemory(&m_rcLayeredInset, sizeof(m_rcLayeredInset));
		::ZeroMemory(&m_rcLayeredUpdate, sizeof(m_rcLayeredUpdate));
		m_ptLastMousePos.x = m_ptLastMousePos.y = -1;

		m_pGdiplusStartupInput = new Gdiplus::GdiplusStartupInput;
		Gdiplus::GdiplusStartup(&m_gdiplusToken, m_pGdiplusStartupInput, NULL); // 闁告梻濮惧ù鍢扗I闁规亽鍎辫ぐ?

		CShadowUI::Initialize(m_hInstance);

		m_pDragDrop = NULL;
	}

	CPaintManagerUI::~CPaintManagerUI()
	{
		// Delete the control-tree structures
		for (int i = 0; i < m_aDelayedCleanup.GetSize(); i++) delete static_cast<CControlUI*>(m_aDelayedCleanup[i]);
		m_aDelayedCleanup.Resize(0);
		for (int i = 0; i < m_aAsyncNotify.GetSize(); i++) delete static_cast<TNotifyUI*>(m_aAsyncNotify[i]);
		m_aAsyncNotify.Resize(0);

		m_mapName.clear();
		if (m_pRoot != NULL) delete m_pRoot;

		::DeleteObject(m_ResInfo.m_DefaultFontInfo.hFont);
		RemoveAllFonts();
		RemoveAllImages();
		RemoveAllStyle();
		RemoveAllDefaultAttributeList();
		RemoveAllWindowCustomAttribute();
		RemoveAllOptionGroups();
		RemoveAllTimers();
		RemoveAllDrawInfos();

		if (m_hwndTooltip != NULL) {
			::DestroyWindow(m_hwndTooltip);
			m_hwndTooltip = NULL;
		}
		if (!m_aFonts.empty()) {
			for (int i = 0; i < m_aFonts.GetSize(); ++i)
			{
				HANDLE handle = static_cast<HANDLE>(m_aFonts.GetAt(i));
				::RemoveFontMemResourceEx(handle);
			}
		}
		ClearNativeWindowCache();
		m_offscreenSurface.Reset();
		m_backgroundSurface.Reset();
		ResetMeasureDC();
		m_aPreMessages.Remove(m_aPreMessages.Find(this));
		// 闂佸簱鍋撴慨锝勭劍鐎氬骞忛挊澶嬬闁?
		if (m_hDragBitmap != NULL) CRenderEngine::FreeBitmap(m_hDragBitmap);
		//闁告鐡曞ù鍢扗IPlus
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
		delete m_pGdiplusStartupInput;
		// DPI缂佺媴绱曢幃濠勨偓鐢殿攰閽?
		if (m_pDPI != NULL) {
			delete m_pDPI;
			m_pDPI = NULL;
		}
	}

	void CPaintManagerUI::Init(HWND hWnd, std::wstring_view name)
	{
		m_mapName.clear();
		RemoveAllFonts();
		RemoveAllImages();
		RemoveAllStyle();
		RemoveAllDefaultAttributeList();
		RemoveAllWindowCustomAttribute();
		RemoveAllOptionGroups();
		RemoveAllTimers();
		ClearNativeWindowCache();
		m_aNativeWindow.Empty();
		m_aNativeWindowControl.Empty();

		m_sName.assign(name);

		if (m_hWndPaint != hWnd) {
			m_hWndPaint = hWnd;
			ResetMeasureDC();
			m_aPreMessages.Add(this);
		}
	}

	void CPaintManagerUI::DeletePtr(void* ptr)
	{
		if (ptr) { delete ptr; ptr = NULL; }
	}

	HINSTANCE CPaintManagerUI::GetInstance()
	{
		return m_hInstance;
	}

	std::wstring CPaintManagerUI::GetInstancePath()
	{
		if (m_hInstance == NULL) return std::wstring();

		TCHAR tszModule[MAX_PATH + 1] = { 0 };
		::GetModuleFileName(m_hInstance, tszModule, MAX_PATH);
		std::wstring sInstancePath = tszModule;
		const std::wstring::size_type pos = sInstancePath.rfind(_T('\\'));
		if (pos != std::wstring::npos) {
			sInstancePath = sInstancePath.substr(0, pos + 1);
		}
		return sInstancePath;
	}

	std::wstring CPaintManagerUI::GetCurrentPath()
	{
		TCHAR tszModule[MAX_PATH + 1] = { 0 };
		::GetCurrentDirectory(MAX_PATH, tszModule);
		return tszModule;
	}

	HINSTANCE CPaintManagerUI::GetResourceDll()
	{
		if (m_hResourceInstance == NULL) return m_hInstance;
		return m_hResourceInstance;
	}

	const std::wstring& CPaintManagerUI::GetResourcePath()
	{
		return m_pStrResourcePath;
	}

	const std::wstring& CPaintManagerUI::GetResourceZip()
	{
		return m_pStrResourceZip;
	}

	const std::wstring& CPaintManagerUI::GetResourceZipPwd()
	{
		return m_pStrResourceZipPwd;
	}

	bool CPaintManagerUI::IsCachedResourceZip()
	{
		return m_bCachedResourceZip;
	}

	HANDLE CPaintManagerUI::GetResourceZipHandle()
	{
		return m_hResourceZip;
	}

	void CPaintManagerUI::SetInstance(HINSTANCE hInst)
	{
		m_hInstance = hInst;
	}

	void CPaintManagerUI::SetCurrentPath(std::wstring_view path)
	{
		if (path.empty()) {
			return;
		}
		const std::wstring currentPath(path);
		::SetCurrentDirectory(currentPath.c_str());
	}

	void CPaintManagerUI::SetResourceDll(HINSTANCE hInst)
	{
		if (m_hResourceInstance == hInst) {
			return;
		}
		m_hResourceInstance = hInst;
		g_bPendingResourceEnvironmentReload = true;
		InvalidateConfiguredResourceMetadata();
		m_SharedResInfo.m_AttrHash.clear();
		m_SharedResInfo.m_StyleHash.clear();
	}

	void CPaintManagerUI::SetResourcePath(std::wstring_view path)
	{
		std::wstring normalizedPath(path);
		if (!normalizedPath.empty()) {
			TCHAR cEnd = normalizedPath[normalizedPath.size() - 1];
			if (cEnd != _T('\\') && cEnd != _T('/')) normalizedPath += _T('\\');
		}
		if (m_pStrResourcePath == normalizedPath) {
			return;
		}
		m_pStrResourcePath.swap(normalizedPath);
		g_bPendingResourceEnvironmentReload = true;
		InvalidateConfiguredResourceMetadata();
		m_SharedResInfo.m_AttrHash.clear();
		m_SharedResInfo.m_StyleHash.clear();
	}

	void CPaintManagerUI::SetResourceZip(LPVOID pVoid, unsigned int len, std::wstring_view password)
	{
		if (m_bCachedResourceZip && m_hResourceZip != NULL) {
			CloseZip((HZIP)m_hResourceZip);
			m_hResourceZip = NULL;
		}
		m_pStrResourceZip = _T("membuffer");
		if (m_cbZipBuf)
		{
			delete[] m_cbZipBuf;
			m_cbZipBuf = nullptr;
		}
		if (!m_cbZipBuf)
		{
			m_cbZipBuf = new BYTE[len];
			memcpy(m_cbZipBuf, pVoid, len);
		}

		m_bCachedResourceZip = true;
		m_pStrResourceZipPwd.assign(password);  //Garfield 20160325 閻㈩垽绠戦惁鎴︽儘娴滃儷p闁告牕鎳撹閻?
		if (m_bCachedResourceZip)
		{
#ifdef UNICODE
			char* pwd = password.empty() ? nullptr : w2a(const_cast<wchar_t*>(m_pStrResourceZipPwd.c_str()));
			m_hResourceZip = (HANDLE)OpenZip(m_cbZipBuf, len, pwd);
			if (pwd) {
				delete[] pwd;
				pwd = NULL;
			}
#else
			m_hResourceZip = (HANDLE)OpenZip(m_cbZipBuf, len, password);
#endif
		}
		g_bPendingResourceEnvironmentReload = true;
		InvalidateConfiguredResourceMetadata();
		m_SharedResInfo.m_AttrHash.clear();
		m_SharedResInfo.m_StyleHash.clear();
	}

	void CPaintManagerUI::SetResourceZip(std::wstring_view zipPath, bool bCachedResourceZip, std::wstring_view password)
	{
		if (m_pStrResourceZip == zipPath && m_bCachedResourceZip == bCachedResourceZip && m_pStrResourceZipPwd == password) return;
		if (m_bCachedResourceZip && m_hResourceZip != NULL) {
			CloseZip((HZIP)m_hResourceZip);
			m_hResourceZip = NULL;
		}
		if (m_cbZipBuf != nullptr) {
			delete[] m_cbZipBuf;
			m_cbZipBuf = nullptr;
		}
		m_pStrResourceZip.assign(zipPath);
		m_bCachedResourceZip = bCachedResourceZip;
		m_pStrResourceZipPwd.assign(password);
		if (m_bCachedResourceZip) 
		{
			std::wstring sFile = CPaintManagerUI::GetResourcePath();
			sFile += CPaintManagerUI::GetResourceZip();
#ifdef UNICODE
			char* pwd = password.empty() ? nullptr : w2a(const_cast<wchar_t*>(m_pStrResourceZipPwd.c_str()));
			m_hResourceZip = (HANDLE)OpenZip(sFile.c_str(), pwd);
			if (pwd) {
				delete[] pwd;
				pwd = NULL;
			}
#else
			m_hResourceZip = (HANDLE)OpenZip(sFile.c_str(), password);
#endif
		}
		g_bPendingResourceEnvironmentReload = true;
		InvalidateConfiguredResourceMetadata();
		m_SharedResInfo.m_AttrHash.clear();
		m_SharedResInfo.m_StyleHash.clear();
	}

	void CPaintManagerUI::SetResourceType(int nType)
	{
		m_nResType = nType;
	}

	int CPaintManagerUI::GetResourceType()
	{
		return m_nResType;
	}

	bool CPaintManagerUI::GetHSL(short* H, short* S, short* L)
	{
		*H = m_H;
		*S = m_S;
		*L = m_L;
		return m_bUseHSL;
	}

	void CPaintManagerUI::SetHSL(bool bUseHSL, short H, short S, short L)
	{
		if (m_bUseHSL || m_bUseHSL != bUseHSL) {
			m_bUseHSL = bUseHSL;
			if (H == m_H && S == m_S && L == m_L) return;
			m_H = CLAMP(H, 0, 360);
			m_S = CLAMP(S, 0, 200);
			m_L = CLAMP(L, 0, 200);
			AdjustSharedImagesHSL();
			for (int i = 0; i < m_aPreMessages.GetSize(); i++) {
				CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
				if (pManager != NULL) pManager->AdjustImagesHSL();
			}
		}
	}

	void CPaintManagerUI::ReloadSkin()
	{
		const bool bResourceEnvironmentChanged = g_bPendingResourceEnvironmentReload;
		g_bPendingResourceEnvironmentReload = false;

		ReloadSharedImages();
		for (int i = 0; i < m_aPreMessages.GetSize(); i++) {
			CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
			if (pManager == NULL) {
				continue;
			}
			pManager->ReloadImages();
			if (bResourceEnvironmentChanged) {
				pManager->NeedUpdate();
				pManager->Invalidate();
			}
		}
	}

	CPaintManagerUI* CPaintManagerUI::GetPaintManager(std::wstring_view name)
	{
		if (name.empty()) return NULL;
		for (int i = 0; i < m_aPreMessages.GetSize(); i++) {
			CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
			if (pManager != NULL && name == pManager->GetNameView()) return pManager;
		}
		return NULL;
	}

	CStdPtrArray* CPaintManagerUI::GetPaintManagers()
	{
		return &m_aPreMessages;
	}

	bool CPaintManagerUI::LoadPlugin(std::wstring_view moduleName)
	{
		if (moduleName.empty()) return false;
		const std::wstring moduleNameStr(moduleName);
		HMODULE hModule = ::LoadLibrary(moduleNameStr.c_str());
		if (hModule != NULL) {
			LPCREATECONTROL lpCreateControl = (LPCREATECONTROL)::GetProcAddress(hModule, "CreateControl");
			if (lpCreateControl != NULL) {
				if (m_aPlugins.Find(lpCreateControl) >= 0) return true;
				m_aPlugins.Add(lpCreateControl);
				return true;
			}
		}
		return false;
	}

	CStdPtrArray* CPaintManagerUI::GetPlugins()
	{
		return &m_aPlugins;
	}

	HWND CPaintManagerUI::GetPaintWindow() const
	{
		return m_hWndPaint;
	}

	HWND CPaintManagerUI::GetTooltipWindow() const
	{
		return m_hwndTooltip;
	}

	int CPaintManagerUI::GetHoverTime() const
	{
		return m_iHoverTime;
	}

	void CPaintManagerUI::SetHoverTime(int iTime)
	{
		m_iHoverTime = iTime;
	}

	const std::wstring& CPaintManagerUI::GetName() const
	{
		return m_sName;
	}

	HDC CPaintManagerUI::GetPaintDC() const
	{
		if (m_pCurrentRenderContext != nullptr && m_pCurrentRenderContext->GetDC() != NULL) {
			return m_pCurrentRenderContext->GetDC();
		}
		return GetMeasureDC();
	}

	HDC CPaintManagerUI::GetMeasureDC() const
	{
		return const_cast<CPaintManagerUI*>(this)->EnsureMeasureDC();
	}

	POINT CPaintManagerUI::GetMousePos() const
	{
		return m_ptLastMousePos;
	}

	SIZE CPaintManagerUI::GetClientSize() const
	{
		RECT rcClient = { 0 };
		::GetClientRect(m_hWndPaint, &rcClient);
		return CDuiSize(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);
	}

	SIZE CPaintManagerUI::GetInitSize()
	{
		return m_szInitWindowSize;
	}

	void CPaintManagerUI::SetInitSize(int cx, int cy)
	{
		m_szInitWindowSize.cx = cx;
		m_szInitWindowSize.cy = cy;
		if (m_pRoot == NULL && m_hWndPaint != NULL) {
			::SetWindowPos(m_hWndPaint, NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		}
	}

	int CPaintManagerUI::ScaleValue(int value) const
	{
		return EnsureDPIState().LogicalToDevice(value);
	}

	RECT CPaintManagerUI::ScaleRect(RECT rc) const
	{
		return EnsureDPIState().LogicalToDevice(rc);
	}

	void CPaintManagerUI::ScaleRect(RECT* rc) const
	{
		EnsureDPIState().LogicalToDevice(rc);
	}

	SIZE CPaintManagerUI::ScaleSize(SIZE sz) const
	{
		return EnsureDPIState().LogicalToDevice(sz);
	}

	void CPaintManagerUI::ScaleSize(SIZE* sz) const
	{
		EnsureDPIState().LogicalToDevice(sz);
	}

	POINT CPaintManagerUI::ScalePoint(POINT pt) const
	{
		return EnsureDPIState().LogicalToDevice(pt);
	}

	void CPaintManagerUI::ScalePoint(POINT* pt) const
	{
		EnsureDPIState().LogicalToDevice(pt);
	}

	int CPaintManagerUI::UnscaleValue(int value) const
	{
		return EnsureDPIState().DeviceToLogical(value);
	}

	RECT CPaintManagerUI::UnscaleRect(RECT rc) const
	{
		return EnsureDPIState().DeviceToLogical(rc);
	}

	void CPaintManagerUI::UnscaleRect(RECT* rc) const
	{
		EnsureDPIState().DeviceToLogical(rc);
	}

	SIZE CPaintManagerUI::UnscaleSize(SIZE sz) const
	{
		return EnsureDPIState().DeviceToLogical(sz);
	}

	void CPaintManagerUI::UnscaleSize(SIZE* sz) const
	{
		EnsureDPIState().DeviceToLogical(sz);
	}

	POINT CPaintManagerUI::UnscalePoint(POINT pt) const
	{
		return EnsureDPIState().DeviceToLogical(pt);
	}

	void CPaintManagerUI::UnscalePoint(POINT* pt) const
	{
		EnsureDPIState().DeviceToLogical(pt);
	}

	RECT CPaintManagerUI::GetSizeBox()
	{
		return ScaleRect(m_rcSizeBox);
	}

	void CPaintManagerUI::SetSizeBox(RECT& rcSizeBox)
	{
		m_rcSizeBox = rcSizeBox;
	}

	RECT CPaintManagerUI::GetCaptionRect()
	{
		return ScaleRect(m_rcCaption);
	}

	void CPaintManagerUI::SetCaptionRect(RECT& rcCaption)
	{
		m_rcCaption = rcCaption;
	}

	SIZE CPaintManagerUI::GetRoundCorner()
	{
		return ScaleSize(m_szRoundCorner);
	}

	void CPaintManagerUI::SetRoundCorner(int cx, int cy)
	{
		m_szRoundCorner.cx = cx;
		m_szRoundCorner.cy = cy;
	}

	SIZE CPaintManagerUI::GetMinInfo()
	{
		return ScaleSize(m_szMinWindow);
	}

	void CPaintManagerUI::SetMinInfo(int cx, int cy)
	{

		m_szMinWindow.cx = cx;
		m_szMinWindow.cy = cy;
	}

	SIZE CPaintManagerUI::GetMaxInfo()
	{
		return ScaleSize(m_szMaxWindow);
	}

	void CPaintManagerUI::SetMaxInfo(int cx, int cy)
	{
		m_szMaxWindow.cx = cx;
		m_szMaxWindow.cy = cy;
	}

	bool CPaintManagerUI::IsShowUpdateRect()
	{
		return m_bShowUpdateRect;
	}

	void CPaintManagerUI::SetShowUpdateRect(bool show)
	{
		m_bShowUpdateRect = show;
	}

	bool CPaintManagerUI::IsNoActivate()
	{
		return m_bNoActivate;
	}

	void CPaintManagerUI::SetNoActivate(bool bNoActivate)
	{
		m_bNoActivate = bNoActivate;
	}

	BYTE CPaintManagerUI::GetOpacity() const
	{
		return m_nOpacity;
	}

	void CPaintManagerUI::SetOpacity(BYTE nOpacity)
	{
		m_nOpacity = nOpacity;
		if (m_hWndPaint != NULL) {
			typedef BOOL(__stdcall* PFUNCSETLAYEREDWINDOWATTR)(HWND, COLORREF, BYTE, DWORD);

			DWORD dwStyle = ::GetWindowLong(m_hWndPaint, GWL_EXSTYLE);
			DWORD dwNewStyle = dwStyle;
			if (nOpacity >= 0 && nOpacity < 256) dwNewStyle |= WS_EX_LAYERED;
			else dwNewStyle &= ~WS_EX_LAYERED;
			if (dwStyle != dwNewStyle) ::SetWindowLong(m_hWndPaint, GWL_EXSTYLE, dwNewStyle);
			SetLayeredWindowAttributes(m_hWndPaint, 0, nOpacity, LWA_ALPHA);
		}
	}

	bool CPaintManagerUI::IsLayered()
	{
		return m_bLayered;
	}

	void CPaintManagerUI::SetLayered(bool bLayered)
	{
		if (m_hWndPaint != NULL && bLayered != m_bLayered) {
			UINT uStyle = GetWindowStyle(m_hWndPaint);
			if ((uStyle & WS_CHILD) != 0) return;

			m_bLayered = bLayered;
			if (m_pRoot != NULL) m_pRoot->NeedUpdate();
			Invalidate();
		}
	}

	RECT& CPaintManagerUI::GetLayeredInset()
	{
		return m_rcLayeredInset;
	}

	void CPaintManagerUI::SetLayeredInset(RECT& rcLayeredInset)
	{
		m_rcLayeredInset = rcLayeredInset;
		m_bLayeredChanged = true;
		Invalidate();
	}

	BYTE CPaintManagerUI::GetLayeredOpacity()
	{
		return m_nOpacity;
	}

	void CPaintManagerUI::SetLayeredOpacity(BYTE nOpacity)
	{
		m_nOpacity = nOpacity;
		m_bLayeredChanged = true;
		Invalidate();
	}

	void CPaintManagerUI::SetLayeredImage(std::wstring_view image)
	{
		m_diLayered.sDrawString.assign(image);
		RECT rcNull = { 0 };
		LegacyDrawImageInfo(NULL, this, rcNull, rcNull, &m_diLayered);
		m_bLayeredChanged = true;
		Invalidate();
	}

	CShadowUI* CPaintManagerUI::GetShadow()
	{
		return &m_shadow;
	}

	void CPaintManagerUI::SetRenderBackend(RenderBackendType backend)
	{
		if (m_renderBackend == backend) return;
		m_renderBackend = backend;
		if (IsValid()) Invalidate();
	}

	RenderBackendType CPaintManagerUI::GetRenderBackend() const
	{
		return m_renderBackend;
	}

	RenderBackendType CPaintManagerUI::GetActiveRenderBackend()
	{
		ApplyRenderPreferences();
		return CRenderEngine::GetActiveRenderBackend();
	}

	void CPaintManagerUI::SetDirect2DRenderMode(Direct2DRenderMode mode)
	{
		if (m_direct2DRenderMode == mode) return;
		m_direct2DRenderMode = mode;
		if (IsValid()) Invalidate();
	}

	Direct2DRenderMode CPaintManagerUI::GetDirect2DRenderMode() const
	{
		return m_direct2DRenderMode;
	}

	Direct2DRenderMode CPaintManagerUI::GetActiveDirect2DRenderMode()
	{
		ApplyRenderPreferences();
		return CRenderEngine::GetActiveDirect2DRenderMode();
	}

	void CPaintManagerUI::SetRenderDiagnosticsEnabled(bool bEnabled, bool bDebugTrace)
	{
		m_bRenderDiagnosticsEnabled = bEnabled;
		m_bRenderDiagnosticsDebugTrace = bEnabled && bDebugTrace;
		ResetRenderDiagnostics();
	}

	bool CPaintManagerUI::IsRenderDiagnosticsEnabled() const
	{
		return m_bRenderDiagnosticsEnabled;
	}

	bool CPaintManagerUI::IsRenderDiagnosticsDebugTraceEnabled() const
	{
		return m_bRenderDiagnosticsDebugTrace;
	}

	const CPaintRenderContext* CPaintManagerUI::GetCurrentRenderContext() const
	{
		return m_pCurrentRenderContext;
	}

	void CPaintManagerUI::NotifyImageScaleCacheHit()
	{
		++m_nImageScaleCacheHits;
		++m_nSampleImageScaleCacheHits;
	}

	void CPaintManagerUI::NotifyImageScaleCacheRefresh()
	{
		++m_nImageScaleCacheRefreshes;
		++m_nSampleImageScaleCacheRefreshes;
	}

	void CPaintManagerUI::GetRenderDiagnostics(TRenderDiagnostics& diagnostics)
	{
		diagnostics.bEnabled = m_bRenderDiagnosticsEnabled;
		diagnostics.bDebugTrace = m_bRenderDiagnosticsDebugTrace;
		diagnostics.nLastPaintMs = m_nLastPaintMs;
		diagnostics.nAveragePaintMs = m_nAveragePaintMs;
		diagnostics.nWorstPaintMs = m_nWorstPaintMs;
		diagnostics.nLastFrameIntervalMs = m_nLastFrameIntervalMs;
		diagnostics.nAverageFrameIntervalMs = m_nAverageFrameIntervalMs;
		diagnostics.nCurrentFPS = m_nCurrentFPS;
		diagnostics.nAverageFPS = m_nAverageFPS;
		diagnostics.nTotalFrames = m_nRenderedFrameCount;
		diagnostics.nOverBudgetFrames = m_nOverBudgetFrameCount;
		diagnostics.nNativeWindowCacheHits = m_nNativeWindowCacheHits;
		diagnostics.nNativeWindowCacheRefreshes = m_nNativeWindowCacheRefreshes;
		diagnostics.nImageScaleCacheHits = m_nImageScaleCacheHits;
		diagnostics.nImageScaleCacheRefreshes = m_nImageScaleCacheRefreshes;
		diagnostics.nDirect2DBatchFlushes = m_nDirect2DBatchFlushes;
		diagnostics.nDirect2DStandaloneDraws = m_nDirect2DStandaloneDraws;
		diagnostics.nLegacyImageFallbackDraws = m_nLegacyImageFallbackDraws;
		diagnostics.nLegacyTextFallbackDraws = m_nLegacyTextFallbackDraws;
		diagnostics.nTextLayoutCacheHits = m_nTextLayoutCacheHits;
		diagnostics.nTextLayoutCacheRefreshes = m_nTextLayoutCacheRefreshes;
		diagnostics.nTextMetricsCacheHits = m_nTextMetricsCacheHits;
		diagnostics.nTextMetricsCacheRefreshes = m_nTextMetricsCacheRefreshes;
		diagnostics.nTextNormalizationCacheHits = m_nTextNormalizationCacheHits;
		diagnostics.nTextNormalizationCacheRefreshes = m_nTextNormalizationCacheRefreshes;
		diagnostics.nHtmlParseCacheHits = m_nHtmlParseCacheHits;
		diagnostics.nHtmlParseCacheRefreshes = m_nHtmlParseCacheRefreshes;
		diagnostics.nHtmlLayoutCacheHits = m_nHtmlLayoutCacheHits;
		diagnostics.nHtmlLayoutCacheRefreshes = m_nHtmlLayoutCacheRefreshes;
		diagnostics.nHtmlMetricsCacheHits = m_nHtmlMetricsCacheHits;
		diagnostics.nHtmlMetricsCacheRefreshes = m_nHtmlMetricsCacheRefreshes;
		diagnostics.nHtmlHitTestCacheHits = m_nHtmlHitTestCacheHits;
		diagnostics.nHtmlHitTestCacheRefreshes = m_nHtmlHitTestCacheRefreshes;
		diagnostics.nHtmlDrawingEffectCacheHits = m_nHtmlDrawingEffectCacheHits;
		diagnostics.nHtmlDrawingEffectCacheRefreshes = m_nHtmlDrawingEffectCacheRefreshes;
		diagnostics.nHtmlDirectWriteParseFailures = m_nHtmlDirectWriteParseFailures;
		diagnostics.nHtmlDirectWriteLayoutFailures = m_nHtmlDirectWriteLayoutFailures;
		diagnostics.nHtmlDirectWriteRenderFailures = m_nHtmlDirectWriteRenderFailures;
		diagnostics.activeBackend = GetActiveRenderBackend();
		diagnostics.activeDirect2DMode = GetActiveDirect2DRenderMode();
	}

	void CPaintManagerUI::ApplyRenderPreferences()
	{
		CRenderEngine::SetPreferredRenderBackend(m_renderBackend);
		CRenderEngine::SetPreferredDirect2DRenderMode(m_direct2DRenderMode);
	}

	void CPaintManagerUI::ResetRenderDiagnostics()
	{
		if (m_nRenderPerfFrequency <= 0) {
			LARGE_INTEGER frequency = {};
			if (::QueryPerformanceFrequency(&frequency)) {
				m_nRenderPerfFrequency = frequency.QuadPart;
			}
		}

		m_nLastPaintEndQpc = 0;
		m_nDiagnosticsSampleStartQpc = 0;
		m_nLastPaintMs = 0.0;
		m_nAveragePaintMs = 0.0;
		m_nWorstPaintMs = 0.0;
		m_nLastFrameIntervalMs = 0.0;
		m_nAverageFrameIntervalMs = 0.0;
		m_nCurrentFPS = 0.0;
		m_nAverageFPS = 0.0;
		m_nSamplePaintTotalMs = 0.0;
		m_nRenderedFrameCount = 0;
		m_nOverBudgetFrameCount = 0;
		m_nSampleFrameCount = 0;
		m_nNativeWindowCacheHits = 0;
		m_nNativeWindowCacheRefreshes = 0;
		m_nSampleNativeWindowCacheHits = 0;
		m_nSampleNativeWindowCacheRefreshes = 0;
		m_nImageScaleCacheHits = 0;
		m_nImageScaleCacheRefreshes = 0;
		m_nSampleImageScaleCacheHits = 0;
		m_nSampleImageScaleCacheRefreshes = 0;
		m_nDirect2DBatchFlushes = 0;
		m_nDirect2DStandaloneDraws = 0;
		m_nSampleDirect2DBatchFlushes = 0;
		m_nSampleDirect2DStandaloneDraws = 0;
		m_nLegacyImageFallbackDraws = 0;
		m_nSampleLegacyImageFallbackDraws = 0;
		m_nLegacyTextFallbackDraws = 0;
		m_nSampleLegacyTextFallbackDraws = 0;
		m_nTextLayoutCacheHits = 0;
		m_nTextLayoutCacheRefreshes = 0;
		m_nSampleTextLayoutCacheHits = 0;
		m_nSampleTextLayoutCacheRefreshes = 0;
		m_nTextMetricsCacheHits = 0;
		m_nTextMetricsCacheRefreshes = 0;
		m_nSampleTextMetricsCacheHits = 0;
		m_nSampleTextMetricsCacheRefreshes = 0;
		m_nTextNormalizationCacheHits = 0;
		m_nTextNormalizationCacheRefreshes = 0;
		m_nSampleTextNormalizationCacheHits = 0;
		m_nSampleTextNormalizationCacheRefreshes = 0;
		m_nHtmlParseCacheHits = 0;
		m_nHtmlParseCacheRefreshes = 0;
		m_nSampleHtmlParseCacheHits = 0;
		m_nSampleHtmlParseCacheRefreshes = 0;
		m_nHtmlLayoutCacheHits = 0;
		m_nHtmlLayoutCacheRefreshes = 0;
		m_nSampleHtmlLayoutCacheHits = 0;
		m_nSampleHtmlLayoutCacheRefreshes = 0;
		m_nHtmlMetricsCacheHits = 0;
		m_nHtmlMetricsCacheRefreshes = 0;
		m_nSampleHtmlMetricsCacheHits = 0;
		m_nSampleHtmlMetricsCacheRefreshes = 0;
		m_nHtmlHitTestCacheHits = 0;
		m_nHtmlHitTestCacheRefreshes = 0;
		m_nSampleHtmlHitTestCacheHits = 0;
		m_nSampleHtmlHitTestCacheRefreshes = 0;
		m_nHtmlDrawingEffectCacheHits = 0;
		m_nHtmlDrawingEffectCacheRefreshes = 0;
		m_nSampleHtmlDrawingEffectCacheHits = 0;
		m_nSampleHtmlDrawingEffectCacheRefreshes = 0;
		m_nHtmlDirectWriteParseFailures = 0;
		m_nHtmlDirectWriteLayoutFailures = 0;
		m_nHtmlDirectWriteRenderFailures = 0;
		m_nSampleHtmlDirectWriteParseFailures = 0;
		m_nSampleHtmlDirectWriteLayoutFailures = 0;
		m_nSampleHtmlDirectWriteRenderFailures = 0;
	}

	void CPaintManagerUI::UpdateRenderDiagnostics(LONGLONG nPaintStartQpc, LONGLONG nPaintEndQpc)
	{
		if (!m_bRenderDiagnosticsEnabled || m_nRenderPerfFrequency <= 0 || nPaintEndQpc <= nPaintStartQpc) {
			return;
		}

		const double nTargetFrameMs = 1000.0 / 120.0;
		const double nPaintMs = QpcDeltaToMilliseconds(nPaintStartQpc, nPaintEndQpc, m_nRenderPerfFrequency);
		m_nLastPaintMs = nPaintMs;
		m_nAveragePaintMs = (m_nRenderedFrameCount == 0) ? nPaintMs : (m_nAveragePaintMs * 0.9 + nPaintMs * 0.1);
		m_nWorstPaintMs = (std::max)(m_nWorstPaintMs, nPaintMs);

		if (m_nLastPaintEndQpc > 0) {
			m_nLastFrameIntervalMs = QpcDeltaToMilliseconds(m_nLastPaintEndQpc, nPaintEndQpc, m_nRenderPerfFrequency);
			if (m_nLastFrameIntervalMs > 0.0) {
				m_nCurrentFPS = 1000.0 / m_nLastFrameIntervalMs;
				m_nAverageFrameIntervalMs = (m_nRenderedFrameCount == 0)
					? m_nLastFrameIntervalMs
					: (m_nAverageFrameIntervalMs * 0.9 + m_nLastFrameIntervalMs * 0.1);
				if (m_nAverageFrameIntervalMs > 0.0) {
					m_nAverageFPS = 1000.0 / m_nAverageFrameIntervalMs;
				}
			}
		}

		++m_nRenderedFrameCount;
		m_nSamplePaintTotalMs += nPaintMs;
		++m_nSampleFrameCount;
		if (nPaintMs > nTargetFrameMs) {
			++m_nOverBudgetFrameCount;
		}

		if (m_nDiagnosticsSampleStartQpc == 0) {
			m_nDiagnosticsSampleStartQpc = nPaintEndQpc;
		}
		else if (m_bRenderDiagnosticsDebugTrace) {
			const double nSampleMs = QpcDeltaToMilliseconds(m_nDiagnosticsSampleStartQpc, nPaintEndQpc, m_nRenderPerfFrequency);
			if (nSampleMs >= 1000.0 && m_nSampleFrameCount > 0) {
				const double nSampleFPS = static_cast<double>(m_nSampleFrameCount) * 1000.0 / nSampleMs;
				const double nSamplePaintAvg = m_nSamplePaintTotalMs / static_cast<double>(m_nSampleFrameCount);
				TCHAR szTrace[768] = { 0 };
				_stprintf_s(
					szTrace,
					_T("[FYUI] fps=%.1f paint(avg=%.2fms last=%.2fms worst=%.2fms) over8.33ms=%u native(hit=%u refresh=%u) image(hit=%u refresh=%u legacy=%u) text(prepHit=%u prepRefresh=%u layoutHit=%u layoutRefresh=%u metricsHit=%u metricsRefresh=%u legacy=%u) html(parseHit=%u parseRefresh=%u layoutHit=%u layoutRefresh=%u metricsHit=%u metricsRefresh=%u hitTestHit=%u hitTestRefresh=%u effectHit=%u effectRefresh=%u parseFail=%u layoutFail=%u renderFail=%u) d2d(flush=%u standalone=%u) backend=%d d2d=%d\r\n"),
					nSampleFPS,
					nSamplePaintAvg,
					m_nLastPaintMs,
					m_nWorstPaintMs,
					m_nOverBudgetFrameCount,
					m_nSampleNativeWindowCacheHits,
					m_nSampleNativeWindowCacheRefreshes,
					m_nSampleImageScaleCacheHits,
					m_nSampleImageScaleCacheRefreshes,
					m_nSampleLegacyImageFallbackDraws,
					m_nSampleTextNormalizationCacheHits,
					m_nSampleTextNormalizationCacheRefreshes,
					m_nSampleTextLayoutCacheHits,
					m_nSampleTextLayoutCacheRefreshes,
					m_nSampleTextMetricsCacheHits,
					m_nSampleTextMetricsCacheRefreshes,
					m_nSampleLegacyTextFallbackDraws,
					m_nSampleHtmlParseCacheHits,
					m_nSampleHtmlParseCacheRefreshes,
					m_nSampleHtmlLayoutCacheHits,
					m_nSampleHtmlLayoutCacheRefreshes,
					m_nSampleHtmlMetricsCacheHits,
					m_nSampleHtmlMetricsCacheRefreshes,
					m_nSampleHtmlHitTestCacheHits,
					m_nSampleHtmlHitTestCacheRefreshes,
					m_nSampleHtmlDrawingEffectCacheHits,
					m_nSampleHtmlDrawingEffectCacheRefreshes,
					m_nSampleHtmlDirectWriteParseFailures,
					m_nSampleHtmlDirectWriteLayoutFailures,
					m_nSampleHtmlDirectWriteRenderFailures,
					m_nSampleDirect2DBatchFlushes,
					m_nSampleDirect2DStandaloneDraws,
					static_cast<int>(GetActiveRenderBackend()),
					static_cast<int>(GetActiveDirect2DRenderMode()));
				::OutputDebugString(szTrace);
				m_nDiagnosticsSampleStartQpc = nPaintEndQpc;
				m_nSamplePaintTotalMs = 0.0;
				m_nSampleFrameCount = 0;
				m_nSampleNativeWindowCacheHits = 0;
				m_nSampleNativeWindowCacheRefreshes = 0;
				m_nSampleImageScaleCacheHits = 0;
				m_nSampleImageScaleCacheRefreshes = 0;
				m_nSampleDirect2DBatchFlushes = 0;
				m_nSampleDirect2DStandaloneDraws = 0;
				m_nSampleLegacyImageFallbackDraws = 0;
				m_nSampleLegacyTextFallbackDraws = 0;
				m_nSampleTextNormalizationCacheHits = 0;
				m_nSampleTextNormalizationCacheRefreshes = 0;
				m_nSampleTextLayoutCacheHits = 0;
				m_nSampleTextLayoutCacheRefreshes = 0;
				m_nSampleTextMetricsCacheHits = 0;
				m_nSampleTextMetricsCacheRefreshes = 0;
				m_nSampleHtmlParseCacheHits = 0;
				m_nSampleHtmlParseCacheRefreshes = 0;
				m_nSampleHtmlLayoutCacheHits = 0;
				m_nSampleHtmlLayoutCacheRefreshes = 0;
				m_nSampleHtmlMetricsCacheHits = 0;
				m_nSampleHtmlMetricsCacheRefreshes = 0;
				m_nSampleHtmlHitTestCacheHits = 0;
				m_nSampleHtmlHitTestCacheRefreshes = 0;
				m_nSampleHtmlDrawingEffectCacheHits = 0;
				m_nSampleHtmlDrawingEffectCacheRefreshes = 0;
				m_nSampleHtmlDirectWriteParseFailures = 0;
				m_nSampleHtmlDirectWriteLayoutFailures = 0;
				m_nSampleHtmlDirectWriteRenderFailures = 0;
			}
		}

		m_nLastPaintEndQpc = nPaintEndQpc;
	}

	void CPaintManagerUI::ClearNativeWindowCache()
	{
		for (int i = 0; i < m_aNativeWindowCache.GetSize(); ++i) {
			ClearNativeWindowCache(i);
		}
		m_aNativeWindowCache.Empty();
	}

	void CPaintManagerUI::ClearNativeWindowCache(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_aNativeWindowCache.GetSize()) {
			return;
		}

		TNativeWindowBitmapCache* pCache = static_cast<TNativeWindowBitmapCache*>(m_aNativeWindowCache.GetAt(iIndex));
		DestroyNativeWindowBitmapCache(pCache);
		m_aNativeWindowCache.SetAt(iIndex, NULL);
	}

	bool CPaintManagerUI::RemoveNativeWindowEntryAt(int iIndex)
	{
		if (iIndex < 0 || iIndex >= m_aNativeWindow.GetSize()) {
			return false;
		}

		ClearNativeWindowCache(iIndex);
		if (!m_aNativeWindow.Remove(iIndex)) {
			return false;
		}
		m_aNativeWindowControl.Remove(iIndex);
		if (iIndex < m_aNativeWindowCache.GetSize()) {
			m_aNativeWindowCache.Remove(iIndex);
		}
		return true;
	}

	void CPaintManagerUI::SetUseGdiplusText(bool bUse)
	{
		m_bUseGdiplusText = bUse;
	}

	bool CPaintManagerUI::IsUseGdiplusText() const
	{
		return m_bUseGdiplusText;
	}

	void CPaintManagerUI::SetGdiplusTextRenderingHint(int trh)
	{
		m_trh = trh;
	}

	int CPaintManagerUI::GetGdiplusTextRenderingHint() const
	{
		return m_trh;
	}

	bool CPaintManagerUI::PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes)
	{
		for (int i = 0; i < m_aPreMessageFilters.GetSize(); i++)
		{
			bool bHandled = false;
			LRESULT lResult = static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i])->MessageHandler(uMsg, wParam, lParam, bHandled);
			if (bHandled) {
				lRes = lResult;
				return true;
			}
		}
		switch (uMsg) {
		case WM_KEYDOWN:
		{
			// Tabbing between controls
			if (wParam == VK_TAB)
			{
				if (m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && StringUtil::Find(m_pFocus->GetClass(), _T("RichEditUI")) != -1) {
					if (static_cast<CRichEditUI*>(m_pFocus)->IsWantTab()) return false;
				}
				if (m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && StringUtil::Find(m_pFocus->GetClass(), _T("WkeWebkitUI")) != -1) {
					return false;
				}
				SetNextTabControl(::GetKeyState(VK_SHIFT) >= 0);
				return false;
			}
		}
		break;
		case WM_SYSCHAR:
		{
			// Handle ALT-shortcut key-combinations
			FINDSHORTCUT fs = { 0 };
			fs.ch = toupper((int)wParam);
			CControlUI* pControl = m_pRoot->FindControl(__FindControlFromShortcut, &fs, UIFIND_ENABLED | UIFIND_ME_FIRST | UIFIND_TOP_FIRST);
			if (pControl != NULL) {
				pControl->SetFocus();
				pControl->Activate();
				return true;
			}
		}
		break;
		case WM_SYSKEYDOWN:
		{
			if (m_pFocus != NULL) {
				TEventUI event = { 0 };
				event.Type = UIEVENT_SYSKEY;
				event.chKey = (TCHAR)wParam;
				event.ptMouse = m_ptLastMousePos;
				event.wKeyState = MapKeyState();
				event.dwTimestamp = ::GetTickCount();
				m_pFocus->Event(event);
			}
		}
		break;
		}
		return false;
	}

	bool CPaintManagerUI::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes)
	{
		if (m_hWndPaint == NULL) return false;
		// Cycle through listeners
		for (int i = 0; i < m_aMessageFilters.GetSize(); i++)
		{
			bool bHandled = false;

			LRESULT lResult = static_cast<IMessageFilterUI*>(m_aMessageFilters[i])->MessageHandler(uMsg, wParam, lParam, bHandled);
			if (bHandled) {
				lRes = lResult;
				switch (uMsg) {
				case WM_MOUSEMOVE:
				case WM_LBUTTONDOWN:
				case WM_LBUTTONDBLCLK:
				case WM_LBUTTONUP:
				{
					POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
					m_ptLastMousePos = pt;
				}
				break;
				case WM_CONTEXTMENU:
				case WM_MOUSEWHEEL:
				{
					POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
					::ScreenToClient(m_hWndPaint, &pt);
					m_ptLastMousePos = pt;
				}
				break;
				}
				return true;
			}
		}

		if (m_bLayered) {
			switch (uMsg) {
			case WM_NCACTIVATE:
				if (!::IsIconic(m_hWndPaint)) {
					lRes = (wParam == 0) ? TRUE : FALSE;
					return true;
				}
				break;
			case WM_NCCALCSIZE:
			case WM_NCPAINT:
				lRes = 0;
				return true;
			}
		}
		// Custom handling of events
		switch (uMsg) {
		case WM_APP + 1:
		{
			for (int i = 0; i < m_aDelayedCleanup.GetSize(); i++)
				delete static_cast<CControlUI*>(m_aDelayedCleanup[i]);
			m_aDelayedCleanup.Empty();

			m_bAsyncNotifyPosted = false;

			TNotifyUI* pMsg = NULL;
			while (pMsg = static_cast<TNotifyUI*>(m_aAsyncNotify.GetAt(0))) {
				m_aAsyncNotify.Remove(0);
				if (pMsg->pSender != NULL) {
					if (pMsg->pSender->OnNotify) pMsg->pSender->OnNotify(pMsg);
				}
				for (int j = 0; j < m_aNotifiers.GetSize(); j++) {
					static_cast<INotifyUI*>(m_aNotifiers[j])->Notify(*pMsg);
				}
				delete pMsg;
			}
		}
		break;
		case WM_CLOSE:
		{
			// Make sure all matching "closing" events are sent
			TEventUI event = { 0 };
			event.ptMouse = m_ptLastMousePos;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			if (m_pEventHover != NULL) {
				event.Type = UIEVENT_MOUSELEAVE;
				event.pSender = m_pEventHover;
				m_pEventHover->Event(event);
			}
			if (m_pEventClick != NULL) {
				event.Type = UIEVENT_BUTTONUP;
				event.pSender = m_pEventClick;
				m_pEventClick->Event(event);
			}
			if (m_pEventRClick != NULL)
			{
				event.Type = UIEVENT_RBUTTONUP;
				event.pSender = m_pEventClick;
				m_pEventRClick->Event(event);
			}

			SetFocus(NULL);

			if (::GetActiveWindow() == m_hWndPaint) {
				HWND hwndParent = GetWindowOwner(m_hWndPaint);
				if (hwndParent != NULL) ::SetFocus(hwndParent);
			}

			if (m_hwndTooltip != NULL) {
				::DestroyWindow(m_hwndTooltip);
				m_hwndTooltip = NULL;
			}
		}
		break;
		case WM_ERASEBKGND:
		{
			// We'll do the painting here...
			lRes = 1;
		}
		return true;
		case WM_PAINT:
		{
			return HandlePaintMessage();
		}
		case WM_PRINTCLIENT:
		{
			if (!HandlePrintClientMessage((HDC)wParam, lParam)) {
				break;
			}
			return true;
		}
		break;
		case WM_GETMINMAXINFO:
		{
			MONITORINFO Monitor = {};
			Monitor.cbSize = sizeof(Monitor);
			::GetMonitorInfo(::MonitorFromWindow(m_hWndPaint, MONITOR_DEFAULTTOPRIMARY), &Monitor);
			RECT rcWork = Monitor.rcWork;
			if (Monitor.dwFlags != MONITORINFOF_PRIMARY) {
				::OffsetRect(&rcWork, -rcWork.left, -rcWork.top);
			}

			LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
			if (m_szMinWindow.cx > 0) lpMMI->ptMinTrackSize.x = m_szMinWindow.cx;
			if (m_szMinWindow.cy > 0) lpMMI->ptMinTrackSize.y = m_szMinWindow.cy;
			if (m_szMaxWindow.cx > 0) lpMMI->ptMaxTrackSize.x = m_szMaxWindow.cx;
			if (m_szMaxWindow.cy > 0) lpMMI->ptMaxTrackSize.y = m_szMaxWindow.cy;
			if (m_szMaxWindow.cx > 0) lpMMI->ptMaxSize.x = m_szMaxWindow.cx;
			if (m_szMaxWindow.cy > 0) lpMMI->ptMaxSize.y = m_szMaxWindow.cy;
		}
		break;
		case WM_SIZE:
		{
			if (m_pFocus != NULL) {
				TEventUI event = { 0 };
				event.Type = UIEVENT_WINDOWSIZE;
				event.pSender = m_pFocus;
				event.dwTimestamp = ::GetTickCount();
				m_pFocus->Event(event);
			}
			if (m_pRoot != NULL) m_pRoot->NeedUpdate();
		}
		return true;
		case WM_TIMER:
		{
			for (int i = 0; i < m_aTimers.GetSize(); i++) {
				const TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
				if (pTimer->hWnd == m_hWndPaint &&
					pTimer->uWinTimer == LOWORD(wParam) &&
					pTimer->bKilled == false)
				{
					TEventUI event = { 0 };
					event.Type = UIEVENT_TIMER;
					event.pSender = pTimer->pSender;
					event.dwTimestamp = ::GetTickCount();
					event.ptMouse = m_ptLastMousePos;
					event.wKeyState = MapKeyState();
					event.wParam = pTimer->nLocalID;
					event.lParam = lParam;
					pTimer->pSender->Event(event);
					break;
				}
			}
		}
		break;
		case WM_MOUSEHOVER:
		{
			m_bMouseTracking = false;
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			CControlUI* pHover = FindControl(pt);
			if (pHover == NULL) break;
			if (pHover->IsMouseEnabled() == false)
				break;
			if (pHover->IsEnabled() == false)
				break;

			// Generate mouse hover event
			if (m_pEventHover != NULL) {
				TEventUI event = { 0 };
				event.Type = UIEVENT_MOUSEHOVER;
				event.pSender = m_pEventHover;
				event.wParam = wParam;
				event.lParam = lParam;
				event.dwTimestamp = ::GetTickCount();
				event.ptMouse = pt;
				event.wKeyState = MapKeyState();
				m_pEventHover->Event(event);
			}
			// Create tooltip information

			std::wstring sToolTip = pHover->GetToolTip();
			if (sToolTip.empty()) return true;

			m_sToolTipText = sToolTip;
			::ZeroMemory(&m_ToolTip, sizeof(TOOLINFO));
			m_ToolTip.cbSize = sizeof(TOOLINFO);
			m_ToolTip.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
			m_ToolTip.hwnd = m_hWndPaint;
			m_ToolTip.uId = (UINT_PTR)m_hWndPaint;
			m_ToolTip.hinst = m_hInstance;
			m_ToolTip.lpszText = const_cast<LPTSTR>(m_sToolTipText.c_str());
			m_ToolTip.rect = pHover->GetPos();

			#ifdef UserToolTipWnd
				POINT ptLeftTop = { m_ToolTip.rect.left, m_ToolTip.rect.top };
				POINT ptRightBottom = { m_ToolTip.rect.right, m_ToolTip.rect.bottom };
				::ClientToScreen(m_hWndPaint, &ptLeftTop);
				::ClientToScreen(m_hWndPaint, &ptRightBottom);
				RECT rcPos = { 0 };
				rcPos.left = ptLeftTop.x;
				rcPos.top = ptLeftTop.y;
				rcPos.right = ptRightBottom.x;
				rcPos.bottom = ptRightBottom.y;

				TooltipInfo* pTooltipInfo = new TooltipInfo();
				pTooltipInfo->strText = sToolTip;
				pTooltipInfo->rcPos = rcPos;
				pTooltipInfo->emToolTipType = pHover->GetToolTipShowMode();
				pTooltipInfo->szTooltipGap = pHover->GetToolTipGap();
				pTooltipInfo->nMaxWidth = pHover->GetToolTipWidth();
				pTooltipInfo->nDpi = static_cast<int>(GetDPI());

				if (m_hwndTooltip == NULL)
				{
					TooltipWnd* tooltip = new TooltipWnd();
					tooltip->Create(m_hWndPaint, _T(""), 0, WS_EX_WINDOWEDGE | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW);
					::ShowWindow(tooltip->GetHWND(), SW_SHOWNOACTIVATE);
					m_hwndTooltip = tooltip->GetHWND();
				}

				// 闂侇偅淇虹换鍐ㄢ槈閸喍绱栭柛娆愬灴閳?TooltipInfo 闁圭娲幏锟犳晬鐎涚oltipWnd 閻犳劗鍠曢惌妤呮煂婵犲啯鏉归柛鎰噹閻?
				::SendMessage(m_hwndTooltip, 11858, 0, (LPARAM)pTooltipInfo);
			#else

			if (m_hwndTooltip == NULL)
			{
				m_hwndTooltip = ::CreateWindowEx(
					WS_EX_TOPMOST,
					TOOLTIPS_CLASS,
					NULL,
					WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
					CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
					m_hWndPaint, NULL, m_hInstance, NULL);

				::SetWindowPos(m_hwndTooltip, HWND_TOPMOST, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

				::SendMessage(m_hwndTooltip, TTM_ACTIVATE, TRUE, 0);
				::SendMessage(m_hwndTooltip, TTM_ADDTOOL, 0, (LPARAM)&m_ToolTip);
				::SendMessage(m_hwndTooltip, TTM_SETMAXTIPWIDTH, 0, pHover->GetToolTipWidth());
			}

			POINT ptScreen = pt;
			::ClientToScreen(m_hWndPaint, &ptScreen);
			::SendMessage(m_hwndTooltip, TTM_SETTOOLINFO, 0, (LPARAM)&m_ToolTip);
			::SendMessage(m_hwndTooltip, TTM_TRACKPOSITION, 0, MAKELPARAM(ptScreen.x, ptScreen.y));
			::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ToolTip);
			#endif 

			
		}
		return true;
		case WM_MOUSELEAVE:
		{
#ifdef UserToolTipWnd
			if (m_hwndTooltip != NULL)
			{
				::ShowWindow(m_hwndTooltip, SW_HIDE);
			}
			//::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ToolTip);
			if (m_bMouseTracking) {
				POINT pt = { 0 };
				RECT rcWnd = { 0 };
				::GetCursorPos(&pt);
				::GetWindowRect(m_hWndPaint, &rcWnd);
				if (!::IsIconic(m_hWndPaint) && ::GetActiveWindow() == m_hWndPaint && ::PtInRect(&rcWnd, pt)) {
					if (::SendMessage(m_hWndPaint, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) == HTCLIENT) {
						::ScreenToClient(m_hWndPaint, &pt);
						::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
					}
					else
						::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM)-1);
				}
				else
					::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM)-1);
			}
			m_bMouseTracking = false;
#else

			if( m_hwndTooltip != NULL ) 
				::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
			if( m_bMouseTracking )
			{
				POINT pt = { 0 };
				RECT rcWnd = { 0 };
				::GetCursorPos(&pt);
				::GetWindowRect(m_hWndPaint, &rcWnd);
				if( !::IsIconic(m_hWndPaint) && ::GetActiveWindow() == m_hWndPaint && ::PtInRect(&rcWnd, pt) ) {
					if( ::SendMessage(m_hWndPaint, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y)) == HTCLIENT ) {
						::ScreenToClient(m_hWndPaint, &pt);
						::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
					}
					else 
						::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM)-1);
				}
				else 
					::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM)-1);
			}
			m_bMouseTracking = false;

#endif


		}
		break;
		case WM_MOUSEMOVE:
		{
			// Start tracking this entire window again...
			//闁告瑦鍨块埀顑跨窔缁卞爼寮介崶銊ヤ壕闁?
			if (!m_bMouseTracking) {
				TRACKMOUSEEVENT tme = { 0 };
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_HOVER | TME_LEAVE;
				tme.hwndTrack = m_hWndPaint;
				tme.dwHoverTime = 100;
				//tme.dwHoverTime = m_hwndTooltip == NULL ? m_iHoverTime : (DWORD) ::SendMessage(m_hwndTooltip, TTM_GETDELAYTIME, TTDT_INITIAL, 0L);
				_TrackMouseEvent(&tme);
				m_bMouseTracking = true;
			}

			// Generate the appropriate mouse messages
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			// 闁哄嫷鍨伴幆浣虹矓鐠囨彃袟
			bool bNeedDrag = true;
			if (m_ptLastMousePos.x == pt.x && m_ptLastMousePos.y == pt.y) {
				bNeedDrag = false;
			}
			// 閻犱焦婢樼紞宥單楅悩宕囧灱濞达絽绉堕悿?
			m_ptLastMousePos = pt;
			CControlUI* pNewHover = FindControl(pt);
			if (pNewHover != NULL && pNewHover->GetManager() != this) break;

			// 闁归攱鐗楃€氭寧绂嶇€ｂ晜顐?			if (bNeedDrag && m_bDragMode && wParam == MK_LBUTTON)
			{
				// 闂佹彃锕ラ弬涓哸pture
				::ReleaseCapture();
				// 闁规亽鍎辫ぐ?
				if (m_pDragDrop != NULL && m_pDragDrop->OnDragDrop(m_pEventClick)) {

					m_bDragMode = false;
					break;
				}

				CIDropSource* pdsrc = new CIDropSource;
				if (pdsrc == NULL) return 0;
				pdsrc->AddRef();

				CIDataObject* pdobj = new CIDataObject(pdsrc);
				if (pdobj == NULL) return 0;
				pdobj->AddRef();

				FORMATETC fmtetc = { 0 };
				STGMEDIUM medium = { 0 };
				fmtetc.dwAspect = DVASPECT_CONTENT;
				fmtetc.lindex = -1;
				fmtetc.cfFormat = CF_BITMAP;
				fmtetc.tymed = TYMED_GDI;

				//////////////////////////////////////
				HBITMAP hBitmap = (HBITMAP)OleDuplicateData(m_hDragBitmap, fmtetc.cfFormat, NULL);
				medium.hBitmap = hBitmap;
				pdobj->SetData(&fmtetc, &medium, FALSE);

				//////////////////////////////////////
				BITMAP bmap;
				GetObject(hBitmap, sizeof(BITMAP), &bmap);
				RECT rc = { 0, 0, bmap.bmWidth, bmap.bmHeight };
				fmtetc.cfFormat = CF_ENHMETAFILE;
				fmtetc.tymed = TYMED_ENHMF;
				HDC hReferenceDC = GetMeasureDC();
				if (hReferenceDC != NULL) {
					HDC hMetaDC = CreateEnhMetaFile(hReferenceDC, NULL, NULL, NULL);
					HDC hdcMem = CreateCompatibleDC(hReferenceDC);
					if (hMetaDC != NULL && hdcMem != NULL) {
						HGDIOBJ hOldBmp = ::SelectObject(hdcMem, hBitmap);
						::BitBlt(hMetaDC, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
						::SelectObject(hdcMem, hOldBmp);
						medium.hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
						DeleteDC(hdcMem);
						if (medium.hEnhMetaFile != NULL) {
							medium.tymed = TYMED_ENHMF;
							pdobj->SetData(&fmtetc, &medium, TRUE);
						}
					}
					else {
						if (hdcMem != NULL) {
							DeleteDC(hdcMem);
						}
						if (hMetaDC != NULL) {
							HENHMETAFILE hEnhMetaFile = CloseEnhMetaFile(hMetaDC);
							if (hEnhMetaFile != NULL) {
								DeleteEnhMetaFile(hEnhMetaFile);
							}
						}
					}
				}
				//////////////////////////////////////
				CDragSourceHelper dragSrcHelper;
				POINT ptDrag = { 0 };
				ptDrag.x = bmap.bmWidth / 2;
				ptDrag.y = bmap.bmHeight / 2;
				dragSrcHelper.InitializeFromBitmap(hBitmap, ptDrag, rc, pdobj); //will own the bmp
				DWORD dwEffect;
				HRESULT hr = ::DoDragDrop(pdobj, pdsrc, DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
				if (dwEffect) pdsrc->Release();
				else delete pdsrc;
				pdobj->Release();
				m_bDragMode = false;
				break;
			}

			TEventUI event = { 0 };
			event.ptMouse = pt;
			event.wParam = wParam;
			event.lParam = lParam;
			event.dwTimestamp = ::GetTickCount();
			event.wKeyState = MapKeyState();
			if (!IsCaptured()) {
				pNewHover = FindControl(pt);
				if (pNewHover != NULL && pNewHover->GetManager() != this) break;
				if (pNewHover != m_pEventHover && m_pEventHover != NULL) {
					event.Type = UIEVENT_MOUSELEAVE;
					event.pSender = m_pEventHover;

					CStdPtrArray aNeedMouseLeaveNeeded(m_aNeedMouseLeaveNeeded.GetSize());
					aNeedMouseLeaveNeeded.Resize(m_aNeedMouseLeaveNeeded.GetSize());
					::CopyMemory(aNeedMouseLeaveNeeded.c_str(), m_aNeedMouseLeaveNeeded.c_str(), m_aNeedMouseLeaveNeeded.GetSize() * sizeof(LPVOID));
					for (int i = 0; i < aNeedMouseLeaveNeeded.GetSize(); i++) {
						static_cast<CControlUI*>(aNeedMouseLeaveNeeded[i])->Event(event);
					}

					m_pEventHover->Event(event);
					m_pEventHover = NULL;
					::ShowWindow(m_hwndTooltip, SW_HIDE);
#ifdef UserToolTipWnd
#else
					if( m_hwndTooltip != NULL ) 
						::SendMessage(m_hwndTooltip, TTM_TRACKACTIVATE, FALSE, (LPARAM) &m_ToolTip);
#endif
					
				}
				if (pNewHover != m_pEventHover && pNewHover != NULL) {
					event.Type = UIEVENT_MOUSEENTER;
					event.pSender = pNewHover;
					pNewHover->Event(event);
					m_pEventHover = pNewHover;
				}
			}
			if (m_pEventClick != NULL) {
				event.Type = UIEVENT_MOUSEMOVE;
				event.pSender = m_pEventClick;
				m_pEventClick->Event(event);
			}
			else if (pNewHover != NULL) {
				event.Type = UIEVENT_MOUSEMOVE;
				event.pSender = pNewHover;
				pNewHover->Event(event);
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			// We alway set focus back to our app (this helps
			// when Win32 child windows are placed on the dialog
			// and we need to remove them on focus change).
			if (!m_bNoActivate) ::SetFocus(m_hWndPaint);
			if (m_pRoot == NULL) break;
			// 闁哄被鍎叉竟姗€骞掕濞?
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			m_ptLastMousePos = pt;
			CControlUI* pControl = FindControl(pt);
			if (pControl == NULL) break;
			if (pControl->GetManager() != this) break;

			// 闁告垵妫楅ˇ顒勫箯閺嶃劌顏?			if (m_bDragDrop && pControl->IsDragEnabled()) {
				m_bDragMode = true;
				if (m_hDragBitmap != NULL) {
			CRenderEngine::FreeBitmap(m_hDragBitmap);
					m_hDragBitmap = NULL;
				}
				m_hDragBitmap = CRenderEngine::GenerateBitmap(this, pControl, pControl->GetPos());
			}

			// 鐎殿喒鍋撻柛姘煎灡瀹曠喖鎳?
			SetCapture();
			// 濞存粌顑勫▎銏″緞閸曨厽鍊?			m_pEventClick = pControl;
			pControl->SetFocus();

			TEventUI event = { 0 };
			event.Type = UIEVENT_BUTTONDOWN;
			event.pSender = pControl;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
			pControl->Event(event);
		}
		break;
		case WM_LBUTTONDBLCLK:
		{
			if (!m_bNoActivate) ::SetFocus(m_hWndPaint);

			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			m_ptLastMousePos = pt;
			CControlUI* pControl = FindControl(pt);
			if (pControl == NULL) break;
			if (pControl->GetManager() != this) break;
			//SetCapture();
			TEventUI event = { 0 };
			event.Type = UIEVENT_DBLCLICK;
			event.pSender = pControl;
			event.ptMouse = pt;
			event.wParam = wParam;
			event.lParam = lParam;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
			pControl->Event(event);
			m_pEventClick = pControl;
		}
		break;
		case WM_LBUTTONUP:
		{
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			m_ptLastMousePos = pt;
			if (m_pEventClick == NULL) break;
			ReleaseCapture();
			TEventUI event = { 0 };
			event.Type = UIEVENT_BUTTONUP;
			event.pSender = m_pEventClick;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();

			CControlUI* pClick = m_pEventClick;
			m_pEventClick = NULL;
			pClick->Event(event);
		}
		break;
		case WM_RBUTTONDOWN:
		{
			if (!m_bNoActivate) ::SetFocus(m_hWndPaint);
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			m_ptLastMousePos = pt;
			CControlUI* pControl = FindControl(pt);
			if (pControl == NULL) break;
			if (pControl->GetManager() != this) break;
			pControl->SetFocus();
			SetCapture();
			TEventUI event = { 0 };
			event.Type = UIEVENT_RBUTTONDOWN;
			event.pSender = pControl;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
			pControl->Event(event);
			m_pEventRClick = pControl;
		}
		break;
		case WM_RBUTTONUP:
		{
			if (m_bMouseCapture) ReleaseCapture();

			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			m_ptLastMousePos = pt;
			m_pEventRClick = FindControl(pt);
			if (m_pEventRClick == NULL) break;

			TEventUI event = { 0 };
			event.Type = UIEVENT_RBUTTONUP;
			event.pSender = m_pEventRClick;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
			m_pEventRClick->Event(event);
		}
		break;
		case WM_MBUTTONDOWN:
		{
			if (!m_bNoActivate) ::SetFocus(m_hWndPaint);
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			m_ptLastMousePos = pt;
			CControlUI* pControl = FindControl(pt);
			if (pControl == NULL) break;
			if (pControl->GetManager() != this) break;
			pControl->SetFocus();
			SetCapture();
			TEventUI event = { 0 };
			event.Type = UIEVENT_MBUTTONDOWN;
			event.pSender = pControl;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
			pControl->Event(event);
			m_pEventClick = pControl;
		}
		break;
		case WM_MBUTTONUP:
		{
			if (m_bMouseCapture) ReleaseCapture();
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			m_ptLastMousePos = pt;
			m_pEventClick = FindControl(pt);
			if (m_pEventClick == NULL) break;

			TEventUI event = { 0 };
			event.Type = UIEVENT_MBUTTONUP;
			event.pSender = m_pEventClick;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
			m_pEventClick->Event(event);
		}
		break;
		case WM_CONTEXTMENU:
		{
			if (m_pRoot == NULL) break;
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			::ScreenToClient(m_hWndPaint, &pt);
			m_ptLastMousePos = pt;
			if (m_pEventRClick == NULL) break;
			ReleaseCapture();
			TEventUI event = { 0 };
			event.Type = UIEVENT_CONTEXTMENU;
			event.pSender = m_pEventRClick;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.lParam = (LPARAM)m_pEventRClick;
			event.dwTimestamp = ::GetTickCount();
			m_pEventRClick->Event(event);
			m_pEventRClick = NULL;
		}
		break;
		case WM_MOUSEWHEEL:
		{
			if (m_pRoot == NULL) break;
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			::ScreenToClient(m_hWndPaint, &pt);
			m_ptLastMousePos = pt;
			CControlUI* pControl = FindControl(pt);
			if (pControl == NULL) break;
			if (pControl->GetManager() != this) break;
			int zDelta = (int)(short)HIWORD(wParam);
			TEventUI event = { 0 };
			event.Type = UIEVENT_SCROLLWHEEL;
			event.pSender = pControl;
			event.wParam = MAKEWPARAM(zDelta < 0 ? SB_LINEDOWN : SB_LINEUP, zDelta);
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			pControl->Event(event);

			// Let's make sure that the scroll item below the cursor is the same as before...
			::SendMessage(m_hWndPaint, WM_MOUSEMOVE, 0, (LPARAM)MAKELPARAM(m_ptLastMousePos.x, m_ptLastMousePos.y));
		}
		break;
		case WM_CHAR:
		{
			if (m_pRoot == NULL) break;
			if (m_pFocus == NULL) break;
			TEventUI event = { 0 };
			event.Type = UIEVENT_CHAR;
			event.pSender = m_pFocus;
			event.wParam = wParam;
			event.lParam = lParam;
			event.chKey = (TCHAR)wParam;
			event.ptMouse = m_ptLastMousePos;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			m_pFocus->Event(event);
		}
		break;
		case WM_KEYDOWN:
		{
			if (m_pRoot == NULL) break;
			if (m_pFocus == NULL) break;
			TEventUI event = { 0 };
			event.Type = UIEVENT_KEYDOWN;
			event.pSender = m_pFocus;
			event.wParam = wParam;
			event.lParam = lParam;
			event.chKey = (TCHAR)wParam;
			event.ptMouse = m_ptLastMousePos;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			m_pFocus->Event(event);
			m_pEventKey = m_pFocus;
		}
		break;
		case WM_KEYUP:
		{
			if (m_pRoot == NULL) break;
			if (m_pEventKey == NULL) break;
			TEventUI event = { 0 };
			event.Type = UIEVENT_KEYUP;
			event.pSender = m_pEventKey;
			event.wParam = wParam;
			event.lParam = lParam;
			event.chKey = (TCHAR)wParam;
			event.ptMouse = m_ptLastMousePos;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			m_pEventKey->Event(event);
			m_pEventKey = NULL;
		}
		break;
		case WM_SETCURSOR:
		{

			if (m_pRoot == NULL) break;
			if (LOWORD(lParam) != HTCLIENT) break;
			if (m_bMouseCapture) return true;

			POINT pt = { 0 };
			::GetCursorPos(&pt);
			::ScreenToClient(m_hWndPaint, &pt);
			CControlUI* pControl = FindControl(pt);
			if (pControl == NULL) break;
			//wstring strName =pControl->GetName().c_str();
			//printf("%s\n",w2s(strName).c_str());
			if ((pControl->GetControlFlags() & UIFLAG_SETCURSOR) == 0)
			{
				break;
			}


			TEventUI event = { 0 };
			event.Type = UIEVENT_SETCURSOR;
			event.pSender = pControl;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			pControl->Event(event);
		}
		return true;
		case WM_SETFOCUS:
		{

			if (m_pFocus != NULL) {
				TEventUI event = { 0 };
				event.Type = UIEVENT_SETFOCUS;
				event.wParam = wParam;
				event.lParam = lParam;
				event.pSender = m_pFocus;
				event.dwTimestamp = ::GetTickCount();
				m_pFocus->Event(event);
			}
			break;
		}
		case WM_KILLFOCUS:
		{
			if (IsCaptured()) ReleaseCapture();
			break;
		}
		case WM_NOTIFY:
		{
			if (lParam == 0) break;
			LPNMHDR lpNMHDR = (LPNMHDR)lParam;
			if (lpNMHDR != NULL) lRes = ::SendMessage(lpNMHDR->hwndFrom, OCM__BASE + uMsg, wParam, lParam);
			return true;
		}
		break;
		case WM_COMMAND:
		{
			if (lParam == 0) break;
			HWND hWndChild = (HWND)lParam;
			lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
			if (lRes != 0) return true;
		}
		break;
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
		{
			// Refer To: http://msdn.microsoft.com/en-us/library/bb761691(v=vs.85).aspx
			// Read-only or disabled edit controls do not send the WM_CTLCOLOREDIT message; instead, they send the WM_CTLCOLORSTATIC message.
			if (lParam == 0) break;
			HWND hWndChild = (HWND)lParam;
			lRes = ::SendMessage(hWndChild, OCM__BASE + uMsg, wParam, lParam);
			if (lRes != 0) return true;
		}
		break;
		default:
			break;
		}
		return false;
	}

	bool CPaintManagerUI::IsUpdateNeeded() const
	{
		return m_bUpdateNeeded;
	}

	void CPaintManagerUI::NeedUpdate()
	{
		m_bUpdateNeeded = true;
	}

	void CPaintManagerUI::Invalidate()
	{
		RECT rcClient = { 0 };
		::GetClientRect(m_hWndPaint, &rcClient);
		AccumulateLayeredUpdateRect(rcClient);
		::InvalidateRect(m_hWndPaint, NULL, FALSE);
	}

	void CPaintManagerUI::Invalidate(RECT& rcItem)
	{
		if (rcItem.left < 0) rcItem.left = 0;
		if (rcItem.top < 0) rcItem.top = 0;
		if (rcItem.right < rcItem.left) rcItem.right = rcItem.left;
		if (rcItem.bottom < rcItem.top) rcItem.bottom = rcItem.top;
		AccumulateLayeredUpdateRect(rcItem);
		::InvalidateRect(m_hWndPaint, &rcItem, FALSE);
	}

	bool CPaintManagerUI::IsValid()
	{
		return m_hWndPaint != NULL && m_pRoot != NULL;
	}

	bool CPaintManagerUI::AttachDialog(CControlUI* pControl)
	{

		m_shadow.Create(this);

		// Reset any previous attachment
		SetFocus(NULL);
		m_pEventKey = NULL;
		m_pEventHover = NULL;
		m_pEventClick = NULL;
		m_pEventRClick = NULL;
		// Remove the existing control-tree. We might have gotten inside this function as
		// a result of an event fired or similar, so we cannot just delete the objects and
		// pull the internal memory of the calling code. We'll delay the cleanup.
		if (m_pRoot != NULL) {
			m_mapPostPaintControls.clear();
			AddDelayedCleanup(m_pRoot);
		}
		ClearNativeWindowCache();
		m_aNativeWindow.Empty();
		m_aNativeWindowControl.Empty();
		// Set the dialog root element
		m_pRoot = pControl;
		// Go ahead...
		m_bUpdateNeeded = true;
		m_bFirstLayout = true;
		m_bFocusNeeded = true;
		// Initiate all control
		return InitControls(pControl);
	}

	bool CPaintManagerUI::InitControls(CControlUI* pControl, CControlUI* pParent /*= NULL*/)
	{

		if (pControl == NULL) return false;
		pControl->SetManager(this, pParent != NULL ? pParent : pControl->GetParent(), true);
		pControl->FindControl(__FindControlFromNameHash, this, UIFIND_ALL);
		return true;
	}

	void CPaintManagerUI::ReapObjects(CControlUI* pControl)
	{
		if (pControl == m_pEventKey) m_pEventKey = NULL;
		if (pControl == m_pEventHover) m_pEventHover = NULL;
		if (pControl == m_pEventClick) m_pEventClick = NULL;
		if (pControl == m_pEventRClick) m_pEventRClick = NULL;
		if (pControl == m_pFocus) m_pFocus = NULL;
		if (pControl == nullptr)
			return;

		KillTimer(pControl);
		const std::wstring& sName = pControl->GetName();
		if (!sName.empty()) {
			if (pControl == FindControl(sName))
			{
				auto iter = m_mapName.find(sName.c_str());
				if(iter!=m_mapName.end())
					m_mapName.erase(iter);
			}
		}
		for (int i = 0; i < m_aAsyncNotify.GetSize(); i++) {
			TNotifyUI* pMsg = static_cast<TNotifyUI*>(m_aAsyncNotify[i]);
			if (pMsg->pSender == pControl) pMsg->pSender = NULL;
		}
	}

	bool CPaintManagerUI::AddOptionGroup(std::wstring_view groupName, CControlUI* pControl)
	{
		if (groupName.empty() || pControl == nullptr) {
			return false;
		}

		const auto it = m_mOptionGroup.find(groupName);
		if (it != m_mOptionGroup.end()) {
			CStdPtrArray* aOptionGroup = it->second;
			for (int i = 0; i < aOptionGroup->GetSize(); i++) {
				if (static_cast<CControlUI*>(aOptionGroup->GetAt(i)) == pControl) {
					return false;
				}
			}
			aOptionGroup->Add(pControl);
		}
		else {
			CStdPtrArray* aOptionGroup = new CStdPtrArray(6);
			aOptionGroup->Add(pControl);
			m_mOptionGroup.emplace(std::wstring(groupName), aOptionGroup);
		}
		return true;
	}

	CStdPtrArray* CPaintManagerUI::GetOptionGroup(std::wstring_view groupName)
	{
		if (groupName.empty()) {
			return NULL;
		}

		const auto it = m_mOptionGroup.find(groupName);
		return it != m_mOptionGroup.end() ? it->second : NULL;
	}

	void CPaintManagerUI::RemoveOptionGroup(std::wstring_view groupName, CControlUI* pControl)
	{
		if (groupName.empty() || pControl == nullptr) {
			return;
		}

		const auto it = m_mOptionGroup.find(groupName);
		if (it != m_mOptionGroup.end()) {
			CStdPtrArray* aOptionGroup = it->second;
			if (aOptionGroup == NULL) return;
			for (int i = 0; i < aOptionGroup->GetSize(); i++) {
				if (static_cast<CControlUI*>(aOptionGroup->GetAt(i)) == pControl) {
					aOptionGroup->Remove(i);
					break;
				}
			}
			if (aOptionGroup->IsEmpty()) {
				delete aOptionGroup;
				m_mOptionGroup.erase(it);
			}
		}
	}

	void CPaintManagerUI::RemoveAllOptionGroups()
	{
		for (auto& entry : m_mOptionGroup) {
			delete entry.second;
			entry.second = NULL;
		}
		m_mOptionGroup.clear();
	}

	void CPaintManagerUI::MessageLoop()
	{
		MSG msg = { 0 };
		while (::GetMessage(&msg, NULL, 0, 0)) {
			if (!CPaintManagerUI::TranslateMessage(&msg)) {
				::TranslateMessage(&msg);
				try {
					::DispatchMessage(&msg);
				}
				catch (...) {
					//DUITRACE(_T("EXCEPTION: %s(%d)\n"), __FILET__, __LINE__);
#ifdef _DEBUG
					throw "CPaintManagerUI::MessageLoop";
#endif
				}
			}
		}
	}

	void CPaintManagerUI::Term()
	{
		// 闂佸簱鍋撴慨锝勬祰缁侇偄鈹冮幇顕呭悁闁荤偛妫楀▍?
		CResourceManager::GetInstance()->Release();
		CControlFactory::GetInstance()->Release();

		// 婵炴挸鎳愰幃濠囧礂閸欐﹢鐓╅悹褍瀚花?
		// 闁搞儱澧芥晶?
		TImageInfo* data;
		for (auto& pair : m_SharedResInfo.m_ImageHash) {
			data = pair.second;
			if (data) {
				CRenderEngine::FreeImage(data);
			}
		}
		m_SharedResInfo.m_ImageHash.clear();

		// Custom fonts
		for (auto& entry : m_SharedResInfo.m_CustomFonts) {
			TFontInfo* pFontInfo = entry.second;
			if (pFontInfo != NULL) {
				::DeleteObject(pFontInfo->hFont);
				delete pFontInfo;
				entry.second = NULL;
			}
		}
		m_SharedResInfo.m_CustomFonts.clear();
		// 濮掓稒顭堥鑽も偓娑欍仦缂?
		if (m_SharedResInfo.m_DefaultFontInfo.hFont != NULL) {
			::DeleteObject(m_SharedResInfo.m_DefaultFontInfo.hFont);
		}
		// 闁哄秴鍢茬槐?/ 濮掓稒顭堥鑽や沪閻愮补鍋?
		m_SharedResInfo.m_StyleHash.clear();
		m_SharedResInfo.m_AttrHash.clear();

		// 闁稿繑濞婂Λ纰疘P
		if (m_bCachedResourceZip && m_hResourceZip != NULL) {
			CloseZip((HZIP)m_hResourceZip);
			m_hResourceZip = NULL;
		}

		if (m_cbZipBuf)
		{
			delete[] m_cbZipBuf;
			m_cbZipBuf = nullptr;
		}
	}

	CDPI& CPaintManagerUI::EnsureDPIState()
	{
		if (m_pDPI == NULL) {
			m_pDPI = new CDPI;
		}
		return *m_pDPI;
	}

	const CDPI& CPaintManagerUI::EnsureDPIState() const
	{
		return const_cast<CPaintManagerUI*>(this)->EnsureDPIState();
	}

	CDPI* FYUI::CPaintManagerUI::GetDPIObj()
	{
		return &EnsureDPIState();
	}

	const CDPI* CPaintManagerUI::GetDPIObj() const
	{
		return &EnsureDPIState();
	}

	UINT CPaintManagerUI::GetDPI() const
	{
		return EnsureDPIState().GetDPI();
	}

	UINT CPaintManagerUI::GetScale() const
	{
		return EnsureDPIState().GetScale();
	}

	ULONGLONG CPaintManagerUI::GetDPIGeneration() const
	{
		return m_uDPIGeneration;
	}

	void CPaintManagerUI::SeedDPI(int iDPI)
	{
		const UINT dpi = iDPI > 0 ? static_cast<UINT>(iDPI) : 96U;
		CDPI& dpiState = EnsureDPIState();
		const UINT previousScale = dpiState.GetScale();
		dpiState.SetScale(dpi);
		if (previousScale != dpiState.GetScale()) {
			++m_uDPIGeneration;
		}
	}

	void CPaintManagerUI::SyncDPIFrom(const CPaintManagerUI* pSourceManager)
	{
		if (pSourceManager == NULL) {
			return;
		}
		CDPI& dpiState = EnsureDPIState();
		const UINT previousScale = dpiState.GetScale();
		dpiState.CopyContextFrom(pSourceManager->EnsureDPIState());
		if (previousScale != dpiState.GetScale()) {
			++m_uDPIGeneration;
		}
	}

	void CPaintManagerUI::SyncDPIFromWindow(HWND hWnd)
	{
		if (hWnd == NULL) {
			return;
		}
		SeedDPI(CDPI::GetDPIOfWindow(hWnd));
	}

	void FYUI::CPaintManagerUI::SetDPI(int iDPI)
	{
		SetDPI(iDPI, nullptr);
	}

	void CPaintManagerUI::SetDPI(int iDPI, const RECT* pSuggestedWindowRect)
	{
		const UINT dpi = iDPI > 0 ? static_cast<UINT>(iDPI) : 96U;
		const UINT scale1 = GetScale();
		EnsureDPIState().SetScale(dpi);
		const UINT scale2 = GetScale();
		if (scale1 != scale2) {
			++m_uDPIGeneration;
		}
		if (scale1 == scale2 && pSuggestedWindowRect == nullptr) {
			return;
		}

		ResetDPIAssets();

		HWND hPaintWindow = GetPaintWindow();
		if (hPaintWindow != NULL && ::IsWindow(hPaintWindow)) {
			RECT rcWnd = { 0 };
			RECT rcNewWindow = { 0 };
			const RECT* prcNewWindow = pSuggestedWindowRect;

			if (prcNewWindow == nullptr) {
				::GetWindowRect(hPaintWindow, &rcWnd);
				rcNewWindow = rcWnd;
				if (!::IsZoomed(hPaintWindow) && scale1 != 0) {
					rcNewWindow.right = rcWnd.left + ::MulDiv(rcWnd.right - rcWnd.left, static_cast<int>(scale2), static_cast<int>(scale1));
					rcNewWindow.bottom = rcWnd.top + ::MulDiv(rcWnd.bottom - rcWnd.top, static_cast<int>(scale2), static_cast<int>(scale1));
				}
				prcNewWindow = &rcNewWindow;
			}

			if (prcNewWindow != nullptr) {
				::SetWindowPos(
					hPaintWindow,
					NULL,
					prcNewWindow->left,
					prcNewWindow->top,
					prcNewWindow->right - prcNewWindow->left,
					prcNewWindow->bottom - prcNewWindow->top,
					SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}

		if (GetRoot() != NULL) {
			GetRoot()->NeedUpdate();
		}
		if (hPaintWindow != NULL && ::IsWindow(hPaintWindow)) {
			::PostMessage(hPaintWindow, UIMSG_SET_DPI, dpi, scale2);
		}
	}

	void FYUI::CPaintManagerUI::SetAllDPI(int iDPI)
	{
		for (int i = 0; i < m_aPreMessages.GetSize(); i++) {
			CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
			pManager->SetDPI(iDPI);
		}
	}

	void FYUI::CPaintManagerUI::ResetDPIAssets()
	{
		RemoveAllDrawInfos();
		RemoveAllImages();;

		for (const auto& entry : m_ResInfo.m_CustomFonts) {
			RebuildFont(entry.second);
		}
		RebuildFont(&m_ResInfo.m_DefaultFontInfo);

		for (const auto& entry : m_SharedResInfo.m_CustomFonts) {
			RebuildFont(entry.second);
		}
		RebuildFont(&m_SharedResInfo.m_DefaultFontInfo);

		CStdPtrArray* richEditList = FindSubControlsByClass(GetRoot(), _T("RichEditUI"));
		if (richEditList == nullptr)
			return;
		for (int i = 0; i < richEditList->GetSize(); i++)
		{
			CRichEditUI* pT = static_cast<CRichEditUI*>((*richEditList)[i]);
			pT->SetFont(pT->GetFont());

		}
	}

	void FYUI::CPaintManagerUI::RebuildFont(TFontInfo* pFontInfo)
	{
		::DeleteObject(pFontInfo->hFont);
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, pFontInfo->sFontName.c_str(), _TRUNCATE);

		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -ScaleValue(pFontInfo->iSize);
		lf.lfQuality = CLEARTYPE_QUALITY;
		if (pFontInfo->bBold) lf.lfWeight += FW_BOLD;
		if (pFontInfo->bUnderline) lf.lfUnderline = TRUE;
		if (pFontInfo->bItalic) lf.lfItalic = TRUE;
		HFONT hFont = ::CreateFontIndirect(&lf);
		pFontInfo->hFont = hFont;
		::ZeroMemory(&(pFontInfo->tm), sizeof(pFontInfo->tm));
		UpdateFontTextMetrics(GetMeasureDC(), pFontInfo);
	}

	CControlUI* CPaintManagerUI::GetFocus() const
	{
		return m_pFocus;
	}

	void CPaintManagerUI::SetFocus(CControlUI* pControl)
	{
		// Paint manager window has focus?
		HWND hFocusWnd = ::GetFocus();
		if (hFocusWnd != m_hWndPaint && pControl != m_pFocus)
		{
			::SetFocus(m_hWndPaint);
		}
		// Already has focus?
		if (pControl == m_pFocus) return;
		// Remove focus from old control
		if (m_pFocus != NULL)
		{
			TEventUI event = { 0 };
			event.Type = UIEVENT_KILLFOCUS;
			event.pSender = pControl;
			event.dwTimestamp = ::GetTickCount();
			m_pFocus->Event(event);
			SendNotify(m_pFocus, DUI_MSGTYPE_KILLFOCUS);
			m_pFocus = NULL;
		}
		if (pControl == NULL) return;
		// Set focus to new control
		if (pControl != NULL
			&& pControl->GetManager() == this
			&& pControl->IsVisible()
			&& pControl->IsEnabled())
		{
			m_pFocus = pControl;
			TEventUI event = { 0 };
			event.Type = UIEVENT_SETFOCUS;
			event.pSender = pControl;
			event.dwTimestamp = ::GetTickCount();
			m_pFocus->Event(event);
			SendNotify(m_pFocus, DUI_MSGTYPE_SETFOCUS);
		}
	}

	void CPaintManagerUI::SetFocusNeeded(CControlUI* pControl)
	{
		::SetFocus(m_hWndPaint);
		if (pControl == NULL) return;
		if (m_pFocus != NULL) {
			TEventUI event = { 0 };
			event.Type = UIEVENT_KILLFOCUS;
			event.pSender = pControl;
			event.dwTimestamp = ::GetTickCount();
			m_pFocus->Event(event);
			SendNotify(m_pFocus, DUI_MSGTYPE_KILLFOCUS);
			m_pFocus = NULL;
		}
		FINDTABINFO info = { 0 };
		info.pFocus = pControl;
		info.bForward = false;
		m_pFocus = m_pRoot->FindControl(__FindControlFromTab, &info, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
		m_bFocusNeeded = true;
		if (m_pRoot != NULL) m_pRoot->NeedUpdate();
	}

	void CPaintManagerUI::SetNoFocusNeeded()
	{
		m_bFocusNeeded = false;
	}

	bool CPaintManagerUI::SetTimer(CControlUI* pControl, UINT nTimerID, UINT uElapse)
	{

		for (int i = 0; i < m_aTimers.GetSize(); i++) {
			TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
			if (pTimer->pSender == pControl
				&& pTimer->hWnd == m_hWndPaint
				&& pTimer->nLocalID == nTimerID) {
				if (pTimer->bKilled == true) {
					if (::SetTimer(m_hWndPaint, pTimer->uWinTimer, uElapse, NULL)) {
						pTimer->bKilled = false;
						return true;
					}
					return false;
				}
				return false;
			}
		}

		m_uTimerID = (++m_uTimerID) % 0xF0; //0xf1-0xfe闁绘顫夐悾鈺呮偨閵娾斁鍋?
		if (!::SetTimer(m_hWndPaint, m_uTimerID, uElapse, NULL)) return FALSE;
		TIMERINFO* pTimer = new TIMERINFO;
		if (pTimer == NULL) return FALSE;
		pTimer->hWnd = m_hWndPaint;
		pTimer->pSender = pControl;
		pTimer->nLocalID = nTimerID;
		pTimer->uWinTimer = m_uTimerID;
		pTimer->bKilled = false;
		return m_aTimers.Add(pTimer);
	}

	bool CPaintManagerUI::KillTimer(CControlUI* pControl, UINT nTimerID)
	{

		for (int i = 0; i < m_aTimers.GetSize(); i++) {
			TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
			if (pTimer->pSender == pControl
				&& pTimer->hWnd == m_hWndPaint
				&& pTimer->nLocalID == nTimerID)
			{
				if (pTimer->bKilled == false) {
					if (::IsWindow(m_hWndPaint)) ::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
					pTimer->bKilled = true;
					return true;
				}
			}
		}
		return false;
	}

	void CPaintManagerUI::KillTimer(CControlUI* pControl)
	{

		int count = m_aTimers.GetSize();
		for (int i = 0, j = 0; i < count; i++) {
			TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i - j]);
			if (pTimer->pSender == pControl && pTimer->hWnd == m_hWndPaint) {
				if (pTimer->bKilled == false) ::KillTimer(pTimer->hWnd, pTimer->uWinTimer);
				delete pTimer;
				m_aTimers.Remove(i - j);
				j++;
			}
		}
	}

	void CPaintManagerUI::RemoveAllTimers()
	{
		for (int i = 0; i < m_aTimers.GetSize(); i++) {
			TIMERINFO* pTimer = static_cast<TIMERINFO*>(m_aTimers[i]);
			if (pTimer->hWnd == m_hWndPaint) {
				if (pTimer->bKilled == false) {
					if (::IsWindow(m_hWndPaint)) ::KillTimer(m_hWndPaint, pTimer->uWinTimer);
				}
				delete pTimer;
			}
		}

		m_aTimers.Empty();
	}

	void CPaintManagerUI::SetCapture()
	{
		::SetCapture(m_hWndPaint);
		m_bMouseCapture = true;
	}

	void CPaintManagerUI::ReleaseCapture()
	{
		::ReleaseCapture();
		m_bMouseCapture = false;
		m_bDragMode = false;
	}

	bool CPaintManagerUI::IsCaptured()
	{
		return m_bMouseCapture;
	}

	bool CPaintManagerUI::IsPainting()
	{
		return m_bIsPainting;
	}

	void CPaintManagerUI::SetPainting(bool bIsPainting)
	{
		m_bIsPainting = bIsPainting;
	}

	bool CPaintManagerUI::SetNextTabControl(bool bForward)
	{
		// If we're in the process of restructuring the layout we can delay the
		// focus calulation until the next repaint.
		if (m_bUpdateNeeded && bForward) {
			m_bFocusNeeded = true;
			::InvalidateRect(m_hWndPaint, NULL, FALSE);
			return true;
		}
		// Find next/previous tabbable control
		FINDTABINFO info1 = { 0 };
		info1.pFocus = m_pFocus;
		info1.bForward = bForward;
		CControlUI* pControl = m_pRoot->FindControl(__FindControlFromTab, &info1, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
		if (pControl == NULL) {
			if (bForward) {
				// Wrap around
				FINDTABINFO info2 = { 0 };
				info2.pFocus = bForward ? NULL : info1.pLast;
				info2.bForward = bForward;
				pControl = m_pRoot->FindControl(__FindControlFromTab, &info2, UIFIND_VISIBLE | UIFIND_ENABLED | UIFIND_ME_FIRST);
			}
			else {
				pControl = info1.pLast;
			}
		}
		if (pControl != NULL) SetFocus(pControl);
		m_bFocusNeeded = false;
		return true;
	}

	bool CPaintManagerUI::AddNotifier(INotifyUI* pNotifier)
	{

		return m_aNotifiers.Add(pNotifier);
	}

	bool CPaintManagerUI::RemoveNotifier(INotifyUI* pNotifier)
	{
		for (int i = 0; i < m_aNotifiers.GetSize(); i++) {
			if (static_cast<INotifyUI*>(m_aNotifiers[i]) == pNotifier) {
				return m_aNotifiers.Remove(i);
			}
		}
		return false;
	}

	bool CPaintManagerUI::AddPreMessageFilter(IMessageFilterUI* pFilter)
	{

		return m_aPreMessageFilters.Add(pFilter);
	}

	bool CPaintManagerUI::RemovePreMessageFilter(IMessageFilterUI* pFilter)
	{
		for (int i = 0; i < m_aPreMessageFilters.GetSize(); i++) {
			if (static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i]) == pFilter) {
				return m_aPreMessageFilters.Remove(i);
			}
		}
		return false;
	}

	bool CPaintManagerUI::AddMessageFilter(IMessageFilterUI* pFilter)
	{

		return m_aMessageFilters.Add(pFilter);
	}

	bool CPaintManagerUI::RemoveMessageFilter(IMessageFilterUI* pFilter)
	{
		for (int i = 0; i < m_aMessageFilters.GetSize(); i++) {
			if (static_cast<IMessageFilterUI*>(m_aMessageFilters[i]) == pFilter) {
				return m_aMessageFilters.Remove(i);
			}
		}
		return false;
	}

	int CPaintManagerUI::GetPostPaintCount() const
	{
		const size_t count = m_mapPostPaintControls.size();
		if (count > static_cast<size_t>(INT_MAX)) {
			return INT_MAX;
		}
		return static_cast<int>(count);
	}

	bool CPaintManagerUI::IsPostPaint(CControlUI* pControl)
	{
		auto iter = m_mapPostPaintControls.find(pControl);
		if (iter != m_mapPostPaintControls.end())
			return true;
		return false;
	}

	bool CPaintManagerUI::AddPostPaint(CControlUI* pControl)
	{
		m_mapPostPaintControls[pControl] = 0;
		return true;
	}

	bool CPaintManagerUI::RemovePostPaint(CControlUI* pControl)
	{
		auto iter = m_mapPostPaintControls.find(pControl);
		if (iter != m_mapPostPaintControls.end())
			m_mapPostPaintControls.erase(iter);
		return true;
	}

	bool CPaintManagerUI::SetPostPaintIndex(CControlUI* pControl, int iIndex)
	{
		return true;
		//RemovePostPaint(pControl);
		//return m_aPostPaintControls.InsertAt(iIndex, pControl);
	}

	int CPaintManagerUI::GetNativeWindowCount() const
	{
		return m_aNativeWindow.GetSize();
	}

	bool CPaintManagerUI::AddNativeWindow(CControlUI* pControl, HWND hChildWnd)
	{
		if (pControl == NULL || hChildWnd == NULL) return false;

		RECT rcChildWnd = GetNativeWindowRect(hChildWnd);
		Invalidate(rcChildWnd);

		if (m_aNativeWindow.Find(hChildWnd) >= 0) return false;
		if (m_aNativeWindow.Add(hChildWnd)) {
			if (!m_aNativeWindowControl.Add(pControl)) {
				m_aNativeWindow.Remove(m_aNativeWindow.GetSize() - 1);
				return false;
			}
			if (!m_aNativeWindowCache.Add(NULL)) {
				RemoveNativeWindowEntryAt(m_aNativeWindow.GetSize() - 1);
				return false;
			}
			return true;
		}
		return false;
	}

	bool CPaintManagerUI::RemoveNativeWindow(HWND hChildWnd)
	{
		for (int i = 0; i < m_aNativeWindow.GetSize(); i++) {
			if (static_cast<HWND>(m_aNativeWindow[i]) == hChildWnd) {
				return RemoveNativeWindowEntryAt(i);
			}
		}
		return false;
	}

	RECT CPaintManagerUI::GetNativeWindowRect(HWND hChildWnd)
	{
		RECT rcChildWnd = { 0 };
		TryGetWindowRectInClient(m_hWndPaint, hChildWnd, rcChildWnd);
		return rcChildWnd;
	}

	void CPaintManagerUI::AddDelayedCleanup(CControlUI* pControl)
	{
		if (pControl == NULL) return;
		pControl->SetManager(this, NULL, false);
		m_aDelayedCleanup.Add(pControl);
		PostAsyncNotify();
	}

	void CPaintManagerUI::AddMouseLeaveNeeded(CControlUI* pControl)
	{
		if (pControl == NULL) return;
		for (int i = 0; i < m_aNeedMouseLeaveNeeded.GetSize(); i++) {
			if (static_cast<CControlUI*>(m_aNeedMouseLeaveNeeded[i]) == pControl) {
				return;
			}
		}
		m_aNeedMouseLeaveNeeded.Add(pControl);
	}

	bool CPaintManagerUI::RemoveMouseLeaveNeeded(CControlUI* pControl)
	{
		if (pControl == NULL) return false;
		for (int i = 0; i < m_aNeedMouseLeaveNeeded.GetSize(); i++) {
			if (static_cast<CControlUI*>(m_aNeedMouseLeaveNeeded[i]) == pControl) {
				return m_aNeedMouseLeaveNeeded.Remove(i);
			}
		}
		return false;
	}

	void CPaintManagerUI::SendNotify(CControlUI* pControl, std::wstring_view message, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/, bool bAsync /*= false*/)
	{
		TNotifyUI Msg;
		Msg.pSender = pControl;
		Msg.sType.assign(message);
		Msg.wParam = wParam;
		Msg.lParam = lParam;
		SendNotify(Msg, bAsync);
	}

	void CPaintManagerUI::SendNotify(TNotifyUI& Msg, bool bAsync /*= false*/)
	{
		Msg.ptMouse = m_ptLastMousePos;
		Msg.dwTimestamp = ::GetTickCount();
		if (m_bUsedVirtualWnd)
		{
			Msg.sVirtualWnd = Msg.pSender->GetVirtualWnd();
		}

		if (!bAsync) {
			// Send to all listeners
			if (Msg.pSender != NULL) {
				if (Msg.pSender->OnNotify) Msg.pSender->OnNotify(&Msg);
			}
			for (int i = 0; i < m_aNotifiers.GetSize(); i++) {
				static_cast<INotifyUI*>(m_aNotifiers[i])->Notify(Msg);
			}
		}
		else {
			TNotifyUI* pMsg = new TNotifyUI;
			pMsg->sVirtualWnd = Msg.sVirtualWnd;
			pMsg->pSender = Msg.pSender;
			pMsg->sType = Msg.sType;
			pMsg->wParam = Msg.wParam;
			pMsg->lParam = Msg.lParam;
			pMsg->ptMouse = Msg.ptMouse;
			pMsg->dwTimestamp = Msg.dwTimestamp;
			m_aAsyncNotify.Add(pMsg);

			PostAsyncNotify();
		}
	}

	LONG_PTR CPaintManagerUI::GetMainWnd()
	{
		LONG_PTR ptrManager = ::GetWindowLongPtr(m_hWndPaint, GWLP_USERDATA);
		return ptrManager;
	}

	bool CPaintManagerUI::IsForceUseSharedRes() const
	{
		return m_bForceUseSharedRes;
	}

	void CPaintManagerUI::SetForceUseSharedRes(bool bForce)
	{
		m_bForceUseSharedRes = bForce;
	}

	DWORD CPaintManagerUI::GetDefaultDisabledColor() const
	{
		return m_ResInfo.m_dwDefaultDisabledColor;
	}

	void CPaintManagerUI::SetDefaultDisabledColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultDisabledColor == m_SharedResInfo.m_dwDefaultDisabledColor)
				m_ResInfo.m_dwDefaultDisabledColor = dwColor;
			m_SharedResInfo.m_dwDefaultDisabledColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultDisabledColor = dwColor;
		}
	}

	DWORD CPaintManagerUI::GetDefaultFontColor() const
	{
		return m_ResInfo.m_dwDefaultFontColor;
	}

	void CPaintManagerUI::SetDefaultFontColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultFontColor == m_SharedResInfo.m_dwDefaultFontColor)
				m_ResInfo.m_dwDefaultFontColor = dwColor;
			m_SharedResInfo.m_dwDefaultFontColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultFontColor = dwColor;
		}
	}

	DWORD CPaintManagerUI::GetDefaultLinkFontColor() const
	{
		return m_ResInfo.m_dwDefaultLinkFontColor;
	}

	void CPaintManagerUI::SetDefaultLinkFontColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultLinkFontColor == m_SharedResInfo.m_dwDefaultLinkFontColor)
				m_ResInfo.m_dwDefaultLinkFontColor = dwColor;
			m_SharedResInfo.m_dwDefaultLinkFontColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultLinkFontColor = dwColor;
		}
	}

	DWORD CPaintManagerUI::GetDefaultLinkHoverFontColor() const
	{
		return m_ResInfo.m_dwDefaultLinkHoverFontColor;
	}

	void CPaintManagerUI::SetDefaultLinkHoverFontColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultLinkHoverFontColor == m_SharedResInfo.m_dwDefaultLinkHoverFontColor)
				m_ResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
			m_SharedResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultLinkHoverFontColor = dwColor;
		}
	}

	DWORD CPaintManagerUI::GetDefaultSelectedBkColor() const
	{
		return m_ResInfo.m_dwDefaultSelectedBkColor;
	}

	void CPaintManagerUI::SetDefaultSelectedBkColor(DWORD dwColor, bool bShared)
	{
		if (bShared)
		{
			if (m_ResInfo.m_dwDefaultSelectedBkColor == m_SharedResInfo.m_dwDefaultSelectedBkColor)
				m_ResInfo.m_dwDefaultSelectedBkColor = dwColor;
			m_SharedResInfo.m_dwDefaultSelectedBkColor = dwColor;
		}
		else
		{
			m_ResInfo.m_dwDefaultSelectedBkColor = dwColor;
		}
	}

	TFontInfo* CPaintManagerUI::GetDefaultFontInfo()
	{
		if (m_ResInfo.m_DefaultFontInfo.sFontName.empty())
		{
			if (m_SharedResInfo.m_DefaultFontInfo.tm.tmHeight == 0)
			{
				UpdateFontTextMetrics(GetMeasureDC(), &m_SharedResInfo.m_DefaultFontInfo);
			}
			return &m_SharedResInfo.m_DefaultFontInfo;
		}
		else
		{
			if (m_ResInfo.m_DefaultFontInfo.tm.tmHeight == 0)
			{
				UpdateFontTextMetrics(GetMeasureDC(), &m_ResInfo.m_DefaultFontInfo);
			}
			return &m_ResInfo.m_DefaultFontInfo;
		}
	}

	void CPaintManagerUI::SetDefaultFont(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared)
	{
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		if (!fontName.empty()) {
			TCHAR szFaceName[32] = { 0 };//_T("@");
			_tcsncat_s(szFaceName, LF_FACESIZE, std::wstring(fontName).c_str(), _TRUNCATE);
			_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, szFaceName, _TRUNCATE);
		}
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -ScaleValue(nSize);;
		if (bBold) lf.lfWeight += FW_BOLD;
		if (bUnderline) lf.lfUnderline = TRUE;
		if (bItalic) lf.lfItalic = TRUE;
		if (bStrikeout) lf.lfStrikeOut = TRUE;

		HFONT hFont = ::CreateFontIndirect(&lf);
		if (hFont == NULL) return;

		if (bShared)
		{
			::DeleteObject(m_SharedResInfo.m_DefaultFontInfo.hFont);
			m_SharedResInfo.m_DefaultFontInfo.hFont = hFont;
			m_SharedResInfo.m_DefaultFontInfo.sFontName = lf.lfFaceName;
			m_SharedResInfo.m_DefaultFontInfo.iSize = nSize;
			m_SharedResInfo.m_DefaultFontInfo.bBold = bBold;
			m_SharedResInfo.m_DefaultFontInfo.bUnderline = bUnderline;
			m_SharedResInfo.m_DefaultFontInfo.bItalic = bItalic;
			m_SharedResInfo.m_DefaultFontInfo.bStrikeout = bStrikeout;
			::ZeroMemory(&m_SharedResInfo.m_DefaultFontInfo.tm, sizeof(m_SharedResInfo.m_DefaultFontInfo.tm));
			UpdateFontTextMetrics(GetMeasureDC(), &m_SharedResInfo.m_DefaultFontInfo);
		}
		else
		{
			::DeleteObject(m_ResInfo.m_DefaultFontInfo.hFont);
			m_ResInfo.m_DefaultFontInfo.hFont = hFont;
			m_ResInfo.m_DefaultFontInfo.sFontName = lf.lfFaceName;
			m_ResInfo.m_DefaultFontInfo.iSize = nSize;
			m_ResInfo.m_DefaultFontInfo.bBold = bBold;
			m_ResInfo.m_DefaultFontInfo.bUnderline = bUnderline;
			m_ResInfo.m_DefaultFontInfo.bItalic = bItalic;
			m_ResInfo.m_DefaultFontInfo.bStrikeout = bStrikeout;
			::ZeroMemory(&m_ResInfo.m_DefaultFontInfo.tm, sizeof(m_ResInfo.m_DefaultFontInfo.tm));
			UpdateFontTextMetrics(GetMeasureDC(), &m_ResInfo.m_DefaultFontInfo);
		}
	}

	DWORD CPaintManagerUI::GetCustomFontCount(bool bShared) const
	{
		if (bShared)
			return static_cast<DWORD>(m_SharedResInfo.m_CustomFonts.size());
		else
			return static_cast<DWORD>(m_ResInfo.m_CustomFonts.size());
	}

	HFONT CPaintManagerUI::AddFont(int id, std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared)
	{
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		if (!fontName.empty()) {
			TCHAR szFaceName[32] = { 0 };//_T("@");
			_tcsncat_s(szFaceName, LF_FACESIZE, std::wstring(fontName).c_str(), _TRUNCATE);
			_tcsncpy_s(lf.lfFaceName, LF_FACESIZE, szFaceName, _TRUNCATE);
		}
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -ScaleValue(nSize);
		if (bBold) lf.lfWeight = FW_BOLD;
		if (bUnderline) lf.lfUnderline = TRUE;
		if (bItalic) lf.lfItalic = TRUE;
		if (bStrikeout) lf.lfStrikeOut = TRUE;
		HFONT hFont = ::CreateFontIndirect(&lf);
		if (hFont == NULL) return NULL;

		TFontInfo* pFontInfo = new TFontInfo;
		if (!pFontInfo) return NULL;
		::ZeroMemory(pFontInfo, sizeof(TFontInfo));
		pFontInfo->hFont = hFont;
		pFontInfo->sFontName = lf.lfFaceName;
		pFontInfo->iSize = nSize;
		pFontInfo->bBold = bBold;
		pFontInfo->bUnderline = bUnderline;
		pFontInfo->bItalic = bItalic;
		pFontInfo->bStrikeout = bStrikeout;
		UpdateFontTextMetrics(GetMeasureDC(), pFontInfo);
		const std::wstring idKey = std::to_wstring(id);
		auto& fontMap = (bShared || m_bForceUseSharedRes) ? m_SharedResInfo.m_CustomFonts : m_ResInfo.m_CustomFonts;
		const auto it = fontMap.find(idKey);
		if (it != fontMap.end())
		{
			TFontInfo* pOldFontInfo = it->second;
			if (pOldFontInfo)
			{
				::DeleteObject(pOldFontInfo->hFont);
				delete pOldFontInfo;
			}
			fontMap.erase(it);
		}

		fontMap.emplace(idKey, pFontInfo);

		return hFont;
	}
	void CPaintManagerUI::AddFontArray(std::wstring_view pstrPath) {
		const std::wstring pathStorage(pstrPath);
		LPBYTE pData = NULL;
		DWORD dwSize = 0;
		if (!LoadBinaryResourceData(STRINGorID(pathStorage.c_str()), pData, dwSize)) {
			return;
		}
		if (pData == NULL || dwSize == 0) {
			return;
		}
		DWORD nFonts;
		HANDLE hFont = ::AddFontMemResourceEx(pData, dwSize, NULL, &nFonts);
		delete[] pData;
		pData = NULL;
		m_aFonts.Add(hFont);
	}
	HFONT CPaintManagerUI::GetFont(int id)
	{
		if (id < 0) return GetDefaultFontInfo()->hFont;

		const std::wstring idKey = std::to_wstring(id);
		TFontInfo* pFontInfo = NULL;
		const auto localIt = m_ResInfo.m_CustomFonts.find(idKey);
		if (localIt != m_ResInfo.m_CustomFonts.end()) pFontInfo = localIt->second;
		if (!pFontInfo) {
			const auto sharedIt = m_SharedResInfo.m_CustomFonts.find(idKey);
			if (sharedIt != m_SharedResInfo.m_CustomFonts.end()) pFontInfo = sharedIt->second;
		}
		if (!pFontInfo) return GetDefaultFontInfo()->hFont;
		return pFontInfo->hFont;
	}

	HFONT CPaintManagerUI::GetFont(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout)
	{
		for (const auto& entry : m_ResInfo.m_CustomFonts) {
			TFontInfo* pFontInfo = entry.second;
			if (pFontInfo && pFontInfo->sFontName == fontName && pFontInfo->iSize == nSize &&
				pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic
				&& pFontInfo->bStrikeout == bStrikeout) {
				return pFontInfo->hFont;
			}
		}
		for (const auto& entry : m_SharedResInfo.m_CustomFonts) {
			TFontInfo* pFontInfo = entry.second;
			if (pFontInfo && pFontInfo->sFontName == fontName && pFontInfo->iSize == nSize &&
				pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic
				&& pFontInfo->bStrikeout == bStrikeout) {
				return pFontInfo->hFont;
			}
		}

		return NULL;
	}

	int CPaintManagerUI::GetFontIndex(HFONT hFont, bool bShared)
	{
		if (bShared)
		{
			for (const auto& entry : m_SharedResInfo.m_CustomFonts) {
				TFontInfo* pFontInfo = entry.second;
				if (pFontInfo && pFontInfo->hFont == hFont) return _wtoi(entry.first.c_str());
			}
		}
		else
		{
			for (const auto& entry : m_ResInfo.m_CustomFonts) {
				TFontInfo* pFontInfo = entry.second;
				if (pFontInfo && pFontInfo->hFont == hFont) return _wtoi(entry.first.c_str());
			}
		}

		return -1;
	}

	int CPaintManagerUI::GetFontIndex(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared)
	{
		if (bShared)
		{
			for (const auto& entry : m_SharedResInfo.m_CustomFonts) {
				TFontInfo* pFontInfo = entry.second;
				if (pFontInfo && pFontInfo->sFontName == fontName && pFontInfo->iSize == nSize &&
					pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic
					&& pFontInfo->bStrikeout == bStrikeout) {
					return _wtoi(entry.first.c_str());
				}
			}
		}
		else
		{
			for (const auto& entry : m_ResInfo.m_CustomFonts) {
				TFontInfo* pFontInfo = entry.second;
				if (pFontInfo && pFontInfo->sFontName == fontName && pFontInfo->iSize == nSize &&
					pFontInfo->bBold == bBold && pFontInfo->bUnderline == bUnderline && pFontInfo->bItalic == bItalic
					&& pFontInfo->bStrikeout == bStrikeout) {
					return _wtoi(entry.first.c_str());
				}
			}
		}

		return -1;
	}

	void CPaintManagerUI::RemoveFont(HFONT hFont, bool bShared)
	{
		if (bShared)
		{
			for (auto it = m_SharedResInfo.m_CustomFonts.begin(); it != m_SharedResInfo.m_CustomFonts.end(); ++it)
			{
				TFontInfo* pFontInfo = it->second;
				if (pFontInfo && pFontInfo->hFont == hFont)
				{
					::DeleteObject(pFontInfo->hFont);
					delete pFontInfo;
					m_SharedResInfo.m_CustomFonts.erase(it);
					return;
				}
			}
		}
		else
		{
			for (auto it = m_ResInfo.m_CustomFonts.begin(); it != m_ResInfo.m_CustomFonts.end(); ++it)
			{
				TFontInfo* pFontInfo = it->second;
				if (pFontInfo && pFontInfo->hFont == hFont)
				{
					::DeleteObject(pFontInfo->hFont);
					delete pFontInfo;
					m_ResInfo.m_CustomFonts.erase(it);
					return;
				}
			}
		}
	}

	void CPaintManagerUI::RemoveFont(int id, bool bShared)
	{
		const std::wstring idKey = std::to_wstring(id);
		if (bShared)
		{
			const auto it = m_SharedResInfo.m_CustomFonts.find(idKey);
			if (it != m_SharedResInfo.m_CustomFonts.end())
			{
				TFontInfo* pFontInfo = it->second;
				::DeleteObject(pFontInfo->hFont);
				delete pFontInfo;
				m_SharedResInfo.m_CustomFonts.erase(it);
			}
		}
		else
		{
			const auto it = m_ResInfo.m_CustomFonts.find(idKey);
			if (it != m_ResInfo.m_CustomFonts.end())
			{
				TFontInfo* pFontInfo = it->second;
				::DeleteObject(pFontInfo->hFont);
				delete pFontInfo;
				m_ResInfo.m_CustomFonts.erase(it);
			}
		}
	}

	void CPaintManagerUI::RemoveAllFonts(bool bShared)
	{
		if (bShared)
		{
			for (auto& entry : m_SharedResInfo.m_CustomFonts) {
				TFontInfo* pFontInfo = entry.second;
				if (pFontInfo) {
					::DeleteObject(pFontInfo->hFont);
					delete pFontInfo;
					entry.second = NULL;
				}
			}
			m_SharedResInfo.m_CustomFonts.clear();
		}
		else
		{
			for (auto& entry : m_ResInfo.m_CustomFonts) {
				TFontInfo* pFontInfo = entry.second;
				if (pFontInfo) {
					::DeleteObject(pFontInfo->hFont);
					delete pFontInfo;
					entry.second = NULL;
				}
			}
			m_ResInfo.m_CustomFonts.clear();
		}
	}

	TFontInfo* CPaintManagerUI::GetFontInfo(int id)
	{
		if (id < 0) return GetDefaultFontInfo();

		const std::wstring idKey = std::to_wstring(id);
		TFontInfo* pFontInfo = NULL;
		const auto localIt = m_ResInfo.m_CustomFonts.find(idKey);
		if (localIt != m_ResInfo.m_CustomFonts.end()) pFontInfo = localIt->second;
		if (!pFontInfo) {
			const auto sharedIt = m_SharedResInfo.m_CustomFonts.find(idKey);
			if (sharedIt != m_SharedResInfo.m_CustomFonts.end()) pFontInfo = sharedIt->second;
		}
		if (!pFontInfo) pFontInfo = GetDefaultFontInfo();
		if (pFontInfo->tm.tmHeight == 0)
		{
			UpdateFontTextMetrics(GetMeasureDC(), pFontInfo);
		}
		return pFontInfo;
	}

	TFontInfo* CPaintManagerUI::GetFontInfo(HFONT hFont)
	{
		TFontInfo* pFontInfo = NULL;
		for (const auto& entry : m_ResInfo.m_CustomFonts)
		{
			pFontInfo = entry.second;
			if (pFontInfo && pFontInfo->hFont == hFont) break;
		}
		if (!pFontInfo)
		{
			for (const auto& entry : m_SharedResInfo.m_CustomFonts)
			{
				pFontInfo = entry.second;
				if (pFontInfo && pFontInfo->hFont == hFont) break;
			}
		}
		if (!pFontInfo) pFontInfo = GetDefaultFontInfo();
		if (pFontInfo->tm.tmHeight == 0) {
			UpdateFontTextMetrics(GetMeasureDC(), pFontInfo);
		}
		return pFontInfo;
	}

	const TImageInfo* CPaintManagerUI::GetImage(std::wstring_view bitmap)
	{
		if (bitmap.empty()) {
			return nullptr;
		}
		const std::wstring bitmapKey(bitmap);
		TImageInfo* data = nullptr;
		auto it = m_ResInfo.m_ImageHash.find(bitmapKey);
		if (it != m_ResInfo.m_ImageHash.end()) data = it->second;
		if (!data) {
			auto it2 = m_SharedResInfo.m_ImageHash.find(bitmapKey);
			if (it2 != m_SharedResInfo.m_ImageHash.end()) data = it2->second;
		}
		return data;
	}

	const TImageInfo* CPaintManagerUI::GetImageEx(std::wstring_view bitmap, std::wstring_view type, DWORD mask, bool bUseHSL, bool bGdiplus, HINSTANCE instance)
	{
		const TImageInfo* data = GetImage(bitmap);
		if (!data) {
			const std::wstring bitmapString(bitmap);
			if (AddImage(bitmap, type, mask, bUseHSL, bGdiplus, false, instance)) {
				if (m_bForceUseSharedRes) {
					auto it = m_SharedResInfo.m_ImageHash.find(bitmapString);
					if (it != m_SharedResInfo.m_ImageHash.end()) data = it->second;
				}
				else {
					auto it = m_ResInfo.m_ImageHash.find(bitmapString);
					if (it != m_ResInfo.m_ImageHash.end()) data = it->second;
				}
			}
		}

		return data;
	}

	const TImageInfo* CPaintManagerUI::AddImage(std::wstring_view bitmap, std::wstring_view type, DWORD mask, bool bUseHSL, bool bGdiplus, bool bShared, HINSTANCE instance)
	{
		if (bitmap.empty()) return NULL;

		const std::wstring bitmapKey(bitmap);
		const std::wstring typeName(type);
		const wchar_t* bitmapPtr = bitmapKey.c_str();
		const wchar_t* typePtr = type.empty() ? nullptr : typeName.c_str();

		TImageInfo* data = NULL;
		if (typePtr != NULL && !type.empty()) {
			if (iswdigit(bitmapKey.front())) {
				LPTSTR pstr = NULL;
				int iIndex = _tcstol(bitmapPtr, &pstr, 10);

				data = bGdiplus ? CRenderEngine::GdiplusLoadImage(iIndex, typePtr, mask, instance) : CRenderEngine::LoadImage(iIndex, ScaleValue(100), typePtr, mask, instance);
			}
		}
		else {
			data = bGdiplus ? CRenderEngine::GdiplusLoadImage(bitmap, {}, mask, instance) : CRenderEngine::LoadImage(bitmap, ScaleValue(100), {}, mask, instance);
			if (!data) {
				std::wstring sImageName = bitmapKey;
				const std::wstring::size_type iAtIdx = sImageName.rfind(_T('@'));
				const std::wstring::size_type iDotIdx = sImageName.rfind(_T('.'));
				if (iAtIdx != std::wstring::npos && iDotIdx != std::wstring::npos) {
					std::wstring sExe = StringUtil::Mid(sImageName, static_cast<int>(iDotIdx));
					sImageName = sImageName.substr(0, iAtIdx) + sExe;
					data = bGdiplus ? CRenderEngine::GdiplusLoadImage(sImageName, {}, mask, instance) : CRenderEngine::LoadImage(sImageName, ScaleValue(100), {}, mask, instance);
				}
			}

		}

		if (data == NULL) {
			return NULL;
		}
		data->bUseHSL = bUseHSL;
		if (!type.empty()) data->sResType = typeName;
		data->dwMask = mask;
		if (data->bUseHSL) {
			data->pSrcBits = new BYTE[data->nX * data->nY * 4];
			::CopyMemory(data->pSrcBits, data->pBits, data->nX * data->nY * 4);
		}
		else data->pSrcBits = NULL;
		if (m_bUseHSL) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);
		if (data)
		{
			if (bShared || m_bForceUseSharedRes)
			{
				auto it = m_SharedResInfo.m_ImageHash.find(bitmapKey);
				if (it != m_SharedResInfo.m_ImageHash.end())
				{
					CRenderEngine::FreeImage(it->second);
					m_SharedResInfo.m_ImageHash.erase(it);
				}

				m_SharedResInfo.m_ImageHash[bitmapKey] = data;
			}
			else
			{
				auto it = m_ResInfo.m_ImageHash.find(bitmapKey);
				if (it != m_ResInfo.m_ImageHash.end())
				{
					CRenderEngine::FreeImage(it->second);
					m_ResInfo.m_ImageHash.erase(it);
				}

				m_ResInfo.m_ImageHash[bitmapKey] = data;
			}
		}

		return data;
	}

	const TImageInfo* CPaintManagerUI::AddImage(std::wstring_view bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared)
	{
		// 闁搞儳濮靛Λ銈呪枖閺囩姭鈧鈧鑹鹃ˇ濠氭焾閳╁ITMAP闁哄秶鍘х槐锟犳晬鐏炶偐鐟濋柤铏灊婵炲洭鎮介埡顦檒閻犲鍟弳?		if (bitmap.empty()) return NULL;
		if (hBitmap == NULL || iWidth <= 0 || iHeight <= 0) return NULL;

		const std::wstring bitmapKey(bitmap);

		TImageInfo* data = new TImageInfo;
		data->pBits = NULL;
		data->pSrcBits = NULL;
		data->hBitmap = hBitmap;
		data->pBits = NULL;
		data->nX = iWidth;
		data->nY = iHeight;
		data->bAlpha = bAlpha;
		data->bUseHSL = false;
		data->pSrcBits = NULL;
		data->dwMask = 0;

		if (bShared || m_bForceUseSharedRes)
		{
			m_SharedResInfo.m_ImageHash[bitmapKey] = data;
		}
		else
		{
			m_ResInfo.m_ImageHash[bitmapKey] = data;
		}

		return data;
	}

	void CPaintManagerUI::RemoveImage(std::wstring_view bitmap, bool bShared)
	{
		if (bitmap.empty()) {
			return;
		}

		const std::wstring bitmapKey(bitmap);
		if (bShared)
		{
			auto it = m_SharedResInfo.m_ImageHash.find(bitmapKey);
			if (it != m_SharedResInfo.m_ImageHash.end())
			{
				CRenderEngine::FreeImage(it->second);
				m_SharedResInfo.m_ImageHash.erase(it);
			}
		}
		else
		{
			auto it = m_ResInfo.m_ImageHash.find(bitmapKey);
			if (it != m_ResInfo.m_ImageHash.end())
			{
				CRenderEngine::FreeImage(it->second);
				m_ResInfo.m_ImageHash.erase(it);
			}
		}
	}

	void CPaintManagerUI::RemoveAllImages(bool bShared)
	{
		if (bShared)
		{
			for (auto& pair : m_SharedResInfo.m_ImageHash) {
				if (pair.second) {
					CRenderEngine::FreeImage(pair.second);
				}
			}
			m_SharedResInfo.m_ImageHash.clear();
		}
		else
		{
			for (auto& pair : m_ResInfo.m_ImageHash) {
				if (pair.second) {
					CRenderEngine::FreeImage(pair.second);
				}
			}
			m_ResInfo.m_ImageHash.clear();
		}
	}

	void CPaintManagerUI::AdjustSharedImagesHSL()
	{
		TImageInfo* data;
		for (auto& pair : m_SharedResInfo.m_ImageHash) {
			data = pair.second;
			if (data && data->bUseHSL) {
				CRenderEngine::AdjustImage(m_bUseHSL, data, m_H, m_S, m_L);
			}
		}
	}

	void CPaintManagerUI::AdjustImagesHSL()
	{
		TImageInfo* data;
		for (auto& pair : m_ResInfo.m_ImageHash) {
			data = pair.second;
			if (data && data->bUseHSL) {
				CRenderEngine::AdjustImage(m_bUseHSL, data, m_H, m_S, m_L);
			}
		}
		for (auto& pair : m_ResInfo.m_DrawInfoHash) {
			if (pair.second != NULL) {
				pair.second->ClearCachedBitmap();
			}
		}
		Invalidate();
	}

	void CPaintManagerUI::PostAsyncNotify()
	{
		if (!m_bAsyncNotifyPosted) {
			::PostMessage(m_hWndPaint, WM_APP + 1, 0, 0L);
			m_bAsyncNotifyPosted = true;
		}
	}
	void CPaintManagerUI::ReloadSharedImages()
	{
		CDPI* pDpi = new CDPI();
		TImageInfo* data = NULL;
		TImageInfo* pNewData = NULL;
		std::vector<std::wstring> keysToRemove;
		for (auto& pair : m_SharedResInfo.m_ImageHash) {
				const wchar_t* bitmap = pair.first.c_str();
			data = pair.second;
				if (data != NULL) {
					if (!data->sResType.empty()) {
						if (isdigit(*bitmap)) {
							LPTSTR pstr = NULL;
							int iIndex = _tcstol(bitmap, &pstr, 10);
							pNewData = CRenderEngine::LoadImage(iIndex, pDpi->Scale(100), data->sResType.c_str(), data->dwMask);
						}
					}
					else {
						pNewData = CRenderEngine::LoadImage(std::wstring_view(bitmap), pDpi->Scale(100), {}, data->dwMask);
					}
					if (pNewData == NULL) {
						CRenderEngine::FreeImage(data);
						keysToRemove.push_back(pair.first);
						continue;
					}

					CRenderEngine::FreeImage(data, false);
					data->hBitmap = pNewData->hBitmap;
					data->pImage = pNewData->pImage;
					data->pBits = pNewData->pBits;
					data->nX = pNewData->nX;
					data->nY = pNewData->nY;
					data->bAlpha = pNewData->bAlpha;
					data->pSrcBits = NULL;
					if (data->bUseHSL) {
						data->pSrcBits = new BYTE[data->nX * data->nY * 4];
						::CopyMemory(data->pSrcBits, data->pBits, data->nX * data->nY * 4);
					}
					else data->pSrcBits = NULL;
					if (m_bUseHSL) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);

					delete pNewData;
				}
		}
		for (const auto& key : keysToRemove) {
			m_SharedResInfo.m_ImageHash.erase(key);
		}

		delete pDpi;
	}

	void CPaintManagerUI::ReloadImages()
	{
		RemoveAllDrawInfos();

		TImageInfo* data = NULL;
		TImageInfo* pNewData = NULL;
		// 濞达綀娉曢弫?vector 闁衡偓閸洘鑲犻梻鍥ｅ亾閻熸洑绀侀崹褰掓⒔閵堝洦鐣?key闁挎稑鐭傛导鈺呭礂瀹ュ鎲鹃柛妯烘濡炲倹绌遍鑺ユ毉閻庡湱鎳撳▍?
		std::vector<std::wstring> keysToRemove;
		for (auto& pair : m_ResInfo.m_ImageHash) {
				const wchar_t* bitmap = pair.first.c_str();
			data = pair.second;
				if (data != NULL) {
					if (!data->sResType.empty()) {
						if (isdigit(*bitmap)) {
							LPTSTR pstr = NULL;
							int iIndex = _tcstol(bitmap, &pstr, 10);
							pNewData = CRenderEngine::LoadImage(iIndex, ScaleValue(100), data->sResType.c_str(), data->dwMask);
						}
					}
					else {
						pNewData = CRenderEngine::LoadImage(std::wstring_view(bitmap), ScaleValue(100), {}, data->dwMask);
					}

					CRenderEngine::FreeImage(data, false);
					if (pNewData == NULL) {
						keysToRemove.push_back(pair.first);
						continue;
					}
					data->hBitmap = pNewData->hBitmap;
					data->pBits = pNewData->pBits;
					data->nX = pNewData->nX;
					data->nY = pNewData->nY;
					data->bAlpha = pNewData->bAlpha;
					data->pSrcBits = NULL;
					if (data->bUseHSL) {
						data->pSrcBits = new BYTE[data->nX * data->nY * 4];
						::CopyMemory(data->pSrcBits, data->pBits, data->nX * data->nY * 4);
					}
					else data->pSrcBits = NULL;
					if (m_bUseHSL) CRenderEngine::AdjustImage(true, data, m_H, m_S, m_L);

					delete pNewData;
				}
		}
		for (auto& key : keysToRemove) {
			m_ResInfo.m_ImageHash.erase(key);
		}

		if (m_pRoot) m_pRoot->Invalidate();
	}

	const TImageInfo* CPaintManagerUI::ModifyImage(std::wstring_view bitmap, HBITMAP hBitmap, LPBYTE pBits, int iWidth, int iHeight, bool bAlpha)
	{
		if (bitmap.empty()) {
			return nullptr;
		}

		const std::wstring bitmapKey(bitmap);
		TImageInfo* data = nullptr;
		auto it = m_ResInfo.m_ImageHash.find(bitmapKey);
		if (it != m_ResInfo.m_ImageHash.end()) data = it->second;
		if (!data) {
			auto it2 = m_SharedResInfo.m_ImageHash.find(bitmapKey);
			if (it2 != m_SharedResInfo.m_ImageHash.end()) data = it2->second;
		}
		if (!data) {
			return nullptr;
		}

		data->hBitmap = hBitmap;
		data->pBits = pBits;
		data->nX = iWidth;
		data->nY = iHeight;
		data->bAlpha = bAlpha;

		return data;
	}

	TDrawInfo* CPaintManagerUI::GetDrawInfo(std::wstring_view image, std::wstring_view modify)
	{
		std::wstring sKey = StringUtil::Format(L"{}{}",
			image,
			modify);

		TDrawInfo* pDrawInfo = nullptr;
		auto it = m_ResInfo.m_DrawInfoHash.find(sKey.c_str());
		if (it != m_ResInfo.m_DrawInfoHash.end()) pDrawInfo = it->second;
		if (pDrawInfo == NULL && !sKey.empty()) {
			pDrawInfo = new TDrawInfo();
			pDrawInfo->Parse(std::wstring(image), modify.empty() ? std::wstring() : std::wstring(modify), this);
			m_ResInfo.m_DrawInfoHash[sKey.c_str()] = pDrawInfo;
		}
		return pDrawInfo;
	}

	void CPaintManagerUI::RemoveDrawInfo(std::wstring_view image, std::wstring_view modify)
	{
		std::wstring sKey = StringUtil::Format(L"{}{}",
			image,
			modify);
		auto it = m_ResInfo.m_DrawInfoHash.find(sKey.c_str());
		if (it != m_ResInfo.m_DrawInfoHash.end()) {
			delete it->second;
			m_ResInfo.m_DrawInfoHash.erase(it);
		}
	}

	void CPaintManagerUI::RemoveAllDrawInfos()
	{
		for (auto& pair : m_ResInfo.m_DrawInfoHash) {
			if (pair.second) {
				delete pair.second;
			}
		}
		m_ResInfo.m_DrawInfoHash.clear();
	}

	void CPaintManagerUI::AddDefaultAttributeList(std::wstring_view controlName, std::wstring_view controlAttrList, bool bShared)
	{
		auto& attrMap = (bShared || m_bForceUseSharedRes) ? m_SharedResInfo.m_AttrHash : m_ResInfo.m_AttrHash;
		attrMap[std::wstring(controlName)] = controlAttrList;
	}

	std::wstring_view CPaintManagerUI::GetDefaultAttributeList(std::wstring_view controlName) const
	{
		const auto it = m_ResInfo.m_AttrHash.find(controlName);
		if (it != m_ResInfo.m_AttrHash.end()) return it->second;
		const auto sharedIt = m_SharedResInfo.m_AttrHash.find(controlName);
		if (sharedIt != m_SharedResInfo.m_AttrHash.end()) return sharedIt->second;
		return std::wstring_view();
	}

	bool CPaintManagerUI::RemoveDefaultAttributeList(std::wstring_view controlName, bool bShared)
	{
		if (controlName.empty()) {
			return false;
		}

		auto& attrMap = bShared ? m_SharedResInfo.m_AttrHash : m_ResInfo.m_AttrHash;
		const auto it = attrMap.find(controlName);
		if (it == attrMap.end()) return false;
		attrMap.erase(it);
		return true;
	}

	void CPaintManagerUI::RemoveAllDefaultAttributeList(bool bShared)
	{
		auto& attrMap = bShared ? m_SharedResInfo.m_AttrHash : m_ResInfo.m_AttrHash;
		attrMap.clear();
	}

	void CPaintManagerUI::AddWindowCustomAttribute(std::wstring_view name, std::wstring_view attr)
	{
		if (name.empty() || attr.empty()) return;
		m_mWindowCustomAttrHash.try_emplace(std::wstring(name), attr);
	}

	std::wstring_view CPaintManagerUI::GetWindowCustomAttribute(std::wstring_view name) const
	{
		if (name.empty()) return std::wstring_view();
		const auto it = m_mWindowCustomAttrHash.find(name);
		if (it != m_mWindowCustomAttrHash.end()) return it->second;
		return std::wstring_view();
	}

	bool CPaintManagerUI::RemoveWindowCustomAttribute(std::wstring_view name)
	{
		if (name.empty()) return false;
		const auto it = m_mWindowCustomAttrHash.find(name);
		if (it == m_mWindowCustomAttrHash.end()) return false;
		m_mWindowCustomAttrHash.erase(it);
		return true;
	}

	void CPaintManagerUI::RemoveAllWindowCustomAttribute()
	{
		m_mWindowCustomAttrHash.clear();
	}

	CControlUI* CPaintManagerUI::GetRoot() const
	{
		if (m_pRoot == nullptr)
			return nullptr;

		return m_pRoot;
	}

	CControlUI* CPaintManagerUI::FindControl(POINT pt) const
	{

		return m_pRoot->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}

	CControlUI* CPaintManagerUI::FindControl(std::wstring_view name) const
	{
		if (m_pRoot == nullptr)
			return nullptr;
		if (name.empty())
			return nullptr;
		auto it = m_mapName.find(std::wstring(name));
		if (it != m_mapName.end())
			return it->second;
		return nullptr;
	}

	CControlUI* CPaintManagerUI::FindSubControlByPoint(CControlUI* pParent, POINT pt) const
	{
		if (pParent == NULL) pParent = GetRoot();

		return pParent->FindControl(__FindControlFromPoint, &pt, UIFIND_VISIBLE | UIFIND_HITTEST | UIFIND_TOP_FIRST);
	}

	CControlUI* CPaintManagerUI::FindSubControlByName(CControlUI* pParent, std::wstring_view name) const
	{
		if (pParent == NULL) pParent = GetRoot();
		if (pParent == nullptr || name.empty()) return nullptr;

		TStringMatchViewData matchData{ name };
		return pParent->FindControl(__FindControlFromName, &matchData, UIFIND_ALL);
	}

	CControlUI* CPaintManagerUI::FindSubControlByClass(CControlUI* pParent, std::wstring_view className, int iIndex)
	{
		if (pParent == NULL) pParent = GetRoot();
		if (pParent == nullptr || className.empty()) return nullptr;

		m_aFoundControls.Resize(iIndex + 1);
		TStringMatchViewData matchData{ className };
		return pParent->FindControl(__FindControlFromClass, &matchData, UIFIND_ALL);
	}

	CStdPtrArray* CPaintManagerUI::FindSubControlsByClass(CControlUI* pParent, std::wstring_view className)
	{
		if (pParent == NULL) pParent = GetRoot();

		if (pParent == nullptr || className.empty())
			return nullptr;
		m_aFoundControls.Empty();
		TStringMatchViewData matchData{ className };
		pParent->FindControl(__FindControlsFromClass, &matchData, UIFIND_ALL);
		return &m_aFoundControls;
	}

	CStdPtrArray* CPaintManagerUI::GetFoundControls()
	{
		return &m_aFoundControls;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromNameHash(CControlUI* pThis, LPVOID pData)
	{
		CPaintManagerUI* pManager = static_cast<CPaintManagerUI*>(pData);
		const std::wstring& sName = pThis->GetName();
		if (sName.empty()) return NULL;
		// Add this control to the hash list
		pManager->m_mapName[sName.c_str()] = pThis;
		//pManager->m_mNameHash.Set(sName, pThis);
		return NULL; // Attempt to add all controls
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromCount(CControlUI* /*pThis*/, LPVOID pData)
	{
		int* pnCount = static_cast<int*>(pData);
		(*pnCount)++;
		return NULL;  // Count all controls
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromPoint(CControlUI* pThis, LPVOID pData)
	{
		LPPOINT pPoint = static_cast<LPPOINT>(pData);
		return ::PtInRect(&pThis->GetPos(), *pPoint) ? pThis : NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromTab(CControlUI* pThis, LPVOID pData)
	{
		FINDTABINFO* pInfo = static_cast<FINDTABINFO*>(pData);
		if (pInfo->pFocus == pThis) {
			if (pInfo->bForward) pInfo->bNextIsIt = true;
			return pInfo->bForward ? NULL : pInfo->pLast;
		}
		if ((pThis->GetControlFlags() & UIFLAG_TABSTOP) == 0) return NULL;
		pInfo->pLast = pThis;
		if (pInfo->bNextIsIt) return pThis;
		if (pInfo->pFocus == NULL) return pThis;
		return NULL;  // Examine all controls
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromShortcut(CControlUI* pThis, LPVOID pData)
	{
		if (!pThis->IsVisible()) return NULL;
		FINDSHORTCUT* pFS = static_cast<FINDSHORTCUT*>(pData);
		if (pFS->ch == toupper(pThis->GetShortcut())) pFS->bPickNext = true;
		if (StringUtil::Find(pThis->GetClass(), _T("LabelUI")) != -1) return NULL;   // Labels never get focus!
		return pFS->bPickNext ? pThis : NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromName(CControlUI* pThis, LPVOID pData)
	{
		const TStringMatchViewData* matchData = static_cast<const TStringMatchViewData*>(pData);
		if (matchData == nullptr || matchData->value.empty()) return NULL;
		const std::wstring_view sName = pThis->GetNameView();
		if (sName.empty()) return NULL;
		return EqualsInsensitive(sName, matchData->value) ? pThis : NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlFromClass(CControlUI* pThis, LPVOID pData)
	{
		const TStringMatchViewData* matchData = static_cast<const TStringMatchViewData*>(pData);
		if (matchData == nullptr || matchData->value.empty()) return NULL;
		const std::wstring_view pstrType = matchData->value;
		const std::wstring_view pType = pThis->GetClass();
		CStdPtrArray* pFoundControls = pThis->GetManager()->GetFoundControls();
		if (pstrType == L"*" || pstrType == pType) {
			int iIndex = -1;
			while (pFoundControls->GetAt(++iIndex) != NULL);
			if (iIndex < pFoundControls->GetSize()) pFoundControls->SetAt(iIndex, pThis);
		}
		if (pFoundControls->GetAt(pFoundControls->GetSize() - 1) != NULL) return pThis;
		return NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlsFromClass(CControlUI* pThis, LPVOID pData)
	{
		const TStringMatchViewData* matchData = static_cast<const TStringMatchViewData*>(pData);
		if (matchData == nullptr || matchData->value.empty()) return NULL;
		const std::wstring_view pstrType = matchData->value;
		const std::wstring_view pType = pThis->GetClass();
		if (pstrType == L"*" || pstrType == pType)
			pThis->GetManager()->GetFoundControls()->Add((LPVOID)pThis);
		return NULL;
	}

	CControlUI* CALLBACK CPaintManagerUI::__FindControlsFromUpdate(CControlUI* pThis, LPVOID pData)
	{
		if (pThis->IsUpdateNeeded()) {
			pThis->GetManager()->GetFoundControls()->Add((LPVOID)pThis);
			return pThis;
		}
		return NULL;
	}

	bool CPaintManagerUI::TranslateAccelerator(LPMSG pMsg)
	{
		for (int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
		{
			LRESULT lResult = static_cast<ITranslateAccelerator*>(m_aTranslateAccelerator[i])->TranslateAccelerator(pMsg);
			if (lResult == S_OK) return true;
		}
		return false;
	}

	bool CPaintManagerUI::TranslateMessage(const LPMSG pMsg)
	{
		// Pretranslate Message takes care of system-wide messages, such as
		// tabbing and shortcut key-combos. We'll look for all messages for
		// each window and any child control attached.
		UINT uStyle = GetWindowStyle(pMsg->hwnd);
		UINT uChildRes = uStyle & WS_CHILD;
		LRESULT lRes = 0;
		if (uChildRes != 0)
		{
			HWND hWndParent = ::GetParent(pMsg->hwnd);

			for (int i = 0; i < m_aPreMessages.GetSize(); i++)
			{
				CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
				HWND hTempParent = hWndParent;
				while (hTempParent)
				{
					if (pMsg->hwnd == pT->GetPaintWindow() || hTempParent == pT->GetPaintWindow())
					{
						if (pT->TranslateAccelerator(pMsg))
							return true;

						pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes);
					}
					hTempParent = GetParent(hTempParent);
				}
			}
		}
		else
		{
			for (int i = 0; i < m_aPreMessages.GetSize(); i++)
			{
				CPaintManagerUI* pT = static_cast<CPaintManagerUI*>(m_aPreMessages[i]);
				if (pMsg->hwnd == pT->GetPaintWindow())
				{
					if (pT->TranslateAccelerator(pMsg))
						return true;

					if (pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes))
						return true;

					return false;
				}
			}
		}
		return false;
	}

	bool CPaintManagerUI::AddTranslateAccelerator(ITranslateAccelerator* pTranslateAccelerator)
	{
		return m_aTranslateAccelerator.Add(pTranslateAccelerator);
	}

	bool CPaintManagerUI::RemoveTranslateAccelerator(ITranslateAccelerator* pTranslateAccelerator)
	{
		for (int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
		{
			if (static_cast<ITranslateAccelerator*>(m_aTranslateAccelerator[i]) == pTranslateAccelerator)
			{
				return m_aTranslateAccelerator.Remove(i);
			}
		}
		return false;
	}

	void CPaintManagerUI::UsedVirtualWnd(bool bUsed)
	{
		m_bUsedVirtualWnd = bUsed;
	}

	// 闁哄秴鍢茬槐锛勭不閿涘嫭鍊?	void CPaintManagerUI::AddStyle(std::wstring_view name, std::wstring_view declarationList, bool bShared)
	{
		if (bShared || m_bForceUseSharedRes) {
			m_SharedResInfo.m_StyleHash.try_emplace(std::wstring(name), declarationList);
		}
		else
		{
			m_ResInfo.m_StyleHash.try_emplace(std::wstring(name), declarationList);
		}
	}

	std::wstring_view CPaintManagerUI::GetStyle(std::wstring_view name) const
	{
		const auto it = m_ResInfo.m_StyleHash.find(name);
		if (it != m_ResInfo.m_StyleHash.end()) return it->second;
		const auto sharedIt = m_SharedResInfo.m_StyleHash.find(name);
		if (sharedIt != m_SharedResInfo.m_StyleHash.end()) return sharedIt->second;
		return std::wstring_view();
	}

	BOOL CPaintManagerUI::RemoveStyle(std::wstring_view name, bool bShared)
	{
		if (name.empty()) {
			return FALSE;
		}

		auto& styleMap = bShared ? m_SharedResInfo.m_StyleHash : m_ResInfo.m_StyleHash;
		const auto it = styleMap.find(name);
		if (it != styleMap.end()) {
			styleMap.erase(it);
		}
		return true;
	}

	const std::map<std::wstring, std::wstring, std::less<>>& CPaintManagerUI::GetStyles(bool bShared) const
	{
		if (bShared) return m_SharedResInfo.m_StyleHash;
		else return m_ResInfo.m_StyleHash;
	}

	void CPaintManagerUI::RemoveAllStyle(bool bShared)
	{
		auto& styleMap = bShared ? m_SharedResInfo.m_StyleHash : m_ResInfo.m_StyleHash;
		styleMap.clear();
	}

	const TImageInfo* CPaintManagerUI::GetImageString(std::wstring_view image, std::wstring_view modify)
	{
		std::wstring imageName(image);
		std::wstring imageResType;
		DWORD mask = 0;

		auto applyImageAttributes = [&](std::wstring_view source) {
			if (source.empty()) {
				return;
			}

			const wchar_t* cursor = source.data();
			wchar_t* parseEnd = nullptr;
			std::wstring item;
			std::wstring value;

			while (*cursor != L'\0') {
				item.clear();
				value.clear();

				while (*cursor > L'\0' && *cursor <= L' ') {
					cursor = ::CharNextW(cursor);
				}

				while (*cursor != L'\0' && *cursor != L'=' && *cursor > L' ') {
					const wchar_t* next = ::CharNextW(cursor);
					while (cursor < next) {
						item.push_back(*cursor++);
					}
				}

				while (*cursor > L'\0' && *cursor <= L' ') {
					cursor = ::CharNextW(cursor);
				}
				if (*cursor++ != L'=') {
					break;
				}

				while (*cursor > L'\0' && *cursor <= L' ') {
					cursor = ::CharNextW(cursor);
				}
				if (*cursor++ != L'\'') {
					break;
				}

				while (*cursor != L'\0' && *cursor != L'\'') {
					const wchar_t* next = ::CharNextW(cursor);
					while (cursor < next) {
						value.push_back(*cursor++);
					}
				}
				if (*cursor++ != L'\'') {
					break;
				}

				if (!value.empty()) {
					if (item == L"file" || item == L"res") {
						imageName = value;
					}
					else if (item == L"restype") {
						imageResType = value;
					}
					else if (item == L"mask") {
						const wchar_t* maskSource = value.c_str();
						if (!value.empty() && value.front() == L'#') {
							maskSource += 1;
						}
						mask = _tcstoul(maskSource, &parseEnd, 16);
					}
				}

				if (*cursor++ != L' ') {
					break;
				}
			}
		};

		applyImageAttributes(image);
		applyImageAttributes(modify);
		return GetImageEx(imageName, imageResType, mask);
	}

	bool CPaintManagerUI::EnableDragDrop(bool bEnable)
	{
		if (m_bDragDrop == bEnable) {
			return false;
		}

		m_bDragDrop = bEnable;
		if (bEnable) {
			AddRef();

			if (FAILED(RegisterDragDrop(m_hWndPaint, this))) {
				return false;
			}

			SetTargetWnd(m_hWndPaint);

			FORMATETC ftetc = {};
			ftetc.dwAspect = DVASPECT_CONTENT;
			ftetc.lindex = -1;

			ftetc.cfFormat = CF_BITMAP;
			ftetc.tymed = TYMED_GDI;
			AddSuportedFormat(ftetc);

			ftetc.cfFormat = CF_DIB;
			ftetc.tymed = TYMED_HGLOBAL;
			AddSuportedFormat(ftetc);

			ftetc.cfFormat = CF_HDROP;
			ftetc.tymed = TYMED_HGLOBAL;
			AddSuportedFormat(ftetc);

			ftetc.cfFormat = CF_ENHMETAFILE;
			ftetc.tymed = TYMED_ENHMF;
			AddSuportedFormat(ftetc);
			return true;
		}

		Release();
		if (FAILED(RevokeDragDrop(m_hWndPaint))) {
			return false;
		}
		return true;
	}

	void CPaintManagerUI::SetDragDrop(IDragDropUI* pDragDrop)
	{
		m_pDragDrop = pDragDrop;
	}

	static WORD DIBNumColors(void* pv)
	{
		const LPBITMAPINFOHEADER lpbi = static_cast<LPBITMAPINFOHEADER>(pv);
		const LPBITMAPCOREHEADER lpbc = static_cast<LPBITMAPCOREHEADER>(pv);
		int bits = 0;

		if (lpbi->biSize != sizeof(BITMAPCOREHEADER)) {
			if (lpbi->biClrUsed != 0) {
				return static_cast<WORD>(lpbi->biClrUsed);
			}
			bits = lpbi->biBitCount;
		}
		else {
			bits = lpbc->bcBitCount;
		}

		switch (bits) {
		case 1:
			return 2;
		case 4:
			return 16;
		case 8:
			return 256;
		default:
			return 0;
		}
	}

	static WORD ColorTableSize(LPVOID lpv)
	{
		const LPBITMAPINFOHEADER lpbih = static_cast<LPBITMAPINFOHEADER>(lpv);
		if (lpbih->biSize != sizeof(BITMAPCOREHEADER)) {
			if (lpbih->biCompression == BI_BITFIELDS) {
				return static_cast<WORD>((sizeof(DWORD) * 3) + (DIBNumColors(lpbih) * sizeof(RGBQUAD)));
			}
			return static_cast<WORD>(DIBNumColors(lpbih) * sizeof(RGBQUAD));
		}

		return static_cast<WORD>(DIBNumColors(lpbih) * sizeof(RGBTRIPLE));
	}

	bool CPaintManagerUI::OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium, DWORD* pdwEffect)
	{
		(void)pdwEffect;

		POINT ptMouse = {};
		::GetCursorPos(&ptMouse);
		::SendMessage(m_hTargetWnd, WM_LBUTTONUP, 0, MAKELPARAM(ptMouse.x, ptMouse.y));

		if (pFmtEtc->cfFormat == CF_DIB && medium.tymed == TYMED_HGLOBAL && medium.hGlobal != NULL) {
			LPBITMAPINFOHEADER lpbi = static_cast<LPBITMAPINFOHEADER>(::GlobalLock(medium.hGlobal));
			if (lpbi != NULL) {
				HBITMAP hbm = NULL;
				HDC hdc = ::GetDC(NULL);
				if (hdc != NULL) {
					hbm = ::CreateDIBitmap(
						hdc,
						lpbi,
						CBM_INIT,
						reinterpret_cast<LPBYTE>(lpbi) + lpbi->biSize + ColorTableSize(lpbi),
						reinterpret_cast<LPBITMAPINFO>(lpbi),
						DIB_RGB_COLORS);
					::ReleaseDC(NULL, hdc);
				}

				::GlobalUnlock(medium.hGlobal);
				if (hbm != NULL) {
					hbm = reinterpret_cast<HBITMAP>(::SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hbm)));
				}
				if (hbm != NULL) {
					::DeleteObject(hbm);
				}
				return true;
			}
		}

		if (pFmtEtc->cfFormat == CF_BITMAP && medium.tymed == TYMED_GDI && medium.hBitmap != NULL) {
			HBITMAP hBmp = reinterpret_cast<HBITMAP>(::SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(medium.hBitmap)));
			if (hBmp != NULL) {
				CRenderEngine::FreeBitmap(hBmp);
			}
			return false;
		}

		if (pFmtEtc->cfFormat == CF_ENHMETAFILE && medium.tymed == TYMED_ENHMF) {
			ENHMETAHEADER emh = {};
			::GetEnhMetaFileHeader(medium.hEnhMetaFile, sizeof(ENHMETAHEADER), &emh);

			RECT rc = {};
			HDC hDC = ::GetDC(m_hTargetWnd);
			const float pixelsX = static_cast<float>(::GetDeviceCaps(hDC, HORZRES));
			const float pixelsY = static_cast<float>(::GetDeviceCaps(hDC, VERTRES));
			const float mmx = static_cast<float>(::GetDeviceCaps(hDC, HORZSIZE));
			const float mmy = static_cast<float>(::GetDeviceCaps(hDC, VERTSIZE));

			rc.top = static_cast<int>(static_cast<float>(emh.rclFrame.top) * pixelsY / (mmy * 100.0f));
			rc.left = static_cast<int>(static_cast<float>(emh.rclFrame.left) * pixelsX / (mmx * 100.0f));
			rc.right = static_cast<int>(static_cast<float>(emh.rclFrame.right) * pixelsX / (mmx * 100.0f));
			rc.bottom = static_cast<int>(static_cast<float>(emh.rclFrame.bottom) * pixelsY / (mmy * 100.0f));

			HDC hdcMem = ::CreateCompatibleDC(hDC);
			HGDIOBJ hBmpMem = ::CreateCompatibleBitmap(hDC, emh.rclBounds.right, emh.rclBounds.bottom);
			HGDIOBJ hOldBmp = ::SelectObject(hdcMem, hBmpMem);
			::PlayEnhMetaFile(hdcMem, medium.hEnhMetaFile, &rc);
			HBITMAP hBmp = reinterpret_cast<HBITMAP>(::SelectObject(hdcMem, hOldBmp));
			::DeleteDC(hdcMem);
			::ReleaseDC(m_hTargetWnd, hDC);

			hBmp = reinterpret_cast<HBITMAP>(::SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBmp)));
			if (hBmp != NULL) {
				CRenderEngine::FreeBitmap(hBmp);
			}
			return true;
		}

		if (pFmtEtc->cfFormat == CF_HDROP && medium.tymed == TYMED_HGLOBAL) {
			HDROP hDrop = static_cast<HDROP>(::GlobalLock(medium.hGlobal));
			if (hDrop != NULL) {
				TCHAR szFileName[MAX_PATH] = {};
				const UINT cFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
				if (cFiles > 0) {
					::DragQueryFile(hDrop, 0, szFileName, static_cast<UINT>(std::size(szFileName)));
					HBITMAP hBitmap = static_cast<HBITMAP>(::LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE));
					if (hBitmap != NULL) {
						HBITMAP hBmp = reinterpret_cast<HBITMAP>(::SendMessage(m_hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBitmap)));
						if (hBmp != NULL) {
							CRenderEngine::FreeBitmap(hBmp);
						}
					}
				}
				::GlobalUnlock(medium.hGlobal);
			}
		}

		return true;
	}

	void CPaintManagerUI::SetEventControl(CControlUI* pControl)
	{
		m_pEventClick = pControl;
	}
} // namespace FYUI

#pragma warning(pop)


} // namespace FYUI

#pragma warning(pop)
