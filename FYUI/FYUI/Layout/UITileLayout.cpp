#include "pch.h"
#include "UITileLayout.h"
#include "UILayoutContentUtil.h"

namespace FYUI
{
	namespace
	{
		SIZE ClampTileMeasuredSize(CControlUI* pControl, SIZE szTile)
		{
			if( szTile.cx < pControl->GetMinWidth() ) szTile.cx = pControl->GetMinWidth();
			if( szTile.cx > pControl->GetMaxWidth() ) szTile.cx = pControl->GetMaxWidth();
			if( szTile.cy < pControl->GetMinHeight() ) szTile.cy = pControl->GetMinHeight();
			if( szTile.cy > pControl->GetMaxHeight() ) szTile.cy = pControl->GetMaxHeight();
			return szTile;
		}

		int CalculateTileColumnWidth(const RECT& rc, int nColumns, int nHorSpacing, bool bHasHorizontalScroll, int iHorizontalScrollRange)
		{
			const int cxVisibleWidth = rc.right - rc.left;
			const int iSpacingWidth = (nColumns > 1) ? (nColumns - 1) * nHorSpacing : 0;
			int cxAvailableWidth = cxVisibleWidth - iSpacingWidth;
			if( bHasHorizontalScroll ) {
				cxAvailableWidth = cxVisibleWidth + iHorizontalScrollRange - iSpacingWidth;
			}
			if( cxAvailableWidth < 0 ) cxAvailableWidth = 0;
			return nColumns > 0 ? cxAvailableWidth / nColumns : 0;
		}

		int ResolveTileColumnCount(const RECT& rc, int nConfiguredColumns, bool bFixedColumns, const SIZE& szItem, int nHorSpacing)
		{
			if( bFixedColumns ) return max(nConfiguredColumns, 1);

			if( szItem.cx <= 0 ) return max(nConfiguredColumns, 1);

			const int cxStep = szItem.cx + nHorSpacing;
			if( cxStep <= 0 ) return max(nConfiguredColumns, 1);

			const int cxLayout = rc.right - rc.left;
			const int nResolvedColumns = cxLayout / cxStep;
			return max(nResolvedColumns, 1);
		}

		int MeasureTileRowHeight(const CStdPtrArray& items, int iStartIndex, int iVisibleIndex, int nColumns, const RECT& rcTile, int iChildPadding, CControlUI* pReferenceControl)
		{
			int cyHeight = 0;
			int iIndex = iVisibleIndex;
			for( int it = iStartIndex; it < items.GetSize(); ++it ) {
				CControlUI* pLineControl = static_cast<CControlUI*>(items[it]);
				if( !pLineControl->IsVisible() ) continue;
				if( pLineControl->IsFloat() ) continue;

				RECT rcPadding = pLineControl->GetPadding();
				SIZE szAvailable = { rcTile.right - rcTile.left - rcPadding.left - rcPadding.right, 9999999 };
				if( iIndex == iVisibleIndex || ((iIndex + 1) % nColumns) == 0 ) {
					szAvailable.cx -= iChildPadding / 2;
				}
				else {
					szAvailable.cx -= iChildPadding;
				}

				if( szAvailable.cx < pReferenceControl->GetMinWidth() ) szAvailable.cx = pReferenceControl->GetMinWidth();
				if( szAvailable.cx > pReferenceControl->GetMaxWidth() ) szAvailable.cx = pReferenceControl->GetMaxWidth();

				SIZE szTile = pLineControl->EstimateSize(szAvailable);
				if( szTile.cx < pReferenceControl->GetMinWidth() ) szTile.cx = pReferenceControl->GetMinWidth();
				if( szTile.cx > pReferenceControl->GetMaxWidth() ) szTile.cx = pReferenceControl->GetMaxWidth();
				if( szTile.cy < pReferenceControl->GetMinHeight() ) szTile.cy = pReferenceControl->GetMinHeight();
				if( szTile.cy > pReferenceControl->GetMaxHeight() ) szTile.cy = pReferenceControl->GetMaxHeight();

				cyHeight = MAX(cyHeight, szTile.cy + rcPadding.top + rcPadding.bottom);
				if( (++iIndex % nColumns) == 0 ) break;
			}
			return cyHeight;
		}

