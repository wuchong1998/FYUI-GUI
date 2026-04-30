#pragma once

#include "../Utils/observer_impl_base.h"
#include <map>

namespace FYUI {

	struct ContextMenuParam
	{
		// 1: remove all
		// 2: remove the sub menu
		WPARAM wParam;
		HWND hWnd;
	};

	struct MenuItemInfo
	{
		wchar_t szName[256];
		bool bChecked;
	};
	using MenuCheckInfoMap = std::map<std::wstring, MenuItemInfo*, std::less<>>;
	struct MenuCmd
	{
		wchar_t szName[256];
		wchar_t szUserData[1024];
		wchar_t szText[1024];
		BOOL bChecked;
	};

	enum MenuAlignment
	{
		eMenuAlignment_Left = 1 << 1,
		eMenuAlignment_Top = 1 << 2,
		eMenuAlignment_Right = 1 << 3,
		eMenuAlignment_Bottom = 1 << 4,
	};


	enum MenuItemDefaultInfo
	{
		ITEM_DEFAULT_HEIGHT = 30,		//濮ｅ繋绔存稉鐚em閻ㄥ嫰绮拋銈夌彯鎼达讣绱欓崣顏勬躬缁旀牜濮搁幒鎺戝灙閺冩儼鍤滅€规矮绠熼敍?
		ITEM_DEFAULT_WIDTH = 150,		//缁愭褰涢惃鍕帛鐠併倕顔旀惔?

		ITEM_DEFAULT_ICON_WIDTH = 26,	//姒涙顓婚崶鐐垼閹碘偓閸楃姴顔旀惔?
		ITEM_DEFAULT_ICON_SIZE = 16,	//姒涙顓婚崶鐐垼閻ㄥ嫬銇囩亸?

		ITEM_DEFAULT_EXPLAND_ICON_WIDTH = 20,	//姒涙顓绘稉瀣獓閼挎粌宕熼幍鈺佺潔閸ョ偓鐖ｉ幍鈧崡鐘差啍鎼?
		ITEM_DEFAULT_EXPLAND_ICON_SIZE = 9,		//姒涙顓绘稉瀣獓閼挎粌宕熼幍鈺佺潔閸ョ偓鐖ｉ惃鍕亣鐏?

		DEFAULT_LINE_LEFT_INSET = ITEM_DEFAULT_ICON_WIDTH + 3,	//姒涙顓婚崚鍡涙缁捐法娈戝锕佺珶鐠?
		DEFAULT_LINE_RIGHT_INSET = 7,	//姒涙顓婚崚鍡涙缁捐法娈戦崣瀹犵珶鐠?
		DEFAULT_LINE_HEIGHT = 6,		//姒涙顓婚崚鍡涙缁炬寧澧嶉崡鐘荤彯鎼?
		DEFAULT_LINE_COLOR = 0xFFBCBFC4	//姒涙顓婚崚鍡涙缁惧潡顤侀懝?

	};

	///////////////////////////////////////////////
	class MenuMenuReceiverImplBase;
	class MenuMenuObserverImplBase
	{
	public:
		/**
		 * @brief 添加Receiver
		 * @details 用于添加Receiver。具体行为由当前对象状态以及传入参数共同决定。
		 * @param receiver [in] receiver参数
		 */
		virtual void AddReceiver(MenuMenuReceiverImplBase* receiver) = 0;
		/**
		 * @brief 移除Receiver
		 * @details 用于移除Receiver。具体行为由当前对象状态以及传入参数共同决定。
		 * @param receiver [in] receiver参数
		 */
		virtual void RemoveReceiver(MenuMenuReceiverImplBase* receiver) = 0;
		/**
		 * @brief 执行 RBroadcast 操作
		 * @details 用于执行 RBroadcast 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return BOOL 返回 执行 RBroadcast 操作 的结果
		 */
		virtual BOOL RBroadcast(ContextMenuParam param) = 0;
	};
	/////////////////////////////////////////////////
	class MenuMenuReceiverImplBase
	{
	public:
		/**
		 * @brief 添加Observer
		 * @details 用于添加Observer。具体行为由当前对象状态以及传入参数共同决定。
		 * @param observer [in] observer参数
		 */
		virtual void AddObserver(MenuMenuObserverImplBase* observer) = 0;
		/**
		 * @brief 移除Observer
		 * @details 用于移除Observer。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void RemoveObserver() = 0;
		/**
		 * @brief 执行 Receive 操作
		 * @details 用于执行 Receive 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return BOOL 返回 执行 Receive 操作 的结果
		 */
		virtual BOOL Receive(ContextMenuParam param) = 0;
	};
	/////////////////////////////////////////////////

