#pragma once

#include <map>
#include <string>
#include <string_view>

namespace FYUI
{
	class CPaintRenderContext;

	typedef CControlUI* (CALLBACK* FINDCONTROLPROC)(CControlUI*, LPVOID);

	class FYUI_API CControlUI
	{
		DECLARE_DUICONTROL(CControlUI)

	public:
		CControlUI();
		virtual ~CControlUI();

	public:
		virtual const std::wstring& GetName() const;
		virtual const std::wstring& GetNameRef() const;
		virtual std::wstring_view GetNameView() const;
		virtual void SetName(std::wstring_view name);
		virtual std::wstring_view GetClass() const;
		virtual LPVOID GetInterface(std::wstring_view pstrName);
		virtual UINT GetControlFlags() const;

		virtual bool Activate();
		virtual CPaintManagerUI* GetManager() const;
		virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
		virtual CControlUI* GetParent() const;
		void setInstance(HINSTANCE instance = NULL);

		virtual bool IsContainerControl() const;
		virtual void RemoveAll(bool bChildDelayed);
		bool SetTimer(UINT nTimerID, UINT nElapse);
		void KillTimer(UINT nTimerID);

		virtual std::wstring GetText() const;
		virtual void SetText(std::wstring_view text);
		virtual std::wstring GetHotText() const;
		virtual void SetHotText(std::wstring_view text);
		virtual bool IsResourceText() const;
		virtual void SetResourceText(bool bResource);
		virtual bool IsDragEnabled() const;
		virtual void SetDragEnable(bool bDrag);
		virtual bool IsDropEnabled() const;
		virtual void SetDropEnable(bool bDrop);
		virtual bool IsRichEvent() const;
		virtual void SetRichEvent(bool bEnable);

		std::wstring_view GetGradient() const;
		void SetGradient(std::wstring_view image);
		DWORD GetBkColor() const;
		void SetBkColor(DWORD dwBackColor, bool bIsUpdate = true);
		DWORD GetBkColor2() const;
		void SetBkColor2(DWORD dwBackColor);
		DWORD GetBkColor3() const;
		void SetBkColor3(DWORD dwBackColor);
		void SetDisableBkColor(DWORD dwDisableBkColor);
		DWORD GetDisableBkColor() const;

		DWORD GetForeColor() const;
		void SetForeColor(DWORD dwForeColor);
		std::wstring_view GetBkImage() const;
		void SetBkImage(std::wstring_view image);
		std::wstring_view GetForeImage() const;
		void SetForeImage(std::wstring_view image);

		DWORD GetFocusBorderColor() const;
		void SetFocusBorderColor(DWORD dwBorderColor);
		DWORD GetFocusBKColor() const;
		void SetFocusBKColor(DWORD dwFocusBkColor);

		bool IsColorHSL() const;
		void SetColorHSL(bool bColorHSL);
		SIZE GetBorderRound() const;
		void SetBorderRound(SIZE cxyRound);
		bool DrawImage(CPaintRenderContext& renderContext, std::wstring_view pStrImage, std::wstring_view pStrModify = {});

		int GetBorderSize() const;
		void SetBorderSize(int nSize);
		DWORD GetBorderColor() const;
		void SetBorderColor(DWORD dwBorderColor);
		RECT GetBorderRectSize() const;
		void SetBorderSize(RECT rc);
		int GetLeftBorderSize() const;
		void SetLeftBorderSize(int nSize);
		int GetTopBorderSize() const;
		void SetTopBorderSize(int nSize);
		int GetRightBorderSize() const;
		void SetRightBorderSize(int nSize);
		int GetBottomBorderSize() const;
		void SetBottomBorderSize(int nSize);
		int GetBorderStyle() const;
		void SetBorderStyle(int nStyle);

		virtual RECT GetRelativePos() const;
		virtual RECT GetClientPos() const;
		virtual const RECT& GetPos() const;
		virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
		void SetOnlyPos(RECT& rc);
		virtual void Move(SIZE szOffset, bool bNeedInvalidate = true);
		virtual int GetWidth() const;
		virtual int GetHeight() const;
		virtual int GetX() const;
		virtual int GetY() const;

		virtual RECT GetPadding() const;
		virtual void SetPadding(RECT rcPadding, bool NeedUpdate = true);

