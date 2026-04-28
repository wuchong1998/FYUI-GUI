# FYUI Adaptive Layout Roadmap

## Goal

在保持 FYUI 现有 XML、控件类型和使用习惯基本不变的前提下，逐步增强 `Layout` 层，让 FYUI 在窗口尺寸变化、DPI 变化和不同内容密度下具备更现代的自适应布局能力。

## Guiding Rules

- 不把“响应式布局”直接混进当前 Direct2D/DirectWrite 渲染重构。
- 先增强 `Layout` 层能力，再考虑引入窗口宽度状态切换。
- 新能力优先做成可选属性，不破坏现有布局语义。
- 继续复用 FYUI 现有 `EstimateSize / SetPos / ChildPadding / Inset / ScrollBar` 模型。

## Phase 1: Low-Risk Layout Enhancements

- 为 `Vertical / Horizontal / Tile / Tab / Child` 统一更多的测量前置与排列前置 helper。
- 增加更明确的最小/最大尺寸组合语义。
- 补齐布局层的公共内容区域准备、padding 收缩、scrollbar 收缩 helper。
- 为后续百分比尺寸和断点规则准备共享内部工具层。

## Phase 2: Optional Adaptive Size Rules

- 支持可选的百分比宽高。
- 支持更明确的 `min / max / preferred` 三段式尺寸表达。
- 支持“内容驱动尺寸”和“剩余空间分配”的混合策略。
- 继续保持老 XML 能直接运行，新属性不影响旧布局。

## Phase 3: Adaptive State Switching

- 参考 WinUI 的思路，但不照搬实现。
- 增加基于窗口宽度或高度的状态切换规则。
- 允许同一布局在不同宽度下切换：
  - 横向排列/纵向排列
  - 列数
  - 控件显隐
  - padding / spacing

## Phase 4: Layout Diagnostics

- 增加布局测量与排列诊断输出。
- 统计布局重算次数、控件 `EstimateSize` 热点、反复布局区域。
- 让布局优化也像渲染优化一样可观测，而不是只靠经验调整。

## Entry Files

- `D:\work\FYUI\FYUI\FYUI\Layout\UIVerticalLayout.cpp`
- `D:\work\FYUI\FYUI\FYUI\Layout\UIHorizontalLayout.cpp`
- `D:\work\FYUI\FYUI\FYUI\Layout\UITileLayout.cpp`
- `D:\work\FYUI\FYUI\FYUI\Layout\UITabLayout.cpp`
- `D:\work\FYUI\FYUI\FYUI\Layout\UIChildLayout.cpp`

## Current Recommendation

当前最合适的推进顺序是：

1. 继续做 `Layout` 内部公共 helper 收口。
2. 继续做 `UIList / UIListEx` 的编辑与事件策略清理。
3. 等布局内部结构更干净后，再单独引入“可选响应式规则”。
