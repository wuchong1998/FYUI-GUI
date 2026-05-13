# FYUI 框架架构 Skill

## 一、项目概览

FYUI 是一个基于 Windows 平台的 **DirectUI** 风格 C++ GUI 框架，采用 **无子窗口（Windowless）** 设计，所有控件均在单一 `HWND` 上通过自绘实现。框架支持 **GDI** 和 **Direct2D** 双渲染后端，具备 DPI 感知、XML 皮肤布局、虚拟列表、动画系统等现代 UI 能力。

### 解决方案结构

```
FYUI-GUI/
├── FYUI/                        # FYUI 框架库（静态库/DLL）
│   └── FYUI/
│       ├── Core/                # 核心模块
│       ├── Control/             # 控件集合
│       ├── Layout/              # 布局管理器
│       └── Utils/               # 工具集
├── FYTest/                      # 演示/测试应用程序
│   └── FYTest/
│       └── FYTest/
│           ├── FYTest.cpp       # 程序入口 WinMain
│           ├── MainDemoWindow   # 主演示窗口
│           ├── DemoPopupWindow  # 弹窗演示
│           └── res/             # 资源文件（XML皮肤、图片）
├── third_party/                 # 第三方库（libwebp）
└── skill/                       # 本 Skill 文档目录
```

### 编译配置

| 配置项 | 说明 |
|--------|------|
| 编译器 | MSVC (Visual Studio 2022+) |
| C++ 标准 | C++17 (`std::wstring_view`, `std::string_view`) |
| 字符集 | Unicode (宽字符 `wchar_t`) |
| 链接模式 | 静态库 (`FYUI_STATIC`) 或 DLL (`FYUI_EXPORTS`) |
| 平台 | x64 / Win32 |
| 依赖 | GDI+, Direct2D, DirectWrite, OLE (拖拽), libwebp |

---

## 二、命名空间与导出

- **命名空间**：所有 FYUI 类均位于 `FYUI` 命名空间
- **导出宏**：
  - `FYUI_API`：静态库下为空，DLL 下分别为 `__declspec(dllexport)` / `__declspec(dllimport)`
  - `FYUI_COMDAT`：`__declspec(selectany)`，用于消息映射表的链接器去重
- **统一入口头文件**：`FYUILib.h`——按顺序 include 所有模块头文件，外部应用只需 `#include "FYUILib.h"`

---

## 三、核心类层级

```
CWindowWnd                      ← Win32 窗口封装（HWND 持有者）
  └─ WindowImplBase             ← 应用窗口基类（多继承 CNotifyPump + INotifyUI + IMessageFilterUI + IDialogBuilderCallback）
       └─ MainDemoWindow        ← 具体应用窗口（FYTest 示例）

CNotifyPump                     ← DUI 消息泵（消息映射表分发）

CPaintManagerUI                 ← 绘制管理器（每窗口一个实例，核心枢纽）
  ├─ 控件树根 (CControlUI*)
  ├─ 资源管理 (TResInfo)
  ├─ 渲染表面 (CRenderSurface)
  ├─ 事件分发 / 定时器 / 焦点管理
  └─ DPI 缩放 / 渲染诊断

CControlUI                      ← 所有控件基类
  └─ CContainerUI               ← 容器控件基类（IContainerUI 接口）
       ├─ CVerticalLayoutUI      ← 垂直布局
       ├─ CHorizontalLayoutUI    ← 水平布局
       ├─ CTileLayoutUI          ← 瓦片/网格布局
       ├─ CTabLayoutUI           ← 选项卡布局
       ├─ CAnimationTabLayoutUI  ← 动画切换布局
       ├─ CChildLayoutUI         ← 子 XML 布局
       └─ CListUI / CVirtualListUI ← 列表控件
```

---

## 四、核心模块详解

### 4.1 Core/ — 核心引擎

