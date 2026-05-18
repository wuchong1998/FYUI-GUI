// ============================================================
// UIRenderPath.cpp — CRenderPath 路径对象 + Matrix2D 算子的实现
//
// 设计分层（2026-05-18 Step B+C+D）：
//   - 本文件：CRenderPath 公开成员函数 + Matrix2D 几何算子；
//             仅维护命令序列，不直接调用 D2D。
//   - UIRender.cpp：CRenderEngine::StrokePath / FillPath / PushPathClip /
//             PopPathClip / PushTransform / PopTransform 的真实 D2D 实现。
//             同时定义 BuildOrReuseGeometryForRenderPathInternal —— 把
//             本文件里收集的命令序列翻译成 ID2D1PathGeometry。
//   - UIRenderPathInternal.h：CRenderPathImpl 完整定义对二者可见。
// ============================================================

#include "pch.h"

#include "UIRenderPathInternal.h"

#include <algorithm>
#include <cmath>

#include "../UIRender.h"

namespace FYUI
{
	// ------------------------------------------------------------------
	// 工具：POINT → POINTF；包围盒重算
	// ------------------------------------------------------------------
	namespace
	{
		POINTF ToPOINTF(const POINT& p)
		{
			return POINTF{ static_cast<float>(p.x), static_cast<float>(p.y) };
		}

		// 简单的轴对齐包围盒（不考虑贝塞尔实际曲率，使用所有控制/端点的外框）
		void RecomputeBoundsForImpl(CRenderPathImpl* pImpl)
		{
			if (pImpl == nullptr) {
				return;
			}
			if (pImpl->commands.empty()) {
				pImpl->cachedBounds = { 0, 0, 0, 0 };
				pImpl->hasBounds = false;
				return;
			}

			float minX = 0, minY = 0, maxX = 0, maxY = 0;
			bool first = true;

			auto consume = [&](const POINTF& pt) {
				if (first) {
					minX = maxX = pt.X;
					minY = maxY = pt.Y;
					first = false;
				}
				else {
					minX = (std::min)(minX, pt.X);
					minY = (std::min)(minY, pt.Y);
					maxX = (std::max)(maxX, pt.X);
					maxY = (std::max)(maxY, pt.Y);
				}
			};

			for (const CRenderPathImpl::Command& c : pImpl->commands) {
				switch (c.type) {
				case CRenderPathImpl::CommandType::MoveTo:
				case CRenderPathImpl::CommandType::LineTo:
				case CRenderPathImpl::CommandType::ArcTo:
					consume(c.p0);
					break;
				case CRenderPathImpl::CommandType::BezierTo:
					consume(c.p1);
					consume(c.p2);
					consume(c.p0);
					break;
				case CRenderPathImpl::CommandType::QuadraticTo:
					consume(c.p1);
					consume(c.p0);
					break;
				case CRenderPathImpl::CommandType::Close:
					break;
				}
			}

			if (first) {
				pImpl->cachedBounds = { 0, 0, 0, 0 };
				pImpl->hasBounds = false;
			}
			else {
				pImpl->cachedBounds = { minX, minY, maxX, maxY };
				pImpl->hasBounds = true;
			}
		}
	}

	// ==================================================================
	// CRenderPath 公开实现
	// ==================================================================
	CRenderPath::CRenderPath()
		: m_pImpl(new CRenderPathImpl())
	{
	}

	CRenderPath::~CRenderPath()
	{
		delete m_pImpl;
		m_pImpl = nullptr;
	}

	CRenderPath::CRenderPath(CRenderPath&& other) noexcept
		: m_pImpl(other.m_pImpl)
	{
		other.m_pImpl = nullptr;
	}

	CRenderPath& CRenderPath::operator=(CRenderPath&& other) noexcept
	{
		if (this != &other) {
			delete m_pImpl;
			m_pImpl = other.m_pImpl;
			other.m_pImpl = nullptr;
		}
		return *this;
	}

	void CRenderPath::Begin()
	{
		if (m_pImpl == nullptr) {
			return;
		}
		m_pImpl->Reset();
		m_pImpl->building = true;
	}

	void CRenderPath::MoveTo(const POINT& pt)
	{
		MoveTo(ToPOINTF(pt));
	}

