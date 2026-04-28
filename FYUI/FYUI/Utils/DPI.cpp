#include "pch.h"
#include "DPI.h"
#include "VersionHelpers.h"

namespace FYUI
{
	//96 DPI = 100% scaling
	//120 DPI = 125% scaling
	//144 DPI = 150% scaling
	//168 DPI = 175% scaling
	//192 DPI = 200% scaling

	typedef HRESULT (WINAPI *LPSetProcessDpiAwareness)(
		_In_ PROCESS_DPI_AWARENESS value
		);

	typedef HRESULT (WINAPI *LPGetProcessDpiAwareness)(
		_In_  HANDLE                hprocess,
		_Out_ PROCESS_DPI_AWARENESS *value
		);


	typedef HRESULT (WINAPI *LPGetDpiForMonitor)(
		_In_  HMONITOR         hmonitor,
		_In_  MONITOR_DPI_TYPE dpiType,
		_Out_ UINT             *dpiX,
		_Out_ UINT             *dpiY
		);

	typedef UINT (WINAPI *LPGetDpiForWindow)(
		_In_ HWND hwnd
		);

	namespace
	{
		constexpr UINT kDefaultDpi = 96;
		constexpr UINT kDefaultScale = 100;

		class CScopedScreenDC
		{
		public:
			CScopedScreenDC()
				: m_hDC(::GetDC(nullptr))
			{
			}

			~CScopedScreenDC()
			{
				if (m_hDC != NULL) {
					::ReleaseDC(nullptr, m_hDC);
				}
			}

			CScopedScreenDC(const CScopedScreenDC&) = delete;
			CScopedScreenDC& operator=(const CScopedScreenDC&) = delete;

			HDC Get() const
			{
				return m_hDC;
			}

		private:
			HDC m_hDC = NULL;
		};

		HMODULE GetShcoreModule()
		{
			static HMODULE s_module = ::LoadLibraryW(L"Shcore.dll");
			return s_module;
		}

		LPGetDpiForMonitor GetGetDpiForMonitorFn()
		{
			static LPGetDpiForMonitor s_fn = reinterpret_cast<LPGetDpiForMonitor>(
				::GetProcAddress(GetShcoreModule(), "GetDpiForMonitor"));
			return s_fn;
		}

		LPGetProcessDpiAwareness GetGetProcessDpiAwarenessFn()
		{
			static LPGetProcessDpiAwareness s_fn = reinterpret_cast<LPGetProcessDpiAwareness>(
				::GetProcAddress(GetShcoreModule(), "GetProcessDpiAwareness"));
			return s_fn;
		}

		LPSetProcessDpiAwareness GetSetProcessDpiAwarenessFn()
		{
			static LPSetProcessDpiAwareness s_fn = reinterpret_cast<LPSetProcessDpiAwareness>(
				::GetProcAddress(GetShcoreModule(), "SetProcessDpiAwareness"));
			return s_fn;
		}

		LPGetDpiForWindow GetGetDpiForWindowFn()
		{
			static HMODULE s_user32 = ::GetModuleHandleW(L"user32.dll");
			static LPGetDpiForWindow s_fn = s_user32 == NULL ? nullptr :
				reinterpret_cast<LPGetDpiForWindow>(::GetProcAddress(s_user32, "GetDpiForWindow"));
			return s_fn;
		}

		UINT NormalizeDpi(UINT dpi)
		{
			return dpi == 0 ? kDefaultDpi : dpi;
		}

		UINT GetSystemScreenDpi()
		{
			CScopedScreenDC screenDC;
			if (screenDC.Get() == NULL) {
				return kDefaultDpi;
			}
			const int dpi = ::GetDeviceCaps(screenDC.Get(), LOGPIXELSX);
			return dpi > 0 ? static_cast<UINT>(dpi) : kDefaultDpi;
		}

		int ScaleValue(int value, UINT scalePercent)
		{
			if (scalePercent == 0 || scalePercent == kDefaultScale) {
				return value;
			}

			int result = ::MulDiv(value, static_cast<int>(scalePercent), static_cast<int>(kDefaultScale));
			if (value > 0 && result == 0) {
				result = 1;
			}
			if (value < 0 && result == 0) {
				result = -1;
			}
			return result;
		}

		int UnscaleValue(int value, UINT scalePercent)
		{
			if (scalePercent == 0 || scalePercent == kDefaultScale) {
				return value;
			}
			return ::MulDiv(value, static_cast<int>(kDefaultScale), static_cast<int>(scalePercent));
		}
	}


	CDPI::CDPI()
	{
		m_nScaleFactor = 0;
		m_nScaleFactorSDA = 0;
		m_Awareness = PROCESS_PER_MONITOR_DPI_AWARE;

		SetScale(kDefaultDpi);

	}

