#pragma once

#include <vector>

#include "../Core/Render/UIRenderSurface.h"

namespace FYUI
{
	class FYUI_API CWebpAnimUI : public CControlUI
	{
		enum
		{
			EVENT_TIMER_ID = 101,
		};

		DECLARE_DUICONTROL(CWebpAnimUI)

	public:
		enum WebpHideAction
		{
			WebpHideAction_Continue = 0,
			WebpHideAction_Pause,
			WebpHideAction_Stop,
		};
		enum WebpPlaybackState
		{
			WebpPlaybackState_Stopped = 0,
			WebpPlaybackState_Playing,
			WebpPlaybackState_Paused,
			WebpPlaybackState_Completed,
		};

		/**
		 * @brief 构造 WebP 动画控件
		 * @details 初始化控件的播放状态、资源来源和自动播放配置。控件既可以从 `bkimage` 指向的资源路径加载，
		 * 也可以通过 `SetWebpData` 直接接收内存中的 WebP 数据。
		 */
		CWebpAnimUI();
		/**
		 * @brief 析构 WebP 动画控件
		 * @details 停止计时器并释放已经解码的所有 WebP 帧位图。
		 */
		~CWebpAnimUI() override;

		/**
		 * @brief 获取控件类名
		 * @details 返回 FYUI 运行时使用的类名字符串。
		 * @return std::wstring_view 返回 `WebpAnimUI`
		 */
		std::wstring_view GetClass() const override;
		/**
		 * @brief 获取指定接口
		 * @details 当传入 `DUI_CTR_WEBPANIM` 时返回当前控件实例指针，便于外部按接口类型访问专有能力。
		 * @param pstrName [in] 接口名称
		 * @return LPVOID 匹配成功返回当前控件指针，否则返回基类接口结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName) override;
		/**
		 * @brief 执行控件初始化
		 * @details 在控件初次挂接到管理器后尝试加载 WebP 数据，并根据配置决定是否自动开始播放。
		 */
		void DoInit();
		/**
		 * @brief 绘制当前 WebP 帧
		 * @details 若数据尚未解码，会在首次绘制前自动完成加载；静态 WebP 绘制第 1 帧，动画 WebP 绘制当前播放帧。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 始终返回 true，表示该控件已处理自己的绘制逻辑
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/**
		 * @brief 处理控件事件
		 * @details 目前主要响应内部计时器事件，用于推进 WebP 动画帧。
		 * @param event [in,out] 事件对象
		 */
		void DoEvent(TEventUI& event);
		/**
		 * @brief 设置控件可见状态
		 * @details 控件隐藏时会暂停动画推进，重新显示后如果开启了自动播放则会继续播放。
		 * @param bVisible [in] 是否可见
		 */
		void SetVisible(bool bVisible = true, bool bSendFocus = true) override;
		/**
		 * @brief 设置控件内部可见状态
		 * @details 当控件所在 Tab/Page 被切换时，FYUI 可能只更新内部可见标记。WebP 动画控件重载该接口后，
		 * 可以在页面重新显示时自动恢复播放，在页面隐藏时停止内部计时器。
		 * @param bVisible [in] 是否内部可见
		 */
		void SetInternVisible(bool bVisible = true) override;
		/**
		 * @brief 设置 XML 属性
		 * @details 支持 `bkimage`、`autoplay`、`autosize`、`loop`、`pauseonhide`、`hideaction` 六个常用属性。
		 * 其中 `loop` 控制是否循环播放，`pauseonhide` 是便捷布尔开关，`hideaction` 支持 `continue`、`pause`、`stop`
		 * 三种隐藏策略，`bkimage` 用于指定 WebP 资源路径。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);

		/**
		 * @brief 设置 WebP 资源路径
		 * @details 接收与 FYUI 其它图片控件一致的 `bkimage` 路径字符串，并在需要时自动从资源目录或 zip 中读取 WebP 数据。
		 * @param pStrImage [in] WebP 资源路径描述字符串
		 */
		void SetBkImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取当前 WebP 资源路径
		 * @details 返回最近一次通过 `SetBkImage` 设置的资源路径；如果当前数据来自内存，则返回空字符串。
		 * @return std::wstring_view WebP 资源路径
		 */
		std::wstring_view GetBkImage() const;

