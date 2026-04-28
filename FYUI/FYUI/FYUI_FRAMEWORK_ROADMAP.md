# FYUI Framework Improvement Roadmap

This roadmap records the current framework state, known gaps, and the next
work batches for the FYUI Direct2D/DirectWrite migration.

## Current State

- ActiveX/Edit/FreeType and the old text/html HDC fallback files are removed
  from the build graph and guarded against return.
- Built public headers no longer expose generic HDC rendering entry points.
- `UIRichEdit.*` is the only remaining control-level HDC host island.
- Remaining HDC/native DC usage is concentrated in cpp-local boundaries:
  `UIManager.cpp` Win32 paint/print/DPI/metafile boundaries,
  `UIRenderSurface.cpp` native surface interop, `UIRender.cpp` D2D DC render
  target binding, clip/bitmap utility internals, and RichEdit TextServices.
- The render regression guard is part of the MSBuild flow and passes in
  `Debug|x64`.

## Known Gaps

- `UIRender.cpp` is still too broad: D2D runtime state, DirectWrite plain text,
  DirectWrite HTML parsing/layout/caches, bitmap caches, and diagnostics share
  one large implementation file.
- `UIManager.cpp` still owns too many framework responsibilities: message
  handling, paint orchestration, resource/image cache, DPI/font metrics,
  drag/drop, diagnostics, and native window cache.
- GDI+/legacy text compatibility flags still exist as public compatibility
  surface, even though normal text rendering is DirectWrite-only.
- Internal render headers must stay visible to the project and guard rules so
  IDE navigation and regression checks do not miss them.
- Automated validation is mostly build plus guard script; there are no focused
  smoke/snapshot tests for text measurement, HTML link metrics, image scaling,
  DPI, or layered rendering.
- Many older modules still use raw ownership patterns (`new`/`delete`,
  `malloc`/`free`) and should gradually move toward RAII.

## Work Batches

1. Project hygiene and guard coverage.
   Keep all active internal headers in the project/filters, scan source-tree
   files as well as project items, and block dead HDC-heavy source from
   returning as unbuilt files.

2. Legacy text compatibility cleanup.
   Stop internal callers from passing GDI+ text flags into `CRenderEngine`
   text drawing. Keep compatibility wrappers only where needed for external
   source compatibility, and document that DirectWrite is the active path.

