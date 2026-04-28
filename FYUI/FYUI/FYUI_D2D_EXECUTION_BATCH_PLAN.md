# FYUI Direct2D 工程级执行批次清单

## 1. 目标

这份清单比阶段路线文档更偏执行层。

目标不是再讨论“要不要改”，而是明确：

- 下一阶段按什么顺序做
- 哪些任务可以并行
- 哪些任务必须串行
- 哪些任务是低风险收口，哪些是高风险主干改造

当前默认原则：

- 保持 FYUI 原有控件接口、XML 用法和总体逻辑不变
- Direct2D / DirectWrite 继续作为主渲染后端
- `HDC` 不追求一次性消灭，而是收缩到兼容孤岛
- 优先收口边界、减少双轨，再继续铺新功能

## 2. 当前状态摘要

### 已基本完成

- `CPaintManagerUI` 已经从旧的长期窗口 DC 模式切到 `frame state + render context + render surface`
- `CControlUI / CContainerUI` 已经以 `CPaintRenderContext` 为主路径
- `UIRender` 已经形成 Direct2D / DirectWrite 主路径
- `UIRender.cpp` 已经开始按图片、图元、bitmap util、clip 等主题拆分
- `RichEdit / ActiveX` 已经明确为宿主孤岛，而不是普通控件

### 当前主要问题

- `Core` 层的抽象边界已经出现，但还没完全收口
- `Control` 层复杂控件仍存在 “context 入口 + 旧 HDC 真逻辑” 双轨
- `Layout` 与工程组织层缺少更明确的契约和执行入口
- `UIRender` 文本 / HtmlText 子系统仍然偏重

## 3. 执行总策略

下一阶段按四条主线推进：

1. `Core` 边界收口
2. 复杂控件双轨收尾
3. 宿主孤岛边界固化
4. 工程组织与验证载体补齐

推进原则：

- 每轮同时做 1 个主任务 + 2 到 4 个低风险小任务
- 高风险任务尽量单独成包，避免互相污染
- 文档、工程视图、代码边界同步推进，不等最后补文档

## 4. 批次规划

### Batch A：Core 边界收口

优先级：最高

目标：

- 把 `Core` 层里已经出现的抽象正式定型
- 避免后续控件层和资源层继续反向侵入 `UIManager / UIRender`

任务：

1. 资源读流统一层
   - 文件：`Core/UIRenderImageCodec.cpp`、`Core/UIMakup.cpp`、`Core/UIResourceManager.cpp`、`Core/UIManager.cpp`
   - 风险：中
   - 说明：统一 `file / zip / resource / membuffer` 读取路径、失败路径和 zip 句柄策略

2. `UIRenderContext / UIRenderSurface` 契约落盘
   - 文件：`Core/UIRenderContext.h`、`Core/UIRenderSurface.h`、`Core/UIControl.cpp`、`Core/UIManager.cpp`
   - 风险：中
   - 说明：把兼容 context、surface 创建与使用边界从“事实存在”变成“正式契约”

3. 图片 `DrawInfo / 缩放缓存 / DPI-HSL` 失效归一
   - 文件：`Core/UIRenderImageEntry.cpp`、`Core/UIRenderImageScale.cpp`、`Core/UIRenderImageRuntime.cpp`、`Core/UIRenderImageSpec.cpp`、`Core/UIManager.cpp`
   - 风险：中
   - 说明：避免 manager 和 render 两边同时管理图片失效

4. `UIRender` 文本 / HtmlText 子系统继续减混责
   - 文件：`Core/UIRender.cpp`、`Core/UIRender.h`
   - 风险：高
   - 说明：继续做同文件内分层，不急着继续横向拆散

### Batch B：复杂控件双轨收尾

优先级：高

目标：

- 把 “context 入口 + HDC 真逻辑” 双轨变成“context 真逻辑 + HDC 兼容壳”

任务：

1. `UIList / UIListEx` 主干去双轨
   - 文件：`Control/UIList.cpp`、`Control/UIListEx.cpp`、`Control/UIList.h`
   - 风险：高

2. `UIMenu` popup 宿主与 item paint 收口
   - 文件：`Control/UIMenu.cpp`、`Control/UIMenu.h`
   - 风险：高

3. `UICombo / UIComboBox` 下拉宿主与当前项绘制统一
   - 文件：`Control/UICombo.cpp`、`Control/UIComboBox.cpp`
   - 风险：中高

4. `UITreeView / VirListBodyUI / VirtualListUI` 跟进
   - 文件：`Control/UITreeView.cpp`、`Control/VirListBodyUI.cpp`、`Control/VirtualListUI.cpp`
   - 风险：中高