		/**
		 * @brief 直接设置内存中的 WebP 数据
		 * @details 控件会复制一份传入的 WebP 二进制数据并立即解码，适合业务层从网络、数据库或自定义资源系统拿到内存图后直接播放。
		 * 设置成功后，控件会清空 `bkimage` 路径来源并改以内存数据作为唯一播放源。
		 * @param pData [in] WebP 二进制数据起始地址
		 * @param dwSize [in] WebP 二进制数据长度，单位为字节
		 * @param bInvalidate [in] 是否在成功加载后立即触发重绘
		 * @return bool 设置并解码成功返回 true，数据为空或解码失败返回 false
		 */
		bool SetWebpData(const void* pData, DWORD dwSize, bool bInvalidate = true);
		/**
		 * @brief 清空内存 WebP 数据源
		 * @details 释放已经缓存的内存 WebP 数据和帧位图，并清除当前资源路径。
		 */
		void ClearWebpData();

		/**
		 * @brief 设置是否自动播放
		 * @details 自动播放开启后，控件在可见且已挂接管理器时会自动启动动画定时器；静态 WebP 仍只显示首帧。
		 * @param bIsAuto [in] 是否自动播放
		 */
		void SetAutoPlay(bool bIsAuto = true);
		/**
		 * @brief 查询是否自动播放
		 * @details 返回当前控件的自动播放开关状态。
		 * @return bool 自动播放开启返回 true，否则返回 false
		 */
		bool IsAutoPlay() const;
		/**
		 * @brief 设置是否按 WebP 原始尺寸自动定宽高
		 * @details 开启后，控件加载成功时会把固定宽高设置为 WebP 画布尺寸。
		 * @param bIsAuto [in] 是否自动按原始尺寸设置大小
		 */
		void SetAutoSize(bool bIsAuto = true);
		/**
		 * @brief 查询是否启用自动尺寸
		 * @details 返回当前控件的自动尺寸配置。
		 * @return bool 启用自动尺寸返回 true，否则返回 false
		 */
		bool IsAutoSize() const;
		/**
		 * @brief 设置是否循环播放
		 * @details 开启后，动画播放到最后一帧会自动从首帧继续；关闭后，动画在最后一帧停住并发送
		 * `webpplaycomplete` 通知。
		 * @param bLoop [in] 是否循环播放
		 */
		void SetLoop(bool bLoop = true);
		/**
		 * @brief 查询是否循环播放
		 * @details 返回当前 WebP 动画的循环播放开关状态。
		 * @return bool 循环播放开启返回 true，否则返回 false
		 */
		bool IsLoop() const;
		/**
		 * @brief 设置隐藏时的播放策略
		 * @details `continue` 表示即使控件不可见也继续推进时间轴，`pause` 表示隐藏时暂停并在重新显示后继续，
		 * `stop` 表示隐藏时停止并回到首帧。
		 * @param action [in] 隐藏策略
		 */
		void SetHideAction(WebpHideAction action);
		/**
		 * @brief 查询隐藏时的播放策略
		 * @details 返回当前控件在不可见时采用的动画处理方式。
		 * @return WebpHideAction 当前隐藏策略
		 */
		WebpHideAction GetHideAction() const;

