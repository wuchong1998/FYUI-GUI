#include "pch.h"
#include "UITileLayout.h"
#include "UILayoutContentUtil.h"

namespace FYUI
{
	IMPLEMENT_DUICONTROL(CTileLayoutUI)
	CTileLayoutUI::CTileLayoutUI() : m_nColumns(1)
	{
		m_szItem.cx = m_szItem.cy = 0;
	}

	std::wstring_view CTileLayoutUI::GetClass() const
	{
		return _T("TileLayoutUI");
	}

	LPVOID CTileLayoutUI::GetInterface(std::wstring_view pstrName)
	{
		if( StringUtil::CompareNoCase(pstrName, DUI_CTR_TILELAYOUT) == 0 ) return static_cast<CTileLayoutUI*>(this);
		return CContainerUI::GetInterface(pstrName);
	}

	SIZE CTileLayoutUI::GetItemSize() const
	{
		if( m_pManager != NULL ) return m_pManager->ScaleSize(m_szItem);
		return m_szItem;
	}

	void CTileLayoutUI::SetItemSize(SIZE szItem)
	{
		if( m_szItem.cx != szItem.cx || m_szItem.cy != szItem.cy ) {
			m_szItem = szItem;
			NeedUpdate();
		}
	}

	int CTileLayoutUI::GetColumns() const
	{
		return m_nColumns;
	}

	void CTileLayoutUI::SetColumns(int nCols)
	{
		if( nCols <= 0 ) {
			if( m_bFixedColumns ) {
				m_bFixedColumns = false;
				NeedUpdate();
			}
			return;
		}

		m_bFixedColumns = true;
		m_nColumns = nCols;
		NeedUpdate();
	}

	void CTileLayoutUI::SetAutoSize(bool bAutoSize)
	{
		if( m_bAutoSize != bAutoSize ) {
			m_bAutoSize = bAutoSize;
			NeedUpdate();
		}
	}

	void CTileLayoutUI::SetHorSpacing(int nSpacing)
	{
		m_nHorSpacing = nSpacing;
		NeedUpdate();
	}

	int CTileLayoutUI::GetVerSpacing() const
	{
		return m_nVerSpacing;
	}

	void CTileLayoutUI::SetVerSpacing(int nSpacing)
	{
		if( nSpacing < 0 ) nSpacing = -1;
		if( m_nVerSpacing != nSpacing ) {
			m_nVerSpacing = nSpacing;
			NeedUpdate();
		}
	}

