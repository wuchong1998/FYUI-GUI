#pragma once

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>

namespace FYUI
{
	class CPaintRenderContext;
	class CControlUI;

	typedef CControlUI* (CALLBACK* FINDCONTROLPROC)(CControlUI*, LPVOID);

	/**
	 * @brief 控件绘制阶段枚举
	 * @details 标识 CControlUI::DoPaint 链路中的 7 个绘制位点，
	 *          供 SetPaintCallback / ClearPaintCallback 选择目标 Paint 函数。
	 *          数值顺序与 DoPaint 内部调用顺序一致，可用于排序断言。
	 */
	enum PaintStage
	{
		PaintStageBkColor = 0,        // PaintBkColor
		PaintStageBkImage,            // PaintBkImage
		PaintStageStatusImage,        // PaintStatusImage
		PaintStageForeColor,          // PaintForeColor
		PaintStageForeImage,          // PaintForeImage
		PaintStageText,               // PaintText
		PaintStageBorder,             // PaintBorder
		PaintStageCount
	};

	/**
	 * @brief 控件绘制回调函数类型
	 * @details C++11 std::function 形式的绘制回调。在每个 Paint 位点的默认绘制前后被调用，
	 *          允许调用方读取 sender 状态，并直接通过 ctx 调用 CRenderEngine::* 在控件之上绘制自定义内容。
	 * @param sender 触发回调的控件实例
	 * @param ctx    当前绘制上下文，与 PaintXxx 中的 renderContext 同一对象
	 */
	using PaintCallback = std::function<void(CControlUI* sender, CPaintRenderContext& ctx)>;

	class FYUI_API CControlUI
	{
		DECLARE_DUICONTROL(CControlUI)

	public:
		/**
		 * @brief 构造 CControlUI 对象
		 * @details 用于构造 CControlUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CControlUI();
		/**
		 * @brief 析构 CControlUI 对象
		 * @details 用于析构 CControlUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual ~CControlUI();

	public:
		/**
		 * @brief 获取名称
		 * @details 用于获取名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual const std::wstring& GetName() const;
		/**
		 * @brief 获取名称Ref
		 * @details 用于获取名称Ref。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual const std::wstring& GetNameRef() const;
		/**
		 * @brief 获取名称视图
		 * @details 用于获取名称视图。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetNameView() const;
		/**
		 * @brief 设置名称
		 * @details 用于设置名称。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 */
		virtual void SetName(std::wstring_view name);
		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		virtual LPVOID GetInterface(std::wstring_view pstrName);
		/**
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetControlFlags() const;

		/**
		 * @brief 激活对象
		 * @details 用于激活对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Activate();
		/**
		 * @brief 获取管理器
		 * @details 用于获取管理器。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CPaintManagerUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CPaintManagerUI* GetManager() const;
		/**
		 * @brief 设置管理器和父级关系
		 * @details 用于设置管理器和父级关系。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pManager [in] 管理器对象
		 * @param pParent [in] 父级控件对象
		 * @param bInit [in] 是否执行初始化
		 */
		virtual void SetManager(CPaintManagerUI* pManager, CControlUI* pParent, bool bInit = true);
		/**
		 * @brief 获取父级控件
		 * @details 用于获取父级控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CControlUI* GetParent() const;
		/**
		 * @brief 执行 setInstance 操作
		 * @details 用于执行 setInstance 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param instance [in] 模块实例句柄
		 */
		void setInstance(HINSTANCE instance = NULL);

		/**
		 * @brief 判断是否Container控件
		 * @details 用于判断是否Container控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsContainerControl() const;
		/**
		 * @brief 移除全部子项
		 * @details 用于移除全部子项。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bChildDelayed [in] 是否延迟处理子项
		 */
		virtual void RemoveAll(bool bChildDelayed);
		/**
		 * @brief 设置定时器
		 * @details 用于设置定时器。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nTimerID [in] 定时器标识
		 * @param nElapse [in] 定时周期（毫秒）
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool SetTimer(UINT nTimerID, UINT nElapse);
		/**
		 * @brief 注销定时器
		 * @details 用于注销定时器。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nTimerID [in] 定时器标识
		 */
		void KillTimer(UINT nTimerID);

