#pragma once

namespace FYUI
{
	class FYUI_API CProgressUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CProgressUI)
	public:
		CProgressUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		bool IsShowText();
		void SetShowText(bool bShowText = true);
		bool IsHorizontal();
		void SetHorizontal(bool bHorizontal = true);
		bool IsStretchForeImage();
		void SetStretchForeImage(bool bStretchForeImage = true);
		int GetMinValue() const;
		void SetMinValue(int nMin);
		int GetMaxValue() const;
		void SetMaxValue(int nMax);
		int GetValue() const;
		void SetValue(int nValue);
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void PaintForeColor(CPaintRenderContext& renderContext) override;
		void PaintForeImage(CPaintRenderContext& renderContext) override;
		virtual void UpdateText();
		void SetForeImagePadding(SIZE szForeImage);

		virtual CProgressUI* Clone();
		virtual void CopyData(CProgressUI* pControl) ;


	protected:
		bool m_bShowText;
		bool m_bHorizontal;
		bool m_bStretchForeImage;
		int m_nMax;
		int m_nMin;
		int m_nValue;
		SIZE m_szForeImage;
		std::wstring m_sForeImageModify;
	};

} 


