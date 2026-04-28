#pragma once
namespace FYUI
{
	class FYUI_API CScrollBarUI : public CControlUI
	{
		DECLARE_DUICONTROL(CScrollBarUI)
	public:
		CScrollBarUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		CContainerUI* GetOwner() const;
		void SetOwner(CContainerUI* pOwner);

		void SetVisible(bool bVisible = true);
		void SetEnabled(bool bEnable = true);
		void SetFocus();

		bool IsHorizontal();
		void SetHorizontal(bool bHorizontal = true);
		int GetScrollRange() const;
		void SetScrollRange(int nRange, bool bIsSetMax = true);
		int GetScrollPos() const;
		void SetScrollPos(int nPos, bool bIsSetMax = true);
		int GetLineSize() const;
		void SetLineSize(int nSize);

		bool GetMouseDown() { return m_bMouseDown; }

		bool GetShowButton1();
		void SetShowButton1(bool bShow);
		std::wstring_view GetButton1NormalImage() const;
		void SetButton1NormalImage(std::wstring_view pStrImage);
		std::wstring_view GetButton1HotImage() const;
		void SetButton1HotImage(std::wstring_view pStrImage);
		std::wstring_view GetButton1PushedImage() const;
		void SetButton1PushedImage(std::wstring_view pStrImage);
		std::wstring_view GetButton1DisabledImage() const;
		void SetButton1DisabledImage(std::wstring_view pStrImage);

		bool GetShowButton2();
		void SetShowButton2(bool bShow);
		std::wstring_view GetButton2NormalImage() const;
		void SetButton2NormalImage(std::wstring_view pStrImage);
		std::wstring_view GetButton2HotImage() const;
		void SetButton2HotImage(std::wstring_view pStrImage);
		std::wstring_view GetButton2PushedImage() const;
		void SetButton2PushedImage(std::wstring_view pStrImage);
		std::wstring_view GetButton2DisabledImage() const;
		void SetButton2DisabledImage(std::wstring_view pStrImage);

		std::wstring_view GetThumbNormalImage() const;
		void SetThumbNormalImage(std::wstring_view pStrImage);
		std::wstring_view GetThumbHotImage() const;
		void SetThumbHotImage(std::wstring_view pStrImage);
		std::wstring_view GetThumbPushedImage() const;
		void SetThumbPushedImage(std::wstring_view pStrImage);
		std::wstring_view GetThumbDisabledImage() const;
		void SetThumbDisabledImage(std::wstring_view pStrImage);

		std::wstring_view GetRailNormalImage() const;
		void SetRailNormalImage(std::wstring_view pStrImage);
		std::wstring_view GetRailHotImage() const;
		void SetRailHotImage(std::wstring_view pStrImage);
		std::wstring_view GetRailPushedImage() const;
		void SetRailPushedImage(std::wstring_view pStrImage);
		std::wstring_view GetRailDisabledImage() const;
		void SetRailDisabledImage(std::wstring_view pStrImage);

		std::wstring_view GetBkNormalImage() const;
		void SetBkNormalImage(std::wstring_view pStrImage);
		std::wstring_view GetBkHotImage() const;
		void SetBkHotImage(std::wstring_view pStrImage);
		std::wstring_view GetBkPushedImage() const;
		void SetBkPushedImage(std::wstring_view pStrImage);
		std::wstring_view GetBkDisabledImage() const;
		void SetBkDisabledImage(std::wstring_view pStrImage);

		bool GetShow();
		void SetShow(bool bShow);

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void DoEvent(TEventUI& event);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;

		void PaintBk(CPaintRenderContext& renderContext);
		void PaintButton1(CPaintRenderContext& renderContext);
		void PaintButton2(CPaintRenderContext& renderContext);
		void PaintThumb(CPaintRenderContext& renderContext);
		void PaintRail(CPaintRenderContext& renderContext);
		void SetHSpace(int cx);
		void SetVSpace(int cy);

		int GetHSpace() const;
		int GetVSpace() const;

	protected:
		enum
		{
			DEFAULT_SCROLLBAR_SIZE = 16,
			DEFAULT_TIMERID = 10,
		};

		bool m_bMouseDown = false;

		bool m_bShow;
		bool m_bHorizontal;
		__int64 m_nRange;
		__int64 m_nScrollPos;
		int m_nLineSize;
		CContainerUI* m_pOwner;
		POINT m_ptLastMouse;
		int m_nLastScrollPos;
		int m_nLastScrollOffset;
		int m_nScrollRepeatDelay;
		int m_nSpaceX;
		int m_nSpaceY;

		std::wstring m_sBkNormalImage;
		std::wstring m_sBkHotImage;
		std::wstring m_sBkPushedImage;
		std::wstring m_sBkDisabledImage;

		bool m_bShowButton1;
		RECT m_rcButton1;
		UINT m_uButton1State;
		std::wstring m_sButton1NormalImage;
		std::wstring m_sButton1HotImage;
		std::wstring m_sButton1PushedImage;
		std::wstring m_sButton1DisabledImage;

		bool m_bShowButton2;
		RECT m_rcButton2;
		UINT m_uButton2State;
		std::wstring m_sButton2NormalImage;
		std::wstring m_sButton2HotImage;
		std::wstring m_sButton2PushedImage;
		std::wstring m_sButton2DisabledImage;

		RECT m_rcThumb;
		UINT m_uThumbState;
		std::wstring m_sThumbNormalImage;
		std::wstring m_sThumbHotImage;
		std::wstring m_sThumbPushedImage;
		std::wstring m_sThumbDisabledImage;

		std::wstring m_sRailNormalImage;
		std::wstring m_sRailHotImage;
		std::wstring m_sRailPushedImage;
		std::wstring m_sRailDisabledImage;

		std::wstring m_sImageModify;
	};
}