		/**
		 * @brief 获取文本
		 * @details 用于获取文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring GetText() const;
		/**
		 * @brief 设置文本
		 * @details 用于设置文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 */
		virtual void SetText(std::wstring_view text);
		/**
		 * @brief 获取热状态文本
		 * @details 用于获取热状态文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring GetHotText() const;
		/**
		 * @brief 设置热状态文本
		 * @details 用于设置热状态文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 */
		virtual void SetHotText(std::wstring_view text);
		/**
		 * @brief 判断是否资源文本
		 * @details 用于判断是否资源文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsResourceText() const;
		/**
		 * @brief 设置资源文本
		 * @details 用于设置资源文本。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bResource [in] 是否资源
		 */
		virtual void SetResourceText(bool bResource);
		/**
		 * @brief 判断是否拖拽启用状态
		 * @details 用于判断是否拖拽启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsDragEnabled() const;
		/**
		 * @brief 设置拖拽启用
		 * @details 用于设置拖拽启用。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bDrag [in] 是否拖拽
		 */
		virtual void SetDragEnable(bool bDrag);
		/**
		 * @brief 判断是否拖放启用状态
		 * @details 用于判断是否拖放启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsDropEnabled() const;
		/**
		 * @brief 设置拖放启用
		 * @details 用于设置拖放启用。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bDrop [in] 是否拖放
		 */
		virtual void SetDropEnable(bool bDrop);
		/**
		 * @brief 判断是否富文本事件
		 * @details 用于判断是否富文本事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsRichEvent() const;
		/**
		 * @brief 设置富文本事件
		 * @details 用于设置富文本事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		virtual void SetRichEvent(bool bEnable);

		/**
		 * @brief 获取渐变
		 * @details 用于获取渐变。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetGradient() const;
		/**
		 * @brief 设置渐变
		 * @details 用于设置渐变。具体行为由当前对象状态以及传入参数共同决定。
		 * @param image [in] 图像描述字符串
		 */
		void SetGradient(std::wstring_view image);
		/**
		 * @brief 获取背景颜色
		 * @details 用于获取背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetBkColor() const;
		/**
		 * @brief 设置背景颜色
		 * @details 用于设置背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBackColor [in] Back颜色数值
		 * @param bIsUpdate [in] 是否判断是否更新
		 */
		void SetBkColor(DWORD dwBackColor, bool bIsUpdate = true);
		/**
		 * @brief 获取背景颜色2
		 * @details 用于获取背景颜色2。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetBkColor2() const;
		/**
		 * @brief 设置背景颜色2
		 * @details 用于设置背景颜色2。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBackColor [in] Back颜色数值
		 */
		void SetBkColor2(DWORD dwBackColor);
		/**
		 * @brief 获取背景颜色3
		 * @details 用于获取背景颜色3。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetBkColor3() const;
		/**
		 * @brief 设置背景颜色3
		 * @details 用于设置背景颜色3。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBackColor [in] Back颜色数值
		 */
		void SetBkColor3(DWORD dwBackColor);
		/**
		 * @brief 设置Disable背景颜色
		 * @details 用于设置Disable背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwDisableBkColor [in] Disable背景颜色数值
		 */
		void SetDisableBkColor(DWORD dwDisableBkColor);
		/**
		 * @brief 获取Disable背景颜色
		 * @details 用于获取Disable背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisableBkColor() const;

		/**
		 * @brief 获取前景颜色
		 * @details 用于获取前景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetForeColor() const;
		/**
		 * @brief 设置前景颜色
		 * @details 用于设置前景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwForeColor [in] 前景颜色数值
		 */
		void SetForeColor(DWORD dwForeColor);
		/**
		 * @brief 获取背景图像
		 * @details 用于获取背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetBkImage() const;
		/**
		 * @brief 设置背景图像
		 * @details 用于设置背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param image [in] 图像描述字符串
		 */
		void SetBkImage(std::wstring_view image);
		/**
		 * @brief 获取前景图像
		 * @details 用于获取前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetForeImage() const;
		/**
		 * @brief 设置前景图像
		 * @details 用于设置前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param image [in] 图像描述字符串
		 */
		void SetForeImage(std::wstring_view image);

		/**
		 * @brief 获取焦点边框颜色
		 * @details 用于获取焦点边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetFocusBorderColor() const;
		/**
		 * @brief 设置焦点边框颜色
		 * @details 用于设置焦点边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBorderColor [in] 边框颜色数值
		 */
		void SetFocusBorderColor(DWORD dwBorderColor);

		/**
		 * @brief 获取热状态边框颜色
		 * @details 控件处于鼠标悬停（Hot）状态时绘制的边框颜色，由 IsHot() 决定是否触发。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetHotBorderColor() const;
		/**
		 * @brief 设置热状态边框颜色
		 * @details 设置控件处于鼠标悬停（Hot）状态时绘制的边框颜色，配合 IsHot() 生效。
		 * @param dwBorderColor [in] 边框颜色数值
		 */
		void SetHotBorderColor(DWORD dwBorderColor);
		/**
		 * @brief 获取按下状态边框颜色
		 * @details 控件处于按下（Pushed/Captured）状态时绘制的边框颜色，由 IsPushed() 决定是否触发。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetPushedBorderColor() const;
		/**
		 * @brief 设置按下状态边框颜色
		 * @details 设置控件处于按下状态时绘制的边框颜色，配合 IsPushed() 生效。
		 * @param dwBorderColor [in] 边框颜色数值
		 */
		void SetPushedBorderColor(DWORD dwBorderColor);
		/**
		 * @brief 获取禁用状态边框颜色
		 * @details 控件处于禁用状态时绘制的边框颜色，由 IsEnabled() == false 触发。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetDisabledBorderColor() const;
		/**
		 * @brief 设置禁用状态边框颜色
		 * @details 设置控件禁用时绘制的边框颜色。
		 * @param dwBorderColor [in] 边框颜色数值
		 */
		void SetDisabledBorderColor(DWORD dwBorderColor);