		SIZE GetFixedXY() const;
		SIZE GetFixedSize() const;
		virtual void SetFixedXY(SIZE szXY);
		int GetFixedWidth() const;
		int GetFixedHeight() const;
		virtual void SetFixedWidth(int cx, bool NeedUpdate = true);
		virtual void SetFixedHeight(int cy, bool NeedUpdate = true);

		int GetMinWidth() const;
		int GetMinHeight() const;
		virtual void SetMinWidth(int cx);
		virtual void SetMinHeight(int cy);

		int GetMaxWidth() const;
		int GetMaxHeight() const;
		virtual void SetMaxWidth(int cx);
		virtual void SetMaxHeight(int cy);

		virtual TPercentInfo GetFloatPercent() const;
		virtual void SetFloatPercent(TPercentInfo piFloatPercent);
		virtual void SetFloatAlign(UINT uAlign);
		virtual UINT GetFloatAlign() const;

		virtual std::wstring GetToolTip() const;
		virtual void SetToolTip(std::wstring_view text);
		virtual void SetToolTipWidth(int nWidth);
		virtual int GetToolTipWidth(void);
		virtual void SetToolTipGap(SIZE szGap);
		virtual SIZE GetToolTipGap();
		virtual void SetToolTipShowMode(ToolTipType emToolType);
		virtual ToolTipType GetToolTipShowMode();

		virtual WORD GetCursor();
		virtual HCURSOR GetHCursor();
		virtual void SetCursor(WORD wCursor);
		virtual void SetCursor(HCURSOR hCursor);

		virtual wchar_t GetShortcut() const;
		virtual void SetShortcut(wchar_t ch);

		virtual bool IsContextMenuUsed() const;
		virtual void SetContextMenuUsed(bool bMenuUsed);

		virtual const std::wstring& GetUserData() const;
		virtual std::wstring_view GetUserDataView() const;
		virtual void SetUserData(std::wstring_view text);
		virtual void SetMapUserData(const std::wstring& strTextFirst, const std::wstring& pstrTextSecond);
		virtual std::wstring& GetMapUserData(const std::wstring& strTextFirst);
		virtual void ClearMapUserData();
		virtual void EraseMapUserData(const std::wstring& strTextFirst);

		virtual UINT_PTR GetTag() const;
		virtual void SetTag(UINT_PTR pTag);

		bool IsVisible();
		bool GetVisible() const;
		virtual void SetVisible(bool bVisible = true, bool bSendFocus = true);
		virtual void SetInternVisible(bool bVisible = true);
		virtual bool IsEnabled() const;
		virtual void SetEnabled(bool bEnable = true);
		virtual bool IsMouseEnabled() const;
		virtual void SetMouseEnabled(bool bEnable = true);
		virtual bool IsKeyboardEnabled() const;
		virtual void SetKeyboardEnabled(bool bEnable = true);
		virtual bool IsFocused() const;
		virtual void SetFocus();
		bool IsFloat() const;
		virtual void SetFloat(bool bFloat = true);
		virtual void SetGdiPlusDrawText(bool bGDIPlusDrawText);
		virtual bool GetGdiPlusDrawText() const;

		virtual CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);
		virtual CControlUI* Clone();
		virtual void CopyData(CControlUI* pControl);

		void Invalidate();
		bool IsUpdateNeeded() const;
		void NeedUpdate();
		void NeedParentUpdate();
		DWORD GetAdjustColor(DWORD dwColor);

		virtual void Init();
		virtual void DoInit();
		virtual void Event(TEventUI& event);
		virtual void DoEvent(TEventUI& event);

		void AddCustomAttribute(std::wstring_view name, std::wstring_view attr);
		std::wstring_view GetCustomAttribute(std::wstring_view name) const;
		bool RemoveCustomAttribute(std::wstring_view name);
		void RemoveAllCustomAttribute();

		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		CControlUI* ApplyAttributeList(std::wstring_view pstrList);

