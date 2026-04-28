#pragma once

#include <chrono>
#include <string_view>

namespace FYUI
{
	/////////////////////////////////////////////////////////////////////////////////////
	//

	class IContainerUI
	{
	public:
		virtual CControlUI* GetItemAt(int iIndex) const = 0;
		virtual int GetItemIndex(CControlUI* pControl) const = 0;
		virtual bool SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate = true) = 0;
		virtual int GetCount() const = 0;
		virtual bool Add(CControlUI* pControl) = 0;
		virtual bool AddAt(CControlUI* pControl, int iIndex) = 0;
		virtual bool Remove(CControlUI* pControl, bool bChildDelayed = true) = 0;
		virtual bool RemoveAt(int iIndex, bool bChildDelayed = true) = 0;
		virtual void RemoveAll(bool bChildDelayed = true) = 0;
	};


	/////////////////////////////////////////////////////////////////////////////////////
	//
	class CScrollBarUI;

	class FYUI_API CContainerUI : public CControlUI, public IContainerUI
	{
		DECLARE_DUICONTROL(CContainerUI)

	public:
		CContainerUI();
		virtual ~CContainerUI();

	public:
		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		CControlUI* GetItemAt(int iIndex) const;
		int GetItemIndex(CControlUI* pControl) const;
		bool SetItemIndex(CControlUI* pControl, int iIndex, bool bUpdate = true);
		int GetCount() const;
		bool Add(CControlUI* pControl);
		bool AddAt(CControlUI* pControl, int iIndex);
		bool Remove(CControlUI* pControl, bool bChildDelayed = true);
		bool RemoveAt(int iIndex, bool bChildDelayed = true);
		void RemoveAll(bool bChildDelayed = true);

		void DoEvent(TEventUI& event);
		void SetVisible(bool bVisible = true, bool bSendFocus = true);
		void SetInternVisible(bool bVisible = true);
		void SetEnabled(bool bEnabled);
		void SetMouseEnabled(bool bEnable = true);

		virtual RECT GetInset() const;
		virtual void SetInset(RECT rcInset); // 璁剧疆鍐呰竟璺濓紝鐩稿綋浜庤缃鎴峰尯
		virtual int GetChildPadding() const;
		virtual void SetChildPadding(int iPadding);
		virtual UINT GetChildAlign() const;
		virtual void SetChildAlign(UINT iAlign);
		virtual UINT GetChildVAlign() const;
		virtual void SetChildVAlign(UINT iVAlign);
		virtual bool IsAutoDestroy() const;
		virtual void SetAutoDestroy(bool bAuto);
		virtual bool IsDelayedDestroy() const;
		virtual void SetDelayedDestroy(bool bDelayed);


		virtual bool IsMouseChildEnabled() const;
		virtual void SetMouseChildEnabled(bool bEnable = true);
		bool IsContainerControl() const override
		{
			return true;
		}

		virtual CContainerUI* Clone();
		virtual void CopyData(CContainerUI* pControl);

		void SetHScrollSpace(int cx);
		void SetVScrollSpace(int cy);

		void SetCursorMouse(bool bCursorMouse);

		bool IsFixedScrollbar();
		void SetFixedScrollbar(bool bFixed);

		bool IsShowScrollbar();
		void SetShowScrollbar(bool bShow);

		void SetSmoothScrollbar(bool bSmooth);
		bool IsSmoothScrollbar();

		void SetQuickScrolling(bool bQuickScrolling);
		bool GetQuickScrolling();

		virtual int FindSelectable(int iIndex, bool bForward = true) const;

		RECT GetClientPos() const;
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void Move(SIZE szOffset, bool bNeedInvalidate = true);
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void SetSepImmBorderColor(DWORD dwSepImmBorderColor);
		DWORD GetSepImmBorderColor() const;
		void SetSepImmLeaveBorderColor(DWORD dwSepImmBorderColor);
		DWORD GetSepImmLeaveBorderColor() const;

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
		CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);

		//浼樺寲鐗堟湰FindSubControl锛岃姣斾箣鍓嶇増鏈殑FindSubControl蹇?0鍊嶉€熷害浠ヤ笂
		CControlUI* FindSubControl(CControlUI* pRoot, std::wstring_view strSubControlName);
		CControlUI* FindSubControl(CControlUI* pRoot, std::wstring_view strSubControlName, std::wstring_view strEliminateControlName);
		bool SetSubControlText(std::wstring_view pstrSubControlName, std::wstring_view pstrText);
		bool SetSubControlFixedHeight(std::wstring_view pstrSubControlName, int cy);
		bool SetSubControlFixedWdith(std::wstring_view pstrSubControlName, int cx);
		bool SetSubControlUserData(std::wstring_view pstrSubControlName, std::wstring_view pstrText);
		bool SetSubControlAttribute(std::wstring_view pstrSubControlName, std::wstring_view pstrName, std::wstring_view pstrValue);

		std::wstring GetSubControlText(std::wstring_view pstrSubControlName);
		int GetSubControlFixedHeight(std::wstring_view pstrSubControlName);
		int GetSubControlFixedWdith(std::wstring_view pstrSubControlName);
		std::wstring GetSubControlUserData(std::wstring_view pstrSubControlName);
		/*
		** 鏍规嵁鍚嶅瓧鏌ユ壘瀛愭帶浠讹紝杩斿洖鎵惧埌鐨勭涓€涓帶浠舵寚閽?
		*  param pstrSubControlName: 瀛愭帶浠跺悕瀛?
		*  param strEliminateControlName: 鎺掗櫎鏌愪釜鍚嶅瓧鐨勬帶浠?涓嶄細鍦ㄨ鍚嶇О鎺т欢杩涜瀛愭帶浠舵煡鎵?
		*  return CControlUI*: 鎵惧埌鐨勫瓙鎺т欢鎸囬拡锛屾湭鎵惧埌杩斿洖nullptr
		*/
		CControlUI* FindSubControl(std::wstring_view pstrSubControlName, std::wstring_view strEliminateControlName = {});

		/*
		** 鏍规嵁鍚嶅瓧鏌ユ壘瀛愭帶浠讹紝鍙煡鎵?m_items 閲岀殑瀛愭帶浠讹紝杩斿洖鎵惧埌鐨勭涓€涓帶浠舵寚閽?
		*  param pstrChildControlName: 瀛愭帶浠跺悕瀛?
		*  return CControlUI*: 鎵惧埌鐨勫瓙鎺т欢鎸囬拡锛屾湭鎵惧埌杩斿洖nullptr
		*/
		virtual CControlUI* FindItmeControl(std::wstring_view pstrChildControlName);

		virtual SIZE GetScrollPos() const;
		virtual SIZE GetScrollRange() const;
		virtual void SetScrollPos(SIZE szPos, bool bMsg = true, bool bScroolVisible = true);
		virtual void SetScrollStepSize(int nSize);
		virtual int GetScrollStepSize() const;
		virtual void LineUp(bool bScroolVisible = true);
		virtual void LineDown(bool bScroolVisible = true);
		virtual void PageUp();
		virtual void PageDown();
		virtual void HomeUp();
		virtual void EndDown();
		virtual void LineLeft();
		virtual void LineRight();
		virtual void PageLeft();
		virtual void PageRight();
		virtual void HomeLeft();
		virtual void EndRight();
		virtual void EnableScrollBar(bool bEnableVertical = true, bool bEnableHorizontal = false);
		virtual CScrollBarUI* GetVerticalScrollBar() const;
		virtual CScrollBarUI* GetHorizontalScrollBar() const;
	public:

		DWORD GetNowTime();


		void   InertiaTick();
		void   ApplyWheelAcceleration(int direction, bool bScroolVisible);
		void   PostWheelAnimationMessageFromTimer();
		bool   HandleWheelAnimationFrameMessage();

		void StopWheelAnimation();           // 鍋滄鏂扮殑婊氬姩鍔ㄧ敾

	public:
		UINT m_uState = 0;       ///< 璁板綍褰撳墠鎺т欢鐨勪氦浜掔姸鎬侊紙Normal/Hover/Pushed锛?

	protected:
		virtual void SetFloatPos(int iIndex);
		virtual void ProcessScrollBar(RECT rc, int cxRequired, int cyRequired);


	protected:

		CStdPtrArray m_items;
		RECT m_rcInset;
		int m_iChildPadding;
		int m_nScrollStepSize;
		UINT m_iChildAlign;
		UINT m_iChildVAlign;
		bool m_bAutoDestroy;
		bool m_bDelayedDestroy;
		bool m_bMouseChildEnabled;
		bool m_bFixedScrollbar;
		bool m_bShowScrollbar;
		bool m_bSmoothScrollbar;
		bool m_bCursorMouse;
		bool m_bQuickScrolling = false;



		CScrollBarUI* m_pVerticalScrollBar;
		CScrollBarUI* m_pHorizontalScrollBar;
		std::wstring m_sVerticalScrollBarStyle;
		std::wstring m_sHorizontalScrollBarStyle;

		// Inertial scroll members
	protected:

		DWORD m_dwSepImmBorderColor = 0;
		DWORD m_dwLeaveSepImmBorderColor = 0;

		int  m_nWheelAnimPerTick{ 0 };         // 姣忔瀹氭椂鍣ㄥ熀纭€浣嶇Щ
		int  m_nWheelAnimRemainder{ 0 };       // 浣欐暟(鐢ㄤ簬鍧囧寑鍒嗛厤)
		int  m_nWheelAnimRemainingTicks{ 0 };  // 鍓╀綑 tick 娆℃暟
		bool m_bWheelAnimating{ false };       // 褰撳墠鏄惁澶勪簬婊氬姩鍔ㄧ敾
		bool m_bWheelScrollVisible{ false };   // 璁板綍婊氬姩鏉″綋鍓嶅彲瑙佹€?鍥炶皟鏃朵娇鐢?

		// 鏂板锛氱疮璁＄洰鏍囦笌宸叉粴鍔?
		int  m_nWheelAnimTargetTotalY{ 0 };    // 绱鐩爣鎬讳綅绉伙紙鍙鍙礋锛?
		int  m_nWheelAnimMovedY{ 0 };          // 宸茬粡鎵ц鐨勬€讳綅绉?
		UINT m_uWheelAnimationTimerId{ 0 };
		volatile LONG m_lWheelAnimationMessagePending{ 0 };

		bool   m_bInertiaActive;

		static const UINT kInertiaTimerId = 0x7A11; // 鎯€у畾鏃跺櫒ID

		

	};

} 


