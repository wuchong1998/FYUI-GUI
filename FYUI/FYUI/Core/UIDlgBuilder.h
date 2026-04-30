#pragma once

#include <string>
#include <string_view>

namespace FYUI {

	class IDialogBuilderCallback
	{
	public:
		/**
		 * @brief 创建控件
		 * @details 用于创建控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrClass [in] 类名字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CControlUI* CreateControl(std::wstring_view pstrClass) = 0;
	};


	class FYUI_API CDialogBuilder
	{
	public:
		/**
		 * @brief 构造 CDialogBuilder 对象
		 * @details 用于构造 CDialogBuilder 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CDialogBuilder();
		/**
		 * @brief 执行 Create 操作
		 * @details 用于执行 Create 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param xml [in] xml参数
		 * @param type [in] 类型参数
		 * @param pCallback [in] 回调接口对象
		 * @param pManager [in] 管理器对象
		 * @param pParent [in] 父级控件对象
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* Create(STRINGorID xml, std::wstring_view type = {}, IDialogBuilderCallback* pCallback = NULL,
			CPaintManagerUI* pManager = NULL, CControlUI* pParent = NULL);
		/**
		 * @brief 执行 Create 操作
		 * @details 用于执行 Create 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pCallback [in] 回调接口对象
		 * @param pManager [in] 管理器对象
		 * @param pParent [in] 父级控件对象
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* Create(IDialogBuilderCallback* pCallback = NULL, CPaintManagerUI* pManager = NULL,
			CControlUI* pParent = NULL);

		/**
		 * @brief 获取标记文档
		 * @details 用于获取标记文档。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CMarkup* 返回结果对象指针，失败时返回 nullptr
		 */
		CMarkup* GetMarkup();

		/**
		 * @brief 获取LastError消息
		 * @details 用于获取LastError消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrMessage [in] 消息字符串
		 * @param cchMax [in] cch最大参数
		 */
		void GetLastErrorMessage(wchar_t* pstrMessage, SIZE_T cchMax) const;
		/**
		 * @brief 获取LastErrorLocation
		 * @details 用于获取LastErrorLocation。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrSource [in] 来源字符串
		 * @param cchMax [in] cch最大参数
		 */
		void GetLastErrorLocation(wchar_t* pstrSource, SIZE_T cchMax) const;
		/**
		 * @brief 设置Instance
		 * @details 用于设置Instance。具体行为由当前对象状态以及传入参数共同决定。
		 * @param instance [in] 模块实例句柄
		 */
		void SetInstance(HINSTANCE instance){ m_instance = instance;};
	private:
		/**
		 * @brief 执行 _ParseControlNode 操作
		 * @details 用于执行 _ParseControlNode 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param node [in,out] 节点参数
		 * @param pParent [in] 父级控件对象
		 * @param pManager [in] 管理器对象
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* _ParseControlNode(CMarkupNode& node, CControlUI* pParent = NULL, CPaintManagerUI* pManager = NULL);
		/**
		 * @brief 执行 _Parse 操作
		 * @details 用于执行 _Parse 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param parent [in] 父级控件参数
		 * @param pParent [in] 父级控件对象
		 * @param pManager [in] 管理器对象
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* _Parse(CMarkupNode* parent, CControlUI* pParent = NULL, CPaintManagerUI* pManager = NULL);

		CMarkup m_xml;
		IDialogBuilderCallback* m_pCallback;
		std::wstring m_typeStorage;
		const wchar_t* m_pstrtype;
		HINSTANCE m_instance;
	};

} 


