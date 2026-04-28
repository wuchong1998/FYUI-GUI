
#pragma once

namespace FYUI
{
	class FYUI_API CChildLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CChildLayoutUI)
	public:
		CChildLayoutUI();

		void Init();
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void SetChildLayoutXML(std::wstring_view pXML);
		std::wstring GetChildLayoutXML();
		virtual LPVOID GetInterface(std::wstring_view pstrName);
		virtual std::wstring_view GetClass() const;
		virtual CChildLayoutUI* Clone();
		virtual void CopyData(CChildLayoutUI* pControl) ;
		bool IsContainerControl() const override
		{
			return true;
		}
	private:
			bool HasChildLayoutSource() const;
			bool HasLoadedChildLayout() const;
			bool ReloadChildLayout();
			void ClearChildLayoutContainer();
			CContainerUI* CreateChildLayoutContainer() const;
			void ApplyChildLayoutContainer(CContainerUI* pChildWindow);
		std::wstring m_pstrXMLFile;
	};
} 


