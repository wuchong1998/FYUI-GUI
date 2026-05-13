# FYUI UI 事件处理 Skill

## 一、消息分发总览

```
Win32 消息 (WM_*)
  → CWindowWnd::__WndProc()
    → WindowImplBase::HandleMessage(uMsg, wParam, lParam)
      ├─ OnCreate / OnClose / OnDestroy / OnSize ...  ← 窗口生命周期
      ├─ OnNcHitTest / OnNcCalcSize / OnNcActivate    ← 非客户区（无边框窗口）
      ├─ OnGetMinMaxInfo                               ← 窗口最大最小尺寸
      ├─ OnDpiChange                                   ← DPI 变化
      └─ CPaintManagerUI::MessageHandler(uMsg, wParam, lParam)
           ├─ WM_PAINT        → 布局 + 绘制
           ├─ WM_MOUSEMOVE    → 命中测试 → DoEvent(UIEVENT_MOUSEMOVE)
           ├─ WM_LBUTTONDOWN  → DoEvent(UIEVENT_BUTTONDOWN)
           ├─ WM_LBUTTONUP    → DoEvent(UIEVENT_BUTTONUP) → SendNotify("click")
           ├─ WM_RBUTTONDOWN  → DoEvent(UIEVENT_RBUTTONDOWN)
           ├─ WM_LBUTTONDBLCLK→ DoEvent(UIEVENT_DBLCLICK)
           ├─ WM_MOUSEWHEEL   → DoEvent(UIEVENT_SCROLLWHEEL)
           ├─ WM_KEYDOWN      → DoEvent(UIEVENT_KEYDOWN)
           ├─ WM_CHAR         → DoEvent(UIEVENT_CHAR)
           ├─ WM_TIMER        → DoEvent(UIEVENT_TIMER)
           ├─ WM_SETCURSOR    → DoEvent(UIEVENT_SETCURSOR)
           ├─ WM_SETFOCUS     → DoEvent(UIEVENT_SETFOCUS)
           └─ WM_KILLFOCUS    → DoEvent(UIEVENT_KILLFOCUS)
```

---

## 二、EVENTTYPE_UI 事件类型枚举

| 事件 | 来源 Win32 消息 | 说明 |
|------|----------------|------|
| `UIEVENT_KEYDOWN` | `WM_KEYDOWN` | 键盘按下 |
| `UIEVENT_KEYUP` | `WM_KEYUP` | 键盘抬起 |
| `UIEVENT_CHAR` | `WM_CHAR` | 字符输入 |
| `UIEVENT_SYSKEY` | `WM_SYSKEYDOWN` | 系统按键 |
| `UIEVENT_MOUSEMOVE` | `WM_MOUSEMOVE` | 鼠标移动 |
| `UIEVENT_MOUSELEAVE` | `WM_MOUSELEAVE` | 鼠标离开窗口 |
| `UIEVENT_MOUSEENTER` | 内部检测 | 鼠标进入控件 |
| `UIEVENT_MOUSEHOVER` | `WM_MOUSEHOVER` | 鼠标悬停 |
| `UIEVENT_BUTTONDOWN` | `WM_LBUTTONDOWN` | 左键按下 |
| `UIEVENT_BUTTONUP` | `WM_LBUTTONUP` | 左键抬起 |
| `UIEVENT_RBUTTONDOWN` | `WM_RBUTTONDOWN` | 右键按下 |
| `UIEVENT_RBUTTONUP` | `WM_RBUTTONUP` | 右键抬起 |
| `UIEVENT_DBLCLICK` | `WM_LBUTTONDBLCLK` | 双击 |
| `UIEVENT_CONTEXTMENU` | `WM_CONTEXTMENU` | 上下文菜单 |
| `UIEVENT_SCROLLWHEEL` | `WM_MOUSEWHEEL` | 滚轮 |
| `UIEVENT_KILLFOCUS` | 内部管理 | 失去焦点 |
| `UIEVENT_SETFOCUS` | 内部管理 | 获得焦点 |
| `UIEVENT_SETCURSOR` | `WM_SETCURSOR` | 设置光标 |
| `UIEVENT_TIMER` | `WM_TIMER` | 定时器 |
| `UIEVENT_WINDOWSIZE` | `WM_SIZE` | 窗口尺寸变化 |

