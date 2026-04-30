#pragma once
#include <map>
namespace FYUI 
{
	typedef CControlUI* (*CreateClass)();
	typedef std::map<std::wstring, CreateClass> MAP_DUI_CTRATECLASS;

	class FYUI_API CControlFactory
	{
	public:
		/**
		 * @brief 创建控件
		 * @details 用于创建控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param strClassName [in] 类名名称字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* CreateControl(std::wstring strClassName);
		/**
		 * @brief 执行 RegistControl 操作
		 * @details 用于执行 RegistControl 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param strClassName [in] 类名名称字符串
		 * @param pFunc [in] Func对象
		 */
		void RegistControl(std::wstring strClassName, CreateClass pFunc);

		/**
		 * @brief 获取Instance
		 * @details 用于获取Instance。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CControlFactory* 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlFactory* GetInstance();
		/**
		 * @brief 执行 Release 操作
		 * @details 用于执行 Release 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Release();

	private:	
		/**
		 * @brief 构造 CControlFactory 对象
		 * @details 用于构造 CControlFactory 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CControlFactory();
		/**
		 * @brief 析构 CControlFactory 对象
		 * @details 用于析构 CControlFactory 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CControlFactory();

	private:
		MAP_DUI_CTRATECLASS m_mapControl;
	};

	/**
	 * @brief 声明控件创建函数
	 * @details 为控件类型声明统一的静态创建入口，供控件工厂按类名实例化控件对象。
	 */
#define DECLARE_DUICONTROL(class_name)\
public:\
	static CControlUI* CreateControl();

	/**
	 * @brief 实现控件创建函数
	 * @details 为指定控件类型生成静态创建函数实现，并返回新的控件实例。
	 */
#define IMPLEMENT_DUICONTROL(class_name)\
	CControlUI* class_name::CreateControl()\
	{\
		return new class_name;\
	}

#define REGIST_DUICONTROL(class_name)\
	CControlFactory::GetInstance()->RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);

	/**
	 * @brief 在工厂内部注册控件
	 * @details 将控件类名与对应的静态创建函数绑定到控件工厂中，便于后续按名称创建。
	 */
#define INNER_REGISTER_DUICONTROL(class_name)\
	RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);


#define INNER_REGISTER_DUICONTROL_EX(str_class_name,class_type)\
	{\
		std::wstring sRegClassName;\
		sRegClassName = StringUtil::Format(L"C{}UI", str_class_name); \
		RegistControl(sRegClassName, (CreateClass)class_type::CreateControl);\
    }
}
