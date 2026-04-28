#pragma once

#include "UIRenderContext.h"

namespace FYUI
{
	class CScopedDirect2DBatchInternal
	{
	public:
		explicit CScopedDirect2DBatchInternal(CPaintRenderContext& renderContext);
		~CScopedDirect2DBatchInternal();

		bool IsActive() const
		{
			return m_bActive;
		}

	private:
		CPaintRenderContext* m_pRenderContext = nullptr;
		bool m_bActive = false;
	};

	void FlushDirect2DBatchForContextInternal(CPaintRenderContext& renderContext);
}
