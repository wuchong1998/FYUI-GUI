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
		/**
		 * @brief 构造 CDPI 对象
		 * @details 用于构造 CDPI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CDPI(void);

	public:
		/**
		 * @brief 获取MainMonitorDPI
		 * @details 用于获取MainMonitorDPI。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		static int GetMainMonitorDPI();
		/**
		 * @brief 获取DPIOfMonitor
		 * @details 用于获取DPIOfMonitor。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hMonitor [in] hMonitor参数
		 * @return int 返回对应的数值结果
		 */
		static int GetDPIOfMonitor(HMONITOR hMonitor);
		/**
		 * @brief 获取DPIOfMonitorNearestToPoint
		 * @details 用于获取DPIOfMonitorNearestToPoint。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return int 返回对应的数值结果
		 */
		static int GetDPIOfMonitorNearestToPoint(POINT pt);
		/**
		 * @brief 获取DPIOf窗口
		 * @details 用于获取DPIOf窗口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 * @return int 返回对应的数值结果
		 */
		static int GetDPIOfWindow(HWND hWnd);

	public:
		/**
		 * @brief 获取DPIAwareness
		 * @details 用于获取DPIAwareness。具体行为由当前对象状态以及传入参数共同决定。
		 * @return PROCESS_DPI_AWARENESS 返回 获取DPIAwareness 的结果
		 */
		PROCESS_DPI_AWARENESS GetDPIAwareness();
		/**
		 * @brief 设置DPIAwareness
		 * @details 用于设置DPIAwareness。具体行为由当前对象状态以及传入参数共同决定。
		 * @param Awareness [in] Awareness参数
		 * @return BOOL 返回 设置DPIAwareness 的结果
		 */
		BOOL SetDPIAwareness(PROCESS_DPI_AWARENESS Awareness);
		/**
		 * @brief 获取DPI
		 * @details 用于获取DPI。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetDPI() const;
		/**
		 * @brief 获取缩放比例
		 * @details 用于获取缩放比例。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetScale() const;
		/**
		 * @brief 设置缩放比例
		 * @details 用于设置缩放比例。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uDPI [in] DPI标志
		 */
		void SetScale(UINT uDPI);
		/**
		 * @brief 复制上下文From
		 * @details 用于复制上下文From。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dpi [in] dpi参数
		 */
		void CopyContextFrom(const CDPI& dpi);

		/**
		 * @brief 执行 LogicalToDevice 操作
		 * @details 用于执行 LogicalToDevice 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iValue [in] 值值
		 * @return int 返回对应的数值结果
		 */
		int LogicalToDevice(int iValue) const;
		/**
		 * @brief 执行 LogicalToDevice 操作
		 * @details 用于执行 LogicalToDevice 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcRect [in] 矩形矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT LogicalToDevice(RECT rcRect) const;
		/**
		 * @brief 执行 LogicalToDevice 操作
		 * @details 用于执行 LogicalToDevice 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRect [in] 矩形对象
		 */
		void LogicalToDevice(RECT* pRect) const;
		/**
		 * @brief 执行 LogicalToDevice 操作
		 * @details 用于执行 LogicalToDevice 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ptPoint [in] Point坐标点
		 * @return 返回对应的几何结果
		 */
		POINT LogicalToDevice(POINT ptPoint) const;
		/**
		 * @brief 执行 LogicalToDevice 操作
		 * @details 用于执行 LogicalToDevice 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pPoint [in] Point对象
		 */
		void LogicalToDevice(POINT* pPoint) const;
		/**
		 * @brief 执行 LogicalToDevice 操作
		 * @details 用于执行 LogicalToDevice 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szSize [in] 尺寸尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE LogicalToDevice(SIZE szSize) const;
		/**
		 * @brief 执行 LogicalToDevice 操作
		 * @details 用于执行 LogicalToDevice 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pSize [in] 尺寸对象
		 */
		void LogicalToDevice(SIZE* pSize) const;

		/**
		 * @brief 执行 DeviceToLogical 操作
		 * @details 用于执行 DeviceToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iValue [in] 值值
		 * @return int 返回对应的数值结果
		 */
		int DeviceToLogical(int iValue) const;
		/**
		 * @brief 执行 DeviceToLogical 操作
		 * @details 用于执行 DeviceToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcRect [in] 矩形矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT DeviceToLogical(RECT rcRect) const;
		/**
		 * @brief 执行 DeviceToLogical 操作
		 * @details 用于执行 DeviceToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRect [in] 矩形对象
		 */
		void DeviceToLogical(RECT* pRect) const;
		/**
		 * @brief 执行 DeviceToLogical 操作
		 * @details 用于执行 DeviceToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ptPoint [in] Point坐标点
		 * @return 返回对应的几何结果
		 */
		POINT DeviceToLogical(POINT ptPoint) const;
		/**
		 * @brief 执行 DeviceToLogical 操作
		 * @details 用于执行 DeviceToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pPoint [in] Point对象
		 */
		void DeviceToLogical(POINT* pPoint) const;
		/**
		 * @brief 执行 DeviceToLogical 操作
		 * @details 用于执行 DeviceToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szSize [in] 尺寸尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE DeviceToLogical(SIZE szSize) const;
		/**
		 * @brief 执行 DeviceToLogical 操作
		 * @details 用于执行 DeviceToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pSize [in] 尺寸对象
		 */
		void DeviceToLogical(SIZE* pSize) const;

		/**
		 * @brief 执行 Scale 操作
		 * @details 用于执行 Scale 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcRect [in] 矩形矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT Scale(RECT rcRect) const;
		/**
		 * @brief 执行 Scale 操作
		 * @details 用于执行 Scale 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRect [in] 矩形对象
		 */
		void Scale(RECT *pRect) const;
		/**
		 * @brief 执行 Scale 操作
		 * @details 用于执行 Scale 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ptPoint [in] Point坐标点
		 * @return 返回对应的几何结果
		 */
		POINT Scale(POINT ptPoint) const;
		/**
		 * @brief 执行 Scale 操作
		 * @details 用于执行 Scale 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pPoint [in] Point对象
		 */
		void Scale(POINT *pPoint) const;
		/**
		 * @brief 执行 Scale 操作
		 * @details 用于执行 Scale 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szSize [in] 尺寸尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE Scale(SIZE szSize) const;
		/**
		 * @brief 执行 Scale 操作
		 * @details 用于执行 Scale 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pSize [in] 尺寸对象
		 */
		void Scale(SIZE *pSize) const;
		/**
		 * @brief 执行 Scale 操作
		 * @details 用于执行 Scale 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iValue [in] 值值
		 * @return int 返回对应的数值结果
		 */
		int Scale(int iValue) const;

		/**
		 * @brief 执行 ScaleBack 操作
		 * @details 用于执行 ScaleBack 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iValue [in] 值值
		 * @return int 返回对应的数值结果
		 */
		int ScaleBack(int iValue) const;
		/**
		 * @brief 执行 ScaleBack 操作
		 * @details 用于执行 ScaleBack 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcRect [in] 矩形矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT ScaleBack(RECT rcRect) const;
		/**
		 * @brief 执行 ScaleBack 操作
		 * @details 用于执行 ScaleBack 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pRect [in] 矩形对象
		 */
		void ScaleBack(RECT *pRect) const;
		/**
		 * @brief 执行 ScaleBack 操作
		 * @details 用于执行 ScaleBack 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ptPoint [in] Point坐标点
		 * @return 返回对应的几何结果
		 */
		POINT ScaleBack(POINT ptPoint) const;
		/**
		 * @brief 执行 ScaleBack 操作
		 * @details 用于执行 ScaleBack 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pPoint [in] Point对象
		 */
		void ScaleBack(POINT *pPoint) const;
		/**
		 * @brief 执行 ScaleBack 操作
		 * @details 用于执行 ScaleBack 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szSize [in] 尺寸尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE ScaleBack(SIZE szSize) const;
		/**
		 * @brief 执行 ScaleBack 操作
		 * @details 用于执行 ScaleBack 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pSize [in] 尺寸对象
		 */
		void ScaleBack(SIZE *pSize) const;

	private:
		int m_nScaleFactor;
		int m_nScaleFactorSDA;
		PROCESS_DPI_AWARENESS m_Awareness;
	};
}
