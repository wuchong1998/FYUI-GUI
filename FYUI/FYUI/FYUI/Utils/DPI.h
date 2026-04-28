#pragma once
#ifndef DPI_ENUMS_DECLARED

typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

typedef enum MONITOR_DPI_TYPE {
	MDT_EFFECTIVE_DPI = 0,
	MDT_ANGULAR_DPI = 1,
	MDT_RAW_DPI = 2,
	MDT_DEFAULT = MDT_EFFECTIVE_DPI
} MONITOR_DPI_TYPE;

#define DPI_ENUMS_DECLARED
#endif // (DPI_ENUMS_DECLARED)

namespace FYUI
{
	class FYUI_API CDPI
	{
	public:
		CDPI(void);

	public:
		static int GetMainMonitorDPI();
		static int GetDPIOfMonitor(HMONITOR hMonitor);
		static int GetDPIOfMonitorNearestToPoint(POINT pt);
		static int GetDPIOfWindow(HWND hWnd);

	public:
		PROCESS_DPI_AWARENESS GetDPIAwareness();
		BOOL SetDPIAwareness(PROCESS_DPI_AWARENESS Awareness);
		UINT GetDPI() const;
		UINT GetScale() const;
		void SetScale(UINT uDPI);
		void CopyContextFrom(const CDPI& dpi);

		int LogicalToDevice(int iValue) const;
		RECT LogicalToDevice(RECT rcRect) const;
		void LogicalToDevice(RECT* pRect) const;
		POINT LogicalToDevice(POINT ptPoint) const;
		void LogicalToDevice(POINT* pPoint) const;
		SIZE LogicalToDevice(SIZE szSize) const;
		void LogicalToDevice(SIZE* pSize) const;

		int DeviceToLogical(int iValue) const;
		RECT DeviceToLogical(RECT rcRect) const;
		void DeviceToLogical(RECT* pRect) const;
		POINT DeviceToLogical(POINT ptPoint) const;
		void DeviceToLogical(POINT* pPoint) const;
		SIZE DeviceToLogical(SIZE szSize) const;
		void DeviceToLogical(SIZE* pSize) const;

		RECT Scale(RECT rcRect) const;
		void Scale(RECT *pRect) const;
		POINT Scale(POINT ptPoint) const;
		void Scale(POINT *pPoint) const;
		SIZE Scale(SIZE szSize) const;
		void Scale(SIZE *pSize) const;
		int Scale(int iValue) const;

		int ScaleBack(int iValue) const;
		RECT ScaleBack(RECT rcRect) const;
		void ScaleBack(RECT *pRect) const;
		POINT ScaleBack(POINT ptPoint) const;
		void ScaleBack(POINT *pPoint) const;
		SIZE ScaleBack(SIZE szSize) const;
		void ScaleBack(SIZE *pSize) const;

	private:
		int m_nScaleFactor;
		int m_nScaleFactorSDA;
		PROCESS_DPI_AWARENESS m_Awareness;
	};
}