		/**
		 * @brief 开始播放 WebP 动画
		 * @details 若当前资源包含多帧并且控件已挂接到管理器，则启动定时器按帧延迟推进动画。
		 */
		void PlayWebp();
		/**
		 * @brief 暂停播放 WebP 动画
		 * @details 停止计时器但保留当前帧位置，适合临时暂停后继续播放。
		 */
		void PauseWebp();
		/**
		 * @brief 停止播放 WebP 动画
		 * @details 停止计时器并把当前帧重置到首帧。
		 */
		void StopWebp();
		/**
		 * @brief 从首帧重新开始播放 WebP 动画
		 * @details 该接口会把当前帧重置到首帧，并按显式播放指令立即启动动画，适合“重播”按钮或播放完成后的再次播放场景。
		 */
		void RestartWebp();
		/**
		 * @brief 跳转到指定帧
		 * @details 可用于调试动画帧、实现进度条拖动或外部控制播放位置。若当前正在播放，会自动按目标帧的延迟重新挂定时器。
		 * @param nFramePosition [in] 目标帧下标，从 0 开始
		 * @param bInvalidate [in] 是否立即触发重绘
		 * @return bool 跳转成功返回 true，下标越界或尚未加载成功时返回 false
		 */
		bool SeekToWebpFrame(UINT nFramePosition, bool bInvalidate = true);
		/**
		 * @brief 获取当前动画总帧数
		 * @details 若控件尚未成功解码 WebP，则返回 0。
		 * @return UINT 当前动画总帧数
		 */
		UINT GetWebpFrameCount() const;
		/**
		 * @brief 获取当前帧下标
		 * @details 返回当前正在显示或即将显示的帧位置，下标从 0 开始。
		 * @return UINT 当前帧下标
		 */
		UINT GetCurrentWebpFrameIndex() const;
		/**
		 * @brief 获取当前播放状态
		 * @details 返回停止、播放中、暂停中或已播放完成四种状态之一，便于业务层更新按钮、状态栏或调试面板。
		 * @return WebpPlaybackState 当前播放状态
		 */
		WebpPlaybackState GetWebpPlaybackState() const;
		/**
		 * @brief 判断当前是否正在播放
		 * @details 当内部计时器处于活动状态时返回 true。
		 * @return bool 正在播放返回 true，否则返回 false
		 */
		bool IsWebpPlaying() const;

		/**
		 * @brief 克隆当前控件
		 * @details 创建一个新的 `CWebpAnimUI`，并复制自动播放、自动尺寸以及当前资源来源配置。
		 * @return CWebpAnimUI* 克隆成功返回新控件指针，失败返回 nullptr
		 */
		virtual CWebpAnimUI* Clone();
		/**
		 * @brief 复制控件数据
		 * @details 把另一个 `CWebpAnimUI` 的资源来源和播放配置复制到当前控件。
		 * @param pControl [in] 源控件
		 */
		virtual void CopyData(CWebpAnimUI* pControl);

	private:
		struct WebpFrame
		{
			HBITMAP hBitmap = nullptr;
			UINT delayMs = 100;
			bool hasAlpha = false;
		};

		bool HasWebpFrames() const;
		bool HasPlayableWebpFrames() const;
		bool HasActiveWebpPlaybackState() const;
		bool IsWebpPlaybackVisible() const;
		bool CanRunWebpPlayback() const;
		bool ShouldAutoPlayWebp();
		bool EnsureWebpLoaded();
		bool ReloadWebpImage();
		bool DecodeWebpBytes(const BYTE* pData, DWORD dwSize);
		bool UpdateWebpSource(std::wstring_view pStrImage, bool bInvalidate);
		UINT GetFrameDelayMs(UINT nFramePosition) const;
		void ReleaseWebpFrames();
		void DeleteWebp();
		void KillWebpTimer();
		void StopWebpPlayback(bool bResetFrame, bool bInvalidate);
		void ResetWebpPlaybackState();
		void SyncWebpPlaybackState();
		void ScheduleFrameTimer(UINT nFramePosition);
		bool AdvanceWebpFrame();
		void OnTimer(UINT_PTR idEvent);
		void NotifyWebpPlaybackComplete();
		void DrawFrame(CPaintRenderContext& renderContext);

	private:
		std::wstring m_sBkImage;
		std::vector<BYTE> m_webpData;
		std::vector<WebpFrame> m_frames;
		SIZE m_szCanvas = { 0, 0 };
		UINT m_nFramePosition = 0;
		bool m_bUseCustomData = false;
		bool m_bIsAutoPlay = true;
		bool m_bIsAutoSize = false;
		bool m_bLoop = true;
		bool m_bIsPlaying = false;
		bool m_bResumePendingOnVisible = false;
		WebpHideAction m_hideAction = WebpHideAction_Pause;
		WebpPlaybackState m_playbackState = WebpPlaybackState_Stopped;
	};
}
