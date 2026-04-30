#pragma once

using namespace std;

namespace FYUI
{
	class CTreeViewUI;
	class CCheckBoxUI;
	class CLabelUI;
	class COptionUI;

	class FYUI_API CTreeNodeUI : public CListContainerElementUI
	{
		DECLARE_DUICONTROL(CTreeNodeUI)
	public:
		/**
		 * @brief 构造 CTreeNodeUI 对象
		 * @details 用于构造 CTreeNodeUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _ParentNode [in] 父级控件节点参数
		 */
		CTreeNodeUI(CTreeNodeUI* _ParentNode = NULL);
		/**
		 * @brief 析构 CTreeNodeUI 对象
		 * @details 用于析构 CTreeNodeUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CTreeNodeUI(void);

	public:
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
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 触发重绘
		 * @details 用于触发重绘。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Invalidate();
		/**
		 * @brief 执行 Select 操作
		 * @details 用于执行 Select 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelect [in] 是否选中
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Select(bool bSelect = true);
		/**
		 * @brief 选中多选
		 * @details 用于选中多选。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bSelect [in] 是否选中
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SelectMulti(bool bSelect = true);

		/**
		 * @brief 执行 Add 操作
		 * @details 用于执行 Add 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _pTreeNodeUI [in] p树节点UI参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Add(CControlUI* _pTreeNodeUI);
		/**
		 * @brief 添加At
		 * @details 用于添加At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddAt(CControlUI* pControl, int iIndex);
		/**
		 * @brief 执行 Remove 操作
		 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Remove(CControlUI* pControl);

		/**
		 * @brief 设置可见状态标记
		 * @details 用于设置可见状态标记。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _IsVisible [in] 判断是否可见状态参数
		 */
		void SetVisibleTag(bool _IsVisible);
		/**
		 * @brief 获取可见状态标记
		 * @details 用于获取可见状态标记。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetVisibleTag();
		/**
		 * @brief 设置子项文本
		 * @details 用于设置子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrValue [in] 属性值
		 */
		void SetItemText(std::wstring_view pstrValue);
		/**
		 * @brief 获取子项文本
		 * @details 用于获取子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetItemText();
		/**
		 * @brief 执行 CheckBoxSelected 操作
		 * @details 用于执行 CheckBoxSelected 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _Selected [in] Selected参数
		 */
		void CheckBoxSelected(bool _Selected);
		/**
		 * @brief 判断是否CheckBoxSelected
		 * @details 用于判断是否CheckBoxSelected。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsCheckBoxSelected() const;
		/**
		 * @brief 判断是否判断是否具有子控件
		 * @details 用于判断是否判断是否具有子控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsHasChild() const;
		/**
		 * @brief 获取树Level
		 * @details 用于获取树Level。具体行为由当前对象状态以及传入参数共同决定。
		 * @return long 返回 获取树Level 的结果
		 */
		long GetTreeLevel();
		/**
		 * @brief 添加子控件节点
		 * @details 用于添加子控件节点。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _pTreeNodeUI [in] p树节点UI参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddChildNode(CTreeNodeUI* _pTreeNodeUI);
		/**
		 * @brief 移除At
		 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _pTreeNodeUI [in] p树节点UI参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveAt(CTreeNodeUI* _pTreeNodeUI);
		/**
		 * @brief 设置父级控件节点
		 * @details 用于设置父级控件节点。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _pParentTreeNode [in] p父级控件树节点参数
		 */
		void SetParentNode(CTreeNodeUI* _pParentTreeNode);
		/**
		 * @brief 获取父级控件节点
		 * @details 用于获取父级控件节点。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CTreeNodeUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CTreeNodeUI* GetParentNode();
		/**
		 * @brief 获取数量子控件
		 * @details 用于获取数量子控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return long 返回 获取数量子控件 的结果
		 */
		long GetCountChild();
		/**
		 * @brief 设置树视图
		 * @details 用于设置树视图。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _CTreeViewUI [in] C树视图UI参数
		 */
		void SetTreeView(CTreeViewUI* _CTreeViewUI);
		/**
		 * @brief 获取树视图
		 * @details 用于获取树视图。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CTreeViewUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CTreeViewUI* GetTreeView();
		/**
		 * @brief 获取子控件节点
		 * @details 用于获取子控件节点。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _nIndex [in] n索引参数
		 * @return CTreeNodeUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CTreeNodeUI* GetChildNode(int _nIndex);
		/**
		 * @brief 设置可见状态FolderBtn
		 * @details 用于设置可见状态FolderBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _IsVisibled [in] 判断是否Visibled参数
		 */
		void SetVisibleFolderBtn(bool _IsVisibled);
		/**
		 * @brief 获取可见状态FolderBtn
		 * @details 用于获取可见状态FolderBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetVisibleFolderBtn();
		/**
		 * @brief 设置可见状态CheckBtn
		 * @details 用于设置可见状态CheckBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _IsVisibled [in] 判断是否Visibled参数
		 */
		void SetVisibleCheckBtn(bool _IsVisibled);
		/**
		 * @brief 获取可见状态CheckBtn
		 * @details 用于获取可见状态CheckBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetVisibleCheckBtn();
		/**
		 * @brief 设置子项文本颜色
		 * @details 用于设置子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _dwItemTextColor [in] dw子项文本颜色参数
		 */
		void SetItemTextColor(DWORD _dwItemTextColor);
		/**
		 * @brief 获取子项文本颜色
		 * @details 用于获取子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemTextColor() const;
		/**
		 * @brief 设置子项热状态文本颜色
		 * @details 用于设置子项热状态文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _dwItemHotTextColor [in] dw子项热状态文本颜色参数
		 */
		void SetItemHotTextColor(DWORD _dwItemHotTextColor);
		/**
		 * @brief 获取子项热状态文本颜色
		 * @details 用于获取子项热状态文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetItemHotTextColor() const;
		/**
		 * @brief 设置选区子项文本颜色
		 * @details 用于设置选区子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _dwSelItemTextColor [in] dw选区子项文本颜色参数
		 */
		void SetSelItemTextColor(DWORD _dwSelItemTextColor);
		/**
		 * @brief 获取选区子项文本颜色
		 * @details 用于获取选区子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelItemTextColor() const;
		/**
		 * @brief 设置选区子项热状态文本颜色
		 * @details 用于设置选区子项热状态文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _dwSelHotItemTextColor [in] dw选区热状态子项文本颜色参数
		 */
		void SetSelItemHotTextColor(DWORD _dwSelHotItemTextColor);
		/**
		 * @brief 获取选区子项热状态文本颜色
		 * @details 用于获取选区子项热状态文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetSelItemHotTextColor() const;
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 获取树Nodes
		 * @details 用于获取树Nodes。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CStdPtrArray 返回 获取树Nodes 的结果
		 */
		CStdPtrArray GetTreeNodes();
		/**
		 * @brief 获取树索引
		 * @details 用于获取树索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int			 GetTreeIndex();
		/**
		 * @brief 获取节点索引
		 * @details 用于获取节点索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int			 GetNodeIndex();

	public:
		/**
		 * @brief 获取树节点Horiznotal
		 * @details 用于获取树节点Horiznotal。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CHorizontalLayoutUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CHorizontalLayoutUI*	GetTreeNodeHoriznotal() const {return pHoriz;};
		/**
		 * @brief 获取Folder按钮
		 * @details 用于获取Folder按钮。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CCheckBoxUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CCheckBoxUI*			GetFolderButton() const {return pFolderButton;};
		/**
		 * @brief 获取Dotted行
		 * @details 用于获取Dotted行。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CLabelUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CLabelUI*				GetDottedLine() const {return pDottedLine;};
		/**
		 * @brief 获取CheckBox
		 * @details 用于获取CheckBox。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CCheckBoxUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CCheckBoxUI*			GetCheckBox() const {return pCheckBox;};
		/**
		 * @brief 获取子项按钮
		 * @details 用于获取子项按钮。具体行为由当前对象状态以及传入参数共同决定。
		 * @return COptionUI* 返回结果对象指针，失败时返回 nullptr
		 */
		COptionUI*				GetItemButton() const {return pItemButton;};