		/**
		 * @brief 判断控件是否处于鼠标悬停（Hot）状态
		 * @details 基类默认始终返回 false，派生类如 CButtonUI、Layout 分隔条等可重写返回真实状态。
		 *          PaintBorder 会据此决定是否使用 HotBorderColor。
		 * @return bool 处于 Hot 状态返回 true
		 */
		virtual bool IsHot() const;
		/**
		 * @brief 判断控件是否处于按下（Pushed）状态
		 * @details 基类默认始终返回 false，派生类如 CButtonUI、Layout 分隔条捕获等可重写返回真实状态。
		 *          PaintBorder 会据此决定是否使用 PushedBorderColor。
		 * @return bool 处于 Pushed 状态返回 true
		 */
		virtual bool IsPushed() const;
		/**
		 * @brief 获取焦点背景颜色
		 * @details 用于获取焦点背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetFocusBKColor() const;
		/**
		 * @brief 设置焦点背景颜色
		 * @details 用于设置焦点背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwFocusBkColor [in] 焦点背景颜色数值
		 */
		void SetFocusBKColor(DWORD dwFocusBkColor);

		/**
		 * @brief 判断是否颜色HSL 颜色
		 * @details 用于判断是否颜色HSL 颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsColorHSL() const;
		/**
		 * @brief 设置颜色HSL 颜色
		 * @details 用于设置颜色HSL 颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bColorHSL [in] 是否颜色HSL 颜色
		 */
		void SetColorHSL(bool bColorHSL);
		bool IsShadow() const;
		void SetShadow(bool bShadow);
		int GetShadowWidth() const;
		void SetShadowWidth(int nShadowWidth);
		DWORD GetShadowColor() const;
		void SetShadowColor(DWORD dwShadowColor);
		/**
		 * @brief 获取边框圆角
		 * @details 用于获取边框圆角。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetBorderRound() const;
		/**
		 * @brief 设置边框圆角
		 * @details 用于设置边框圆角。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cxyRound [in] cxy圆角参数
		 */
		void SetBorderRound(SIZE cxyRound);
		/**
		 * @brief 绘制图像
		 * @details 用于绘制图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStrImage [in] 图像描述字符串
		 * @param pStrModify [in] StrModify对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DrawImage(CPaintRenderContext& renderContext, std::wstring_view pStrImage, std::wstring_view pStrModify = {});

		/**
		 * @brief 获取边框尺寸
		 * @details 用于获取边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetBorderSize() const;
		/**
		 * @brief 设置边框尺寸
		 * @details 用于设置边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSize [in] 尺寸数值
		 */
		void SetBorderSize(int nSize);
		/**
		 * @brief 获取边框颜色
		 * @details 用于获取边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetBorderColor() const;
		/**
		 * @brief 设置边框颜色
		 * @details 用于设置边框颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwBorderColor [in] 边框颜色数值
		 */
		void SetBorderColor(DWORD dwBorderColor);
		/**
		 * @brief 获取边框矩形尺寸
		 * @details 用于获取边框矩形尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		RECT GetBorderRectSize() const;
		/**
		 * @brief 设置边框尺寸
		 * @details 用于设置边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 */
		void SetBorderSize(RECT rc);
		/**
		 * @brief 获取Left边框尺寸
		 * @details 用于获取Left边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetLeftBorderSize() const;
		/**
		 * @brief 设置Left边框尺寸
		 * @details 用于设置Left边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSize [in] 尺寸数值
		 */
		void SetLeftBorderSize(int nSize);
		/**
		 * @brief 获取Top边框尺寸
		 * @details 用于获取Top边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetTopBorderSize() const;
		/**
		 * @brief 设置Top边框尺寸
		 * @details 用于设置Top边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSize [in] 尺寸数值
		 */
		void SetTopBorderSize(int nSize);
		/**
		 * @brief 获取Right边框尺寸
		 * @details 用于获取Right边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetRightBorderSize() const;
		/**
		 * @brief 设置Right边框尺寸
		 * @details 用于设置Right边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSize [in] 尺寸数值
		 */
		void SetRightBorderSize(int nSize);
		/**
		 * @brief 获取Bottom边框尺寸
		 * @details 用于获取Bottom边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetBottomBorderSize() const;
		/**
		 * @brief 设置Bottom边框尺寸
		 * @details 用于设置Bottom边框尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nSize [in] 尺寸数值
		 */
		void SetBottomBorderSize(int nSize);
		/**
		 * @brief 获取边框样式
		 * @details 用于获取边框样式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetBorderStyle() const;
		/**
		 * @brief 设置边框样式
		 * @details 设置控件边框绘制时使用的线条样式。当前支持 `PS_SOLID`、`PS_DASH`、`PS_DOT`、
		 * `PS_DASHDOT`、`PS_DASHDOTDOT`，其它样式会按最接近的虚线样式或实线处理。
		 * @param nStyle [in] 边框样式标志
		 */
		void SetBorderStyle(int nStyle);

