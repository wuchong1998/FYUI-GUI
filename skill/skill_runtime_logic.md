# FYUI 代码运行逻辑 Skill

## 一、程序生命周期

### 1.1 初始化阶段

```
CPaintManagerUI::SetInstance(hInstance)    ← 保存全局模块句柄
CPaintManagerUI::SetResourceType(...)     ← 设置资源加载方式
CPaintManagerUI::SetResourcePath(...)     ← 设置资源根路径
OleInitialize()                           ← OLE 拖放初始化
```

### 1.2 窗口创建阶段

```
WindowImplBase::Create(hwndParent, name, style, exStyle, rect)
  → CWindowWnd::Create()
    → RegisterWindowClass()         ← 注册 Win32 窗口类
    → ::CreateWindowEx()            ← 创建原生窗口
    → __WndProc()                   ← 静态窗口过程
         ↓ WM_CREATE
    → WindowImplBase::OnCreate()
      1. m_pm.Init(m_hWnd, name)    ← 初始化 CPaintManagerUI
      2. m_pm.AddPreMessageFilter(this)
      3. m_pm.AddNotifier(this)
      4. CDialogBuilder builder
      5. builder.Create(GetSkinFile(), GetSkinType(), this, &m_pm)
         → 解析 XML → 递归创建控件树
      6. m_pm.AttachDialog(pRoot)   ← 绑定根控件到管理器
      7. m_pm.InitControls(pRoot)   ← 初始化所有控件
      8. InitResource()             ← 可选：加载语言资源
      9. InitWindow()               ← 虚函数：应用初始化逻辑
```

### 1.3 消息循环

```cpp
CPaintManagerUI::MessageLoop()
  while (GetMessage(&msg)) {
    // 1. 预处理消息过滤器
    for (pFilter in preMessageFilters)
      if (pFilter->MessageHandler(msg)) continue;
    // 2. 翻译加速键
    for (pAccelerator in translators)
      if (pAccelerator->TranslateAccelerator(&msg)) continue;
    // 3. 标准翻译和分派
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
```

### 1.4 销毁阶段

```
WindowImplBase::OnDestroy()
  → PostQuitMessage(0)             ← 退出消息循环

CPaintManagerUI::Term()              ← 清理全局资源

窗口析构后：
  → OnFinalMessage(hWnd)           ← 最终清理回调
```

---

## 二、XML 皮肤解析流程

### 2.1 解析入口

```
CDialogBuilder::Create(xml, type, pCallback, pManager, pParent)
  1. CMarkup::LoadFromFile(xmlPath)   ← 加载 XML 文件
     或 CMarkup::Load(xmlString)      ← 加载 XML 字符串
  2. CMarkupNode root = m_xml.GetRoot()
  3. _Parse(&root, pParent, pManager) ← 递归解析
```

### 2.2 XML 标签 → 控件映射

XML 根节点 `<Window>` 的属性直接设置到 `CPaintManagerUI`：

| XML 属性 | 对应设置 |
|----------|----------|
| `size` | `SetInitSize()` |
| `sizebox` | `SetSizeBox()` |
| `caption` | `SetCaptionRect()` |
| `roundcorner` | `SetRoundCorner()` |
| `mininfo` / `maxinfo` | `SetMinInfo()` / `SetMaxInfo()` |
| `showdirty` | `SetShowUpdateRect()` |
| `opacity` | `SetOpacity()` |
| `layered` | `SetLayered()` |
| `renderbackend` | `SetRenderBackend()` |
| `d2dmode` | `SetDirect2DRenderMode()` |

子节点按标签名创建控件：

```
_ParseControlNode(node)
  1. 获取标签名（如 "Button", "VBox"）
  2. 尝试 IDialogBuilderCallback::CreateControl()   ← 应用自定义控件
  3. 若失败，CControlFactory::CreateControl()        ← 工厂创建内置控件
  4. 遍历节点属性，调用 pControl->SetAttribute(name, value)
  5. 若有子节点，递归 _Parse() 创建子控件
  6. pParent->Add(pControl) 或 pParent->AddAt(pControl, index)
```

### 2.3 特殊 XML 节点处理