		virtual SIZE EstimateSize(SIZE szAvailable);
		virtual bool Paint(CPaintRenderContext& renderContext, CControlUI* pStopControl = NULL);
		virtual bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl);
		virtual void PaintBkColor(CPaintRenderContext& renderContext);
		virtual void PaintBkImage(CPaintRenderContext& renderContext);
		virtual void PaintStatusImage(CPaintRenderContext& renderContext);
		virtual void PaintForeColor(CPaintRenderContext& renderContext);
		virtual void PaintForeImage(CPaintRenderContext& renderContext);
		virtual void PaintText(CPaintRenderContext& renderContext);
		virtual void PaintBorder(CPaintRenderContext& renderContext);

		virtual void DoPostPaint(CPaintRenderContext& renderContext);

		void SetVirtualWnd(std::wstring_view value);
		std::wstring_view GetVirtualWnd() const;

	protected:
		int ScaleValue(int value) const;
		RECT ScaleRect(RECT rc) const;
		SIZE ScaleSize(SIZE sz) const;
		POINT ScalePoint(POINT pt) const;
		int PixelsToLogical(int value) const;
		RECT PixelsToLogical(RECT rc) const;
		SIZE PixelsToLogical(SIZE sz) const;
		POINT PixelsToLogical(POINT pt) const;
		void SetPaddingFromPixels(RECT rcPadding, bool NeedUpdate = true);
		void SetFixedWidthFromPixels(int cx, bool NeedUpdate = true);
		void SetFixedHeightFromPixels(int cy, bool NeedUpdate = true);
		ULONGLONG GetCurrentDPIGeneration() const;

	public:
		CEventSource OnInit;
		CEventSource OnDestroy;
		CEventSource OnSize;
		CEventSource OnEvent;
		CEventSource OnNotify;

	protected:
		CPaintManagerUI* m_pManager;
		CControlUI* m_pParent;
		std::wstring m_sVirtualWnd;
		std::wstring m_sName;
		bool m_bUpdateNeeded;
		bool m_bMenuUsed;
		RECT m_rcItem;
		RECT m_rcPadding;
		SIZE m_cXY;
		SIZE m_cxyFixed;
		SIZE m_cxyMin;
		SIZE m_cxyMax;
		SIZE m_szGap = { 0, 0 };

		mutable RECT m_rcPaddingScaled;
		mutable SIZE m_cXYScaled;
		mutable SIZE m_cxyFixedScaled;
		mutable SIZE m_cxyMinScaled;
		mutable SIZE m_cxyMaxScaled;
		mutable ULONGLONG m_uPaddingDpiGeneration = 0;
		mutable ULONGLONG m_uXYDpiGeneration = 0;
		mutable ULONGLONG m_uFixedDpiGeneration = 0;
		mutable ULONGLONG m_uMinDpiGeneration = 0;
		mutable ULONGLONG m_uMaxDpiGeneration = 0;

		bool m_bVisible;
		bool m_bInternVisible;
		bool m_bEnabled;
		bool m_bMouseEnabled;
		bool m_bKeyboardEnabled;
		bool m_bFocused;
		bool m_bFloat;
		bool m_bGdiPlusDrawText = false;
		TPercentInfo m_piFloatPercent;
		UINT m_uFloatAlign;
		bool m_bSetPos;

		bool m_bRichEvent;
		bool m_bDragEnabled;
		bool m_bDropEnabled;

		bool m_bResourceText;
		bool m_bContainerControl = false;

		std::wstring m_sText;
		std::wstring m_sHotText;
		std::wstring m_sToolTip;
		wchar_t m_chShortcut;
		std::wstring m_sUserData;
		UINT_PTR m_pTag;

		std::wstring m_sGradient;
		DWORD m_dwBackColor;
		DWORD m_dwBackColor2;
		DWORD m_dwBackColor3;
		DWORD m_dwDisableBkColor = 0;
		DWORD m_dwForeColor;
		std::wstring m_sBkImage;
		std::wstring m_sForeImage;

		DWORD m_dwBorderColor;
		DWORD m_dwFocusBorderColor;
		DWORD m_dwFocusBkColor;
		bool m_bColorHSL;
		int m_nBorderSize;
		int m_nBorderStyle;
		int m_nTooltipWidth;
		WORD m_wCursor;
		SIZE m_cxyBorderRound;
		RECT m_rcPaint;
		RECT m_rcBorderSize;
		HINSTANCE m_instance;
		HCURSOR m_Cursor = nullptr;

		std::map<std::wstring, std::wstring> m_mapUserData;
		std::map<std::wstring, std::wstring, std::less<>> m_mCustomAttrHash;

		ToolTipType m_emToolTipType = Tool_Bottom;
	};

} // namespace DuiLib