3. Split `UIRender.cpp` by stable responsibilities.
   Start with D2D runtime/cache and DirectWrite text/html helpers. Avoid moving
   private types into shared headers unless they are stable and HDC-free.
   The first split is `UIRenderD2DSharedInternal`, which owns pure D2D helper
   conversions and small utility functions used by runtime/cache code.
   `UIRenderD2DCacheTypesInternal` now owns D2D/DirectWrite cache key/value
   structs so future runtime/cache implementation splits do not have to keep
   those type definitions in `UIRender.cpp`.
   DirectWrite cache key construction and shared text color resolution should
   stay in DirectWrite/text shared internals, leaving `UIRender.cpp` focused on
   orchestration and stateful cache use.
   `UIRenderHtmlParseInternal` owns pure HTML parser string/tag helpers, while
   layout, inline image, link rectangle, and cache state stay in `UIRender.cpp`
   until they can be split without exporting private rendering state.
   `UIRenderHtmlMetricsInternal` owns pure DirectWrite hit-test metric rectangle
   helpers used by HTML link/background layout, keeping those geometry utilities
   HDC-free and outside the main render implementation.
   `UIRenderImageD2DInternal` owns pure Direct2D bitmap segment, tiling, and
   rotated bounds helpers plus WIC-to-D2D bitmap conversion details;
   `UIRender.cpp` should keep only factory/cache/state-aware image
   orchestration.
   `UIRenderD2DFrameMetricsInternal` owns frame diagnostics counters and
   reset/consume helpers so `D2DRenderState` can keep metrics as one private
   member without exposing render target/cache state.
   `UIRenderD2DResourceInternal` owns pure brush, stroke, and gradient resource
   creation helpers, D2D resource cache limits, and stroke cache key policy;
   cache lookup, cache eviction, common resource-cache clearing, and render
   state ownership remain in `UIRender.cpp`.
   `UIRenderDirectWriteTextInternal` owns plain text layout/normalization cache
   eligibility and key construction; `UIRender.cpp` should keep only cache
   lookup/writeback and layout object lifetime.
   It also owns HTML DirectWrite layout cache eligibility and key construction
   through primitive/string/signature parameters, while `UIRender.cpp` keeps
   `HtmlParsedText`/style structures and signature calculation private.
   HTML parse cache eligibility and key construction should follow the same
   pattern: helper-owned primitive/string key rules, with `UIRender.cpp`
   retaining parser state, cache lookup/writeback, and parsed value lifetime.
   Cached text layout value construction, cached-metrics fallback, and text/html
   cache size limits are also centralized there; `UIRender.cpp` keeps the maps,
   eviction calls, and frame metric counters.
   COM cache hit/writeback boilerplate for D2D bitmap/resource caches and
   DirectWrite text format/ellipsis caches is shared through `UIRender.cpp`
   cpp-local helpers, keeping ownership private without repeating AddRef/Detach
   patterns.
   Non-COM text/html cache writeback for plain layouts, normalization, HTML
   hit-test metrics, HTML layouts, and HTML parse results is also centralized in
   cpp-local store helpers; parser state and cache maps still stay private to
   `UIRender.cpp`.
   Plain and HTML DirectWrite layout cache hit paths now share a cpp-local hit
   helper for layout retrieval, cached metrics fallback, and frame hit counters.
   HTML foreground/background brush reuse and cached layout drawing-effect
   state decisions are also isolated behind cpp-local helpers, keeping
   `TryDrawHtmlTextWithDirectWrite` focused on render orchestration.
   Primitive Direct2D entry points no longer use an extra private
   `TryDraw*WithDirect2D` trampoline; `Draw*Internal` now owns the actual D2D
   primitive draw body while public `CRenderEngine::Draw*` methods remain the
   stable API boundary in `UIRenderPrimitive.cpp`.
   Image Direct2D runtime entry points follow the same rule:
   `TryDrawBitmapRectWithDirect2DInternal`,
   `TryDrawImageWithDirect2DInternal`, and
   `InvalidateD2DBitmapCacheInternal` own their implementation directly instead
   of delegating through private one-line wrappers.
   Text/html public render APIs now delegate directly to cpp-local
   `DrawTextInternal`, `DrawHtmlTextInternal`, and `GetTextSizeInternal`;
   those internal entries own DirectWrite availability checks and measurement
   behavior while public overloads remain the compatibility boundary.
   The batch header no longer exposes an unused `FlushDirect2DBatchInternal`
   shim; batch begin/end remain hidden behind the RAII scope. `UIRender.cpp`
   has also dropped stale GDI+/Msimg32 include/link dependencies so the main
   runtime file stays focused on Direct2D/DirectWrite state, cache, and native
   DC binding.
   Image entry/legacy callers now use `CanUseImageRenderContextInternal` for
   context validity instead of repeating raw `GetDC()` checks; the remaining
   image-scale native DC touch is isolated to the CreateDIBSection boundary.
   UIManager native-window bitmap cache does the same through
   `CanUseNativeWindowBitmapRenderContext`, keeping that Win32 interop island
   explicit without leaking native DC checks across the render path.
   UIManager paint, print, present, surface-reference fallback, and update-rect
   paths now ask `HasNativeRenderContext` for simple target-context validity;
   the remaining direct native DC access is reserved for creating/deriving
   contexts, SaveDC/RestoreDC, WM_PRINT child-window forwarding, and final
   surface presentation boundaries.
   UIManager native DC boundary helpers now use explicit `NativeDC` naming:
   `CScopedRenderContextNativeDCSave`, scoped window `GetNativeDC()` accessors,
   and `paintNativeDC` for the WM_PRINTCLIENT bridge. Guard rules block the old
   generic `DCSave`/`GetDC` accessor spelling from returning in these scoped
   Win32 helper areas.
   Direct `CPaintRenderContext::GetDC()` access inside UIManager is now
   centralized in `GetRenderContextNativeDC`; native context creation/derivation
   helpers are named `CreateManagerNativeRenderContext`,
   `CreatePrintClientNativeRenderContext`, and
   `CreateDerivedNativeRenderContext` to keep bridge semantics explicit.
   Drag/drop enhanced-metafile handling now uses cpp-local RAII wrappers for
   compatible memory DCs, enhanced metafile DCs, selected GDI objects, and
   temporary GDI object ownership. The Win32/metafile island remains allowed,
   but manual `CreateCompatibleDC`/`DeleteDC`,
   `CreateEnhMetaFile`/`CloseEnhMetaFile`, and `SelectObject` restore skeletons
   are guarded against returning at call sites.
   Drag/drop `CIDropSource` and `CIDataObject` lifetimes are also scoped through
   `CScopedComObjectRef`, replacing local `pdsrc`/`pdobj` AddRef/Release/delete
   branches in UIManager's OLE drag source path.
   Drop target `CF_DIB` and `CF_HDROP` HGLOBAL reads now use
   `CScopedGlobalLock`, removing manual `GlobalLock`/`GlobalUnlock` pairs from
   OnDrop call sites.
   Drop target bitmap replacement is now centralized in
   `ReplaceNativeStaticBitmapImage`; all `STM_SETIMAGE` ownership transfer and
   previous-bitmap release goes through one native static-control boundary.
   `DragDropImpl.cpp` also has tighter `STGMEDIUM` hygiene: `GetData` zeros the
   full output medium, `Drop` value-initializes the temporary medium,
   `SetData` no longer allocates per-entry heap nodes, and `TYMED_FILE`
   duplication writes to the destination medium.
   `CIDataObject` now stores `FORMATETC` and `STGMEDIUM` as value arrays rather
   than heap node arrays, so OLE medium lifetime is released directly from the
   owned `STGMEDIUM` values and the old pointer-vector typedefs are guarded
   against returning.

4. Split `UIManager.cpp` by framework services.
   Prefer extracting resource/image cache, DPI/font metric helpers, and paint
   frame orchestration before touching message-loop or Win32 boundary code.

5. Add lightweight validation.
   Extend guard checks first, then add small render/text/image smoke tests that
   can run after `Debug|x64` builds.

6. Gradual RAII cleanup.
   Target local ownership hotspots in `UIManager.cpp`, `UIMakup.cpp`,
   `UIGifAnim.cpp`, and non-RichEdit controls after render boundaries are
   stable.