		/**
		 * @brief 获取Relative位置
		 * @details 用于获取Relative位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		virtual RECT GetRelativePos() const;
		/**
		 * @brief 获取客户区位置
		 * @details 用于获取客户区位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		virtual RECT GetClientPos() const;
		/**
		 * @brief 获取位置
		 * @details 用于获取位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		virtual const RECT& GetPos() const;
		/**
		 * @brief 设置位置
		 * @details 用于设置位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		virtual void SetPos(RECT rc, bool bNeedInvalidate = true);
		/**
		 * @brief 设置Only位置
		 * @details 用于设置Only位置。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in,out] 矩形区域
		 */
		void SetOnlyPos(RECT& rc);
		/**
		 * @brief 执行 Move 操作
		 * @details 用于执行 Move 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szOffset [in] 位移尺寸
		 * @param bNeedInvalidate [in] 是否需要触发重绘
		 */
		virtual void Move(SIZE szOffset, bool bNeedInvalidate = true);
		/**
		 * @brief 获取宽度
		 * @details 用于获取宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetWidth() const;
		/**
		 * @brief 获取高度
		 * @details 用于获取高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetHeight() const;
		/**
		 * @brief 获取X
		 * @details 用于获取X。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetX() const;
		/**
		 * @brief 获取Y
		 * @details 用于获取Y。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetY() const;

		/**
		 * @brief 获取内边距
		 * @details 用于获取内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		virtual RECT GetPadding() const;
		/**
		 * @brief 设置内边距
		 * @details 用于设置内边距。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcPadding [in] 内边距矩形
		 * @param NeedUpdate [in] 需要更新参数
		 */
		virtual void SetPadding(RECT rcPadding, bool NeedUpdate = true);

		/**
		 * @brief 获取固定XY
		 * @details 用于获取固定XY。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetFixedXY() const;
		/**
		 * @brief 获取固定尺寸
		 * @details 用于获取固定尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		SIZE GetFixedSize() const;
		/**
		 * @brief 设置固定XY
		 * @details 用于设置固定XY。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szXY [in] XY尺寸参数
		 */
		virtual void SetFixedXY(SIZE szXY);
		/**
		 * @brief 获取固定宽度
		 * @details 用于获取固定宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetFixedWidth() const;
		/**
		 * @brief 获取固定高度
		 * @details 用于获取固定高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetFixedHeight() const;
		/**
		 * @brief 设置固定宽度
		 * @details 用于设置固定宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param NeedUpdate [in] 需要更新参数
		 */
		virtual void SetFixedWidth(int cx, bool NeedUpdate = true);
		/**
		 * @brief 设置固定高度
		 * @details 用于设置固定高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cy [in] cy参数
		 * @param NeedUpdate [in] 需要更新参数
		 */
		virtual void SetFixedHeight(int cy, bool NeedUpdate = true);

		/**
		 * @brief 获取最小宽度
		 * @details 用于获取最小宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMinWidth() const;
		/**
		 * @brief 获取最小高度
		 * @details 用于获取最小高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMinHeight() const;
		/**
		 * @brief 设置最小宽度
		 * @details 用于设置最小宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 */
		virtual void SetMinWidth(int cx);
		/**
		 * @brief 设置最小高度
		 * @details 用于设置最小高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cy [in] cy参数
		 */
		virtual void SetMinHeight(int cy);

		/**
		 * @brief 获取最大宽度
		 * @details 用于获取最大宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMaxWidth() const;
		/**
		 * @brief 获取最大高度
		 * @details 用于获取最大高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetMaxHeight() const;
		/**
		 * @brief 设置最大宽度
		 * @details 用于设置最大宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 */
		virtual void SetMaxWidth(int cx);
		/**
		 * @brief 设置最大高度
		 * @details 用于设置最大高度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cy [in] cy参数
		 */
		virtual void SetMaxHeight(int cy);

		/**
		 * @brief 获取浮动百分比
		 * @details 用于获取浮动百分比。具体行为由当前对象状态以及传入参数共同决定。
		 * @return TPercentInfo 返回 获取浮动百分比 的结果
		 */
		virtual TPercentInfo GetFloatPercent() const;
		/**
		 * @brief 设置浮动百分比
		 * @details 用于设置浮动百分比。具体行为由当前对象状态以及传入参数共同决定。
		 * @param piFloatPercent [in] pi浮动百分比参数
		 */
		virtual void SetFloatPercent(TPercentInfo piFloatPercent);
		/**
		 * @brief 设置浮动对齐方式
		 * @details 用于设置浮动对齐方式。具体行为由当前对象状态以及传入参数共同决定。
		 * @param uAlign [in] 对齐方式标志
		 */
		virtual void SetFloatAlign(UINT uAlign);
		/**
		 * @brief 获取浮动对齐方式
		 * @details 用于获取浮动对齐方式。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		virtual UINT GetFloatAlign() const;

		/**
		 * @brief 设置中心比例浮动
		 * @details 形如 "x,y"：控件中心定位于父控件 (x%, y%) 处。值 <=1.0 视为 0~1 比例，
		 *          >1 视为 0~100 百分比，自动归一化到 [0,1]。设置后自动启用 float。
		 * @param pstrValue [in] 形如 "0.5,0.5" 或 "50,50"
		 */
		void SetFloatingRatio(std::wstring_view pstrValue);
		/**
		 * @brief 是否启用了 floating_ratio
		 */
		bool HasFloatingRatio() const;
		/**
		 * @brief 获取 floating_ratio 横向比例（0~1）
		 */
		double GetFloatingRatioX() const;
		/**
		 * @brief 获取 floating_ratio 纵向比例（0~1）
		 */
		double GetFloatingRatioY() const;
		/**
		 * @brief 清除 floating_ratio 设置（不影响 float 标志本身）
		 */
		void ClearFloatingRatio();

