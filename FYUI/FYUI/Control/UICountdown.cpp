#include "pch.h"
#include "UICountdown.h"
#include "../Core/UIRenderContext.h"

namespace FYUI
{
    IMPLEMENT_DUICONTROL(CCountdownUI)

        CCountdownUI::CCountdownUI() : CUIAnimation(&(*this)) {
        m_bAutoStart=false;
        m_bRuning=false;
    }

    CCountdownUI::~CCountdownUI() {
        StopAnimation(frame_id_);
    }

    void CCountdownUI::Start() {
        if (!m_bVisible)return;
        m_bRuning = true;
        StartAnimation(frame_ellapse_, frame_count_, frame_id_, loop_);
        Invalidate();
    }

    void CCountdownUI::Stop()
    {
        m_bRuning = false;
        StopAnimation(frame_id_);
    }

    std::wstring_view CCountdownUI::GetClass() const {
        return _T("CountdownUI");
    }

    LPVOID CCountdownUI::GetInterface(std::wstring_view pstrName) {
        if (StringUtil::CompareNoCase(pstrName, _T("Countdown")) == 0)
            return static_cast<CCountdownUI*>(this);
        return CControlUI::GetInterface(pstrName);
    }

    void CCountdownUI::PaintBkImage(CPaintRenderContext& renderContext) {
        if (m_pManager == nullptr || bk_img_.empty()) {
            CLabelUI::PaintBkImage(renderContext);
            return;
        }

        auto rc = GetPos();
        TDrawInfo* pDrawInfo = m_pManager->GetDrawInfo(bk_img_, NULL);
        if (pDrawInfo == nullptr) {
            CLabelUI::PaintBkImage(renderContext);
            return;
        }
        CRenderEngine::DrawImageInfo(renderContext, rc, pDrawInfo, NULL);
    }

    void CCountdownUI::DoEvent(TEventUI& event) {
        if (event.Type == UIEVENT_TIMER) {
            OnTimer(static_cast<int>(event.wParam));
        }
        CLabelUI::DoEvent(event);
    }

    void CCountdownUI::OnTimer(int nTimerID) {
        OnAnimationElapse(nTimerID);
    }

    void CCountdownUI::OnAnimationStep(INT nTotalFrame, INT nCurFrame, INT nAnimationID) {
        std::wstring num;
        num = StringUtil::Format(L"{}/{}.png", img_dir_, nCurFrame);
        bk_img_ = num;
        Invalidate();
    }

    void CCountdownUI::SetVisible(bool bVisible)
    {
        if (m_bVisible == bVisible) return;

        bool v = IsVisible();
        m_bVisible = bVisible;
        if (m_bFocused) m_bFocused = false;
        if (!bVisible && m_pManager && m_pManager->GetFocus() == this) {
            m_pManager->SetFocus(NULL);
        }
        if (IsVisible() != v) {
            NeedParentUpdate();
        }
        /* if (bVisible&& m_bAutoStart&& !m_bRuning)
        {
        Start();
        }

        if (!bVisible)
        {
        if (m_bAutoStart&& m_bRuning)
        {
        Stop();
        }
        }*/
    }

    void CCountdownUI::SetAttribute(std::wstring_view pstrName, std::wstring_view pstrValue)
    {
        if (StringUtil::CompareNoCase(pstrName, _T("imgdir")) == 0) SetImgDir(pstrValue);
        else if (StringUtil::CompareNoCase(pstrName, _T("ellapse")) == 0) {
            int val = 0;
            if (StringUtil::TryParseInt(pstrValue, val)) SetFrameEllapse(val);
        }
        else if (StringUtil::CompareNoCase(pstrName, _T("framecount")) == 0) {
            int val = 0;
            if (StringUtil::TryParseInt(pstrValue, val)) SetFrameCount(val);
        }
        else if (StringUtil::CompareNoCase(pstrName, _T("frameid")) == 0) {
            int val = 0;
            if (StringUtil::TryParseInt(pstrValue, val)) SetFrameID(val);
        }
        else if (StringUtil::CompareNoCase(pstrName, _T("loop")) == 0) {
            SetLoop(StringUtil::ParseBool(pstrValue));
        }
        else if (StringUtil::CompareNoCase(pstrName, _T("autostart")) == 0) {
            m_bAutoStart = true;
            Start();
        }
        else CControlUI::SetAttribute(pstrName, pstrValue);
    }

    void CCountdownUI::SetImgDir(std::wstring_view pstrName)
    {
        img_dir_ = pstrName;
    }

    CCountdownUI* CCountdownUI::Clone()
    {
        CCountdownUI* pClone = new CCountdownUI();
        pClone->CopyData(this);
        return pClone;
    }

    void CCountdownUI::CopyData(CCountdownUI* pControl)
    {
        bk_img_ = pControl->bk_img_;
        img_dir_ = pControl->img_dir_;
        frame_ellapse_ = pControl->frame_ellapse_;
        frame_count_ = pControl->frame_count_;
        frame_id_ = pControl->frame_id_;
        loop_ = pControl->loop_;
        m_bAutoStart = pControl->m_bAutoStart;
        m_bRuning = pControl->m_bRuning;
        CLabelUI::CopyData(pControl);
    }

}