建议顺序：

- 先 `UIList / UIListEx`
- 再 `UIMenu`
- 再 `UICombo / UIComboBox`
- 最后收 `Tree / VirtualList`

### Batch C：宿主孤岛固化

优先级：高

目标：

- 明确必须保留 `HDC / HWND / COM / TextServices` 的边界
- 不再把这些协议细节扩散回普通渲染链

任务：

1. `RichEdit` TextServices / 宿主 DC 收口
   - 文件：`Control/UIRichEdit.cpp`、`Control/UIRichEdit.h`
   - 风险：高

2. `ActiveX` 宿主窗口 / windowless 边界收口
   - 文件：`Control/UIActiveX.cpp`、`Control/UIActiveX.h`
   - 风险：高

3. `Edit` 原生 HWND 宿主孤岛收口
   - 文件：`Control/UIEdit.cpp`、`Control/UIEdit.h`
   - 风险：中高

说明：

- 这三个任务都不追求“去掉 HDC”
- 目标是把 `HDC` 变成清晰、集中、可维护的边界

### Batch D：独立控件残余 HDC 边界收尾

优先级：中

目标：

- 并行清理一批独立且耦合较小的残余 HDC 边界

任务：

1. `UIGifAnim` 帧缓存边界收口
   - 文件：`Control/UIGifAnim.cpp`
   - 风险：中

2. `UIColorPalette` 离屏缓存 / MemDC 收口
   - 文件：`Control/UIColorPalette.cpp`
   - 风险：中

这些任务适合与 `Batch B / C` 并行穿插推进。

### Batch E：工程组织与验证载体

优先级：中

目标：

- 让工程本身更容易维护、验证和继续推进

任务：

1. 构建配置矩阵收敛
   - 文件：`FYUI.vcxproj`、`FYUI.sln`
   - 风险：中

2. 构建产物隔离与忽略策略
   - 文件：工程目录与输出目录配置
   - 风险：中

3. `vcxproj / filters` 与真实源码树同步
   - 文件：`FYUI.vcxproj`、`FYUI.vcxproj.filters`
   - 风险：低

4. 布局链说明文档
   - 文件：`Layout/*.h`、`Layout/*.cpp`、`Core/ControlFactory.cpp`、`Core/UIDlgBuilder.cpp`
   - 风险：低

5. 最小 smoke / demo 验证载体
   - 文件：待新增
   - 风险：中

## 5. 并行包建议

### 可并行包 1

- `Core`：资源读流统一层
- `Control`：`UIGifAnim`
- `工程`：`vcxproj / filters` 同步

### 可并行包 2

- `Core`：`UIRenderContext / UIRenderSurface` 契约落盘
- `Control`：`UIColorPalette`
- `文档`：布局链说明

### 可并行包 3

- `Control`：`UIList / UIListEx`
- `Control`：`UIMenu`
- `Core`：`UIRender` 文本 / HtmlText 继续减混责

### 可并行包 4

- `宿主`：`RichEdit`
- `宿主`：`ActiveX`
- `宿主`：`Edit`

说明：

- 包 3 与包 4 不建议混做太多共享文件
- `UIManager.cpp`、`UIRender.cpp`、`UIList.cpp` 这些热点文件不要在多个 worker 间同时写

## 6. 近期建议的三轮执行顺序

### 第 1 轮

- `UIRender` 文本 / HtmlText 继续做低风险内部收口
- `vcxproj / filters` 与真实源码树同步
- 增补工程级执行清单和布局链文档入口

### 第 2 轮

- 资源读流统一层
- `UIGifAnim`
- `UIColorPalette`

### 第 3 轮

- `UIList / UIListEx`
- `UIMenu`
- `UICombo / UIComboBox`

## 7. 当前轮次执行规则

后续每轮默认这样做：

- 先确认这轮主任务属于哪个 batch
- 再挑 2 到 4 个低风险小任务一起做
- 最后必须全量编译验证

如果某轮任务涉及：

- `UIManager.cpp`
- `UIRender.cpp`
- `UIList.cpp`
- `UIRichEdit.cpp`
- `UIActiveX.cpp`

则尽量避免同轮再去改另一个高冲突大文件。

## 8. 结论

从现在开始，FYUI 的改造不再适合按“看到哪里改哪里”的方式推进。

更适合的方式是：

- 先按 batch 划分任务
- 每轮做一个主任务包
- 再叠加几个低风险收尾项
- 通过编译和诊断持续验证

这样速度会更快，也更不容易把工程重新改回屎山。
