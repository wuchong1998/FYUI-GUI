#pragma once
namespace FYUI
{
	class FYUI_API WindowImplBase
		: public CWindowWnd
		, public CNotifyPump
		, public INotifyUI
		, public IMessageFilterUI
		, public IDialogBuilderCallback
		, public IQueryControlText
	{
	public:
		/**
		 * @brief 构造 WindowImplBase 对象
		 * @details 用于构造 WindowImplBase 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		WindowImplBase(){};
		/**
		 * @brief 析构 WindowImplBase 对象
		 * @details 用于析构 WindowImplBase 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~WindowImplBase(){};
		// 鍙渶涓荤獥鍙ｉ噸鍐欙紙鍒濆鍖栬祫婧愪笌澶氳瑷€鎺ュ彛锛?
		/**
		 * @brief 执行 InitResource 操作
		 * @details 用于执行 InitResource 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void InitResource(){};
		// 姣忎釜绐楀彛閮藉彲浠ラ噸鍐?
		/**
		 * @brief 执行 InitWindow 操作
		 * @details 用于执行 InitWindow 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void InitWindow(){};
		/**
		 * @brief 执行 OnFinalMessage 操作
		 * @details 用于执行 OnFinalMessage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hWnd [in] 窗口句柄
		 */
		virtual void OnFinalMessage( HWND hWnd );
		/**
		 * @brief 执行 Notify 操作
		 * @details 用于执行 Notify 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param msg [in,out] msg参数
		 */
		virtual void Notify(TNotifyUI& msg);

		/**
		 * @brief 执行 DUI_DECLARE_MESSAGE_MAP 操作
		 * @details 用于执行 DUI_DECLARE_MESSAGE_MAP 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		DUI_DECLARE_MESSAGE_MAP()
			virtual void OnClick(TNotifyUI& msg);
		/**
		 * @brief 判断是否InStatic控件
		 * @details 用于判断是否InStatic控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 * @return BOOL 返回 判断是否InStatic控件 的结果
		 */
		virtual BOOL IsInStaticControl(CControlUI *pControl);

	protected:
		/**
		 * @brief 获取Skin类型
		 * @details 用于获取Skin类型。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring GetSkinType() { return _T(""); }
		virtual std::wstring GetSkinFile() = 0;
		/**
		 * @brief 获取窗口类名名称
		 * @details 用于获取窗口类名名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetWindowClassName(void) const = 0;
		/**
		 * @brief 获取管理器名称
		 * @details 用于获取管理器名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetManagerName() { return {}; }
		virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);

		CPaintManagerUI m_pm;

	public:
		/**
		 * @brief 获取类名样式
		 * @details 用于获取类名样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetClassStyle() const;
		/**
		 * @brief 创建控件
		 * @details 用于创建控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrClass [in] 类名字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CControlUI* CreateControl(std::wstring_view pstrClass);
		/**
		 * @brief 执行 QueryControlText 操作
		 * @details 用于执行 QueryControlText 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param lpstrId [in] lpstrId参数
		 * @param lpstrType [in] lpstr类型参数
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring QueryControlText(std::wstring_view lpstrId, std::wstring_view lpstrType);

		/**
		 * @brief 处理窗口消息
		 * @details 用于处理窗口消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& bHandled);
		/**
		 * @brief 执行 OnClose 操作
		 * @details 用于执行 OnClose 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		/**
		 * @brief 执行 OnDestroy 操作
		 * @details 用于执行 OnDestroy 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

#if defined(WIN32) && !defined(UNDER_CE)
		/**
		 * @brief 执行 OnNcActivate 操作
		 * @details 用于执行 OnNcActivate 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param wParam [in] wParam参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		/**
		 * @brief 执行 OnNcCalcSize 操作
		 * @details 用于执行 OnNcCalcSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnNcPaint 操作
		 * @details 用于执行 OnNcPaint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in,out] 回调参数
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		/**
		 * @brief 执行 OnNcHitTest 操作
		 * @details 用于执行 OnNcHitTest 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnGetMinMaxInfo 操作
		 * @details 用于执行 OnGetMinMaxInfo 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnMouseWheel 操作
		 * @details 用于执行 OnMouseWheel 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		/**
		 * @brief 执行 OnMouseHover 操作
		 * @details 用于执行 OnMouseHover 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnDpiChange 操作
		 * @details 用于执行 OnDpiChange 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnDpiChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif
		/**
		 * @brief 执行 OnSize 操作
		 * @details 用于执行 OnSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnChar 操作
		 * @details 用于执行 OnChar 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnSysCommand 操作
		 * @details 用于执行 OnSysCommand 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnCreate 操作
		 * @details 用于执行 OnCreate 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 执行 OnKeyDown 操作
		 * @details 用于执行 OnKeyDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		/**
		 * @brief 执行 OnKillFocus 操作
		 * @details 用于执行 OnKillFocus 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		/**
		 * @brief 执行 OnSetFocus 操作
		 * @details 用于执行 OnSetFocus 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		/**
		 * @brief 执行 OnLButtonDown 操作
		 * @details 用于执行 OnLButtonDown 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		/**
		 * @brief 执行 OnLButtonUp 操作
		 * @details 用于执行 OnLButtonUp 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		/**
		 * @brief 执行 OnMouseMove 操作
		 * @details 用于执行 OnMouseMove 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param param [in] 回调参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
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
		 * @brief 处理Custom消息
		 * @details 用于处理Custom消息。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uMsg [in] Msg标志
		 * @param wParam [in] wParam参数
		 * @param lParam [in] lParam参数
		 * @param bHandled [in,out] 是否Handled
		 * @return LRESULT 返回对应的数值结果
		 */
		virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		/**
		 * @brief 获取样式
		 * @details 用于获取样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return LONG 返回对应的数值结果
		 */
		virtual LONG GetStyle();
	};
}



