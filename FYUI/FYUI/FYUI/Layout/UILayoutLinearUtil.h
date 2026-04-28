#pragma once

namespace FYUI
{
	enum class LinearLayoutAxis
	{
		Vertical,
		Horizontal,
	};

	struct LinearLayoutInfo
	{
		CControlUI* pControl;
		SIZE sz;
		RECT rcPadding;
		bool bVisible;
		bool bFloat;
		int iControlMaxWidth;
		int iControlMaxHeight;

		LinearLayoutInfo()
			: pControl(nullptr),
			sz{ 0, 0 },
			rcPadding{ 0, 0, 0, 0 },
			bVisible(false),
			bFloat(false),
			iControlMaxWidth(0),
			iControlMaxHeight(0)
		{
		}
	};

	inline void ClampLinearLayoutAvailableSize(SIZE& szAvailable, const LinearLayoutInfo& info)
	{
		if (szAvailable.cx > info.iControlMaxWidth) szAvailable.cx = info.iControlMaxWidth;
		if (szAvailable.cy > info.iControlMaxHeight) szAvailable.cy = info.iControlMaxHeight;
	}

	inline void ClampLinearMeasuredSize(CControlUI* pControl, SIZE& sz, LinearLayoutAxis axis, int& nAdjustables)
	{
		LONG& iPrimarySize = (axis == LinearLayoutAxis::Vertical) ? sz.cy : sz.cx;
		if (iPrimarySize == 0) {
			nAdjustables++;
		}
		else if (axis == LinearLayoutAxis::Vertical) {
			if (iPrimarySize < pControl->GetMinHeight()) iPrimarySize = pControl->GetMinHeight();
			if (iPrimarySize > pControl->GetMaxHeight()) iPrimarySize = pControl->GetMaxHeight();
		}
		else {
			if (iPrimarySize < pControl->GetMinWidth()) iPrimarySize = pControl->GetMinWidth();
			if (iPrimarySize > pControl->GetMaxWidth()) iPrimarySize = pControl->GetMaxWidth();
		}

		LONG& iSecondarySize = (axis == LinearLayoutAxis::Vertical) ? sz.cx : sz.cy;
		iSecondarySize = MAX(iSecondarySize, 0);
		if (axis == LinearLayoutAxis::Vertical) {
			if (iSecondarySize < pControl->GetMinWidth()) iSecondarySize = pControl->GetMinWidth();
			if (iSecondarySize > pControl->GetMaxWidth()) iSecondarySize = pControl->GetMaxWidth();
		}
		else {
			if (iSecondarySize < pControl->GetMinHeight()) iSecondarySize = pControl->GetMinHeight();
			if (iSecondarySize > pControl->GetMaxHeight()) iSecondarySize = pControl->GetMaxHeight();
		}
	}

	inline void CollectLinearLayoutInfo(CControlUI* pControl, const SIZE& szAvailable, LinearLayoutAxis axis,
		LinearLayoutInfo& info, int& iPrimaryFixed, int& iSecondaryNeeded, int& nAdjustables, int& nEstimateNum)
	{
		info.pControl = pControl;
		info.bVisible = pControl->IsVisible();
		info.bFloat = pControl->IsFloat();
		if (!info.bVisible || info.bFloat) return;

		info.rcPadding = pControl->GetPadding();
		SIZE szControlAvailable = szAvailable;
		if (axis == LinearLayoutAxis::Vertical) {
			szControlAvailable.cx -= info.rcPadding.left + info.rcPadding.right;
		}
		else {
			szControlAvailable.cy -= info.rcPadding.top + info.rcPadding.bottom;
		}

		info.iControlMaxWidth = pControl->GetFixedWidth();
		info.iControlMaxHeight = pControl->GetFixedHeight();
		if (info.iControlMaxWidth <= 0) info.iControlMaxWidth = pControl->GetMaxWidth();
		if (info.iControlMaxHeight <= 0) info.iControlMaxHeight = pControl->GetMaxHeight();

		ClampLinearLayoutAvailableSize(szControlAvailable, info);
		info.sz = pControl->EstimateSize(szControlAvailable);
		ClampLinearMeasuredSize(pControl, info.sz, axis, nAdjustables);

		if (axis == LinearLayoutAxis::Vertical) {
			iPrimaryFixed += info.sz.cy + info.rcPadding.top + info.rcPadding.bottom;
			iSecondaryNeeded = MAX(iSecondaryNeeded, info.sz.cx + info.rcPadding.left + info.rcPadding.right);
		}
		else {
			iPrimaryFixed += info.sz.cx + info.rcPadding.left + info.rcPadding.right;
			iSecondaryNeeded = MAX(iSecondaryNeeded, info.sz.cy + info.rcPadding.top + info.rcPadding.bottom);
		}

		nEstimateNum++;
	}
}
