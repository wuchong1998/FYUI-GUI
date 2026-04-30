
#pragma once

namespace FYUI
{
	class FYUI_API CChildLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CChildLayoutUI)
	public:
		/**
		 * @brief 构造 CChildLayoutUI 对象
		 * @details 用于构造 CChildLayoutUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CChildLayoutUI();

		/**
		 * @brief 初始化对象
		 * @details 用于初始化对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Init();
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 设置子控件布局XML
		 * @details 用于设置子控件布局XML。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pXML [in] XML对象
		 */
		void SetChildLayoutXML(std::wstring_view pXML);
		/**
		 * @brief 获取子控件布局XML
		 * @details 用于获取子控件布局XML。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetChildLayoutXML();
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		virtual LPVOID GetInterface(std::wstring_view pstrName);
		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetClass() const;
		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CChildLayoutUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CChildLayoutUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CChildLayoutUI* pControl) ;
		/**
		 * @brief 判断是否Container控件
		 * @details 用于判断是否Container控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsContainerControl() const override
		{
			return true;
		}
	private:
			/**
			 * @brief 判断是否具有子控件布局来源
			 * @details 用于判断是否具有子控件布局来源。具体行为由当前对象状态以及传入参数共同决定。
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool HasChildLayoutSource() const;
			/**
			 * @brief 判断是否具有Loaded子控件布局
			 * @details 用于判断是否具有Loaded子控件布局。具体行为由当前对象状态以及传入参数共同决定。
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool HasLoadedChildLayout() const;
			/**
			 * @brief 重新加载子控件布局
			 * @details 用于重新加载子控件布局。具体行为由当前对象状态以及传入参数共同决定。
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool ReloadChildLayout();
			/**
			 * @brief 清空子控件布局Container
			 * @details 用于清空子控件布局Container。具体行为由当前对象状态以及传入参数共同决定。
			 */
			void ClearChildLayoutContainer();
			/**
			 * @brief 创建子控件布局Container
			 * @details 用于创建子控件布局Container。具体行为由当前对象状态以及传入参数共同决定。
			 * @return CContainerUI* 返回结果对象指针，失败时返回 nullptr
			 */
			CContainerUI* CreateChildLayoutContainer() const;
			/**
			 * @brief 应用子控件布局Container
			 * @details 用于应用子控件布局Container。具体行为由当前对象状态以及传入参数共同决定。
			 * @param pChildWindow [in] 子控件窗口对象
			 */
			void ApplyChildLayoutContainer(CContainerUI* pChildWindow);
		std::wstring m_pstrXMLFile;
	};
} 