		/**
		 * @brief 设置相对父右下角的像素偏移
		 * @details 形如 "x,y"：控件右下角距父右下角 (x, y) 逻辑像素，绘制时会按 DPI 缩放。
		 *          设置后自动启用 float。
		 * @param pstrValue [in] 形如 "50,50"
		 */
		void SetFloatRBPadding(std::wstring_view pstrValue);
		/**
		 * @brief 是否启用了 float_right_bottom_padding
		 */
		bool HasFloatRBPadding() const;
		/**
		 * @brief 获取 float_right_bottom_padding 的横向逻辑像素
		 */
		int GetFloatRBPaddingX() const;
		/**
		 * @brief 获取 float_right_bottom_padding 的纵向逻辑像素
		 */
		int GetFloatRBPaddingY() const;
		/**
		 * @brief 清除 float_right_bottom_padding 设置（不影响 float 标志本身）
		 */
		void ClearFloatRBPadding();

		/**
		 * @brief 获取工具提示
		 * @details 用于获取工具提示。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring GetToolTip() const;
		/**
		 * @brief 设置工具提示
		 * @details 用于设置工具提示。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 */
		virtual void SetToolTip(std::wstring_view text);
		/**
		 * @brief 设置工具提示宽度
		 * @details 用于设置工具提示宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nWidth [in] 宽度数值
		 */
		virtual void SetToolTipWidth(int nWidth);
		/**
		 * @brief 获取工具提示宽度
		 * @details 用于获取工具提示宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		virtual int GetToolTipWidth(void);
		/**
		 * @brief 设置工具提示Gap
		 * @details 用于设置工具提示Gap。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szGap [in] Gap尺寸参数
		 */
		virtual void SetToolTipGap(SIZE szGap);
		/**
		 * @brief 获取工具提示Gap
		 * @details 用于获取工具提示Gap。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的几何结果
		 */
		virtual SIZE GetToolTipGap();
		/**
		 * @brief 设置工具提示显示Mode
		 * @details 用于设置工具提示显示Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @param emToolType [in] em工具类型参数
		 */
		virtual void SetToolTipShowMode(ToolTipType emToolType);
		/**
		 * @brief 获取工具提示显示Mode
		 * @details 用于获取工具提示显示Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ToolTipType 返回 获取工具提示显示Mode 的结果
		 */
		virtual ToolTipType GetToolTipShowMode();

		/**
		 * @brief 获取光标
		 * @details 用于获取光标。具体行为由当前对象状态以及传入参数共同决定。
		 * @return WORD 返回对应的数值结果
		 */
		virtual WORD GetCursor();
		/**
		 * @brief 获取H光标
		 * @details 用于获取H光标。具体行为由当前对象状态以及传入参数共同决定。
		 * @return HCURSOR 返回 获取H光标 的结果
		 */
		virtual HCURSOR GetHCursor();
		/**
		 * @brief 设置光标
		 * @details 用于设置光标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param wCursor [in] w光标参数
		 */
		virtual void SetCursor(WORD wCursor);
		/**
		 * @brief 设置光标
		 * @details 用于设置光标。具体行为由当前对象状态以及传入参数共同决定。
		 * @param hCursor [in] h光标参数
		 */
		virtual void SetCursor(HCURSOR hCursor);

		/**
		 * @brief 获取快捷键
		 * @details 用于获取快捷键。具体行为由当前对象状态以及传入参数共同决定。
		 * @return wchar_t 返回 获取快捷键 的结果
		 */
		virtual wchar_t GetShortcut() const;
		/**
		 * @brief 设置快捷键
		 * @details 用于设置快捷键。具体行为由当前对象状态以及传入参数共同决定。
		 * @param ch [in] ch参数
		 */
		virtual void SetShortcut(wchar_t ch);

		/**
		 * @brief 判断是否上下文菜单Used
		 * @details 用于判断是否上下文菜单Used。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsContextMenuUsed() const;
		/**
		 * @brief 设置上下文菜单Used
		 * @details 用于设置上下文菜单Used。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bMenuUsed [in] 是否菜单Used
		 */
		virtual void SetContextMenuUsed(bool bMenuUsed);

		/**
		 * @brief 获取用户数据
		 * @details 用于获取用户数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual const std::wstring& GetUserData() const;
		/**
		 * @brief 获取用户数据视图
		 * @details 用于获取用户数据视图。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring_view GetUserDataView() const;
		/**
		 * @brief 设置用户数据
		 * @details 用于设置用户数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param text [in] 文本内容
		 */
		virtual void SetUserData(std::wstring_view text);
		/**
		 * @brief 设置映射用户数据
		 * @details 用于设置映射用户数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param strTextFirst [in] 文本First字符串
		 * @param pstrTextSecond [in] 文本Second字符串
		 */
		virtual void SetMapUserData(const std::wstring& strTextFirst, const std::wstring& pstrTextSecond);
		/**
		 * @brief 获取映射用户数据
		 * @details 用于获取映射用户数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param strTextFirst [in] 文本First字符串
		 * @return 返回对应的字符串内容
		 */
		virtual std::wstring& GetMapUserData(const std::wstring& strTextFirst);
		/**
		 * @brief 清空映射用户数据
		 * @details 用于清空映射用户数据。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void ClearMapUserData();
		/**
		 * @brief 执行 EraseMapUserData 操作
		 * @details 用于执行 EraseMapUserData 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param strTextFirst [in] 文本First字符串
		 */
		virtual void EraseMapUserData(const std::wstring& strTextFirst);

