#pragma once
#include "Layout/UIVerticalLayout.h"
#include "Layout/UIHorizontalLayout.h"

namespace FYUI {

	class CPaintRenderContext;

	class IListComboCallbackUI
	{
	public:
		/**
		 * @brief 获取子项下拉框文本Array
		 * @details 用于获取子项下拉框文本Array。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pCtrl [in] Ctrl对象
		 * @param iItem [in] 子项下标
		 * @param iSubItem [in] 子项下标
		 */
		virtual void GetItemComboTextArray(CControlUI* pCtrl, int iItem, int iSubItem) = 0;
	};

	class CComboBoxUI;
	class CListContainerHeaderItemUI;
	class CListTextExtElementUI;

	class FYUI_API CListExUI : public CListUI, public INotifyUI
	{
		DECLARE_DUICONTROL(CListExUI)

	public:
		/**
		 * @brief 构造 CListExUI 对象
		 * @details 用于构造 CListExUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListExUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

	public: 
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void DoEvent(TEventUI& event);

		public:
			/**
			 * @brief 执行 InitListCtrl 操作
			 * @details 用于执行 InitListCtrl 操作。具体行为由当前对象状态以及传入参数共同决定。
			 */
			void InitListCtrl();
			/**
			 * @brief 执行 Remove 操作
			 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param pControl [in] 控件对象
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool Remove(CControlUI* pControl);
			/**
			 * @brief 移除At
			 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 * @return bool 操作成功返回 true，否则返回 false
			 */
			bool RemoveAt(int iIndex);
			/**
			 * @brief 移除全部子项
			 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
			 */
			void RemoveAll();

		protected:
			CComboBoxUI*	m_pComboBoxUI;

	public:
		/**
		 * @brief 执行 CheckColumEditable 操作
		 * @details 用于执行 CheckColumEditable 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @return BOOL 返回 执行 CheckColumEditable 操作 的结果
		 */
		virtual BOOL CheckColumEditable(int nColum);

		/**
		 * @brief 执行 CheckColumComboBoxable 操作
		 * @details 用于执行 CheckColumComboBoxable 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @return BOOL 返回 执行 CheckColumComboBoxable 操作 的结果
		 */
		virtual BOOL CheckColumComboBoxable(int nColum);
		/**
		 * @brief 获取下拉框BoxUI
		 * @details 用于获取下拉框BoxUI。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CComboBoxUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CComboBoxUI* GetComboBoxUI();

		/**
		 * @brief 执行 CheckColumCheckBoxable 操作
		 * @details 用于执行 CheckColumCheckBoxable 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @return BOOL 返回 执行 CheckColumCheckBoxable 操作 的结果
		 */
		virtual BOOL CheckColumCheckBoxable(int nColum);

	public:
		/**
		 * @brief 执行 Notify 操作
		 * @details 用于执行 Notify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in,out] msg参数
		 */
		virtual void Notify(TNotifyUI& msg);
		BOOL	m_bAddMessageFilter;
		int		m_nRow,m_nColum;
		/**
		 * @brief 设置Edit行AndColum
		 * @details 用于设置Edit行AndColum。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nRow [in] 行数值
		 * @param nColum [in] Colum数值
		 */
		void	SetEditRowAndColum(int nRow,int nColum) { m_nRow = nRow; m_nColum = nColum; };

	public:
		IListComboCallbackUI* m_pXCallback;
		/**
		 * @brief 获取文本Array回调
		 * @details 用于获取文本Array回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @return IListComboCallbackUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual IListComboCallbackUI* GetTextArrayCallback() const;
		/**
		 * @brief 设置文本Array回调
		 * @details 用于设置文本Array回调。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pCallback [in] 回调接口对象
		 */
		virtual void SetTextArrayCallback(IListComboCallbackUI* pCallback);