		RECT BuildTileItemRect(const RECT& rcTile, const RECT& rcPadding, int iChildPadding, int iVisibleIndex, int nColumns, int cyHeight)
		{
			RECT rcContent = rcTile;
			rcContent.left += rcPadding.left + iChildPadding / 2;
			rcContent.right -= rcPadding.right + iChildPadding / 2;
			if( (iVisibleIndex % nColumns) == 0 ) {
				rcContent.left -= iChildPadding / 2;
			}
			if( ((iVisibleIndex + 1) % nColumns) == 0 ) {
				rcContent.right += iChildPadding / 2;
			}
			rcContent.top += rcPadding.top;
			rcContent.bottom = rcTile.top + cyHeight;
			return rcContent;
		}
	}

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
		__super::CopyData(pControl);
	}

	void CTileLayoutUI::SetPos(RECT rc, bool bNeedInvalidate)
	{
		CControlUI::SetPos(rc, bNeedInvalidate);
		rc = ApplyLayoutInsetRect(m_rcItem, GetInset());

		if( m_items.GetSize() == 0 ) {
			ProcessScrollBar(rc, 0, 0);
			return;
		}

		rc = AdjustLayoutRectForVisibleScrollBars(rc, m_pVerticalScrollBar, m_pHorizontalScrollBar);

		const SIZE szItem = GetItemSize();
		const int nHorSpacing = m_pManager != NULL ? m_pManager->ScaleValue(m_nHorSpacing) : m_nHorSpacing;
		const int nColumns = ResolveTileColumnCount(rc, m_nColumns, m_bFixedColumns, szItem, nHorSpacing);

		const int cxWidth = CalculateTileColumnWidth(rc, nColumns, nHorSpacing,
			m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible(),
			m_pHorizontalScrollBar ? m_pHorizontalScrollBar->GetScrollRange() : 0);

		int cyNeeded = 0;
		int cyHeight = 0;
		int iCount = 0;
		POINT ptTile = { rc.left, rc.top };
		if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
			ptTile.y -= m_pVerticalScrollBar->GetScrollPos();
		}
		int iPosX = rc.left;
		if( m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible() ) {
			iPosX -= m_pHorizontalScrollBar->GetScrollPos();
			ptTile.x = iPosX;
		}

		for( int it = 0; it < m_items.GetSize(); ++it ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_items[it]);
			if( !pControl->IsVisible() ) continue;
			if( pControl->IsFloat() ) {
				SetFloatPos(it);
				continue;
			}

			RECT rcTile = { ptTile.x, ptTile.y, ptTile.x + cxWidth, ptTile.y };
			if( (iCount % nColumns) == 0 ) {
				cyHeight = MeasureTileRowHeight(m_items, it, iCount, nColumns, rcTile, m_iChildPadding, pControl);
				if( szItem.cy > 0 ) cyHeight = MAX(cyHeight, szItem.cy);
			}

			const RECT rcPadding = pControl->GetPadding();
			const RECT rcContent = BuildTileItemRect(rcTile, rcPadding, m_iChildPadding, iCount, nColumns, cyHeight);
			SIZE szAvailable = { rcContent.right - rcContent.left, rcContent.bottom - rcContent.top };
			SIZE szMeasured = pControl->EstimateSize(szAvailable);
			if( szMeasured.cx == 0 ) szMeasured.cx = szAvailable.cx;
			if( szMeasured.cy == 0 ) szMeasured.cy = szAvailable.cy;
			szMeasured = ClampTileMeasuredSize(pControl, szMeasured);

			RECT rcPos = {
				(rcContent.left + rcContent.right - szMeasured.cx) / 2,
				(rcContent.top + rcContent.bottom - szMeasured.cy) / 2,
				(rcContent.left + rcContent.right - szMeasured.cx) / 2 + szMeasured.cx,
				(rcContent.top + rcContent.bottom - szMeasured.cy) / 2 + szMeasured.cy
			};
			pControl->SetPos(rcPos);

			if( (++iCount % nColumns) == 0 ) {
				ptTile.x = iPosX;
				ptTile.y += cyHeight + m_iChildPadding;
				cyHeight = 0;
			}
			else {
				ptTile.x += cxWidth + nHorSpacing;
			}

			cyNeeded = rcContent.bottom - rc.top;
			if( m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible() ) {
				cyNeeded += m_pVerticalScrollBar->GetScrollPos();
			}
		}

		ProcessScrollBar(rc, 0, cyNeeded);
	}
}
