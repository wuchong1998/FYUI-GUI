#pragma once
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <gdiplus.h>
#include "UIRenderTypes.h"
#include "UIRenderSurface.h"
#pragma comment( lib, "GdiPlus.lib" )

namespace FYUI
{
	class CControlUI;
	class CRichEditUI;
	class CIDropTarget;
	class CPaintManagerUI;

	/////////////////////////////////////////////////////////////////////////////////////
	//
	enum UILIB_RESTYPE
	{
		UILIB_FILE=1,		// 鏉ヨ嚜纾佺洏鏂囦欢
		UILIB_ZIP,			// 鏉ヨ嚜纾佺洏zip鍘嬬缉鍖?
		UILIB_RESOURCE,		// 鏉ヨ嚜璧勬簮
		UILIB_ZIPRESOURCE,	// 鏉ヨ嚜璧勬簮鐨剒ip鍘嬬缉鍖?
	};
	/////////////////////////////////////////////////////////////////////////////////////
	//
	enum EVENTTYPE_UI
	{
		UIEVENT__FIRST = 1,
		UIEVENT__KEYBEGIN,
		UIEVENT_KEYDOWN,
		UIEVENT_KEYUP,
		UIEVENT_CHAR,
		UIEVENT_SYSKEY,
		UIEVENT__KEYEND,
		UIEVENT__MOUSEBEGIN,
		UIEVENT_MOUSEMOVE,
		UIEVENT_MOUSELEAVE,
		UIEVENT_MOUSEENTER,
		UIEVENT_MOUSEHOVER,
		UIEVENT_BUTTONDOWN,
		UIEVENT_BUTTONUP,
		UIEVENT_RBUTTONDOWN,
		UIEVENT_RBUTTONUP,
		UIEVENT_MBUTTONDOWN,
		UIEVENT_MBUTTONUP,
		UIEVENT_DBLCLICK,
		UIEVENT_CONTEXTMENU,
		UIEVENT_SCROLLWHEEL,
		UIEVENT__MOUSEEND,
		UIEVENT_KILLFOCUS,
		UIEVENT_SETFOCUS,
		UIEVENT_WINDOWSIZE,
		UIEVENT_SETCURSOR,
		UIEVENT_TIMER,
		UIEVENT__LAST,
	};

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// 鍐呴儴淇濈暀鐨勬秷鎭?
	enum MSGTYPE_UI
	{
		UIMSG_TRAYICON = WM_USER + 1,// 鎵樼洏娑堟伅
		UIMSG_SET_DPI,				 // DPI
		WM_MENUCLICK,				 // 鑿滃崟娑堟伅
		UIMSG_SCROLL_ANIMATE = WM_APP + 2,
		UIMSG_USER = WM_USER + 100,	 // 绋嬪簭鑷畾涔夋秷鎭?
	};


		// Flags for CControlUI::GetControlFlags()
	#define UIFLAG_TABSTOP       0x00000001
	#define UIFLAG_SETCURSOR     0x00000002
	#define UIFLAG_WANTRETURN    0x00000004

		// Flags for FindControl()
	#define UIFIND_ALL           0x00000000
	#define UIFIND_VISIBLE       0x00000001
	#define UIFIND_ENABLED       0x00000002
	#define UIFIND_HITTEST       0x00000004
	#define UIFIND_UPDATETEST    0x00000008
	#define UIFIND_TOP_FIRST     0x00000010
	#define UIFIND_ME_FIRST      0x80000000

		// Flags used for controlling the paint
	#define UISTATE_FOCUSED      0x00000001
	#define UISTATE_SELECTED     0x00000002
	#define UISTATE_DISABLED     0x00000004
	#define UISTATE_HOT          0x00000008
	#define UISTATE_PUSHED       0x00000010
	#define UISTATE_READONLY     0x00000020
	#define UISTATE_CAPTURED     0x00000040


	typedef struct FYUI_API tagTFontInfo
	{
		HFONT hFont = nullptr;
		std::wstring sFontName;
		int iSize = 0;
		bool bBold = false;
		bool bUnderline = false;
		bool bItalic = false;
		bool bStrikeout = false;
		TEXTMETRIC tm = {};
	} TFontInfo;

	typedef struct FYUI_API tagTImageInfo
	{
		tagTImageInfo();
		Gdiplus::Image* pImage =nullptr;
		HBITMAP hBitmap = nullptr;
		LPBYTE pBits = nullptr;
		LPBYTE pSrcBits = nullptr;
		int nX;
		int nY;
		int nDestWidth;
		int nDestHeight;
		int nOriWidth;
		int nOriHeight;
		float fPresent;
		bool bAlpha;
		bool bUseHSL;

		std::wstring sResType;
		DWORD dwMask;
		void* pHandle =nullptr;

	} TImageInfo;