	int CDPI::GetDPIOfMonitor(HMONITOR hMonitor)
	{
		if (hMonitor == NULL) {
			return static_cast<int>(GetSystemScreenDpi());
		}

		UINT dpiX = kDefaultDpi;
		UINT dpiY = kDefaultDpi;
		if (IsWindows8Point1OrGreater()) {
			LPGetDpiForMonitor getDpiForMonitor = GetGetDpiForMonitorFn();
			if (getDpiForMonitor != NULL &&
				SUCCEEDED(getDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)) &&
				dpiX > 0) {
				return static_cast<int>(dpiX);
			}
		}

		return static_cast<int>(GetSystemScreenDpi());
	}

	int CDPI::GetDPIOfMonitorNearestToPoint(POINT pt)
	{
		HMONITOR hMonitor;
		hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		return GetDPIOfMonitor(hMonitor);
	}

	int CDPI::GetDPIOfWindow(HWND hWnd)
	{
		if (hWnd == NULL || !::IsWindow(hWnd)) {
			return GetMainMonitorDPI();
		}

		LPGetDpiForWindow getDpiForWindow = GetGetDpiForWindowFn();
		if (getDpiForWindow != NULL) {
			const UINT dpi = getDpiForWindow(hWnd);
			if (dpi > 0) {
				return static_cast<int>(dpi);
			}
		}

		return GetDPIOfMonitor(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST));
	}

	int CDPI::GetMainMonitorDPI()
	{
		POINT    pt;
		// Get the DPI for the main monitor
		pt.x = 1;
		pt.y = 1;
		return GetDPIOfMonitorNearestToPoint(pt);
	}

	PROCESS_DPI_AWARENESS CDPI::GetDPIAwareness()
	{
		if (IsWindows8Point1OrGreater()) {
			LPGetProcessDpiAwareness getProcessDpiAwareness = GetGetProcessDpiAwarenessFn();
			if (getProcessDpiAwareness != NULL) {
				PROCESS_DPI_AWARENESS awareness = m_Awareness;
				if (SUCCEEDED(getProcessDpiAwareness(::GetCurrentProcess(), &awareness))) {
					m_Awareness = awareness;
				}
			}
		}

		return m_Awareness;
	}

	BOOL CDPI::SetDPIAwareness(PROCESS_DPI_AWARENESS Awareness)
	{
		BOOL bRet = FALSE;
		if (IsWindows8Point1OrGreater()) {
			LPSetProcessDpiAwareness setProcessDpiAwareness = GetSetProcessDpiAwarenessFn();
			if (setProcessDpiAwareness != NULL && setProcessDpiAwareness(Awareness) == S_OK) {
				m_Awareness = Awareness;
				bRet = TRUE;
			}
		}
		else {
			m_Awareness = Awareness;
			bRet = TRUE;
		}
		return bRet;
	}

	UINT FYUI::CDPI::GetDPI() const
	{
		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return kDefaultDpi;
		}

		if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
			return NormalizeDpi(static_cast<UINT>(::MulDiv(m_nScaleFactorSDA, kDefaultDpi, kDefaultScale)));
		}

		return NormalizeDpi(static_cast<UINT>(::MulDiv(m_nScaleFactor, kDefaultDpi, kDefaultScale)));
	}

	UINT CDPI::GetScale() const
	{
		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return kDefaultScale;
		}
		if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
			return static_cast<UINT>(m_nScaleFactorSDA);
		}
		return static_cast<UINT>(m_nScaleFactor);
	}


	void CDPI::SetScale(UINT uDPI)
	{
		const UINT dpi = NormalizeDpi(uDPI);
		m_nScaleFactor = ::MulDiv(dpi, kDefaultScale, kDefaultDpi);
		if (m_nScaleFactorSDA == 0) {
			m_nScaleFactorSDA = m_nScaleFactor;
		}
	}

	void CDPI::CopyContextFrom(const CDPI& dpi)
	{
		m_nScaleFactor = dpi.m_nScaleFactor;
		m_nScaleFactorSDA = dpi.m_nScaleFactorSDA;
		m_Awareness = dpi.m_Awareness;
	}

	int CDPI::LogicalToDevice(int iValue) const
	{
		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return iValue;
		}

		if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
			return ScaleValue(iValue, static_cast<UINT>(m_nScaleFactorSDA));
		}

		return ScaleValue(iValue, static_cast<UINT>(m_nScaleFactor));
	}

	int CDPI::DeviceToLogical(int iValue) const
	{
		if (m_Awareness == PROCESS_DPI_UNAWARE) {
			return iValue;
		}

		if (m_Awareness == PROCESS_SYSTEM_DPI_AWARE) {
			return UnscaleValue(iValue, static_cast<UINT>(m_nScaleFactorSDA));
		}

		return UnscaleValue(iValue, static_cast<UINT>(m_nScaleFactor));
	}

	RECT CDPI::LogicalToDevice(RECT rcRect) const
	{
		RECT rcScale = rcRect;
		LogicalToDevice(&rcScale);
		return rcScale;
	}

	void CDPI::LogicalToDevice(RECT* pRect) const
	{
		int sw = LogicalToDevice(pRect->right - pRect->left);
		int sh = LogicalToDevice(pRect->bottom - pRect->top);
		pRect->left = LogicalToDevice(pRect->left);
		pRect->top = LogicalToDevice(pRect->top);
		pRect->right = pRect->left + sw;
		pRect->bottom = pRect->top + sh;
	}

	RECT CDPI::DeviceToLogical(RECT rcRect) const
	{
		RECT rcScale = rcRect;
		DeviceToLogical(&rcScale);
		return rcScale;
	}

	void CDPI::DeviceToLogical(RECT* pRect) const
	{
		int sw = DeviceToLogical(pRect->right - pRect->left);
		int sh = DeviceToLogical(pRect->bottom - pRect->top);
		pRect->left = DeviceToLogical(pRect->left);
		pRect->top = DeviceToLogical(pRect->top);
		pRect->right = pRect->left + sw;
		pRect->bottom = pRect->top + sh;
	}

	POINT CDPI::LogicalToDevice(POINT ptPoint) const
	{
		POINT ptScale = ptPoint;
		LogicalToDevice(&ptScale);
		return ptScale;
	}

	void CDPI::LogicalToDevice(POINT* pPoint) const
	{
		pPoint->x = LogicalToDevice(pPoint->x);
		pPoint->y = LogicalToDevice(pPoint->y);
	}

	POINT CDPI::DeviceToLogical(POINT ptPoint) const
	{
		POINT ptScale = ptPoint;
		DeviceToLogical(&ptScale);
		return ptScale;
	}

	void CDPI::DeviceToLogical(POINT* pPoint) const
	{
		pPoint->x = DeviceToLogical(pPoint->x);
		pPoint->y = DeviceToLogical(pPoint->y);
	}

	SIZE CDPI::LogicalToDevice(SIZE szSize) const
	{
		SIZE szScale = szSize;
		LogicalToDevice(&szScale);
		return szScale;
	}

	void CDPI::LogicalToDevice(SIZE* pSize) const
	{
		pSize->cx = LogicalToDevice(pSize->cx);
		pSize->cy = LogicalToDevice(pSize->cy);
	}

	SIZE CDPI::DeviceToLogical(SIZE szSize) const
	{
		SIZE szScale = szSize;
		DeviceToLogical(&szScale);
		return szScale;
	}

	void CDPI::DeviceToLogical(SIZE* pSize) const
	{
		pSize->cx = DeviceToLogical(pSize->cx);
		pSize->cy = DeviceToLogical(pSize->cy);
	}

	RECT CDPI::Scale(RECT rcRect) const
	{
		return LogicalToDevice(rcRect);
	}

	void CDPI::Scale(RECT *pRect) const
	{
		LogicalToDevice(pRect);
	}

	POINT CDPI::Scale(POINT ptPoint) const
	{
		return LogicalToDevice(ptPoint);
	}

	void CDPI::Scale(POINT *pPoint) const
	{
		LogicalToDevice(pPoint);
	}

	SIZE CDPI::Scale(SIZE szSize) const
	{
		return LogicalToDevice(szSize);
	}

	void CDPI::Scale(SIZE *pSize) const
	{
		LogicalToDevice(pSize);
	}

	int CDPI::Scale(int iValue) const
	{
		return LogicalToDevice(iValue);
	}

	int CDPI::ScaleBack(int iValue) const
	{
		return DeviceToLogical(iValue);
	}

	RECT CDPI::ScaleBack(RECT rcRect) const
	{
		return DeviceToLogical(rcRect);
	}

	void CDPI::ScaleBack(RECT *pRect) const
	{
		DeviceToLogical(pRect);
	}

	POINT CDPI::ScaleBack(POINT ptPoint) const
	{
		return DeviceToLogical(ptPoint);
	}

	void CDPI::ScaleBack(POINT *pPoint) const
	{
		DeviceToLogical(pPoint);
	}

	SIZE CDPI::ScaleBack(SIZE szSize) const
	{
		return DeviceToLogical(szSize);
	}

	void CDPI::ScaleBack(SIZE *pSize) const
	{
		DeviceToLogical(pSize);
	}
}
