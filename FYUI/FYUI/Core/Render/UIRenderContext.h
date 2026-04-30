#pragma once

#include <windows.h>

#include "UIRenderTypes.h"

namespace FYUI
{
	class CPaintManagerUI;

	class FYUI_API CPaintRenderContext
	{
	public:
		/**
		 * @brief 构造 CPaintRenderContext 对象
		 * @details 用于构造 CPaintRenderContext 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param hDC [in] hDC参数
		 * @param rcPaint [in] 重绘区域
		 * @param activeBackend [in] active渲染后端参数
		 * @param activeDirect2DMode [in] active直接2DMode参数
		 */
		CPaintRenderContext(
			CPaintManagerUI* pManager,
			HDC hDC,
			const RECT& rcPaint,
			RenderBackendType activeBackend = RenderBackendAuto,
			Direct2DRenderMode activeDirect2DMode = Direct2DRenderModeAuto)
			: m_pManager(pManager),
			m_hDC(hDC),
			m_rcPaint(rcPaint),
			m_activeBackend(activeBackend),
			m_activeDirect2DMode(activeDirect2DMode)
		{
		}

		/**
		 * @brief 获取管理器
		 * @details 用于获取管理器。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CPaintManagerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CPaintManagerUI* GetManager() const
		{
			return m_pManager;
		}

		/**
		 * @brief 获取DC
		 * @details 用于获取DC。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HDC 返回 获取DC 的结果
		 */
		HDC GetDC() const
		{
			return m_hDC;
		}

		/**
		 * @brief 获取绘制矩形
		 * @details 用于获取绘制矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		const RECT& GetPaintRect() const
		{
			return m_rcPaint;
		}

		/**
		 * @brief 获取Active渲染后端
		 * @details 用于获取Active渲染后端。具体行为由当前对象状态以及传入参数共同决定。
		 * @return RenderBackendType 返回 获取Active渲染后端 的结果
		 */
		RenderBackendType GetActiveBackend() const
		{
			return m_activeBackend;
		}

		/**
		 * @brief 获取Active直接2D渲染Mode
		 * @details 用于获取Active直接2D渲染Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return Direct2DRenderMode 返回 获取Active直接2D渲染Mode 的结果
		 */
		Direct2DRenderMode GetActiveDirect2DRenderMode() const
		{
			return m_activeDirect2DMode;
		}

		/**
		 * @brief 判断是否直接2D渲染后端
		 * @details 用于判断是否直接2D渲染后端。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsDirect2DBackend() const
		{
			return m_activeBackend == RenderBackendDirect2D;
		}

	private:
		CPaintManagerUI* m_pManager = nullptr;
		HDC m_hDC = NULL;
		RECT m_rcPaint = { 0, 0, 0, 0 };
		RenderBackendType m_activeBackend = RenderBackendAuto;
		Direct2DRenderMode m_activeDirect2DMode = Direct2DRenderModeAuto;
	};

}