		public:
			/**
			 * @brief 执行 OnListItemClicked 操作
			 * @details 用于执行 OnListItemClicked 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param nIndex [in] 索引值
			 * @param nColum [in] Colum数值
			 * @param lpRCColum [in] lpRCColum参数
			 * @param lpstrText [in] lpstr文本参数
			 */
			void OnListItemClicked(int nIndex, int nColum, RECT* lpRCColum, std::wstring_view lpstrText);
			/**
			 * @brief 执行 OnListItemChecked 操作
			 * @details 用于执行 OnListItemChecked 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param nIndex [in] 索引值
			 * @param nColum [in] Colum数值
			 * @param bChecked [in] 是否Checked
			 */
			void OnListItemChecked(int nIndex, int nColum, BOOL bChecked);
			/**
			 * @brief 执行 DismissInlineEditorsForRow 操作
			 * @details 用于执行 DismissInlineEditorsForRow 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param nRow [in] 行数值
			 */
			void DismissInlineEditorsForRow(int nRow);

		public:
			/**
			 * @brief 设置Colum子项颜色
			 * @details 用于设置Colum子项颜色。具体行为由当前对象状态以及传入参数共同决定。
			 * @param nIndex [in] 索引值
			 * @param nColum [in] Colum数值
			 * @param iBKColor [in] 背景颜色值
			 */
			void SetColumItemColor(int nIndex, int nColum, DWORD iBKColor);
		/**
		 * @brief 获取Colum子项颜色
		 * @details 用于获取Colum子项颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nIndex [in] 索引值
		 * @param nColum [in] Colum数值
		 * @param iBKColor [in,out] 背景颜色值
		 * @return BOOL 返回 获取Colum子项颜色 的结果
		 */
		BOOL GetColumItemColor(int nIndex, int nColum, DWORD& iBKColor);

	private:
		/**
		 * @brief 获取表头子项At
		 * @details 用于获取表头子项At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @return CListContainerHeaderItemUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CListContainerHeaderItemUI* GetHeaderItemAt(int nColum) const;
		/**
		 * @brief 获取文本Ext子项At
		 * @details 用于获取文本Ext子项At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nIndex [in] 索引值
		 * @return CListTextExtElementUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CListTextExtElementUI* GetTextExtItemAt(int nIndex) const;
		/**
		 * @brief 处理表头Checked通知
		 * @details 用于处理表头Checked通知。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in] msg参数
		 */
		void HandleHeaderCheckedNotify(const TNotifyUI& msg);
		/**
		 * @brief 处理子项Checked通知
		 * @details 用于处理子项Checked通知。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in] msg参数
		 */
		void HandleItemCheckedNotify(const TNotifyUI& msg);
		/**
		 * @brief 处理Inline下拉框通知
		 * @details 用于处理Inline下拉框通知。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in] msg参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HandleInlineComboNotify(const TNotifyUI& msg);
		/**
		 * @brief 处理Inline滚动通知
		 * @details 用于处理Inline滚动通知。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in] msg参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HandleInlineScrollNotify(const TNotifyUI& msg);
		/**
		 * @brief 隐藏Inline下拉框控件
		 * @details 用于隐藏Inline下拉框控件。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void HideInlineComboControl();
		/**
		 * @brief 隐藏EditAnd下拉框Ctrl
		 * @details 用于隐藏EditAnd下拉框Ctrl。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void HideEditAndComboCtrl();
		/**
		 * @brief 执行 CommitInlineComboSelection 操作
		 * @details 用于执行 CommitInlineComboSelection 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void CommitInlineComboSelection();
			/**
			 * @brief 显示Inline下拉框
			 * @details 用于显示Inline下拉框。具体行为由当前对象状态以及传入参数共同决定。
			 * @param nIndex [in] 索引值
			 * @param nColum [in] Colum数值
			 * @param rcColumn [in] 列矩形区域
			 * @param lpstrText [in] lpstr文本参数
			 */
			void ShowInlineCombo(int nIndex, int nColum, const RECT& rcColumn, std::wstring_view lpstrText);
			/**
			 * @brief 执行 ActivateInlineCellEditor 操作
			 * @details 用于执行 ActivateInlineCellEditor 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param nIndex [in] 索引值
			 * @param nColum [in] Colum数值
			 * @param rcColumn [in] 列矩形区域
			 * @param lpstrText [in] lpstr文本参数
			 */
			void ActivateInlineCellEditor(int nIndex, int nColum, const RECT& rcColumn, std::wstring_view lpstrText);
			/**
			 * @brief 执行 SyncHeaderCheckState 操作
			 * @details 用于执行 SyncHeaderCheckState 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param nColum [in] Colum数值
			 * @param bChecked [in] 是否Checked
			 */
			void SyncHeaderCheckState(int nColum, BOOL bChecked);
			/**
			 * @brief 执行 SyncInlineEditorAfterRemoval 操作
			 * @details 用于执行 SyncInlineEditorAfterRemoval 操作。具体行为由当前对象状态以及传入参数共同决定。
			 * @param nRow [in] 行数值
			 */
			void SyncInlineEditorAfterRemoval(int nRow);
		};

