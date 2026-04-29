#include "pch.h"
#include "UIRenderBatchInternal.h"

namespace FYUI
{
	void BeginDirect2DBatchInternal(CPaintRenderContext& renderContext);
	void EndDirect2DBatchInternal(CPaintRenderContext& renderContext);
	void FlushDirect2DBatchForContextInternal(CPaintRenderContext& renderContext);

	CScopedDirect2DBatchInternal::CScopedDirect2DBatchInternal(CPaintRenderContext& renderContext)
		: m_pRenderContext(&renderContext)
		, m_bActive(renderContext.GetDC() != NULL)
	{
		if (m_bActive) {
			BeginDirect2DBatchInternal(renderContext);
		}
	}

	CScopedDirect2DBatchInternal::~CScopedDirect2DBatchInternal()
	{
		if (m_bActive) {
			EndDirect2DBatchInternal(*m_pRenderContext);
		}
	}
}
