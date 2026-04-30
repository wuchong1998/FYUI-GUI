#pragma once

namespace FYUI
{
	class FYUI_API CTileLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CTileLayoutUI)
	public:
		/**
		 * @brief 构造 CTileLayoutUI 对象
		 * @details 用于构造 CTileLayoutUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CTileLayoutUI();

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
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true);

		/**
		 * @brief 获取子项尺寸
		 * @details 用于获取子项尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetItemSize() const;
		/**
		 * @brief 设置子项尺寸
		 * @details 用于设置子项尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szItem [in] 子项尺寸参数
		 */
		void SetItemSize(SIZE szItem);
		/**
		 * @brief 获取Columns
		 * @details 用于获取Columns。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetColumns() const;
		/**
		 * @brief 设置Columns
		 * @details 用于设置Columns。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nCols [in] Cols数值
		 */
		void SetColumns(int nCols);
		/**
		 * @brief 设置Auto尺寸
		 * @details 用于设置Auto尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bAutoSize [in] 是否Auto尺寸
		 */
		void SetAutoSize(bool bAutoSize);
		/**
		 * @brief 设置HorSpacing
		 * @details 用于设置HorSpacing。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSpacing [in] Spacing数值
		 */
		void SetHorSpacing(int nSpacing);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

	public:
		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CTileLayoutUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CTileLayoutUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CTileLayoutUI* pControl) ;
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
		bool m_bAutoSize = false;
		bool m_bFixedColumns = false;
		SIZE m_szItem;
		int m_nColumns;
		int m_nHorSpacing = 20;
	};
}

