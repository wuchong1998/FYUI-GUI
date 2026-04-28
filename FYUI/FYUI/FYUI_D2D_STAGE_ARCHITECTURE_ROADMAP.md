# FYUI Direct2D 阶段性架构梳理与后续改造清单

## 1. 目标

本次重构的目标不是“把所有 GDI 代码立刻删光”，而是把 FYUI 逐步改造成：

- 使用 Direct2D/DirectWrite 作为主绘制后端
- 支持 GPU / CPU 两种 Direct2D 渲染模式
- 保持 Win7 及以上可用
- 在无法使用 GPU 时自动回退到 CPU 软绘制
- 尽量保持 FYUI 原有控件使用方式、消息模型、XML 语义和整体框架逻辑

当前工程中已经完成了第一阶段和第二阶段的大部分基础设施重构，后续应避免继续零散修补，而要按层次推进。

## 2. 当前架构分层

### 2.1 Manager 调度层

核心文件：

- `Core/UIManager.h`
- `Core/UIManager.cpp`

职责：

- 窗口消息分发
- 整帧 paint 生命周期调度
- 离屏 surface 管理
- layered / 非 layered 输出编排
- native child window 合成
- render diagnostics 统计
- 当前帧 `CPaintRenderContext` 挂接

当前状态：

- `CPaintManagerUI` 已经不再是“到处直接持有窗口 DC 并四处散发”的旧结构
- paint 主链已经围绕统一的 `TPaintFrameState`、`CPaintRenderSurface`、`CPaintRenderContext` 展开
- `WM_PAINT` 与 `WM_PRINTCLIENT` 壳层已经明显变薄
- `measure DC` 已独立出来，字体度量和纯测量逻辑不再依赖长期窗口 DC

### 2.2 Render Surface / Context 层

核心文件：

- `Core/UIRenderContext.h`
- `Core/UIRenderSurface.h`
- `Core/UIRenderTypes.h`

职责：

- `CPaintRenderContext`：描述当前帧绘制上下文
- `CPaintRenderSurface`：描述可复用的离屏 DIB surface
- `RenderBackendType / Direct2DRenderMode`：描述当前后端和 Direct2D 模式

当前状态：

- `CPaintRenderContext` 已成为控件树新的主绘制入口
- `CPaintRenderSurface` 已接住 manager 的主离屏 surface 与位图导出路径
- `RenderBackendType` 和 `Direct2DRenderMode` 已经从渲染器内部概念提升到框架层公共类型

### 2.3 Render Backend 层

核心文件：

- `Core/UIRender.h`
- `Core/UIRender.cpp`

职责：

- Direct2D / DirectWrite / WIC 基础设施
- GDI / GDI+ / FreeType 兼容 fallback
- 图元、图片、文本、HtmlText 绘制
- D2D batch、bitmap cache、text layout cache、html cache
- 逐帧性能指标统计

当前状态：

- Direct2D 已经接管了大量常用路径：颜色、渐变、线框、图片、普通文本、HtmlText 主路径
- 旧 GDI/GDI+ 目前仍保留为兼容 fallback，而不是主路径
- `UIRender` 目前更像真正的“后端层”，不应再继续承接 manager 编排职责

### 2.4 Control / Container 层

核心文件：

- `Core/UIControl.h`
- `Core/UIControl.cpp`
- `Core/UIContainer.h`
- `Core/UIContainer.cpp`
- `Control/*.h`
- `Control/*.cpp`

职责：

- 控件属性、状态、事件、布局估算
- 控件自己的背景 / 前景 / 文本 / 状态图 / 边框绘制
- 容器子控件递归绘制

当前状态：

- `CControlUI` 和 `CContainerUI` 已经是 `CPaintRenderContext` 主路径
- 旧 `HDC` 版本基本退化为兼容 shim
- 常见基础控件已迁入 context 绘制主链：
  - `UILabel`
  - `UIButton`
  - `UIScrollBar`
  - `UICombo`
  - `UIOption`
  - `UIProgress`
  - `UISlider`
  - `UIList` 大量叶子类
  - `UIMenu` 大量叶子类

