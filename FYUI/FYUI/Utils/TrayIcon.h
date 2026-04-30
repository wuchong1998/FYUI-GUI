#pragma once

#include <ShellAPI.h>

namespace FYUI
{
	class FYUI_API CTrayIcon
	{
	public:
		/**
		 * @brief 构造 CTrayIcon 对象
		 * @details 用于构造 CTrayIcon 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CTrayIcon(void);
		/**
		 * @brief 析构 CTrayIcon 对象
		 * @details 用于析构 CTrayIcon 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CTrayIcon(void);

	public:
		/**
		 * @brief 创建托盘图标
		 * @details 用于创建托盘图标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _RecvHwnd [in] RecvHwnd参数
		 * @param _IconIDResource [in] 图标ID资源参数
		 * @param _ToolTipText [in] 工具提示文本参数
		 * @param _Message [in] 消息参数
		 */
		void CreateTrayIcon(HWND _RecvHwnd, UINT _IconIDResource, std::wstring_view _ToolTipText = {}, UINT _Message = NULL);
		/**
		 * @brief 删除托盘图标
		 * @details 用于删除托盘图标。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void DeleteTrayIcon();
		/**
		 * @brief 设置Tooltip文本
		 * @details 用于设置Tooltip文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _ToolTipText [in] 工具提示文本参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetTooltipText(std::wstring_view _ToolTipText);
		/**
		 * @brief 设置Tooltip文本
		 * @details 用于设置Tooltip文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _IDResource [in] ID资源参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetTooltipText(UINT _IDResource);
		/**
		 * @brief 获取Tooltip文本
		 * @details 用于获取Tooltip文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring GetTooltipText() const;

		/**
		 * @brief 设置图标
		 * @details 用于设置图标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _Hicon [in] Hicon参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetIcon(HICON _Hicon);
		/**
		 * @brief 设置图标
		 * @details 用于设置图标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _IconFile [in] 图标文件参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetIcon(std::wstring_view _IconFile);
		/**
		 * @brief 设置图标
		 * @details 用于设置图标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param _IDResource [in] ID资源参数
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetIcon(UINT _IDResource);
		/**
		 * @brief 获取图标
		 * @details 用于获取图标。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HICON 返回 获取图标 的结果
		 */
		HICON GetIcon() const;
		/**
		 * @brief 设置隐藏图标
		 * @details 用于设置隐藏图标。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SetHideIcon();
		/**
		 * @brief 设置显示图标
		 * @details 用于设置显示图标。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SetShowIcon();
		/**
		 * @brief 移除图标
		 * @details 用于移除图标。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RemoveIcon();
		/**
		 * @brief 启用d
		 * @details 用于启用d。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool Enabled(){return m_bEnabled;};
		/**
		 * @brief 判断是否可见状态
		 * @details 用于判断是否可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsVisible(){return !m_bVisible;};

	private:
		bool m_bEnabled;
		bool m_bVisible;
		HWND m_hWnd;
		UINT m_uMessage;
		HICON m_hIcon;
		NOTIFYICONDATA	m_trayData;
	};
}
