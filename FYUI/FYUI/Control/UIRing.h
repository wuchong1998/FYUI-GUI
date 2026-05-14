#pragma once

namespace FYUI
{
	class FYUI_API CRingUI : public CLabelUI
	{
		enum
		{ 
			RING_TIMERID = 100,
		};
		DECLARE_DUICONTROL(CRingUI)
	public:
		/**
		 * @brief 执行 CRingUI 操作
		 * @details 用于执行 CRingUI 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CRingUI();
		/**
		 * @brief 执行 ~CRingUI 操作
		 * @details 用于执行 ~CRingUI 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		~CRingUI();

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
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		/**
		 * @brief 设置背景图像
		 * @details 用于设置背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pStrImage [in] 图像描述字符串
		 */
		void SetBkImage(std::wstring_view pStrImage);	
		/**
		 * @brief 处理事件
		 * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
		 * @param event [in,out] 事件对象
		 */
		virtual void DoEvent(TEventUI& event);
		/**
		 * @brief 绘制背景图像
		 * @details 用于绘制背景图像。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintBkImage(CPaintRenderContext& renderContext) override;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CRingUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CRingUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CRingUI* pControl) ;

		// ---- 播放控制 API ----
		/**
		 * @brief 开始旋转
		 * @details 若图像已加载则立即启动定时器；否则等下次绘制完成图像加载后自动启动。
		 */
		void Play();
		/**
		 * @brief 暂停旋转
		 * @details 保留当前角度，停止定时器。
		 */
		void Pause();
		/**
		 * @brief 停止旋转
		 * @details 停止定时器并将角度归零。
		 */
		void Stop();
		/**
		 * @brief 是否正在旋转
		 */
		bool IsPlaying() const;

		/**
		 * @brief 设置自动开始属性
		 * @details true 时立即开始播放，false 时立即暂停。
		 */
		void SetAutoPlay(bool bAutoPlay);
		bool IsAutoPlay() const;

		/**
		 * @brief 设置旋转速度
		 * @param fSpeed 度/秒，负值反向旋转
		 */
		void SetRotateSpeed(float fSpeed);
		float GetRotateSpeed() const;

		/**
		 * @brief 设置定时器刷新间隔（毫秒）
		 * @details 越小越流畅，但 CPU 占用越高。最小 1ms。
		 */
		void SetRotateInterval(int nIntervalMs);
		int GetRotateInterval() const;

		/**
		 * @brief 设置/获取当前角度（0~360）
		 */
		void SetAngle(float fAngle);
		float GetAngle() const;

	private:
		/**
		 * @brief 执行 InitImage 操作
		 * @details 用于执行 InitImage 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void InitImage();
		/**
		 * @brief 删除图像
		 * @details 用于删除图像。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void DeleteImage();
		/**
		 * @brief 按需启动 / 重启定时器
		 * @details 仅当 m_bPlaying && 图像已加载 && m_pManager 已挂上时才会启动。
		 */
		void StartTimerIfNeeded();
		/**
		 * @brief 停止定时器（保留状态）
		 */
		void StopTimer();

	public:
		float m_fCurAngle;
		TImageInfo* m_pBkimageInfo;

	private:
		bool  m_bAutoPlay;       // XML autoplay 属性
		bool  m_bPlaying;        // 当前是否处于播放状态
		bool  m_bTimerStarted;   // 是否已通过 m_pManager->SetTimer 启动
		float m_fRotateSpeed;    // 度/秒
		int   m_nIntervalMs;     // 定时器间隔毫秒
	};
}
