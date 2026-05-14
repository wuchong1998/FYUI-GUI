#pragma once

#include "../Core/Render/UIRenderSurface.h"

namespace FYUI
{
	class FYUI_API CGifAnimUI : public CControlUI
	{
		enum
		{ 
			EVENT_TIEM_ID = 100,
		};
		DECLARE_DUICONTROL(CGifAnimUI)
	private:
		struct GifFrame
		{
			HBITMAP hBitmap = nullptr;
			UINT delayMs = 100;
			bool hasAlpha = false;
		};
	public:
		/**
		 * @brief 执行 CGifAnimUI 操作
		 * @details 用于执行 CGifAnimUI 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CGifAnimUI(void);
		/**
		 * @brief 执行 ~CGifAnimUI 操作
		 * @details 用于执行 ~CGifAnimUI 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CGifAnimUI(void);

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view	GetClass() const;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID	GetInterface(std::wstring_view pstrName);
		/**
		 * @brief 执行初始化逻辑
		 * @details 用于执行初始化逻辑。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	DoInit();
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		void	DoEvent(TEventUI& event);
		/**
		 * @brief 设置可见状态
		 * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 */
		void	SetVisible(bool bVisible = true );
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void	SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 设置背景图像
		 * @details 用于设置背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void	SetBkImage(std::wstring_view pStrImage);
		/**
		 * @brief 获取背景图像
		 * @details 用于获取背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetBkImage();

		/**
		 * @brief 设置AutoPlay
		 * @details 用于设置AutoPlay。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bIsAuto [in] 是否判断是否Auto
		 */
		void	SetAutoPlay(bool bIsAuto = true );
		/**
		 * @brief 判断是否AutoPlay
		 * @details 用于判断是否AutoPlay。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	IsAutoPlay() const;
		/**
		 * @brief 设置Auto尺寸
		 * @details 用于设置Auto尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bIsAuto [in] 是否判断是否Auto
		 */
		void	SetAutoSize(bool bIsAuto = true );
		/**
		 * @brief 判断是否Auto尺寸
		 * @details 用于判断是否Auto尺寸。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	IsAutoSize() const;
		/**
		 * @brief 执行 PlayGif 操作
		 * @details 用于执行 PlayGif 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	PlayGif();
		/**
		 * @brief 执行 PauseGif 操作
		 * @details 用于执行 PauseGif 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	PauseGif();
		/**
		 * @brief 执行 StopGif 操作
		 * @details 用于执行 StopGif 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	StopGif();

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CGifAnimUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CGifAnimUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CGifAnimUI* pControl) ;


	private:
		/**
		 * @brief 确保GIF 动画图像Loaded
		 * @details 用于确保GIF 动画图像Loaded。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	EnsureGifImageLoaded();
		/**
		 * @brief 判断是否具有GIF 动画图像
		 * @details 用于判断是否具有GIF 动画图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	HasGifImage() const;
		/**
		 * @brief 判断是否具有PlayableGIF 动画Frames
		 * @details 用于判断是否具有PlayableGIF 动画Frames。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	HasPlayableGifFrames() const;
		/**
		 * @brief 执行 ShouldAutoPlayGif 操作
		 * @details 用于执行 ShouldAutoPlayGif 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	ShouldAutoPlayGif();
		/**
		 * @brief 检查是否可以RunGIF 动画Playback
		 * @details 用于检查是否可以RunGIF 动画Playback。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	CanRunGifPlayback() const;
		/**
		 * @brief 判断是否具有ActiveGIF 动画Playback状态
		 * @details 用于判断是否具有ActiveGIF 动画Playback状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	HasActiveGifPlaybackState() const;
		/**
		 * @brief 获取帧延迟Ms
		 * @details 用于获取帧延迟Ms。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nFramePosition [in] 帧Position数值
		 * @return UINT 返回对应的数值结果
		 */
		UINT	GetFrameDelayMs(UINT nFramePosition) const;
		/**
		 * @brief 执行 KillGifTimer 操作
		 * @details 用于执行 KillGifTimer 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	KillGifTimer();
		/**
		 * @brief 执行 StopGifPlayback 操作
		 * @details 用于执行 StopGifPlayback 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bResetFrame [in] 是否重置帧
		 * @param bInvalidate [in] 是否Invalidate
		 */
		void	StopGifPlayback(bool bResetFrame, bool bInvalidate);
		/**
		 * @brief 重置GIF 动画Playback状态
		 * @details 用于重置GIF 动画Playback状态。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	ResetGifPlaybackState();
		/**
		 * @brief 执行 SyncGifPlaybackState 操作
		 * @details 用于执行 SyncGifPlaybackState 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	SyncGifPlaybackState();
		/**
		 * @brief 执行 AttachGifImageInfo 操作
		 * @details 用于执行 AttachGifImageInfo 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pImageInfo [in] 图像信息对象
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		void	ReleaseGifFrames();
		/**
		 * @brief 更新GIF 动画来源
		 * @details 用于更新GIF 动画来源。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 * @param bInvalidate [in] 是否Invalidate
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	UpdateGifSource(std::wstring_view pStrImage, bool bInvalidate);
		/**
		 * @brief 更新GIF 动画来源
		 * @details 用于更新GIF 动画来源。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 * @param bInvalidate [in] 是否Invalidate
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool	UpdateGifSource(const std::wstring& pStrImage, bool bInvalidate) { return UpdateGifSource(std::wstring_view(pStrImage), bInvalidate); }
		/**
		 * @brief 执行 ScheduleFrameTimer 操作
		 * @details 用于执行 ScheduleFrameTimer 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param nFramePosition [in] 帧Position数值
		 */
		void	ScheduleFrameTimer(UINT nFramePosition);
		/**
		 * @brief 执行 AdvanceGifFrame 操作
		 * @details 用于执行 AdvanceGifFrame 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	AdvanceGifFrame();
		/**
		 * @brief 重新加载GIF 动画图像
		 * @details 用于重新加载GIF 动画图像。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	ReloadGifImage();
		/**
		 * @brief 执行 InitGifImage 操作
		 * @details 用于执行 InitGifImage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	InitGifImage();
		/**
		 * @brief 删除GIF 动画
		 * @details 用于删除GIF 动画。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void	DeleteGif();
		/**
		 * @brief 执行 OnTimer 操作
		 * @details 用于执行 OnTimer 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param idEvent [in] id事件参数
		 */
		void    OnTimer(UINT_PTR idEvent);
		/**
		 * @brief 绘制当前帧
		 * @details 把 m_frames[m_nFramePosition].hBitmap 作为 D2D 渲染管线的位图源贴到控件矩形。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void	DrawFrame(CPaintRenderContext& renderContext);

	private:
		std::wstring				m_sBkImage;
		std::vector<GifFrame>		m_frames;
		SIZE						m_szCanvas;
		UINT						m_nFramePosition;
		bool						m_bIsAutoPlay;
		bool						m_bIsAutoSize;
		bool						m_bIsPlaying;
	};
}
