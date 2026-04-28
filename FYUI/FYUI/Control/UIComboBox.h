#pragma once

namespace FYUI
{
	class FYUI_API CComboBoxUI : public CComboUI
	{
		DECLARE_DUICONTROL(CComboBoxUI)
	public:
		CComboBoxUI();
		std::wstring_view GetClass() const;

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void PaintText(CPaintRenderContext& renderContext) override;
		void PaintStatusImage(CPaintRenderContext& renderContext) override;

	protected:
		std::wstring m_sArrowImage;
		int        m_nArrowWidth;
	};
}