	class MenuReceiverImpl;
	class FYUI_API MenuObserverImpl : public MenuMenuObserverImplBase
	{
		friend class Iterator;
	public:
		/**
		 * @brief 构造 MenuObserverImpl 对象
		 * @details 用于构造 MenuObserverImpl 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		MenuObserverImpl():
			m_pMainWndPaintManager(NULL),
			m_pMenuCheckInfo(NULL)
		{
			pReceivers_ = new ReceiversVector;
		}

		/**
		 * @brief 析构 MenuObserverImpl 对象
		 * @details 用于析构 MenuObserverImpl 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~MenuObserverImpl()
		{
			if (pReceivers_ != NULL)
			{
				delete pReceivers_;
				pReceivers_ = NULL;
			}

		}

		/**
		 * @brief 添加Receiver
		 * @details 用于添加Receiver。具体行为由当前对象状态以及传入参数共同决定。
		 * @param receiver [in] receiver参数
		 */
		virtual void AddReceiver(MenuMenuReceiverImplBase* receiver)
		{
			if (receiver == NULL)
				return;

			pReceivers_->push_back(receiver);
			receiver->AddObserver(this);
		}

		/**
		 * @brief 移除Receiver
		 * @details 用于移除Receiver。具体行为由当前对象状态以及传入参数共同决定。
		 * @param receiver [in] receiver参数
		 */
		virtual void RemoveReceiver(MenuMenuReceiverImplBase* receiver)
		{
			if (receiver == NULL)
				return;

			ReceiversVector::iterator it = pReceivers_->begin();
			for (; it != pReceivers_->end(); ++it)
			{
				if (*it == receiver)
				{
					pReceivers_->erase(it);
					break;
				}
			}
		}

		/**
		 * @brief 执行 RBroadcast 操作
		 * @details 用于执行 RBroadcast 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return BOOL 返回 执行 RBroadcast 操作 的结果
		 */
		virtual BOOL RBroadcast(ContextMenuParam param)
		{
			ReceiversVector::reverse_iterator it = pReceivers_->rbegin();
			for (; it != pReceivers_->rend(); ++it)
			{
				(*it)->Receive(param);
			}

			return BOOL();
		}


		class Iterator
		{
			MenuObserverImpl & _tbl;
			DWORD index;
			MenuMenuReceiverImplBase* ptr;
		public:
			/**
			 * @brief 构造 Iterator 对象
			 * @details 用于构造 Iterator 对象。具体行为由当前对象状态以及传入参数共同决定。
			 * @param table [in,out] table参数
			 */
			Iterator( MenuObserverImpl & table )
				: _tbl( table ), index(0), ptr(NULL)
			{}

			Iterator( const Iterator & v )
				: _tbl( v._tbl ), index(v.index), ptr(v.ptr)
			{}

			MenuMenuReceiverImplBase* next()
			{
				if ( index >= _tbl.pReceivers_->size() )
					return NULL;

				for ( ; index < _tbl.pReceivers_->size(); )
				{
					ptr = (*(_tbl.pReceivers_))[ index++ ];
					if ( ptr )
						return ptr;
				}
				return NULL;
			}
		};

		/**
		 * @brief 设置Manger
		 * @details 用于设置Manger。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 */
		virtual void SetManger(CPaintManagerUI* pManager)
		{
			if (pManager != NULL)
				m_pMainWndPaintManager = pManager;
		}

