# FYUI Direct2D 专项梳理：UIRender 主路径与 Fallback 边界

## 1. 目的

这份文档只聚焦 `Core/UIRender.*`。

目标不是继续把更多框架逻辑塞进 `UIRender`，而是把当前已经形成的三条核心绘制链说明白：

- 图片链
- 普通文本链
- `HtmlText` 链

并明确：

- 现在谁是主路径
- 谁是兼容 fallback
- 哪些边界必须保留
- 下一步该按什么顺序继续收口

## 2. 当前 `UIRender` 的角色

核心文件：

- `Core/UIRender.h`
- `Core/UIRender.cpp`

当前 `UIRender` 应被视为：

- 绘图后端层
- D2D / DWrite / WIC / GDI / GDI+ / FreeType 兼容层
- 图片、文本、富文本的底层绘制实现层

当前 `UIRender` 不应继续承担：

- manager 的帧编排职责
- 控件层的业务判断职责
- 宿主控件的协议适配职责

## 3. 图片链边界

### 3.1 当前主入口

图片链的主要公开入口仍然是：

- `CRenderEngine::DrawImage`
- `CRenderEngine::DrawRotateImage`
- `CRenderEngine::DrawImageInfo`
- `CRenderEngine::DrawImageString`
- `CRenderEngine::GenerateBitmap`

相关实现集中在：

- `Core/UIRender.cpp`

### 3.2 当前主路径

图片主路径已经明显偏向 Direct2D：

- `CRenderEngine::DrawImage(HDC, HBITMAP, ...)`
  - 优先进入 `TryDrawImageWithDirect2D(...)`
- `CRenderEngine::DrawRotateImage(...)`
  - 优先进入 `TryDrawBitmapRectWithDirect2D(...)`
- `DrawImage(...)` 内部图片解析后
  - 非 `bGdiplus` 路径最终仍优先走 `CRenderEngine::DrawImage(...)`
  - `bGdiplus` 路径先尝试 `TryDrawBitmapRectWithDirect2D(...)`

也就是说：

- Bitmap 贴图、九宫格、平铺、中空、旋转图，当前已经是 Direct2D 主导

### 3.3 当前 fallback

图片链当前仍保留三类重要 fallback：

- `GdiplusDrawImage(...)`
  - 用于 `bGdiplus` 图片、旋转图片、Direct2D 无法直接接住的场景
- `DrawImage(...)` 内部 legacy 缩放路径
  - 通过 `smt / smt2` 生成临时缩放位图
  - 当前只在非 Direct2D 后端时才真正有意义
- `GenerateBitmap(...)`
  - 已经接入 `CPaintRenderSurface + CPaintRenderContext`
  - 但本质仍是“离屏截图/导出”，不应和普通控件在线绘制主路径混在一起

### 3.4 当前边界结论

图片链当前应明确成：

- 主路径：Direct2D bitmap 绘制
- 兼容 fallback：GDI+ 与 legacy 缩放位图
- 特殊路径：`GenerateBitmap` 属于离屏导出，不属于普通在线绘制主链

### 3.5 下一步建议

- 提炼图片链“主路径可用性判定” helper
- 把 legacy 缩放位图路径的存在条件写清楚
- 把 `bGdiplus` 图片路径与普通 bitmap 路径的边界写清楚
- 后续不要再把 manager / control 的图片逻辑反向塞进这里

## 4. 普通文本链边界

### 4.1 当前主入口

普通文本公开入口是：

- `CRenderEngine::DrawText(...)`
- `CRenderEngine::GetTextSize(...)`

### 4.2 当前主路径

普通文本当前主路径已经是：

- `DrawText(...)`
  - 先走 `TryDrawTextWithDirectWrite(...)`

这条 DirectWrite 路径背后已经拥有：

- 文本预处理缓存
- `IDWriteTextFormat` 缓存
- `IDWriteTextLayout` 缓存
- metrics cache
- 普通文本 normalization cache

因此对于大多数正常文本场景：

- DirectWrite 是主绘制路径
- 量字和绘制都已经偏向缓存化

### 4.3 当前 fallback

普通文本当前 fallback 顺序大致是：

- 若 `TryDrawTextWithDirectWrite(...)` 失败
  - 根据调用参数与字体路径，进入 `FreeTypeDrawText(...)`
  - 再失败则进入 `GdiplusDrawText(...)`

这里要注意：

- `bGDIPlusDrawText` 参数会强制走 `GdiplusDrawText(...)`
- `FreeTypeDrawText(...)` 不是主路径，而是兼容路径
- `GdiplusDrawText(...)` 是最后兜底，不应继续向上扩散使用场景

