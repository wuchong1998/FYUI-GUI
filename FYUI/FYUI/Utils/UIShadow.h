#pragma once
#include <map>

namespace FYUI
{

	class FYUI_API CShadowUI
	{
	public:
		friend class CPaintManagerUI;

		/**
		 * @brief 构造 CShadowUI 对象
		 * @details 用于构造 CShadowUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CShadowUI(void);
		/**
		 * @brief 析构 CShadowUI 对象
		 * @details 用于析构 CShadowUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CShadowUI(void);

	public:
		// bShow涓虹湡鏃舵墠浼氬垱寤洪槾褰?
		/**
		 * @brief 显示阴影
		 * @details 用于显示阴影。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShow [in] 是否显示
		 */
		void ShowShadow(bool bShow);	
		/**
		 * @brief 判断是否显示阴影
		 * @details 用于判断是否显示阴影。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsShowShadow() const;

		/**
		 * @brief 禁用阴影
		 * @details 用于禁用阴影。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bDisable [in] 是否Disable
		 */
		void DisableShadow(bool bDisable);
		/**
		 * @brief 判断是否Disable阴影
		 * @details 用于判断是否Disable阴影。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsDisableShadow() const;

		// 绠楁硶闃村奖鐨勫嚱鏁?
		/**
		 * @brief 设置尺寸
		 * @details 用于设置尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param NewSize [in] 新的尺寸参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetSize(int NewSize = 0);
		/**
		 * @brief 设置Sharpness
		 * @details 用于设置Sharpness。具体行为由当前对象状态以及传入参数共同决定。
		 * @param NewSharpness [in] 新的Sharpness参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetSharpness(unsigned int NewSharpness = 5);
		/**
		 * @brief 设置Darkness
		 * @details 用于设置Darkness。具体行为由当前对象状态以及传入参数共同决定。
		 * @param NewDarkness [in] 新的Darkness参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetDarkness(unsigned int NewDarkness = 200);
		/**
		 * @brief 设置Position
		 * @details 用于设置Position。具体行为由当前对象状态以及传入参数共同决定。
		 * @param NewXOffset [in] 新的XOffset参数
		 * @param NewYOffset [in] 新的YOffset参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetPosition(int NewXOffset = 5, int NewYOffset = 5);
		/**
		 * @brief 设置颜色
		 * @details 用于设置颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param NewColor [in] 新的颜色参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetColor(COLORREF NewColor = 0);

		// 鍥剧墖闃村奖鐨勫嚱鏁?
		/**
		 * @brief 设置图像
		 * @details 用于设置图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szImage [in] 图像尺寸参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetImage(std::wstring_view szImage);
		/**
		 * @brief 设置阴影Corner
		 * @details 用于设置阴影Corner。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcCorner [in] Corner矩形区域
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetShadowCorner(RECT rcCorner);	// 涔濆鏍兼柟寮忔弿杩伴槾褰?

		// 鎶婅嚜宸辩殑闃村奖鏍峰紡澶嶅埗鍒颁紶鍏ュ弬鏁?
		bool CopyShadow(CShadowUI* pShadow);

		//	鍒涘缓闃村奖绐椾綋锛岀敱CPaintManagerUI鑷姩璋冪敤,闄ら潪鑷繁瑕佸崟鐙垱寤洪槾褰?
		/**
		 * @brief 执行 Create 操作
		 * @details 用于执行 Create 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pPaintManager [in] 绘制管理器对象
		 */
		void Create(CPaintManagerUI* pPaintManager);
	protected:

		//	鍒濆鍖栧苟娉ㄥ唽闃村奖绫?
		/**
		 * @brief 执行 RefreshScaledMetrics 操作
		 * @details 用于执行 RefreshScaledMetrics 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RefreshScaledMetrics();
		/**
		 * @brief 确保Scaled度量信息UpToDate
		 * @details 用于确保Scaled度量信息UpToDate。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EnsureScaledMetricsUpToDate();
		/**
		 * @brief 执行 ScaleMetric 操作
		 * @details 用于执行 ScaleMetric 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 * @return int 返回对应的数值结果
		 */
		int ScaleMetric(int value) const;
		/**
		 * @brief 执行 ScaleMetric 操作
		 * @details 用于执行 ScaleMetric 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcValue [in] 值矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT ScaleMetric(RECT rcValue) const;
		/**
		 * @brief 执行 Initialize 操作
		 * @details 用于执行 Initialize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hInstance [in] hInstance参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		static bool Initialize(HINSTANCE hInstance);

		// 淇濆瓨宸茬粡闄勫姞鐨勭獥浣撳彞鏌勫拰涓庡叾鍏宠仈鐨勯槾褰辩被,鏂逛究鍦≒arentProc()鍑芥暟涓€氳繃鍙ユ焺寰楀埌闃村奖绫?
		/**
		 * @brief 获取阴影映射
		 * @details 用于获取阴影映射。具体行为由当前对象状态以及传入参数共同决定。
		 * @return std::map<HWND, CShadowUI *>& 返回结果对象指针，失败时返回 nullptr
		 */
		static std::map<HWND, CShadowUI *>& GetShadowMap();

		//	瀛愮被鍖栫埗绐椾綋
		/**
		 * @brief 执行 ParentProc 操作
		 * @details 用于执行 ParentProc 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hwnd [in] hwnd参数
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @return LRESULT CALLBACK 返回 执行 ParentProc 操作 的结果
		 */
		static LRESULT CALLBACK ParentProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		// 鐖剁獥浣撴敼鍙樺ぇ灏忥紝绉诲姩锛屾垨鑰呬富鍔ㄩ噸缁橀槾褰辨椂璋冪敤
		/**
		 * @brief 执行 Update 操作
		 * @details 用于执行 Update 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hParent [in] h父级控件参数
		 */
		void Update(HWND hParent);

		// 閫氳繃绠楁硶璁＄畻闃村奖
		/**
		 * @brief 执行 MakeShadow 操作
		 * @details 用于执行 MakeShadow 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pShadBits [in] ShadBits对象
		 * @param hParent [in] h父级控件参数
		 * @param rcParent [in] 父级控件矩形区域
		 */
		void MakeShadow(UINT32 *pShadBits, HWND hParent, RECT *rcParent);

		// 璁＄畻alpha棰勪箻鍊?
		/**
		 * @brief 执行 PreMultiply 操作
		 * @details 用于执行 PreMultiply 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cl [in] cl参数
		 * @param nAlpha [in] Alpha数值
		 * @return DWORD 返回对应的数值结果
		 */
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