		/**
		 * @brief 获取管理器
		 * @details 用于获取管理器。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CPaintManagerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CPaintManagerUI* GetManager() const
		{
			return m_pMainWndPaintManager;
		}

		/**
		 * @brief 设置菜单Check信息
		 * @details 用于设置菜单Check信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pInfo [in] 信息对象
		 */
		virtual void SetMenuCheckInfo(MenuCheckInfoMap* pInfo)
		{
			if (pInfo != NULL)
				m_pMenuCheckInfo = pInfo;
			else
				m_pMenuCheckInfo = NULL;
		}

		/**
		 * @brief 获取菜单Check信息
		 * @details 用于获取菜单Check信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @return MenuCheckInfoMap* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual MenuCheckInfoMap* GetMenuCheckInfo() const
		{
			return m_pMenuCheckInfo;
		}

	protected:
		typedef std::vector<MenuMenuReceiverImplBase*> ReceiversVector;
		ReceiversVector *pReceivers_;
		CPaintManagerUI* m_pMainWndPaintManager;
		MenuCheckInfoMap* m_pMenuCheckInfo;
	};

	////////////////////////////////////////////////////
	class FYUI_API MenuReceiverImpl : public MenuMenuReceiverImplBase
	{
	public:
		/**
		 * @brief 构造 MenuReceiverImpl 对象
		 * @details 用于构造 MenuReceiverImpl 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		MenuReceiverImpl()
		{
			pObservers_ = new ObserversVector;
		}

		/**
		 * @brief 析构 MenuReceiverImpl 对象
		 * @details 用于析构 MenuReceiverImpl 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~MenuReceiverImpl()
		{
			if (pObservers_ != NULL)
			{
				delete pObservers_;
				pObservers_ = NULL;
			}
		}

		/**
		 * @brief 添加Observer
		 * @details 用于添加Observer。具体行为由当前对象状态以及传入参数共同决定。
		 * @param observer [in] observer参数
		 */
		virtual void AddObserver(MenuMenuObserverImplBase* observer)
		{
			pObservers_->push_back(observer);
		}

		/**
		 * @brief 移除Observer
		 * @details 用于移除Observer。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void RemoveObserver()
		{
			if (!pObservers_) return;

			ObserversVector::iterator it = pObservers_->begin();
			for (; it != pObservers_->end(); ++it)
			{
				(*it)->RemoveReceiver(this);
			}
		}

		/**
		 * @brief 执行 Receive 操作
		 * @details 用于执行 Receive 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return BOOL 返回 执行 Receive 操作 的结果
		 */
		virtual BOOL Receive(ContextMenuParam param)
		{
			return BOOL();
		}