### 4.4 当前边界结论

普通文本链当前应明确成：

- 主路径：DirectWrite
- 次级 fallback：FreeType
- 最终 fallback：GDI+

### 4.5 下一步建议

- 提炼“为什么 DirectWrite 没接住”的显式原因
- 不再让新功能默认走 GDI+/FreeType
- 把 `bGDIPlusDrawText` 这类特殊参数的适用范围继续收窄

## 5. `HtmlText` 链边界

### 5.1 当前主入口

富文本入口是：

- `CRenderEngine::DrawHtmlText(...)`

### 5.2 当前主路径

`HtmlText` 当前主路径是：

- `DrawHtmlText(...)`
  - 先走 `TryDrawHtmlTextWithDirectWrite(...)`

这条新路径已经包含：

- mini-html 解析
- 解析缓存
- layout 缓存
- metrics 缓存
- hit-test 缓存
- drawing effect 缓存
- inline image 支持
- 链接命中矩形回填
- 前景色、背景高亮、选中态等一批样式处理

因此 `HtmlText` 现在已经不只是“试验性路径”，而是实际主路径。

### 5.3 当前 fallback

`HtmlText` 的旧路径仍完整保留在 `DrawHtmlText(...)` 后半段。

当前 fallback 主要发生在：

- DirectWrite 解析失败
- DirectWrite 布局失败
- DirectWrite 渲染失败
- 当前标签语义未完全被新路径覆盖
- 某些旧 FYUI 特殊行为仍只在 legacy parser 中存在

当前 legacy fallback 特征：

- 真正绘制时会计入 `frameLegacyTextFallbackCount`
- 真正绘制时会先 `FlushDirect2DBatch(hDC)`
- 使用旧 mini-html parser 与 GDI 量字/绘制语义

### 5.4 当前边界结论

`HtmlText` 应明确成：

- 主路径：DirectWrite `HtmlText`
- 兼容 fallback：旧 mini-html GDI 路径

并且当前最大的风险不是“功能不能用”，而是：

- fallback 原因还不够显式
- 某些标签/语义是否必须继续保留旧行为，还需要梳理

### 5.5 下一步建议

- 把 `HtmlText` fallback 按“解析失败 / 语义未支持 / 渲染失败”明确分类
- 明确哪些标签必须继续迁移，哪些可以长期留在 legacy 路径
- 不再在控件层直接绕开 `DrawHtmlText(...)` 做特殊富文本绘制

## 6. 当前 `UIRender` 里的三条“不要再做”的事情

### 6.1 不要再把 manager 编排逻辑塞回 `UIRender`

例如：

- 不要让 `UIRender` 再去决定帧生命周期
- 不要让 `UIRender` 反向操作 `CPaintManagerUI` 的消息流程

### 6.2 不要让控件层直接依赖 fallback 细节

控件层最多应该依赖：

- `DrawImage`
- `DrawText`
- `DrawHtmlText`

不应让控件层知道：

- 这次到底走了 GDI+ 还是 FreeType
- 这次是 Direct2D bitmap 还是 legacy 缩放位图

### 6.3 不要为了“全 D2D”而删除有效兼容路径

当前必须保留的兼容路径包括：

- 图片 GDI+ 特殊路径
- FreeType 文本路径
- `HtmlText` legacy parser
- 宿主控件相关截图/导出路径

## 7. Phase A 的后续拆解

### A1：图片链

- 输出图片链决策表
- 明确 `DrawImageInfo -> DrawImage -> TryDrawImageWithDirect2D / fallback` 的职责
- 明确 legacy 缩放位图缓存是“兼容路径”，不是主路径

### A2：普通文本链

- 输出普通文本链决策表
- 明确 `DrawText -> DirectWrite / FreeType / GDI+` 的顺序与原因
- 尽量让 fallback 原因可观测

### A3：`HtmlText` 链

- 输出 `HtmlText` 标签支持矩阵
- 分类列出新路径已支持、部分支持、仍依赖 legacy 的标签
- 明确以后优先迁什么，不优先迁什么

## 8. 当前阶段结论

`UIRender` 现在已经不是“单纯的旧 GDI 绘图文件”，而是 FYUI 新架构里的后端层。

当前最重要的不是继续无差别加功能，而是把以下边界稳住：

- 图片：Direct2D 主路径，GDI+/legacy 缩放为 fallback
- 普通文本：DirectWrite 主路径，FreeType/GDI+ 为 fallback
- `HtmlText`：DirectWrite 主路径，legacy parser 为 fallback

接下来应该围绕这三条链继续收口，而不是再把 manager / control 的职责反向塞回 `UIRender`。