	void CTileLayoutUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
	{
		if( StringUtil::CompareNoCase(pstrName, _T("itemsize")) == 0 ) {
			SIZE szItem = { 0 };
			if (StringUtil::TryParseSize(pstrValue, szItem)) {
				SetItemSize(szItem);
			}
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("columns")) == 0 ) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetColumns(value);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("hor_spacing")) == 0 ) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetHorSpacing(value);
		}
		else if( StringUtil::CompareNoCase(pstrName, _T("ver_spacing")) == 0 ) {
			int value = 0;
			if (StringUtil::TryParseInt(pstrValue, value)) SetVerSpacing(value);
		}
		else CContainerUI::SetAttribute(pstrName, pstrValue);
	}

	CTileLayoutUI* CTileLayoutUI::Clone()
	{
		CTileLayoutUI* pClone = new CTileLayoutUI();
		pClone->CopyData(this);
		return pClone;
	}

	void CTileLayoutUI::CopyData(CTileLayoutUI* pControl)
	{
		m_bAutoSize = pControl->m_bAutoSize;
		m_bFixedColumns = pControl->m_bFixedColumns;
		m_szItem = pControl->m_szItem;
		m_nColumns = pControl->m_nColumns;
		m_nHorSpacing = pControl->m_nHorSpacing;
		m_nVerSpacing = pControl->m_nVerSpacing;
		__super::CopyData(pControl);
	}

	void CTileLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		// 布局规则（重写后）：
		//  1. 子控件占位严格等于 itemsize（DPI 缩放后），不再读取子控件 FixedWidth/Height。
		//  2. hor_spacing 为水平间距的"最小值"（DPI 缩放后）；剩余可显示宽度均分到所有间隙，
		//     实际间距 >= hor_spacing。
		//  3. 容器宽度变化时子控件尺寸不变、间距动态扩大，且 N 列总占位永远 <= 可显示宽度。
		//  4. 行间距：ver_spacing 显式设置时使用其 DPI 缩放后值；未设置时回退到 GetChildPadding。
		//  5. 极窄场景：N=1 且 itemW 仍超过可显示宽度时，将子项宽压到可显示宽度，保证不裁切。

		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = ApplyLayoutInsetRect(m_rcItem, GetInset());

		if( m_items.GetSize() == 0 ) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		rc = AdjustLayoutRectForVisibleScrollBars(rc, m_pVerticalScrollBar, m_pHorizontalScrollBar, m_bScrollFloat);

		// 子项占位尺寸：itemsize 优先（已 DPI 适配），否则取首个可见非浮动子项的 EstimateSize 兜底
		SIZE szItem = GetItemSize();
		int itemW = szItem.cx;
		int itemH = szItem.cy;
		if( itemW <= 0 || itemH <= 0 ) {
			SIZE szFallbackAvailable = { rc.right - rc.left, rc.bottom - rc.top };
			for( int it = 0; it < m_items.GetSize(); ++it ) {
				CControlUI* p = static_cast<CControlUI*>(m_items[it]);
				if( !p->IsVisible() || p->IsFloat() ) continue;
				SIZE szEst = p->EstimateSize(szFallbackAvailable);
				if( itemW <= 0 && szEst.cx > 0 ) itemW = szEst.cx;
				if( itemH <= 0 && szEst.cy > 0 ) itemH = szEst.cy;
				break;
			}
		}
		if( itemW <= 0 ) itemW = 1;
		if( itemH <= 0 ) itemH = 1;

		const int minSpacing = m_pManager != NULL ? m_pManager->ScaleValue(m_nHorSpacing) : m_nHorSpacing;
		// 行间距：ver_spacing 显式设置时优先使用（DPI 缩放后），否则回退到 childpadding 兼容旧配置
		const int rowSpacing = (m_nVerSpacing >= 0)
			? (m_pManager != NULL ? m_pManager->ScaleValue(m_nVerSpacing) : m_nVerSpacing)
			: GetChildPadding();
		const int cxAvailable = MAX(rc.right - rc.left, 0);

		// 计算列数 N：N * itemW + (N-1) * minSpacing <= cxAvailable
		int nColumns = 1;
		if( m_bFixedColumns ) {
			nColumns = MAX(m_nColumns, 1);
		}
		else {
			const int denom = itemW + minSpacing;
			if( denom > 0 ) {
				nColumns = (cxAvailable + minSpacing) / denom;
			}
			if( nColumns < 1 ) nColumns = 1;
		}

		// 极窄兜底：单列且 itemW 仍超过可显示宽 → 压缩子项宽到 cxAvailable
		int effectiveItemW = itemW;
		if( nColumns == 1 && effectiveItemW > cxAvailable && cxAvailable > 0 ) {
			effectiveItemW = cxAvailable;
		}

		// 实际水平间距：均分剩余空间，但不小于 minSpacing
		int actualSpacing = minSpacing;
		if( nColumns >= 2 ) {
			const int totalItemW = nColumns * effectiveItemW;
			const int remaining = cxAvailable - totalItemW;
			if( remaining > 0 ) {
				actualSpacing = remaining / (nColumns - 1);
				if( actualSpacing < minSpacing ) actualSpacing = minSpacing;
			}
		}

		POINT ptTile = { rc.left, rc.top };
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			ptTile.y -= m_pVerticalScrollBar->GetScrollPos();
		}
		const int xStart = ptTile.x;

		int visibleCount = 0;
		int iCount = 0;
		for( int it = 0; it < m_items.GetSize(); ++it ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it);
				continue;
			}

			RECT rcPos = {
				ptTile.x,
				ptTile.y,
				ptTile.x + effectiveItemW,
				ptTile.y + itemH
			};
			pControl->SetPos(rcPos);

			++visibleCount;
			if( (++iCount % nColumns) == 0 ) {
				ptTile.x = xStart;
				ptTile.y += itemH + rowSpacing;
			}
			else {
				ptTile.x += effectiveItemW + actualSpacing;
			}
		}

		// 总内容高度（供 vscrollbar range 使用）
		int cyNeeded = 0;
		if( visibleCount > 0 ) {
			const int nRows = (visibleCount + nColumns - 1) / nColumns;
			cyNeeded = nRows * itemH + (nRows > 1 ? (nRows - 1) * rowSpacing : 0);
		}

		ProcessScrollBar(rc, 0, cyNeeded);
	}
}
