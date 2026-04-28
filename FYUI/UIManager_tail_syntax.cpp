#include "pch.h"
#include "Core/UIManager.h"

namespace FYUI {
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




