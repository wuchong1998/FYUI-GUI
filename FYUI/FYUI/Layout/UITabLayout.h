#pragma once
namespace FYUI
{
	class FYUI_API CTabLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CTabLayoutUI)
	public:
		CTabLayoutUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

			bool Add(CControlUI* pControl) override;
			bool AddAt(CControlUI* pControl, int iIndex) override;
			bool Remove(CControlUI* pControl, bool bChildDelayed = true) override;
			bool RemoveAt(int iIndex, bool bChildDelayed = true) override;
			void RemoveAll(bool bChildDelayed = true) override;
		int GetCurSel() const;
		virtual bool SelectItem(int iIndex,bool bIsFocus = true);
		virtual bool SelectItem(CControlUI* pControl,bool bIsFocus = true);

		void SetPos(RECT rc, bool bNeedInvalidate = true);

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		virtual CTabLayoutUI* Clone();
		virtual void CopyData(CTabLayoutUI* pControl) ;

		bool IsContainerControl() const override
		{
			return true;
		}


	protected:
		CControlUI* GetSelectedItemControl() const;
		void UpdateSelectionVisibility(int iSelected, bool bSetFocus);
		int ResolveTabSelectionAfterAdd(CControlUI* pControl, int iIndex) const;
		int ResolveTabSelectionAfterRemoval(int iRemovedIndex) const;
		int m_iCurSel;
	};
}

