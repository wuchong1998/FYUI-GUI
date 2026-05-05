#pragma once
namespace FYUI
{
	class FYUI_API CScrollBarUI : public CControlUI
	{
		DECLARE_DUICONTROL(CScrollBarUI)
	public:
		/**
		 * @brief 构造 CScrollBarUI 对象
		 * @details 用于构造 CScrollBarUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CScrollBarUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName);

		/**
		 * @brief 获取所属对象
		 * @details 用于获取所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CContainerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CContainerUI* GetOwner() const;
		/**
		 * @brief 设置所属对象
		 * @details 用于设置所属对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pOwner [in] 所属对象
		 */
		void SetOwner(CContainerUI* pOwner);

		/**
		 * @brief 设置可见状态
		 * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 */
		void SetVisible(bool bVisible = true);
		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		void SetEnabled(bool bEnable = true);
		/**
		 * @brief 设置焦点
		 * @details 用于设置焦点。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void SetFocus();

		/**
		 * @brief 判断是否水平
		 * @details 用于判断是否水平。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsHorizontal();
		/**
		 * @brief 设置水平
		 * @details 用于设置水平。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bHorizontal [in] 是否水平
		 */
		void SetHorizontal(bool bHorizontal = true);
		/**
		 * @brief 获取滚动Range
		 * @details 用于获取滚动Range。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetScrollRange() const;
		/**
		 * @brief 设置滚动Range
		 * @details 用于设置滚动Range。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nRange [in] Range数值
		 * @param bIsSetMax [in] 是否判断是否设置最大
		 */
		void SetScrollRange(int nRange, bool bIsSetMax = true);
		/**
		 * @brief 获取滚动位置
		 * @details 用于获取滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetScrollPos() const;
		/**
		 * @brief 设置滚动位置
		 * @details 用于设置滚动位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nPos [in] 位置数值
		 * @param bIsSetMax [in] 是否判断是否设置最大
		 */
		void SetScrollPos(int nPos, bool bIsSetMax = true);
		/**
		 * @brief 获取行尺寸
		 * @details 用于获取行尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetLineSize() const;
		/**
		 * @brief 设置行尺寸
		 * @details 用于设置行尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSize [in] 尺寸数值
		 */
		void SetLineSize(int nSize);
		/**
		 * @brief 获取最小滑块尺寸
		 * @details 用于获取最小滑块尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMinThumbSize() const;
		/**
		 * @brief 设置最小滑块尺寸
		 * @details 用于设置最小滑块尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSize [in] 尺寸数值
		 */
		void SetMinThumbSize(int nSize);

		/**
		 * @brief 获取鼠标Down
		 * @details 用于获取鼠标Down。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetMouseDown() { return m_bMouseDown; }

		bool GetShowButton1();
		/**
		 * @brief 设置显示按钮1
		 * @details 用于设置显示按钮1。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShow [in] 是否显示
		 */
		void SetShowButton1(bool bShow);
		/**
		 * @brief 获取按钮1Normal图像
		 * @details 用于获取按钮1Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetButton1NormalImage() const;
		/**
		 * @brief 设置按钮1Normal图像
		 * @details 用于设置按钮1Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetButton1NormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取按钮1热状态图像
		 * @details 用于获取按钮1热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetButton1HotImage() const;
		/**
		 * @brief 设置按钮1热状态图像
		 * @details 用于设置按钮1热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetButton1HotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取按钮1Pushed图像
		 * @details 用于获取按钮1Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetButton1PushedImage() const;
		/**
		 * @brief 设置按钮1Pushed图像
		 * @details 用于设置按钮1Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetButton1PushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取按钮1Disabled图像
		 * @details 用于获取按钮1Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetButton1DisabledImage() const;
		/**
		 * @brief 设置按钮1Disabled图像
		 * @details 用于设置按钮1Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetButton1DisabledImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取显示按钮2
		 * @details 用于获取显示按钮2。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetShowButton2();
		/**
		 * @brief 设置显示按钮2
		 * @details 用于设置显示按钮2。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShow [in] 是否显示
		 */
		void SetShowButton2(bool bShow);
		/**
		 * @brief 获取按钮2Normal图像
		 * @details 用于获取按钮2Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetButton2NormalImage() const;
		/**
		 * @brief 设置按钮2Normal图像
		 * @details 用于设置按钮2Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetButton2NormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取按钮2热状态图像
		 * @details 用于获取按钮2热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetButton2HotImage() const;
		/**
		 * @brief 设置按钮2热状态图像
		 * @details 用于设置按钮2热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetButton2HotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取按钮2Pushed图像
		 * @details 用于获取按钮2Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetButton2PushedImage() const;
		/**
		 * @brief 设置按钮2Pushed图像
		 * @details 用于设置按钮2Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetButton2PushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取按钮2Disabled图像
		 * @details 用于获取按钮2Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetButton2DisabledImage() const;
		/**
		 * @brief 设置按钮2Disabled图像
		 * @details 用于设置按钮2Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetButton2DisabledImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取滑块Normal图像
		 * @details 用于获取滑块Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetThumbNormalImage() const;
		/**
		 * @brief 设置滑块Normal图像
		 * @details 用于设置滑块Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetThumbNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取滑块热状态图像
		 * @details 用于获取滑块热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetThumbHotImage() const;
		/**
		 * @brief 设置滑块热状态图像
		 * @details 用于设置滑块热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetThumbHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取滑块Pushed图像
		 * @details 用于获取滑块Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetThumbPushedImage() const;
		/**
		 * @brief 设置滑块Pushed图像
		 * @details 用于设置滑块Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetThumbPushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取滑块Disabled图像
		 * @details 用于获取滑块Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetThumbDisabledImage() const;
		/**
		 * @brief 设置滑块Disabled图像
		 * @details 用于设置滑块Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetThumbDisabledImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取RailNormal图像
		 * @details 用于获取RailNormal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetRailNormalImage() const;
		/**
		 * @brief 设置RailNormal图像
		 * @details 用于设置RailNormal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetRailNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取Rail热状态图像
		 * @details 用于获取Rail热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetRailHotImage() const;
		/**
		 * @brief 设置Rail热状态图像
		 * @details 用于设置Rail热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetRailHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取RailPushed图像
		 * @details 用于获取RailPushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetRailPushedImage() const;
		/**
		 * @brief 设置RailPushed图像
		 * @details 用于设置RailPushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetRailPushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取RailDisabled图像
		 * @details 用于获取RailDisabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetRailDisabledImage() const;
		/**
		 * @brief 设置RailDisabled图像
		 * @details 用于设置RailDisabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetRailDisabledImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取背景Normal图像
		 * @details 用于获取背景Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetBkNormalImage() const;
		/**
		 * @brief 设置背景Normal图像
		 * @details 用于设置背景Normal图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetBkNormalImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取背景热状态图像
		 * @details 用于获取背景热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetBkHotImage() const;
		/**
		 * @brief 设置背景热状态图像
		 * @details 用于设置背景热状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetBkHotImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取背景Pushed图像
		 * @details 用于获取背景Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetBkPushedImage() const;
		/**
		 * @brief 设置背景Pushed图像
		 * @details 用于设置背景Pushed图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetBkPushedImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取背景Disabled图像
		 * @details 用于获取背景Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetBkDisabledImage() const;
		/**
		 * @brief 设置背景Disabled图像
		 * @details 用于设置背景Disabled图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetBkDisabledImage(std::wstring_view pStrImage);

		/**
		 * @brief 获取显示
		 * @details 用于获取显示。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetShow();
		/**
		 * @brief 设置显示
		 * @details 用于设置显示。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bShow [in] 是否显示
		 */
		void SetShow(bool bShow);

		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;

