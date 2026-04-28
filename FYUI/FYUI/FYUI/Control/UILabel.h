#pragma once
namespace FYUI
{
	class FYUI_API CLabelUI : public CControlUI
	{
		DECLARE_DUICONTROL(CLabelUI)
	public:
		CLabelUI();
		~CLabelUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetControlFlags() const;

		void SetTextStyle(UINT uStyle);
		UINT GetTextStyle() const;
		void SetTextColor(DWORD dwTextColor);
		DWORD GetTextColor() const;
		void SetDisabledTextColor(DWORD dwTextColor);
		DWORD GetDisabledTextColor() const;
		void SetFont(int index);
		int GetFont() const;
		RECT GetTextPadding() const;
		void SetTextPadding(RECT rc);
		bool IsShowHtml();
		void SetShowHtml(bool bShowHtml = true);
		void SetFollowSize(bool bFollowSize);
		bool GetFollowSize() const;


		SIZE EstimateSize(SIZE szAvailable);
		void DoEvent(TEventUI& event);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void PaintText(CPaintRenderContext& renderContext) override;

		virtual bool GetAutoCalcWidth() const;
		virtual void SetAutoCalcWidth(bool bAutoCalcWidth);
		virtual bool GetAutoCalcHeight() const;
		virtual void SetAutoCalcHeight(bool bAutoCalcHeight);
		using CControlUI::SetText;
		virtual void SetText(std::wstring_view pstrText) override;


		virtual CLabelUI* Clone();
		virtual void CopyData(CLabelUI* pControl);

		bool IsContainerControl() const override
		{
			return false;
		}

	protected:
		DWORD	m_dwTextColor;
		DWORD	m_dwDisabledTextColor;
		int		m_iFont;
		UINT	m_uTextStyle;
		RECT	m_rcTextPadding;
		bool	m_bShowHtml;
		bool	m_bAutoCalcWidth;
		bool	m_bAutoCalcHeight;
		bool    m_bFollowSize = false;
		SIZE    m_szAvailableLast;
		SIZE    m_cxyFixedLast;
		bool    m_bNeedEstimateSize;
	};

}

