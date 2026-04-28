#pragma once
namespace FYUI
{
	class FYUI_API CButtonUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CButtonUI)

	public:
		CButtonUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetControlFlags() const;

		bool Activate();
		void SetEnabled(bool bEnable = true);
		void DoEvent(TEventUI& event);

		virtual std::wstring_view GetNormalImage() const;
		virtual void SetNormalImage(std::wstring_view pStrImage);
		virtual std::wstring_view GetHotImage() const;
		virtual void SetHotImage(std::wstring_view pStrImage);
		virtual std::wstring_view GetPushedImage() const;
		virtual void SetPushedImage(std::wstring_view pStrImage);
		virtual std::wstring_view GetFocusedImage() const;
		virtual void SetFocusedImage(std::wstring_view pStrImage);
		virtual std::wstring_view GetDisabledImage() const;
		virtual void SetDisabledImage(std::wstring_view pStrImage);
		virtual std::wstring_view GetHotForeImage() const;
		virtual void SetHotForeImage(std::wstring_view pStrImage);
		virtual std::wstring_view GetPushedForeImage() const;
		virtual void SetPushedForeImage(std::wstring_view pStrImage);
		void SetStateCount(int nCount);
		int GetStateCount() const;
		void SetState(UINT uButtonState);
		virtual std::wstring_view GetStateImage() const;
		virtual void SetStateImage(std::wstring_view pStrImage);

		void BindTabIndex(int _BindTabIndex);
		void BindTabLayoutName(std::wstring_view _TabLayoutName);
		void BindTriggerTabSel(int _SetSelectIndex = -1);
		void RemoveBindTabIndex();
		int	 GetBindTabLayoutIndex();
		std::wstring_view GetBindTabLayoutName() const;

		void SetHotFont(int index);
		int GetHotFont() const;
		void SetPushedFont(int index);
		int GetPushedFont() const;
		void SetFocusedFont(int index);
		int GetFocusedFont() const;

		void SetHotBkColor(DWORD dwColor);
		DWORD GetHotBkColor() const;
		void SetPushedBkColor(DWORD dwColor);
		DWORD GetPushedBkColor() const;
		void SetDisabledBkColor(DWORD dwColor);
		DWORD GetDisabledBkColor() const;
		void SetHotTextColor(DWORD dwColor);
		DWORD GetHotTextColor() const;
		void SetPushedTextColor(DWORD dwColor);
		DWORD GetPushedTextColor() const;
		void SetFocusedTextColor(DWORD dwColor);
		DWORD GetFocusedTextColor() const;

		void SetHotBorderColor(DWORD dwColor);
		DWORD GetHotBorderColor() const;
		void SetPushedBorderColor(DWORD dwColor);
		DWORD GetPushedBorderColor() const;
		void SetDisabledBorderColor(DWORD dwColor);
		DWORD GetDisabledBorderColor() const;

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void PaintText(CPaintRenderContext& renderContext) override;

		void PaintBkColor(CPaintRenderContext& renderContext) override;
		void PaintStatusImage(CPaintRenderContext& renderContext) override;
		void PaintBorder(CPaintRenderContext& renderContext) override;
		void PaintForeImage(CPaintRenderContext& renderContext) override;

		void DrawBorder(CPaintRenderContext& renderContext, const RECT& rcItem, const DWORD& dwBorderColor, const int& nBorderSize, const RECT& rcBorderSize, const SIZE& cxyBorderRound, const int& nBorderStyle);

		virtual CButtonUI* Clone();
		virtual void CopyData(CButtonUI* pControl) ;
		bool IsContainerControl() const override
		{
			return false;
		}
	protected:
		UINT m_uButtonState;

		int		m_iHotFont;
		int		m_iPushedFont;
		int		m_iFocusedFont;

		DWORD m_dwHotBkColor;
		DWORD m_dwPushedBkColor;
		DWORD m_dwDisabledBkColor;
		DWORD m_dwHotTextColor;
		DWORD m_dwPushedTextColor;
		DWORD m_dwFocusedTextColor;
		DWORD m_dwHotBorderColor;
		DWORD m_dwPushedBorderColor;
		DWORD m_dwDisabledBorderColor;

		std::wstring m_sNormalImage;
		std::wstring m_sHotImage;
		std::wstring m_sHotForeImage;
		std::wstring m_sPushedImage;
		std::wstring m_sPushedForeImage;
		std::wstring m_sFocusedImage;
		std::wstring m_sDisabledImage;
		int m_nStateCount;
		std::wstring m_sStateImage;

		int			m_iBindTabIndex;
		std::wstring	m_sBindTabLayoutName;
	};

}	


