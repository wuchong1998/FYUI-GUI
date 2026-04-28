#pragma once

#include <string>
#include <string_view>

namespace FYUI {

	class IDialogBuilderCallback
	{
	public:
		virtual CControlUI* CreateControl(std::wstring_view pstrClass) = 0;
	};


	class FYUI_API CDialogBuilder
	{
	public:
		CDialogBuilder();
		CControlUI* Create(STRINGorID xml, std::wstring_view type = {}, IDialogBuilderCallback* pCallback = NULL,
			CPaintManagerUI* pManager = NULL, CControlUI* pParent = NULL);
		CControlUI* Create(IDialogBuilderCallback* pCallback = NULL, CPaintManagerUI* pManager = NULL,
			CControlUI* pParent = NULL);

		CMarkup* GetMarkup();

		void GetLastErrorMessage(wchar_t* pstrMessage, SIZE_T cchMax) const;
		void GetLastErrorLocation(wchar_t* pstrSource, SIZE_T cchMax) const;
		void SetInstance(HINSTANCE instance){ m_instance = instance;};
	private:
		CControlUI* _Parse(CMarkupNode* parent, CControlUI* pParent = NULL, CPaintManagerUI* pManager = NULL);

		CMarkup m_xml;
		IDialogBuilderCallback* m_pCallback;
		std::wstring m_typeStorage;
		const wchar_t* m_pstrtype;
		HINSTANCE m_instance;
	};

} 