	typedef struct FYUI_API tagTDrawInfo
	{
		tagTDrawInfo();
		~tagTDrawInfo();
		void Parse(std::wstring_view pStrImage, std::wstring_view pStrModify, CPaintManagerUI* pManager);
		void Parse(const std::wstring& pStrImage, const std::wstring& pStrModify, CPaintManagerUI* pManager)
		{
			Parse(std::wstring_view(pStrImage), std::wstring_view(pStrModify), pManager);
		}
		void Clear();
		void ClearCachedBitmap();

		TImageInfo* pImageInfo = nullptr;
		std::wstring sDrawString;
		std::wstring sDrawModify;
		std::wstring sImageName;
		std::wstring sResType;
		RECT rcDest;
		RECT rcSource;
		RECT rcCorner;
		RECT rcPicDest;
		DWORD dwMask;
		UINT uFade;
		UINT uRotate;
		bool bHole;
		bool bTiledX;
		bool bTiledY;
		bool bHSL;
		bool bGdiplus;
		bool bLoaded;
		bool bSvg;
		float fPresent = 1.0f;
		HBITMAP hCachedScaledBitmap = nullptr;
		HBITMAP hCachedScaledBitmapSource = nullptr;
		RECT rcCachedScaledSource = { 0 };
		SIZE szCachedScaledBitmap = { 0, 0 };

		CDuiSize szImage;
		RECT rcPadding;
		std::wstring sAlign;
	} TDrawInfo;

	typedef struct FYUI_API tagTPercentInfo
	{
		double left;
		double top;
		double right;
		double bottom;
	} TPercentInfo;

	typedef struct FYUI_API tagTResInfo
	{
		DWORD m_dwDefaultDisabledColor;
		DWORD m_dwDefaultFontColor;
		DWORD m_dwDefaultLinkFontColor;
		DWORD m_dwDefaultLinkHoverFontColor;
		DWORD m_dwDefaultSelectedBkColor;
		TFontInfo m_DefaultFontInfo;
		std::map<std::wstring, TFontInfo*, std::less<>> m_CustomFonts;
		std::unordered_map<std::wstring, TImageInfo*> m_ImageHash;
		std::map<std::wstring, std::wstring, std::less<>> m_AttrHash;
		std::map<std::wstring, std::wstring, std::less<>> m_StyleHash;
		std::unordered_map<std::wstring, TDrawInfo*> m_DrawInfoHash;
	} TResInfo;

	// Structure for notifications from the system
	// to the control implementation.
	typedef struct FYUI_API tagTEventUI
	{
		int Type;
		CControlUI* pSender;
		DWORD dwTimestamp;
		POINT ptMouse;
		wchar_t chKey;
		WORD wKeyState;
		WPARAM wParam;
		LPARAM lParam;
	} TEventUI;

	// Drag&Drop control
	const wchar_t* const CF_MOVECONTROL = L"CF_MOVECONTROL";

	typedef struct FYUI_API tagTCFMoveUI
	{
		CControlUI* pControl;
	} TCFMoveUI;

	// Listener interface
	class INotifyUI
	{
	public:
		virtual void Notify(TNotifyUI& msg) = 0;
	};

	// MessageFilter interface
	class IMessageFilterUI
	{
	public:
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;
	};

	class ITranslateAccelerator
	{
	public:
		virtual LRESULT TranslateAccelerator(MSG *pMsg) = 0;
	};

	class CPaintRenderContext;

	class IDragDropUI
	{
	public:
		virtual bool OnDragDrop(CControlUI* pControl) { return false; }
	};