		/**
		 * @brief 获取标记
		 * @details 用于获取标记。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT_PTR 返回对应的数值结果
		 */
		virtual UINT_PTR GetTag() const;
		/**
		 * @brief 设置标记
		 * @details 用于设置标记。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pTag [in] 标记对象
		 */
		virtual void SetTag(UINT_PTR pTag);

		/**
		 * @brief 判断是否可见状态
		 * @details 用于判断是否可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsVisible();
		/**
		 * @brief 获取可见状态
		 * @details 用于获取可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool GetVisible() const;
		/**
		 * @brief 设置可见状态
		 * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 * @param bSendFocus [in] 是否Send焦点
		 */
		virtual void SetVisible(bool bVisible = true, bool bSendFocus = true);
		/**
		 * @brief 设置内部可见状态
		 * @details 用于设置内部可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 */
		virtual void SetInternVisible(bool bVisible = true);
		/**
		 * @brief 判断是否启用状态
		 * @details 用于判断是否启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsEnabled() const;
		/**
		 * @brief 设置启用状态
		 * @details 用于设置启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		virtual void SetEnabled(bool bEnable = true);
		/**
		 * @brief 判断是否鼠标启用状态
		 * @details 用于判断是否鼠标启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsMouseEnabled() const;
		/**
		 * @brief 设置鼠标启用状态
		 * @details 用于设置鼠标启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		virtual void SetMouseEnabled(bool bEnable = true);
		/**
		 * @brief 判断是否键盘启用状态
		 * @details 用于判断是否键盘启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsKeyboardEnabled() const;
		/**
		 * @brief 设置键盘启用状态
		 * @details 用于设置键盘启用状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bEnable [in] 是否启用
		 */
		virtual void SetKeyboardEnabled(bool bEnable = true);
		/**
		 * @brief 判断是否Focused
		 * @details 用于判断是否Focused。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool IsFocused() const;
		/**
		 * @brief 设置焦点
		 * @details 用于设置焦点。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void SetFocus();
		/**
		 * @brief 判断是否浮动
		 * @details 用于判断是否浮动。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsFloat() const;
		/**
		 * @brief 设置浮动
		 * @details 用于设置浮动。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bFloat [in] 是否浮动
		 */
		virtual void SetFloat(bool bFloat = true);

		/**
		 * @brief 查找控件
		 * @details 用于查找控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param Proc [in] Proc参数
		 * @param pData [in] 数据对象
		 * @param uFlags [in] Flags标志
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CControlUI* FindControl(FINDCONTROLPROC Proc, LPVOID pData, UINT uFlags);
		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CControlUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CControlUI* pControl);

		/**
		 * @brief 触发重绘
		 * @details 用于触发重绘。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void Invalidate();
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
		 * @brief 标记父级需要更新
		 * @details 用于标记父级需要更新。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void NeedParentUpdate();
		/**
		 * @brief 获取调整颜色
		 * @details 用于获取调整颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param dwColor [in] 颜色值
		 * @return DWORD 返回对应的数值结果
		 */
		DWORD GetAdjustColor(DWORD dwColor);

		/**
		 * @brief 初始化对象
		 * @details 用于初始化对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void Init();
		/**
		 * @brief 执行初始化逻辑
		 * @details 用于执行初始化逻辑。具体行为由当前对象状态以及传入参数共同决定。
		 */
		virtual void DoInit();
		/**
		 * @brief 响应事件
		 * @details 用于响应事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void Event(TEventUI& event);
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void DoEvent(TEventUI& event);

		/**
		 * @brief 添加Custom属性
		 * @details 用于添加Custom属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @param attr [in] attr参数
		 */
		void AddCustomAttribute(std::wstring_view name, std::wstring_view attr);
		/**
		 * @brief 获取Custom属性
		 * @details 用于获取Custom属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetCustomAttribute(std::wstring_view name) const;
		/**
		 * @brief 移除Custom属性
		 * @details 用于移除Custom属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param name [in] 名称字符串
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool RemoveCustomAttribute(std::wstring_view name);
		/**
		 * @brief 移除AllCustom属性
		 * @details 用于移除AllCustom属性。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RemoveAllCustomAttribute();

		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		virtual void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 应用属性列表
		 * @details 用于应用属性列表。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrList [in] 列表字符串
		 * @return CControlUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CControlUI* ApplyAttributeList(std::wstring_view pstrList);

		/**
		 * @brief 计算预估尺寸
		 * @details 用于计算预估尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param szAvailable [in] 可用尺寸
		 * @return 返回对应的几何结果
		 */
		virtual SIZE EstimateSize(SIZE szAvailable);
		/**
		 * @brief 执行 Paint 操作
		 * @details 用于执行 Paint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool Paint(CPaintRenderContext& renderContext, CControlUI* pStopControl = NULL);
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		virtual bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl);
		/**
		 * @brief 绘制背景颜色
		 * @details 用于绘制背景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void PaintBkColor(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制背景图像
		 * @details 用于绘制背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void PaintBkImage(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制状态图像
		 * @details 用于绘制状态图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void PaintStatusImage(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制前景颜色
		 * @details 用于绘制前景颜色。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void PaintForeColor(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制前景图像
		 * @details 用于绘制前景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void PaintForeImage(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制文本内容
		 * @details 用于绘制文本内容。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void PaintText(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制边框
		 * @details 用于绘制边框。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void PaintBorder(CPaintRenderContext& renderContext);

		/**
		 * @brief 为指定绘制阶段设置回调函数
		 * @details 在 PaintBkColor / PaintBkImage / PaintStatusImage / PaintForeColor / PaintForeImage / PaintText / PaintBorder
		 *          的默认绘制逻辑前后插入用户回调。`before` 在默认绘制前调用，`after` 在默认绘制后调用，
		 *          任意一个可传空函数表示不挂钩。回调内部可通过 ctx 直接调用 CRenderEngine::* 在控件之上绘制自定义内容。
		 *          多次为同一 stage 设置会覆盖之前的回调。
		 *          注意：回调不会改变默认绘制结果，仅在其前后做补充绘制；如需完全替代默认行为，请重写对应 Paint 虚函数。
		 *			//使用方式如下:
		 * control->SetPaintCallback(PaintStageBkColor,
			[](CControlUI* sender, CPaintRenderContext& ctx) {
				// 在默认 PaintBkColor 之前绘制（垫底层）
			},
			[](CControlUI* sender, CPaintRenderContext& ctx) {
				// 在默认 PaintBkColor 之后绘制（覆盖装饰，例如水印 / hover ring）
				RECT rc = sender->GetPos();
				CRenderEngine::DrawColor(ctx, rc, 0x40FF0000);
			});

		 * @param stage  绘制阶段枚举
		 * @param before 默认绘制前回调，可为空
		 * @param after  默认绘制后回调，可为空
		 */
		void SetPaintCallback(PaintStage stage, PaintCallback before, PaintCallback after = {});