---

## 三、DUI 通知消息类型 (DUI_MSGTYPE_*)

| 消息类型 | 触发场景 |
|----------|----------|
| `click` | 按钮/控件点击 |
| `dbclick` | 双击 |
| `menu` | 右键菜单 |
| `timer` | 定时器触发 |
| `setfocus` / `killfocus` | 焦点变化 |
| `mouseenter` / `mouseleave` | 鼠标进出控件 |
| `buttondown` | 鼠标按下 |
| `textchanged` | 文本变化（RichEdit） |
| `valuechanged` / `movevaluechanged` | 值变化（Slider/Progress） |
| `selectchanged` / `unselected` | 选择状态变化（Option） |
| `itemclick` / `itemrclick` / `itemdbclick` | 列表项点击 |
| `itemselect` / `listitemselect` | 列表项选择 |
| `tabselect` | 选项卡切换 |
| `dropdown` / `predropdown` | 下拉框展开 |
| `scroll` / `scrolltools` | 滚动事件 |
| `headerclick` / `listheaderclick` | 列表头点击 |
| `colorchanged` | 调色板颜色变化 |
| `windowinit` | 窗口初始化完成 |
| `windowsize` | 窗口尺寸变化 |
| `return` | 回车键 |
| `link` | 超链接点击 |
| `splitmove` / `splitmoveup` | 分隔条拖动 |
| `webpplaycomplete` | WebP 动画播放完成 |

---

## 四、消息映射宏系统

### 4.1 声明与实现

```cpp
// 在类声明中
class MainDemoWindow : public WindowImplBase {
    DUI_DECLARE_MESSAGE_MAP()        // 声明消息映射表
    void OnClick(TNotifyUI& msg);    // 消息处理函数
};

// 在 cpp 中实现映射表
DUI_BEGIN_MESSAGE_MAP(MainDemoWindow, WindowImplBase)
    DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
    DUI_ON_MSGTYPE(DUI_MSGTYPE_TIMER, OnTimer)
    DUI_ON_MSGTYPE_CTRNAME(DUI_MSGTYPE_CLICK, L"btn_ok", OnOkClick)
DUI_END_MESSAGE_MAP()
```

### 4.2 映射宏说明

| 宏 | 用途 |
|----|------|
| `DUI_ON_MSGTYPE(type, fn)` | 按消息类型路由到处理函数 |
| `DUI_ON_MSGTYPE_CTRNAME(type, name, fn)` | 按消息类型+控件名称路由 |
| `DUI_ON_CLICK_CTRNAME(name, fn)` | 快捷：click+控件名称 |
| `DUI_ON_SELECTCHANGED_CTRNAME(name, fn)` | 快捷：selectchanged+控件名称 |
| `DUI_ON_KILLFOCUS_CTRNAME(name, fn)` | 快捷：killfocus+控件名称 |
| `DUI_ON_MENU_CTRNAME(name, fn)` | 快捷：menu+控件名称 |
| `DUI_ON_TIMER()` | 定时器消息路由到 OnTimer |

### 4.3 分发流程

```
CPaintManagerUI::SendNotify(TNotifyUI& msg)
  → 遍历 INotifyUI 通知器列表
    → WindowImplBase::Notify(msg)
      → CNotifyPump::NotifyPump(msg)
        1. 查找虚拟窗口映射 m_VirtualWndMap
           → 若匹配 sVirtualWnd，转发给对应 CNotifyPump
        2. LoopDispatch(msg)
           → 遍历 DUI_MSGMAP 消息映射表链
             → 匹配 sType 和 sCtrlName
             → 调用绑定的成员函数 pfn(msg)
```

---

## 五、鼠标事件流程

### 5.1 命中测试

```
WM_MOUSEMOVE → CPaintManagerUI::MessageHandler()
  → FindControl(pt)                       ← 在控件树中查找命中控件
    → CContainerUI::FindControl(Proc, pData, UIFIND_HITTEST)
      → 从后向前遍历子控件（Z 序最高优先）
      → 检查可见、启用、命中矩形
      → 返回命中的叶子控件
```