	void CRenderPath::MoveTo(const POINTF& pt)
	{
		if (m_pImpl == nullptr || !m_pImpl->building) {
			return;
		}
		CRenderPathImpl::Command c;
		c.type = CRenderPathImpl::CommandType::MoveTo;
		c.p0 = pt;
		m_pImpl->commands.push_back(c);
	}

	void CRenderPath::LineTo(const POINT& pt)
	{
		LineTo(ToPOINTF(pt));
	}

	void CRenderPath::LineTo(const POINTF& pt)
	{
		if (m_pImpl == nullptr || !m_pImpl->building) {
			return;
		}
		CRenderPathImpl::Command c;
		c.type = CRenderPathImpl::CommandType::LineTo;
		c.p0 = pt;
		m_pImpl->commands.push_back(c);
	}

	void CRenderPath::BezierTo(const POINT& c1, const POINT& c2, const POINT& pt)
	{
		BezierTo(ToPOINTF(c1), ToPOINTF(c2), ToPOINTF(pt));
	}

	void CRenderPath::BezierTo(const POINTF& c1, const POINTF& c2, const POINTF& pt)
	{
		if (m_pImpl == nullptr || !m_pImpl->building) {
			return;
		}
		CRenderPathImpl::Command c;
		c.type = CRenderPathImpl::CommandType::BezierTo;
		c.p0 = pt;
		c.p1 = c1;
		c.p2 = c2;
		m_pImpl->commands.push_back(c);
	}

	void CRenderPath::QuadraticTo(const POINT& c, const POINT& pt)
	{
		QuadraticTo(ToPOINTF(c), ToPOINTF(pt));
	}

	void CRenderPath::QuadraticTo(const POINTF& c, const POINTF& pt)
	{
		if (m_pImpl == nullptr || !m_pImpl->building) {
			return;
		}
		CRenderPathImpl::Command cmd;
		cmd.type = CRenderPathImpl::CommandType::QuadraticTo;
		cmd.p0 = pt;
		cmd.p1 = c;
		m_pImpl->commands.push_back(cmd);
	}

	void CRenderPath::ArcTo(const POINTF& endPt, SIZE radius, float rotationAngle,
		bool largeArc, bool sweepClockwise)
	{
		if (m_pImpl == nullptr || !m_pImpl->building) {
			return;
		}
		CRenderPathImpl::Command c;
		c.type = CRenderPathImpl::CommandType::ArcTo;
		c.p0 = endPt;
		c.arc.radiusX = static_cast<float>(radius.cx);
		c.arc.radiusY = static_cast<float>(radius.cy);
		c.arc.rotationAngle = rotationAngle;
		c.arc.largeArc = largeArc;
		c.arc.sweepClockwise = sweepClockwise;
		m_pImpl->commands.push_back(c);
	}

	void CRenderPath::Close()
	{
		if (m_pImpl == nullptr || !m_pImpl->building) {
			return;
		}
		CRenderPathImpl::Command c;
		c.type = CRenderPathImpl::CommandType::Close;
		m_pImpl->commands.push_back(c);
	}

	void CRenderPath::End()
	{
		if (m_pImpl == nullptr || !m_pImpl->building) {
			return;
		}
		m_pImpl->building = false;
		m_pImpl->sealed = true;
		// 任何重新构建意味着旧 D2D 资源失效（StrokePath/FillPath 第一次绘制时再按需构建）
		m_pImpl->d2dGeometry.Reset();
		m_pImpl->d2dGeometryDirty = true;
		RecomputeBoundsForImpl(m_pImpl);
	}

	void CRenderPath::Reset()
	{
		if (m_pImpl == nullptr) {
			return;
		}
		m_pImpl->Reset();
	}

	bool CRenderPath::IsEmpty() const
	{
		if (m_pImpl == nullptr) {
			return true;
		}
		return m_pImpl->commands.empty();
	}

	RECT CRenderPath::GetBoundsRect() const
	{
		RECTF rcF = GetBoundsRectF();
		RECT rc = {};
		rc.left = static_cast<LONG>(::floorf(rcF.left));
		rc.top = static_cast<LONG>(::floorf(rcF.top));
		rc.right = static_cast<LONG>(::ceilf(rcF.right));
		rc.bottom = static_cast<LONG>(::ceilf(rcF.bottom));
		return rc;
	}

