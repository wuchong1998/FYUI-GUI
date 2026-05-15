#pragma once

#include <memory>
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

		/**
		 * @brief 控制本次 Create 是否使用进程级 XML 缓存
		 * @details 默认开启。XML 解析结果按 (instance + xml 标识 + type) 缓存为 CMarkup AST，
		 *          全局共享只读，跨窗口/跨次复用，避免重复磁盘 IO 与 XML 解析。
		 *          内联 XML（以 '<' 开头的字符串）以及加载失败的输入不会被缓存。
		 * @param bUse [in] true 启用缓存，false 关闭缓存（仅本次 Create 生效）
		 */
		void SetUseGlobalXmlCache(bool bUse) { m_bUseGlobalXmlCache = bUse; }

		/**
		 * @brief 全局便捷接口：一行从 XML 创建控件树
		 * @details 内部等价于：CDialogBuilder b; b.SetInstance(instance); return b.Create(xml,type,pCallback,pManager,pParent);
		 *          同时启用全局 XML 缓存，多次调用同一 XML 时跳过磁盘 IO 与 XML 解析。
		 *          典型用法：auto* root = CDialogBuilder::CreateControlsFromXml(L"main.xml", L"", this, &m_pm);
		 * @param xml [in] XML 文件路径 / 资源 ID / 内联 XML 文本
		 * @param type [in] 资源类型字符串
		 * @param pCallback [in] 自定义控件回调
		 * @param pManager [in] PaintManager
		 * @param pParent [in] 父控件
		 * @param instance [in] 模块实例句柄；为 NULL 时使用 CPaintManagerUI::GetResourceDll()
		 * @return CControlUI* 解析得到的根控件，失败返回 nullptr
		 */
		static CControlUI* CreateControlsFromXml(STRINGorID xml, std::wstring_view type = {},
			IDialogBuilderCallback* pCallback = NULL, CPaintManagerUI* pManager = NULL,
			CControlUI* pParent = NULL, HINSTANCE instance = NULL);

		/**
		 * @brief 清空进程级 XML 缓存
		 * @details 在切换皮肤资源、切换语言、热更新 XML 资源后调用，强制下次重新加载与解析。
		 *          线程安全。
		 */
		static void ClearXmlCache();
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

		// 返回当前 Create 调用使用的 XML 文档：优先返回缓存中的共享 AST，
		// 当缓存禁用或加载失败回退到实例自有 m_xml
		CMarkup& _ActiveMarkup();

		CMarkup m_xml;
		// 命中全局缓存时持有共享 CMarkup（只读，不会写入），由 _ActiveMarkup 选用
		std::shared_ptr<CMarkup> m_pSharedXml;
		IDialogBuilderCallback* m_pCallback;
		std::wstring m_typeStorage;
		const wchar_t* m_pstrtype;
		HINSTANCE m_instance;
		// 默认开启全局 XML 缓存，可通过 SetUseGlobalXmlCache(false) 关闭
		bool m_bUseGlobalXmlCache = true;
	};

} 