| 节点 | 处理方式 |
|------|---------|
| `<Image>` | `pManager->AddImage()` 预加载图像 |
| `<Font>` | `pManager->AddFont()` 注册字体 |
| `<Default>` | `pManager->AddDefaultAttributeList()` 设置控件默认属性 |
| `<Style>` | `pManager->AddStyle()` 注册样式 |
| `<Include>` | 递归加载子 XML 文件 |

---

## 三、控件属性系统

### 3.1 SetAttribute 机制

每个控件类重写 `SetAttribute(name, value)` 方法，按属性名解析值：

```cpp
void CButtonUI::SetAttribute(wstring_view name, wstring_view value) {
    if (name == L"normalimage")  SetNormalImage(value);
    else if (name == L"hotimage")  SetHotImage(value);
    else if (name == L"pushedimage") SetPushedImage(value);
    // ...
    else CLabelUI::SetAttribute(name, value);  // 向父类传递
}
```

属性查找链：`具体控件 → 父类控件 → ... → CControlUI`

### 3.2 Style / DefaultAttribute 机制

- **Style**：XML 中定义 `<Style name="ActionButton" value="..." />`，控件通过 `ApplyAttributeList("ActionButton")` 应用
- **DefaultAttribute**：XML 中定义 `<Default name="Button" value="..." />`，所有该类型控件自动应用默认属性

### 3.3 图像属性描述字符串

图像属性支持丰富的描述语法：

```
file='xxx.png' res='IDB_XX' restype='PNG'
dest='x,y,cx,cy'     ← 目标矩形
source='x,y,cx,cy'   ← 源矩形（九宫格裁剪）
corner='l,t,r,b'     ← 九宫格四角
mask='#AARRGGBB'      ← 颜色遮罩
fade='200'            ← 透明度 0~255
hole='true'           ← 九宫格中间镂空
xtiled='true'         ← 水平平铺
ytiled='true'         ← 垂直平铺
hsl='true'            ← 启用 HSL 颜色调整
```

---

## 四、布局计算流程

### 4.1 布局触发

```
CPaintManagerUI::NeedUpdate()
  → m_bUpdateNeeded = true
  → ::InvalidateRect(m_hWnd, NULL, FALSE)  ← 触发 WM_PAINT
```

### 4.2 SetPos 递归

```
WM_PAINT → CPaintManagerUI::MessageHandler()
  → m_pRoot->SetPos(rcClient)              ← 从根控件开始
    → CVerticalLayoutUI::SetPos(rc)
      1. 计算 Inset（内边距）
      2. 处理滚动偏移
      3. 遍历子控件：
         a. 浮动控件 → SetFloatPos(index) → 按百分比/对齐计算位置
         b. 普通控件 → 按固定高度或剩余空间分配
      4. ProcessScrollBar(rc, cxNeeded, cyNeeded)  ← 更新滚动条
```

### 4.3 各布局算法

| 布局 | 算法 |
|------|------|
| `CVerticalLayoutUI` | 垂直排列，按固定高度/权重分配剩余高度 |
| `CHorizontalLayoutUI` | 水平排列，按固定宽度/权重分配剩余宽度 |
| `CTileLayoutUI` | 网格排列，按列数和子项固定尺寸计算 |
| `CTabLayoutUI` | 仅显示当前选中页（`m_iCurSel`），其余页隐藏 |
| `CAnimationTabLayoutUI` | 切换时带滑动动画效果 |
| 浮动控件 | `float="true"` 的控件独立定位，不参与父容器流式布局 |

---

## 五、绘制流程

### 5.1 WM_PAINT 处理

```
CPaintManagerUI::MessageHandler(WM_PAINT)
  1. BeginPaint() → 获取 HDC 和 rcPaint
  2. 如果 m_bUpdateNeeded：
     m_pRoot->SetPos(rcClient)     ← 重新布局
  3. 创建 CPaintRenderContext（D2D 或 GDI 离屏位图）
  4. m_pRoot->DoPaint(renderContext, nullptr)  ← 递归绘制
  5. 提交渲染结果到屏幕
  6. EndPaint()
```

### 5.2 DoPaint 递归