### 5.2 鼠标移动 → Enter/Leave 检测

```
FindControl(pt) 返回 pNewHover

if (pNewHover != m_pEventHover) {
    // 旧控件离开
    m_pEventHover->DoEvent(UIEVENT_MOUSELEAVE)
    SendNotify(m_pEventHover, DUI_MSGTYPE_MOUSELEAVE)
    
    // 新控件进入
    m_pEventHover = pNewHover
    pNewHover->DoEvent(UIEVENT_MOUSEENTER)
    SendNotify(pNewHover, DUI_MSGTYPE_MOUSEENTER)
}
```

### 5.3 点击流程

```
WM_LBUTTONDOWN:
  m_pEventClick = FindControl(pt)
  m_pEventClick->DoEvent(UIEVENT_BUTTONDOWN)
  SetCapture()                            ← 捕获鼠标

WM_LBUTTONUP:
  ReleaseCapture()
  m_pEventClick->DoEvent(UIEVENT_BUTTONUP)
  → 控件内部 DoEvent 判断是否在控件矩形内
    → 若是：Activate() → SendNotify(DUI_MSGTYPE_CLICK)
```

### 5.4 光标管理

```
WM_SETCURSOR → CPaintManagerUI
  → pHover->DoEvent(UIEVENT_SETCURSOR)
    → CControlUI::DoEvent()
      → SetCursor(::LoadCursor(NULL, MAKEINTRESOURCE(GetCursor())))
      
控件可设置光标：
  SetCursor(DUI_HAND)    ← 手形
  SetCursor(DUI_IBEAM)   ← I 型（文本）
  SetCursor(DUI_SIZEWE)  ← 水平拖动
  SetCursor(DUI_SIZENS)  ← 垂直拖动
  SetCursor(DUI_ARROW)   ← 箭头（默认）
```

---

## 六、键盘事件流程

```
WM_KEYDOWN → CPaintManagerUI
  1. 消息过滤器链 (IMessageFilterUI)
     → WindowImplBase::MessageHandler()
       → ResponseDefaultKeyEvent(wParam)
         → VK_RETURN / VK_ESCAPE 等默认处理
  2. 若未处理，发送给焦点控件：
     m_pFocus->DoEvent(UIEVENT_KEYDOWN)
  3. Tab 键特殊处理：
     → SetNextTabControl(bForward)
```

---

## 七、WindowImplBase 消息处理链

```cpp
LRESULT WindowImplBase::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL bHandled = FALSE;
    LRESULT lRes = 0;
    switch (uMsg) {
        case WM_CREATE:      lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
        case WM_CLOSE:       lRes = OnClose(uMsg, wParam, lParam, bHandled); break;
        case WM_DESTROY:     lRes = OnDestroy(uMsg, wParam, lParam, bHandled); break;
        case WM_SIZE:        lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
        case WM_CHAR:        lRes = OnChar(uMsg, wParam, lParam, bHandled); break;
        case WM_SYSCOMMAND:  lRes = OnSysCommand(uMsg, wParam, lParam, bHandled); break;
        case WM_KEYDOWN:     lRes = OnKeyDown(uMsg, wParam, lParam, bHandled); break;
        case WM_KILLFOCUS:   lRes = OnKillFocus(uMsg, wParam, lParam, bHandled); break;
        case WM_SETFOCUS:    lRes = OnSetFocus(uMsg, wParam, lParam, bHandled); break;
        case WM_LBUTTONDOWN: lRes = OnLButtonDown(uMsg, wParam, lParam, bHandled); break;
        case WM_LBUTTONUP:   lRes = OnLButtonUp(uMsg, wParam, lParam, bHandled); break;
        case WM_MOUSEMOVE:   lRes = OnMouseMove(uMsg, wParam, lParam, bHandled); break;
        // 非客户区处理...
        case WM_NCACTIVATE:  lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
        case WM_NCCALCSIZE:  lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
        case WM_NCPAINT:     lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
        case WM_NCHITTEST:   lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
        case WM_GETMINMAXINFO: lRes = OnGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
        case WM_MOUSEWHEEL:  lRes = OnMouseWheel(uMsg, wParam, lParam, bHandled); break;
        case WM_DPICHANGED:  lRes = OnDpiChange(uMsg, wParam, lParam, bHandled); break;
        default:
            lRes = HandleCustomMessage(uMsg, wParam, lParam, bHandled); break;
    }
    if (bHandled) return lRes;
    // 未处理的消息交给 CPaintManagerUI
    if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) return lRes;
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}
```

