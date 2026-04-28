# FYUI HDC Boundary Policy

This note documents the allowed HDC islands while FYUI moves toward the
Direct2D/DirectWrite rendering path.

## Allowed HDC Zones

- `Control/UIRichEdit.*`: the only remaining control-level HDC host island.
- `Core/UIRenderContext.h`: the intentional bridge that owns the current native
  paint handle and exposes it to Core internals.
- `Core/UIRenderSurface.cpp`: render-surface allocation, presentation, and
  offscreen bitmap interop stay inside the implementation boundary.
- Core runtime cpp-local paths: Direct2D batch flush/begin/end,
  render-surface/native interop, clip restoration, and the minimal native
  window/screen DC boundaries.
- Necessary screen/reference DC acquisition for DPI, metrics, print, and native
  window presentation.

## Blocked Areas

- Built non-RichEdit controls must not use `HDC`, `GetDC`, direct DIB/DC drawing,
  or HDC paint overrides.
- Public manager/surface headers must not expose HDC getters.
- `UIRenderBatchInternal.h`, `UIRenderImageLegacyInternal.h`,
  `UIRenderImageRuntimeInternal.h`, `UIRenderPrimitiveInternal.h`,
  `UIRenderTextLegacyInternal.h`, `UIRenderTextSharedInternal.h`, and
  `UIRenderSurfaceInternal.h` must stay `CPaintRenderContext`-based,
  surface-operation based, or pure-helper based.
- ActiveX/Edit/FreeType/`UIRenderLegacyHdc` must not return to the source tree
  or build graph.

## Migration Rule

Prefer moving raw HDC handling inward, not sideways. A new helper should accept
`CPaintRenderContext&` unless it is explicitly part of one of the allowed HDC
fallback/runtime islands above.

## CPP Audit Snapshot

- Must-stay Win32 boundaries: `UIManager.cpp` paint/print/metafile/DPI/native
  child-window bridges, `UIRenderSurface.cpp` allocation/presentation interop,
  `Utils/DPI.cpp` screen DPI probing, and the RichEdit TextServices host island.
- Core runtime/fallback islands: Direct2D DC render target binding and batch
  flush, render-surface/native interop, and clip-region state restoration.
- `UIRenderClip.cpp` remains a clip-region native DC boundary for
  `GetClipBox`, `ExtSelectClipRgn`, and `SelectClipRgn`, but locals and state
  should use explicit `hNativeDC`, `hClipRgn`, and `hPreviousClipRgn` names.
- Primitive drawing is D2D-only for non-RichEdit paths. Do not restore
  `DrawColorFallback`, `DrawGradientFallback`, `DrawLineFallback`,
  `DrawRectFallback`, `DrawRoundRectFallback`, or the old `AlphaBitBlt` HDC
  helper.
- Ordinary image/text/html rendering is D2D/DirectWrite-only. Do not restore
  generic image HDC fallback, `DrawTextFallbackInternal`, or the Html parser
  HDC fallback to the project build graph.
- The old image fallback DC cache (`imageFallbackDC`,
  `GetImageFallbackDCInternal`) has been removed; image runtime paths should
  stay Direct2D/context-based.
- Legacy image/text fallback diagnostics are removed with those fallback paths;
  do not restore `LegacyImageFallback` / `LegacyTextFallback` counters or
  `RecordLegacy*FallbackInternal` helpers.
- Contextized image paths: `UIRenderImageEntry.cpp` and
  `UIRenderImageLegacy.cpp` are now HDC-free wrapper layers; scaled image
  helpers accept `CPaintRenderContext&` and keep DIB allocation details local.
  `UIRenderImageScale.cpp` should use explicit image-scale `NativeDC` names for
  its `CreateDIBSection` boundary.
- `UIRenderImageCodec.cpp` may allocate decoded image DIBs with
  `CreateDIBSection(NULL, ...)`, but that allocation should stay centralized in
  `CreateTopDownImageCodecDibInternal`, with decoded pixels named `pDibBits`.