```
CControlUI::DoPaint(renderContext, pStopControl)
  1. CRenderClip::GenerateClip()    ← 设置裁剪区域
  2. PaintBkColor()                 ← 绘制背景色
  3. PaintBkImage()                 ← 绘制背景图
  4. PaintStatusImage()             ← 绘制状态图（由子类实现）
  5. PaintText()                    ← 绘制文本
  6. PaintBorder()                  ← 绘制边框
  7. PaintForeColor()               ← 绘制前景色
  8. PaintForeImage()               ← 绘制前景图

CContainerUI::DoPaint(renderContext, pStopControl)
  1. CControlUI::DoPaint()          ← 绘制自身
  2. 遍历 m_items：
     for (child : m_items)
       if (child 在可见范围内)
         child->DoPaint(renderContext, pStopControl)
  3. 绘制滚动条
```

### 5.3 渲染缓存与优化

| 优化策略 | 说明 |
|----------|------|
| 脏矩形 | `Invalidate(rcItem)` 只重绘变脏区域 |
| 滚动渲染缓存 | `ScrollRenderCacheRect()` — 滚动时 BitBlt 位移已绘制内容，仅补绘新露出区域 |
| 图像缩放缓存 | `TDrawInfo::hCachedScaledBitmap` 缓存已缩放的位图 |
| D2D 批处理 | 合并多个绘制调用减少 GPU 提交 |
| 文本布局缓存 | DirectWrite 文本布局和度量缓存 |
| HTML 解析缓存 | HTML 富文本解析结果缓存 |
| 动画帧请求 | `RequestAnimationFrame()` 按需触发下一帧 |

---

## 六、滚动系统

### 6.1 滚动触发

```
鼠标滚轮 → WM_MOUSEWHEEL → CPaintManagerUI
  → 找到滚轮命中的容器控件
  → CContainerUI::DoEvent(UIEVENT_SCROLLWHEEL)
    → 计算滚动增量（含 DPI 缩放）
    → SetScrollPos(newPos)
```

### 6.2 平滑滚动

```
CContainerUI::SetScrollPos(szTarget)
  if (IsSmoothScrollbar())
    → QueueSmoothScroll(szTarget)  ← 创建滚动动画
    → UIMSG_SCROLL_ANIMATE 消息    ← 定时器驱动
    → 缓动函数插值
    → ApplyScrollPos(interpolated) ← 每帧应用
  else
    → ApplyScrollPos(szTarget)     ← 直接跳转
```

### 6.3 ApplyScrollPos 内部

```
ApplyScrollPos(szPos)
  1. ClampScrollPos(szPos)          ← 限制在有效范围
  2. 如果支持滚动渲染缓存：
     → ScrollRenderCacheRect(...)  ← BitBlt 位移
  3. m_pVerticalScrollBar->SetScrollPos(szPos.cy)
  4. SetPos(m_rcItem)              ← 重新布局子控件
  5. Invalidate()                  ← 触发重绘
  6. SendNotify(DUI_MSGTYPE_SCROLL) ← 通知外部
```

---

## 七、虚拟列表系统 (`CVirtualListUI`)

### 7.1 核心设计

虚拟列表**不创建所有子控件**，仅维护可视区域内的少量控件实例，通过回调动态绑定数据：

```cpp
// 设置回调
virtualList->SetCreateItemCallback([](CVirtualListUI*) -> CControlUI* {
    return new CLabelUI();    // 创建复用的子项控件
});
virtualList->SetBindItemCallback([](CVirtualListUI*, CControlUI* ctrl, ItemIndex idx) {
    ctrl->SetText(L"Item " + std::to_wstring(idx));  // 绑定数据
});
```

### 7.2 支持的模式

| 模式 | API | 说明 |
|------|-----|------|
| 固定行高 | `SetFixedItemHeight(h)` + `SetItemCount(n)` | 所有行高相同，O(1) 定位 |
| 可变行高 | `SetItemHeights(vector<int>)` | 每行不同高度，前缀和定位 |
| 选择 | `GetSelectedIndex()` / `IsItemSelected()` | 单选支持 |
| 点击/双击 | `SetItemClickCallback()` / `SetItemDoubleClickCallback()` | 事件回调 |
| 预加载 | `SetOverscanItemCount(n)` | 可视区域外预加载 n 个子项 |

### 7.3 性能特性

- 支持 **1000 万+** 数据量
- 滚动时仅创建/绑定可视范围内的控件
- 控件实例复用，避免频繁 `new/delete`

---

## 八、定时器系统

### 8.1 控件定时器