	struct TRenderDiagnostics
	{
		bool bEnabled = false;
		bool bDebugTrace = false;
		double nTargetFrameMs = 1000.0 / 240.0;
		double nLastPaintMs = 0.0;
		double nAveragePaintMs = 0.0;
		double nWorstPaintMs = 0.0;
		double nLastFrameIntervalMs = 0.0;
		double nAverageFrameIntervalMs = 0.0;
		double nCurrentFPS = 0.0;
		double nAverageFPS = 0.0;
		UINT nTotalFrames = 0;
		UINT nOverBudgetFrames = 0;
		UINT nNativeWindowCacheHits = 0;
		UINT nNativeWindowCacheRefreshes = 0;
		UINT nImageScaleCacheHits = 0;
		UINT nImageScaleCacheRefreshes = 0;
		UINT nScrollRenderCacheHits = 0;
		UINT nScrollRenderCacheMisses = 0;
		UINT nDirect2DBatchFlushes = 0;
		UINT nDirect2DStandaloneDraws = 0;
		UINT nTextLayoutCacheHits = 0;
		UINT nTextLayoutCacheRefreshes = 0;
		UINT nTextMetricsCacheHits = 0;
		UINT nTextMetricsCacheRefreshes = 0;
		UINT nTextNormalizationCacheHits = 0;
		UINT nTextNormalizationCacheRefreshes = 0;
		UINT nHtmlParseCacheHits = 0;
		UINT nHtmlParseCacheRefreshes = 0;
		UINT nHtmlLayoutCacheHits = 0;
		UINT nHtmlLayoutCacheRefreshes = 0;
		UINT nHtmlMetricsCacheHits = 0;
		UINT nHtmlMetricsCacheRefreshes = 0;
		UINT nHtmlHitTestCacheHits = 0;
		UINT nHtmlHitTestCacheRefreshes = 0;
		UINT nHtmlDrawingEffectCacheHits = 0;
		UINT nHtmlDrawingEffectCacheRefreshes = 0;
		UINT nHtmlDirectWriteParseFailures = 0;
		UINT nHtmlDirectWriteLayoutFailures = 0;
		UINT nHtmlDirectWriteRenderFailures = 0;
		RenderBackendType activeBackend = RenderBackendAuto;
		Direct2DRenderMode activeDirect2DMode = Direct2DRenderModeAuto;
	};
	/////////////////////////////////////////////////////////////////////////////////////
	//
	typedef CControlUI* (*LPCREATECONTROL)(const wchar_t* pstrType);

	class FYUI_API CPaintManagerUI : public CIDropTarget
	{
	public:
		CPaintManagerUI();
		~CPaintManagerUI();

	public:
		void Init(HWND hWnd, std::wstring_view name = {});
		bool IsUpdateNeeded() const;
		void NeedUpdate();
		void Invalidate();
		void Invalidate(RECT& rcItem);
		bool ScrollRenderCacheRect(const RECT& rcScroll, int dx, int dy);

		const std::wstring& GetName() const;
		std::wstring_view GetNameView() const { return m_sName; }
		CPaintRenderContext CreateMeasureRenderContext(const RECT& rcPaint);
		HWND GetPaintWindow() const;
		HWND GetTooltipWindow() const;
		int GetHoverTime() const;
		void SetHoverTime(int iTime);

		POINT GetMousePos() const;
		SIZE GetClientSize() const;
		SIZE GetInitSize();
		void SetInitSize(int cx, int cy);
		int ScaleValue(int value) const;
		RECT ScaleRect(RECT rc) const;
		void ScaleRect(RECT* rc) const;
		SIZE ScaleSize(SIZE sz) const;
		void ScaleSize(SIZE* sz) const;
		POINT ScalePoint(POINT pt) const;
		void ScalePoint(POINT* pt) const;
		int UnscaleValue(int value) const;
		RECT UnscaleRect(RECT rc) const;
		void UnscaleRect(RECT* rc) const;
		SIZE UnscaleSize(SIZE sz) const;
		void UnscaleSize(SIZE* sz) const;
		POINT UnscalePoint(POINT pt) const;
		void UnscalePoint(POINT* pt) const;
		RECT GetSizeBox();
		void SetSizeBox(RECT& rcSizeBox);
		RECT GetCaptionRect();
		void SetCaptionRect(RECT& rcCaption);
		SIZE GetRoundCorner();
		void SetRoundCorner(int cx, int cy);
		SIZE GetMinInfo();
		void SetMinInfo(int cx, int cy);
		SIZE GetMaxInfo();
		void SetMaxInfo(int cx, int cy);
		bool IsShowUpdateRect();
		void SetShowUpdateRect(bool show);
		bool IsNoActivate();
		void SetNoActivate(bool bNoActivate);

		BYTE GetOpacity() const;
		void SetOpacity(BYTE nOpacity);

		bool IsLayered();
		void SetLayered(bool bLayered);
		RECT& GetLayeredInset();
		void SetLayeredInset(RECT& rcLayeredInset);
		BYTE GetLayeredOpacity();
		void SetLayeredOpacity(BYTE nOpacity);
		std::wstring_view GetLayeredImage() const { return m_diLayered.sDrawString; }
		void SetLayeredImage(std::wstring_view image);

		CShadowUI* GetShadow();
		void SetRenderBackend(RenderBackendType backend);
		RenderBackendType GetRenderBackend() const;
		RenderBackendType GetActiveRenderBackend();
		void SetDirect2DRenderMode(Direct2DRenderMode mode);
		Direct2DRenderMode GetDirect2DRenderMode() const;
		Direct2DRenderMode GetActiveDirect2DRenderMode();
		void SetRenderDiagnosticsEnabled(bool bEnabled, bool bDebugTrace = false);
		bool IsRenderDiagnosticsEnabled() const;
		bool IsRenderDiagnosticsDebugTraceEnabled() const;
		const CPaintRenderContext* GetCurrentRenderContext() const;
		void GetRenderDiagnostics(TRenderDiagnostics& diagnostics);
		void NotifyImageScaleCacheHit();
		void NotifyImageScaleCacheRefresh();

