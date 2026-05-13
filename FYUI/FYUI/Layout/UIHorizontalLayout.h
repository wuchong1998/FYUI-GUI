#pragma once


namespace FYUI
{
	class FYUI_API CHorizontalLayoutUI : public CContainerUI
	{
		DECLARE_DUICONTROL(CHorizontalLayoutUI)
	public:
		/**
		 * @brief 构造 CHorizontalLayoutUI 对象
		 * @details 用于构造 CHorizontalLayoutUI 对象。具体行为由当前对象状态以及传入参数共同决定。
		 */
		CHorizontalLayoutUI();

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
		 * @brief 获取控件标志
		 * @details 用于获取控件标志。具体行为由当前对象状态以及传入参数共同决定。
		 * @return UINT 返回对应的数值结果
		 */
		UINT GetControlFlags() const;

		/**
		 * @brief 设置分隔宽度
		 * @details 用于设置分隔宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @param iWidth [in] 宽度值
		 */
		void SetSepWidth(int iWidth);
		/**
		 * @brief 获取分隔宽度
		 * @details 用于获取分隔宽度。具体行为由当前对象状态以及传入参数共同决定。
		 * @return int 返回对应的数值结果
		 */
		int GetSepWidth() const;
		/**
		 * @brief 设置分隔立即Mode
		 * @details 用于设置分隔立即Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @param bImmediately [in] 是否Immediately
		 */
		void SetSepImmMode(bool bImmediately);
		/**
		 * @brief 判断是否分隔立即Mode
		 * @details 用于判断是否分隔立即Mode。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsSepImmMode() const;
		/**
		 * @brief 设置属性
		 * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pstrName [in] 属性名称
		 * @param pstrValue [in] 属性值
		 */
		void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
		void DoEvent(TEventUI& event);
		void SetPos(RECT rc, bool bNeedInvalidate = true);
		void DoPostPaint(CPaintRenderContext& renderContext) override;
		RECT GetThumbRect(bool bUseNew = false) const;

		// ============================================================
		// 显隐动画
		// ============================================================
		enum ShowHideAnimDir {
			AnimNone = 0,   // 无动画
			AnimLeft,       // 从左侧收起/展开
			AnimRight,      // 从右侧收起/展开
		};

		/**
		 * @brief 设置显隐动画方向
		 * @param dir [in] 动画方向
		 */
		void SetShowHideAnimDir(ShowHideAnimDir dir);
		ShowHideAnimDir GetShowHideAnimDir() const;

		/**
		 * @brief 设置可见状态（支持动画过渡）
		 * @details 当 animation_show_hide 属性不为空时，SetVisible 会启动匀速动画过渡。
		 */
		void SetVisible(bool bVisible = true, bool bSendFocus = true);

		/** @brief 显隐动画是否正在进行 */
		bool IsShowHideAnimating() const;

		/**
		 * @brief 克隆当前对象
		 * @details 用于克隆当前对象。具体行为由当前对象状态以及传入参数共同决定。
		 * @return CHorizontalLayoutUI* 返回结果对象指针，失败时返回 nullptr
		 */
		virtual CHorizontalLayoutUI* Clone();
		/**
		 * @brief 复制对象数据
		 * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
		 * @param pControl [in] 控件对象
		 */
		virtual void CopyData(CHorizontalLayoutUI* pControl) ;

		/**
		 * @brief 判断是否Container控件
		 * @details 用于判断是否Container控件。具体行为由当前对象状态以及传入参数共同决定。
		 * @return bool 操作成功返回 true，否则返回 false
		 */
		bool IsContainerControl() const override
		{
			return true;
		}

	protected:
		int m_iSepWidth;
		UINT m_uButtonState;
		POINT ptLastMouse;
		RECT m_rcNewPos;
		bool m_bImmMode;

		// 显隐动画
		ShowHideAnimDir m_animDir = AnimNone;
		bool   m_bAnimating = false;         // 动画进行中
		bool   m_bAnimShowing = false;       // true=展开, false=收起
		int    m_nAnimTargetPx = 0;          // 目标像素宽度
		int    m_nAnimCurrentPx = 0;         // 当前像素宽度
		int    m_nAnimOrigFixedW = 0;        // 原始 FixedWidth（逻辑值，0=自适应）

		enum : UINT {
			SHOWHIDE_ANIM_TIMERID = 0x5F30,
			SHOWHIDE_ANIM_INTERVAL_MS = 16,  // ~60fps
		};

		void StartShowHideAnim(bool bShow);
		void AdvanceShowHideAnim();
		void StopShowHideAnim();
	};
}

