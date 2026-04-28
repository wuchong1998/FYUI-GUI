#pragma once

using namespace FYUI;

class TooltipWnd : public WindowImplBase
{
public:
	TooltipWnd();
	~TooltipWnd();
	void OnFinalMessage(HWND hWnd);

	virtual void InitWindow() override;
	virtual void Notify(TNotifyUI& msg) override;

	virtual std::wstring GetSkinFile() override;
	virtual std::wstring_view GetWindowClassName(void) const override;

	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	void SetTooltipInfo(const std::wstring &strText, RECT rcPos, ToolTipType emToolTipType, SIZE szTooltipGap,int nMaxWidth);

};
