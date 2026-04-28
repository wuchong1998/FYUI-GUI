#include "framework.h"
#include "DemoPopupWindow.h"

namespace
{
    constexpr wchar_t kPopupSkin[] = L"fytest_popup.xml";

    template <typename T>
    T* FindControlAs(FYUI::CPaintManagerUI& manager, const wchar_t* name)
    {
        return static_cast<T*>(manager.FindControl(name));
    }
}

namespace FYTestApp
{
    DemoPopupWindow::DemoPopupWindow(bool autoDestroy)
        : autoDestroy_(autoDestroy)
    {
    }

    std::wstring DemoPopupWindow::GetSkinFile()
    {
        return kPopupSkin;
    }

    std::wstring_view DemoPopupWindow::GetWindowClassName() const
    {
        return L"FYTestPopupWindow";
    }

    void DemoPopupWindow::InitWindow()
    {
        if (auto* caption = FindControlAs<FYUI::CLabelUI>(m_pm, L"popup_caption")) {
            caption->SetText(L"FYUI child window test surface");
        }
    }

    void DemoPopupWindow::OnClick(FYUI::TNotifyUI& msg)
    {
        if (msg.pSender != nullptr) {
            const std::wstring& name = msg.pSender->GetName();
            if (name == L"popup_ok" || name == L"popup_cancel") {
                Close(name == L"popup_ok" ? IDOK : IDCANCEL);
                return;
            }
        }
        WindowImplBase::OnClick(msg);
    }

    void DemoPopupWindow::OnFinalMessage(HWND hWnd)
    {
        WindowImplBase::OnFinalMessage(hWnd);
        if (autoDestroy_) {
            delete this;
        }
    }
}
