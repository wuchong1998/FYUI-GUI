#pragma once

using namespace FYUI;

class TooltipWnd : public WindowImplBase
{
public:
	/**
	 * @brief 构造 TooltipWnd 对象
	 * @details 用于构造 TooltipWnd 对象。具体行为由当前对象状态以及传入参数共同决定。
	 */
	TooltipWnd();
	/**
	 * @brief 析构 TooltipWnd 对象
	 * @details 用于析构 TooltipWnd 对象。具体行为由当前对象状态以及传入参数共同决定。
	 */
	~TooltipWnd();
	/**
	 * @brief 执行 OnFinalMessage 操作
	 * @details 用于执行 OnFinalMessage 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param hWnd [in] 窗口句柄
	 */
	void OnFinalMessage(HWND hWnd);

	/**
	 * @brief 执行 InitWindow 操作
	 * @details 用于执行 InitWindow 操作。具体行为由当前对象状态以及传入参数共同决定。
	 */
	virtual void InitWindow() override;
	/**
	 * @brief 执行 Notify 操作
	 * @details 用于执行 Notify 操作。具体行为由当前对象状态以及传入参数共同决定。
	 * @param msg [in,out] msg参数
	 */
	virtual void Notify(TNotifyUI& msg) override;

	/**
	 * @brief 获取Skin文件
	 * @details 用于获取Skin文件。具体行为由当前对象状态以及传入参数共同决定。
	 * @return 返回对应的字符串内容
	 */
	virtual std::wstring GetSkinFile() override;
	/**
	 * @brief 获取窗口类名名称
	 * @details 用于获取窗口类名名称。具体行为由当前对象状态以及传入参数共同决定。
	 * @return 返回对应的字符串内容
	 */
	virtual std::wstring_view GetWindowClassName(void) const override;

	/**
	 * @brief 处理消息
	 * @details 用于处理消息。具体行为由当前对象状态以及传入参数共同决定。
	 * @param uMsg [in] Msg标志
	 * @param wParam [in] wParam参数
	 * @param lParam [in] lParam参数
	 * @return LRESULT 返回对应的数值结果
	 */
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	/**
	 * @brief 设置Tooltip信息
	 * @details 用于设置Tooltip信息。具体行为由当前对象状态以及传入参数共同决定。
	 * @param strText [in] 文本字符串
	 * @param rcPos [in] 位置矩形区域
	 * @param emToolTipType [in] em工具提示类型参数
	 * @param szTooltipGap [in] TooltipGap尺寸参数
	 * @param nMaxWidth [in] 最大宽度数值
	 */
	void SetTooltipInfo(const std::wstring &strText, RECT rcPos, ToolTipType emToolTipType, SIZE szTooltipGap,int nMaxWidth);

};
