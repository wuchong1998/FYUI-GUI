#pragma once
#include <map>

namespace FYUI
{

	class FYUI_API CShadowUI
	{
	public:
		friend class CPaintManagerUI;

		CShadowUI(void);
		virtual ~CShadowUI(void);

	public:
		// bShow涓虹湡鏃舵墠浼氬垱寤洪槾褰?
		void ShowShadow(bool bShow);	
		bool IsShowShadow() const;

		void DisableShadow(bool bDisable);
		bool IsDisableShadow() const;

		// 绠楁硶闃村奖鐨勫嚱鏁?
		bool SetSize(int NewSize = 0);
		bool SetSharpness(unsigned int NewSharpness = 5);
		bool SetDarkness(unsigned int NewDarkness = 200);
		bool SetPosition(int NewXOffset = 5, int NewYOffset = 5);
		bool SetColor(COLORREF NewColor = 0);

		// 鍥剧墖闃村奖鐨勫嚱鏁?
		bool SetImage(std::wstring_view szImage);
		bool SetShadowCorner(RECT rcCorner);	// 涔濆鏍兼柟寮忔弿杩伴槾褰?

		// 鎶婅嚜宸辩殑闃村奖鏍峰紡澶嶅埗鍒颁紶鍏ュ弬鏁?
		bool CopyShadow(CShadowUI* pShadow);

		//	鍒涘缓闃村奖绐椾綋锛岀敱CPaintManagerUI鑷姩璋冪敤,闄ら潪鑷繁瑕佸崟鐙垱寤洪槾褰?
		void Create(CPaintManagerUI* pPaintManager);
	protected:

		//	鍒濆鍖栧苟娉ㄥ唽闃村奖绫?
		void RefreshScaledMetrics();
		void EnsureScaledMetricsUpToDate();
		int ScaleMetric(int value) const;
		RECT ScaleMetric(RECT rcValue) const;
		static bool Initialize(HINSTANCE hInstance);

		// 淇濆瓨宸茬粡闄勫姞鐨勭獥浣撳彞鏌勫拰涓庡叾鍏宠仈鐨勯槾褰辩被,鏂逛究鍦≒arentProc()鍑芥暟涓€氳繃鍙ユ焺寰楀埌闃村奖绫?
		static std::map<HWND, CShadowUI *>& GetShadowMap();

		//	瀛愮被鍖栫埗绐椾綋
		static LRESULT CALLBACK ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		// 鐖剁獥浣撴敼鍙樺ぇ灏忥紝绉诲姩锛屾垨鑰呬富鍔ㄩ噸缁橀槾褰辨椂璋冪敤
		void Update(HWND hParent);

		// 閫氳繃绠楁硶璁＄畻闃村奖
		void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent);

		// 璁＄畻alpha棰勪箻鍊?
		inline DWORD PreMultiply(COLORREF cl, unsigned char nAlpha)
		{
			return (GetRValue(cl) * (DWORD)nAlpha / 255) |
				(GetGValue(cl) * (DWORD)nAlpha / 255) << 8 |
				(GetBValue(cl) * (DWORD)nAlpha / 255) << 16 ;
		}

	protected:
		enum ShadowStatus
		{
			SS_ENABLED = 1,				// Shadow is enabled, if not, the following one is always false
			SS_VISABLE = 1 << 1,		// Shadow window is visible
			SS_PARENTVISIBLE = 1<< 2	// Parent window is visible, if not, the above one is always false
		};


		static bool s_bHasInit;

		CPaintManagerUI	*m_pManager;		// 鐖剁獥浣撶殑CPaintManagerUI锛岀敤鏉ヨ幏鍙栫礌鏉愯祫婧愬拰鐖剁獥浣撳彞鏌?
		HWND			 m_hWnd;			// 闃村奖绐椾綋鐨勫彞鏌?
		LONG_PTR		 m_OriParentProc;	// 瀛愮被鍖栫埗绐椾綋
		BYTE			 m_Status;
		bool			 m_bIsImageMode;	// 鏄惁涓哄浘鐗囬槾褰辨ā寮?
		bool			 m_bIsShowShadow;	// 鏄惁瑕佹樉绀洪槾褰?
		bool			m_bIsDisableShadow;
		// 绠楁硶闃村奖鎴愬憳鍙橀噺
		unsigned char m_nDarkness;	// Darkness, transparency of blurred area
		int m_nSharpness;	// Current device-pixel sharpness
		int m_nSize;	// Current device-pixel shadow size

		// The X and Y offsets of shadow window,
		// relative to the parent window, at center of both windows (not top-left corner), signed
		int m_nxOffset;
		int m_nyOffset;
		int m_nLogicalSharpness;
		int m_nLogicalSize;
		int m_nLogicalXOffset;
		int m_nLogicalYOffset;
		ULONGLONG m_uAppliedDpiGeneration;

		// Restore last parent window size, used to determine the update strategy when parent window is resized
		LPARAM m_WndSize;

		// Set this to true if the shadow should not be update until next WM_PAINT is received
		bool m_bUpdate;

		COLORREF m_Color;	// Color of shadow

		// 鍥剧墖闃村奖鎴愬憳鍙橀噺
		std::wstring	m_sShadowImage;
		RECT		m_rcShadowCorner;
		RECT		m_rcShadowCornerLogical;
	};

}