	/////////////////////////////////////////////////////////////////////////////////////
	//
	class FYUI_API CListContainerHeaderItemUI : public CHorizontalLayoutUI
	{
		DECLARE_DUICONTROL(CListContainerHeaderItemUI)

	public:
		/**
		 * @brief 构造 CListContainerHeaderItemUI 对象
		 * @details 用于构造 CListContainerHeaderItemUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListContainerHeaderItemUI();

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
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;

		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetEnabled(BOOL bEnable = TRUE);

		/**
		 * @brief 判断是否Dragable
		 * @details 用于判断是否Dragable。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BOOL 返回 判断是否Dragable 的结果
		 */
		BOOL IsDragable() const;
		/**
		 * @brief 设置Dragable
		 * @details 用于设置Dragable。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bDragable [in] 是否Dragable
		 */
		void SetDragable(BOOL bDragable);
		/**
		 * @brief 获取分隔宽度
		 * @details 用于获取分隔宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSepWidth() const;
		/**
		 * @brief 设置分隔宽度
		 * @details 用于设置分隔宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iWidth [in] 宽度值
		 */
		void SetSepWidth(int iWidth);
		/**
		 * @brief 获取文本样式
		 * @details 用于获取文本样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetTextStyle() const;
		/**
		 * @brief 设置文本样式
		 * @details 用于设置文本样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uStyle [in] 样式标志
		 */
		void SetTextStyle(UINT uStyle);
		/**
		 * @brief 获取文本颜色
		 * @details 用于获取文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetTextColor() const;
		/**
		 * @brief 设置文本颜色
		 * @details 用于设置文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwTextColor [in] 文本颜色值
		 */
		void SetTextColor(DWORD dwTextColor);
		/**
		 * @brief 设置文本内边距
		 * @details 用于设置文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void SetTextPadding(RECT rc);
		/**
		 * @brief 获取文本内边距
		 * @details 用于获取文本内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetTextPadding() const;
		/**
		 * @brief 设置字体
		 * @details 用于设置字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param index [in] 子项下标
		 */
		void SetFont(int index);
		/**
		 * @brief 判断是否显示HTML 文本
		 * @details 用于判断是否显示HTML 文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BOOL 返回 判断是否显示HTML 文本 的结果
		 */
		BOOL IsShowHtml();
		/**
		 * @brief 设置显示HTML 文本
		 * @details 用于设置显示HTML 文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShowHtml [in] 是否显示HTML 文本
		 */
		void SetShowHtml(BOOL bShowHtml = TRUE);
		/**
		 * @brief 获取Normal图像
		 * @details 用于获取Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetNormalImage() const;
		/**
		 * @brief 设置Normal图像
		 * @details 用于设置Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取热状态图像
		 * @details 用于获取热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetHotImage() const;
		/**
		 * @brief 设置热状态图像
		 * @details 用于设置热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Pushed图像
		 * @details 用于获取Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetPushedImage() const;
		/**
		 * @brief 设置Pushed图像
		 * @details 用于设置Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetPushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Focused图像
		 * @details 用于获取Focused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetFocusedImage() const;
		/**
		 * @brief 设置Focused图像
		 * @details 用于设置Focused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetFocusedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取分隔图像
		 * @details 用于获取分隔图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetSepImage() const;
		/**
		 * @brief 设置分隔图像
		 * @details 用于设置分隔图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetSepImage(std::wstring_view pStrImage);

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 获取滑块矩形
		 * @details 用于获取滑块矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetThumbRect() const;

		/**
		 * @brief 绘制文本内容
		 * @details 用于绘制文本内容。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintText(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制状态图像
		 * @details 用于绘制状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

	protected:
		POINT ptLastMouse;
		BOOL m_bDragable;
		UINT m_uButtonState;
		int m_iSepWidth;
		DWORD m_dwTextColor;
		int m_iFont;
		UINT m_uTextStyle;
		BOOL m_bShowHtml;
		RECT m_rcTextPadding;
		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sPushedImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sSepImage;
		std::wstring m_sSepImageModify;

		//閺€顖涘瘮缂傛牞绶?
		BOOL m_bEditable;

		//閺€顖涘瘮缂佸嫬鎮庡?
		BOOL m_bComboable;

		//閺€顖涘瘮婢跺秹鈧顢?
		BOOL m_bCheckBoxable;

	public:
		/**
		 * @brief 获取ColumeEditable
		 * @details 用于获取ColumeEditable。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BOOL 返回 获取ColumeEditable 的结果
		 */
		BOOL GetColumeEditable();
		/**
		 * @brief 设置ColumeEditable
		 * @details 用于设置ColumeEditable。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetColumeEditable(BOOL bEnable);

		/**
		 * @brief 获取ColumeComboable
		 * @details 用于获取ColumeComboable。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BOOL 返回 获取ColumeComboable 的结果
		 */
		BOOL GetColumeComboable();
		/**
		 * @brief 设置ColumeComboable
		 * @details 用于设置ColumeComboable。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetColumeComboable(BOOL bEnable);

		/**
		 * @brief 获取ColumeCheckable
		 * @details 用于获取ColumeCheckable。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BOOL 返回 获取ColumeCheckable 的结果
		 */
		BOOL GetColumeCheckable();
		/**
		 * @brief 设置ColumeCheckable
		 * @details 用于设置ColumeCheckable。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetColumeCheckable(BOOL bEnable);

	public:
		/**
		 * @brief 设置Check
		 * @details 用于设置Check。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bCheck [in] 是否Check
		 */
		void SetCheck(BOOL bCheck);
		/**
		 * @brief 获取Check
		 * @details 用于获取Check。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BOOL 返回 获取Check 的结果
		 */
		BOOL GetCheck();

