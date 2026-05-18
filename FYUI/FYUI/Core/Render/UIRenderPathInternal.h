#pragma once

// ============================================================
// UIRenderPathInternal.h — CRenderPath PIMPL 完整定义（仅 FYUI 内部）
//
// 出于以下两点原因把 CRenderPathImpl 抽到独立的 internal 头：
//   1) UIRenderPath.cpp 维护 PIMPL 的命令缓存与 D2D PathGeometry 缓存；
//   2) UIRender.cpp 实现 StrokePath / FillPath / PushPathClip / PopPathClip
//      时也需要访问 PIMPL 的 d2dGeometry 与 commands，
//      因此 PIMPL 的完整定义必须对二者可见，而对业务 .h 保持隐藏。
//
// 业务代码不应该 include 本头。
// ============================================================

#include <windows.h>
#include <d2d1.h>
#include <wrl/client.h>

#include <cstdint>
#include <vector>

#include "UIRenderPath.h"

namespace FYUI
{
	class CRenderPathImpl
	{
	public:
		enum class CommandType : std::uint8_t
		{
			MoveTo,
			LineTo,
			BezierTo,
			QuadraticTo,
			ArcTo,
			Close,
		};

		struct ArcParams
		{
			float radiusX = 0.0f;
			float radiusY = 0.0f;
			float rotationAngle = 0.0f;
			bool  largeArc = false;
			bool  sweepClockwise = false;
		};

		struct Command
		{
			CommandType type = CommandType::MoveTo;
			POINTF p0 = { 0, 0 };  // MoveTo/LineTo 终点；ArcTo 终点；BezierTo/QuadraticTo 终点
			POINTF p1 = { 0, 0 };  // BezierTo c1；QuadraticTo c
			POINTF p2 = { 0, 0 };  // BezierTo c2
			ArcParams arc;          // 仅 ArcTo 有效
		};

		std::vector<Command> commands;

		bool   building = false;   // Begin 之后、End 之前
		bool   sealed   = false;   // End 之后
		bool   hasBounds = false;
		RECTF  cachedBounds = { 0, 0, 0, 0 };

		// D2D 几何缓存：
		// - End() 后置 dirty=true，但不立即构建（避免没人用的路径也走 D2D 资源）
		// - StrokePath/FillPath/PushPathClip 第一次使用时按需构建并缓存；
		// - 后续若再次 Begin()，d2dGeometry 会被释放，dirty 重置。
		Microsoft::WRL::ComPtr<ID2D1PathGeometry> d2dGeometry;
		bool d2dGeometryDirty = false;

		void Reset()
		{
			commands.clear();
			building = false;
			sealed = false;
			hasBounds = false;
			cachedBounds = { 0, 0, 0, 0 };
			d2dGeometry.Reset();
			d2dGeometryDirty = false;
		}
	};

	// ----------------------------------------------------------------
	// 由 UIRender.cpp 实现：把 CRenderPathImpl 中的命令序列写入 ID2D1PathGeometry
	// 并缓存到 impl->d2dGeometry。如果已有有效缓存且未 dirty 则直接复用。
	//
	// 失败时返回 false，d2dGeometry 留空。
	// ----------------------------------------------------------------
	bool BuildOrReuseGeometryForRenderPathInternal(CRenderPathImpl* pImpl);

	// ----------------------------------------------------------------
	// 由 UIRender.cpp 实现：从 CRenderPath 的几何包围盒构造 D2DDrawScope 的
	// 绑定矩形（D2DDrawScope 是 UIRender.cpp 文件内私有类，所以由 UIRender.cpp 内调用）。
	// ----------------------------------------------------------------
	RECT ToBindRectFromRenderPathInternal(const CRenderPathImpl* pImpl, float strokeInset);
}
