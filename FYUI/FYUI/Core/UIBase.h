
#pragma once

#include <map>
#include <string_view>

namespace FYUI 
{
	/////////////////////////////////////////////////////////////////////////////////////
	//

#define UI_WNDSTYLE_CONTAINER  (0)
#define UI_WNDSTYLE_FRAME      (WS_VISIBLE | WS_OVERLAPPEDWINDOW)
#define UI_WNDSTYLE_CHILD      (WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
#define UI_WNDSTYLE_DIALOG     (WS_VISIBLE | WS_POPUPWINDOW | WS_CAPTION | WS_DLGFRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)

#define UI_WNDSTYLE_EX_FRAME   (WS_EX_WINDOWEDGE)
#define UI_WNDSTYLE_EX_DIALOG  (WS_EX_TOOLWINDOW | WS_EX_DLGMODALFRAME)

#define UI_CLASSSTYLE_CONTAINER  (0)
#define UI_CLASSSTYLE_FRAME      (CS_VREDRAW | CS_HREDRAW)
#define UI_CLASSSTYLE_CHILD      (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_SAVEBITS)
#define UI_CLASSSTYLE_DIALOG     (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_SAVEBITS)


	/////////////////////////////////////////////////////////////////////////////////////
	//
#ifndef ASSERT
#define ASSERT(expr)  _ASSERTE(expr)
#endif

#ifdef _DEBUG
#ifndef DUITRACE
#define DUITRACE DUI__Trace
#endif
#define DUITRACEMSG DUI__TraceMsg
#else
#ifndef DUITRACE
#define DUITRACE
#endif
#define DUITRACEMSG _T("")
#endif

	/**
	 * @brief 执行 DUI__Trace 操作
	 * @details 用于执行 DUI__Trace 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param pstrFormat [in] Format字符串
	 * @param ... [in] 可变参数列表
	 */
	void FYUI_API DUI__Trace(std::wstring_view pstrFormat, ...);
	/**
	 * @brief 执行 DUI__TraceMsg 操作
	 * @details 用于执行 DUI__TraceMsg 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param uMsg [in] Msg标志
	 * @return 返回对应的字符串内容
	 */
	std::wstring_view FYUI_API DUI__TraceMsg(UINT uMsg);

	/////////////////////////////////////////////////////////////////////////////////////
	//

	class FYUI_API CNotifyPump
	{
	public:
		/**
		 * @brief 添加VirtualWnd
		 * @details 用于添加VirtualWnd。具体行为由当前对象状态以及传入参数共同决定。
		 * @param strName [in] 名称字符串
		 * @param pObject [in] Object对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool AddVirtualWnd(std::wstring strName, CNotifyPump* pObject);
		/**
		 * @brief 移除VirtualWnd
		 * @details 用于移除VirtualWnd。具体行为由当前对象状态以及传入参数共同决定。
		 * @param strName [in] 名称字符串
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveVirtualWnd(std::wstring strName);
		/**
		 * @brief 执行 NotifyPump 操作
		 * @details 用于执行 NotifyPump 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in,out] msg参数
		 */
		void NotifyPump(TNotifyUI& msg);
		/**
		 * @brief 执行 LoopDispatch 操作
		 * @details 用于执行 LoopDispatch 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in,out] msg参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool LoopDispatch(TNotifyUI& msg);
		/**
		 * @brief 执行 DUI_DECLARE_MESSAGE_MAP 操作
		 * @details 用于执行 DUI_DECLARE_MESSAGE_MAP 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		DUI_DECLARE_MESSAGE_MAP()
	private:
		std::map<std::wstring, CNotifyPump*, std::less<>> m_VirtualWndMap;
	};

	class FYUI_API CWindowWnd
	{
	public:
		/**
		 * @brief 构造 CWindowWnd 对象
		 * @details 用于构造 CWindowWnd 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CWindowWnd();

		/**
		 * @brief 获取HWND
		 * @details 用于获取HWND。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HWND 返回 获取HWND 的结果
		 */
		HWND GetHWND() const;
		/**
		 * @brief 执行 operator HWND 运算
		 * @details 用于执行 operator HWND 运算。具体行为由当前对象状态以及传入参数共同决定。
		 */
		operator HWND() const;

