#pragma once

namespace FYUI
{
	class FYUI_API CFrameTestUI : public CLabelUI
	{
		DECLARE_DUICONTROL(CFrameTestUI)

	public:
		/**
		 * @brief 构造 CFrameTestUI 对象
		 * @details 用于构造 CFrameTestUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CFrameTestUI();

		/**
		 * @brief 获取类名
		 * @details 用于获取类名。具体行为由当前对象状态以及传入参数共同决定。
		 * @return 返回对应的字符串内容
		 */
		std::wstring_view GetClass() const override;
		/**
		 * @brief 获取指定接口
		 * @details 用于获取指定接口。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @return LPVOID 返回 获取指定接口 的结果
		 */
		LPVOID GetInterface(std::wstring_view pstrName) override;
		/**
		 * @brief 设置可见状态
		 * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 */
		void SetVisible(bool bVisible = true);
		/**
		 * @brief 执行绘制
		 * @details 用于执行绘制。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 * @param pStopControl [in] 绘制终止控件
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool DoPaint(CPaintRenderContext& renderContext, CControlUI* pStopControl) override;
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue) override;

		/**
		 * @brief 设置Running
		 * @details 用于设置Running。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bRunning [in] 是否Running
		 */
		void SetRunning(bool bRunning);
		/**
		 * @brief 判断是否Running
		 * @details 用于判断是否Running。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsRunning() const;
		/**
		 * @brief 设置Stats可见状态
		 * @details 用于设置Stats可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bVisible [in] 是否可见状态
		 */
		void SetStatsVisible(bool bVisible);
		/**
		 * @brief 判断是否Stats可见状态
		 * @details 用于判断是否Stats可见状态。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsStatsVisible() const;
		/**
		 * @brief 获取当前FPS
		 * @details 用于获取当前FPS。具体行为由当前对象状态以及传入参数共同决定。
		 * @return double 返回对应的数值结果
		 */
		double GetCurrentFPS() const;
		/**
		 * @brief 获取AverageFPS
		 * @details 用于获取AverageFPS。具体行为由当前对象状态以及传入参数共同决定。
		 * @return double 返回对应的数值结果
		 */
		double GetAverageFPS() const;
		/**
		 * @brief 获取TotalFrames
		 * @details 用于获取TotalFrames。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT64 返回 获取TotalFrames 的结果
		 */
		UINT64 GetTotalFrames() const;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CFrameTestUI* 返回结果对象指针，失败时返回 nullptr
		 */
		CFrameTestUI* Clone() override;
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		void CopyData(CFrameTestUI* pControl);

	private:
		/**
		 * @brief 重置帧Stats
		 * @details 用于重置帧Stats。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void ResetFrameStats();
		/**
		 * @brief 更新帧Stats
		 * @details 用于更新帧Stats。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void UpdateFrameStats();
		/**
		 * @brief 绘制动画
		 * @details 用于绘制动画。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintAnimation(CPaintRenderContext& renderContext);
		/**
		 * @brief 绘制Stats
		 * @details 用于绘制Stats。具体行为由当前对象状态以及传入参数共同决定。
		 * @param renderContext [in,out] 绘制上下文
		 */
		void PaintStats(CPaintRenderContext& renderContext);
		/**
		 * @brief 执行 RequestNextFrame 操作
		 * @details 用于执行 RequestNextFrame 操作。具体行为由当前对象状态以及传入参数共同决定。
		 */
		void RequestNextFrame();
		/**
		 * @brief 执行 QpcToMilliseconds 操作
		 * @details 用于执行 QpcToMilliseconds 操作。具体行为由当前对象状态以及传入参数共同决定。
		 * @param qpcDelta [in] qpcDelta参数
		 * @return double 返回对应的数值结果
		 */
		double QpcToMilliseconds(LONGLONG qpcDelta) const;

	private:
		bool m_bRunning = true;
		bool m_bShowStats = true;
		LONGLONG m_nQpcFrequency = 0;
		LONGLONG m_nStartQpc = 0;
		LONGLONG m_nLastFrameQpc = 0;
		LONGLONG m_nSampleStartQpc = 0;
		UINT64 m_nTotalFrames = 0;
		UINT64 m_nSampleFrames = 0;
		double m_nCurrentFPS = 0.0;
		double m_nAverageFPS = 0.0;
		double m_nLastFrameMs = 0.0;
	};
}
