#pragma once

namespace FYUI
{
	class FYUI_API CTileLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CTileLayoutUI)
	public:
		CTileLayoutUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		void SetPos(RECT rc, bool bNeedInvalidate = true);

		SIZE GetItemSize() const;
		void SetItemSize(SIZE szItem);
		int GetColumns() const;
		void SetColumns(int nCols);
		void SetAutoSize(bool bAutoSize);
		void SetHorSpacing(int nSpacing);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

	public:
		virtual CTileLayoutUI* Clone();
		virtual void CopyData(CTileLayoutUI* pControl) ;
		bool IsContainerControl() const override
		{
			return true;
		}

	protected:
		bool m_bAutoSize = false;
		bool m_bFixedColumns = false;
		SIZE m_szItem;
		int m_nColumns;
		int m_nHorSpacing = 20;
	};
}