		/**
		 * @brief 绘制背景
		 * @details 用于绘制背景。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintBk(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制按钮1
		 * @details 用于绘制按钮1。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintButton1(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制按钮2
		 * @details 用于绘制按钮2。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintButton2(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制滑块
		 * @details 用于绘制滑块。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintThumb(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制Rail
		 * @details 用于绘制Rail。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintRail(CPaintRenderContext& renderContext);
		/**
		 * @brief 设置HSpace
		 * @details 用于设置HSpace。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 */
		void SetHSpace(int cx);
		/**
		 * @brief 设置VSpace
		 * @details 用于设置VSpace。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cy [in] cy参数
		 */
		void SetVSpace(int cy);

		/**
		 * @brief 获取HSpace
		 * @details 用于获取HSpace。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetHSpace() const;
		/**
		 * @brief 获取VSpace
		 * @details 用于获取VSpace。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetVSpace() const;

	protected:
		bool UseBuiltInScrollBarStyle() const;
		int GetEffectiveMinThumbSize(int nMainAxisLength) const;
		void PaintBuiltInThumb(CPaintRenderContext& renderContext, const RECT& rcThumb) const;
		void PaintBuiltInButton(CPaintRenderContext& renderContext, const RECT& rcButton, bool firstButton, UINT buttonState) const;

		enum
		{
			DEFAULT_SCROLLBAR_SIZE = 8,
			DEFAULT_TIMERID = 10,
		};

		bool m_bMouseDown = false;

		bool m_bShow;
		bool m_bHorizontal;
		__int64 m_nRange;
		__int64 m_nScrollPos;
		int m_nLineSize;
		int m_nMinThumbSize;
		bool m_bMinThumbSizeExplicit = false;
		CContainerUI* m_pOwner;
		POINT m_ptLastMouse;
		int m_nLastScrollPos;
		int m_nLastScrollOffset;
		int m_nScrollRepeatDelay;
		int m_nSpaceX;
		int m_nSpaceY;

		std::wstring m_sBkNormalImage;
		std::wstring m_sBkHotImage;
		std::wstring m_sBkPushedImage;
		std::wstring m_sBkDisabledImage;

		bool m_bShowButton1;
		RECT m_rcButton1;
		UINT m_uButton1State;
		std::wstring m_sButton1NormalImage;
		std::wstring m_sButton1HotImage;
		std::wstring m_sButton1PushedImage;
		std::wstring m_sButton1DisabledImage;

		bool m_bShowButton2;
		RECT m_rcButton2;
		UINT m_uButton2State;
		std::wstring m_sButton2NormalImage;
		std::wstring m_sButton2HotImage;
		std::wstring m_sButton2PushedImage;
		std::wstring m_sButton2DisabledImage;

		RECT m_rcThumb;
		UINT m_uThumbState;
		std::wstring m_sThumbNormalImage;
		std::wstring m_sThumbHotImage;
		std::wstring m_sThumbPushedImage;
		std::wstring m_sThumbDisabledImage;

		std::wstring m_sRailNormalImage;
		std::wstring m_sRailHotImage;
		std::wstring m_sRailPushedImage;
		std::wstring m_sRailDisabledImage;

		std::wstring m_sImageModify;
	};
}

