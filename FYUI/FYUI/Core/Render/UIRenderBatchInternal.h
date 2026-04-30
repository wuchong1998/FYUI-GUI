#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	class CScopedDirect2DBatchInternal
	{
	public:
		/**
		 * @brief 构造 CScopedDirect2DBatchInternal 对象
		 * @details 用于构造 CScopedDirect2DBatchInternal 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		explicit CScopedDirect2DBatchInternal(CPaintRenderContext& renderContext);
		/**
		 * @brief 析构 CScopedDirect2DBatchInternal 对象
		 * @details 用于析构 CScopedDirect2DBatchInternal 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CScopedDirect2DBatchInternal();

		/**
		 * @brief 判断是否Active
		 * @details 用于判断是否Active。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsActive() const
		{
			return m_bActive;
		}

	private:
		CPaintRenderContext* m_pRenderContext = nullptr;
		bool m_bActive = false;
	};

	/**
	 * @brief 执行 FlushDirect2DBatchForContextInternal 操作
	 * @details 用于执行 FlushDirect2DBatchForContextInternal 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param renderContext [in,out] 绘制上下文
	 */
	void FlushDirect2DBatchForContextInternal(CPaintRenderContext& renderContext);
}