	private:
		/**
		 * @brief 获取Last节点
		 * @details 用于获取Last节点。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CTreeNodeUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CTreeNodeUI* GetLastNode();
		/**
		 * @brief 执行 CalLocation 操作
		 * @details 用于执行 CalLocation 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _pTreeNodeUI [in] p树节点UI参数
		 * @return CTreeNodeUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CTreeNodeUI* CalLocation(CTreeNodeUI* _pTreeNodeUI);

	private:
		long	m_iTreeLavel;
		bool	m_bIsVisable;
		bool	m_bIsCheckBox;
		DWORD	m_dwItemTextColor;
		DWORD	m_dwItemHotTextColor;
		DWORD	m_dwSelItemTextColor;
		DWORD	m_dwSelItemHotTextColor;

		CTreeViewUI*			pTreeView;
		CHorizontalLayoutUI*	pHoriz;
		CCheckBoxUI*			pFolderButton;
		CLabelUI*				pDottedLine;
		CCheckBoxUI*			pCheckBox;
		COptionUI*				pItemButton;
		CTreeNodeUI*			pParentTreeNode;
		CStdPtrArray			mTreeNodes;
	};

	class FYUI_API CTreeViewUI : public CListUI,public INotifyUI
	{
		DECLARE_DUICONTROL(CTreeViewUI)
	public:
		/**
		 * @brief 构造 CTreeViewUI 对象
		 * @details 用于构造 CTreeViewUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CTreeViewUI(void);
		/**
		 * @brief 析构 CTreeViewUI 对象
		 * @details 用于析构 CTreeViewUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CTreeViewUI(void);

	public:
		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		virtual LPVOID	GetInterface(std::wstring_view pstrName);

		/**
		 * @brief 获取列表类型
		 * @details 用于获取列表类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetListType();
		/**
		 * @brief 执行 Add 操作
		 * @details 用于执行 Add 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Add(CTreeNodeUI* pControl );
		/**
		 * @brief 添加At
		 * @details 用于添加At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @return long 返回 添加At 的结果
		 */
		virtual long AddAt(CTreeNodeUI* pControl, int iIndex );
		/**
		 * @brief 添加At
		 * @details 用于添加At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param _IndexNode [in] 索引节点参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool AddAt(CTreeNodeUI* pControl,CTreeNodeUI* _IndexNode);
		/**
		 * @brief 执行 Remove 操作
		 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Remove(CTreeNodeUI* pControl);
		/**
		 * @brief 移除At
		 * @details 用于移除At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool RemoveAt(int iIndex);
		/**
		 * @brief 移除全部子项
		 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void RemoveAll();
		/**
		 * @brief 执行 OnCheckBoxChanged 操作
		 * @details 用于执行 OnCheckBoxChanged 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool OnCheckBoxChanged(void* param);
		/**
		 * @brief 执行 OnFolderChanged 操作
		 * @details 用于执行 OnFolderChanged 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool OnFolderChanged(void* param);
		/**
		 * @brief 执行 OnDBClickItem 操作
		 * @details 用于执行 OnDBClickItem 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool OnDBClickItem(void* param);
		/**
		 * @brief 设置子项CheckBox
		 * @details 用于设置子项CheckBox。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _Selected [in] Selected参数
		 * @param _TreeNode [in] 树节点参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool SetItemCheckBox(bool _Selected,CTreeNodeUI* _TreeNode = NULL);
		/**
		 * @brief 设置子项展开
		 * @details 用于设置子项展开。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _Expanded [in] Expanded参数
		 * @param _TreeNode [in] 树节点参数
		 */
		virtual void SetItemExpand(bool _Expanded,CTreeNodeUI* _TreeNode = NULL);
		/**
		 * @brief 执行 Notify 操作
		 * @details 用于执行 Notify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in,out] msg参数
		 */
		virtual void Notify(TNotifyUI& msg);
		/**
		 * @brief 设置可见状态FolderBtn
		 * @details 用于设置可见状态FolderBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _IsVisibled [in] 判断是否Visibled参数
		 */
		virtual void SetVisibleFolderBtn(bool _IsVisibled);
		/**
		 * @brief 获取可见状态FolderBtn
		 * @details 用于获取可见状态FolderBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool GetVisibleFolderBtn();
		/**
		 * @brief 设置可见状态CheckBtn
		 * @details 用于设置可见状态CheckBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _IsVisibled [in] 判断是否Visibled参数
		 */
		virtual void SetVisibleCheckBtn(bool _IsVisibled);
		/**
		 * @brief 获取可见状态CheckBtn
		 * @details 用于获取可见状态CheckBtn。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool GetVisibleCheckBtn();
		/**
		 * @brief 设置子项最小宽度
		 * @details 用于设置子项最小宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _ItemMinWidth [in] 子项最小宽度参数
		 */
		virtual void SetItemMinWidth(UINT _ItemMinWidth);
		/**
		 * @brief 获取子项最小宽度
		 * @details 用于获取子项最小宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetItemMinWidth();
		/**
		 * @brief 设置子项文本颜色
		 * @details 用于设置子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _dwItemTextColor [in] dw子项文本颜色参数
		 */
		virtual void SetItemTextColor(DWORD _dwItemTextColor);
		/**
		 * @brief 设置子项热状态文本颜色
		 * @details 用于设置子项热状态文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _dwItemHotTextColor [in] dw子项热状态文本颜色参数
		 */
		virtual void SetItemHotTextColor(DWORD _dwItemHotTextColor);
		/**
		 * @brief 设置选区子项文本颜色
		 * @details 用于设置选区子项文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _dwSelItemTextColor [in] dw选区子项文本颜色参数
		 */
		virtual void SetSelItemTextColor(DWORD _dwSelItemTextColor);
		/**
		 * @brief 设置选区子项热状态文本颜色
		 * @details 用于设置选区子项热状态文本颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _dwSelHotItemTextColor [in] dw选区热状态子项文本颜色参数
		 */
		virtual void SetSelItemHotTextColor(DWORD _dwSelHotItemTextColor);

		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
	private:
		UINT m_uItemMinWidth;
		bool m_bVisibleFolderBtn;
		bool m_bVisibleCheckBtn;
	};
}




