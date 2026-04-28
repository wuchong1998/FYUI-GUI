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


	public:
		UINT m_uState = 0;      

	protected:
		virtual void SetFloatPos(int iIndex);
		virtual void ProcessScrollBar(RECT rc, int cxRequired, int cyRequired);
		RECT GetScrollViewRect() const;
		SIZE ClampScrollPos(SIZE szPos) const;
		int GetWheelScrollStep(bool bHorizontal) const;
		bool CanUseScrollRenderCache(const RECT& rcScrollView) const;
		void ApplyScrollPos(SIZE szPos, bool bMsg, bool bScroolVisible);
		void QueueSmoothScroll(SIZE szTarget, bool bMsg = true, bool bScroolVisible = true);
		void ScrollByDelta(int cxDelta, int cyDelta, bool bMsg = true, bool bScroolVisible = true, bool bPreferSmooth = true);
		bool AdvanceSmoothScroll();
		void StopSmoothScroll(bool bSnapToTarget = false);


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
		bool m_bQuickScrolling = true;



		CScrollBarUI* m_pVerticalScrollBar;
		CScrollBarUI* m_pHorizontalScrollBar;
		std::wstring m_sVerticalScrollBarStyle;
		std::wstring m_sHorizontalScrollBarStyle;

	protected:

		DWORD m_dwSepImmBorderColor = 0;
		DWORD m_dwLeaveSepImmBorderColor = 0;

		
		bool   m_bInertiaActive;
		bool m_bSmoothScrollAnimating = false;
		bool m_bApplyingSmoothScroll = false;
		bool m_bSmoothScrollNotify = true;
		bool m_bSmoothScrollVisible = true;
		bool m_bSmoothScrollPeriodRaised = false;
		SIZE m_szSmoothScrollTarget = { 0, 0 };
		double m_fSmoothScrollPosX = 0.0;
		double m_fSmoothScrollPosY = 0.0;
		double m_fWheelRemainderX = 0.0;
		double m_fWheelRemainderY = 0.0;
		std::chrono::steady_clock::time_point m_smoothScrollLastTick;

		enum : UINT
		{
			SMOOTH_SCROLL_TIMERID = 0x5F10,
			SMOOTH_SCROLL_INTERVAL_MS = 4,
		};

	};

} 


