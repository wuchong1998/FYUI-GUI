#pragma once

#include <ShellAPI.h>

namespace FYUI
{
	class FYUI_API CTrayIcon
	{
	public:
		CTrayIcon(void);
		~CTrayIcon(void);

	public:
		void CreateTrayIcon(HWND _RecvHwnd, UINT _IconIDResource, std::wstring_view _ToolTipText = {}, UINT _Message = NULL);
		void DeleteTrayIcon();
		bool SetTooltipText(std::wstring_view _ToolTipText);
		bool SetTooltipText(UINT _IDResource);
		std::wstring GetTooltipText() const;

		bool SetIcon(HICON _Hicon);
		bool SetIcon(std::wstring_view _IconFile);
		bool SetIcon(UINT _IDResource);
		HICON GetIcon() const;
		void SetHideIcon();
		void SetShowIcon();
		void RemoveIcon();
		bool Enabled(){return m_bEnabled;};
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