	protected:
		typedef std::vector<MenuMenuObserverImplBase*> ObserversVector;
		ObserversVector* pObservers_;
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CListUI;
	class CMenuWnd;
	class FYUI_API CMenuUI : public CListUI
	{
		DECLARE_DUICONTROL(CMenuUI)
	public:
		/**
		 * @brief 构造 CMenuUI 对象
		 * @details 用于构造 CMenuUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CMenuUI();
		/**
		 * @brief 析构 CMenuUI 对象
		 * @details 用于析构 CMenuUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CMenuUI();
		CMenuWnd*	m_pWindow;
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
		 * @brief 获取列表类型
		 * @details 用于获取列表类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetListType();

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void DoEvent(TEventUI& event);

		/**
		 * @brief 执行 Add 操作
		 * @details 用于执行 Add 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Add(CControlUI* pControl);
		/**
		 * @brief 添加At
		 * @details 用于添加At。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool AddAt(CControlUI* pControl, int iIndex);

		/**
		 * @brief 获取子项索引
		 * @details 用于获取子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return int 返回对应的数值结果
		 */
		virtual int GetItemIndex(CControlUI* pControl) const;
		/**
		 * @brief 设置子项索引
		 * @details 用于设置子项索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @param bUpdate [in] 是否更新
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate = true);
		/**
		 * @brief 执行 Remove 操作
		 * @details 用于执行 Remove 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Remove(CControlUI* pControl);

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
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CMenuElementUI;
	class FYUI_API CMenuWnd : public CWindowWnd, public MenuReceiverImpl, public INotifyUI, public IDialogBuilderCallback
	{
	public:
		/**
		 * @brief 获取Global上下文菜单Observer
		 * @details 用于获取Global上下文菜单Observer。具体行为由当前对象状态以及传入参数共同决定。
		 * @return MenuObserverImpl& 返回 获取Global上下文菜单Observer 的结果
		 */
		static MenuObserverImpl& GetGlobalContextMenuObserver()
		{
			static MenuObserverImpl s_context_menu_observer;
			return s_context_menu_observer;
		}
		/**
		 * @brief 创建菜单
		 * @details 用于创建菜单。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 * @param xml [in] xml参数
		 * @param point [in] point参数
		 * @param pMainPaintManager [in] Main绘制管理器对象
		 * @param pMenuCheckInfo [in] 菜单Check信息对象
		 * @param dwAlignment [in] Alignment数值
		 * @return CMenuWnd* 返回结果对象指针，失败时返回 nullptr
		 */
		static CMenuWnd* CreateMenu(CMenuElementUI* pOwner, STRINGorID xml, POINT point,
			CPaintManagerUI* pMainPaintManager, MenuCheckInfoMap* pMenuCheckInfo = NULL,
			DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top);
		/**
		 * @brief 销毁菜单
		 * @details 用于销毁菜单。具体行为由当前对象状态以及传入参数共同决定。
		 */
		static void DestroyMenu();
		/**
		 * @brief 设置菜单子项信息
		 * @details 用于设置菜单子项信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param bChecked [in] 是否Checked
		 * @return MenuItemInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		static MenuItemInfo* SetMenuItemInfo(std::wstring_view pstrName, bool bChecked);

	public:
		/**
		 * @brief 构造 CMenuWnd 对象
		 * @details 用于构造 CMenuWnd 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CMenuWnd();
		/**
		 * @brief 析构 CMenuWnd 对象
		 * @details 用于析构 CMenuWnd 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CMenuWnd();
		/**
		 * @brief 执行 Close 操作
		 * @details 用于执行 Close 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nRet [in] Ret数值
		 */
		void Close(UINT nRet = IDOK);
		bool isClosing;
		/*
		*	@pOwner 娑撯偓缁狙嗗綅閸楁洑绗夌憰浣瑰瘹鐎规俺绻栨稉顏勫棘閺佸府绱濇潻娆愭Ц閼挎粌宕熼崘鍛村劥娴ｈ法鏁ら惃?
		*	@xml	閼挎粌宕熼惃鍕鐏炩偓閺傚洣娆?
		*	@point	閼挎粌宕熼惃鍕箯娑撳﹨顫楅崸鎰垼
		*	@pMainPaintManager	閼挎粌宕熼惃鍕煑缁愭ぞ缍嬬粻锛勬倞閸ｃ劍瀵氶柦?
		*	@pMenuCheckInfo	娣囨繂鐡ㄩ懣婊冨礋閻ㄥ嫬宕熼柅澶婃嫲婢跺秹鈧淇婇幁顖滅波閺嬪嫭瀵氶柦?
		*	@dwAlignment		閼挎粌宕熼惃鍕毉閻滈缍呯純顕嗙礉姒涙顓婚崙铏瑰箛閸︺劑绱堕弽鍥╂畱閸欏厖绗呮笟褋鈧?
		*/

