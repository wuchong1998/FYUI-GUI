#pragma once

namespace FYUI
{
	class FYUI_API CSliderUI : public CProgressUI
	{
		DECLARE_DUICONTROL(CSliderUI)
	public:
		CSliderUI();

		std::wstring_view GetClass() const;
		UINT GetControlFlags() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		void SetEnabled(bool bEnable = true);

		int GetChangeStep();
		void SetChangeStep(int step);
		void SetThumbSize(SIZE szXY);
		RECT GetThumbRect() const;
		std::wstring_view GetThumbImage() const;
		void SetThumbImage(std::wstring_view pStrImage);
		std::wstring_view GetThumbHotImage() const;
		void SetThumbHotImage(std::wstring_view pStrImage);
		std::wstring_view GetThumbPushedImage() const;
		void SetThumbPushedImage(std::wstring_view pStrImage);

		void DoEvent(TEventUI& event);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void PaintForeImage(CPaintRenderContext& renderContext) override;

		void SetValue(int nValue);
		void SetCanSendMove(bool bCanSend);
		bool GetCanSendMove() const;

		virtual CSliderUI* Clone();
		virtual void CopyData(CSliderUI* pControl) ;

	protected:
		SIZE m_szThumb;
		UINT m_uButtonState;
		int m_nStep;

		std::wstring m_sThumbImage;
		std::wstring m_sThumbHotImage;
		std::wstring m_sThumbPushedImage;

		std::wstring m_sImageModify;
		bool	   m_bSendMove;
	};
}


