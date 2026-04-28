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
		virtual void AddReceiver(MenuMenuReceiverImplBase* receiver) = 0;
		virtual void RemoveReceiver(MenuMenuReceiverImplBase* receiver) = 0;
		virtual BOOL RBroadcast(ContextMenuParam param) = 0;
	};
	/////////////////////////////////////////////////
	class MenuMenuReceiverImplBase
	{
	public:
		virtual void AddObserver(MenuMenuObserverImplBase* observer) = 0;
		virtual void RemoveObserver() = 0;
		virtual BOOL Receive(ContextMenuParam param) = 0;
	};
	/////////////////////////////////////////////////

	class MenuReceiverImpl;
	class FYUI_API MenuObserverImpl : public MenuMenuObserverImplBase
	{
		friend class Iterator;
	public:
		MenuObserverImpl():
			m_pMainWndPaintManager(NULL),
			m_pMenuCheckInfo(NULL)
		{
			pReceivers_ = new ReceiversVector;
		}

		~MenuObserverImpl()
		{
			if (pReceivers_ != NULL)
			{
				delete pReceivers_;
				pReceivers_ = NULL;
			}

		}

		virtual void AddReceiver(MenuMenuReceiverImplBase* receiver)
		{
			if (receiver == NULL)
				return;

			pReceivers_->push_back(receiver);
			receiver->AddObserver(this);
		}

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

		virtual void SetManger(CPaintManagerUI* pManager)
		{
			if (pManager != NULL)
				m_pMainWndPaintManager = pManager;
		}

		virtual CPaintManagerUI* GetManager() const
		{
			return m_pMainWndPaintManager;
		}

		virtual void SetMenuCheckInfo(MenuCheckInfoMap* pInfo)
		{
			if (pInfo != NULL)
				m_pMenuCheckInfo = pInfo;
			else
				m_pMenuCheckInfo = NULL;
		}

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
		MenuReceiverImpl()
		{
			pObservers_ = new ObserversVector;
		}

		~MenuReceiverImpl()
		{
			if (pObservers_ != NULL)
			{
				delete pObservers_;
				pObservers_ = NULL;
			}
		}

		virtual void AddObserver(MenuMenuObserverImplBase* observer)
		{
			pObservers_->push_back(observer);
		}

		virtual void RemoveObserver()
		{
			if (!pObservers_) return;

			ObserversVector::iterator it = pObservers_->begin();
			for (; it != pObservers_->end(); ++it)
			{
				(*it)->RemoveReceiver(this);
			}
		}

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
		CMenuUI();
		virtual ~CMenuUI();
		CMenuWnd*	m_pWindow;
		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetListType();

		virtual void DoEvent(TEventUI& event);

		virtual bool Add(CControlUI* pControl);
		virtual bool AddAt(CControlUI* pControl, int iIndex);

		virtual int GetItemIndex(CControlUI* pControl) const;
		virtual bool SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate = true);
		virtual bool Remove(CControlUI* pControl);

		SIZE EstimateSize(SIZE szAvailable);

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class CMenuElementUI;
	class FYUI_API CMenuWnd : public CWindowWnd, public MenuReceiverImpl, public INotifyUI, public IDialogBuilderCallback
	{
	public:
		static MenuObserverImpl& GetGlobalContextMenuObserver()
		{
			static MenuObserverImpl s_context_menu_observer;
			return s_context_menu_observer;
		}
		static CMenuWnd* CreateMenu(CMenuElementUI* pOwner, STRINGorID xml, POINT point,
			CPaintManagerUI* pMainPaintManager, MenuCheckInfoMap* pMenuCheckInfo = NULL,
			DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top);
		static void DestroyMenu();
		static MenuItemInfo* SetMenuItemInfo(std::wstring_view pstrName, bool bChecked);

	public:
		CMenuWnd();
		~CMenuWnd();
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

		void Init(CMenuElementUI* pOwner, STRINGorID xml, POINT point,
			CPaintManagerUI* pMainPaintManager, MenuCheckInfoMap* pMenuCheckInfo = NULL,
			DWORD dwAlignment = eMenuAlignment_Left | eMenuAlignment_Top);
		std::wstring_view GetWindowClassName() const override;
		void OnFinalMessage(HWND hWnd);
		void Notify(TNotifyUI& msg);
		CControlUI* CreateControl(std::wstring_view pstrClassName);

		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

		BOOL Receive(ContextMenuParam param);

		// 閼惧嘲褰囬弽纭呭綅閸楁洘甯舵禒璁圭礉閻劋绨崝銊︹偓浣瑰潑閸旂姴鐡欓懣婊冨礋
		CMenuUI* GetMenuUI();

		// 闁插秵鏌婄拫鍐╂殻閼挎粌宕熼惃鍕亣鐏?
		void ResizeMenu();

		// 闁插秵鏌婄拫鍐╂殻鐎涙劘褰嶉崡鏇犳畱婢堆冪毈
		void ResizeSubMenu();
		void setDPI(int DPI);
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
		CMenuElementUI();
		~CMenuElementUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		void DrawItemText(CPaintRenderContext& renderContext, const RECT& rcItem);
		SIZE EstimateSize(SIZE szAvailable);

		void DoEvent(TEventUI& event);

		CMenuWnd* GetMenuWnd();
		void CreateMenuWnd();

		void SetLineType();
		void SetLineColor(DWORD color);
		DWORD GetLineColor() const;
		void SetLinePadding(RECT rcInset);
		RECT GetLinePadding() const;
		void SetIcon(std::wstring_view strIcon);
		void SetIconSize(LONG cx, LONG cy);
		SIZE GetIconSize();
		void DrawItemIcon(CPaintRenderContext& renderContext, const RECT& rcItem);
		void SetChecked(bool bCheck = true);
		bool GetChecked() const;
		void SetCheckItem(bool bCheckItem = false);
		bool GetCheckItem() const;

		void SetSubMenuSpacing(int nSpacing);

		void SetShowExplandIcon(bool bShow);
		void DrawItemExpland(CPaintRenderContext& renderContext, const RECT& rcItem);

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		MenuItemInfo* GetItemInfo(std::wstring_view pstrName);
		MenuItemInfo* GetItemInfo(const std::wstring& pstrName) { return GetItemInfo(std::wstring_view(pstrName)); }
		MenuItemInfo* SetItemInfo(std::wstring_view pstrName, bool bChecked);
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


