#pragma once
#include "UIButton.h"

namespace FYUI
{
	class FYUI_API COptionUI : public CButtonUI
	{
		DECLARE_DUICONTROL(COptionUI)
	public:
		COptionUI();
		virtual ~COptionUI();

		std::wstring_view GetClass() const;
		LPVOID GetInterface(std::wstring_view pstrName);

		void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);

		bool Activate();

		std::wstring_view GetSelectedImage();
		void SetSelectedImage(std::wstring_view pStrImage);

		std::wstring_view GetSelectedHotImage();
		void SetSelectedHotImage(std::wstring_view pStrImage);

		std::wstring_view GetSelectedPushedImage();
		void SetSelectedPushedImage(std::wstring_view pStrImage);

		void SetSelectedTextColor(DWORD dwTextColor);
		DWORD GetSelectedTextColor();

		void SetSelectedBkColor(DWORD dwBkColor);
		DWORD GetSelectBkColor();

		void SetSelectedDisableBkColor(DWORD dwBkColor);
		DWORD GetSelectedDisableBkColor();

		std::wstring_view GetSelectedForedImage();
		void SetSelectedForedImage(std::wstring_view pStrImage);

		void SetSelectedStateCount(int nCount);
		int GetSelectedStateCount() const;
		virtual std::wstring_view GetSelectedStateImage();
		virtual void SetSelectedStateImage(std::wstring_view pStrImage);

		void SetSelectedFont(int index);
		int GetSelectedFont() const;

		std::wstring_view GetGroup() const;
		void SetGroup(std::wstring_view pStrGroupName = {});
		std::wstring_view GetGroupType() const;
		void SetGroupType(std::wstring_view pStrGroupType = {});

		bool IsSelected() const;
		virtual void Selected(bool bSelected, bool bMsg = true);

		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		void PaintBkColor(CPaintRenderContext& renderContext) override;
		void PaintStatusImage(CPaintRenderContext& renderContext) override;
		void PaintForeImage(CPaintRenderContext& renderContext) override;
		void PaintText(CPaintRenderContext& renderContext) override;

		virtual COptionUI* Clone();
		virtual void CopyData(COptionUI* pControl) ;

	protected:
		bool			m_bSelected;
		std::wstring		m_sGroupName;
		std::wstring		m_sGroupType;

		int				m_iSelectedFont;

		DWORD			m_dwSelectedBkColor;
		DWORD			m_dwSelectedDisableBkColor;
		DWORD			m_dwSelectedTextColor;

		std::wstring		m_sSelectedImage;
		std::wstring		m_sSelectedHotImage;
		std::wstring		m_sSelectedPushedImage;
		std::wstring		m_sSelectedForeImage;

		int m_nSelectedStateCount;
		std::wstring m_sSelectedStateImage;
	};

	class FYUI_API CCheckBoxUI : public COptionUI
	{
		DECLARE_DUICONTROL(CCheckBoxUI)
	public:
		CCheckBoxUI();

	public:
		virtual std::wstring_view GetClass() const;
		virtual LPVOID GetInterface(std::wstring_view pstrName);

		void SetCheck(bool bCheck);
		bool GetCheck() const;

	public:
		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void SetAutoCheck(bool bEnable);
		virtual void DoEvent(TEventUI& event);
		virtual void Selected(bool bSelected, bool bMsg = true);

		virtual CCheckBoxUI* Clone();
		virtual void CopyData(CCheckBoxUI* pControl) ;

	protected:
		bool m_bAutoCheck; 
	};
};