---

## 八、非客户区处理（无边框窗口）

### 8.1 NcHitTest（拖动和缩放）

```
OnNcHitTest():
  1. 获取鼠标屏幕坐标 → 转换为客户区坐标
  2. 检查 SizeBox 区域（窗口边缘缩放区域）
     → 返回 HTTOPLEFT / HTTOP / HTTOPRIGHT / HTLEFT / HTRIGHT / ...
  3. 检查 CaptionRect 区域（标题栏拖动区域）
     → 排除静态控件（按钮、编辑框等）
     → 返回 HTCAPTION（允许拖动窗口）
  4. 默认返回 HTCLIENT
```

### 8.2 CaptionRect 与 IsInStaticControl

```
CPaintManagerUI::SetCaptionRect(rcCaption)
  → 定义标题栏区域（通常是窗口顶部一条区域）

WindowImplBase::IsInStaticControl(pControl)
  → 检查控件是否为"静态控件"（按钮、编辑框、滑块等）
  → 静态控件不响应 HTCAPTION（不能通过点击它们拖动窗口）
```

---

## 九、FYTest 事件处理示例

### 9.1 Notify 路由

```cpp
void MainDemoWindow::Notify(TNotifyUI& msg) {
    if (msg.sType == DUI_MSGTYPE_VALUECHANGED) 
        UpdateValueStatus(msg.pSender);        // 滑块值变化
    else if (msg.sType == DUI_MSGTYPE_SCROLL)
        UpdateFpsMeterIfDue();                 // 滚动时更新 FPS
    else if (msg.sType == DUI_MSGTYPE_TABSELECT)
        UpdateStatusFromNotify(msg);           // 选项卡切换
    
    WindowImplBase::Notify(msg);               // 传递给消息映射表
}
```

### 9.2 OnClick 路由

```cpp
void MainDemoWindow::OnClick(TNotifyUI& msg) {
    const std::wstring& name = msg.pSender->GetName();
    
    if (name == L"open_modal")       OpenModalPopup();
    else if (name == L"open_modeless") OpenModelessPopup();
    else if (name == L"open_menu")   OpenPopupMenu(msg.pSender);
    else if (name == L"refresh_metrics") RefreshDiagnostics();
    else if (name == L"progress_step")  StepProgress();
    // ... 虚拟列表相关按钮
    
    WindowImplBase::OnClick(msg);    // 默认处理（关闭按钮等）
}
```

### 9.3 自定义消息处理

```cpp
LRESULT MainDemoWindow::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (uMsg == WM_TIMER && wParam == 1001) {
        UpdateFpsMeter();     // 250ms 定时器更新 FPS
        bHandled = TRUE;
        return 0;
    }
    bHandled = FALSE;
    return 0;
}
```

---

## 十、虚拟窗口机制

虚拟窗口允许将一个 HWND 窗口的消息按名称分发到不同的 `CNotifyPump` 子对象：

```cpp
// 注册虚拟窗口
AddVirtualWnd(L"page_settings", &settingsHandler);
AddVirtualWnd(L"page_main", &mainHandler);

// XML 中控件指定虚拟窗口
<Button name="btn_save" virtualwnd="page_settings" />

// 分发时
NotifyPump(msg)
  → 检查 msg.sVirtualWnd
  → 若匹配 "page_settings"，路由给 settingsHandler
  → settingsHandler.LoopDispatch(msg)
```

这种机制将复杂窗口的事件处理拆分到多个独立的处理器中，提升代码组织性。