		void SetUseGdiplusText(bool bUse);
		bool IsUseGdiplusText() const;
		void SetGdiplusTextRenderingHint(int trh);
		int GetGdiplusTextRenderingHint() const;

		static HINSTANCE GetInstance();
		static std::wstring GetInstancePath();
		static std::wstring GetCurrentPath();
		static HINSTANCE GetResourceDll();
		static const std::wstring& GetResourcePath();
		static const std::wstring& GetResourceZip();
		static const std::wstring& GetResourceZipPwd();
		static bool IsCachedResourceZip();
		static HANDLE GetResourceZipHandle();
		static void SetInstance(HINSTANCE hInst);
		static void SetCurrentPath(std::wstring_view path);
		static void SetResourceDll(HINSTANCE hInst);
		static void SetResourcePath(std::wstring_view path);
		static void SetResourceZip(LPVOID pVoid, unsigned int len, std::wstring_view password);
		static void SetResourceZip(std::wstring_view zipPath, bool bCachedResourceZip = false, std::wstring_view password = {});
		static void SetResourceType(int nType);
		static int GetResourceType();
		static bool GetHSL(short* H, short* S, short* L);
		static void SetHSL(bool bUseHSL, short H, short S, short L); // H:0~360, S:0~200, L:0~200 
		static void ReloadSkin();
		static CPaintManagerUI* GetPaintManager(std::wstring_view name);
		static CStdPtrArray* GetPaintManagers();
		static bool LoadPlugin(std::wstring_view moduleName);
		static CStdPtrArray* GetPlugins();

		LONG_PTR GetMainWnd();
		bool IsForceUseSharedRes() const;
		void SetForceUseSharedRes(bool bForce);
		// 娉ㄦ剰锛氬彧鏀寔绠€鍗曠被鍨嬫寚閽堬紝鍥犱负鍙噴鏀惧唴瀛橈紝涓嶄細璋冪敤绫诲璞＄殑鏋愭瀯鍑芥暟
		void DeletePtr(void* ptr);

