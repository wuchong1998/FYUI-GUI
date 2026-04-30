#pragma once
namespace FYUI
{
	class FYUI_API CTabLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CTabLayoutUI)
	public:
		/**
		 * @brief 构造 CTabLayoutUI 对象
		 * @details 用于构造 CTabLayoutUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CTabLayoutUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

			/**
			 * @brief 执行 Add 操作
			 * @details 用于执行 Add 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param pControl [in] 控件对象
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool Add(CControlUI* pControl) override;
			/**
			 * @brief 添加At
			 * @details 用于添加At。具体行为由当前对象状态以及传入参数共同决定。
			 * @param pControl [in] 控件对象
			 * @param iIndex [in] 子项下标
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool AddAt(CControlUI* pControl, int iIndex) override;
			/**
			 * @brief 执行 Remove 操作
			 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param pControl [in] 控件对象
			 * @param bChildDelayed [in] 是否延迟处理子项
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool Remove(CControlUI* pControl, bool bChildDelayed = true) override;
			/**
			 * @brief 移除At
			 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 * @param bChildDelayed [in] 是否延迟处理子项
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool RemoveAt(int iIndex, bool bChildDelayed = true) override;
			/**
			 * @brief 移除全部子项
			 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
			 * @param bChildDelayed [in] 是否延迟处理子项
			 */
			void RemoveAll(bool bChildDelayed = true) override;
		/**
		 * @brief 获取当前选区
		 * @details 用于获取当前选区。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetCurSel() const;
		/**
		 * @brief 选中指定子项
		 * @details 用于选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param bIsFocus [in] 是否判断是否焦点
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool SelectItem(int iIndex,bool bIsFocus = true);
		/**
		 * @brief 选中指定子项
		 * @details 用于选中指定子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param bIsFocus [in] 是否判断是否焦点
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool SelectItem(CControlUI* pControl,bool bIsFocus = true);

		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true);

		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CTabLayoutUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CTabLayoutUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CTabLayoutUI* pControl) ;

		/**
		 * @brief 判断是否Container控件
		 * @details 用于判断是否Container控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsContainerControl() const override
		{
			return true;
		}


	protected:
		/**
		 * @brief 获取Selected子项控件
		 * @details 用于获取Selected子项控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* GetSelectedItemControl() const;
		/**
		 * @brief 更新SelectionVisibility
		 * @details 用于更新SelectionVisibility。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iSelected [in] Selected值
		 * @param bSetFocus [in] 是否设置焦点
		 */
		void UpdateSelectionVisibility(int iSelected, bool bSetFocus);
		/**
		 * @brief 执行 ResolveTabSelectionAfterAdd 操作
		 * @details 用于执行 ResolveTabSelectionAfterAdd 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @return int 返回对应的数值结果
		 */
		int ResolveTabSelectionAfterAdd(CControlUI* pControl, int iIndex) const;
		/**
		 * @brief 执行 ResolveTabSelectionAfterRemoval 操作
		 * @details 用于执行 ResolveTabSelectionAfterRemoval 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iRemovedIndex [in] Removed索引值
		 * @return int 返回对应的数值结果
		 */
		int ResolveTabSelectionAfterRemoval(int iRemovedIndex) const;
		int m_iCurSel;
	};
}