		/**
		 * @brief 初始化对象
		 * @details 用于初始化对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 * @param xml [in] xml参数
		 * @param point [in] point参数
		 * @param pMainPaintManager [in] Main绘制管理器对象
		 * @param pMenuCheckInfo [in] 菜单Check信息对象
		 * @param dwAlignment [in] Alignment数值
		 */
		void Init(CMenuElementUI* pOwner, STRINGorID xml, POINT point,
			CPaintManagerUI* pMainPaintManager, MenuCheckInfoMap* pMenuCheckInfo = NULL,
			DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top);
		/**
		 * @brief 获取窗口类名名称
		 * @details 用于获取窗口类名名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetWindowClassName() const override;
		/**
		 * @brief 执行 OnFinalMessage 操作
		 * @details 用于执行 OnFinalMessage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 */
		void OnFinalMessage(HWND hWnd);
		/**
		 * @brief 执行 Notify 操作
		 * @details 用于执行 Notify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in,out] msg参数
		 */
		void Notify(TNotifyUI& msg);
		/**
		 * @brief 创建控件
		 * @details 用于创建控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrClassName [in] 类名名称字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* CreateControl(std::wstring_view pstrClassName);

		/**
		 * @brief 执行 OnCreate 操作
		 * @details 用于执行 OnCreate 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnKillFocus 操作
		 * @details 用于执行 OnKillFocus 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnSize 操作
		 * @details 用于执行 OnSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 处理消息
		 * @details 用于处理消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @return LRESULT 返回对应的数值结果
		 */
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

		/**
		 * @brief 执行 Receive 操作
		 * @details 用于执行 Receive 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return BOOL 返回 执行 Receive 操作 的结果
		 */
		BOOL Receive(ContextMenuParam param);

		// 閼惧嘲褰囬弽纭呭綅閸楁洘甯舵禒璁圭礉閻劋绨崝銊︹偓浣瑰潑閸旂姴鐡欓懣婊冨礋
		/**
		 * @brief 获取菜单UI
		 * @details 用于获取菜单UI。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CMenuUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CMenuUI* GetMenuUI();

		// 闁插秵鏌婄拫鍐╂殻閼挎粌宕熼惃鍕亣鐏?
		/**
		 * @brief 执行 ResizeMenu 操作
		 * @details 用于执行 ResizeMenu 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResizeMenu();

		// 闁插秵鏌婄拫鍐╂殻鐎涙劘褰嶉崡鏇犳畱婢堆冪毈
		/**
		 * @brief 执行 ResizeSubMenu 操作
		 * @details 用于执行 ResizeSubMenu 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResizeSubMenu();
		/**
		 * @brief 执行 setDPI 操作
		 * @details 用于执行 setDPI 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param DPI [in] DPI参数
		 */
		void setDPI(int DPI);
		/**
		 * @brief 设置Sub菜单Spacing
		 * @details 用于设置Sub菜单Spacing。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSpacing [in] Spacing数值
		 */
		void SetSubMenuSpacing(int nSpacing);

	public:
		bool m_bCaptured;
		POINT			m_BasedPoint;
		STRINGorID		m_xml;
		CPaintManagerUI m_pm;
		CMenuElementUI* m_pOwner;
		CMenuUI*	m_pLayout;
		int m_nSpacing =0;
		DWORD		m_dwAlignment;	//閼挎粌宕熺€靛綊缍堥弬鐟扮础
		HWND hParentWnd;
	};

