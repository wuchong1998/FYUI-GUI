#pragma once

namespace FYUI
{
    class FYUI_API CCountdownUI : public CLabelUI, public CUIAnimation 
    {
        DECLARE_DUICONTROL(CCountdownUI)
    public:
        CCountdownUI();
        ~CCountdownUI();
        void Start();
        void Stop();
        virtual std::wstring_view GetClass() const;
        virtual LPVOID GetInterface(std::wstring_view pstrName);
        void PaintBkImage(CPaintRenderContext& renderContext) override;
        virtual void SetVisible(bool bVisible = true);
        void DoEvent(TEventUI& event);
        void OnTimer(int nTimerID);
        virtual void OnAnimationStart(INT nAnimationID, BOOL bFirstLoop) {}
        virtual void OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID);
        virtual void OnAnimationStop(INT nAnimationID) {
            m_pManager->SendNotify(this, _T("animationstop"));
        }

        void SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue);
        void SetImgDir(std::wstring_view pstrName);
        void SetFrameEllapse(int ellapse) { frame_ellapse_ = ellapse; }
        void SetFrameCount(int count) { frame_count_ = count; }
        void SetFrameID(int ID) { frame_id_ = ID; }
        void SetLoop(BOOL loop) { loop_ = loop; }

        virtual CCountdownUI* Clone();
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