| 文件 | 职责 |
|------|------|
| `UIManager.h/cpp` | **CPaintManagerUI** — 绘制管理器，框架枢纽。管理控件树、消息循环、资源（字体/图像/样式）、定时器、焦点、渲染后端、DPI 缩放、拖拽等 |
| `UIControl.h/cpp` | **CControlUI** — 所有控件基类。定义名称、位置(RECT)、可见性、启用状态、背景/前景/边框、内边距、文本、ToolTip、光标、事件处理(`DoEvent`)、绘制(`DoPaint`) 等通用接口 |
| `UIContainer.h/cpp` | **CContainerUI** — 容器基类。管理子控件列表(`m_items`)、滚动条（垂直/水平 `CScrollBarUI`）、平滑滚动、浮动布局、克隆等 |
| `UIDefine.h` | 消息类型宏定义 (`DUI_MSGTYPE_*`)、控件名称宏 (`DUI_CTR_*`)、消息映射宏 (`DUI_DECLARE_MESSAGE_MAP` 等)、通知结构体 `TNotifyUI` |
| `UIBase.h/cpp` | **CWindowWnd** — Win32 窗口封装。注册窗口类、创建窗口、子类化、消息处理(`HandleMessage`)。**CNotifyPump** — 消息泵，支持虚拟窗口分发 |
| `UIMakup.h/cpp` | **CMarkup / CMarkupNode** — 轻量 XML 解析器（非 DOM，原地解析）。支持 UTF-8/Unicode/ANSI 编码 |
| `UIDlgBuilder.h/cpp` | **CDialogBuilder** — XML 皮肤构建器。解析 XML 递归创建控件树，支持 `IDialogBuilderCallback` 自定义控件创建 |
| `ControlFactory.h/cpp` | **CControlFactory** — 控件工厂（单例）。通过 `DECLARE_DUICONTROL` / `IMPLEMENT_DUICONTROL` / `REGIST_DUICONTROL` 宏自动注册控件 |
| `UIRender.h/cpp` | **CRenderEngine** — 全局静态绘制引擎。**CRenderClip** — 裁剪栈。提供图像加载/绘制、几何绘制（矩形/圆角/渐变/椭圆/弧/贝塞尔/多边形）、文本绘制（普通/HTML 富文本）、HSL 颜色工具、位图快照 |
| `UIResourceManager.h/cpp` | 多语言资源管理 |
| `UIResourceData.h/cpp` | 资源数据辅助 |
| `Render/` | 渲染子系统（D2D 资源管理、DirectWrite 文本缓存、图像编解码、渲染表面等 38 个文件） |

### 4.2 Control/ — 控件集合

| 控件类 | 说明 |
|--------|------|
| `CButtonUI` | 按钮（多状态图像：normal/hot/pushed/focused/disabled） |
| `CButtonExUI` | 扩展按钮 |
| `CFadeButtonUI` | 渐变过渡按钮 |
| `COptionUI` | 单选/多选按钮（支持 Group） |
| `CLabelUI` | 标签文本（支持 HTML 富文本） |
| `CRichEditUI` | 富文本编辑框（基于 RichEdit 控件） |
| `CComboUI` / `CComboBoxUI` | 下拉框 |
| `CListUI` / `CListExUI` | 列表控件 |
| `CVirtualListUI` | 虚拟列表（支持千万级数据量、固定/可变行高） |
| `CScrollBarUI` | 滚动条（支持浮动悬浮显示） |
| `CSliderUI` | 滑块 |
| `CProgressUI` | 进度条 |
| `CTreeViewUI` | 树形控件 |
| `CMenuUI` (`CMenuWnd`) | 右键/弹出菜单 |
| `CGifAnimUI` | GIF 动画 |
| `CWebpAnimUI` | WebP 动画 |
| `CColorPaletteUI` | 调色板 |
| `CAnimationUI` | 动画基类 |
| `CCountdownUI` | 倒计时 |
| `CRingUI` | 环形控件 |
| `CFrameTestUI` | 帧测试控件 |

### 4.3 Layout/ — 布局管理器

| 布局类 | 说明 |
|--------|------|
| `CVerticalLayoutUI` | 垂直排列（`VBox`），支持分隔线拖拽 |
| `CHorizontalLayoutUI` | 水平排列（`HBox`），支持分隔线拖拽 |
| `CTileLayoutUI` | 瓦片/网格排列（`TileBox`） |
| `CTabLayoutUI` | 选项卡切换（`TabBox`） |
| `CAnimationTabLayoutUI` | 带动画效果的选项卡切换 |
| `CChildLayoutUI` | 引用子 XML 文件的布局（`ChildBox`） |

辅助头文件：
- `UILayoutContentUtil.h` — 内容适配布局工具
- `UILayoutLinearUtil.h` — 线性布局计算工具

### 4.4 Utils/ — 工具集

