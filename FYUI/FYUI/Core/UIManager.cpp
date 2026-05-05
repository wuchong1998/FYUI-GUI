#include "pch.h"
#include "UIManager.h"
#include "UIContainer.h"
#include "Render/UIRenderBatchInternal.h"
#include "Render/UIRenderContext.h"
#include "Render/UIRenderImageRuntimeInternal.h"
#include "Render/UIRenderSurfaceInternal.h"
#include "Render/UIRenderTextSharedInternal.h"
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

	// UIManager.cpp HDC boundary whitelist:
	// - CreateManagerNativeRenderContext bridges native paint/print DCs into CPaintRenderContext.
	// - CScopedWindowPaint owns BeginPaint/EndPaint.
	// - CScopedRenderContextNativeDCSave owns SaveDC/RestoreDC around context painting.
	static HDC GetRenderContextNativeDC(const CPaintRenderContext& renderContext)
	{
		return renderContext.GetDC();
	}

	static CPaintRenderContext CreateManagerNativeRenderContext(CPaintManagerUI* pManager, HDC hNativeDC, const RECT& rcPaint)
	{
		RenderBackendType activeBackend = RenderBackendAuto;
		Direct2DRenderMode activeDirect2DMode = Direct2DRenderModeAuto;
		if (pManager != NULL) {
			if (const CPaintRenderContext* pCurrentRenderContext = pManager->GetCurrentRenderContext()) {
				if (GetRenderContextNativeDC(*pCurrentRenderContext) == hNativeDC) {
					return CPaintRenderContext(
						pManager,
						hNativeDC,
						pCurrentRenderContext->GetPaintRect(),
						pCurrentRenderContext->GetActiveBackend(),
						pCurrentRenderContext->GetActiveDirect2DRenderMode());
				}
			}
			activeBackend = pManager->GetActiveRenderBackend();
			activeDirect2DMode = pManager->GetActiveDirect2DRenderMode();
		}
		return CPaintRenderContext(pManager, hNativeDC, rcPaint, activeBackend, activeDirect2DMode);
	}

	static CPaintRenderContext CreatePrintClientNativeRenderContext(CPaintManagerUI* pManager, WPARAM paintNativeDC, const RECT& rcPaint)
	{
		return CreateManagerNativeRenderContext(pManager, reinterpret_cast<HDC>(paintNativeDC), rcPaint);
	}

	static CPaintRenderContext CreateDerivedNativeRenderContext(CPaintManagerUI* pManager, const CPaintRenderContext& sourceContext, const RECT& rcPaint)
	{
		return CPaintRenderContext(
			pManager,
			GetRenderContextNativeDC(sourceContext),
			rcPaint,
			sourceContext.GetActiveBackend(),
			sourceContext.GetActiveDirect2DRenderMode());
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

	static void CopyLogFontFaceName(LOGFONT& lf, std::wstring_view fontName)
	{
		lf.lfFaceName[0] = L'\0';
		const size_t count = (std::min)(fontName.size(), static_cast<size_t>(LF_FACESIZE - 1));
		for (size_t i = 0; i < count; ++i) {
			lf.lfFaceName[i] = fontName[i];
		}
		lf.lfFaceName[count] = L'\0';
	}

	static bool TryParseNonNegativeInt(std::wstring_view text, int& value)
	{
		return StringUtil::TryParseInt(text, value) && value >= 0;
	}

	static void ClearFontDescriptor(TFontInfo& fontInfo)
	{
		fontInfo.sFontName.clear();
		fontInfo.iSize = 0;
		fontInfo.bBold = false;
		fontInfo.bUnderline = false;
		fontInfo.bItalic = false;
		fontInfo.bStrikeout = false;
		::ZeroMemory(&fontInfo.tm, sizeof(fontInfo.tm));
	}

	static void ReleaseFontHandle(TFontInfo& fontInfo, bool clearDescriptor)
	{
		if (fontInfo.hFont != NULL) {
			::DeleteObject(fontInfo.hFont);
			fontInfo.hFont = NULL;
		}
		if (clearDescriptor) {
			ClearFontDescriptor(fontInfo);
		}
		else {
			::ZeroMemory(&fontInfo.tm, sizeof(fontInfo.tm));
		}
	}

	static void AdoptImagePayload(TImageInfo& target, TImageInfo& source)
	{
		target.pImage = source.pImage;
		target.hBitmap = source.hBitmap;
		target.pBits = source.pBits;
		target.pHandle = source.pHandle;
		target.nX = source.nX;
		target.nY = source.nY;
		target.nDestWidth = source.nDestWidth;
		target.nDestHeight = source.nDestHeight;
		target.nOriWidth = source.nOriWidth;
		target.nOriHeight = source.nOriHeight;
		target.fPresent = source.fPresent;
		target.bAlpha = source.bAlpha;

		source.pImage = nullptr;
		source.hBitmap = nullptr;
		source.pBits = nullptr;
		source.pHandle = nullptr;
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
				m_hPaintNativeDC = ::BeginPaint(m_hWnd, &m_ps);
			}
		}

		~CScopedWindowPaint()
		{
			if (m_hWnd != NULL && m_hPaintNativeDC != NULL) {
				::EndPaint(m_hWnd, &m_ps);
			}
		}

		HDC GetNativeDC() const
		{
			return m_hPaintNativeDC;
		}

		const RECT& GetPaintRect() const
		{
			return m_ps.rcPaint;
		}

		bool IsActive() const
		{
			return m_hPaintNativeDC != NULL;
		}

		CPaintRenderContext CreateRenderContext(CPaintManagerUI* pManager, const RECT& rcPaint) const
		{
			return CreateManagerNativeRenderContext(pManager, m_hPaintNativeDC, rcPaint);
		}

		CPaintRenderContext CreateRenderContext(CPaintManagerUI* pManager) const
		{
			return CreateRenderContext(pManager, m_ps.rcPaint);
		}

	private:
		HWND m_hWnd = NULL;
		HDC m_hPaintNativeDC = NULL;
		PAINTSTRUCT m_ps = {};
	};

	class CScopedWindowDC
	{
	public:
		explicit CScopedWindowDC(HWND hWnd)
			: m_hWnd(hWnd)
			, m_hWindowNativeDC(::GetDC(hWnd))
		{
		}

		~CScopedWindowDC()
		{
			if (m_hWindowNativeDC != NULL) {
				::ReleaseDC(m_hWnd, m_hWindowNativeDC);
			}
		}

		CScopedWindowDC(const CScopedWindowDC&) = delete;
		CScopedWindowDC& operator=(const CScopedWindowDC&) = delete;

		HDC GetNativeDC() const
		{
			return m_hWindowNativeDC;
		}

		bool IsValid() const
		{
			return m_hWindowNativeDC != NULL;
		}

	private:
		HWND m_hWnd = NULL;
		HDC m_hWindowNativeDC = NULL;
	};

	class CScopedCompatibleNativeDC
	{
	public:
		explicit CScopedCompatibleNativeDC(HDC hReferenceNativeDC)
			: m_hMemoryNativeDC(::CreateCompatibleDC(hReferenceNativeDC))
		{
		}

		~CScopedCompatibleNativeDC()
		{
			if (m_hMemoryNativeDC != NULL) {
				::DeleteDC(m_hMemoryNativeDC);
			}
		}

		CScopedCompatibleNativeDC(const CScopedCompatibleNativeDC&) = delete;
		CScopedCompatibleNativeDC& operator=(const CScopedCompatibleNativeDC&) = delete;

		bool IsValid() const
		{
			return m_hMemoryNativeDC != NULL;
		}

		HDC GetNativeDC() const
		{
			return m_hMemoryNativeDC;
		}

	private:
		HDC m_hMemoryNativeDC = NULL;
	};

	class CScopedNativeGdiObject
	{
	public:
		explicit CScopedNativeGdiObject(HGDIOBJ hObject)
			: m_hObject(hObject)
		{
		}

		~CScopedNativeGdiObject()
		{
			if (m_hObject != NULL) {
				::DeleteObject(m_hObject);
			}
		}

		CScopedNativeGdiObject(const CScopedNativeGdiObject&) = delete;
		CScopedNativeGdiObject& operator=(const CScopedNativeGdiObject&) = delete;

		bool IsValid() const
		{
			return m_hObject != NULL;
		}

		HGDIOBJ Get() const
		{
			return m_hObject;
		}

		HGDIOBJ Detach()
		{
			HGDIOBJ hObject = m_hObject;
			m_hObject = NULL;
			return hObject;
		}

	private:
		HGDIOBJ m_hObject = NULL;
	};

	template<typename TObject>
	class CScopedComObjectRef
	{
	public:
		explicit CScopedComObjectRef(TObject* pObject)
			: m_pObject(pObject)
		{
			if (m_pObject != NULL) {
				m_pObject->AddRef();
			}
		}

		~CScopedComObjectRef()
		{
			if (m_pObject != NULL) {
				m_pObject->Release();
			}
		}

		CScopedComObjectRef(const CScopedComObjectRef&) = delete;
		CScopedComObjectRef& operator=(const CScopedComObjectRef&) = delete;

		bool IsValid() const
		{
			return m_pObject != NULL;
		}

		TObject* Get() const
		{
			return m_pObject;
		}

		TObject* operator->() const
		{
			return m_pObject;
		}

	private:
		TObject* m_pObject = NULL;
	};

	class CScopedGlobalLock
	{
	public:
		explicit CScopedGlobalLock(HGLOBAL hGlobal)
			: m_hGlobal(hGlobal)
			, m_pData(hGlobal != NULL ? ::GlobalLock(hGlobal) : NULL)
		{
		}

		~CScopedGlobalLock()
		{
			if (m_pData != NULL) {
				::GlobalUnlock(m_hGlobal);
			}
		}

		CScopedGlobalLock(const CScopedGlobalLock&) = delete;
		CScopedGlobalLock& operator=(const CScopedGlobalLock&) = delete;

		bool IsValid() const
		{
			return m_pData != NULL;
		}

		void* Get() const
		{
			return m_pData;
		}

		template<typename T>
		T* As() const
		{
			return static_cast<T*>(m_pData);
		}

	private:
		HGLOBAL m_hGlobal = NULL;
		void* m_pData = NULL;
	};

	class CScopedEnhMetaFileNativeDC
	{
	public:
		explicit CScopedEnhMetaFileNativeDC(HDC hReferenceNativeDC)
			: m_hMetaNativeDC(::CreateEnhMetaFile(hReferenceNativeDC, NULL, NULL, NULL))
		{
		}

		~CScopedEnhMetaFileNativeDC()
		{
			HENHMETAFILE hEnhMetaFile = CloseAndDetach();
			if (hEnhMetaFile != NULL) {
				::DeleteEnhMetaFile(hEnhMetaFile);
			}
		}

		CScopedEnhMetaFileNativeDC(const CScopedEnhMetaFileNativeDC&) = delete;
		CScopedEnhMetaFileNativeDC& operator=(const CScopedEnhMetaFileNativeDC&) = delete;

		bool IsValid() const
		{
			return m_hMetaNativeDC != NULL;
		}

		HDC GetNativeDC() const
		{
			return m_hMetaNativeDC;
		}

		HENHMETAFILE CloseAndDetach()
		{
			if (m_hMetaNativeDC == NULL) {
				return NULL;
			}

			HENHMETAFILE hEnhMetaFile = ::CloseEnhMetaFile(m_hMetaNativeDC);
			m_hMetaNativeDC = NULL;
			return hEnhMetaFile;
		}

	private:
		HDC m_hMetaNativeDC = NULL;
	};

	class CScopedSelectedNativeObject
	{
	public:
		CScopedSelectedNativeObject(HDC hNativeDC, HGDIOBJ hObject)
			: m_hNativeDC(hNativeDC)
		{
			if (m_hNativeDC != NULL && hObject != NULL) {
				m_hPreviousObject = ::SelectObject(m_hNativeDC, hObject);
			}
		}

		~CScopedSelectedNativeObject()
		{
			Restore();
		}

		CScopedSelectedNativeObject(const CScopedSelectedNativeObject&) = delete;
		CScopedSelectedNativeObject& operator=(const CScopedSelectedNativeObject&) = delete;

		bool IsValid() const
		{
			return m_hNativeDC != NULL && m_hPreviousObject != NULL && m_hPreviousObject != HGDI_ERROR;
		}

		HGDIOBJ Restore()
		{
			if (!IsValid()) {
				return NULL;
			}

			HGDIOBJ hSelectedObject = ::SelectObject(m_hNativeDC, m_hPreviousObject);
			m_hPreviousObject = NULL;
			return hSelectedObject;
		}

	private:
		HDC m_hNativeDC = NULL;
		HGDIOBJ m_hPreviousObject = NULL;
	};

	void ReplaceNativeStaticBitmapImage(HWND hTargetWnd, HBITMAP hBitmap)
	{
		if (hTargetWnd == NULL || hBitmap == NULL) {
			return;
		}

		// The static control takes ownership of the new bitmap; only the
		// returned previous bitmap belongs to us and must be released here.
		HBITMAP hPreviousBitmap = reinterpret_cast<HBITMAP>(::SendMessage(hTargetWnd, STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBitmap)));
		if (hPreviousBitmap != NULL) {
			CRenderEngine::FreeBitmap(hPreviousBitmap);
		}
	}

	class CScopedRenderContextNativeDCSave
	{
	public:
		explicit CScopedRenderContextNativeDCSave(CPaintRenderContext& renderContext)
			: m_hNativeDC(GetRenderContextNativeDC(renderContext))
		{
			if (m_hNativeDC != NULL) {
				m_iSaveDC = ::SaveDC(m_hNativeDC);
			}
		}

		~CScopedRenderContextNativeDCSave()
		{
			Restore();
		}

		CScopedRenderContextNativeDCSave(const CScopedRenderContextNativeDCSave&) = delete;
		CScopedRenderContextNativeDCSave& operator=(const CScopedRenderContextNativeDCSave&) = delete;

		bool IsValid() const
		{
			return m_hNativeDC != NULL && m_iSaveDC != 0;
		}

		HDC GetNativeDC() const
		{
			return m_hNativeDC;
		}

		void Restore()
		{
			if (m_hNativeDC != NULL && m_iSaveDC != 0) {
				::RestoreDC(m_hNativeDC, m_iSaveDC);
				m_iSaveDC = 0;
			}
		}

	private:
		HDC m_hNativeDC = NULL;
		int m_iSaveDC = 0;
	};

	template <typename TRenderBody>
	static bool ExecuteBatchedPaintScope(CPaintRenderContext& renderContext, TRenderBody&& renderBody)
	{
		CScopedDirect2DBatchInternal batchScope(renderContext);
		return renderBody();
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

	static DWORD* GetBottomUpPixelRow(BYTE* pBits, LONG width, LONG height, LONG y)
	{
		if (pBits == nullptr || width <= 0 || height <= 0 || y < 0 || y >= height) {
			return nullptr;
		}

		DWORD* pPixels = reinterpret_cast<DWORD*>(pBits);
		return pPixels + static_cast<size_t>(height - 1 - y) * static_cast<size_t>(width);
	}

	static bool ScrollBottomUpBitsRect(BYTE* pBits, LONG width, LONG height, const RECT& rcScroll, int dx, int dy)
	{
		if (pBits == nullptr || width <= 0 || height <= 0 || (dx == 0 && dy == 0)) {
			return false;
		}

		RECT rcCopy = rcScroll;
		::OffsetRect(&rcCopy, dx, dy);
		if (!::IntersectRect(&rcCopy, &rcCopy, &rcScroll)) {
			return false;
		}

		const LONG copyWidth = rcCopy.right - rcCopy.left;
		if (copyWidth <= 0 || rcCopy.bottom <= rcCopy.top) {
			return false;
		}

		const size_t copyBytes = static_cast<size_t>(copyWidth) * sizeof(DWORD);
		const LONG startY = dy > 0 ? rcCopy.bottom - 1 : rcCopy.top;
		const LONG endY = dy > 0 ? rcCopy.top - 1 : rcCopy.bottom;
		const LONG stepY = dy > 0 ? -1 : 1;
		for (LONG y = startY; y != endY; y += stepY) {
			DWORD* pDstRow = GetBottomUpPixelRow(pBits, width, height, y);
			DWORD* pSrcRow = GetBottomUpPixelRow(pBits, width, height, y - dy);
			if (pDstRow == nullptr || pSrcRow == nullptr) {
				continue;
			}
			::MoveMemory(
				pDstRow + rcCopy.left,
				pSrcRow + (rcCopy.left - dx),
				copyBytes);
		}

		return true;
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
		wchar_t ch;
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
		// 1闂備線娼уΛ鏂款啅椤ь湩.jpg
		// 2闂備線娼уΛ鏃堝礂閻炵灊e='aaa.jpg' res='' restype='0' dest='0,0,0,0' source='0,0,0,0' corner='0,0,0,0' 
		// mask='#FF0000' fade='255' hole='false' xtiled='false' ytiled='false'
		sDrawString = pStrImageView;
		sDrawModify = pStrModifyView;
		sImageName = pStrImageView;

		for (std::wstring_view source : { pStrImageView, pStrModifyView }) {
			if (source.empty()) {
				continue;
			}

			for (const auto& attribute : StringUtil::ParseQuotedAttributes(source, L'\'')) {
				if (attribute.value.empty()) {
					continue;
				}

				if (StringUtil::EqualsNoCase(attribute.key, L"file") || StringUtil::EqualsNoCase(attribute.key, L"res")) {
					sImageName = attribute.value;
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"restype")) {
					sResType = attribute.value;
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"dest")) {
					if (StringUtil::TryParseRect(attribute.value, rcDest) && pManager != NULL) {
						pManager->ScaleRect(&rcDest);
					}
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"source")) {
					if (StringUtil::TryParseRect(attribute.value, rcSource) && pManager != NULL) {
						pManager->ScaleRect(&rcSource);
					}
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"corner")) {
					if (StringUtil::TryParseRect(attribute.value, rcCorner) && pManager != NULL) {
						pManager->ScaleRect(&rcCorner);
					}
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"mask")) {
					StringUtil::TryParseColor(attribute.value, dwMask);
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"fade")) {
					int fade = static_cast<int>(uFade);
					if (StringUtil::TryParseInt(attribute.value, fade) && fade >= 0) {
						uFade = static_cast<UINT>(fade);
					}
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"rotate")) {
					int rotate = static_cast<int>(uRotate);
					if (StringUtil::TryParseInt(attribute.value, rotate) && rotate >= 0) {
						uRotate = static_cast<UINT>(rotate);
						bGdiplus = true;
					}
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"gdiplus")) {
					bGdiplus = StringUtil::ParseBool(attribute.value);
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"hole")) {
					bHole = StringUtil::ParseBool(attribute.value);
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"xtiled")) {
					bTiledX = StringUtil::ParseBool(attribute.value);
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"ytiled")) {
					bTiledY = StringUtil::ParseBool(attribute.value);
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"hsl")) {
					bHSL = StringUtil::ParseBool(attribute.value);
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"size")) {
					StringUtil::TryParseSize(attribute.value, szImage);
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"align")) {
					sAlign = attribute.value;
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"padding")) {
					StringUtil::TryParseRect(attribute.value, rcPadding);
				}
			}
		}

        // Handle DPI-specific raster asset suffixes for non-SVG resources.
        if (pManager != NULL && pManager->GetScale() != 100) {
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


	constexpr DWORD kUpdateRectDebugColor = 0xFFDC0000;

	HINSTANCE CPaintManagerUI::m_hResourceInstance = NULL;
	std::wstring CPaintManagerUI::m_pStrResourcePath;
	std::wstring CPaintManagerUI::m_pStrResourceZip;
	std::wstring CPaintManagerUI::m_pStrResourceZipPwd;  //Garfield 20160325 闂佹眹鍩勯崹鐣岀不閹达附鍎婇柟杈鹃檮閸庢ê霉濠婂啫鍔畃闂備礁鎲￠悧鏇㈠箠閹捐鏆伴梺?
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
	static bool g_bCommonControlsInitialized = false;
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

		bool HasNativeRenderContext(const CPaintRenderContext& renderContext)
		{
			return GetRenderContextNativeDC(renderContext) != NULL;
		}

		struct TNativeWindowBitmapCache
		{
			HWND hWnd = NULL;
			CPaintRenderSurface surface;
			bool bHasContent = false;
		};

		bool CanUseNativeWindowBitmapRenderContext(const CPaintRenderContext& renderContext)
		{
			return renderContext.GetManager() != NULL && HasNativeRenderContext(renderContext);
		}

		void ReleaseNativeWindowBitmapCacheBitmap(TNativeWindowBitmapCache& cache)
		{
			cache.surface.Reset();
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

		bool EnsureNativeWindowBitmapCacheBitmap(TNativeWindowBitmapCache& cache, CPaintRenderContext& referenceContext, LONG nWidth, LONG nHeight)
		{
			if (nWidth <= 0 || nHeight <= 0 || !CanUseNativeWindowBitmapRenderContext(referenceContext)) {
				ReleaseNativeWindowBitmapCacheBitmap(cache);
				return false;
			}
			const SIZE szBitmap = cache.surface.GetSize();
			if (cache.surface.IsReady() &&
				szBitmap.cx == nWidth && szBitmap.cy == nHeight) {
				return true;
			}

			ReleaseNativeWindowBitmapCacheBitmap(cache);
			if (!cache.surface.Ensure(referenceContext, nWidth, nHeight)) {
				ReleaseNativeWindowBitmapCacheBitmap(cache);
				return false;
			}
			return true;
		}

		bool NeedsRefreshNativeWindowBitmapCache(const TNativeWindowBitmapCache& cache, HWND hWnd, LONG nWidth, LONG nHeight)
		{
			const SIZE szBitmap = cache.surface.GetSize();
			if (!cache.bHasContent || szBitmap.cx != nWidth || szBitmap.cy != nHeight) {
				return true;
			}
			return ::GetUpdateRect(hWnd, NULL, FALSE) != FALSE;
		}

		bool RefreshNativeWindowBitmapCache(CPaintRenderContext& referenceContext, HWND hChildWnd, TNativeWindowBitmapCache& cache, LONG nWidth, LONG nHeight)
		{
			if (!EnsureNativeWindowBitmapCacheBitmap(cache, referenceContext, nWidth, nHeight)) {
				return false;
			}
			COLORREF* pBits = cache.surface.GetColorBits();
			if (pBits != NULL) {
				::ZeroMemory(pBits, static_cast<size_t>(nWidth) * static_cast<size_t>(nHeight) * 4);
			}
			if (!PrintWindowToRenderSurfaceInternal(cache.surface, hChildWnd, static_cast<LPARAM>(PRF_CHECKVISIBLE | PRF_CHILDREN | PRF_CLIENT | PRF_OWNED))) {
				return false;
			}
			NormalizeOpaquePixels(pBits, nWidth, nHeight);
			cache.bHasContent = true;
			::ValidateRect(hChildWnd, NULL);
			return true;
		}

		void DrawNativeWindowBitmapCache(CPaintRenderContext& targetContext, const RECT& rcChildWnd, const TNativeWindowBitmapCache& cache)
		{
			if (!CanUseNativeWindowBitmapRenderContext(targetContext) || cache.surface.GetBitmap() == NULL || !cache.bHasContent) {
				return;
			}

			const RECT rcEmptyCorners = { 0, 0, 0, 0 };
			const SIZE szBitmap = cache.surface.GetSize();
			RECT rcChildBitmap = { 0, 0, szBitmap.cx, szBitmap.cy };
			CRenderEngine::DrawImage(targetContext, cache.surface.GetBitmap(), rcChildWnd, rcChildBitmap, rcEmptyCorners, true, 255);
		}

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
			CPaintRenderContext& renderContext,
			HWND hParentWnd,
			const RECT& rcPaint,
			int nCacheIndex,
			HWND hChildWnd)
		{
			if (!CanUseNativeWindowBitmapRenderContext(renderContext) || hParentWnd == NULL || hChildWnd == NULL || !::IsWindowVisible(hChildWnd)) {
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
				if (!RefreshNativeWindowBitmapCache(renderContext, hChildWnd, *pChildCache, childWidth, childHeight)) {
					return NativeWindowRenderResult::Failed;
				}
			}

			DrawNativeWindowBitmapCache(renderContext, rcChildWnd, *pChildCache);
			return bNeedsRefresh ? NativeWindowRenderResult::CacheRefresh : NativeWindowRenderResult::CacheHit;
		}

	}

	bool CPaintManagerUI::EnsureMeasureSurface()
	{
		if (m_measureSurface.IsReady()) {
			return true;
		}

		HWND hReferenceWnd = ::IsWindow(m_hWndPaint) ? m_hWndPaint : NULL;
		CScopedWindowDC referenceDC(hReferenceWnd);
		if (!referenceDC.IsValid()) {
			return false;
		}

		const RECT rcReference = { 0, 0, 1, 1 };
		CPaintRenderContext referenceContext(this, referenceDC.GetNativeDC(), rcReference, GetActiveRenderBackend(), GetActiveDirect2DRenderMode());
		if (!m_measureSurface.Ensure(referenceContext, 1, 1)) {
			ResetMeasureDC();
			return false;
		}

		return true;
	}

	void CPaintManagerUI::ResetMeasureDC()
	{
		m_measureSurface.Reset();
	}

	void CPaintManagerUI::UpdateFontTextMetricsForManager(TFontInfo* pFontInfo)
	{
		if (!EnsureMeasureSurface()) {
			return;
		}
		const RECT rcMeasure = { 0, 0, 1, 1 };
		CPaintRenderContext measureContext = m_measureSurface.CreateRenderContext(this, rcMeasure, GetActiveRenderBackend(), GetActiveDirect2DRenderMode());
		UpdateTextFallbackFontMetricsInternal(measureContext, pFontInfo);
	}

	bool CPaintManagerUI::EnsureMainRenderSurface(const CPaintRenderContext& referenceContext, LONG cx, LONG cy)
	{
		if (!m_bOffscreenPaint) {
			return true;
		}
		CPaintRenderContext surfaceReferenceContext = referenceContext;
		if (!HasNativeRenderContext(surfaceReferenceContext)) {
			if (!EnsureMeasureSurface()) {
				return false;
			}
			const RECT rcMeasure = { 0, 0, 1, 1 };
			surfaceReferenceContext = m_measureSurface.CreateRenderContext(this, rcMeasure, referenceContext.GetActiveBackend(), referenceContext.GetActiveDirect2DRenderMode());
		}
		return m_offscreenSurface.Ensure(surfaceReferenceContext, cx, cy);
	}

	bool CPaintManagerUI::EnsureLayeredBackgroundSurface(const CPaintRenderContext& referenceContext, LONG cx, LONG cy)
	{
		CPaintRenderContext surfaceReferenceContext = referenceContext;
		if (!HasNativeRenderContext(surfaceReferenceContext)) {
			if (!EnsureMeasureSurface()) {
				return false;
			}
			const RECT rcMeasure = { 0, 0, 1, 1 };
			surfaceReferenceContext = m_measureSurface.CreateRenderContext(this, rcMeasure, referenceContext.GetActiveBackend(), referenceContext.GetActiveDirect2DRenderMode());
		}
		return m_backgroundSurface.Ensure(surfaceReferenceContext, cx, cy);
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
			const bool bFirstLayout = m_bFirstLayout;
			m_bUpdateNeeded = false;
			if (!::IsRectEmpty(&rcClient) && !::IsIconic(m_hWndPaint)) {
				if (bFirstLayout || m_pRoot->IsUpdateNeeded()) {
					ResetRenderSurfaces();
					RECT rcRoot = GetRootLayoutRect(rcClient);
					m_pRoot->SetPos(rcRoot, true);
					bNeedSizeMsg = true;
				}
				else {
					UpdateControlsNeedingLayout();
					bNeedSizeMsg = true;
				}

				if (bFirstLayout) {
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

	bool CPaintManagerUI::ExecutePaintFrame(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame)
	{
		if (m_bOffscreenPaint) {
			return RenderFrameToMainSurface(targetContext, paintFrame)
				&& PresentMainRenderSurface(targetContext, paintFrame);
		}

		return RenderFrameDirectToWindow(targetContext);
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
		if (paintScope.IsActive()) {
			CPaintRenderContext renderContext = paintScope.CreateRenderContext(this);
			CRenderEngine::DrawColor(renderContext, paintScope.GetPaintRect(), 0xFF000000);
		}
		return true;
	}

	bool CPaintManagerUI::PreparePaintFrameRects(TPaintFrameState& paintFrame)
	{
		PrepareRenderClientRect(paintFrame.rcClient);
		if (m_bFirstLayout || m_bUpdateNeeded) {
			::InvalidateRect(m_hWndPaint, nullptr, FALSE);
			paintFrame.rcPaint = paintFrame.rcClient;
			return !::IsRectEmpty(&paintFrame.rcPaint);
		}

		const BOOL hasUpdateRect = ::GetUpdateRect(m_hWndPaint, &paintFrame.rcPaint, FALSE);
		if (hasUpdateRect == FALSE) {
			return false;
		}
		return true;
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

	bool CPaintManagerUI::ExecutePaintWindowFrame(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame) const
	{
		if (!HasNativeRenderContext(targetContext) || ShouldSkipPaintExecution(paintFrame)) {
			return false;
		}
		return const_cast<CPaintManagerUI*>(this)->ExecutePaintFrame(targetContext, paintFrame);
	}

	bool CPaintManagerUI::PaintPreparedWindowFrame(const TPaintFrameState& paintFrame)
	{
		CScopedWindowPaint paintScope(m_hWndPaint);
		CPaintRenderContext targetContext = paintScope.CreateRenderContext(this, paintFrame.rcPaint);
		return ExecutePaintWindowFrame(targetContext, paintFrame);
	}

	bool CPaintManagerUI::ShouldSkipPaintExecution(const TPaintFrameState& paintFrame) const
	{
		return ::IsRectEmpty(&paintFrame.rcPaint) && !m_bLayeredChanged && !m_bUpdateNeeded;
	}

	bool CPaintManagerUI::RenderPrintClientFrame(CPaintRenderContext& targetContext, LPARAM lParam)
	{
		if (m_pRoot == NULL || !HasNativeRenderContext(targetContext)) {
			return false;
		}

		RECT rcClient = { 0 };
		PrepareRenderClientRect(rcClient);
		CPaintRenderContext printContext = CreateDerivedNativeRenderContext(this, targetContext, rcClient);
		if (!RenderFrameDirectToWindow(printContext, false)) {
			return false;
		}
		PrintChildWindows(printContext, lParam);
		return true;
	}

	CPaintRenderContext CPaintManagerUI::CreateMeasureRenderContext(const RECT& rcPaint)
	{
		RenderBackendType activeBackend = GetActiveRenderBackend();
		Direct2DRenderMode activeDirect2DMode = GetActiveDirect2DRenderMode();
		if (const CPaintRenderContext* pCurrentRenderContext = GetCurrentRenderContext()) {
			activeBackend = pCurrentRenderContext->GetActiveBackend();
			activeDirect2DMode = pCurrentRenderContext->GetActiveDirect2DRenderMode();
		}

		if (!EnsureMeasureSurface()) {
			return CPaintRenderContext(this, NULL, rcPaint, activeBackend, activeDirect2DMode);
		}
		return m_measureSurface.CreateRenderContext(this, rcPaint, activeBackend, activeDirect2DMode);
	}

	bool CPaintManagerUI::ExecuteBatchedControlRenderPass(CPaintRenderContext& renderContext, bool bIncludeNativeWindows, bool bIncludePostPaint)
	{
		return ExecuteBatchedPaintScope(renderContext, [&]() -> bool {
			RenderControlTree(renderContext, NULL, false);
			if (bIncludeNativeWindows && !RenderNativeWindowsToSurface(renderContext)) {
				return false;
			}
			if (bIncludePostPaint) {
				RenderPostPaintControls(renderContext);
			}
			return true;
		});
	}

	bool CPaintManagerUI::ExecuteBatchedDrawInfoPass(CPaintRenderContext& renderContext, TDrawInfo* pDrawInfo)
	{
		if (pDrawInfo == NULL) {
			return false;
		}

		const RECT rcPaint = renderContext.GetPaintRect();
		return ExecuteBatchedPaintScope(renderContext, [&]() -> bool {
			CRenderClip clip;
			CRenderClip::GenerateClip(renderContext, rcPaint, clip);
			CRenderEngine::DrawImageInfo(renderContext, rcPaint, pDrawInfo);
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

	bool CPaintManagerUI::RenderNativeWindowsToSurface(CPaintRenderContext& renderContext)
	{
		const RECT rcPaint = renderContext.GetPaintRect();
		if (!m_bLayered || m_aNativeWindow.GetSize() <= 0 || !CanUseNativeWindowBitmapRenderContext(renderContext)) {
			return true;
		}

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
				renderContext,
				m_hWndPaint,
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

		return true;
	}

	bool CPaintManagerUI::RenderLayeredBackgroundToSurface(const CPaintRenderContext& referenceContext, const TPaintFrameState& paintFrame)
	{
		if (!m_diLayered.sDrawString.empty()) {
			RECT rcLayeredClient = paintFrame.rcClient;
			rcLayeredClient.left += m_rcLayeredInset.left;
			rcLayeredClient.top += m_rcLayeredInset.top;
			rcLayeredClient.right -= m_rcLayeredInset.right;
			rcLayeredClient.bottom -= m_rcLayeredInset.bottom;
			const bool bHadBackgroundSurface = m_backgroundSurface.IsReady();
			if (!EnsureLayeredBackgroundSurface(referenceContext, static_cast<LONG>(paintFrame.dwWidth), static_cast<LONG>(paintFrame.dwHeight))) {
				return false;
			}
			if (!bHadBackgroundSurface || m_bLayeredChanged) {
				::ZeroMemory(m_backgroundSurface.GetBits(), paintFrame.dwWidth * paintFrame.dwHeight * 4);
				CPaintRenderContext backgroundContext = m_backgroundSurface.CreateRenderContext(
					this,
					rcLayeredClient,
					referenceContext.GetActiveBackend(),
					referenceContext.GetActiveDirect2DRenderMode());
				if (!ExecuteBatchedDrawInfoPass(backgroundContext, &m_diLayered)) {
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

	bool CPaintManagerUI::RenderFrameToMainSurface(const CPaintRenderContext& referenceContext, const TPaintFrameState& paintFrame)
	{
		if (!EnsureMainRenderSurface(referenceContext, static_cast<LONG>(paintFrame.dwWidth), static_cast<LONG>(paintFrame.dwHeight))) {
			return false;
		}

		ClearBottomUpBitsRect(
			m_offscreenSurface.GetBits(),
			static_cast<LONG>(paintFrame.dwWidth),
			paintFrame.rcClient,
			paintFrame.rcPaint);
		CPaintRenderContext renderContext = m_offscreenSurface.CreateRenderContext(
			this,
			paintFrame.rcPaint,
			GetActiveRenderBackend(),
			GetActiveDirect2DRenderMode());
		return ExecuteBatchedControlRenderPass(renderContext, true, true);
	}

	bool CPaintManagerUI::RenderFrameDirectToWindow(CPaintRenderContext& targetContext, bool bIncludePostPaint)
	{
		return ExecuteBatchedControlRenderPass(targetContext, false, bIncludePostPaint);
	}

	void CPaintManagerUI::PrintChildWindows(CPaintRenderContext& targetContext, LPARAM lParam)
	{
		if (!HasNativeRenderContext(targetContext) || (lParam & PRF_CHILDREN) == 0) {
			return;
		}

		HWND hWndChild = ::GetWindow(m_hWndPaint, GW_CHILD);
		while (hWndChild != NULL) {
			RECT rcPos = { 0 };
			::GetWindowRect(hWndChild, &rcPos);
			::MapWindowPoints(HWND_DESKTOP, m_hWndPaint, reinterpret_cast<LPPOINT>(&rcPos), 2);
			CScopedRenderContextNativeDCSave childNativeDCSave(targetContext);
			if (!childNativeDCSave.IsValid()) {
				hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
				continue;
			}
			HDC hTargetNativeDC = childNativeDCSave.GetNativeDC();
			::SetWindowOrgEx(hTargetNativeDC, -rcPos.left, -rcPos.top, NULL);
			::SendMessage(hWndChild, WM_PRINT, reinterpret_cast<WPARAM>(hTargetNativeDC), lParam | PRF_NONCLIENT);
			childNativeDCSave.Restore();
			hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
		}
	}

	bool CPaintManagerUI::PresentLayeredSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame)
	{
		RECT rcWnd = { 0 };
		::GetWindowRect(m_hWndPaint, &rcWnd);

		POINT ptPos = { rcWnd.left, rcWnd.top };
		SIZE sizeWnd = { static_cast<LONG>(paintFrame.dwWidth), static_cast<LONG>(paintFrame.dwHeight) };
		return UpdateLayeredWindowFromRenderSurfaceInternal(
			m_offscreenSurface,
			m_hWndPaint,
			targetContext,
			ptPos,
			sizeWnd,
			static_cast<BYTE>(m_nOpacity),
			&paintFrame.rcPaint);
	}

	void CPaintManagerUI::PresentWindowSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame)
	{
		if (!HasNativeRenderContext(targetContext)) {
			return;
		}

		const POINT ptSource = { paintFrame.rcPaint.left, paintFrame.rcPaint.top };
		BitBltRenderSurfaceInternal(m_offscreenSurface, targetContext, paintFrame.rcPaint, ptSource, SRCCOPY);
	}

	bool CPaintManagerUI::PresentMainRenderSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame)
	{
		if (m_bLayered) {
			CPaintRenderContext offscreenContext = m_offscreenSurface.CreateRenderContext(
				this,
				paintFrame.rcPaint,
				targetContext.GetActiveBackend(),
				targetContext.GetActiveDirect2DRenderMode());
			if (!RenderLayeredBackgroundToSurface(offscreenContext, paintFrame)) {
				return false;
			}
			if (!PresentLayeredSurface(targetContext, paintFrame)) {
				return false;
			}
		}
		else {
			PresentWindowSurface(targetContext, paintFrame);
		}

		if (m_bShowUpdateRect && !m_bLayered && HasNativeRenderContext(targetContext)) {
			CRenderEngine::DrawRect(targetContext, paintFrame.rcPaint, 1, kUpdateRectDebugColor);
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

	bool CPaintManagerUI::HandlePrintClientMessage(CPaintRenderContext& targetContext, LPARAM lParam)
	{
		return RenderPrintClientFrame(targetContext, lParam);
	}

	CPaintManagerUI::CPaintManagerUI() :
		m_hWndPaint(NULL),
		m_bOffscreenPaint(false),
		m_hwndTooltip(NULL),
		m_uTimerID(0x1000),
		m_pRoot(NULL),
		m_pFocus(NULL),
		m_pEventHover(NULL),
		m_pEventClick(NULL),
		m_pEventRClick(NULL),
		m_pEventKey(NULL),
		m_pLastToolTip(NULL),
		m_bFirstLayout(true),
		m_bFocusNeeded(false),
		m_bUpdateNeeded(false),
		m_bMouseTracking(false),
		m_bMouseCapture(false),
		m_bIsPainting(false),
		m_bUsedVirtualWnd(false),
		m_bAsyncNotifyPosted(false),
		m_bAnimationFramePosted(false),
		m_bForceUseSharedRes(false),
		m_nOpacity(0xFF),
		m_bLayered(false),
		m_bLayeredChanged(false),
		m_bNoActivate(false),
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
		m_nScrollRenderCacheHits(0),
		m_nScrollRenderCacheMisses(0),
		m_nDirect2DBatchFlushes(0),
		m_nDirect2DStandaloneDraws(0),
		m_nSampleDirect2DBatchFlushes(0),
		m_nSampleDirect2DStandaloneDraws(0),
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

		if (!g_bCommonControlsInitialized) {
			g_bCommonControlsInitialized = true;
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
		Gdiplus::GdiplusStartup(&m_gdiplusToken, m_pGdiplusStartupInput, NULL); // 闂備礁鎲″缁樻叏閹灐褰掑床閹垫“闂備浇顫夋禍浠嬪磿鏉堫偁浜?

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

		ReleaseFontHandle(m_ResInfo.m_DefaultFontInfo, true);
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
		// 闂傚倷绀佺花閬嶅磻閹惧瓨鍙忛柨婵嗗閸斿秹鎮楀顒夊剶妤犵偛绻橀幐濠冨緞鐎ｎ剦娲梻?
		if (m_hDragBitmap != NULL) CRenderEngine::FreeBitmap(m_hDragBitmap);
		//闂備礁鎲￠〃鍡涙偤閺囩伝褰掑床閹垫“Plus
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
		delete m_pGdiplusStartupInput;
		// DPI缂傚倷鑳舵刊瀵告閺囥垹绠栧┑鐘插閸嬫捇鎮藉▓鎸庢暞闂?
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
		}
		if (m_aPreMessages.Find(this) < 0) {
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

		wchar_t tszModule[MAX_PATH + 1] = { 0 };
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
		wchar_t tszModule[MAX_PATH + 1] = { 0 };
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
			wchar_t cEnd = normalizedPath[normalizedPath.size() - 1];
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
		m_pStrResourceZipPwd.assign(password);  //Garfield 20160325 闂佹眹鍩勯崹鐣岀不閹达附鍎婇柟杈鹃檮閸庢ê霉濠婂啫鍔畃闂備礁鎲￠悧鏇㈠箠閹捐鏆伴梺?
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
			m_bOffscreenPaint = bLayered;
			if (!m_bOffscreenPaint) {
				ResetRenderSurfaces();
			}
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
		m_diLayered.Clear();
		m_diLayered.Parse(image, {}, this);
		m_bLayeredChanged = true;
		Invalidate();
	}

	CShadowUI* CPaintManagerUI::GetShadow()
	{
		return &m_shadow;
	}

	void CPaintManagerUI::SetRenderBackend(RenderBackendType backend)
	{
		const RenderBackendType resolvedBackend =
			backend == RenderBackendAuto ? RenderBackendAuto : RenderBackendDirect2D;
		if (m_renderBackend == resolvedBackend) return;
		m_renderBackend = resolvedBackend;
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
		diagnostics.nScrollRenderCacheHits = m_nScrollRenderCacheHits;
		diagnostics.nScrollRenderCacheMisses = m_nScrollRenderCacheMisses;
		diagnostics.nDirect2DBatchFlushes = m_nDirect2DBatchFlushes;
		diagnostics.nDirect2DStandaloneDraws = m_nDirect2DStandaloneDraws;
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
		m_nScrollRenderCacheHits = 0;
		m_nScrollRenderCacheMisses = 0;
		m_nDirect2DBatchFlushes = 0;
		m_nDirect2DStandaloneDraws = 0;
		m_nSampleDirect2DBatchFlushes = 0;
		m_nSampleDirect2DStandaloneDraws = 0;
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

		const double nTargetFrameMs = 1000.0 / 240.0;
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
				wchar_t szTrace[768] = { 0 };
				swprintf_s(
					szTrace,
					L"[FYUI] fps=%.1f paint(avg=%.2fms last=%.2fms worst=%.2fms) over8.33ms=%u native(hit=%u refresh=%u) image(hit=%u refresh=%u) text(prepHit=%u prepRefresh=%u layoutHit=%u layoutRefresh=%u metricsHit=%u metricsRefresh=%u) html(parseHit=%u parseRefresh=%u layoutHit=%u layoutRefresh=%u metricsHit=%u metricsRefresh=%u hitTestHit=%u hitTestRefresh=%u effectHit=%u effectRefresh=%u parseFail=%u layoutFail=%u renderFail=%u) d2d(flush=%u standalone=%u) backend=%d d2d=%d\r\n",
					nSampleFPS,
					nSamplePaintAvg,
					m_nLastPaintMs,
					m_nWorstPaintMs,
					m_nOverBudgetFrameCount,
					m_nSampleNativeWindowCacheHits,
					m_nSampleNativeWindowCacheRefreshes,
					m_nSampleImageScaleCacheHits,
					m_nSampleImageScaleCacheRefreshes,
					m_nSampleTextNormalizationCacheHits,
					m_nSampleTextNormalizationCacheRefreshes,
					m_nSampleTextLayoutCacheHits,
					m_nSampleTextLayoutCacheRefreshes,
					m_nSampleTextMetricsCacheHits,
					m_nSampleTextMetricsCacheRefreshes,
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

	bool CPaintManagerUI::PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes)
	{
		for (int i = 0; i < m_aPreMessageFilters.GetSize(); i++)
		{
			IMessageFilterUI* pFilter = static_cast<IMessageFilterUI*>(m_aPreMessageFilters[i]);
			if (pFilter == NULL) continue;

			bool bHandled = false;
			LRESULT lResult = pFilter->MessageHandler(uMsg, wParam, lParam, bHandled);
			if (bHandled) {
				lRes = lResult;
				return true;
			}
		}
		if (m_pRoot == NULL) {
			return false;
		}
		switch (uMsg) {
		case WM_KEYDOWN:
		{
			// Tabbing between controls
			if (wParam == VK_TAB)
			{
				if (m_pFocus && m_pFocus->IsVisible() && m_pFocus->IsEnabled() && StringUtil::Find(m_pFocus->GetClass(), _T("WkeWebkitUI")) != -1) {
					return false;
				}
				return SetNextTabControl(::GetKeyState(VK_SHIFT) >= 0);
			}
		}
		break;
		case WM_SYSCHAR:
		{
			// Handle ALT-shortcut key-combinations
			FINDSHORTCUT fs = { 0 };
			fs.ch = static_cast<wchar_t>(towupper(static_cast<wint_t>(wParam)));
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
				event.chKey = (wchar_t)wParam;
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
			IMessageFilterUI* pFilter = static_cast<IMessageFilterUI*>(m_aMessageFilters[i]);
			if (pFilter == NULL) continue;

			bool bHandled = false;

			LRESULT lResult = pFilter->MessageHandler(uMsg, wParam, lParam, bHandled);
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
				case WM_MOUSEHWHEEL:
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
					INotifyUI* pNotifier = static_cast<INotifyUI*>(m_aNotifiers[j]);
					if (pNotifier != NULL) pNotifier->Notify(*pMsg);
				}
				delete pMsg;
			}
		}
		break;
		case UIMSG_ANIMATION_FRAME:
		{
			m_bAnimationFramePosted = false;
			if (::GetUpdateRect(m_hWndPaint, NULL, FALSE)) {
				::UpdateWindow(m_hWndPaint);
			}
		}
		return true;
		
		case WM_CLOSE:
		{
			// Make sure all matching "closing" events are sent
			TEventUI event = { 0 };
			event.ptMouse = m_ptLastMousePos;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			if (m_pEventHover != NULL) {
				CControlUI* pHover = m_pEventHover;
				m_pEventHover = NULL;
				event.Type = UIEVENT_MOUSELEAVE;
				event.pSender = pHover;
				pHover->Event(event);
			}
			if (m_pEventClick != NULL) {
				CControlUI* pClick = m_pEventClick;
				m_pEventClick = NULL;
				event.Type = UIEVENT_BUTTONUP;
				event.pSender = pClick;
				pClick->Event(event);
			}
			if (m_pEventRClick != NULL)
			{
				CControlUI* pRClick = m_pEventRClick;
				m_pEventRClick = NULL;
				event.Type = UIEVENT_RBUTTONUP;
				event.pSender = pRClick;
				pRClick->Event(event);
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
			RECT rcClient = { 0 };
			if (!::GetClientRect(m_hWndPaint, &rcClient)) break;
			CPaintRenderContext printContext = CreatePrintClientNativeRenderContext(this, wParam, rcClient);
			if (!HandlePrintClientMessage(printContext, lParam)) {
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
				if (pTimer != NULL &&
					pTimer->pSender != NULL &&
					pTimer->hWnd == m_hWndPaint &&
					static_cast<UINT_PTR>(pTimer->uWinTimer) == static_cast<UINT_PTR>(wParam) &&
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
			m_ToolTip.lpszText = const_cast<wchar_t*>(m_sToolTipText.c_str());
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

				// 闂傚倷绶￠崑鍛┍閾忚宕查柛鎰╁妷濡插牓鏌涢銈呮瀺缂佽鲸鐗犻弻娑樷枎閹邦剛浼岄梺?TooltipInfo 闂備礁婀遍…鍫澝洪銏犵闁跨喓濮甸弲顒勬倵濞戞凹妫杘ltipWnd 闂佽崵濮甸崝妤呭窗閺囥垺鍎楁俊銈呮噺閻撳倸鈹戦悩鎻掓殲闁哄缍婇弻娑㈠箛椤掍礁娅ｉ梺?
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
			//闂備礁鎲￠悷锕傚垂閸ф鐒垫い鎴ｆ硶缁愭梻绱掗崡鐐靛煟鐎殿喕绮欏畷鍫曞Ω閵夈倕顥氶梻?
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
			// 闂備礁鎼€氱兘宕规导鏉戠畾濞达綀娅ｉ惌鎾绘偣閸ャ劌绲荤悮?
			bool bNeedDrag = true;
			if (m_ptLastMousePos.x == pt.x && m_ptLastMousePos.y == pt.y) {
				bNeedDrag = false;
			}
			// 闂佽崵濮抽悞锕€顭垮Ο鑲╃鐎广儱鏌堝Δ鍛亹鐎规洖娲ら悘杈ㄧ箾鏉堝墽鍒扮紒澶婄埣閹?
			m_ptLastMousePos = pt;
			CControlUI* pNewHover = FindControl(pt);
			if (pNewHover != NULL && pNewHover->GetManager() != this) break;

			// 闂備礁缍婇弨閬嶆偋濡ゅ啠鍋撳顓烆嚋缂佸倸绉堕埀顒婄岛閺呮粓顢?			if (bNeedDrag && m_bDragMode && wParam == MK_LBUTTON)
			if (bNeedDrag && m_bDragMode && wParam == MK_LBUTTON)
			{
				// 闂傚倷鐒﹁ぐ鍐晝閵夆晛妫樺☉鎾虫懗pture
				ReleaseCapture();
				// 闂備浇顫夋禍浠嬪磿鏉堫偁浜?
				if (m_pDragDrop != NULL && m_pDragDrop->OnDragDrop(m_pEventClick)) {

					m_bDragMode = false;
					break;
				}

				CScopedComObjectRef<CIDropSource> dropSource(new CIDropSource);
				if (!dropSource.IsValid()) return 0;

				CScopedComObjectRef<CIDataObject> dataObject(new CIDataObject(dropSource.Get()));
				if (!dataObject.IsValid()) return 0;

				FORMATETC fmtetc = { 0 };
				STGMEDIUM medium = { 0 };
				fmtetc.dwAspect = DVASPECT_CONTENT;
				fmtetc.lindex = -1;
				fmtetc.cfFormat = CF_BITMAP;
				fmtetc.tymed = TYMED_GDI;

				//////////////////////////////////////
				HBITMAP hBitmap = (HBITMAP)OleDuplicateData(m_hDragBitmap, fmtetc.cfFormat, NULL);
				medium.hBitmap = hBitmap;
				dataObject->SetData(&fmtetc, &medium, FALSE);

				//////////////////////////////////////
				BITMAP bmap;
				GetObject(hBitmap, sizeof(BITMAP), &bmap);
				RECT rc = { 0, 0, bmap.bmWidth, bmap.bmHeight };
				fmtetc.cfFormat = CF_ENHMETAFILE;
				fmtetc.tymed = TYMED_ENHMF;
				CPaintRenderContext measureContext = CreateMeasureRenderContext(rc);
				HDC hReferenceNativeDC = GetRenderContextNativeDC(measureContext);
				if (hReferenceNativeDC != NULL) {
					CScopedEnhMetaFileNativeDC metaNativeDC(hReferenceNativeDC);
					CScopedCompatibleNativeDC memoryNativeDC(hReferenceNativeDC);
					if (metaNativeDC.IsValid() && memoryNativeDC.IsValid()) {
						CScopedSelectedNativeObject selectedBitmap(memoryNativeDC.GetNativeDC(), hBitmap);
						if (selectedBitmap.IsValid()) {
							::BitBlt(metaNativeDC.GetNativeDC(), 0, 0, rc.right, rc.bottom, memoryNativeDC.GetNativeDC(), 0, 0, SRCCOPY);
							selectedBitmap.Restore();
							medium.hEnhMetaFile = metaNativeDC.CloseAndDetach();
						}
						if (medium.hEnhMetaFile != NULL) {
							medium.tymed = TYMED_ENHMF;
							dataObject->SetData(&fmtetc, &medium, TRUE);
						}
					}
				}
				//////////////////////////////////////
				CDragSourceHelper dragSrcHelper;
				POINT ptDrag = { 0 };
				ptDrag.x = bmap.bmWidth / 2;
				ptDrag.y = bmap.bmHeight / 2;
				dragSrcHelper.InitializeFromBitmap(hBitmap, ptDrag, rc, dataObject.Get()); //will own the bmp
				DWORD dwEffect = 0;
				::DoDragDrop(dataObject.Get(), dropSource.Get(), DROPEFFECT_COPY | DROPEFFECT_MOVE, &dwEffect);
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
			// 闂備礁鎼悮顐﹀磿閸欏鐝舵慨妞诲亾妤犵偞甯熼ˇ杈ㄧ箾?
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			m_ptLastMousePos = pt;
			CControlUI* pControl = FindControl(pt);
			if (pControl == NULL) break;
			if (pControl->GetManager() != this) break;

            // Prepare the drag image before we start capture.
            if (m_bDragDrop && pControl->IsDragEnabled()) {
				m_bDragMode = true;
				if (m_hDragBitmap != NULL) {
					CRenderEngine::FreeBitmap(m_hDragBitmap);
					m_hDragBitmap = NULL;
				}
				m_hDragBitmap = CRenderEngine::GenerateBitmap(this, pControl, pControl->GetPos());
			}

			// 闁诲孩顔栭崰鎺楀磻閹剧粯鐓曟慨妯煎帶閻忥紕鈧娲滈崰鏍箠?
			SetCapture();
			// Track the pressed control until button-up.
			m_pEventClick = pControl;
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
			SetCapture();
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
			CControlUI* pRClick = m_pEventRClick;
			if (pRClick == NULL) break;

			TEventUI event = { 0 };
			event.Type = UIEVENT_RBUTTONUP;
			event.pSender = pRClick;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
			pRClick->Event(event);
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
			CControlUI* pClick = m_pEventClick;
			m_pEventClick = NULL;
			if (pClick == NULL) break;

			TEventUI event = { 0 };
			event.Type = UIEVENT_MBUTTONUP;
			event.pSender = pClick;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.dwTimestamp = ::GetTickCount();
			pClick->Event(event);
		}
		break;
		case WM_CONTEXTMENU:
		{
			if (m_pRoot == NULL) break;
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			::ScreenToClient(m_hWndPaint, &pt);
			m_ptLastMousePos = pt;
			CControlUI* pContext = m_pEventRClick;
			m_pEventRClick = NULL;
			if (pContext == NULL) {
				pContext = FindControl(pt);
			}
			if (pContext == NULL || pContext->GetManager() != this) break;
			ReleaseCapture();
			TEventUI event = { 0 };
			event.Type = UIEVENT_CONTEXTMENU;
			event.pSender = pContext;
			event.wParam = wParam;
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = (WORD)wParam;
			event.lParam = (LPARAM)pContext;
			event.dwTimestamp = ::GetTickCount();
			pContext->Event(event);
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
		case WM_MOUSEHWHEEL:
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
			event.wParam = MAKEWPARAM(zDelta < 0 ? SB_LINERIGHT : SB_LINELEFT, zDelta);
			event.lParam = lParam;
			event.ptMouse = pt;
			event.wKeyState = MapKeyState();
			event.dwTimestamp = ::GetTickCount();
			pControl->Event(event);
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
			event.chKey = (wchar_t)wParam;
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
			event.chKey = (wchar_t)wParam;
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
			event.chKey = (wchar_t)wParam;
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
		if (m_hWndPaint == NULL || !::IsWindow(m_hWndPaint)) return;
		RECT rcClient = { 0 };
		if (!::GetClientRect(m_hWndPaint, &rcClient)) return;
		if (m_bLayered) {
			AccumulateLayeredUpdateRect(rcClient);
		}
		::InvalidateRect(m_hWndPaint, NULL, FALSE);
	}

	void CPaintManagerUI::Invalidate(RECT& rcItem)
	{
		if (m_hWndPaint == NULL || !::IsWindow(m_hWndPaint)) return;
		if (rcItem.left < 0) rcItem.left = 0;
		if (rcItem.top < 0) rcItem.top = 0;
		if (rcItem.right < rcItem.left) rcItem.right = rcItem.left;
		if (rcItem.bottom < rcItem.top) rcItem.bottom = rcItem.top;
		if (m_bLayered) {
			AccumulateLayeredUpdateRect(rcItem);
		}
		::InvalidateRect(m_hWndPaint, &rcItem, FALSE);
	}

	void CPaintManagerUI::RequestAnimationFrame(CControlUI* pControl)
	{
		if (m_hWndPaint == NULL || !::IsWindow(m_hWndPaint)) {
			return;
		}

		if (pControl != NULL && pControl->GetManager() == this && pControl->IsVisible()) {
			RECT rcFrame = pControl->GetPos();
			Invalidate(rcFrame);
		}
		else {
			Invalidate();
		}

		if (!m_bAnimationFramePosted) {
			m_bAnimationFramePosted = ::PostMessage(m_hWndPaint, UIMSG_ANIMATION_FRAME, 0, 0) != FALSE;
		}
	}

	bool CPaintManagerUI::ScrollRenderCacheRect(const RECT& rcScrollInput, int dx, int dy)
	{
		auto fail = [this]() {
			++m_nScrollRenderCacheMisses;
			return false;
		};

		if (m_hWndPaint == NULL || !::IsWindow(m_hWndPaint) || m_bLayered || (dx == 0 && dy == 0)) {
			return fail();
		}

		BYTE* pBits = m_offscreenSurface.GetBits();
		const SIZE surfaceSize = m_offscreenSurface.GetSize();
		if (pBits == nullptr || surfaceSize.cx <= 0 || surfaceSize.cy <= 0) {
			return fail();
		}

		RECT rcClient = { 0 };
		if (!::GetClientRect(m_hWndPaint, &rcClient)) {
			return fail();
		}

		RECT rcScroll = { 0 };
		if (!::IntersectRect(&rcScroll, &rcScrollInput, &rcClient)) {
			return fail();
		}

		const int width = rcScroll.right - rcScroll.left;
		const int height = rcScroll.bottom - rcScroll.top;
		if (width <= 0 || height <= 0 || std::abs(dx) >= width || std::abs(dy) >= height) {
			return fail();
		}

		RECT rcPending = { 0 };
		if (::GetUpdateRect(m_hWndPaint, &rcPending, FALSE)) {
			RECT rcPendingScroll = { 0 };
			if (::IntersectRect(&rcPendingScroll, &rcPending, &rcScroll)) {
				// Never move unpainted exposed strips again. If a previous scroll left
				// dirty pixels pending, flush them before shifting the surface/cache.
				::UpdateWindow(m_hWndPaint);
				::ZeroMemory(&rcPending, sizeof(rcPending));
				if (::GetUpdateRect(m_hWndPaint, &rcPending, FALSE) &&
					::IntersectRect(&rcPendingScroll, &rcPending, &rcScroll)) {
					return fail();
				}
			}
		}

		if (!ScrollBottomUpBitsRect(pBits, surfaceSize.cx, surfaceSize.cy, rcScroll, dx, dy)) {
			return fail();
		}

		::ScrollWindowEx(m_hWndPaint, dx, dy, &rcScroll, &rcScroll, NULL, NULL, 0);

		auto invalidateExposedScrollRect = [this, &rcScroll](RECT rcExpose) {
			// Pixel scrolling reuses the old back-buffer. Inflate the newly exposed strip
			// slightly so antialiased rounded corners and subpixel text edges are repainted
			// instead of leaving thin seams after a burst of fast wheel ticks.
			::InflateRect(&rcExpose, 4, 4);
			RECT rcDirty = { 0 };
			if (::IntersectRect(&rcDirty, &rcExpose, &rcScroll)) {
				Invalidate(rcDirty);
			}
		};

		if (dy != 0) {
			RECT rcExpose = rcScroll;
			const int exposed = (std::min)(std::abs(dy), height);
			if (dy > 0) {
				rcExpose.bottom = rcExpose.top + exposed;
			}
			else {
				rcExpose.top = rcExpose.bottom - exposed;
			}
			invalidateExposedScrollRect(rcExpose);
		}

		if (dx != 0) {
			RECT rcExpose = rcScroll;
			const int exposed = (std::min)(std::abs(dx), width);
			if (dx > 0) {
				rcExpose.right = rcExpose.left + exposed;
			}
			else {
				rcExpose.left = rcExpose.right - exposed;
			}
			invalidateExposedScrollRect(rcExpose);
		}

		++m_nScrollRenderCacheHits;
		return true;
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
		// 闂傚倷绀佺花閬嶅磻閹惧瓨鍙忛柨婵嗗缁佹壆绱掓笟鍥т簻闁崇懓鍟撮獮鍥敋閸涱厽鍊庨梻浣藉吹閸嬫稑螞濡も偓閳?
		CResourceManager::GetInstance()->Release();
		CControlFactory::GetInstance()->Release();

		// 婵犵數鍋為幐鎼佸箠閹版澘绠栧┑鐘叉搐缁€鍌炴煕濞嗘劧鑰块柣鎾櫊閹顦查悗姘煎幘閼?
		// 闂備焦鎮堕崕杈ㄦ櫠閼恒儲娅?
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
		// 濠殿喗甯楃粙鎺椻€﹂崼銉晣闁煎鍊愰崑鎾斥槈濞嗗秳娌紓?
		ReleaseFontHandle(m_SharedResInfo.m_DefaultFontInfo, true);
		// 闂備礁鎼粔鎾床閼碱剚顫?/ 濠殿喗甯楃粙鎺椻€﹂崼銉晣闁煎鍊撳▽顏堟煟閹邦喛藟闁?
		m_SharedResInfo.m_StyleHash.clear();
		m_SharedResInfo.m_AttrHash.clear();

		// 闂備胶顭堢换鎴炵箾婵犲伣娑氬枈閻ゆザ
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
		ResetRenderSurfaces();

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
	}

	void FYUI::CPaintManagerUI::RebuildFont(TFontInfo* pFontInfo)
	{
		if (pFontInfo == NULL || pFontInfo->sFontName.empty()) {
			return;
		}
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		CopyLogFontFaceName(lf, pFontInfo->sFontName);

		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -ScaleValue(pFontInfo->iSize);
		lf.lfQuality = CLEARTYPE_QUALITY;
		if (pFontInfo->bBold) lf.lfWeight += FW_BOLD;
		if (pFontInfo->bUnderline) lf.lfUnderline = TRUE;
		if (pFontInfo->bItalic) lf.lfItalic = TRUE;
		if (pFontInfo->bStrikeout) lf.lfStrikeOut = TRUE;
		HFONT hFont = ::CreateFontIndirect(&lf);
		if (hFont == NULL) {
			return;
		}
		if (pFontInfo->hFont != NULL) {
			::DeleteObject(pFontInfo->hFont);
		}
		pFontInfo->hFont = hFont;
		::ZeroMemory(&(pFontInfo->tm), sizeof(pFontInfo->tm));
		UpdateFontTextMetricsForManager(pFontInfo);
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
			CControlUI* pOldFocus = m_pFocus;
			m_pFocus = NULL;
			TEventUI event = { 0 };
			event.Type = UIEVENT_KILLFOCUS;
			event.pSender = pOldFocus;
			event.dwTimestamp = ::GetTickCount();
			pOldFocus->Event(event);
			SendNotify(pOldFocus, DUI_MSGTYPE_KILLFOCUS);
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
		if (m_hWndPaint != NULL && ::IsWindow(m_hWndPaint)) {
			::SetFocus(m_hWndPaint);
		}
		if (pControl == NULL) return;
		if (m_pFocus != NULL) {
			CControlUI* pOldFocus = m_pFocus;
			m_pFocus = NULL;
			TEventUI event = { 0 };
			event.Type = UIEVENT_KILLFOCUS;
			event.pSender = pOldFocus;
			event.dwTimestamp = ::GetTickCount();
			pOldFocus->Event(event);
			SendNotify(pOldFocus, DUI_MSGTYPE_KILLFOCUS);
		}
		if (m_pRoot == NULL) {
			m_bFocusNeeded = false;
			return;
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
		if (pControl == NULL || m_hWndPaint == NULL) {
			return false;
		}

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

		const UINT kFirstManagerTimerID = 0x1000;
		UINT uWinTimer = 0;
		for (UINT attempts = 0; attempts < 0xF000; ++attempts) {
			++m_uTimerID;
			if (m_uTimerID < kFirstManagerTimerID) {
				m_uTimerID += kFirstManagerTimerID;
			}

			bool bTimerInUse = false;
			for (int i = 0; i < m_aTimers.GetSize(); ++i) {
				const TIMERINFO* pTimer = static_cast<const TIMERINFO*>(m_aTimers[i]);
				if (pTimer != NULL && pTimer->hWnd == m_hWndPaint && pTimer->uWinTimer == m_uTimerID) {
					bTimerInUse = true;
					break;
				}
			}
			if (!bTimerInUse) {
				uWinTimer = m_uTimerID;
				break;
			}
		}
		if (uWinTimer == 0) {
			return false;
		}
		if (!::SetTimer(m_hWndPaint, uWinTimer, uElapse, NULL)) return FALSE;
		TIMERINFO* pTimer = new TIMERINFO;
		if (pTimer == NULL) {
			::KillTimer(m_hWndPaint, uWinTimer);
			return FALSE;
		}
		pTimer->hWnd = m_hWndPaint;
		pTimer->pSender = pControl;
		pTimer->nLocalID = nTimerID;
		pTimer->uWinTimer = uWinTimer;
		pTimer->bKilled = false;
		if (!m_aTimers.Add(pTimer)) {
			::KillTimer(m_hWndPaint, uWinTimer);
			delete pTimer;
			return false;
		}
		return true;
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
		if (m_hWndPaint == NULL || !::IsWindow(m_hWndPaint)) {
			m_bMouseCapture = false;
			return;
		}
		::SetCapture(m_hWndPaint);
		m_bMouseCapture = (::GetCapture() == m_hWndPaint);
	}

	void CPaintManagerUI::ReleaseCapture()
	{
		if (::GetCapture() == m_hWndPaint) {
			::ReleaseCapture();
		}
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
		if (m_pRoot == NULL) {
			m_bFocusNeeded = false;
			return false;
		}
		// If we're in the process of restructuring the layout we can delay the
		// focus calulation until the next repaint.
		if (m_bUpdateNeeded && bForward) {
			m_bFocusNeeded = true;
			if (m_hWndPaint != NULL && ::IsWindow(m_hWndPaint)) {
				::InvalidateRect(m_hWndPaint, NULL, FALSE);
			}
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
		if (pNotifier == NULL || m_aNotifiers.Find(pNotifier) >= 0) return false;
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
		if (pFilter == NULL || m_aPreMessageFilters.Find(pFilter) >= 0) return false;
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
		if (pFilter == NULL || m_aMessageFilters.Find(pFilter) >= 0) return false;
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
		if (pControl == NULL) return false;
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
		if (pControl == NULL || hChildWnd == NULL || !::IsWindow(hChildWnd)) return false;

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
			RECT rcChildWnd = GetNativeWindowRect(hChildWnd);
			Invalidate(rcChildWnd);
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
		if (m_bUsedVirtualWnd && Msg.pSender != NULL)
		{
			Msg.sVirtualWnd = Msg.pSender->GetVirtualWnd();
		}

		if (!bAsync) {
			// Send to all listeners
			if (Msg.pSender != NULL) {
				if (Msg.pSender->OnNotify) Msg.pSender->OnNotify(&Msg);
			}
			for (int i = 0; i < m_aNotifiers.GetSize(); i++) {
				INotifyUI* pNotifier = static_cast<INotifyUI*>(m_aNotifiers[i]);
				if (pNotifier != NULL) pNotifier->Notify(Msg);
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
			if (!m_aAsyncNotify.Add(pMsg)) {
				delete pMsg;
				return;
			}

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
				UpdateFontTextMetricsForManager(&m_SharedResInfo.m_DefaultFontInfo);
			}
			return &m_SharedResInfo.m_DefaultFontInfo;
		}
		else
		{
			if (m_ResInfo.m_DefaultFontInfo.tm.tmHeight == 0)
			{
				UpdateFontTextMetricsForManager(&m_ResInfo.m_DefaultFontInfo);
			}
			return &m_ResInfo.m_DefaultFontInfo;
		}
	}

	void CPaintManagerUI::SetDefaultFont(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared)
	{
		LOGFONT lf = { 0 };
		::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
		if (!fontName.empty()) {
			CopyLogFontFaceName(lf, fontName);
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
			ReleaseFontHandle(m_SharedResInfo.m_DefaultFontInfo, false);
			m_SharedResInfo.m_DefaultFontInfo.hFont = hFont;
			m_SharedResInfo.m_DefaultFontInfo.sFontName = lf.lfFaceName;
			m_SharedResInfo.m_DefaultFontInfo.iSize = nSize;
			m_SharedResInfo.m_DefaultFontInfo.bBold = bBold;
			m_SharedResInfo.m_DefaultFontInfo.bUnderline = bUnderline;
			m_SharedResInfo.m_DefaultFontInfo.bItalic = bItalic;
			m_SharedResInfo.m_DefaultFontInfo.bStrikeout = bStrikeout;
			::ZeroMemory(&m_SharedResInfo.m_DefaultFontInfo.tm, sizeof(m_SharedResInfo.m_DefaultFontInfo.tm));
			UpdateFontTextMetricsForManager(&m_SharedResInfo.m_DefaultFontInfo);
		}
		else
		{
			ReleaseFontHandle(m_ResInfo.m_DefaultFontInfo, false);
			m_ResInfo.m_DefaultFontInfo.hFont = hFont;
			m_ResInfo.m_DefaultFontInfo.sFontName = lf.lfFaceName;
			m_ResInfo.m_DefaultFontInfo.iSize = nSize;
			m_ResInfo.m_DefaultFontInfo.bBold = bBold;
			m_ResInfo.m_DefaultFontInfo.bUnderline = bUnderline;
			m_ResInfo.m_DefaultFontInfo.bItalic = bItalic;
			m_ResInfo.m_DefaultFontInfo.bStrikeout = bStrikeout;
			::ZeroMemory(&m_ResInfo.m_DefaultFontInfo.tm, sizeof(m_ResInfo.m_DefaultFontInfo.tm));
			UpdateFontTextMetricsForManager(&m_ResInfo.m_DefaultFontInfo);
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
			CopyLogFontFaceName(lf, fontName);
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
		if (!pFontInfo) {
			::DeleteObject(hFont);
			return NULL;
		}
		pFontInfo->hFont = hFont;
		pFontInfo->sFontName = lf.lfFaceName;
		pFontInfo->iSize = nSize;
		pFontInfo->bBold = bBold;
		pFontInfo->bUnderline = bUnderline;
		pFontInfo->bItalic = bItalic;
		pFontInfo->bStrikeout = bStrikeout;
		UpdateFontTextMetricsForManager(pFontInfo);
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
		if (hFont == NULL || nFonts == 0) {
			return;
		}
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
				int index = -1;
				if (pFontInfo && pFontInfo->hFont == hFont && StringUtil::TryParseInt(entry.first, index)) return index;
			}
		}
		else
		{
			for (const auto& entry : m_ResInfo.m_CustomFonts) {
				TFontInfo* pFontInfo = entry.second;
				int index = -1;
				if (pFontInfo && pFontInfo->hFont == hFont && StringUtil::TryParseInt(entry.first, index)) return index;
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
					int index = -1;
					return StringUtil::TryParseInt(entry.first, index) ? index : -1;
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
					int index = -1;
					return StringUtil::TryParseInt(entry.first, index) ? index : -1;
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
			UpdateFontTextMetricsForManager(pFontInfo);
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
			UpdateFontTextMetricsForManager(pFontInfo);
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
		const wchar_t* typePtr = type.empty() ? nullptr : typeName.c_str();

		TImageInfo* data = NULL;
		if (typePtr != NULL && !type.empty()) {
			int iIndex = -1;
			if (TryParseNonNegativeInt(bitmapKey, iIndex)) {
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
        // Empty keys are invalid for bitmap resource registration.
        if (bitmap.empty()) return NULL;
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

		auto& imageMap = (bShared || m_bForceUseSharedRes) ? m_SharedResInfo.m_ImageHash : m_ResInfo.m_ImageHash;
		const auto it = imageMap.find(bitmapKey);
		if (it != imageMap.end())
		{
			if (it->second != NULL && it->second->hBitmap == hBitmap) {
				it->second->hBitmap = NULL;
			}
			CRenderEngine::FreeImage(it->second);
			imageMap.erase(it);
		}
		imageMap[bitmapKey] = data;

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
			if (m_hWndPaint != NULL && ::IsWindow(m_hWndPaint) && ::PostMessage(m_hWndPaint, WM_APP + 1, 0, 0L)) {
				m_bAsyncNotifyPosted = true;
			}
		}
	}
	void CPaintManagerUI::ReloadSharedImages()
	{
		CDPI* pDpi = new CDPI();
		TImageInfo* data = NULL;
		TImageInfo* pNewData = NULL;
		std::vector<std::wstring> keysToRemove;
		for (auto& pair : m_SharedResInfo.m_ImageHash) {
			data = pair.second;
			pNewData = NULL;
				if (data != NULL) {
					if (!data->sResType.empty()) {
						int iIndex = -1;
						if (TryParseNonNegativeInt(pair.first, iIndex)) {
							pNewData = CRenderEngine::LoadImage(iIndex, pDpi->Scale(100), data->sResType.c_str(), data->dwMask);
						}
					}
					else {
						pNewData = CRenderEngine::LoadImage(pair.first, pDpi->Scale(100), {}, data->dwMask);
					}
					if (pNewData == NULL) {
						CRenderEngine::FreeImage(data);
						keysToRemove.push_back(pair.first);
						continue;
					}

					CRenderEngine::FreeImage(data, false);
					AdoptImagePayload(*data, *pNewData);
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
		// 濠电偠鎻紞鈧繛澶嬫礋瀵?vector 闂備浇銆€閸嬫捇鏌涢锝嗙闁艰尙濞€濮婂宕堕敐鍛闂佽崵鍠愬ú鎴犵矆娓氣偓瀹曠銇愰幒鎾寖闂侀潧鐗嗗ú锕傛偩?key闂備焦瀵х粙鎴︽儗閸屾稑顕遍柍鍝勬噹缁€鍌溾偓骞垮劚椤︻噣骞夋ィ鍐╃厱婵☆垳鍎らˉ鐐翠繆閻愭彃鈧湱绮氶柆宥庢晩闁兼亽鍎插В澶愭煟鎼粹剝璐￠柟铏尭閳?
		std::vector<std::wstring> keysToRemove;
		for (auto& pair : m_ResInfo.m_ImageHash) {
			data = pair.second;
			pNewData = NULL;
				if (data != NULL) {
					if (!data->sResType.empty()) {
						int iIndex = -1;
						if (TryParseNonNegativeInt(pair.first, iIndex)) {
							pNewData = CRenderEngine::LoadImage(iIndex, ScaleValue(100), data->sResType.c_str(), data->dwMask);
						}
					}
					else {
						pNewData = CRenderEngine::LoadImage(pair.first, ScaleValue(100), {}, data->dwMask);
					}
					if (pNewData == NULL) {
						CRenderEngine::FreeImage(data);
						keysToRemove.push_back(pair.first);
						continue;
					}
					CRenderEngine::FreeImage(data, false);
					AdoptImagePayload(*data, *pNewData);
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

		if (data->hBitmap == hBitmap) {
			InvalidateD2DBitmapCacheInternal(hBitmap);
			data->hBitmap = NULL;
		}
		if (data->pBits == pBits) {
			data->pBits = NULL;
		}
		CRenderEngine::FreeImage(data, false);
		data->hBitmap = hBitmap;
		data->pBits = pBits;
		data->pImage = nullptr;
		data->pSrcBits = nullptr;
		data->pHandle = nullptr;
		data->nX = iWidth;
		data->nY = iHeight;
		data->nDestWidth = iWidth;
		data->nDestHeight = iHeight;
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
		if (m_pRoot == nullptr)
			return nullptr;
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
		if (pParent == nullptr) return nullptr;

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
		if (pFS->ch == static_cast<wchar_t>(towupper(static_cast<wint_t>(pThis->GetShortcut())))) pFS->bPickNext = true;
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
		if (pMsg == NULL) return false;
		for (int i = 0; i < m_aTranslateAccelerator.GetSize(); i++)
		{
			ITranslateAccelerator* pTranslateAccelerator = static_cast<ITranslateAccelerator*>(m_aTranslateAccelerator[i]);
			if (pTranslateAccelerator == NULL) continue;

			LRESULT lResult = pTranslateAccelerator->TranslateAccelerator(pMsg);
			if (lResult == S_OK) return true;
		}
		return false;
	}

	bool CPaintManagerUI::TranslateMessage(const LPMSG pMsg)
	{
		if (pMsg == NULL || pMsg->hwnd == NULL) return false;
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
				if (pT == NULL) continue;
				HWND hTempParent = hWndParent;
				while (hTempParent)
				{
					if (pMsg->hwnd == pT->GetPaintWindow() || hTempParent == pT->GetPaintWindow())
					{
						if (pT->TranslateAccelerator(pMsg))
							return true;

						if (pT->PreMessageHandler(pMsg->message, pMsg->wParam, pMsg->lParam, lRes))
							return true;
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
				if (pT == NULL) continue;
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
		if (pTranslateAccelerator == NULL || m_aTranslateAccelerator.Find(pTranslateAccelerator) >= 0) return false;
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

	// 闂備礁鎼粔鎾床閼碱剚顫曢柨娑樺娑撳秹鏌ㄥ☉妯侯仾闁?	void CPaintManagerUI::AddStyle(std::wstring_view name, std::wstring_view declarationList, bool bShared)
	void CPaintManagerUI::AddStyle(std::wstring_view name, std::wstring_view declarationList, bool bShared)
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
			for (const auto& attribute : StringUtil::ParseQuotedAttributes(source, L'\'')) {
				if (attribute.value.empty()) {
					continue;
				}

				if (StringUtil::EqualsNoCase(attribute.key, L"file") || StringUtil::EqualsNoCase(attribute.key, L"res")) {
					imageName = attribute.value;
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"restype")) {
					imageResType = attribute.value;
				}
				else if (StringUtil::EqualsNoCase(attribute.key, L"mask")) {
					StringUtil::TryParseColor(attribute.value, mask);
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
			CScopedGlobalLock dibLock(medium.hGlobal);
			if (dibLock.IsValid()) {
				LPBITMAPINFOHEADER lpbi = dibLock.As<BITMAPINFOHEADER>();
				HBITMAP hBitmap = NULL;
				CScopedWindowDC screenDC(NULL);
				if (screenDC.IsValid()) {
					hBitmap = ::CreateDIBitmap(
						screenDC.GetNativeDC(),
						lpbi,
						CBM_INIT,
						reinterpret_cast<LPBYTE>(lpbi) + lpbi->biSize + ColorTableSize(lpbi),
						reinterpret_cast<LPBITMAPINFO>(lpbi),
						DIB_RGB_COLORS);
				}

				ReplaceNativeStaticBitmapImage(m_hTargetWnd, hBitmap);
				return true;
			}
		}

		if (pFmtEtc->cfFormat == CF_BITMAP && medium.tymed == TYMED_GDI && medium.hBitmap != NULL) {
			ReplaceNativeStaticBitmapImage(m_hTargetWnd, medium.hBitmap);
			return false;
		}

		if (pFmtEtc->cfFormat == CF_ENHMETAFILE && medium.tymed == TYMED_ENHMF) {
			ENHMETAHEADER emh = {};
			::GetEnhMetaFileHeader(medium.hEnhMetaFile, sizeof(ENHMETAHEADER), &emh);

			RECT rc = {};
			CScopedWindowDC targetDC(m_hTargetWnd);
			if (!targetDC.IsValid()) {
				return false;
			}

			HDC hTargetNativeDC = targetDC.GetNativeDC();
			const float pixelsX = static_cast<float>(::GetDeviceCaps(hTargetNativeDC, HORZRES));
			const float pixelsY = static_cast<float>(::GetDeviceCaps(hTargetNativeDC, VERTRES));
			const float mmx = static_cast<float>(::GetDeviceCaps(hTargetNativeDC, HORZSIZE));
			const float mmy = static_cast<float>(::GetDeviceCaps(hTargetNativeDC, VERTSIZE));

			rc.top = static_cast<int>(static_cast<float>(emh.rclFrame.top) * pixelsY / (mmy * 100.0f));
			rc.left = static_cast<int>(static_cast<float>(emh.rclFrame.left) * pixelsX / (mmx * 100.0f));
			rc.right = static_cast<int>(static_cast<float>(emh.rclFrame.right) * pixelsX / (mmx * 100.0f));
			rc.bottom = static_cast<int>(static_cast<float>(emh.rclFrame.bottom) * pixelsY / (mmy * 100.0f));

			CScopedCompatibleNativeDC memoryNativeDC(hTargetNativeDC);
			CScopedNativeGdiObject memoryBitmap(::CreateCompatibleBitmap(hTargetNativeDC, emh.rclBounds.right, emh.rclBounds.bottom));
			if (!memoryNativeDC.IsValid() || !memoryBitmap.IsValid()) {
				return false;
			}

			CScopedSelectedNativeObject selectedBitmap(memoryNativeDC.GetNativeDC(), memoryBitmap.Get());
			if (!selectedBitmap.IsValid()) {
				return false;
			}

			::PlayEnhMetaFile(memoryNativeDC.GetNativeDC(), medium.hEnhMetaFile, &rc);
			HBITMAP hBmp = reinterpret_cast<HBITMAP>(selectedBitmap.Restore());
			if (hBmp != memoryBitmap.Get()) {
				return false;
			}
			memoryBitmap.Detach();

			ReplaceNativeStaticBitmapImage(m_hTargetWnd, hBmp);
			return true;
		}

		if (pFmtEtc->cfFormat == CF_HDROP && medium.tymed == TYMED_HGLOBAL && medium.hGlobal != NULL) {
			CScopedGlobalLock dropLock(medium.hGlobal);
			if (dropLock.IsValid()) {
				HDROP hDrop = static_cast<HDROP>(dropLock.Get());
				wchar_t szFileName[MAX_PATH] = {};
				const UINT cFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
				if (cFiles > 0) {
					::DragQueryFile(hDrop, 0, szFileName, static_cast<UINT>(std::size(szFileName)));
					HBITMAP hBitmap = static_cast<HBITMAP>(::LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE));
					if (hBitmap != NULL) {
						ReplaceNativeStaticBitmapImage(m_hTargetWnd, hBitmap);
					}
				}
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




