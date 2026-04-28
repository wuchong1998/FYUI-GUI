# FYUI D2D UIRender Boundary Notes 2026-04

## 本轮确认的边界

- `UIRender.cpp` 不再继续承载整个图片域的所有实现。
- 图片域已经按职责拆分为：
  - `Core/UIRenderImageCodec.cpp`
  - `Core/UIRenderImageEntry.cpp`
  - `Core/UIRenderImageScale.cpp`
  - `Core/UIRenderImageRuntime.cpp`
  - `Core/UIRenderImageSpec.cpp`
  - `Core/UIRenderBitmapUtil.cpp`
  - `Core/UIRenderPrimitive.cpp`
  - `Core/UIRenderClip.cpp`
- `UIRender.cpp` 当前更适合保留：
  - Direct2D / DirectWrite 主路径
  - 需要访问渲染状态缓存的薄分发层
  - 文本与 HtmlText 的主链

## 共享读流层

- 新的共享读流层是 `Core/UIResourceData.cpp`。
- 这层的职责只定义为“把二进制字节读出来”，不吞掉上层语义。
- 当前已经接入：
  - `Core/UIRenderImageCodec.cpp`
  - `Core/UIManager.cpp` 中的 `AddFontArray`
  - `Core/UIResourceManager.cpp`

## XML 语义保留

- `Core/UIMakup.cpp` 已经改成复用共享读流底层能力。
- 但 `UIMakup` 仍然保留 XML 自己的错误语义：
  - `Error opening file`
  - `Could not read file`
  - `Error opening zip file`
  - `Could not find ziped file`
  - `Could not unzip file`
  - `File too large`
- 这意味着共享读流层是“底层能力”，不是“统一错误语义层”。

## legacy HtmlText 当前状态

- `legacy HtmlText` 已经有这些内部 helper：
  - clip/alignment helper
  - line state capture/restore helper
  - line link finalize helper
  - line advance helper
  - paragraph indent helper
- 后续继续收口时，优先方向应当是：
  - 把 `legacy DrawHtmlText` 内部最重的运行时分支继续拆成小 helper
  - 不要继续横向制造过多新 cpp 文件
  - 不要把 manager 或 control 逻辑重新塞回 `UIRender`

## 下一步建议

- 继续收 `legacy HtmlText` 里最重的文本 run 处理块。
- 继续让 `UIResourceData` 保持“只负责取字节”的边界，不扩张成大而全资源系统。
- 在 `Core` 层继续优先做低风险的契约收口，而不是同时推进高风险行为改写。
## Deferred Legacy Branches
- `USE_XIMAGE_EFFECT` 下的 `LoadGifImageX / LoadImage2Memory` 暂不并入共享 `UIResourceData`。
- 当前原因不是功能不需要，而是这条老链仍依赖既有 zip/resource 行为和旧 `UnzipItem` 调用方式。
- 在没有单独隔离和验证这条 legacy gif 分支之前，应优先保持它的行为稳定，而不是为了统一接口去提前改写。
## Removed Legacy Branches
- `USE_XIMAGE_EFFECT` 相关链路已从工程中移除。
- 已删除内容包括 `LoadGifImageX / LoadImage2Memory`、`CxImage` 编译嵌入，以及 `UIGifAnimEx` 控件。
- 后续 gif 动画路径只保留当前已经接入主框架的 `UIGifAnim` 实现，不再维护第二套 ximage 分支。

## 2026-04-22 Text Fallback Split
- `legacy HtmlText` 的执行体已经开始从 `Core/UIRender.cpp` 抽离到 `Core/UIRenderHtmlLegacy.cpp`。
- 当前策略是：
  - `UIRender.cpp` 保留 `DirectWrite` 主入口和最薄的 fallback 调度。
  - `UIRenderHtmlLegacy.cpp` 承接 legacy `HtmlText` 的具体排版与绘制执行。
- 后续继续整理文本域时，优先沿这个边界推进，不再把大块 legacy `HtmlText` 逻辑重新塞回 `UIRender.cpp`。
- 普通文本的 `GDI+` fallback 也已经开始分层，相关实现已独立到 `Core/UIRenderTextLegacy.cpp`。
- 当前文本域推荐边界：
  - `UIRender.cpp`：保留 `DirectWrite` 主链、`FreeType` fallback，以及最薄的 fallback 调度。
  - `UIRenderTextLegacy.cpp`：承接 `GDI+` 文本 fallback 与普通文本 legacy 调度壳。