| 工具 | 说明 |
|------|------|
| `WindowImplBase` | 应用窗口基类，集成 `CWindowWnd` + `CNotifyPump` + `INotifyUI` + `IMessageFilterUI` + `IDialogBuilderCallback`，处理非客户区、DPI 变化等 |
| `Utils.h/cpp` | 字符串格式化(`StringUtil::Format`)、路径工具、几何辅助类(`CDuiPoint/CDuiSize/CDuiRect`)、数组容器(`CStdPtrArray/CStdValArray`)、`STRINGorID` |
| `DPI.h/cpp` | DPI 感知、缩放计算 |
| `UIShadow.h/cpp` | 窗口阴影效果 |
| `UIDelegate.h/cpp` | 事件委托/回调 |
| `DragDropImpl.h/cpp` | OLE 拖放实现 (`CIDropTarget/CIDropSource/CIDataObject`) |
| `TrayIcon.h/cpp` | 系统托盘图标 |
| `observer_impl_base.h/cpp` | 观察者模式基类 |
| `sdk_util.h/cpp` | SDK 辅助工具 |
| `unzip.h/cpp` | ZIP 解压（资源包支持） |
| `stb_image.h` | stb 图像解码库 |

---

## 五、关键数据结构

### 5.1 通知结构体 `TNotifyUI`

```cpp
struct TNotifyUI {
    std::wstring sType;         // 消息类型字符串（如 "click", "timer"）
    std::wstring sVirtualWnd;   // 虚拟窗口名称
    CControlUI* pSender;        // 发送控件
    DWORD dwTimestamp;           // 时间戳
    POINT ptMouse;               // 鼠标位置
    WPARAM wParam;               // 附加参数
    LPARAM lParam;               // 附加参数
};
```

### 5.2 事件结构体 `TEventUI`

```cpp
struct TEventUI {
    int Type;                    // 事件类型（EVENTTYPE_UI 枚举）
    CControlUI* pSender;        // 事件来源控件
    DWORD dwTimestamp;
    POINT ptMouse;
    wchar_t chKey;               // 按键字符
    WORD wKeyState;              // 修饰键状态
    WPARAM wParam;
    LPARAM lParam;
};
```

### 5.3 图像相关

| 结构 | 用途 |
|------|------|
| `TImageInfo` | 已加载图像信息（HBITMAP、尺寸、Alpha、GDI+ Image 等） |
| `TDrawInfo` | 图像绘制描述（图像名、源/目标矩形、九宫格角、淡入度、旋转角等） |
| `TFontInfo` | 字体信息（HFONT、字体名、大小、粗体/斜体等） |
| `TResInfo` | 资源集合（默认颜色、字体表、图像哈希、属性表、样式表） |

### 5.4 渲染相关

| 类型 | 说明 |
|------|------|
| `RenderBackendType` | 渲染后端枚举：`RenderBackendAuto` / `RenderBackendDirect2D` |
| `Direct2DRenderMode` | D2D 模式：`Auto` / `Hardware` / `Software` |
| `CPaintRenderContext` | 绘制上下文（封装 HDC 或 D2D RenderTarget） |
| `CRenderSurface` | 渲染表面管理 |
| `TRenderDiagnostics` | 渲染诊断信息（FPS、帧耗时、缓存命中率等） |

---

## 六、控件工厂与注册机制

FYUI 使用**静态注册+工厂模式**实现 XML 标签到 C++ 控件的映射：

```cpp
// 1. 在控件头文件中声明
class CButtonUI : public CControlUI {
    DECLARE_DUICONTROL(CButtonUI)   // 展开为 static CControlUI* CreateControl();
    ...
};

// 2. 在控件 cpp 中实现
IMPLEMENT_DUICONTROL(CButtonUI)     // 展开为 return new CButtonUI;

// 3. 在 ControlFactory 构造函数中注册
INNER_REGISTER_DUICONTROL(CButtonUI)
// 等价于：RegistControl(L"CButtonUI", CButtonUI::CreateControl);
```

XML 解析时 `CDialogBuilder` → `CControlFactory::CreateControl(className)` → 实例化对应控件。

---

## 七、资源加载机制

### 7.1 资源类型

```cpp
enum UILIB_RESTYPE {
    UILIB_FILE = 1,       // 磁盘文件
    UILIB_ZIP,            // ZIP 压缩包
    UILIB_RESOURCE,       // 可执行文件资源
    UILIB_ZIPRESOURCE,    // 资源内嵌 ZIP
};
```