	private:
		UINT	m_uCheckBoxState;
		BOOL	m_bChecked;

		std::wstring m_sCheckBoxNormalImage;
		std::wstring m_sCheckBoxHotImage;
		std::wstring m_sCheckBoxPushedImage;
		std::wstring m_sCheckBoxFocusedImage;
		std::wstring m_sCheckBoxDisabledImage;

		std::wstring m_sCheckBoxSelectedImage;
		std::wstring m_sCheckBoxForeImage;

		SIZE m_cxyCheckBox;

	public:
		/**
		 * @brief 绘制CheckBox图像
		 * @details 用于绘制CheckBox图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStrImage [in] 图像描述字符串
		 * @param pStrModify [in] StrModify对象
		 * @return BOOL 返回 绘制CheckBox图像 的结果
		 */
		BOOL DrawCheckBoxImage(CPaintRenderContext& renderContext, const std::wstring& pStrImage, const std::wstring& pStrModify = std::wstring());
		/**
		 * @brief 获取CheckBoxNormal图像
		 * @details 用于获取CheckBoxNormal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxNormalImage();
		/**
		 * @brief 设置CheckBoxNormal图像
		 * @details 用于设置CheckBoxNormal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBox热状态图像
		 * @details 用于获取CheckBox热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxHotImage();
		/**
		 * @brief 设置CheckBox热状态图像
		 * @details 用于设置CheckBox热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBoxPushed图像
		 * @details 用于获取CheckBoxPushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxPushedImage();
		/**
		 * @brief 设置CheckBoxPushed图像
		 * @details 用于设置CheckBoxPushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxPushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBoxFocused图像
		 * @details 用于获取CheckBoxFocused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxFocusedImage();
		/**
		 * @brief 设置CheckBoxFocused图像
		 * @details 用于设置CheckBoxFocused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxFocusedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBoxDisabled图像
		 * @details 用于获取CheckBoxDisabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxDisabledImage();
		/**
		 * @brief 设置CheckBoxDisabled图像
		 * @details 用于设置CheckBoxDisabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxDisabledImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取CheckBoxSelected图像
		 * @details 用于获取CheckBoxSelected图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxSelectedImage();
		/**
		 * @brief 设置CheckBoxSelected图像
		 * @details 用于设置CheckBoxSelected图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxSelectedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBox前景图像
		 * @details 用于获取CheckBox前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxForeImage();
		/**
		 * @brief 设置CheckBox前景图像
		 * @details 用于设置CheckBox前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxForeImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取CheckBox矩形
		 * @details 用于获取CheckBox矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in,out] 矩形区域
		 */
		void GetCheckBoxRect(RECT &rc);	

