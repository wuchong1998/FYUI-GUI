#pragma once
#include <map>
namespace FYUI 
{
	typedef CControlUI* (*CreateClass)();
	typedef std::map<std::wstring, CreateClass> MAP_DUI_CTRATECLASS;

	class FYUI_API CControlFactory
	{
	public:
		CControlUI* CreateControl(std::wstring strClassName);
		void RegistControl(std::wstring strClassName, CreateClass pFunc);

		static CControlFactory* GetInstance();
		void Release();

	private:	
		CControlFactory();
		virtual ~CControlFactory();

	private:
		MAP_DUI_CTRATECLASS m_mapControl;
	};

#define DECLARE_DUICONTROL(class_name)\
public:\
	static CControlUI* CreateControl();

#define IMPLEMENT_DUICONTROL(class_name)\
	CControlUI* class_name::CreateControl()\
	{\
		return new class_name;\
	}

#define REGIST_DUICONTROL(class_name)\
	CControlFactory::GetInstance()->RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);

#define INNER_REGISTER_DUICONTROL(class_name)\
	RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);


#define INNER_REGISTER_DUICONTROL_EX(str_class_name,class_type)\
	{\
		std::wstring sRegClassName;\
		sRegClassName = StringUtil::Format(L"C{}UI", str_class_name); \
		RegistControl(sRegClassName, (CreateClass)class_type::CreateControl);\
    }
}