		/**
		 * @brief 清空指定绘制阶段的回调函数
		 * @details 同时清掉该 stage 的 before 与 after 回调。stage 越界时静默忽略。
		 * @param stage 绘制阶段枚举
		 */
		void ClearPaintCallback(PaintStage stage);

		/**
		 * @brief 清空所有绘制阶段的回调函数
		 * @details 释放绑定的回调存储结构，恢复零开销状态。
		 */
		void ClearAllPaintCallbacks();

		/**
		 * @brief [内部使用] 触发指定 Paint 阶段的回调
		 * @details 由 PaintXxx 在默认绘制前后调用，封装空指针检查与边界检查。
		 *          虽暴露在 public 区域以便 PaintAfterScope RAII 工具调用，但属于内部 API，
		 *          外部代码不应直接调用。
		 * @param stage   绘制阶段枚举
		 * @param isAfter true 表示触发 after 回调，false 表示触发 before 回调
		 * @param ctx     当前绘制上下文
		 */
		void _InvokePaintCallback(PaintStage stage, bool isAfter, CPaintRenderContext& ctx);

		/**
		 * @brief 执行后置绘制
		 * @details 用于执行后置绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		virtual void DoPostPaint(CPaintRenderContext& renderContext);

		/**
		 * @brief 设置VirtualWnd
		 * @details 用于设置VirtualWnd。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 */
		void SetVirtualWnd(std::wstring_view value);
		/**
		 * @brief 获取VirtualWnd
		 * @details 用于获取VirtualWnd。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetVirtualWnd() const;

	protected:
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
		 * @brief 执行 ScaleSize 操作
		 * @details 用于执行 ScaleSize 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param sz [in] 尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE ScaleSize(SIZE sz) const;
		/**
		 * @brief 执行 ScalePoint 操作
		 * @details 用于执行 ScalePoint 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return 返回对应的几何结果
		 */
		POINT ScalePoint(POINT pt) const;
		/**
		 * @brief 执行 PixelsToLogical 操作
		 * @details 用于执行 PixelsToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param value [in] 值参数
		 * @return int 返回对应的数值结果
		 */
		int PixelsToLogical(int value) const;
		/**
		 * @brief 执行 PixelsToLogical 操作
		 * @details 用于执行 PixelsToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rc [in] 矩形区域
		 * @return 返回对应的几何结果
		 */
		RECT PixelsToLogical(RECT rc) const;
		/**
		 * @brief 执行 PixelsToLogical 操作
		 * @details 用于执行 PixelsToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param sz [in] 尺寸参数
		 * @return 返回对应的几何结果
		 */
		SIZE PixelsToLogical(SIZE sz) const;
		/**
		 * @brief 执行 PixelsToLogical 操作
		 * @details 用于执行 PixelsToLogical 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pt [in] 坐标点
		 * @return 返回对应的几何结果
		 */
		POINT PixelsToLogical(POINT pt) const;
		/**
		 * @brief 设置内边距FromPixels
		 * @details 用于设置内边距FromPixels。具体行为由当前对象状态以及传入参数共同决定。
		 * @param rcPadding [in] 内边距矩形
		 * @param NeedUpdate [in] 需要更新参数
		 */
		void SetPaddingFromPixels(RECT rcPadding, bool NeedUpdate = true);
		/**
		 * @brief 设置固定宽度FromPixels
		 * @details 用于设置固定宽度FromPixels。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cx [in] cx参数
		 * @param NeedUpdate [in] 需要更新参数
		 */
		void SetFixedWidthFromPixels(int cx, bool NeedUpdate = true);
		/**
		 * @brief 设置固定高度FromPixels
		 * @details 用于设置固定高度FromPixels。具体行为由当前对象状态以及传入参数共同决定。
		 * @param cy [in] cy参数
		 * @param NeedUpdate [in] 需要更新参数
		 */
		void SetFixedHeightFromPixels(int cy, bool NeedUpdate = true);
		/**
		 * @brief 获取当前DPIGeneration
		 * @details 用于获取当前DPIGeneration。具体行为由当前对象状态以及传入参数共同决定。
		 * @return ULONGLONG 返回 获取当前DPIGeneration 的结果
		 */
		ULONGLONG GetCurrentDPIGeneration() const;