### 2.5 Host Island 层

核心文件：

- `Control/UIRichEdit.cpp`
- `Control/UIActiveX.cpp`

职责：

- 承接必须继续使用宿主 HDC 协议的系统组件 / COM 组件

当前状态：

- `RichEdit` 与 `ActiveX` 已明确被视为“宿主孤岛”
- 当前策略不是强行移除 HDC，而是把必须保留的 HDC / COM / TextServices 边界集中成 helper
- 这是正确路线，后续不应试图粗暴“全 Direct2D 化”

## 3. 已完成的关键改造

### 3.1 渲染后端

- Direct2D / DirectWrite 基础接入完成
- 支持 `RenderBackendAuto / GDI / Direct2D`
- 支持 `Direct2DRenderModeAuto / Hardware / Software`
- 完成普通文本、图片、九宫格、旋转图、HtmlText 主路径的大量迁移
- 引入 batch、bitmap cache、brush cache、layout cache、html cache、诊断指标

### 3.2 框架主干

- 引入 `CPaintRenderContext`
- 引入 `CPaintRenderSurface`
- `CPaintManagerUI` 主绘制链 context 化
- `measure DC` 独立，长期窗口 DC 依赖基本去除
- `CPaintManagerUI` 现在已经有统一的 `TPaintFrameState`

### 3.3 控件层

- `CControlUI` / `CContainerUI` 已切换为 context 主实现
- 多数常用控件已迁移到 context 主链
- `UIList / UIMenu / UICombo` 已不再完全依赖旧 HDC 主链

### 3.4 宿主边界

- `RichEdit` 已集中 `TextServices` 消息发送、measure、focus、cursor 等 helper
- `ActiveX` 已集中 frame window、host window、object rect、visibility、host DC、`IViewObject::Draw` 等 helper

## 4. 当前仍未完全收口的区域

### 4.1 `UIRender` 里的兼容 fallback 仍然较多

虽然主路径已迁移，但仍存在：

- GDI+ fallback
- FreeType fallback
- 图片特殊分支
- 少量 HtmlText fallback

这些路径不能乱删，但需要进一步梳理“谁是主路径、谁是兼容路径、谁必须保留”。

### 4.2 复杂控件仍有零散旧逻辑

重点包括：

- `Control/UIListEx.cpp`
- `Control/UITreeView.cpp`
- `Control/VirListBodyUI.cpp`
- `Control/VirtualListUI.cpp`
- `Control/UIComboBox.cpp`

这些控件不是完全不能用，而是仍需要阶段性清理，避免后续又把旧 `HDC` 逻辑从边缘重新带回主链。

### 4.3 宿主孤岛仍需“集中边界”，但不应协议重写

重点包括：

- `UIRichEdit`
- `UIActiveX`

这些模块后续目标不是“去掉 HDC”，而是：

- 让所有 HDC / COM / TextServices 边界尽可能集中
- 让 manager / render / control 三层对它们的依赖更清晰

### 4.4 布局与绘制的长期耦合仍存在

当前布局系统总体还稳定，但后续要继续留意：

- invalidation 粒度
- `NeedUpdate / NeedParentUpdate` 传播
- `EstimateSize` 与绘制测量路径耦合
- 列表 / 富文本 / 菜单等复杂控件的布局与绘制互相依赖

## 5. `CPaintManagerUI` 的结论

这是这次阶段梳理里最关键的一点：

`CPaintManagerUI` 这条 paint-frame 主干已经基本完成了当前阶段应该做的结构收口。

可以认为当前已经完成：

- 从“长期窗口 DC 持有者”转为“帧调度器”
- 从“散落参数传递”转为 `TPaintFrameState`
- 从“消息函数里夹杂大量渲染细节”转为 helper 编排
- 从“直接操作离屏资源”转为围绕 `CPaintRenderSurface` 工作