### 7.2 资源路径配置（以 FYTest 为例）

```cpp
CPaintManagerUI::SetInstance(hInstance);           // 设置模块句柄
CPaintManagerUI::SetResourceType(UILIB_FILE);     // 使用磁盘文件
CPaintManagerUI::SetResourcePath(L"...\\res\\");  // 设置资源目录
```

### 7.3 图像加载链路

```
CControlUI::DrawImage()
  → CRenderEngine::DrawImageString()
    → CPaintManagerUI::GetDrawInfo() / GetImageEx()
      → CRenderEngine::LoadImage()
        → stb_image / GDI+ / WIC 解码
        → 缓存到 TResInfo::m_ImageHash
```

---

## 八、DPI 缩放体系

`CPaintManagerUI` 提供完整的缩放/反缩放 API：

| 方法 | 说明 |
|------|------|
| `ScaleValue(int)` / `UnscaleValue(int)` | 数值缩放 |
| `ScaleRect(RECT)` / `UnscaleRect(RECT)` | 矩形缩放 |
| `ScaleSize(SIZE)` / `UnscaleSize(SIZE)` | 尺寸缩放 |
| `ScalePoint(POINT)` / `UnscalePoint(POINT)` | 坐标缩放 |

控件的 `SetAttribute()` 中解析的像素值通常会经过 `ScaleValue()` 转换。

---

## 九、渲染后端架构

```
CPaintManagerUI
  ├─ SetRenderBackend(RenderBackendType)    # 配置渲染后端
  ├─ SetDirect2DRenderMode(Direct2DRenderMode)  # D2D 硬件/软件模式
  ├─ GetActiveRenderBackend()                # 获取实际激活的后端
  └─ Render/
       ├─ UIRenderSurface        # 渲染表面管理（创建/调整/提交）
       ├─ UIRenderContext        # CPaintRenderContext（HDC 或 D2D RT）
       ├─ UIRenderD2DResource    # D2D 资源（画刷、设备等）
       ├─ UIRenderD2DShared      # D2D 全局共享资源
       ├─ UIRenderDirectWriteText# DirectWrite 文本布局/缓存
       ├─ UIRenderImageCodec     # 图像编解码（stb/GDI+/WIC）
       ├─ UIRenderImageD2D       # D2D 位图缓存
       ├─ UIRenderHtmlParse      # HTML 富文本解析
       └─ UIRenderBatchInternal  # D2D 批处理
```

渲染诊断通过 `TRenderDiagnostics` 结构体暴露：
- FPS / 帧耗时 / 平均帧间隔
- 滚动渲染缓存命中率
- D2D 批处理 Flush 计数
- 文本布局/度量缓存命中率
- HTML 解析/渲染缓存命中率

---

## 十、FYTest 演示应用架构

### 10.1 启动流程

```
wWinMain()
  → CPaintManagerUI::SetInstance(hInstance)
  → CPaintManagerUI::SetResourceType(UILIB_FILE)
  → CPaintManagerUI::SetResourcePath("...\\res\\")
  → OleInitialize()                    # OLE 拖放支持
  → MainDemoWindow::Create()           # 创建主窗口
       → WindowImplBase::OnCreate()
            → CDialogBuilder::Create("fytest_main.xml")  # 解析 XML 皮肤
            → CPaintManagerUI::AttachDialog(root)         # 绑定控件树
            → CPaintManagerUI::AddNotifier(this)          # 注册通知
  → CenterWindow() → ShowWindow()
  → CPaintManagerUI::MessageLoop()     # 消息循环
  → CPaintManagerUI::Term()            # 清理
  → OleUninitialize()
```

### 10.2 MainDemoWindow 功能演示

| 功能 | 实现 |
|------|------|
| 虚拟列表 | `CVirtualListUI` 回调式创建/绑定子项，支持 1000 万级数据 |
| 动画 | HideAnimation 显示/隐藏动画（四方向） |
| 弹窗 | 模态(`ShowModal`)和非模态(`ShowWindow`)弹窗 |
| 右键菜单 | `CMenuWnd::CreateMenu()` |
| 进度条 | `CProgressUI::SetValue()` |
| 滑块 | `CSliderUI` 值变化事件 |
| FPS 计数器 | `TRenderDiagnostics` 实时帧率监控 |
| 压力测试 | TileLayout 200 个按钮压力渲染 |