		/**
		 * @brief 获取CheckBox宽度
		 * @details 用于获取CheckBox宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetCheckBoxWidth() const;       // 鐎圭偤妾径褍鐨担宥囩枂娴ｈ法鏁etPos閼惧嘲褰囬敍宀冪箹闁插苯绶遍崚鎵畱閺勵垶顣╃拋鍓ф畱閸欏倽鈧啫鈧?
		void SetCheckBoxWidth(int cx);      // 妫板嫯顔曢惃鍕棘閼板啫鈧?
		int GetCheckBoxHeight() const;      // 鐎圭偤妾径褍鐨担宥囩枂娴ｈ法鏁etPos閼惧嘲褰囬敍宀冪箹闁插苯绶遍崚鎵畱閺勵垶顣╃拋鍓ф畱閸欏倽鈧啫鈧?
		void SetCheckBoxHeight(int cy);     // 妫板嫯顔曢惃鍕棘閼板啫鈧?


	public:
		CContainerUI* m_pOwner;
		/**
		 * @brief 设置所属对象
		 * @details 用于设置所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		void SetOwner(CContainerUI* pOwner);
		/**
		 * @brief 获取所属对象
		 * @details 用于获取所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CContainerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CContainerUI* GetOwner();
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CListTextExtElementUI : public CListLabelElementUI
	{
		DECLARE_DUICONTROL(CListTextExtElementUI)

	public:
		/**
		 * @brief 构造 CListTextExtElementUI 对象
		 * @details 用于构造 CListTextExtElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CListTextExtElementUI();
		/**
		 * @brief 析构 CListTextExtElementUI 对象
		 * @details 用于析构 CListTextExtElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CListTextExtElementUI();

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
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;

		/**
		 * @brief 获取文本
		 * @details 用于获取文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetText(int iIndex) const;
		/**
		 * @brief 设置文本
		 * @details 用于设置文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param pstrText [in] 文本字符串
		 */
		void SetText(int iIndex, std::wstring_view pstrText);
		/**
		 * @brief 设置文本
		 * @details 用于设置文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @param pstrText [in] 文本字符串
		 */
		void SetText(int iIndex, const std::wstring& pstrText) { SetText(iIndex, std::wstring_view(pstrText)); }