		DWORD GetDefaultDisabledColor() const;
		void SetDefaultDisabledColor(DWORD dwColor, bool bShared = false);
		DWORD GetDefaultFontColor() const;
		void SetDefaultFontColor(DWORD dwColor, bool bShared = false);
		DWORD GetDefaultLinkFontColor() const;
		void SetDefaultLinkFontColor(DWORD dwColor, bool bShared = false);
		DWORD GetDefaultLinkHoverFontColor() const;
		void SetDefaultLinkHoverFontColor(DWORD dwColor, bool bShared = false);
		DWORD GetDefaultSelectedBkColor() const;
		void SetDefaultSelectedBkColor(DWORD dwColor, bool bShared = false);
		TFontInfo* GetDefaultFontInfo();
		void SetDefaultFont(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		DWORD GetCustomFontCount(bool bShared = false) const;
		void AddFontArray(std::wstring_view pstrPath);
		HFONT AddFont(int id, std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		HFONT GetFont(int id);
		HFONT GetFont(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout);
		int GetFontIndex(HFONT hFont, bool bShared = false);
		int GetFontIndex(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		void RemoveFont(HFONT hFont, bool bShared = false);
		void RemoveFont(int id, bool bShared = false);
		void RemoveAllFonts(bool bShared = false);
		TFontInfo* GetFontInfo(int id);
		TFontInfo* GetFontInfo(HFONT hFont);

		const TImageInfo* GetImage(std::wstring_view bitmap);
		const TImageInfo* GetImageEx(std::wstring_view bitmap, std::wstring_view type = {}, DWORD mask = 0, bool bUseHSL = false, bool bGdiplus = false, HINSTANCE instance = NULL);
		const TImageInfo* AddImage(std::wstring_view bitmap, std::wstring_view type = {}, DWORD mask = 0, bool bUseHSL = false, bool bGdiplus = false, bool bShared = false, HINSTANCE instance = NULL);
		const TImageInfo* AddImage(std::wstring_view bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared = false);
		void RemoveImage(std::wstring_view bitmap, bool bShared = false);
		void RemoveAllImages(bool bShared = false);
		static void ReloadSharedImages();
		void ReloadImages();
		const TImageInfo* ModifyImage(std::wstring_view bitmap, HBITMAP hBitmap, LPBYTE pBits, int iWidth, int iHeight, bool bAlpha);

		TDrawInfo* GetDrawInfo(std::wstring_view image, std::wstring_view modify = {});
		void RemoveDrawInfo(std::wstring_view image, std::wstring_view modify = {});
		void RemoveAllDrawInfos();

		void AddDefaultAttributeList(std::wstring_view controlName, std::wstring_view controlAttrList, bool bShared = false);
		std::wstring_view GetDefaultAttributeList(std::wstring_view controlName) const;
		bool RemoveDefaultAttributeList(std::wstring_view controlName, bool bShared = false);
		void RemoveAllDefaultAttributeList(bool bShared = false);

		void AddWindowCustomAttribute(std::wstring_view name, std::wstring_view attr);
		std::wstring_view GetWindowCustomAttribute(std::wstring_view name) const;
		bool RemoveWindowCustomAttribute(std::wstring_view name);
		void RemoveAllWindowCustomAttribute();

		// 鏍峰紡绠＄悊
		void AddStyle(std::wstring_view name, std::wstring_view style, bool bShared = false);
		std::wstring_view GetStyle(std::wstring_view name) const;
		BOOL RemoveStyle(std::wstring_view name, bool bShared = false);
		const std::map<std::wstring, std::wstring, std::less<>>& GetStyles(bool bShared = false) const;
		void RemoveAllStyle(bool bShared = false);

		const TImageInfo* GetImageString(std::wstring_view image, std::wstring_view modify = {});

		// 鍒濆鍖栨嫋鎷?
		bool EnableDragDrop(bool bEnable);
		void SetDragDrop(IDragDropUI* pDragDrop);
		virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect);
		void SetEventControl(CControlUI* pControl);


		bool IsValid();
		bool AttachDialog(CControlUI* pControl);
		bool InitControls(CControlUI* pControl, CControlUI* pParent = NULL);
		void ReapObjects(CControlUI* pControl);

		bool AddOptionGroup(std::wstring_view groupName, CControlUI* pControl);
		CStdPtrArray* GetOptionGroup(std::wstring_view groupName);
		void RemoveOptionGroup(std::wstring_view groupName, CControlUI* pControl);
		void RemoveAllOptionGroups();

		CControlUI* GetFocus() const;
		void SetFocus(CControlUI* pControl);
		void SetFocusNeeded(CControlUI* pControl);
		void SetNoFocusNeeded ();

		bool SetNextTabControl(bool bForward = true);

		bool SetTimer(CControlUI* pControl, UINT nTimerID, UINT uElapse);
		bool KillTimer(CControlUI* pControl, UINT nTimerID);
		void KillTimer(CControlUI* pControl);
		void RemoveAllTimers();

		void SetCapture();
		void ReleaseCapture();
		bool IsCaptured();

		bool IsPainting();
		void SetPainting(bool bIsPainting);

		bool AddNotifier(INotifyUI* pControl);
		bool RemoveNotifier(INotifyUI* pControl);   
		void SendNotify(TNotifyUI& Msg, bool bAsync = false);
		void SendNotify(CControlUI* pControl, std::wstring_view message, WPARAM wParam = 0, LPARAM lParam = 0, bool bAsync = false);

		bool AddPreMessageFilter(IMessageFilterUI* pFilter);
		bool RemovePreMessageFilter(IMessageFilterUI* pFilter);

		bool AddMessageFilter(IMessageFilterUI* pFilter);
		bool RemoveMessageFilter(IMessageFilterUI* pFilter);

		int GetPostPaintCount() const;
		bool IsPostPaint(CControlUI* pControl);
		bool AddPostPaint(CControlUI* pControl);
		bool RemovePostPaint(CControlUI* pControl);
		bool SetPostPaintIndex(CControlUI* pControl, int iIndex);

		int GetNativeWindowCount() const;
		RECT GetNativeWindowRect(HWND hChildWnd);
		bool AddNativeWindow(CControlUI* pControl, HWND hChildWnd);
		bool RemoveNativeWindow(HWND hChildWnd);

		void AddDelayedCleanup(CControlUI* pControl);
		void AddMouseLeaveNeeded(CControlUI* pControl);
		bool RemoveMouseLeaveNeeded(CControlUI* pControl);

		bool AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
		bool RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
		bool TranslateAccelerator(LPMSG pMsg);

		CControlUI* GetRoot() const;
		CControlUI* FindControl(POINT pt) const;
		CControlUI* FindControl(std::wstring_view name) const;
		CControlUI* FindSubControlByPoint(CControlUI* pParent, POINT pt) const;
		CControlUI* FindSubControlByName(CControlUI* pParent, std::wstring_view name) const;
		CControlUI* FindSubControlByClass(CControlUI* pParent, std::wstring_view className, int iIndex = 0);
		CStdPtrArray* FindSubControlsByClass(CControlUI* pParent, std::wstring_view className);

		static void MessageLoop();
		static bool TranslateMessage(const LPMSG pMsg);
		static void Term();

		[[deprecated("Use ScaleValue/ScaleRect/ScaleSize/ScalePoint and Unscale* helpers instead.")]]
		CDPI* GetDPIObj();
		[[deprecated("Use ScaleValue/ScaleRect/ScaleSize/ScalePoint and Unscale* helpers instead.")]]
		const CDPI* GetDPIObj() const;
		UINT GetDPI() const;
		UINT GetScale() const;
		ULONGLONG GetDPIGeneration() const;
		void SeedDPI(int iDPI);
		void SyncDPIFrom(const CPaintManagerUI* pSourceManager);
		void SyncDPIFromWindow(HWND hWnd);
		void ResetDPIAssets();
		void RebuildFont(TFontInfo* pFontInfo);
		void SetDPI(int iDPI);
		void SetDPI(int iDPI, const RECT* pSuggestedWindowRect);
		static void SetAllDPI(int iDPI);

		bool MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
		bool PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
		void UsedVirtualWnd(bool bUsed);

	private:
		CStdPtrArray* GetFoundControls();
		static CControlUI* CALLBACK __FindControlFromNameHash(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromCount(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromPoint(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromTab(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromShortcut(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromName(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlFromClass(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlsFromClass(CControlUI* pThis, LPVOID pData);
		static CControlUI* CALLBACK __FindControlsFromUpdate(CControlUI* pThis, LPVOID pData);
		CDPI& EnsureDPIState();
		const CDPI& EnsureDPIState() const;

		static void AdjustSharedImagesHSL();
		void AdjustImagesHSL();
		void PostAsyncNotify();
		void ApplyRenderPreferences();
		bool EnsureMeasureSurface();
		void ResetMeasureDC();
		void UpdateFontTextMetricsForManager(TFontInfo* pFontInfo);
		bool EnsureMainRenderSurface(const CPaintRenderContext& referenceContext, LONG cx, LONG cy);
		bool EnsureLayeredBackgroundSurface(const CPaintRenderContext& referenceContext, LONG cx, LONG cy);
		RECT GetRootLayoutRect(const RECT& rcClient) const;
		void ResetRenderSurfaces();
		void UpdateControlsNeedingLayout();
		bool HandleLayoutUpdatesForPaint(const RECT& rcClient, bool& bNeedSizeMsg);
		void AccumulateLayeredUpdateRect(const RECT& rcDirty);
		void ConsumeLayeredUpdateRectForPaint(RECT& rcPaint, const RECT& rcClient);
		void PrepareLayeredPaintRect(RECT& rcPaint, const RECT& rcClient);
		struct TPaintFrameState {
			RECT rcClient = { 0 };
			RECT rcPaint = { 0 };
			bool bNeedSizeMsg = false;
			DWORD dwWidth = 0;
			DWORD dwHeight = 0;
			LONGLONG nPaintStartQpc = 0;
		};
		bool ExecutePaintFrame(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame);
		void PrepareRenderClientRect(RECT& rcClient);
		LONGLONG BeginRenderDiagnosticsFrame() const;
		bool PaintEmptyRootFrame();
		bool PreparePaintFrameRects(TPaintFrameState& paintFrame);
		void HandlePendingFocusForPaint();
		bool PreparePaintFrameLayout(TPaintFrameState& paintFrame);
		bool PreparePaintFrameExecution(TPaintFrameState& paintFrame);
		bool ExecutePaintWindowFrame(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame) const;
		bool PaintPreparedWindowFrame(const TPaintFrameState& paintFrame);
		bool ShouldSkipPaintExecution(const TPaintFrameState& paintFrame) const;
		bool RenderPrintClientFrame(CPaintRenderContext& targetContext, LPARAM lParam);
		void ConsumeRenderFrameMetrics();
		void FinalizeRenderDiagnostics(const TPaintFrameState& paintFrame);
		void FinalizePaintState(const TPaintFrameState& paintFrame);
		void FinalizePaintFrame(const TPaintFrameState& paintFrame);
		bool HandlePaintMessage();
		bool HandlePrintClientMessage(CPaintRenderContext& targetContext, LPARAM lParam);
		bool ExecuteBatchedControlRenderPass(CPaintRenderContext& renderContext, bool bIncludeNativeWindows, bool bIncludePostPaint);
		bool ExecuteBatchedDrawInfoPass(CPaintRenderContext& renderContext, TDrawInfo* pDrawInfo);
		void RenderControlTree(CPaintRenderContext& renderContext, CControlUI* pStopControl = NULL, bool bIncludePostPaint = true);
		void RenderPostPaintControls(CPaintRenderContext& renderContext);
		bool RenderNativeWindowsToSurface(CPaintRenderContext& renderContext);
		bool RenderLayeredBackgroundToSurface(const CPaintRenderContext& referenceContext, const TPaintFrameState& paintFrame);
		bool RenderFrameDirectToWindow(CPaintRenderContext& targetContext, bool bIncludePostPaint = true);
		bool RenderFrameToMainSurface(const CPaintRenderContext& referenceContext, const TPaintFrameState& paintFrame);
		void PrintChildWindows(CPaintRenderContext& targetContext, LPARAM lParam);
		bool PresentLayeredSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame);
		void PresentWindowSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame);
		bool PresentMainRenderSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame);
		void ResetRenderDiagnostics();
		void UpdateRenderDiagnostics(LONGLONG nPaintStartQpc, LONGLONG nPaintEndQpc);
		bool RemoveNativeWindowEntryAt(int iIndex);
		void ClearNativeWindowCache();
		void ClearNativeWindowCache(int iIndex);

	private:
		std::wstring m_sName;
		HWND m_hWndPaint =nullptr;	//鎵€闄勫姞鐨勭獥浣撶殑鍙ユ焺
		CPaintRenderSurface m_measureSurface;
		const CPaintRenderContext* m_pCurrentRenderContext = nullptr;
		CPaintRenderSurface m_offscreenSurface;
		CPaintRenderSurface m_backgroundSurface;

		// 鎻愮ず淇℃伅
		HWND m_hwndTooltip = nullptr;
		TOOLINFO m_ToolTip;
		std::wstring m_sToolTipText;
		int m_iHoverTime;
		bool m_bNoActivate;
		bool m_bShowUpdateRect;

		//
		CControlUI* m_pRoot;
		CControlUI* m_pFocus;
		CControlUI* m_pEventHover;
		CControlUI* m_pEventClick;
		CControlUI* m_pEventRClick;
		CControlUI* m_pEventKey;
		CControlUI* m_pLastToolTip;
		//
		POINT m_ptLastMousePos;
		SIZE m_szMinWindow;
		SIZE m_szMaxWindow;
		SIZE m_szInitWindowSize;
		RECT m_rcSizeBox;
		SIZE m_szRoundCorner;
		RECT m_rcCaption;
		UINT m_uTimerID;
		bool m_bFirstLayout;
		bool m_bUpdateNeeded;
		bool m_bFocusNeeded;
		bool m_bOffscreenPaint;

		BYTE m_nOpacity;
		bool m_bLayered;
		RECT m_rcLayeredInset;
		bool m_bLayeredChanged;
		RECT m_rcLayeredUpdate;
		TDrawInfo m_diLayered ;

		bool m_bMouseTracking;
		bool m_bMouseCapture;
		bool m_bIsPainting;
		bool m_bUsedVirtualWnd;
		bool m_bAsyncNotifyPosted;


		//
		CStdPtrArray m_aNotifiers;
		CStdPtrArray m_aTimers;
		CStdPtrArray m_aTranslateAccelerator;
		CStdPtrArray m_aPreMessageFilters;
		CStdPtrArray m_aMessageFilters;
		std::map<CControlUI*,int>m_mapPostPaintControls;
		CStdPtrArray m_aNativeWindow;
		CStdPtrArray m_aNativeWindowControl;
		CStdPtrArray m_aNativeWindowCache;
		CStdPtrArray m_aDelayedCleanup;
		CStdPtrArray m_aAsyncNotify;
		CStdPtrArray m_aFoundControls;
		CStdPtrArray m_aFonts;
		CStdPtrArray m_aNeedMouseLeaveNeeded;
		std::map<std::wstring,CControlUI*> m_mapName;
		std::map<std::wstring, std::wstring, std::less<>> m_mWindowCustomAttrHash;
		std::map<std::wstring, CStdPtrArray*, std::less<>> m_mOptionGroup;

		bool m_bForceUseSharedRes;
		TResInfo m_ResInfo;

		// 绐楀彛闃村奖
		CShadowUI m_shadow;

		// DPI绠＄悊鍣?
		CDPI* m_pDPI;
		ULONGLONG m_uDPIGeneration;
		RenderBackendType m_renderBackend;
		Direct2DRenderMode m_direct2DRenderMode;
		bool m_bRenderDiagnosticsEnabled;
		bool m_bRenderDiagnosticsDebugTrace;
		LONGLONG m_nRenderPerfFrequency;
		LONGLONG m_nLastPaintEndQpc;
		LONGLONG m_nDiagnosticsSampleStartQpc;
		double m_nLastPaintMs;
		double m_nAveragePaintMs;
		double m_nWorstPaintMs;
		double m_nLastFrameIntervalMs;
		double m_nAverageFrameIntervalMs;
		double m_nCurrentFPS;
		double m_nAverageFPS;
		double m_nSamplePaintTotalMs;
		UINT m_nRenderedFrameCount;
		UINT m_nOverBudgetFrameCount;
		UINT m_nSampleFrameCount;
		UINT m_nNativeWindowCacheHits;
		UINT m_nNativeWindowCacheRefreshes;
		UINT m_nSampleNativeWindowCacheHits;
		UINT m_nSampleNativeWindowCacheRefreshes;
		// 鏄惁寮€鍚疓diplus
		UINT m_nImageScaleCacheHits;
		UINT m_nImageScaleCacheRefreshes;
		UINT m_nSampleImageScaleCacheHits;
		UINT m_nSampleImageScaleCacheRefreshes;
		UINT m_nScrollRenderCacheHits;
		UINT m_nScrollRenderCacheMisses;
		UINT m_nDirect2DBatchFlushes;
		UINT m_nDirect2DStandaloneDraws;
		UINT m_nSampleDirect2DBatchFlushes;
		UINT m_nSampleDirect2DStandaloneDraws;
		UINT m_nTextLayoutCacheHits;
		UINT m_nTextLayoutCacheRefreshes;
		UINT m_nSampleTextLayoutCacheHits;
		UINT m_nSampleTextLayoutCacheRefreshes;
		UINT m_nTextMetricsCacheHits;
		UINT m_nTextMetricsCacheRefreshes;
		UINT m_nSampleTextMetricsCacheHits;
		UINT m_nSampleTextMetricsCacheRefreshes;
		UINT m_nTextNormalizationCacheHits;
		UINT m_nTextNormalizationCacheRefreshes;
		UINT m_nSampleTextNormalizationCacheHits;
		UINT m_nSampleTextNormalizationCacheRefreshes;
		UINT m_nHtmlParseCacheHits;
		UINT m_nHtmlParseCacheRefreshes;
		UINT m_nSampleHtmlParseCacheHits;
		UINT m_nSampleHtmlParseCacheRefreshes;
		UINT m_nHtmlLayoutCacheHits;
		UINT m_nHtmlLayoutCacheRefreshes;
		UINT m_nSampleHtmlLayoutCacheHits;
		UINT m_nSampleHtmlLayoutCacheRefreshes;
		UINT m_nHtmlMetricsCacheHits;
		UINT m_nHtmlMetricsCacheRefreshes;
		UINT m_nSampleHtmlMetricsCacheHits;
		UINT m_nSampleHtmlMetricsCacheRefreshes;
		UINT m_nHtmlHitTestCacheHits;
		UINT m_nHtmlHitTestCacheRefreshes;
		UINT m_nSampleHtmlHitTestCacheHits;
		UINT m_nSampleHtmlHitTestCacheRefreshes;
		UINT m_nHtmlDrawingEffectCacheHits;
		UINT m_nHtmlDrawingEffectCacheRefreshes;
		UINT m_nSampleHtmlDrawingEffectCacheHits;
		UINT m_nSampleHtmlDrawingEffectCacheRefreshes;
		UINT m_nHtmlDirectWriteParseFailures;
		UINT m_nHtmlDirectWriteLayoutFailures;
		UINT m_nHtmlDirectWriteRenderFailures;
		UINT m_nSampleHtmlDirectWriteParseFailures;
		UINT m_nSampleHtmlDirectWriteLayoutFailures;
		UINT m_nSampleHtmlDirectWriteRenderFailures;
		bool m_bUseGdiplusText;
		int m_trh;
		ULONG_PTR m_gdiplusToken;
		Gdiplus::GdiplusStartupInput *m_pGdiplusStartupInput;

		// 鎷栨嫿
		bool m_bDragDrop;
		bool m_bDragMode;
		HBITMAP m_hDragBitmap = nullptr;
		IDragDropUI *m_pDragDrop = nullptr;
		//
		static HINSTANCE m_hInstance;
		static HINSTANCE m_hResourceInstance;
		static std::wstring m_pStrResourcePath;
		static std::wstring m_pStrResourceZip;
		static std::wstring m_pStrResourceZipPwd;
		static HANDLE m_hResourceZip;
		static BYTE* m_cbZipBuf;

		static bool m_bCachedResourceZip;
		static int m_nResType;
		static TResInfo m_SharedResInfo;
		static bool m_bUseHSL;
		static short m_H;
		static short m_S;
		static short m_L;
		static CStdPtrArray m_aPreMessages;
		static CStdPtrArray m_aPlugins;
	};


}