- Contextized Core runtime wrappers: `UIRenderBatchInternal.cpp` stays
  `CPaintRenderContext`-based; `UIRender.cpp` Direct2D/DirectWrite try-draw
  helpers accept `CPaintRenderContext&` and only unwrap `HDC` inside the
  D2D draw-scope/runtime boundary.
- `D2DDrawScope` and `Begin/Flush/EndDirect2DBatchInternal` accept
  `CPaintRenderContext&`; raw `HDC` is reserved for the internal `BindDC`,
  clip-rect resolution, DPI query, and batch-state runtime boundary.
- Remaining D2D native DC internals should use explicit boundary names such as
  `batchNativeDC`, `ResolveDCRenderTargetBindRect`,
  `BindDCRenderTargetToNativeDC`, and `FlushDirect2DBatchForNativeDC`; avoid
  restoring generic `batchHDC` / `*ForHDC` helper names.
- Image runtime Direct2D attempts are performed from the
  `CPaintRenderContext` overloads; `UIRenderImageRuntime.cpp` is HDC-free in
  built code.
- `UIRenderSurface.cpp` remains a native interop island, but its cpp-local DC
  accessors and locals should use explicit `NativeDC` names rather than generic
  `GetDC` / `hSurfaceDC` / `hTargetDC` helper names.
- `UIRenderHtmlLegacy.cpp`, `UIRenderTextLegacy.cpp`, and
  `UIRenderTextLegacyInternal.h` have been removed from the active tree. Do not
  restore them as unbuilt references or project items.
- `Control/UIActiveX.*` and `Control/UIEdit.*` have also been physically
  removed. Do not keep unbuilt HDC-heavy control implementations as dead source
  files; `UIRichEdit.*` is the only remaining control-level HDC island.
- `UIRenderTextSharedInternal.cpp` may unwrap `CPaintRenderContext` only at the
  font-metrics `GetTextMetrics` boundary. Do not broaden it into Html legacy
  parser state or a new generic HDC text entry point; use explicit
  `hMetricsNativeDC` / `hPreviousFont` local names in that boundary.
- `UIRenderBitmapUtil.cpp` may unwrap `CPaintRenderContext` for generated
  bitmap `SaveDC` / `SetWindowOrgEx` / `CreateDIBSection` interop, but locals
  should use explicit `NativeDC` names rather than generic `hDC` / `hPaintDC`.
- DPI screen DC acquisition is wrapped by `CScopedScreenDC`; avoid adding raw
  local `GetDC(NULL/nullptr)` pairs in `Utils/DPI.cpp`.
- `Control/UIActiveX.cpp` and `Control/UIEdit.cpp` may still exist on disk as
  unbuilt legacy files. They must stay out of the project build graph and must
  not be used as a reason to widen the active HDC surface.

## UIManager.cpp Native Boundaries

- `BeginPaint` / `EndPaint`, `WM_PRINT`, printing/metafile, DPI, and screen
  reference DC acquisition remain native Win32 boundaries.
- The only whitelisted `HDC` parameter helper in `UIManager.cpp` is
  `CreateManagerRenderContext`; it exists solely to bridge native paint/print
  DCs into `CPaintRenderContext`.
- `WM_PRINTCLIENT` may cast `WPARAM` to `HDC` only inside
  `CreatePrintClientRenderContext`.
- Native child-window bitmap cache helpers should stay `CPaintRenderContext`-
  based; raw `HDC` is only used inside the helper bodies for Win32 calls.
- Offscreen and layered surface presentation should go through
  `UIRenderSurfaceInternal.h` surface operations instead of exposing surface DCs.
- Update-rect diagnostics should use `CRenderEngine` / `CPaintRenderContext`,
  not cached GDI pens or direct `Rectangle` calls.
- Font metric HDC details should live in text shared internals; `UIManager.cpp`
  should request metrics through `CPaintRenderContext` helpers.
- Window/screen DC acquisition in `UIManager.cpp` should stay RAII-wrapped by
  `CScopedWindowDC`; do not add new local `HDC h = ::GetDC(...)` pairs.
- UIManager print/metafile/child-window bridges may still unwrap native DCs,
  but locals should use explicit `NativeDC` names rather than generic
  `hDC` / `hTargetDC` / `hdcMem` names.