			void SetOwner(CControlUI* pOwner);
			/**
			 * @brief 设置可见状态
			 * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
			 * @param bVisible [in] 是否可见状态
			 */
			void SetVisible(bool bVisible = true);
			/**
			 * @brief 设置启用状态
			 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
			 * @param bEnable [in] 是否启用
			 */
			void SetEnabled(bool bEnable = true);
			/**
			 * @brief 获取LinkContent
			 * @details 用于获取LinkContent。具体行为由当前对象状态以及传入参数共同决定。
			 * @param iIndex [in] 子项下标
			 * @return 返回对应的字符串内容
			 */
			std::wstring* GetLinkContent(int iIndex);

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable);

		/**
		 * @brief 绘制子项文本
		 * @details 用于绘制子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem) override;

	protected:
		enum { MAX_LINK = 8 };
		int m_nLinks;
		RECT m_rcLinks[MAX_LINK];
		std::wstring m_sLinks[MAX_LINK];
		int m_nHoverLink;
		CListUI* m_pOwner;
		CStdPtrArray m_aTexts;
		mutable std::wstring m_sTextCache;

	private:
		UINT	m_uCheckBoxState;
		BOOL	m_bChecked;

		std::wstring m_sCheckBoxNormalImage;
		std::wstring m_sCheckBoxHotImage;
		std::wstring m_sCheckBoxPushedImage;
		std::wstring m_sCheckBoxFocusedImage;
		std::wstring m_sCheckBoxDisabledImage;

		std::wstring m_sCheckBoxSelectedImage;
		std::wstring m_sCheckBoxForeImage;

		SIZE m_cxyCheckBox;

	public:
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 绘制状态图像
		 * @details 用于绘制状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void PaintStatusImage(CPaintRenderContext& renderContext) override;
		/**
		 * @brief 绘制CheckBox图像
		 * @details 用于绘制CheckBox图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStrImage [in] 图像描述字符串
		 * @param pStrModify [in] StrModify对象
		 * @param rcCheckBox [in,out] CheckBox矩形区域
		 * @return BOOL 返回 绘制CheckBox图像 的结果
		 */
		BOOL DrawCheckBoxImage(CPaintRenderContext& renderContext, const std::wstring& pStrImage, const std::wstring& pStrModify, RECT& rcCheckBox);
		/**
		 * @brief 获取CheckBoxNormal图像
		 * @details 用于获取CheckBoxNormal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxNormalImage();
		/**
		 * @brief 设置CheckBoxNormal图像
		 * @details 用于设置CheckBoxNormal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBox热状态图像
		 * @details 用于获取CheckBox热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxHotImage();
		/**
		 * @brief 设置CheckBox热状态图像
		 * @details 用于设置CheckBox热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBoxPushed图像
		 * @details 用于获取CheckBoxPushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxPushedImage();
		/**
		 * @brief 设置CheckBoxPushed图像
		 * @details 用于设置CheckBoxPushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxPushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBoxFocused图像
		 * @details 用于获取CheckBoxFocused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxFocusedImage();
		/**
		 * @brief 设置CheckBoxFocused图像
		 * @details 用于设置CheckBoxFocused图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxFocusedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBoxDisabled图像
		 * @details 用于获取CheckBoxDisabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxDisabledImage();
		/**
		 * @brief 设置CheckBoxDisabled图像
		 * @details 用于设置CheckBoxDisabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxDisabledImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取CheckBoxSelected图像
		 * @details 用于获取CheckBoxSelected图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxSelectedImage();
		/**
		 * @brief 设置CheckBoxSelected图像
		 * @details 用于设置CheckBoxSelected图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxSelectedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取CheckBox前景图像
		 * @details 用于获取CheckBox前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCheckBoxForeImage();
		/**
		 * @brief 设置CheckBox前景图像
		 * @details 用于设置CheckBox前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetCheckBoxForeImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取CheckBox矩形
		 * @details 用于获取CheckBox矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nIndex [in] 索引值
		 * @param rc [in,out] 矩形区域
		 */
		void GetCheckBoxRect(int nIndex, RECT &rc);	
		/**
		 * @brief 获取Colum矩形
		 * @details 用于获取Colum矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @param rc [in,out] 矩形区域
		 */
		void GetColumRect(int nColum, RECT &rc);