		/**
		 * @brief 启用Unicode
		 * @details 用于启用Unicode。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void EnableUnicode();

		/**
		 * @brief 执行 RegisterWindowClass 操作
		 * @details 用于执行 RegisterWindowClass 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RegisterWindowClass();
		/**
		 * @brief 执行 RegisterSuperclass 操作
		 * @details 用于执行 RegisterSuperclass 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RegisterSuperclass();

		/**
		 * @brief 执行 Create 操作
		 * @details 用于执行 Create 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hwndParent [in] hwnd父级控件参数
		 * @param pstrName [in] 属性名称
		 * @param dwStyle [in] 样式数值
		 * @param dwExStyle [in] Ex样式数值
		 * @param rc [in] 矩形区域
		 * @param hMenu [in] h菜单参数
		 * @return HWND 返回 执行 Create 操作 的结果
		 */
		HWND Create(HWND hwndParent, std::wstring_view pstrName, DWORD dwStyle, DWORD dwExStyle, const RECT rc, HMENU hMenu = NULL);
		/**
		 * @brief 执行 Create 操作
		 * @details 用于执行 Create 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hwndParent [in] hwnd父级控件参数
		 * @param pstrName [in] 属性名称
		 * @param dwStyle [in] 样式数值
		 * @param dwExStyle [in] Ex样式数值
		 * @param x [in] x参数
		 * @param y [in] y参数
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 * @param hMenu [in] h菜单参数
		 * @return HWND 返回 执行 Create 操作 的结果
		 */
		HWND Create(HWND hwndParent, std::wstring_view pstrName, DWORD dwStyle, DWORD dwExStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT, HMENU hMenu = NULL);
		/**
		 * @brief 创建Dui窗口
		 * @details 用于创建Dui窗口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hwndParent [in] hwnd父级控件参数
		 * @param pstrWindowName [in] 窗口名称字符串
		 * @param dwStyle [in] 样式数值
		 * @param dwExStyle [in] Ex样式数值
		 * @return HWND 返回 创建Dui窗口 的结果
		 */
		HWND CreateDuiWindow(HWND hwndParent, std::wstring_view pstrWindowName, DWORD dwStyle = 0, DWORD dwExStyle = 0);

		/**
		 * @brief 执行 Subclass 操作
		 * @details 用于执行 Subclass 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 * @return HWND 返回 执行 Subclass 操作 的结果
		 */
		HWND Subclass(HWND hWnd);
		/**
		 * @brief 执行 Unsubclass 操作
		 * @details 用于执行 Unsubclass 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Unsubclass();
		/**
		 * @brief 显示窗口
		 * @details 用于显示窗口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShow [in] 是否显示
		 * @param bTakeFocus [in] 是否设置输入焦点
		 */
		void ShowWindow(bool bShow = true, bool bTakeFocus = true);
		/**
		 * @brief 显示Modal
		 * @details 用于显示Modal。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT ShowModal();
		/**
		 * @brief 显示ModalFake
		 * @details 用于显示ModalFake。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ShowModalFake();
		/**
		 * @brief 执行 Close 操作
		 * @details 用于执行 Close 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nRet [in] Ret数值
		 */
		void Close(UINT nRet = IDOK);
		/**
		 * @brief 执行 CenterWindow 操作
		 * @details 用于执行 CenterWindow 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void CenterWindow();	// 灞呬腑锛屾敮鎸佹墿灞曞睆骞?
		void SetIcon(UINT nRes);

		/**
		 * @brief 执行 SendMessage 操作
		 * @details 用于执行 SendMessage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @return LRESULT 返回对应的数值结果
		 */
		LRESULT SendMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);
		/**
		 * @brief 执行 PostMessage 操作
		 * @details 用于执行 PostMessage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @return LRESULT 返回对应的数值结果
		 */
		LRESULT PostMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);
		/**
		 * @brief 执行 ResizeClient 操作
		 * @details 用于执行 ResizeClient 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param cy [in] cy参数
		 */
		void ResizeClient(int cx = -1, int cy = -1);

	protected:
		/**
		 * @brief 获取窗口类名名称
		 * @details 用于获取窗口类名名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetWindowClassName() const = 0;
		/**
		 * @brief 获取Super类名名称
		 * @details 用于获取Super类名名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetSuperClassName() const;
		/**
		 * @brief 获取类名样式
		 * @details 用于获取类名样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetClassStyle() const;

		/**
		 * @brief 处理消息
		 * @details 用于处理消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		/**
		 * @brief 执行 OnFinalMessage 操作
		 * @details 用于执行 OnFinalMessage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 */
		virtual void OnFinalMessage(HWND hWnd);

		/**
		 * @brief 执行 __WndProc 操作
		 * @details 用于执行 __WndProc 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @return LRESULT CALLBACK 返回 执行 __WndProc 操作 的结果
		 */
		static LRESULT CALLBACK __WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		/**
		 * @brief 执行 __ControlProc 操作
		 * @details 用于执行 __ControlProc 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @return LRESULT CALLBACK 返回 执行 __ControlProc 操作 的结果
		 */
		static LRESULT CALLBACK __ControlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	protected:
		HWND m_hWnd;
		WNDPROC m_OldWndProc;
		bool m_bSubclassed;
		bool m_bUnicode;
		bool m_bFakeModal;
	};

} 