	class CListContainerElementUI;
	class FYUI_API CMenuElementUI : public CListContainerElementUI
	{
		DECLARE_DUICONTROL(CMenuElementUI)
		friend CMenuWnd;
	public:
		/**
		 * @brief 构造 CMenuElementUI 对象
		 * @details 用于构造 CMenuElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CMenuElementUI();
		/**
		 * @brief 析构 CMenuElementUI 对象
		 * @details 用于析构 CMenuElementUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CMenuElementUI();

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
		 * @brief 设置所属对象
		 * @details 用于设置所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		void SetOwner(CControlUI* pOwner) override;
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/**
		 * @brief 绘制子项文本
		 * @details 用于绘制子项文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem);
		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		SIZE EstimateSize(SIZE szAvailable);

		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);

		/**
		 * @brief 获取菜单Wnd
		 * @details 用于获取菜单Wnd。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CMenuWnd* 返回结果对象指针，失败时返回 nullptr
		 */
		CMenuWnd* GetMenuWnd();
		/**
		 * @brief 创建菜单Wnd
		 * @details 用于创建菜单Wnd。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void CreateMenuWnd();

		/**
		 * @brief 设置行类型
		 * @details 用于设置行类型。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SetLineType();
		/**
		 * @brief 设置行颜色
		 * @details 用于设置行颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param color [in] 颜色参数
		 */
		void SetLineColor(DWORD color);
		/**
		 * @brief 获取行颜色
		 * @details 用于获取行颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetLineColor() const;
		/**
		 * @brief 设置行内边距
		 * @details 用于设置行内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcInset [in] Inset矩形区域
		 */
		void SetLinePadding(RECT rcInset);
		/**
		 * @brief 获取行内边距
		 * @details 用于获取行内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetLinePadding() const;
		/**
		 * @brief 设置图标
		 * @details 用于设置图标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param strIcon [in] 图标字符串
		 */
		void SetIcon(std::wstring_view strIcon);
		/**
		 * @brief 设置图标尺寸
		 * @details 用于设置图标尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void SetIconSize(LONG cx, LONG cy);
		/**
		 * @brief 获取图标尺寸
		 * @details 用于获取图标尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetIconSize();
		/**
		 * @brief 绘制子项图标
		 * @details 用于绘制子项图标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		void DrawItemIcon(CPaintRenderContext& renderContext, const RECT& rcItem);
		/**
		 * @brief 设置Checked
		 * @details 用于设置Checked。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bCheck [in] 是否Check
		 */
		void SetChecked(bool bCheck = true);
		/**
		 * @brief 获取Checked
		 * @details 用于获取Checked。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetChecked() const;
		/**
		 * @brief 设置Check子项
		 * @details 用于设置Check子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bCheckItem [in] 是否Check子项
		 */
		void SetCheckItem(bool bCheckItem = false);
		/**
		 * @brief 获取Check子项
		 * @details 用于获取Check子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetCheckItem() const;

		/**
		 * @brief 设置Sub菜单Spacing
		 * @details 用于设置Sub菜单Spacing。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSpacing [in] Spacing数值
		 */
		void SetSubMenuSpacing(int nSpacing);

		/**
		 * @brief 设置显示Expland图标
		 * @details 用于设置显示Expland图标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShow [in] 是否显示
		 */
		void SetShowExplandIcon(bool bShow);
		/**
		 * @brief 绘制子项Expland
		 * @details 用于绘制子项Expland。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param rcItem [in] 子项矩形区域
		 */
		void DrawItemExpland(CPaintRenderContext& renderContext, const RECT& rcItem);

		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 获取子项信息
		 * @details 用于获取子项信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return MenuItemInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		MenuItemInfo* GetItemInfo(std::wstring_view pstrName);
		/**
		 * @brief 获取子项信息
		 * @details 用于获取子项信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return MenuItemInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		MenuItemInfo* GetItemInfo(const std::wstring& pstrName) { return GetItemInfo(std::wstring_view(pstrName)); }
		MenuItemInfo* SetItemInfo(std::wstring_view pstrName, bool bChecked);
		/**
		 * @brief 设置子项信息
		 * @details 用于设置子项信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param bChecked [in] 是否Checked
		 * @return MenuItemInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		MenuItemInfo* SetItemInfo(const std::wstring& pstrName, bool bChecked) { return SetItemInfo(std::wstring_view(pstrName), bChecked); }
	protected:
		CMenuWnd*	m_pWindow;

		bool		m_bDrawLine;	//閻㈣鍨庨梾鏃傚殠
		DWORD		m_dwLineColor;  //閸掑棝娈х痪鍧楊杹閼?
		RECT		m_rcLinePadding;	//閸掑棗澹婄痪璺ㄦ畱瀹革箑褰告潏纭呯獩

		SIZE		m_szIconSize; 	//閻㈣娴橀弽?
		std::wstring	m_strIcon;
		bool		m_bCheckItem;
		int m_nSpacing;
		bool		m_bShowExplandIcon;
	};

} 


