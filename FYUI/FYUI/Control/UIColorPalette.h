#pragma once

#include "../Core/Render/UIRenderSurface.h"

namespace FYUI 
{
	/////////////////////////////////////////////////////////////////////////////////////
	//
	class FYUI_API CColorPaletteUI : public CControlUI
	{
		DECLARE_DUICONTROL(CColorPaletteUI)
	public:
		CColorPaletteUI();
		virtual ~CColorPaletteUI();

		//鑾峰彇鏈€缁堣閫夋嫨鐨勯鑹诧紝鍙互鐩存帴鐢ㄤ簬璁剧疆duilib鑳屾櫙鑹?
		DWORD GetSelectColor();
		void SetSelectColor(DWORD dwColor);

		virtual std::wstring_view GetClass() const;
		virtual LPVOID GetInterface(std::wstring_view pstrName);
		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		//璁剧疆/鑾峰彇 Pallet锛堣皟鑹叉澘涓荤晫闈級鐨勯珮搴?
		void SetPalletHeight(int nHeight);
		int	GetPalletHeight() const;

		//璁剧疆/鑾峰彇 涓嬫柟Bar锛堜寒搴﹂€夋嫨鏍忥級鐨勯珮搴?
		void SetBarHeight(int nHeight);
		int GetBarHeight() const;

		//璁剧疆/鑾峰彇 閫夋嫨鍥炬爣鐨勮矾寰?
		void SetPalletImage(std::wstring_view pszImage);
		std::wstring_view GetPalletImage() const;
		void SetBarImage(std::wstring_view pszImage);
		std::wstring_view GetBarImage() const;
		void SetColorBarImage(std::wstring_view pszImage);
		std::wstring_view GetColorBarImage() const;

		void SetPalletImageSize(SIZE szPalletImage);
		void SetBarImageSize(SIZE szBarImage);
		SIZE GetPalletImageSize();
		SIZE GetBarImageSize();

		void SetBarHorMove(int move);
		void SetBarVerMove(int move);

		virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
		virtual void DoInit();
		virtual void DoEvent(TEventUI& event);
		void PaintForeImage(CPaintRenderContext& renderContext) override;
		void PaintPallet(CPaintRenderContext& renderContext);

		void NotifyColorChanged();



	protected:
		//鏇存柊鏁版嵁
		bool EnsurePaletteSurface();
		void UpdatePalletData();
		void UpdateBarData();

	private:
		CPaintRenderSurface m_paletteSurface;
		UINT		m_uButtonState;

		bool		m_activeHandleHueBar = true; // 榛樿閫変腑涓嬫柟 Hue 鏉?
		bool		m_bIsInBar;
		bool		m_bPaletteDataDirty;
		bool		m_bBarDataDirty;

		bool		m_bIsInPallet;
		bool        m_bSetColor =false;
		int			m_nCurH;
		int			m_nCurS;
		int			m_nCurB;

		int			m_nPalletHeight;
		int			m_nBarHeight;
		CDuiPoint		m_ptLastPalletMouse;
		CDuiPoint		m_ptLastBarMouse;
		std::wstring  m_strPalletImage;
		std::wstring  m_strBarImage;
		std::wstring  m_strColorBarImage;

		SIZE m_szPalletImage;
		SIZE m_szBarImage;
	};
}

