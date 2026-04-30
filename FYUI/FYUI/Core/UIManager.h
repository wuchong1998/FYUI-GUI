#pragma once
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <objidl.h>
#include <olectl.h>
#include <gdiplus.h>
#include "Render/UIRenderTypes.h"
#include "Render/UIRenderSurface.h"
#pragma comment( lib, "GdiPlus.lib" )

namespace FYUI
{
	class CControlUI;
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
		UIMSG_ANIMATION_FRAME = WM_APP + 3,
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
		/**
		 * @brief 执行 tagTImageInfo 操作
		 * @details 用于执行 tagTImageInfo 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
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
		/**
		 * @brief 执行 tagTDrawInfo 操作
		 * @details 用于执行 tagTDrawInfo 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		tagTDrawInfo();
		/**
		 * @brief 执行 ~tagTDrawInfo 操作
		 * @details 用于执行 ~tagTDrawInfo 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~tagTDrawInfo();
		/**
		 * @brief 执行 Parse 操作
		 * @details 用于执行 Parse 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 * @param pStrModify [in] StrModify对象
		 * @param pManager [in] 管理器对象
		 */
		void Parse(std::wstring_view pStrImage, std::wstring_view pStrModify, CPaintManagerUI* pManager);
		/**
		 * @brief 执行 Parse 操作
		 * @details 用于执行 Parse 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 * @param pStrModify [in] StrModify对象
		 * @param pManager [in] 管理器对象
		 */
		void Parse(const std::wstring& pStrImage, const std::wstring& pStrModify, CPaintManagerUI* pManager)
		{
			Parse(std::wstring_view(pStrImage), std::wstring_view(pStrModify), pManager);
		}
		/**
		 * @brief 执行 Clear 操作
		 * @details 用于执行 Clear 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Clear();
		/**
		 * @brief 清空Cached位图
		 * @details 用于清空Cached位图。具体行为由当前对象状态以及传入参数共同决定。
		 */
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
		/**
		 * @brief 执行 Notify 操作
		 * @details 用于执行 Notify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in,out] msg参数
		 */
		virtual void Notify(TNotifyUI& msg) = 0;
	};

	// MessageFilter interface
	class IMessageFilterUI
	{
	public:
		/**
		 * @brief 处理窗口消息
		 * @details 用于处理窗口消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) = 0;
	};

	class ITranslateAccelerator
	{
	public:
		/**
		 * @brief 执行 TranslateAccelerator 操作
		 * @details 用于执行 TranslateAccelerator 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pMsg [in] Msg对象
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT TranslateAccelerator(MSG *pMsg) = 0;
	};

	class CPaintRenderContext;

	class IDragDropUI
	{
	public:
		/**
		 * @brief 执行 OnDragDrop 操作
		 * @details 用于执行 OnDragDrop 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
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
		/**
		 * @brief 构造 CPaintManagerUI 对象
		 * @details 用于构造 CPaintManagerUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CPaintManagerUI();
		/**
		 * @brief 析构 CPaintManagerUI 对象
		 * @details 用于析构 CPaintManagerUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CPaintManagerUI();

	public:
		/**
		 * @brief 初始化对象
		 * @details 用于初始化对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 * @param name [in] 名称字符串
		 */
		void Init(HWND hWnd, std::wstring_view name = {});
		/**
		 * @brief 判断是否更新Needed
		 * @details 用于判断是否更新Needed。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsUpdateNeeded() const;
		/**
		 * @brief 标记需要更新
		 * @details 用于标记需要更新。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void NeedUpdate();
		/**
		 * @brief 触发重绘
		 * @details 用于触发重绘。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Invalidate();
		/**
		 * @brief 触发重绘
		 * @details 用于触发重绘。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcItem [in,out] 子项矩形区域
		 */
		void Invalidate(RECT& rcItem);
		/**
		 * @brief 请求下一帧动画刷新
		 * @details 用于请求下一帧动画刷新。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void RequestAnimationFrame(CControlUI* pControl);
		/**
		 * @brief 滚动渲染缓存矩形
		 * @details 用于滚动渲染缓存矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcScroll [in] 滚动矩形区域
		 * @param dx [in] dx参数
		 * @param dy [in] dy参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ScrollRenderCacheRect(const RECT& rcScroll, int dx, int dy);

		/**
		 * @brief 获取名称
		 * @details 用于获取名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		const std::wstring& GetName() const;
		/**
		 * @brief 获取名称视图
		 * @details 用于获取名称视图。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetNameView() const { return m_sName; }
		CPaintRenderContext CreateMeasureRenderContext(const RECT& rcPaint);
		/**
		 * @brief 获取绘制窗口
		 * @details 用于获取绘制窗口。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HWND 返回 获取绘制窗口 的结果
		 */
		HWND GetPaintWindow() const;
		/**
		 * @brief 获取Tooltip窗口
		 * @details 用于获取Tooltip窗口。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HWND 返回 获取Tooltip窗口 的结果
		 */
		HWND GetTooltipWindow() const;
		/**
		 * @brief 获取HoverTime
		 * @details 用于获取HoverTime。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetHoverTime() const;
		/**
		 * @brief 设置HoverTime
		 * @details 用于设置HoverTime。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iTime [in] Time值
		 */
		void SetHoverTime(int iTime);

		/**
		 * @brief 获取鼠标位置
		 * @details 用于获取鼠标位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		POINT GetMousePos() const;
		/**
		 * @brief 获取客户区尺寸
		 * @details 用于获取客户区尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetClientSize() const;
		/**
		 * @brief 获取Init尺寸
		 * @details 用于获取Init尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetInitSize();
		/**
		 * @brief 设置Init尺寸
		 * @details 用于设置Init尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void SetInitSize(int cx, int cy);
		/**
		 * @brief 执行 ScaleValue 操作
		 * @details 用于执行 ScaleValue 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 * @return int 返回对应的数值结果
		 */
		int ScaleValue(int value) const;
		/**
		 * @brief 执行 ScaleRect 操作
		 * @details 用于执行 ScaleRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT ScaleRect(RECT rc) const;
		/**
		 * @brief 执行 ScaleRect 操作
		 * @details 用于执行 ScaleRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void ScaleRect(RECT* rc) const;
		/**
		 * @brief 执行 ScaleSize 操作
		 * @details 用于执行 ScaleSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param sz [in] 尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE ScaleSize(SIZE sz) const;
		/**
		 * @brief 执行 ScaleSize 操作
		 * @details 用于执行 ScaleSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param sz [in] 尺寸参数
		 */
		void ScaleSize(SIZE* sz) const;
		/**
		 * @brief 执行 ScalePoint 操作
		 * @details 用于执行 ScalePoint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return 返回对应的几何结果
		 */
		POINT ScalePoint(POINT pt) const;
		/**
		 * @brief 执行 ScalePoint 操作
		 * @details 用于执行 ScalePoint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 */
		void ScalePoint(POINT* pt) const;
		/**
		 * @brief 执行 UnscaleValue 操作
		 * @details 用于执行 UnscaleValue 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 * @return int 返回对应的数值结果
		 */
		int UnscaleValue(int value) const;
		/**
		 * @brief 执行 UnscaleRect 操作
		 * @details 用于执行 UnscaleRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT UnscaleRect(RECT rc) const;
		/**
		 * @brief 执行 UnscaleRect 操作
		 * @details 用于执行 UnscaleRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void UnscaleRect(RECT* rc) const;
		/**
		 * @brief 执行 UnscaleSize 操作
		 * @details 用于执行 UnscaleSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param sz [in] 尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE UnscaleSize(SIZE sz) const;
		/**
		 * @brief 执行 UnscaleSize 操作
		 * @details 用于执行 UnscaleSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param sz [in] 尺寸参数
		 */
		void UnscaleSize(SIZE* sz) const;
		/**
		 * @brief 执行 UnscalePoint 操作
		 * @details 用于执行 UnscalePoint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return 返回对应的几何结果
		 */
		POINT UnscalePoint(POINT pt) const;
		/**
		 * @brief 执行 UnscalePoint 操作
		 * @details 用于执行 UnscalePoint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 */
		void UnscalePoint(POINT* pt) const;
		/**
		 * @brief 获取尺寸Box
		 * @details 用于获取尺寸Box。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetSizeBox();
		/**
		 * @brief 设置尺寸Box
		 * @details 用于设置尺寸Box。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcSizeBox [in,out] 尺寸Box矩形区域
		 */
		void SetSizeBox(RECT& rcSizeBox);
		/**
		 * @brief 获取Caption矩形
		 * @details 用于获取Caption矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetCaptionRect();
		/**
		 * @brief 设置Caption矩形
		 * @details 用于设置Caption矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcCaption [in,out] Caption矩形区域
		 */
		void SetCaptionRect(RECT& rcCaption);
		/**
		 * @brief 获取圆角Corner
		 * @details 用于获取圆角Corner。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetRoundCorner();
		/**
		 * @brief 设置圆角Corner
		 * @details 用于设置圆角Corner。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void SetRoundCorner(int cx, int cy);
		/**
		 * @brief 获取最小信息
		 * @details 用于获取最小信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetMinInfo();
		/**
		 * @brief 设置最小信息
		 * @details 用于设置最小信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void SetMinInfo(int cx, int cy);
		/**
		 * @brief 获取最大信息
		 * @details 用于获取最大信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetMaxInfo();
		/**
		 * @brief 设置最大信息
		 * @details 用于设置最大信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void SetMaxInfo(int cx, int cy);
		/**
		 * @brief 判断是否显示更新矩形
		 * @details 用于判断是否显示更新矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsShowUpdateRect();
		/**
		 * @brief 设置显示更新矩形
		 * @details 用于设置显示更新矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param show [in] 显示参数
		 */
		void SetShowUpdateRect(bool show);
		/**
		 * @brief 判断是否No激活
		 * @details 用于判断是否No激活。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsNoActivate();
		/**
		 * @brief 设置No激活
		 * @details 用于设置No激活。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bNoActivate [in] 是否No激活
		 */
		void SetNoActivate(bool bNoActivate);

		/**
		 * @brief 获取Opacity
		 * @details 用于获取Opacity。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BYTE 返回 获取Opacity 的结果
		 */
		BYTE GetOpacity() const;
		/**
		 * @brief 设置Opacity
		 * @details 用于设置Opacity。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nOpacity [in] Opacity数值
		 */
		void SetOpacity(BYTE nOpacity);

		/**
		 * @brief 判断是否Layered
		 * @details 用于判断是否Layered。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsLayered();
		/**
		 * @brief 设置Layered
		 * @details 用于设置Layered。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bLayered [in] 是否Layered
		 */
		void SetLayered(bool bLayered);
		/**
		 * @brief 获取LayeredInset
		 * @details 用于获取LayeredInset。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT& GetLayeredInset();
		/**
		 * @brief 设置LayeredInset
		 * @details 用于设置LayeredInset。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcLayeredInset [in,out] LayeredInset矩形区域
		 */
		void SetLayeredInset(RECT& rcLayeredInset);
		/**
		 * @brief 获取LayeredOpacity
		 * @details 用于获取LayeredOpacity。具体行为由当前对象状态以及传入参数共同决定。
		 * @return BYTE 返回 获取LayeredOpacity 的结果
		 */
		BYTE GetLayeredOpacity();
		/**
		 * @brief 设置LayeredOpacity
		 * @details 用于设置LayeredOpacity。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nOpacity [in] Opacity数值
		 */
		void SetLayeredOpacity(BYTE nOpacity);
		/**
		 * @brief 获取Layered图像
		 * @details 用于获取Layered图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetLayeredImage() const { return m_diLayered.sDrawString; }
		void SetLayeredImage(std::wstring_view image);

		/**
		 * @brief 获取阴影
		 * @details 用于获取阴影。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CShadowUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CShadowUI* GetShadow();
		/**
		 * @brief 设置渲染渲染后端
		 * @details 用于设置渲染渲染后端。具体行为由当前对象状态以及传入参数共同决定。
		 * @param backend [in] 渲染后端参数
		 */
		void SetRenderBackend(RenderBackendType backend);
		/**
		 * @brief 获取渲染渲染后端
		 * @details 用于获取渲染渲染后端。具体行为由当前对象状态以及传入参数共同决定。
		 * @return RenderBackendType 返回 获取渲染渲染后端 的结果
		 */
		RenderBackendType GetRenderBackend() const;
		/**
		 * @brief 获取Active渲染渲染后端
		 * @details 用于获取Active渲染渲染后端。具体行为由当前对象状态以及传入参数共同决定。
		 * @return RenderBackendType 返回 获取Active渲染渲染后端 的结果
		 */
		RenderBackendType GetActiveRenderBackend();
		/**
		 * @brief 设置直接2D渲染Mode
		 * @details 用于设置直接2D渲染Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @param mode [in] mode参数
		 */
		void SetDirect2DRenderMode(Direct2DRenderMode mode);
		/**
		 * @brief 获取直接2D渲染Mode
		 * @details 用于获取直接2D渲染Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return Direct2DRenderMode 返回 获取直接2D渲染Mode 的结果
		 */
		Direct2DRenderMode GetDirect2DRenderMode() const;
		/**
		 * @brief 获取Active直接2D渲染Mode
		 * @details 用于获取Active直接2D渲染Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return Direct2DRenderMode 返回 获取Active直接2D渲染Mode 的结果
		 */
		Direct2DRenderMode GetActiveDirect2DRenderMode();
		/**
		 * @brief 设置渲染Diagnostics启用状态
		 * @details 用于设置渲染Diagnostics启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnabled [in] 是否启用状态
		 * @param bDebugTrace [in] 是否DebugTrace
		 */
		void SetRenderDiagnosticsEnabled(bool bEnabled, bool bDebugTrace = false);
		/**
		 * @brief 判断是否渲染Diagnostics启用状态
		 * @details 用于判断是否渲染Diagnostics启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsRenderDiagnosticsEnabled() const;
		/**
		 * @brief 判断是否渲染DiagnosticsDebugTrace启用状态
		 * @details 用于判断是否渲染DiagnosticsDebugTrace启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsRenderDiagnosticsDebugTraceEnabled() const;
		/**
		 * @brief 获取当前渲染上下文
		 * @details 用于获取当前渲染上下文。具体行为由当前对象状态以及传入参数共同决定。
		 * @return const CPaintRenderContext* 返回结果对象指针，失败时返回 nullptr
		 */
		const CPaintRenderContext* GetCurrentRenderContext() const;
		/**
		 * @brief 获取渲染Diagnostics
		 * @details 用于获取渲染Diagnostics。具体行为由当前对象状态以及传入参数共同决定。
		 * @param diagnostics [in,out] diagnostics参数
		 */
		void GetRenderDiagnostics(TRenderDiagnostics& diagnostics);
		/**
		 * @brief 执行 NotifyImageScaleCacheHit 操作
		 * @details 用于执行 NotifyImageScaleCacheHit 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void NotifyImageScaleCacheHit();
		/**
		 * @brief 执行 NotifyImageScaleCacheRefresh 操作
		 * @details 用于执行 NotifyImageScaleCacheRefresh 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void NotifyImageScaleCacheRefresh();

		/**
		 * @brief 获取Instance
		 * @details 用于获取Instance。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HINSTANCE 返回 获取Instance 的结果
		 */
		static HINSTANCE GetInstance();
		/**
		 * @brief 获取Instance路径
		 * @details 用于获取Instance路径。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		static std::wstring GetInstancePath();
		/**
		 * @brief 获取当前路径
		 * @details 用于获取当前路径。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		static std::wstring GetCurrentPath();
		/**
		 * @brief 获取资源动态库
		 * @details 用于获取资源动态库。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HINSTANCE 返回 获取资源动态库 的结果
		 */
		static HINSTANCE GetResourceDll();
		/**
		 * @brief 获取资源路径
		 * @details 用于获取资源路径。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		static const std::wstring& GetResourcePath();
		/**
		 * @brief 获取资源压缩包
		 * @details 用于获取资源压缩包。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		static const std::wstring& GetResourceZip();
		/**
		 * @brief 获取资源压缩包Pwd
		 * @details 用于获取资源压缩包Pwd。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		static const std::wstring& GetResourceZipPwd();
		/**
		 * @brief 判断是否Cached资源压缩包
		 * @details 用于判断是否Cached资源压缩包。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		static bool IsCachedResourceZip();
		/**
		 * @brief 获取资源压缩包处理
		 * @details 用于获取资源压缩包处理。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HANDLE 返回 获取资源压缩包处理 的结果
		 */
		static HANDLE GetResourceZipHandle();
		/**
		 * @brief 设置Instance
		 * @details 用于设置Instance。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hInst [in] 模块实例句柄
		 */
		static void SetInstance(HINSTANCE hInst);
		/**
		 * @brief 设置当前路径
		 * @details 用于设置当前路径。具体行为由当前对象状态以及传入参数共同决定。
		 * @param path [in] 路径字符串
		 */
		static void SetCurrentPath(std::wstring_view path);
		/**
		 * @brief 设置资源动态库
		 * @details 用于设置资源动态库。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hInst [in] 模块实例句柄
		 */
		static void SetResourceDll(HINSTANCE hInst);
		/**
		 * @brief 设置资源路径
		 * @details 用于设置资源路径。具体行为由当前对象状态以及传入参数共同决定。
		 * @param path [in] 路径字符串
		 */
		static void SetResourcePath(std::wstring_view path);
		/**
		 * @brief 设置压缩包资源
		 * @details 用于设置压缩包资源。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pVoid [in] 资源内存起始地址
		 * @param len [in] 资源数据长度
		 * @param password [in] 压缩包密码
		 */
		static void SetResourceZip(LPVOID pVoid, unsigned int len, std::wstring_view password);
		/**
		 * @brief 设置压缩包资源
		 * @details 用于设置压缩包资源。具体行为由当前对象状态以及传入参数共同决定。
		 * @param zipPath [in] 压缩包路径
		 * @param bCachedResourceZip [in] 是否Cached资源压缩包
		 * @param password [in] 压缩包密码
		 */
		static void SetResourceZip(std::wstring_view zipPath, bool bCachedResourceZip = false, std::wstring_view password = {});
		/**
		 * @brief 设置资源类型
		 * @details 用于设置资源类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nType [in] 类型数值
		 */
		static void SetResourceType(int nType);
		/**
		 * @brief 获取资源类型
		 * @details 用于获取资源类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		static int GetResourceType();
		/**
		 * @brief 获取HSL 颜色
		 * @details 用于获取HSL 颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param H [in] H参数
		 * @param S [in] S参数
		 * @param L [in] L参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		static bool GetHSL(short* H, short* S, short* L);
		/**
		 * @brief 设置HSL 颜色
		 * @details 用于设置HSL 颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bUseHSL [in] 是否UseHSL 颜色
		 * @param H [in] H参数
		 * @param S [in] S参数
		 * @param L [in] L参数
		 */
		static void SetHSL(bool bUseHSL, short H, short S, short L); // H:0~360, S:0~200, L:0~200 
		static void ReloadSkin();
		/**
		 * @brief 获取绘制管理器
		 * @details 用于获取绘制管理器。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @return CPaintManagerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		static CPaintManagerUI* GetPaintManager(std::wstring_view name);
		/**
		 * @brief 获取绘制Managers
		 * @details 用于获取绘制Managers。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CStdPtrArray* 返回结果对象指针，失败时返回 nullptr
		 */
		static CStdPtrArray* GetPaintManagers();
		/**
		 * @brief 加载插件
		 * @details 用于加载插件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param moduleName [in] module名称参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		static bool LoadPlugin(std::wstring_view moduleName);
		/**
		 * @brief 获取Plugins
		 * @details 用于获取Plugins。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CStdPtrArray* 返回结果对象指针，失败时返回 nullptr
		 */
		static CStdPtrArray* GetPlugins();

		/**
		 * @brief 获取MainWnd
		 * @details 用于获取MainWnd。具体行为由当前对象状态以及传入参数共同决定。
		 * @return LONG_PTR 返回 获取MainWnd 的结果
		 */
		LONG_PTR GetMainWnd();
		/**
		 * @brief 判断是否ForceUseSharedRes
		 * @details 用于判断是否ForceUseSharedRes。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsForceUseSharedRes() const;
		/**
		 * @brief 设置ForceUseSharedRes
		 * @details 用于设置ForceUseSharedRes。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bForce [in] 是否Force
		 */
		void SetForceUseSharedRes(bool bForce);
		// 娉ㄦ剰锛氬彧鏀寔绠€鍗曠被鍨嬫寚閽堬紝鍥犱负鍙噴鏀惧唴瀛橈紝涓嶄細璋冪敤绫诲璞＄殑鏋愭瀯鍑芥暟
		/**
		 * @brief 删除Ptr
		 * @details 用于删除Ptr。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ptr [in] r坐标点
		 */
		void DeletePtr(void* ptr);

		/**
		 * @brief 获取默认Disabled颜色
		 * @details 用于获取默认Disabled颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDefaultDisabledColor() const;
		/**
		 * @brief 设置默认Disabled颜色
		 * @details 用于设置默认Disabled颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @param bShared [in] 是否Shared
		 */
		void SetDefaultDisabledColor(DWORD dwColor, bool bShared = false);
		/**
		 * @brief 获取默认字体颜色
		 * @details 用于获取默认字体颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDefaultFontColor() const;
		/**
		 * @brief 设置默认字体颜色
		 * @details 用于设置默认字体颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @param bShared [in] 是否Shared
		 */
		void SetDefaultFontColor(DWORD dwColor, bool bShared = false);
		/**
		 * @brief 获取默认Link字体颜色
		 * @details 用于获取默认Link字体颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDefaultLinkFontColor() const;
		/**
		 * @brief 设置默认Link字体颜色
		 * @details 用于设置默认Link字体颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @param bShared [in] 是否Shared
		 */
		void SetDefaultLinkFontColor(DWORD dwColor, bool bShared = false);
		/**
		 * @brief 获取默认LinkHover字体颜色
		 * @details 用于获取默认LinkHover字体颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDefaultLinkHoverFontColor() const;
		/**
		 * @brief 设置默认LinkHover字体颜色
		 * @details 用于设置默认LinkHover字体颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @param bShared [in] 是否Shared
		 */
		void SetDefaultLinkHoverFontColor(DWORD dwColor, bool bShared = false);
		/**
		 * @brief 获取默认Selected背景颜色
		 * @details 用于获取默认Selected背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDefaultSelectedBkColor() const;
		/**
		 * @brief 设置默认Selected背景颜色
		 * @details 用于设置默认Selected背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @param bShared [in] 是否Shared
		 */
		void SetDefaultSelectedBkColor(DWORD dwColor, bool bShared = false);
		/**
		 * @brief 获取默认字体信息
		 * @details 用于获取默认字体信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @return TFontInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		TFontInfo* GetDefaultFontInfo();
		/**
		 * @brief 设置默认字体
		 * @details 用于设置默认字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param fontName [in] 字体名称参数
		 * @param nSize [in] 尺寸数值
		 * @param bBold [in] 是否Bold
		 * @param bUnderline [in] 是否Underline
		 * @param bItalic [in] 是否Italic
		 * @param bStrikeout [in] 是否Strikeout
		 * @param bShared [in] 是否Shared
		 */
		void SetDefaultFont(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		/**
		 * @brief 获取Custom字体数量
		 * @details 用于获取Custom字体数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShared [in] 是否Shared
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetCustomFontCount(bool bShared = false) const;
		/**
		 * @brief 添加字体Array
		 * @details 用于添加字体Array。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrPath [in] 路径字符串
		 */
		void AddFontArray(std::wstring_view pstrPath);
		/**
		 * @brief 添加字体
		 * @details 用于添加字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param id [in] id参数
		 * @param fontName [in] 字体名称参数
		 * @param nSize [in] 尺寸数值
		 * @param bBold [in] 是否Bold
		 * @param bUnderline [in] 是否Underline
		 * @param bItalic [in] 是否Italic
		 * @param bStrikeout [in] 是否Strikeout
		 * @param bShared [in] 是否Shared
		 * @return HFONT 返回 添加字体 的结果
		 */
		HFONT AddFont(int id, std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		/**
		 * @brief 获取字体
		 * @details 用于获取字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param id [in] id参数
		 * @return HFONT 返回 获取字体 的结果
		 */
		HFONT GetFont(int id);
		/**
		 * @brief 获取字体
		 * @details 用于获取字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param fontName [in] 字体名称参数
		 * @param nSize [in] 尺寸数值
		 * @param bBold [in] 是否Bold
		 * @param bUnderline [in] 是否Underline
		 * @param bItalic [in] 是否Italic
		 * @param bStrikeout [in] 是否Strikeout
		 * @return HFONT 返回 获取字体 的结果
		 */
		HFONT GetFont(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout);
		/**
		 * @brief 获取字体索引
		 * @details 用于获取字体索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hFont [in] h字体参数
		 * @param bShared [in] 是否Shared
		 * @return int 返回对应的数值结果
		 */
		int GetFontIndex(HFONT hFont, bool bShared = false);
		/**
		 * @brief 获取字体索引
		 * @details 用于获取字体索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param fontName [in] 字体名称参数
		 * @param nSize [in] 尺寸数值
		 * @param bBold [in] 是否Bold
		 * @param bUnderline [in] 是否Underline
		 * @param bItalic [in] 是否Italic
		 * @param bStrikeout [in] 是否Strikeout
		 * @param bShared [in] 是否Shared
		 * @return int 返回对应的数值结果
		 */
		int GetFontIndex(std::wstring_view fontName, int nSize, bool bBold, bool bUnderline, bool bItalic, bool bStrikeout, bool bShared = false);
		/**
		 * @brief 移除字体
		 * @details 用于移除字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hFont [in] h字体参数
		 * @param bShared [in] 是否Shared
		 */
		void RemoveFont(HFONT hFont, bool bShared = false);
		/**
		 * @brief 移除字体
		 * @details 用于移除字体。具体行为由当前对象状态以及传入参数共同决定。
		 * @param id [in] id参数
		 * @param bShared [in] 是否Shared
		 */
		void RemoveFont(int id, bool bShared = false);
		/**
		 * @brief 移除AllFonts
		 * @details 用于移除AllFonts。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShared [in] 是否Shared
		 */
		void RemoveAllFonts(bool bShared = false);
		/**
		 * @brief 获取字体信息
		 * @details 用于获取字体信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param id [in] id参数
		 * @return TFontInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		TFontInfo* GetFontInfo(int id);
		/**
		 * @brief 获取字体信息
		 * @details 用于获取字体信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hFont [in] h字体参数
		 * @return TFontInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		TFontInfo* GetFontInfo(HFONT hFont);

		/**
		 * @brief 获取图像
		 * @details 用于获取图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bitmap [in] 位图参数
		 * @return const TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		const TImageInfo* GetImage(std::wstring_view bitmap);
		/**
		 * @brief 获取图像Ex
		 * @details 用于获取图像Ex。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bitmap [in] 位图参数
		 * @param type [in] 类型参数
		 * @param mask [in] mask参数
		 * @param bUseHSL [in] 是否UseHSL 颜色
		 * @param bGdiplus [in] 是否Gdiplus
		 * @param instance [in] 模块实例句柄
		 * @return const TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		const TImageInfo* GetImageEx(std::wstring_view bitmap, std::wstring_view type = {}, DWORD mask = 0, bool bUseHSL = false, bool bGdiplus = false, HINSTANCE instance = NULL);
		/**
		 * @brief 添加图像
		 * @details 用于添加图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bitmap [in] 位图参数
		 * @param type [in] 类型参数
		 * @param mask [in] mask参数
		 * @param bUseHSL [in] 是否UseHSL 颜色
		 * @param bGdiplus [in] 是否Gdiplus
		 * @param bShared [in] 是否Shared
		 * @param instance [in] 模块实例句柄
		 * @return const TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		const TImageInfo* AddImage(std::wstring_view bitmap, std::wstring_view type = {}, DWORD mask = 0, bool bUseHSL = false, bool bGdiplus = false, bool bShared = false, HINSTANCE instance = NULL);
		/**
		 * @brief 添加图像
		 * @details 用于添加图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bitmap [in] 位图参数
		 * @param hBitmap [in] h位图参数
		 * @param iWidth [in] 宽度值
		 * @param iHeight [in] 高度值
		 * @param bAlpha [in] 是否Alpha
		 * @param bShared [in] 是否Shared
		 * @return const TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		const TImageInfo* AddImage(std::wstring_view bitmap, HBITMAP hBitmap, int iWidth, int iHeight, bool bAlpha, bool bShared = false);
		/**
		 * @brief 移除图像
		 * @details 用于移除图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bitmap [in] 位图参数
		 * @param bShared [in] 是否Shared
		 */
		void RemoveImage(std::wstring_view bitmap, bool bShared = false);
		/**
		 * @brief 移除AllImages
		 * @details 用于移除AllImages。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShared [in] 是否Shared
		 */
		void RemoveAllImages(bool bShared = false);
		/**
		 * @brief 重新加载SharedImages
		 * @details 用于重新加载SharedImages。具体行为由当前对象状态以及传入参数共同决定。
		 */
		static void ReloadSharedImages();
		/**
		 * @brief 重新加载Images
		 * @details 用于重新加载Images。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ReloadImages();
		/**
		 * @brief 执行 ModifyImage 操作
		 * @details 用于执行 ModifyImage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bitmap [in] 位图参数
		 * @param hBitmap [in] h位图参数
		 * @param pBits [in] Bits对象
		 * @param iWidth [in] 宽度值
		 * @param iHeight [in] 高度值
		 * @param bAlpha [in] 是否Alpha
		 * @return const TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		const TImageInfo* ModifyImage(std::wstring_view bitmap, HBITMAP hBitmap, LPBYTE pBits, int iWidth, int iHeight, bool bAlpha);

		/**
		 * @brief 获取绘制信息
		 * @details 用于获取绘制信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param image [in] 图像描述字符串
		 * @param modify [in] modify参数
		 * @return TDrawInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		TDrawInfo* GetDrawInfo(std::wstring_view image, std::wstring_view modify = {});
		/**
		 * @brief 移除绘制信息
		 * @details 用于移除绘制信息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param image [in] 图像描述字符串
		 * @param modify [in] modify参数
		 */
		void RemoveDrawInfo(std::wstring_view image, std::wstring_view modify = {});
		/**
		 * @brief 移除All绘制Infos
		 * @details 用于移除All绘制Infos。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RemoveAllDrawInfos();

		/**
		 * @brief 添加默认属性列表
		 * @details 用于添加默认属性列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @param controlName [in] 控件名称参数
		 * @param controlAttrList [in] 控件Attr列表参数
		 * @param bShared [in] 是否Shared
		 */
		void AddDefaultAttributeList(std::wstring_view controlName, std::wstring_view controlAttrList, bool bShared = false);
		/**
		 * @brief 获取默认属性列表
		 * @details 用于获取默认属性列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @param controlName [in] 控件名称参数
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetDefaultAttributeList(std::wstring_view controlName) const;
		/**
		 * @brief 移除默认属性列表
		 * @details 用于移除默认属性列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @param controlName [in] 控件名称参数
		 * @param bShared [in] 是否Shared
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveDefaultAttributeList(std::wstring_view controlName, bool bShared = false);
		/**
		 * @brief 移除All默认属性列表
		 * @details 用于移除All默认属性列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShared [in] 是否Shared
		 */
		void RemoveAllDefaultAttributeList(bool bShared = false);

		/**
		 * @brief 添加窗口Custom属性
		 * @details 用于添加窗口Custom属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @param attr [in] attr参数
		 */
		void AddWindowCustomAttribute(std::wstring_view name, std::wstring_view attr);
		/**
		 * @brief 获取窗口Custom属性
		 * @details 用于获取窗口Custom属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetWindowCustomAttribute(std::wstring_view name) const;
		/**
		 * @brief 移除窗口Custom属性
		 * @details 用于移除窗口Custom属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveWindowCustomAttribute(std::wstring_view name);
		/**
		 * @brief 移除All窗口Custom属性
		 * @details 用于移除All窗口Custom属性。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RemoveAllWindowCustomAttribute();

		// 鏍峰紡绠＄悊
		/**
		 * @brief 添加样式
		 * @details 用于添加样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @param style [in] 样式参数
		 * @param bShared [in] 是否Shared
		 */
		void AddStyle(std::wstring_view name, std::wstring_view style, bool bShared = false);
		/**
		 * @brief 获取样式
		 * @details 用于获取样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetStyle(std::wstring_view name) const;
		/**
		 * @brief 移除样式
		 * @details 用于移除样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @param bShared [in] 是否Shared
		 * @return BOOL 返回 移除样式 的结果
		 */
		BOOL RemoveStyle(std::wstring_view name, bool bShared = false);
		/**
		 * @brief 获取Styles
		 * @details 用于获取Styles。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShared [in] 是否Shared
		 * @return 返回对应的字符串内容
		 */
		const std::map<std::wstring, std::wstring, std::less<>>& GetStyles(bool bShared = false) const;
		/**
		 * @brief 移除All样式
		 * @details 用于移除All样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShared [in] 是否Shared
		 */
		void RemoveAllStyle(bool bShared = false);

		/**
		 * @brief 获取图像String
		 * @details 用于获取图像String。具体行为由当前对象状态以及传入参数共同决定。
		 * @param image [in] 图像描述字符串
		 * @param modify [in] modify参数
		 * @return const TImageInfo* 返回结果对象指针，失败时返回 nullptr
		 */
		const TImageInfo* GetImageString(std::wstring_view image, std::wstring_view modify = {});

		// 鍒濆鍖栨嫋鎷?
		/**
		 * @brief 启用拖拽拖放
		 * @details 用于启用拖拽拖放。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool EnableDragDrop(bool bEnable);
		/**
		 * @brief 设置拖拽拖放
		 * @details 用于设置拖拽拖放。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pDragDrop [in] 拖拽拖放对象
		 */
		void SetDragDrop(IDragDropUI* pDragDrop);
		/**
		 * @brief 执行 OnDrop 操作
		 * @details 用于执行 OnDrop 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pFmtEtc [in] FmtEtc对象
		 * @param medium [in,out] medium参数
		 * @param pdwEffect [in] pdwEffect参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool OnDrop(FORMATETC* pFmtEtc, STGMEDIUM& medium,DWORD *pdwEffect);
		/**
		 * @brief 设置事件控件
		 * @details 用于设置事件控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void SetEventControl(CControlUI* pControl);


		/**
		 * @brief 判断是否Valid
		 * @details 用于判断是否Valid。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsValid();
		/**
		 * @brief 执行 AttachDialog 操作
		 * @details 用于执行 AttachDialog 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AttachDialog(CControlUI* pControl);
		/**
		 * @brief 执行 InitControls 操作
		 * @details 用于执行 InitControls 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param pParent [in] 父级控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool InitControls(CControlUI* pControl, CControlUI* pParent = NULL);
		/**
		 * @brief 执行 ReapObjects 操作
		 * @details 用于执行 ReapObjects 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void ReapObjects(CControlUI* pControl);

		/**
		 * @brief 添加选项Group
		 * @details 用于添加选项Group。具体行为由当前对象状态以及传入参数共同决定。
		 * @param groupName [in] group名称参数
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddOptionGroup(std::wstring_view groupName, CControlUI* pControl);
		/**
		 * @brief 获取选项Group
		 * @details 用于获取选项Group。具体行为由当前对象状态以及传入参数共同决定。
		 * @param groupName [in] group名称参数
		 * @return CStdPtrArray* 返回结果对象指针，失败时返回 nullptr
		 */
		CStdPtrArray* GetOptionGroup(std::wstring_view groupName);
		/**
		 * @brief 移除选项Group
		 * @details 用于移除选项Group。具体行为由当前对象状态以及传入参数共同决定。
		 * @param groupName [in] group名称参数
		 * @param pControl [in] 控件对象
		 */
		void RemoveOptionGroup(std::wstring_view groupName, CControlUI* pControl);
		/**
		 * @brief 移除All选项Groups
		 * @details 用于移除All选项Groups。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RemoveAllOptionGroups();

		/**
		 * @brief 获取焦点
		 * @details 用于获取焦点。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* GetFocus() const;
		/**
		 * @brief 设置焦点
		 * @details 用于设置焦点。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void SetFocus(CControlUI* pControl);
		/**
		 * @brief 设置焦点Needed
		 * @details 用于设置焦点Needed。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void SetFocusNeeded(CControlUI* pControl);
		/**
		 * @brief 设置No焦点Needed
		 * @details 用于设置No焦点Needed。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SetNoFocusNeeded ();

		/**
		 * @brief 设置Next页签控件
		 * @details 用于设置Next页签控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bForward [in] 是否Forward
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetNextTabControl(bool bForward = true);

		/**
		 * @brief 设置定时器
		 * @details 用于设置定时器。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param nTimerID [in] 定时器标识
		 * @param uElapse [in] Elapse标志
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetTimer(CControlUI* pControl, UINT nTimerID, UINT uElapse);
		/**
		 * @brief 注销定时器
		 * @details 用于注销定时器。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param nTimerID [in] 定时器标识
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool KillTimer(CControlUI* pControl, UINT nTimerID);
		/**
		 * @brief 注销定时器
		 * @details 用于注销定时器。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void KillTimer(CControlUI* pControl);
		/**
		 * @brief 移除AllTimers
		 * @details 用于移除AllTimers。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RemoveAllTimers();

		/**
		 * @brief 设置Capture
		 * @details 用于设置Capture。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SetCapture();
		/**
		 * @brief 执行 ReleaseCapture 操作
		 * @details 用于执行 ReleaseCapture 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ReleaseCapture();
		/**
		 * @brief 判断是否Captured
		 * @details 用于判断是否Captured。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsCaptured();

		/**
		 * @brief 判断是否Painting
		 * @details 用于判断是否Painting。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsPainting();
		/**
		 * @brief 设置Painting
		 * @details 用于设置Painting。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bIsPainting [in] 是否判断是否Painting
		 */
		void SetPainting(bool bIsPainting);

		/**
		 * @brief 添加Notifier
		 * @details 用于添加Notifier。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddNotifier(INotifyUI* pControl);
		/**
		 * @brief 移除Notifier
		 * @details 用于移除Notifier。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveNotifier(INotifyUI* pControl);   
		/**
		 * @brief 执行 SendNotify 操作
		 * @details 用于执行 SendNotify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param Msg [in,out] Msg参数
		 * @param bAsync [in] 是否Async
		 */
		void SendNotify(TNotifyUI& Msg, bool bAsync = false);
		/**
		 * @brief 执行 SendNotify 操作
		 * @details 用于执行 SendNotify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param message [in] 消息参数
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bAsync [in] 是否Async
		 */
		void SendNotify(CControlUI* pControl, std::wstring_view message, WPARAM wParam = 0, LPARAM lParam = 0, bool bAsync = false);

		/**
		 * @brief 添加Pre消息Filter
		 * @details 用于添加Pre消息Filter。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pFilter [in] Filter对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddPreMessageFilter(IMessageFilterUI* pFilter);
		/**
		 * @brief 移除Pre消息Filter
		 * @details 用于移除Pre消息Filter。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pFilter [in] Filter对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemovePreMessageFilter(IMessageFilterUI* pFilter);

		/**
		 * @brief 添加消息Filter
		 * @details 用于添加消息Filter。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pFilter [in] Filter对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddMessageFilter(IMessageFilterUI* pFilter);
		/**
		 * @brief 移除消息Filter
		 * @details 用于移除消息Filter。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pFilter [in] Filter对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveMessageFilter(IMessageFilterUI* pFilter);

		/**
		 * @brief 获取Post绘制数量
		 * @details 用于获取Post绘制数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetPostPaintCount() const;
		/**
		 * @brief 判断是否Post绘制
		 * @details 用于判断是否Post绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsPostPaint(CControlUI* pControl);
		/**
		 * @brief 添加Post绘制
		 * @details 用于添加Post绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddPostPaint(CControlUI* pControl);
		/**
		 * @brief 移除Post绘制
		 * @details 用于移除Post绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemovePostPaint(CControlUI* pControl);
		/**
		 * @brief 设置Post绘制索引
		 * @details 用于设置Post绘制索引。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetPostPaintIndex(CControlUI* pControl, int iIndex);

		/**
		 * @brief 获取Native窗口数量
		 * @details 用于获取Native窗口数量。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetNativeWindowCount() const;
		/**
		 * @brief 获取Native窗口矩形
		 * @details 用于获取Native窗口矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hChildWnd [in] h子控件Wnd参数
		 * @return 返回对应的几何结果
		 */
		RECT GetNativeWindowRect(HWND hChildWnd);
		/**
		 * @brief 添加Native窗口
		 * @details 用于添加Native窗口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @param hChildWnd [in] h子控件Wnd参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddNativeWindow(CControlUI* pControl, HWND hChildWnd);
		/**
		 * @brief 移除Native窗口
		 * @details 用于移除Native窗口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hChildWnd [in] h子控件Wnd参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveNativeWindow(HWND hChildWnd);

		/**
		 * @brief 添加延迟Cleanup
		 * @details 用于添加延迟Cleanup。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void AddDelayedCleanup(CControlUI* pControl);
		/**
		 * @brief 添加鼠标LeaveNeeded
		 * @details 用于添加鼠标LeaveNeeded。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void AddMouseLeaveNeeded(CControlUI* pControl);
		/**
		 * @brief 移除鼠标LeaveNeeded
		 * @details 用于移除鼠标LeaveNeeded。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveMouseLeaveNeeded(CControlUI* pControl);

		/**
		 * @brief 添加TranslateAccelerator
		 * @details 用于添加TranslateAccelerator。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pTranslateAccelerator [in] TranslateAccelerator对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
		/**
		 * @brief 移除TranslateAccelerator
		 * @details 用于移除TranslateAccelerator。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pTranslateAccelerator [in] TranslateAccelerator对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveTranslateAccelerator(ITranslateAccelerator *pTranslateAccelerator);
		/**
		 * @brief 执行 TranslateAccelerator 操作
		 * @details 用于执行 TranslateAccelerator 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pMsg [in] Msg对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool TranslateAccelerator(LPMSG pMsg);

		/**
		 * @brief 获取Root
		 * @details 用于获取Root。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* GetRoot() const;
		/**
		 * @brief 查找控件
		 * @details 用于查找控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindControl(POINT pt) const;
		/**
		 * @brief 查找控件
		 * @details 用于查找控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindControl(std::wstring_view name) const;
		/**
		 * @brief 查找Sub控件ByPoint
		 * @details 用于查找Sub控件ByPoint。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pParent [in] 父级控件对象
		 * @param pt [in] 坐标点
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindSubControlByPoint(CControlUI* pParent, POINT pt) const;
		/**
		 * @brief 查找Sub控件By名称
		 * @details 用于查找Sub控件By名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pParent [in] 父级控件对象
		 * @param name [in] 名称字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindSubControlByName(CControlUI* pParent, std::wstring_view name) const;
		/**
		 * @brief 查找Sub控件By类名
		 * @details 用于查找Sub控件By类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pParent [in] 父级控件对象
		 * @param className [in] 类名名称参数
		 * @param iIndex [in] 子项下标
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* FindSubControlByClass(CControlUI* pParent, std::wstring_view className, int iIndex = 0);
		/**
		 * @brief 查找SubControlsBy类名
		 * @details 用于查找SubControlsBy类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pParent [in] 父级控件对象
		 * @param className [in] 类名名称参数
		 * @return CStdPtrArray* 返回结果对象指针，失败时返回 nullptr
		 */
		CStdPtrArray* FindSubControlsByClass(CControlUI* pParent, std::wstring_view className);

		/**
		 * @brief 进入消息循环
		 * @details 用于进入消息循环。具体行为由当前对象状态以及传入参数共同决定。
		 */
		static void MessageLoop();
		/**
		 * @brief 执行 TranslateMessage 操作
		 * @details 用于执行 TranslateMessage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pMsg [in] Msg对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		static bool TranslateMessage(const LPMSG pMsg);
		/**
		 * @brief 释放框架全局资源
		 * @details 用于释放框架全局资源。具体行为由当前对象状态以及传入参数共同决定。
		 */
		static void Term();

		/**
		 * @brief 执行 deprecated 操作
		 * @details 用于执行 deprecated 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return [[ 返回 执行 deprecated 操作 的结果
		 */
		[[deprecated("Use ScaleValue/ScaleRect/ScaleSize/ScalePoint and Unscale* helpers instead.")]]
		CDPI* GetDPIObj();
		/**
		 * @brief 执行 deprecated 操作
		 * @details 用于执行 deprecated 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @return [[ 返回 执行 deprecated 操作 的结果
		 */
		[[deprecated("Use ScaleValue/ScaleRect/ScaleSize/ScalePoint and Unscale* helpers instead.")]]
		const CDPI* GetDPIObj() const;
		/**
		 * @brief 获取DPI
		 * @details 用于获取DPI。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetDPI() const;
		/**
		 * @brief 获取缩放比例
		 * @details 用于获取缩放比例。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetScale() const;
		/**
		 * @brief 获取DPIGeneration
		 * @details 用于获取DPIGeneration。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ULONGLONG 返回 获取DPIGeneration 的结果
		 */
		ULONGLONG GetDPIGeneration() const;
		/**
		 * @brief 执行 SeedDPI 操作
		 * @details 用于执行 SeedDPI 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iDPI [in] DPI值
		 */
		void SeedDPI(int iDPI);
		/**
		 * @brief 执行 SyncDPIFrom 操作
		 * @details 用于执行 SyncDPIFrom 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pSourceManager [in] 来源管理器对象
		 */
		void SyncDPIFrom(const CPaintManagerUI* pSourceManager);
		/**
		 * @brief 执行 SyncDPIFromWindow 操作
		 * @details 用于执行 SyncDPIFromWindow 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 */
		void SyncDPIFromWindow(HWND hWnd);
		/**
		 * @brief 重置DPIAssets
		 * @details 用于重置DPIAssets。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetDPIAssets();
		/**
		 * @brief 执行 RebuildFont 操作
		 * @details 用于执行 RebuildFont 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pFontInfo [in] 字体信息对象
		 */
		void RebuildFont(TFontInfo* pFontInfo);
		/**
		 * @brief 设置DPI
		 * @details 用于设置DPI。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iDPI [in] DPI值
		 */
		void SetDPI(int iDPI);
		/**
		 * @brief 设置DPI
		 * @details 用于设置DPI。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iDPI [in] DPI值
		 * @param pSuggestedWindowRect [in] Suggested窗口矩形对象
		 */
		void SetDPI(int iDPI, const RECT* pSuggestedWindowRect);
		/**
		 * @brief 设置AllDPI
		 * @details 用于设置AllDPI。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iDPI [in] DPI值
		 */
		static void SetAllDPI(int iDPI);

		/**
		 * @brief 处理窗口消息
		 * @details 用于处理窗口消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param lRes [in,out] lRes参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
		/**
		 * @brief 执行 PreMessageHandler 操作
		 * @details 用于执行 PreMessageHandler 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param lRes [in,out] lRes参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PreMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
		/**
		 * @brief 执行 UsedVirtualWnd 操作
		 * @details 用于执行 UsedVirtualWnd 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bUsed [in] 是否Used
		 */
		void UsedVirtualWnd(bool bUsed);

	private:
		/**
		 * @brief 获取FoundControls
		 * @details 用于获取FoundControls。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CStdPtrArray* 返回结果对象指针，失败时返回 nullptr
		 */
		CStdPtrArray* GetFoundControls();
		/**
		 * @brief 执行 __FindControlFromNameHash 操作
		 * @details 用于执行 __FindControlFromNameHash 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlFromNameHash(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 执行 __FindControlFromCount 操作
		 * @details 用于执行 __FindControlFromCount 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlFromCount(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 执行 __FindControlFromPoint 操作
		 * @details 用于执行 __FindControlFromPoint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlFromPoint(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 执行 __FindControlFromTab 操作
		 * @details 用于执行 __FindControlFromTab 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlFromTab(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 执行 __FindControlFromShortcut 操作
		 * @details 用于执行 __FindControlFromShortcut 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlFromShortcut(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 执行 __FindControlFromName 操作
		 * @details 用于执行 __FindControlFromName 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlFromName(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 执行 __FindControlFromClass 操作
		 * @details 用于执行 __FindControlFromClass 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlFromClass(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 执行 __FindControlsFromClass 操作
		 * @details 用于执行 __FindControlsFromClass 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlsFromClass(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 执行 __FindControlsFromUpdate 操作
		 * @details 用于执行 __FindControlsFromUpdate 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pThis [in] This对象
		 * @param pData [in] 数据对象
		 * @return CControlUI* CALLBACK 返回结果对象指针，失败时返回 nullptr
		 */
		static CControlUI* CALLBACK __FindControlsFromUpdate(CControlUI* pThis, LPVOID pData);
		/**
		 * @brief 确保DPI状态
		 * @details 用于确保DPI状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CDPI& 返回 确保DPI状态 的结果
		 */
		CDPI& EnsureDPIState();
		/**
		 * @brief 确保DPI状态
		 * @details 用于确保DPI状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return const CDPI& 返回 确保DPI状态 的结果
		 */
		const CDPI& EnsureDPIState() const;

		/**
		 * @brief 调整SharedImagesHSL 颜色
		 * @details 用于调整SharedImagesHSL 颜色。具体行为由当前对象状态以及传入参数共同决定。
		 */
		static void AdjustSharedImagesHSL();
		/**
		 * @brief 调整ImagesHSL 颜色
		 * @details 用于调整ImagesHSL 颜色。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void AdjustImagesHSL();
		/**
		 * @brief 执行 PostAsyncNotify 操作
		 * @details 用于执行 PostAsyncNotify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void PostAsyncNotify();
		/**
		 * @brief 应用渲染Preferences
		 * @details 用于应用渲染Preferences。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ApplyRenderPreferences();
		/**
		 * @brief 确保测量绘制表面
		 * @details 用于确保测量绘制表面。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool EnsureMeasureSurface();
		/**
		 * @brief 重置测量DC
		 * @details 用于重置测量DC。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetMeasureDC();
		/**
		 * @brief 更新字体文本度量信息For管理器
		 * @details 用于更新字体文本度量信息For管理器。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pFontInfo [in] 字体信息对象
		 */
		void UpdateFontTextMetricsForManager(TFontInfo* pFontInfo);
		/**
		 * @brief 确保Main渲染绘制表面
		 * @details 用于确保Main渲染绘制表面。具体行为由当前对象状态以及传入参数共同决定。
		 * @param referenceContext [in] reference上下文参数
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool EnsureMainRenderSurface(const CPaintRenderContext& referenceContext, LONG cx, LONG cy);
		/**
		 * @brief 确保LayeredBackground绘制表面
		 * @details 用于确保LayeredBackground绘制表面。具体行为由当前对象状态以及传入参数共同决定。
		 * @param referenceContext [in] reference上下文参数
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool EnsureLayeredBackgroundSurface(const CPaintRenderContext& referenceContext, LONG cx, LONG cy);
		/**
		 * @brief 获取Root布局矩形
		 * @details 用于获取Root布局矩形。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcClient [in] 客户区矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT GetRootLayoutRect(const RECT& rcClient) const;
		/**
		 * @brief 重置渲染Surfaces
		 * @details 用于重置渲染Surfaces。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetRenderSurfaces();
		/**
		 * @brief 更新ControlsNeeding布局
		 * @details 用于更新ControlsNeeding布局。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void UpdateControlsNeedingLayout();
		/**
		 * @brief 处理布局UpdatesFor绘制
		 * @details 用于处理布局UpdatesFor绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcClient [in] 客户区矩形区域
		 * @param bNeedSizeMsg [in,out] 是否需要尺寸Msg
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HandleLayoutUpdatesForPaint(const RECT& rcClient, bool& bNeedSizeMsg);
		/**
		 * @brief 执行 AccumulateLayeredUpdateRect 操作
		 * @details 用于执行 AccumulateLayeredUpdateRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcDirty [in] Dirty矩形区域
		 */
		void AccumulateLayeredUpdateRect(const RECT& rcDirty);
		/**
		 * @brief 执行 ConsumeLayeredUpdateRectForPaint 操作
		 * @details 用于执行 ConsumeLayeredUpdateRectForPaint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcPaint [in,out] 重绘区域
		 * @param rcClient [in] 客户区矩形区域
		 */
		void ConsumeLayeredUpdateRectForPaint(RECT& rcPaint, const RECT& rcClient);
		/**
		 * @brief 执行 PrepareLayeredPaintRect 操作
		 * @details 用于执行 PrepareLayeredPaintRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcPaint [in,out] 重绘区域
		 * @param rcClient [in] 客户区矩形区域
		 */
		void PrepareLayeredPaintRect(RECT& rcPaint, const RECT& rcClient);
		struct TPaintFrameState {
			RECT rcClient = { 0 };
			RECT rcPaint = { 0 };
			bool bNeedSizeMsg = false;
			DWORD dwWidth = 0;
			DWORD dwHeight = 0;
			LONGLONG nPaintStartQpc = 0;
		};
		/**
		 * @brief 执行 ExecutePaintFrame 操作
		 * @details 用于执行 ExecutePaintFrame 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param paintFrame [in] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ExecutePaintFrame(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 PrepareRenderClientRect 操作
		 * @details 用于执行 PrepareRenderClientRect 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcClient [in,out] 客户区矩形区域
		 */
		void PrepareRenderClientRect(RECT& rcClient);
		/**
		 * @brief 执行 BeginRenderDiagnosticsFrame 操作
		 * @details 用于执行 BeginRenderDiagnosticsFrame 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return LONGLONG 返回 执行 BeginRenderDiagnosticsFrame 操作 的结果
		 */
		LONGLONG BeginRenderDiagnosticsFrame() const;
		/**
		 * @brief 绘制EmptyRoot帧
		 * @details 用于绘制EmptyRoot帧。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PaintEmptyRootFrame();
		/**
		 * @brief 执行 PreparePaintFrameRects 操作
		 * @details 用于执行 PreparePaintFrameRects 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param paintFrame [in,out] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PreparePaintFrameRects(TPaintFrameState& paintFrame);
		/**
		 * @brief 处理Pending焦点For绘制
		 * @details 用于处理Pending焦点For绘制。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void HandlePendingFocusForPaint();
		/**
		 * @brief 执行 PreparePaintFrameLayout 操作
		 * @details 用于执行 PreparePaintFrameLayout 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param paintFrame [in,out] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PreparePaintFrameLayout(TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 PreparePaintFrameExecution 操作
		 * @details 用于执行 PreparePaintFrameExecution 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param paintFrame [in,out] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PreparePaintFrameExecution(TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 ExecutePaintWindowFrame 操作
		 * @details 用于执行 ExecutePaintWindowFrame 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param paintFrame [in] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ExecutePaintWindowFrame(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame) const;
		/**
		 * @brief 绘制Prepared窗口帧
		 * @details 用于绘制Prepared窗口帧。具体行为由当前对象状态以及传入参数共同决定。
		 * @param paintFrame [in] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PaintPreparedWindowFrame(const TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 ShouldSkipPaintExecution 操作
		 * @details 用于执行 ShouldSkipPaintExecution 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param paintFrame [in] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ShouldSkipPaintExecution(const TPaintFrameState& paintFrame) const;
		/**
		 * @brief 执行 RenderPrintClientFrame 操作
		 * @details 用于执行 RenderPrintClientFrame 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param lParam [in] lParam参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RenderPrintClientFrame(CPaintRenderContext& targetContext, LPARAM lParam);
		/**
		 * @brief 执行 ConsumeRenderFrameMetrics 操作
		 * @details 用于执行 ConsumeRenderFrameMetrics 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ConsumeRenderFrameMetrics();
		/**
		 * @brief 执行 FinalizeRenderDiagnostics 操作
		 * @details 用于执行 FinalizeRenderDiagnostics 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param paintFrame [in] 绘制帧参数
		 */
		void FinalizeRenderDiagnostics(const TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 FinalizePaintState 操作
		 * @details 用于执行 FinalizePaintState 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param paintFrame [in] 绘制帧参数
		 */
		void FinalizePaintState(const TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 FinalizePaintFrame 操作
		 * @details 用于执行 FinalizePaintFrame 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param paintFrame [in] 绘制帧参数
		 */
		void FinalizePaintFrame(const TPaintFrameState& paintFrame);
		/**
		 * @brief 处理绘制消息
		 * @details 用于处理绘制消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HandlePaintMessage();
		/**
		 * @brief 处理Print客户区消息
		 * @details 用于处理Print客户区消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param lParam [in] lParam参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool HandlePrintClientMessage(CPaintRenderContext& targetContext, LPARAM lParam);
		/**
		 * @brief 执行 ExecuteBatchedControlRenderPass 操作
		 * @details 用于执行 ExecuteBatchedControlRenderPass 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param bIncludeNativeWindows [in] 是否IncludeNativeWindows
		 * @param bIncludePostPaint [in] 是否IncludePost绘制
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ExecuteBatchedControlRenderPass(CPaintRenderContext& renderContext, bool bIncludeNativeWindows, bool bIncludePostPaint);
		/**
		 * @brief 执行 ExecuteBatchedDrawInfoPass 操作
		 * @details 用于执行 ExecuteBatchedDrawInfoPass 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pDrawInfo [in] 绘制信息对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool ExecuteBatchedDrawInfoPass(CPaintRenderContext& renderContext, TDrawInfo* pDrawInfo);
		/**
		 * @brief 执行 RenderControlTree 操作
		 * @details 用于执行 RenderControlTree 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @param bIncludePostPaint [in] 是否IncludePost绘制
		 */
		void RenderControlTree(CPaintRenderContext& renderContext, CControlUI* pStopControl = NULL, bool bIncludePostPaint = true);
		/**
		 * @brief 执行 RenderPostPaintControls 操作
		 * @details 用于执行 RenderPostPaintControls 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void RenderPostPaintControls(CPaintRenderContext& renderContext);
		/**
		 * @brief 执行 RenderNativeWindowsToSurface 操作
		 * @details 用于执行 RenderNativeWindowsToSurface 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RenderNativeWindowsToSurface(CPaintRenderContext& renderContext);
		/**
		 * @brief 执行 RenderLayeredBackgroundToSurface 操作
		 * @details 用于执行 RenderLayeredBackgroundToSurface 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param referenceContext [in] reference上下文参数
		 * @param paintFrame [in] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RenderLayeredBackgroundToSurface(const CPaintRenderContext& referenceContext, const TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 RenderFrameDirectToWindow 操作
		 * @details 用于执行 RenderFrameDirectToWindow 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param bIncludePostPaint [in] 是否IncludePost绘制
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RenderFrameDirectToWindow(CPaintRenderContext& targetContext, bool bIncludePostPaint = true);
		/**
		 * @brief 执行 RenderFrameToMainSurface 操作
		 * @details 用于执行 RenderFrameToMainSurface 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param referenceContext [in] reference上下文参数
		 * @param paintFrame [in] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RenderFrameToMainSurface(const CPaintRenderContext& referenceContext, const TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 PrintChildWindows 操作
		 * @details 用于执行 PrintChildWindows 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param lParam [in] lParam参数
		 */
		void PrintChildWindows(CPaintRenderContext& targetContext, LPARAM lParam);
		/**
		 * @brief 执行 PresentLayeredSurface 操作
		 * @details 用于执行 PresentLayeredSurface 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param paintFrame [in] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PresentLayeredSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 PresentWindowSurface 操作
		 * @details 用于执行 PresentWindowSurface 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param paintFrame [in] 绘制帧参数
		 */
		void PresentWindowSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame);
		/**
		 * @brief 执行 PresentMainRenderSurface 操作
		 * @details 用于执行 PresentMainRenderSurface 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param targetContext [in,out] 目标上下文参数
		 * @param paintFrame [in] 绘制帧参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool PresentMainRenderSurface(CPaintRenderContext& targetContext, const TPaintFrameState& paintFrame);
		/**
		 * @brief 重置渲染Diagnostics
		 * @details 用于重置渲染Diagnostics。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetRenderDiagnostics();
		/**
		 * @brief 更新渲染Diagnostics
		 * @details 用于更新渲染Diagnostics。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nPaintStartQpc [in] 绘制StartQpc数值
		 * @param nPaintEndQpc [in] 绘制EndQpc数值
		 */
		void UpdateRenderDiagnostics(LONGLONG nPaintStartQpc, LONGLONG nPaintEndQpc);
		/**
		 * @brief 移除Native窗口EntryAt
		 * @details 用于移除Native窗口EntryAt。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveNativeWindowEntryAt(int iIndex);
		/**
		 * @brief 清空Native窗口缓存
		 * @details 用于清空Native窗口缓存。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ClearNativeWindowCache();
		/**
		 * @brief 清空Native窗口缓存
		 * @details 用于清空Native窗口缓存。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iIndex [in] 子项下标
		 */
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
		bool m_bAnimationFramePosted;


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