因此，后续不建议继续把主要精力放在反复整理 `CPaintManagerUI` 壳层上。  
后面如果不是明确发现 bug 或行为问题，应把重点转到其他模块。

## 6. 下一阶段改造总路线

### Phase A：固化 Render Backend 边界

目标：

- 明确 `UIRender` 主路径与 fallback 的职责边界
- 减少未来继续把框架逻辑塞回 `UIRender`

重点文件：

- `Core/UIRender.h`
- `Core/UIRender.cpp`

重点动作：

- 梳理图片绘制分支
- 梳理文本绘制分支
- 梳理 HtmlText fallback 分支
- 标出“必须保留 fallback”与“可继续迁移的主路径”

### Phase B：完成复杂控件迁移清单

目标：

- 让剩余复杂控件统一落到 context 主链
- 避免局部控件重新引入旧 HDC 主路径

重点文件：

- `Control/UIListEx.cpp`
- `Control/UITreeView.cpp`
- `Control/VirListBodyUI.cpp`
- `Control/VirtualListUI.cpp`
- `Control/UIComboBox.cpp`

重点动作：

- 逐个控件梳理 `DoPaint` 主路径
- 明确哪些 leaf helper 已 context 化，哪些仍只是兼容桥
- 优先处理高使用频率控件

### Phase C：明确宿主孤岛边界

目标：

- 把 `RichEdit / ActiveX` 稳定为“可接受的宿主 HDC 岛”
- 不再试图把这两类控件硬塞进纯 D2D 协议模型

重点文件：

- `Control/UIRichEdit.cpp`
- `Control/UIActiveX.cpp`

重点动作：

- 继续集中 TextServices / COM / HDC helper
- 保持行为稳定，不动协议逻辑
- 形成明确的“允许保留 HDC 的边界清单”

### Phase D：形成新的架构边界文档

目标：

- 把当前 FYUI 新架构固化成团队可遵循的规则

需要最终形成的规则：

- manager 只做帧调度
- render surface 只做离屏资源
- render context 只做帧上下文
- render engine 只做后端绘制
- control 只做控件逻辑与绘制决策
- host island 只承接必须保留的宿主协议

## 7. 近期具体执行清单

### 7.1 优先级 P1

- 梳理 `UIRender.cpp` 图片主路径与 fallback 分支
- 梳理 `UIRender.cpp` 文本 / HtmlText 主路径与 fallback 分支
- 输出一份 render backend 边界清单

### 7.2 优先级 P2

- 处理 `UIListEx.cpp`
- 处理 `UITreeView.cpp`
- 处理 `VirListBodyUI.cpp`
- 处理 `VirtualListUI.cpp`

### 7.3 优先级 P3

- 继续收 `UIRichEdit` 宿主 helper
- 继续收 `UIActiveX` 宿主 helper
- 形成“宿主 HDC 允许边界表”

## 8. 开发约束

后续继续重构时，建议严格遵守：

- 不再把 manager 编排逻辑重新塞回 `UIRender`
- 不再把后端细节重新塞进控件层
- 不为了“全 D2D”去破坏 `RichEdit / ActiveX` 的宿主协议
- 优先保持 FYUI 现有使用方式、XML 语义和控件逻辑稳定
- 遇到不确定的旧行为，优先参考备份工程：
  - `D:\work\FYUI2\FYUI\FYUI`

## 9. 当前阶段结论

当前 FYUI 已经完成了“从旧 Duilib 风格 GDI 主框架，转向可持续演进的 Direct2D 主框架”的最难基础工作。

下一阶段不应该继续把精力主要耗在：

- 反复微调 `CPaintManagerUI`
- 继续给 `UIRender` 塞更多框架职责

而应该转向：

- 后端边界梳理
- 复杂控件迁移收尾
- 宿主孤岛边界固化
- 架构规则文档化

这份文档就是后续推进的阶段性主线。