```cpp
// 设置定时器
pControl->SetTimer(timerID, elapse);  // 内部调用 m_pManager->SetTimer()

// 处理定时器
void CControlUI::DoEvent(TEventUI& event) {
    if (event.Type == UIEVENT_TIMER) {
        // event.wParam 包含 timerID
    }
}
```

### 8.2 Win32 定时器映射

```
CPaintManagerUI::SetTimer(pControl, nTimerID, uElapse)
  → ::SetTimer(m_hWnd, MAKETIMERKEY(pControl, nTimerID), uElapse, NULL)

WM_TIMER → CPaintManagerUI::MessageHandler()
  → 查找定时器注册表
  → 构造 TEventUI(UIEVENT_TIMER)
  → pControl->DoEvent(event)
```

---

## 九、焦点管理

```
CPaintManagerUI::SetFocus(pControl)
  1. 若已有焦点控件 pOldFocus：
     → 发送 UIEVENT_KILLFOCUS 给 pOldFocus
     → 发送 DUI_MSGTYPE_KILLFOCUS 通知
  2. m_pFocus = pControl
  3. 发送 UIEVENT_SETFOCUS 给 pControl
  4. 发送 DUI_MSGTYPE_SETFOCUS 通知

Tab 键切换：
  CPaintManagerUI::SetNextTabControl(bForward)
    → 在控件树中查找下一个 UIFLAG_TABSTOP 控件
    → SetFocus(pNext)
```

---

## 十、拖拽系统

### 10.1 OLE 拖放

```
CPaintManagerUI::EnableDragDrop(true)
  → RegisterDragDrop(m_hWnd, this)

拖入事件链：
  CIDropTarget::DragEnter() → DragOver() → Drop()
    → CPaintManagerUI::OnDrop()
      → IDragDropUI::OnDragDrop(pControl)
```

### 10.2 控件级拖拽

```
CControlUI::SetDragEnable(true)    ← 允许拖出
CControlUI::SetDropEnable(true)    ← 允许拖入

拖出流程：
  UIEVENT_BUTTONDOWN → 创建 CIDataObject
  → DoDragDrop(pDataObject, pDropSource, dwEffect)
```

---

## 十一、选项组机制

```cpp
// XML 中定义选项组
<Option name="tab1" group="tabs" ... />
<Option name="tab2" group="tabs" ... />

// 内部实现
COptionUI::SetGroup(groupName)
  → m_pManager->AddOptionGroup(groupName, this)

COptionUI::Selected(true)
  → 获取同组所有选项
  → 取消其他选项的选中状态
  → 发送 DUI_MSGTYPE_SELECTCHANGED 通知
```

---

## 十二、菜单系统

```
CMenuWnd::CreateMenu(pParent, xmlSkin, point, pManager, ...)
  1. 创建菜单窗口（LayeredWindow）
  2. CDialogBuilder 解析菜单 XML
  3. 定位菜单位置（对齐方式：左/右/上/下）
  4. 显示菜单窗口
  5. 菜单项点击 → SendNotify(DUI_MSGTYPE_MENU) → 全局观察者通知
  6. 点击外部或 ESC → CMenuWnd::DestroyMenu()

菜单观察者：
  CMenuWnd::GetGlobalContextMenuObserver()  ← 全局观察者
    → RBroadcast(ContextMenuParam)          ← 广播关闭消息
```

---

## 十三、Tooltip 系统

```
鼠标悬停 → WM_MOUSEMOVE → CPaintManagerUI
  → 查找命中控件 pHover
  → 若 pHover 有 ToolTip 文本：
    → 创建/更新 TooltipWnd
    → TooltipInfo { text, position, type, maxWidth, dpi }
    → 定位 Tooltip 位置（Left/Top/Right/Bottom）
    → 显示 Tooltip

鼠标离开控件 → 隐藏 Tooltip
```

`ToolTipType` 枚举决定提示框相对于控件的弹出方向：`Tool_Left`, `Tool_Top`, `Tool_Right`, `Tool_Bottom`

---

## 十四、阴影系统

```
CShadowUI — 窗口阴影实现

CPaintManagerUI::GetShadow()
  → 返回 CShadowUI 实例

CShadowUI 通过创建独立的分层窗口（Layered Window）在主窗口周围绘制阴影效果。

阴影参数：
  - 阴影图像
  - 阴影偏移
  - 阴影大小
  - 阴影透明度
```