	public:
		CEventSource OnInit;
		CEventSource OnDestroy;
		CEventSource OnSize;
		CEventSource OnEvent;
		CEventSource OnNotify;

	protected:
		// 控件上未设置任何绘制回调时为 nullptr，避免每控件常驻 7*2 个 std::function 槽位开销
		struct PaintCallbackTable
		{
			std::array<PaintCallback, PaintStageCount> beforeCb;
			std::array<PaintCallback, PaintStageCount> afterCb;
		};
		std::unique_ptr<PaintCallbackTable> m_pPaintCallbacks;

		CPaintManagerUI* m_pManager;
		CControlUI* m_pParent;
		std::wstring m_sVirtualWnd;
		std::wstring m_sName;
		bool m_bUpdateNeeded;
		bool m_bMenuUsed;
		RECT m_rcItem;
		RECT m_rcPadding;
		SIZE m_cXY;
		SIZE m_cxyFixed;
		SIZE m_cxyMin;
		SIZE m_cxyMax;
		SIZE m_szGap = { 0, 0 };

		mutable RECT m_rcPaddingScaled;
		mutable SIZE m_cXYScaled;
		mutable SIZE m_cxyFixedScaled;
		mutable SIZE m_cxyMinScaled;
		mutable SIZE m_cxyMaxScaled;
		mutable ULONGLONG m_uPaddingDpiGeneration = 0;
		mutable ULONGLONG m_uXYDpiGeneration = 0;
		mutable ULONGLONG m_uFixedDpiGeneration = 0;
		mutable ULONGLONG m_uMinDpiGeneration = 0;
		mutable ULONGLONG m_uMaxDpiGeneration = 0;

		bool m_bVisible;
		bool m_bInternVisible;
		bool m_bEnabled;
		bool m_bMouseEnabled;
		bool m_bKeyboardEnabled;
		bool m_bFocused;
		bool m_bFloat;
		TPercentInfo m_piFloatPercent;
		UINT m_uFloatAlign;
		bool m_bSetPos;

		// floating_ratio：控件中心定位于父 (x%, y%) 处；m_bHasFloatingRatio 为 true 时生效
		bool m_bHasFloatingRatio = false;
		double m_dFloatingRatioX = 0.0;
		double m_dFloatingRatioY = 0.0;

		// float_right_bottom_padding：控件右下角距父右下角的逻辑像素；m_bHasFloatRBPadding 为 true 时生效
		bool m_bHasFloatRBPadding = false;
		int m_iFloatRBPaddingX = 0;
		int m_iFloatRBPaddingY = 0;

		bool m_bRichEvent;
		bool m_bDragEnabled;
		bool m_bDropEnabled;

		bool m_bResourceText;
		bool m_bContainerControl = false;

		std::wstring m_sText;
		std::wstring m_sHotText;
		std::wstring m_sToolTip;
		wchar_t m_chShortcut;
		std::wstring m_sUserData;
		UINT_PTR m_pTag;

		std::wstring m_sGradient;
		DWORD m_dwBackColor;
		DWORD m_dwBackColor2;
		DWORD m_dwBackColor3;
		DWORD m_dwDisableBkColor = 0;
		DWORD m_dwForeColor;
		std::wstring m_sBkImage;
		std::wstring m_sForeImage;

		DWORD m_dwBorderColor;
		DWORD m_dwFocusBorderColor;
		DWORD m_dwHotBorderColor;
		DWORD m_dwPushedBorderColor;
		DWORD m_dwDisabledBorderColor;
		DWORD m_dwFocusBkColor;
		bool m_bColorHSL;
		bool m_bShadow;
		int m_nShadowWidth;
		DWORD m_dwShadowColor;
		int m_nBorderSize;
		int m_nBorderStyle;
		int m_nTooltipWidth;
		WORD m_wCursor;
		SIZE m_cxyBorderRound;
		RECT m_rcPaint;
		RECT m_rcBorderSize;
		HINSTANCE m_instance;
		HCURSOR m_Cursor = nullptr;

		std::map<std::wstring, std::wstring> m_mapUserData;
		std::map<std::wstring, std::wstring, std::less<>> m_mCustomAttrHash;

		// Pending style name applied via ApplyAttributeList() before this control
		// was attached to a manager. Resolved against m_pManager->GetStyle() in
		// SetManager() so callers can call ApplyAttributeList("Foo") before Add().
		std::wstring m_sPendingStyleName;

		ToolTipType m_emToolTipType = Tool_Bottom;
	};

} // namespace DuiLib
