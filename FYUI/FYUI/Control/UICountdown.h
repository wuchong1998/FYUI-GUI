#pragma once

namespace FYUI
{
    class FYUI_API CCountdownUI : public CLabelUI, public CUIAnimation 
    {
        DECLARE_DUICONTROL(CCountdownUI)
    public:
        /**
         * @brief 构造 CCountdownUI 对象
         * @details 用于构造 CCountdownUI 对象。具体行为由当前对象状态以及传入参数共同决定。
         */
        CCountdownUI();
        /**
         * @brief 析构 CCountdownUI 对象
         * @details 用于析构 CCountdownUI 对象。具体行为由当前对象状态以及传入参数共同决定。
         */
        ~CCountdownUI();
        /**
         * @brief 执行 Start 操作
         * @details 用于执行 Start 操作。具体行为由当前对象状态以及传入参数共同决定。
         */
        void Start();
        /**
         * @brief 执行 Stop 操作
         * @details 用于执行 Stop 操作。具体行为由当前对象状态以及传入参数共同决定。
         */
        void Stop();
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
         * @brief 绘制背景图像
         * @details 用于绘制背景图像。具体行为由当前对象状态以及传入参数共同决定。
         * @param renderContext [in,out] 绘制上下文
         */
        void PaintBkImage(CPaintRenderContext& renderContext) override;
        /**
         * @brief 设置可见状态
         * @details 用于设置可见状态。具体行为由当前对象状态以及传入参数共同决定。
         * @param bVisible [in] 是否可见状态
         */
        virtual void SetVisible(bool bVisible = true);
        /**
         * @brief 处理事件
         * @details 用于处理事件。具体行为由当前对象状态以及传入参数共同决定。
         * @param event [in,out] 事件对象
         */
        void DoEvent(TEventUI& event);
        /**
         * @brief 执行 OnTimer 操作
         * @details 用于执行 OnTimer 操作。具体行为由当前对象状态以及传入参数共同决定。
         * @param nTimerID [in] 定时器标识
         */
        void OnTimer(int nTimerID);
        /**
         * @brief 执行 OnAnimationStart 操作
         * @details 用于执行 OnAnimationStart 操作。具体行为由当前对象状态以及传入参数共同决定。
         * @param nAnimationID [in] 动画ID数值
         * @param bFirstLoop [in] 是否First循环
         */
        virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
        virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
        /**
         * @brief 执行 OnAnimationStop 操作
         * @details 用于执行 OnAnimationStop 操作。具体行为由当前对象状态以及传入参数共同决定。
         * @param nAnimationID [in] 动画ID数值
         */
        virtual void OnAnimationStop(INT nAnimationID) {
            m_pManager->SendNotify(this, _T("animationstop"));
        }

        /**
         * @brief 设置属性
         * @details 用于设置属性。具体行为由当前对象状态以及传入参数共同决定。
         * @param pstrName [in] 属性名称
         * @param pstrValue [in] 属性值
         */
        void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
        /**
         * @brief 设置ImgDir
         * @details 用于设置ImgDir。具体行为由当前对象状态以及传入参数共同决定。
         * @param pstrName [in] 属性名称
         */
        void SetImgDir(std::wstring_view pstrName);
        /**
         * @brief 设置帧Ellapse
         * @details 用于设置帧Ellapse。具体行为由当前对象状态以及传入参数共同决定。
         * @param ellapse [in] ellapse参数
         */
        void SetFrameEllapse(int ellapse) { frame_ellapse_ = ellapse; }
        void SetFrameCount(int count) { frame_count_ = count; }
        void SetFrameID(int ID) { frame_id_ = ID; }
        void SetLoop(BOOL loop) { loop_ = loop; }

        virtual CCountdownUI* Clone();
        /**
         * @brief 复制对象数据
         * @details 用于复制对象数据。具体行为由当前对象状态以及传入参数共同决定。
         * @param pControl [in] 控件对象
         */
        virtual void CopyData(CCountdownUI* pControl) ;



    private:
        std::wstring bk_img_;
        std::wstring img_dir_;
        int frame_ellapse_;
        int frame_count_;
        int frame_id_ = 1;
        BOOL loop_ = TRUE;
        bool m_bAutoStart;
        bool m_bRuning;
    };

}