	RECTF CRenderPath::GetBoundsRectF() const
	{
		if (m_pImpl == nullptr || !m_pImpl->hasBounds) {
			return RECTF{ 0, 0, 0, 0 };
		}
		return m_pImpl->cachedBounds;
	}

	CRenderPathImpl* CRenderPath::GetImpl() const
	{
		return m_pImpl;
	}

	// ==================================================================
	// Matrix2D 工厂 / 算子（纯几何，无 D2D 依赖）
	// CRenderEngine::StrokePath / FillPath / PushPathClip / PopPathClip
	// CRenderEngine::PushTransform / PopTransform 的真实实现位于 UIRender.cpp
	// （需访问 D2DRenderState、D2DDrawScope、画刷与变换栈缓存）
	// ==================================================================

	CRenderEngine::Matrix2D CRenderEngine::Matrix2D::Identity()
	{
		Matrix2D m;
		return m;
	}

	CRenderEngine::Matrix2D CRenderEngine::Matrix2D::Translation(float dx, float dy)
	{
		Matrix2D m;
		m.dx = dx;
		m.dy = dy;
		return m;
	}

	CRenderEngine::Matrix2D CRenderEngine::Matrix2D::Scale(float sx, float sy, POINTF center)
	{
		Matrix2D m;
		m.m11 = sx;
		m.m22 = sy;
		m.dx = center.X - sx * center.X;
		m.dy = center.Y - sy * center.Y;
		return m;
	}

	CRenderEngine::Matrix2D CRenderEngine::Matrix2D::Rotation(float angleDeg, POINTF center)
	{
		const float kPi = 3.14159265358979323846f;
		float rad = angleDeg * kPi / 180.0f;
		float cs = ::cosf(rad);
		float sn = ::sinf(rad);

		Matrix2D m;
		m.m11 = cs;
		m.m12 = sn;
		m.m21 = -sn;
		m.m22 = cs;
		m.dx = center.X - cs * center.X + sn * center.Y;
		m.dy = center.Y - sn * center.X - cs * center.Y;
		return m;
	}

	CRenderEngine::Matrix2D CRenderEngine::Matrix2D::Skew(float angleX, float angleY, POINTF center)
	{
		const float kPi = 3.14159265358979323846f;
		float tx = ::tanf(angleX * kPi / 180.0f);
		float ty = ::tanf(angleY * kPi / 180.0f);

		Matrix2D m;
		m.m12 = ty;
		m.m21 = tx;
		m.dx = -tx * center.Y;
		m.dy = -ty * center.X;
		return m;
	}

	CRenderEngine::Matrix2D CRenderEngine::Matrix2D::operator*(const CRenderEngine::Matrix2D& rhs) const
	{
		Matrix2D r;
		r.m11 = m11 * rhs.m11 + m12 * rhs.m21;
		r.m12 = m11 * rhs.m12 + m12 * rhs.m22;
		r.m21 = m21 * rhs.m11 + m22 * rhs.m21;
		r.m22 = m21 * rhs.m12 + m22 * rhs.m22;
		r.dx = dx * rhs.m11 + dy * rhs.m21 + rhs.dx;
		r.dy = dx * rhs.m12 + dy * rhs.m22 + rhs.dy;
		return r;
	}

	bool CRenderEngine::Matrix2D::Invert(CRenderEngine::Matrix2D& out) const
	{
		float det = m11 * m22 - m12 * m21;
		if (det == 0.0f) {
			return false;
		}
		float inv = 1.0f / det;
		Matrix2D r;
		r.m11 =  m22 * inv;
		r.m12 = -m12 * inv;
		r.m21 = -m21 * inv;
		r.m22 =  m11 * inv;
		r.dx = (m21 * dy - m22 * dx) * inv;
		r.dy = (m12 * dx - m11 * dy) * inv;
		out = r;
		return true;
	}

	POINTF CRenderEngine::Matrix2D::Transform(POINTF pt) const
	{
		POINTF r;
		r.X = pt.X * m11 + pt.Y * m21 + dx;
		r.Y = pt.X * m12 + pt.Y * m22 + dy;
		return r;
	}
}
