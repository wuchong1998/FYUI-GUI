#pragma once

namespace FYUI
{
	class FYUI_API CVerticalLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CVerticalLayoutUI)
	public:
		CVerticalLayoutUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);
		UINT GetControlFlags() const;

		void SetSepHeight(int iHeight);
		int GetSepHeight() const;
		void SetSepImmMode(bool bImmediately);
		bool IsSepImmMode() const;
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void DoEvent(TEventUI& event);

		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void DoPostPaint(CPaintRenderContext& renderContext) override;

		RECT GetThumbRect(bool bUseNew = false) const;

		virtual CVerticalLayoutUI* Clone();
		virtual void CopyData(CVerticalLayoutUI* pControl) ;

		bool IsContainerControl() const override
		{
			return true;
		}
	protected:
		int m_iSepHeight;
		UINT m_uButtonState;
		POINT ptLastMouse;
		RECT m_rcNewPos;
		bool m_bImmMode;

	};
}