		/**
		 * @brief 获取CheckBox宽度
		 * @details 用于获取CheckBox宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetCheckBoxWidth() const;       // 鐎圭偤妾径褍鐨担宥囩枂娴ｈ法鏁etPos閼惧嘲褰囬敍宀冪箹闁插苯绶遍崚鎵畱閺勵垶顣╃拋鍓ф畱閸欏倽鈧啫鈧?
		void SetCheckBoxWidth(int cx);      // 妫板嫯顔曢惃鍕棘閼板啫鈧?
		int GetCheckBoxHeight() const;      // 鐎圭偤妾径褍鐨担宥囩枂娴ｈ法鏁etPos閼惧嘲褰囬敍宀冪箹闁插苯绶遍崚鎵畱閺勵垶顣╃拋鍓ф畱閸欏倽鈧啫鈧?
		void SetCheckBoxHeight(int cy);     // 妫板嫯顔曢惃鍕棘閼板啫鈧?

		void SetCheck(BOOL bCheck);
		/**
		 * @brief 获取Check
		 * @details 用于获取Check。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BOOL 返回 获取Check 的结果
		 */
		BOOL GetCheck() const;

	public:
		/**
		 * @brief 执行 HitTestColum 操作
		 * @details 用于执行 HitTestColum 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ptMouse [in] 鼠标坐标
		 * @return int 返回对应的数值结果
		 */
		int HitTestColum(POINT ptMouse);
		/**
		 * @brief 执行 CheckColumEditable 操作
		 * @details 用于执行 CheckColumEditable 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @return BOOL 返回 执行 CheckColumEditable 操作 的结果
		 */
		BOOL CheckColumEditable(int nColum);

	private:
		/**
		 * @brief 处理Link光标事件
		 * @details 用于处理Link光标事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in] 事件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HandleLinkCursorEvent(const TEventUI& event);
		/**
		 * @brief 处理Link点击事件
		 * @details 用于处理Link点击事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in] 事件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HandleLinkClickEvent(const TEventUI& event);
		/**
		 * @brief 更新LinkHover状态
		 * @details 用于更新LinkHover状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in] 事件对象
		 */
		void UpdateLinkHoverState(const TEventUI& event);
		/**
		 * @brief 执行 NotifyInlineCellClick 操作
		 * @details 用于执行 NotifyInlineCellClick 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in] 事件对象
		 * @param pListCtrl [in] 列表Ctrl对象
		 */
		void NotifyInlineCellClick(const TEventUI& event, CListExUI* pListCtrl);
		/**
		 * @brief 处理列CheckBox事件
		 * @details 用于处理列CheckBox事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in] 事件对象
		 * @param pListCtrl [in] 列表Ctrl对象
		 */
		void HandleColumnCheckBoxEvent(const TEventUI& event, CListExUI* pListCtrl);

		typedef struct tagColumColorNode
		{
			BOOL  bEnable;
			DWORD iTextColor;
			DWORD iBKColor;
		}COLUMCOLORNODE;

		COLUMCOLORNODE ColumCorlorArray[UILIST_MAX_COLUMNS];

	public:
		/**
		 * @brief 设置Colum子项颜色
		 * @details 用于设置Colum子项颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @param iBKColor [in] 背景颜色值
		 */
		void SetColumItemColor(int nColum, DWORD iBKColor);
		/**
		 * @brief 获取Colum子项颜色
		 * @details 用于获取Colum子项颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nColum [in] Colum数值
		 * @param iBKColor [in,out] 背景颜色值
		 * @return BOOL 返回 获取Colum子项颜色 的结果
		 */
		BOOL GetColumItemColor(int nColum, DWORD& iBKColor);

	};
} 



