#include "framework.h"
#include "FYTest.h"

#include <memory>
#include <string>
#include <vector>

#include "MainDemoWindow.h"

namespace
{
    constexpr RECT kMainCreateRect = { 0, 0, 1180, 760 };
    constexpr DWORD kCreateHiddenFrameStyle = UI_WNDSTYLE_FRAME & ~WS_VISIBLE;

    std::wstring TrimTrailingSlash(std::wstring path)
    {
        while (!path.empty() && (path.back() == L'\\' || path.back() == L'/')) {
            path.pop_back();
        }
        return path;
    }

    std::wstring GetDirectoryFromFilePath(const std::wstring& filePath)
    {
        const size_t pos = filePath.find_last_of(L"\\/");
        if (pos == std::wstring::npos) {
            return L".";
        }
        return filePath.substr(0, pos);
    }

    std::wstring GetModuleDirectory(HINSTANCE hInstance)
    {
        wchar_t modulePath[MAX_PATH] = {};
        ::GetModuleFileNameW(hInstance, modulePath, static_cast<DWORD>(std::size(modulePath)));
        return GetDirectoryFromFilePath(modulePath);
    }

    std::wstring GetCurrentDirectoryString()
    {
        wchar_t currentPath[MAX_PATH] = {};
        ::GetCurrentDirectoryW(static_cast<DWORD>(std::size(currentPath)), currentPath);
        return currentPath;
    }

    bool IsDirectory(const std::wstring& path)
    {
        const DWORD attributes = ::GetFileAttributesW(path.c_str());
        return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    std::wstring ResolveFullPath(const std::wstring& path)
    {
        wchar_t resolved[MAX_PATH] = {};
        const DWORD length = ::GetFullPathNameW(path.c_str(), static_cast<DWORD>(std::size(resolved)), resolved, nullptr);
        if (length == 0 || length >= std::size(resolved)) {
            return path;
        }
        return resolved;
    }

    std::wstring ResolveResourceDirectory(HINSTANCE hInstance)
    {
        const std::wstring moduleDir = GetModuleDirectory(hInstance);
        const std::wstring currentDir = GetCurrentDirectoryString();
        const std::vector<std::wstring> candidates = {
            currentDir + L"\\res",
            currentDir + L"\\FYTest\\res",
            moduleDir + L"\\res",
            moduleDir + L"\\..\\..\\FYTest\\res",
            L"D:\\work\\FYUI\\FYTest\\FYTest\\FYTest\\res",
        };

        for (const std::wstring& candidate : candidates) {
            std::wstring resolved = TrimTrailingSlash(ResolveFullPath(candidate));
            if (IsDirectory(resolved)) {
                return resolved + L"\\";
            }
        }
        return L"res\\";
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    FYUI::CPaintManagerUI::SetInstance(hInstance);
    FYUI::CPaintManagerUI::SetResourceType(FYUI::UILIB_FILE);
    FYUI::CPaintManagerUI::SetResourcePath(ResolveResourceDirectory(hInstance));

    const HRESULT oleResult = ::OleInitialize(nullptr);

    auto mainWindow = std::make_unique<FYTestApp::MainDemoWindow>();
    HWND hWnd = mainWindow->Create(nullptr, L"FYUI Demo Lab", kCreateHiddenFrameStyle, 0, kMainCreateRect);
    if (hWnd == nullptr) {
        if (SUCCEEDED(oleResult)) {
            ::OleUninitialize();
        }
        return FALSE;
    }

    mainWindow->CenterWindow();
    mainWindow->ShowWindow(true, true);
    ::UpdateWindow(hWnd);
    ::SetForegroundWindow(hWnd);
    FYUI::CPaintManagerUI::MessageLoop();
    mainWindow.reset();
    FYUI::CPaintManagerUI::Term();

    if (SUCCEEDED(oleResult)) {
        ::OleUninitialize();
    }
    return 0;
}
