#pragma once

namespace FYUI
{
	class FYUI_API CButtonExUI : public CButtonUI
	{
		DECLARE_DUICONTROL(CButtonExUI)

	public:
		CButtonExUI();

		std::wstring_view GetClass() const;
		LPVOID  GetInterface(std::wstring_view pstrName);
		void    PaintText(CPaintRenderContext& renderContext) override;
		void    DrawTextExBtn(CPaintRenderContext& renderContext, RECT& rc, std::wstring_view pstrText, DWORD dwTextColor, int iFont, UINT uStyle);
		void    DrawTextExBtn(CPaintRenderContext& renderContext, RECT& rc, const std::wstring& pstrText, DWORD dwTextColor, int iFont, UINT uStyle)
		{
			DrawTextExBtn(renderContext, rc, std::wstring_view(pstrText), dwTextColor, iFont, uStyle);
		}

		virtual CButtonExUI* Clone();
		virtual void CopyData(CButtonExUI* pControl) ;

	};

}

