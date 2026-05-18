#pragma once
#ifndef __UIRENDERPATH_H__
#define __UIRENDERPATH_H__

// ============================================================
// CRenderPath — 用于描述任意自由几何路径（直线 / 二/三次贝塞尔 / 椭圆弧 / 闭合）
// 的统一对象。配合 CRenderEngine::StrokePath / FillPath / PushPathClip 使用。
//
// 设计目的：替代业务侧 Gdiplus::GraphicsPath 的"直线 + 弧 + 贝塞尔混合"用法，
//          使所有自由路径一次构建后即可在 D2D 中描边 / 填充 / 裁剪，避免分段绘制
//          带来的端帽接缝瑕疵。
//
// 使用模板：
//      CRenderPath path;
//      path.Begin();
//      path.MoveTo(POINTF{ 10, 10 });
//      path.LineTo(POINTF{ 50, 10 });
//      path.BezierTo(POINTF{ 70, 10 }, POINTF{ 70, 50 }, POINTF{ 70, 70 });
//      path.Close();
//      path.End();
//      CRenderEngine::StrokePath(renderContext, path, 2.0f, 0xFF222222);
//
// 当前实现说明（2026-05-18 Step A）：
//   - 头文件已落地完整公开签名；
//   - 内部仅缓存几何描述，不真实构建 D2D PathGeometry；
//   - StrokePath / FillPath / PushPathClip / PopPathClip 暂为 no-op，
//     待 Step B / Step C 落实 D2D 后再切换。
//   - 业务调用方可以正常引用本头并通过链接，但绘制结果暂为空。
// ============================================================

#include <windows.h>

#include "../UIDefine.h"  // RECTF / POINTF / FYUI 命名空间

namespace FYUI
{
	class CRenderPathImpl;  // PIMPL，定义于 UIRenderPath.cpp

	class FYUI_API CRenderPath
	{
	public:
		/**
		 * @brief 构造空路径
		 */
		CRenderPath();
		/**
		 * @brief 析构路径，释放内部 PathGeometry 资源
		 */
		~CRenderPath();

		CRenderPath(const CRenderPath&) = delete;
		CRenderPath& operator=(const CRenderPath&) = delete;

		CRenderPath(CRenderPath&& other) noexcept;
		CRenderPath& operator=(CRenderPath&& other) noexcept;

		// ----------------------------------------------------------------
		// 路径构建：必须先 Begin()，再依次 MoveTo / LineTo / BezierTo / ...，
		// 最后 End()。Begin/End 之间禁止重复构建；若需重新构建，先 Reset()。
		// ----------------------------------------------------------------

		/**
		 * @brief 开始构建一条新路径，丢弃之前的内容
		 */
		void Begin();

		/**
		 * @brief 移动到子路径的起点（隐式开启一段新子路径）
		 */
		void MoveTo(const POINT& pt);
		void MoveTo(const POINTF& pt);

		/**
		 * @brief 从当前点画一条直线到目标点
		 */
		void LineTo(const POINT& pt);
		void LineTo(const POINTF& pt);

		/**
		 * @brief 三次贝塞尔曲线（两个控制点）
		 */
		void BezierTo(const POINT& c1, const POINT& c2, const POINT& pt);
		void BezierTo(const POINTF& c1, const POINTF& c2, const POINTF& pt);

		/**
		 * @brief 二次贝塞尔曲线（一个控制点）
		 */
		void QuadraticTo(const POINT& c, const POINT& pt);
		void QuadraticTo(const POINTF& c, const POINTF& pt);

		/**
		 * @brief 椭圆弧（同 SVG path "A" 命令）
		 * @param endPt          弧终点
		 * @param radius         椭圆 X / Y 半径（像素）
		 * @param rotationAngle  椭圆相对水平方向的旋转角度（度）
		 * @param largeArc       true 为大弧、false 为小弧
		 * @param sweepClockwise true 顺时针、false 逆时针
		 */
		void ArcTo(const POINTF& endPt, SIZE radius, float rotationAngle,
			bool largeArc, bool sweepClockwise);

		/**
		 * @brief 闭合当前子路径（在终点和起点之间补一条直线段）
		 */
		void Close();

		/**
		 * @brief 完成路径构建。之后路径只读，可被 CRenderEngine 使用。
		 */
		void End();

		/**
		 * @brief 重置路径到空状态。等价于"丢弃 + 等待下一次 Begin"。
		 */
		void Reset();

		// ----------------------------------------------------------------
		// 查询
		// ----------------------------------------------------------------

		/**
		 * @brief 路径是否为空（未 Begin 或仅 Begin 未追加任何段）
		 */
		bool IsEmpty() const;

		/**
		 * @brief 返回路径轴对齐包围盒（仅在 End() 后有效）
		 */
		RECT  GetBoundsRect() const;
		RECTF GetBoundsRectF() const;

		// ----------------------------------------------------------------
		// 内部接口（仅供 CRenderEngine 调用）
		// ----------------------------------------------------------------

		/**
		 * @brief 仅供 FYUI 内部使用：取出 PIMPL 指针。
		 *        外部代码不应使用，签名可能在后续版本变化。
		 */
		CRenderPathImpl* GetImpl() const;

	private:
		CRenderPathImpl* m_pImpl = nullptr;
	};
}

#endif // __UIRENDERPATH_H__
