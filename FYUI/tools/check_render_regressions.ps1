param(
    [string]$ProjectFile = (Join-Path $PSScriptRoot '..\FYUI\FYUI.vcxproj'),
    [string]$ProjectRoot = ''
)

$ErrorActionPreference = 'Stop'

if ([string]::IsNullOrWhiteSpace($ProjectRoot)) {
    $ProjectRoot = Split-Path -Parent $ProjectFile
}

if (Test-Path -LiteralPath $ProjectFile) {
    $ProjectFile = (Resolve-Path -LiteralPath $ProjectFile).Path
}
if (Test-Path -LiteralPath $ProjectRoot) {
    $ProjectRoot = (Resolve-Path -LiteralPath $ProjectRoot).Path
}

$failures = New-Object System.Collections.Generic.List[string]

function Add-Failure {
    param([string]$Message)
    $failures.Add($Message) | Out-Null
}

function Get-LineNumber {
    param(
        [string]$Text,
        [int]$Index
    )

    if ($Index -le 0) {
        return 1
    }

    $line = 1
    for ($i = 0; $i -lt $Index -and $i -lt $Text.Length; ++$i) {
        if ($Text[$i] -eq "`n") {
            ++$line
        }
    }
    return $line
}

if (!(Test-Path -LiteralPath $ProjectFile)) {
    Add-Failure "Project file not found: $ProjectFile"
}
else {
    $projectText = Get-Content -LiteralPath $ProjectFile -Raw

    $projectBans = @(
        @{ Pattern = '(?i)Control\\UIActiveX\.(cpp|h)'; Message = 'UIActiveX must stay out of the project build graph.' },
        @{ Pattern = '(?i)Control\\UIEdit\.(cpp|h)'; Message = 'UIEdit must stay out of the project build graph.' },
        @{ Pattern = '(?i)Core\\UIRenderHtmlLegacy\.cpp'; Message = 'Html HDC legacy fallback must stay out of the project build graph.' },
        @{ Pattern = '(?i)Core\\UIRenderTextLegacy\.(cpp|h)'; Message = 'Text HDC legacy fallback must stay out of the project build graph.' },
        @{ Pattern = '(?i)UIRenderLegacyHdc\.(cpp|h)'; Message = 'UIRenderLegacyHdc must not return to the project build graph.' },
        @{ Pattern = '(?i)\bfreetype\b|FreeType'; Message = 'FreeType must not return to project configuration.' }
    )

    foreach ($ban in $projectBans) {
        $match = [regex]::Match($projectText, $ban.Pattern)
        if ($match.Success) {
            Add-Failure "$($ban.Message) ($($ProjectFile):$((Get-LineNumber $projectText $match.Index)))"
        }
    }

    [xml]$projectXml = $projectText
    $ns = New-Object System.Xml.XmlNamespaceManager($projectXml.NameTable)
    $ns.AddNamespace('msb', 'http://schemas.microsoft.com/developer/msbuild/2003')
    $nodes = $projectXml.SelectNodes('//msb:ClCompile|//msb:ClInclude', $ns)
    $projectItems = @()
    foreach ($node in $nodes) {
        $include = $node.Include
        if ([string]::IsNullOrWhiteSpace($include)) {
            continue
        }
        $fullPath = Join-Path $ProjectRoot $include
        if (Test-Path -LiteralPath $fullPath) {
            $projectItems += [pscustomobject]@{
                Include = $include
                Path = $fullPath
            }
        }
    }

    $sourceItems = @()
    Get-ChildItem -LiteralPath $ProjectRoot -Recurse -File | Where-Object {
        $_.Extension -match '(?i)^\.(cpp|h)$'
    } | ForEach-Object {
        if ($_.FullName -notmatch '(?i)\\(x64|Debug|Release)\\' -and $_.FullName -notmatch '(?i)\\Utils\\stb_image\.h$') {
            $relativePath = $_.FullName.Substring($ProjectRoot.Length + 1)
            $sourceItems += [pscustomobject]@{
                Include = $relativePath
                Path = $_.FullName
            }
        }
    }

    $projectItemIncludes = @{}
    foreach ($item in $projectItems) {
        $projectItemIncludes[$item.Include.ToLowerInvariant()] = $true
    }

    $requiredProjectIncludes = @(
        'Core\UIRenderBatchInternal.h',
        'Core\UIRenderD2DCacheTypesInternal.h',
        'Core\UIRenderD2DFrameMetricsInternal.h',
        'Core\UIRenderD2DResourceInternal.h',
        'Core\UIRenderD2DSharedInternal.h',
        'Core\UIRenderHtmlMetricsInternal.h',
        'Core\UIRenderHtmlParseInternal.h',
        'Core\UIRenderImageD2DInternal.h',
        'Core\UIRenderImageLegacyInternal.h',
        'Core\UIRenderPrimitiveInternal.h',
        'Core\UIRenderSurfaceInternal.h'
    )
    foreach ($include in $requiredProjectIncludes) {
        if (!$projectItemIncludes.ContainsKey($include.ToLowerInvariant())) {
            Add-Failure "Active internal render header must stay in the project file for IDE/guard coverage. ($($include):1)"
        }
    }

    $sourceBans = @(
        @{ Pattern = '(?i)#\s*include\s*[<"][^">]*UIRenderLegacyHdc'; Message = 'Do not include UIRenderLegacyHdc from built code.' },
        @{ Pattern = '(?i)\bFreeTypeDrawText\b|\bfreetype\b'; Message = 'FreeType text fallback must not return.' },
        @{ Pattern = '(?i)\bCRenderEngine\s*::\s*(DrawText|DrawHtmlText|GetTextSize)\s*\(\s*HDC\b'; Message = 'Generic CRenderEngine HDC text entry point must not return.' },
        @{ Pattern = '(?i)\bstatic\s+(void|SIZE)\s+(DrawText|DrawHtmlText|GetTextSize)\s*\(\s*HDC\b'; Message = 'Generic static HDC text entry point must not return.' },
        @{ Pattern = '(?i)\bRecordLegacy(?:Image|Text)FallbackInternal\b'; Message = 'Legacy image/text fallback diagnostic recorders must not return.' },
        @{ Pattern = '(?i)\b(?:frame|n|m_n(?:Sample)?)Legacy(?:Image|Text)Fallback(?:Count|Draws)\b'; Message = 'Legacy image/text fallback diagnostic counters must not return.' },
        @{ Pattern = '(?im)^\s*(?:bool\s+DrawImage|CPaintRenderContext\s+CreateCompatPaintRenderContext)\s*\(\s*HDC\b'; Message = 'CControlUI HDC image/compat helper must not return.' },
        @{ Pattern = '(?i)\bCControlUI\s*::\s*(DrawImage|CreateCompatPaintRenderContext)\s*\(\s*HDC\b'; Message = 'CControlUI HDC image/compat helper implementation must not return.' },
        @{ Pattern = '(?i)\bCreateControlCompatRenderContext\s*\('; Message = 'Control-scoped HDC compat render context helper must not return.' },
        @{ Pattern = '(?i)\bCreateCompatRenderContext\s*\('; Message = 'Generic HDC compat render context helper must not return.' },
        @{ Pattern = '(?i)\bHDC\s+CPaintManagerUI\s*::\s*Get(Paint|Measure)DC\s*\(|\bHDC\s+Get(Paint|Measure)DC\s*\(\s*\)\s*const\s*;'; Message = 'CPaintManagerUI HDC getters must not return; use CPaintRenderContext.' },
        @{ Pattern = '(?i)->\s*Get(Paint|Measure)DC\s*\('; Message = 'Do not call CPaintManagerUI HDC getters; use CPaintRenderContext.' },
        @{ Pattern = '(?s)\bCRenderEngine\s*::\s*DrawText\s*\([^;{}]*\bGetGdiPlusDrawText\s*\('; Message = 'Internal text drawing must not pass the legacy GDI+ text flag; DirectWrite is the active path.' }
    )

    foreach ($item in $sourceItems) {
        $text = Get-Content -LiteralPath $item.Path -Raw
        foreach ($ban in $sourceBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    $removedFallbackFiles = @(
        @{ Include = 'Control\UIActiveX.cpp'; Message = 'Removed ActiveX control implementation must not return.' },
        @{ Include = 'Control\UIActiveX.h'; Message = 'Removed ActiveX control header must not return.' },
        @{ Include = 'Control\UIEdit.cpp'; Message = 'Removed Edit control implementation must not return.' },
        @{ Include = 'Control\UIEdit.h'; Message = 'Removed Edit control header must not return.' },
        @{ Include = 'Core\UIRenderHtmlLegacy.cpp'; Message = 'Removed Html HDC fallback implementation must not return.' },
        @{ Include = 'Core\UIRenderTextLegacy.cpp'; Message = 'Removed text HDC fallback implementation must not return.' },
        @{ Include = 'Core\UIRenderTextLegacyInternal.h'; Message = 'Removed text HDC fallback internal header must not return.' }
    )
    foreach ($file in $removedFallbackFiles) {
        $path = Join-Path $ProjectRoot $file.Include
        if (Test-Path -LiteralPath $path) {
            Add-Failure "$($file.Message) ($($file.Include):1)"
        }
    }

    $colorPaletteBans = @(
        @{ Pattern = '(?i)\bHDC\b'; Message = 'UIColorPalette must stay on CPaintRenderContext/CPaintRenderSurface, not HDC.' },
        @{ Pattern = '(?i)\b(CreateCompatibleDC|CreateDIBSection|SaveDC|RestoreDC|StretchBlt|SetStretchBltMode|SelectObject)\b'; Message = 'UIColorPalette must not restore direct DC/DIB drawing.' },
        @{ Pattern = '(?i)\b(m_MemDc|m_hMemBitmap|m_bmInfo|m_pBits)\b'; Message = 'UIColorPalette must not restore legacy HDC bitmap-cache fields.' }
    )

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Control\\UIColorPalette\.(h|cpp)$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        foreach ($ban in $colorPaletteBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    $controlHdcPattern = '(?i)\bHDC\b|\.GetDC\s*\(|\b(CreateCompatibleDC|CreateDIBSection|SaveDC|RestoreDC|StretchBlt|SetStretchBltMode|SelectObject)\b'
    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Control\\') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $match = [regex]::Match($text, $controlHdcPattern)
        if ($match.Success) {
            Add-Failure "Built controls must not access HDC/DC directly; use CPaintRenderContext/CPaintRenderSurface. ($($item.Include):$((Get-LineNumber $text $match.Index)))"
        }
    }

    $surfaceClientBans = @(
        @{
            IncludePattern = '(?i)^Utils\\UIShadow\.cpp$'
            Pattern = '(?i)\bHDC\b|\.GetDC\s*\(|\bUpdateLayeredWindow\s*\('
            Message = 'UIShadow must present through CPaintRenderSurface instead of taking a surface HDC.'
        },
        @{
            IncludePattern = '(?i)^TooltipWnd\.cpp$'
            Pattern = '(?i)\bHDC\b|\bGetMeasureDC\s*\(|\bSelectObject\s*\(|\bDrawTextEx\s*\('
            Message = 'TooltipWnd measurement must stay on CPaintRenderContext/CRenderEngine.'
        }
    )

    foreach ($item in $projectItems) {
        foreach ($ban in $surfaceClientBans) {
            if ($item.Include -notmatch $ban.IncludePattern) {
                continue
            }

            $text = Get-Content -LiteralPath $item.Path -Raw
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    $contextizedImageCppBans = @(
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageEntry\.cpp$'
            Pattern = '(?i)\bHDC\b|\.GetDC\s*\('
            Message = 'Image entry cpp must stay CPaintRenderContext/helper-based; raw/native DC checks belong in shared image internals.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageLegacy\.cpp$'
            Pattern = '(?i)\bHDC\b|\.GetDC\s*\('
            Message = 'Image legacy entry cpp must stay CPaintRenderContext/helper-based; raw/native DC checks belong in shared image internals.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageScale\.cpp$'
            Pattern = '(?im)^\s*HBITMAP\s+CreateARGB32Bitmap\s*\(\s*HDC\b'
            Message = 'Image scale bitmap allocation wrapper must accept CPaintRenderContext, not raw HDC.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageScale\.cpp$'
            Pattern = '\bhDC\b|\bCreateARGB32Bitmap\s*\('
            Message = 'Image scale native DC locals/helpers must use explicit NativeDC/internal names.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageScale\.cpp$'
            Pattern = '(?i)renderContext\.GetDC\s*\(\s*\)\s*==\s*(?:NULL|nullptr)'
            Message = 'Image scale callers should ask CanUseImageRenderContextInternal instead of repeating raw native DC validity checks.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageScale\.cpp$'
            Pattern = '(?im)^\s*HBITMAP\s+ResolveLegacyScaledBitmap\s*\(\s*HDC\b'
            Message = 'Image scale resolver must accept CPaintRenderContext, not raw HDC.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageRuntime\.cpp$'
            Pattern = '(?i)\bHDC\b'
            Message = 'Image runtime cpp must stay D2D/context-only; image HDC fallback is blocked.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderHtmlLegacy\.cpp$'
            Pattern = '(?i)\bDrawImageInternal\s*\(\s*HDC\b|\bDrawImageInternal\s*\(\s*hDC\b'
            Message = 'Html legacy inline images must call the explicitly named DrawImageLegacyFallbackInternal HDC island.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImage(?:DispatchInternal\.h|Legacy\.cpp)$'
            Pattern = '(?i)\bDrawResolvedGdiplusImagePath\b'
            Message = 'Gdiplus-loaded images must stay on the explicitly named Direct2D bitmap path, not an ambiguous legacy image path.'
        }
    )

    foreach ($item in $projectItems) {
        foreach ($ban in $contextizedImageCppBans) {
            if ($item.Include -notmatch $ban.IncludePattern) {
                continue
            }

            $text = Get-Content -LiteralPath $item.Path -Raw
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderImageCodec\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $pDestMatch = [regex]::Match($text, '\bpDest\b')
        if ($pDestMatch.Success) {
            Add-Failure "Image codec DIB bits should use explicit pDibBits naming. ($($item.Include):$((Get-LineNumber $text $pDestMatch.Index)))"
        }

        $dibMatches = [regex]::Matches($text, '(?i)CreateDIBSection\s*\(')
        if ($dibMatches.Count -ne 1) {
            $line = if ($dibMatches.Count -gt 0) { Get-LineNumber $text $dibMatches[0].Index } else { 1 }
            Add-Failure "Image codec DIB allocation must stay centralized in CreateTopDownImageCodecDibInternal. ($($item.Include):$line)"
        } elseif ($text -notmatch '(?s)HBITMAP\s+CreateTopDownImageCodecDibInternal\s*\([^)]*\)\s*\{.*?CreateDIBSection\s*\(') {
            Add-Failure "Image codec DIB allocation must stay inside CreateTopDownImageCodecDibInternal. ($($item.Include):$((Get-LineNumber $text $dibMatches[0].Index)))"
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Utils\\DPI\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        if ($text -notmatch '(?s)class\s+CScopedScreenDC\b') {
            Add-Failure "DPI screen DC acquisition must stay wrapped by CScopedScreenDC. ($($item.Include):1)"
        }

        $dpiRawScreenDcBans = @(
            @{ Pattern = '(?im)^\s*(?:const\s+)?HDC\s+\w+\s*=\s*::GetDC\s*\(\s*(?:NULL|nullptr)\s*\)'; Message = 'DPI.cpp must not reintroduce raw local screen GetDC; use CScopedScreenDC.' },
            @{ Pattern = '(?im)^\s*auto\s+\w+\s*=\s*::GetDC\s*\(\s*(?:NULL|nullptr)\s*\)'; Message = 'DPI.cpp must not reintroduce raw local screen GetDC; use CScopedScreenDC.' }
        )
        foreach ($ban in $dpiRawScreenDcBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderBatchInternal\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $match = [regex]::Match($text, '(?i)\bHDC\b')
        if ($match.Success) {
            Add-Failure "Direct2D batch scope cpp must stay CPaintRenderContext-based; raw HDC batch functions belong in UIRender.cpp runtime internals. ($($item.Include):$((Get-LineNumber $text $match.Index)))"
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderPrimitive\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $primitiveCppBans = @(
            @{ Pattern = '(?i)\bHDC\b|\.GetDC\s*\('; Message = 'Primitive runtime cpp must stay D2D/context-only; primitive HDC drawing is blocked.' },
            @{ Pattern = '(?i)\bDraw(?:Color|Gradient|Line|Rect|RoundRect)Fallback\b'; Message = 'Primitive GDI/GDI+ fallback helpers must not return; primitives draw through D2D only.' },
            @{ Pattern = '(?i)\b(GdiplusDrawRoundRectInternal|GradientFill|MoveToEx|LineTo|::\s*Rectangle|::\s*RoundRect)\b'; Message = 'Primitive HDC/GDI drawing calls must not return.' }
        )
        foreach ($ban in $primitiveCppBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderClip\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $clipCppBans = @(
            @{ Pattern = '\bhDC\b|(?m)^\s*HDC\s+hDC\b'; Message = 'Clip runtime native DC locals must use hNativeDC, not hDC.' },
            @{ Pattern = '\bhRgn\b|\bhOldRgn\b|\bhRgnItem\b|->hRgn\b|->hOldRgn\b|\.hRgn\b|\.hOldRgn\b'; Message = 'Clip runtime region fields/locals must use explicit clip-region names.' }
        )
        foreach ($ban in $clipCppBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderFallbackSharedInternal\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $fallbackSharedBans = @(
            @{ Pattern = '(?i)\bHDC\b'; Message = 'Fallback shared cpp must stay HDC-free after text/html/image fallback removal.' },
            @{ Pattern = '(?i)\bBegin(?:Image|Text)FallbackInternal\s*\(\s*renderContext\.GetDC\s*\('; Message = 'Fallback shared context entry points must not forward through the raw HDC overload.' },
            @{ Pattern = '(?i)\bFlushDirect2DBatchInternal\s*\(\s*renderContext\.GetDC\s*\('; Message = 'Fallback shared context entry points must flush through the CPaintRenderContext overload.' }
        )
        foreach ($ban in $fallbackSharedBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRender(BitmapUtil|Surface)\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $bitmapSurfaceBans = @(
            @{ Pattern = '(?im)^\s*HBITMAP\s+CreateARGB32BitmapInternal\s*\(\s*HDC\b'; Message = 'ARGB bitmap creation helper must accept CPaintRenderContext, not raw HDC.' },
            @{ Pattern = '(?i)\bEnsureForHdc\b'; Message = 'Render surface allocation helper must stay CPaintRenderContext-based, not HDC-named.' },
            @{ Pattern = '(?i)\bCPaintRenderSurfaceInternalAccess\s*::\s*GetDC\b|\bCScopedLayeredWindowPresentDC\b|\b(?:Acquire|Release)LayeredWindowPresentDC\b'; Message = 'Render surface native DC internals must use explicit NativeDC names.' },
            @{ Pattern = '(?i)\b(hSurfaceDC|hTargetDC|hPresentDC|hFallbackTargetDC)\b|(?:m_state|state|surface\.m_state)->hDC\b'; Message = 'Render surface local/state DC variables must use explicit NativeDC names.' },
            @{ Pattern = '\b(hPaintDC|hDC)\b'; Message = 'Bitmap/surface native DC locals must use explicit NativeDC names.' }
        )
        foreach ($ban in $bitmapSurfaceBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderTextSharedInternal\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $textSharedBans = @(
            @{ Pattern = '(?i)\bUpdateTextFallbackFontMetricsInternal\s*\(\s*HDC\b'; Message = 'Text fallback font metrics must stay CPaintRenderContext-based.' },
            @{ Pattern = '(?i)\bInitializeHtmlTextFallbackDrawStateInternal\s*\(\s*CPaintRenderContext\b'; Message = 'Html legacy parser draw state must not be moved into text shared internals as a broad context wrapper.' },
            @{ Pattern = '\bhDC\b|\bhOldFont\b'; Message = 'Text shared font-metrics native DC locals must use explicit NativeDC/previous-font names.' }
        )
        foreach ($ban in $textSharedBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRender\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $renderCppContextizedBans = @(
            @{ Pattern = '(?i)\bTryDraw(?:BitmapRect|Image|Color|Gradient|Line|Rect|RoundRect)WithDirect2D\s*\(\s*HDC\b'; Message = 'UIRender.cpp Direct2D try-draw helpers must accept CPaintRenderContext; raw HDC belongs in D2D draw scope/runtime internals.' },
            @{ Pattern = '(?i)\bTryDraw(?:Text|HtmlText)WithDirectWrite\s*\(\s*HDC\b'; Message = 'UIRender.cpp DirectWrite try-draw helpers must accept CPaintRenderContext; raw HDC belongs inside the draw scope.' },
            @{ Pattern = '(?i)\bTryDraw(?:BitmapRect|Image)WithDirect2DInternal\s*\(\s*HDC\b'; Message = 'Image Direct2D internal wrappers must stay CPaintRenderContext-based.' },
            @{ Pattern = '(?i)\bTryDraw(?:BitmapRect|Image)WithDirect2D\s*\('; Message = 'Image Direct2D internals should not reintroduce a trivial TryDraw wrapper layer in UIRender.cpp; keep the body in the Internal entry point.' },
            @{ Pattern = '(?i)\bInvalidateCachedD2DBitmap\b'; Message = 'D2D bitmap cache invalidation should stay in InvalidateD2DBitmapCacheInternal, not behind a trivial wrapper.' },
            @{ Pattern = '(?i)\bTryDraw(?:Text|HtmlText)WithDirectWrite\b|\bTryMeasureTextWithDirectWrite\b'; Message = 'Text/html DirectWrite main entries should stay in DrawTextInternal/DrawHtmlTextInternal/GetTextSizeInternal, not behind trivial Try* wrappers.' },
            @{ Pattern = '(?i)#\s*include\s*<gdiplus\.h>|using\s+namespace\s+Gdiplus|#pragma\s+comment\s*\(\s*lib\s*,\s*"(?:GdiPlus|gdiplus|Msimg32)\.lib"'; Message = 'UIRender.cpp must stay Direct2D/DirectWrite-focused; GDI+/Msimg32 dependencies belong in image codec/surface islands.' },
            @{ Pattern = '(?i)!CanUseDirect2D\s*\(\s*\)\s*\|\|\s*renderContext\.GetDC\s*\(\s*\)\s*==\s*(?:NULL|nullptr)'; Message = 'UIRender.cpp try-draw/context checks should use CanUseDirect2DRenderContext.' },
            @{ Pattern = '(?i)\bD2DDrawScope\s*\(\s*HDC\b'; Message = 'D2D draw scope must accept CPaintRenderContext; raw HDC is only unwrapped inside the scope.' },
            @{ Pattern = '(?i)\b(?:Begin|Flush|End)Direct2DBatchInternal\s*\(\s*HDC\b'; Message = 'Direct2D batch internal public shims must accept CPaintRenderContext, not raw HDC.' },
            @{ Pattern = '(?i)\b(batchHDC|GetBindRect|PrepareDCRenderTargetForDC|FlushDirect2DBatchForHDC)\b'; Message = 'D2D native DC runtime helpers must use explicit NativeDC/BindDC boundary names.' },
            @{ Pattern = '(?i)\bDraw(?:Color|Gradient|Line|Rect|RoundRect)Fallback\s*\('; Message = 'Primitive HDC fallback calls must not return to UIRender.cpp.' },
            @{ Pattern = '(?i)\bTryDraw(?:Color|Gradient|Line|Rect|RoundRect)WithDirect2D\b'; Message = 'Primitive Direct2D draw internals should not reintroduce a trivial TryDraw wrapper layer in UIRender.cpp.' },
            @{ Pattern = '(?i)\b(AlphaBitBlt|PixelAlpha)\b'; Message = 'Unused HDC alpha blit helpers must not return; use the D2D/image runtime path.' },
            @{ Pattern = '(?i)\bDrawTextFallbackInternal\s*\(|\bDrawHtmlTextFallbackInternal\s*\('; Message = 'CRenderEngine text/html must stay DirectWrite-only; HDC text/html fallback is blocked.' },
            @{ Pattern = '(?i)\b(imageFallbackDC|GetCachedImageFallbackDC|GetImageFallbackDCInternal)\b'; Message = 'Image HDC fallback DC cache must not return to UIRender.cpp.' },
            @{ Pattern = '(?i)\bGetTextExtentPoint32\s*\(|::\s*TextOut\s*\('; Message = 'Generic text measurement/draw HDC calls must not return to UIRender.cpp.' },
            @{ Pattern = '(?im)^\s*(?:DWORD\s+NormalizeRenderableColor|void\s+SetBitmapAlphaChannel|LPVOID\s+IntValueToPtr|int\s+PtrToIntValue|LPVOID\s+DwordValueToPtr|DWORD\s+PtrToDwordValue|D2D1_COLOR_F\s+ToD2DColor|D2D1_RECT_F\s+ToD2DRectF|D2D1_RECT_F\s+ToInsetD2DRectF|D2D1_DASH_STYLE\s+ToD2DDashStyle|D2D1_RENDER_TARGET_TYPE\s+ToD2DRenderTargetType|bool\s+HasVisibleIntersection)\s*\('; Message = 'Shared D2D utility helpers belong in UIRenderD2DSharedInternal, not UIRender.cpp.' },
            @{ Pattern = '(?im)^\s*struct\s+(?:D2DBitmapCacheKey|D2DTextFormatCacheKey|D2DTextLayoutCacheKey|PlainTextNormalizationCacheKey|HtmlParseCacheKey|HtmlTextLayoutCacheKey|HtmlDrawingEffectState|HtmlHitTestMetricsCacheKey|CachedTextLayoutValue)\b'; Message = 'D2D/DirectWrite cache key/value types belong in UIRenderD2DCacheTypesInternal, not UIRender.cpp.' },
            @{ Pattern = '(?im)^\s*UINT\s+frame(?:Batch|Standalone|Text|Html)[A-Za-z0-9_]*\s*='; Message = 'Direct2D frame metric fields belong in UIRenderD2DFrameMetricsInternal, not directly in D2DRenderState.' },
            @{ Pattern = '(?i)\b(?:CreateSolidColorBrush|CreateStrokeStyle|CreateGradientStopCollection)\b'; Message = 'Pure D2D resource creation details belong in UIRenderD2DResourceInternal, not UIRender.cpp.' },
            @{ Pattern = '(?i)\bPS_STYLE_MASK\b'; Message = 'D2D stroke style cache key policy belongs in UIRenderD2DResourceInternal, not UIRender.cpp.' },
            @{ Pattern = '(?is)\b(?:bitmapCache|brushCache|strokeStyleCache|gradientStopCollectionCache)\.size\s*\(\s*\)\s*>=\s*\d+'; Message = 'D2D resource cache limits must be centralized in UIRenderD2DResourceInternal.' },
            @{ Pattern = '(?i)\(\*pp(?:Bitmap|TextFormat|Ellipsis|Brush|StrokeStyle|StopCollection)\)->AddRef\s*\('; Message = 'COM cache hit return skeleton belongs in TryReturnCachedComObject.' },
            @{ Pattern = '(?i)\*pp(?:Bitmap|TextFormat|Ellipsis|Brush|StrokeStyle|StopCollection)\s*=\s*\w+\.Detach\s*\('; Message = 'COM cache writeback return skeleton belongs in StoreCachedComObject.' },
            @{ Pattern = '(?im)^\s*(?:D2DTextFormatCacheKey\s+BuildTextFormatCacheKey|DWORD\s+ResolveTextColor)\s*\('; Message = 'Stable DirectWrite/text shared helpers must stay out of UIRender.cpp.' },
            @{ Pattern = '(?im)^\s*(?:bool\s+CanCache(?:DirectWriteTextLayout|PlainTextNormalization)|D2DTextLayoutCacheKey\s+BuildDirectWriteTextLayoutCacheKey|PlainTextNormalizationCacheKey\s+BuildPlainTextNormalizationCacheKey)\s*\('; Message = 'Plain DirectWrite text layout/normalization cache policy helpers belong in UIRenderDirectWriteTextInternal, not UIRender.cpp.' },
            @{ Pattern = '(?is)\bD2DTextLayoutCacheKey\s+layoutKey\s*;.{0,900}\blayoutKey\.(?:formatKey|text|maxWidth100|maxHeight100|endEllipsis|underline|strikeout|noPrefix)\s*='; Message = 'Plain DirectWrite text layout cache key assembly must stay in UIRenderDirectWriteTextInternal; UIRender.cpp should own only cache lookup/writeback.' },
            @{ Pattern = '(?is)\bPlainTextNormalizationCacheKey\s+cacheKey\s*;.{0,500}\bcacheKey\.(?:text|noPrefix)\s*='; Message = 'Plain text normalization cache key assembly must stay in UIRenderDirectWriteTextInternal; UIRender.cpp should own only cache lookup/writeback.' },
            @{ Pattern = '(?im)^\s*(?:CachedTextLayoutValue\s+cachedValue|cachedValue\.(?:layout|metrics|hasMetrics)\s*=)'; Message = 'CachedTextLayoutValue construction belongs in UIRenderDirectWriteTextInternal; UIRender.cpp should only write cache entries.' },
            @{ Pattern = '(?is)\b(?:textFormatCache|textEllipsisCache|textLayoutCache|plainTextNormalizationCache|htmlParseCache|htmlTextLayoutCache|htmlHitTestMetricsCache)\.size\s*\(\s*\)\s*>=\s*\d+'; Message = 'DirectWrite text/html cache limits must be centralized in UIRenderDirectWriteTextInternal.' },
            @{ Pattern = '(?is)\bstate\.(?:textLayoutCache|plainTextNormalizationCache|htmlParseCache|htmlHitTestMetricsCache)\.size\s*\(\s*\)\s*>='; Message = 'Text/html non-COM cache eviction belongs in StoreCachedValue-backed helpers.' },
            @{ Pattern = '(?i)\bstate\.(?:textLayoutCache|plainTextNormalizationCache|htmlParseCache|htmlHitTestMetricsCache)\[[^\]]+\]\s*='; Message = 'Text/html non-COM cache writeback belongs in StoreCachedValue-backed helpers.' },
            @{ Pattern = '(?i)\bstate\.(?:textLayoutCache|htmlTextLayoutCache)\.find\s*\('; Message = 'DirectWrite layout cache hit lookup belongs in TryReturnCachedDirectWriteTextLayout.' },
            @{ Pattern = '(?i)\bstate\.htmlDrawingEffectStates\.find\s*\('; Message = 'HTML drawing-effect state lookup belongs in ResolveHtmlDrawingEffectPlan.' },
            @{ Pattern = '(?im)^\s*bool\s+CanCacheHtmlTextLayout\s*\('; Message = 'HTML DirectWrite layout cache eligibility belongs in UIRenderDirectWriteTextInternal; keep HtmlParsedText private in UIRender.cpp.' },
            @{ Pattern = '(?is)\bHtmlTextLayoutCacheKey\s+layoutKey\s*;.{0,900}\blayoutKey\.(?:formatKey|text|structureSignature|maxWidth100|maxHeight100|endEllipsis)\s*='; Message = 'HTML DirectWrite layout cache key assembly must stay in UIRenderDirectWriteTextInternal; UIRender.cpp should own only cache lookup/writeback and private structure signatures.' },
            @{ Pattern = '(?im)^\s*(?:bool\s+CanCacheDirectWriteHtmlParse|HtmlParseCacheKey\s+BuildDirectWriteHtmlParseCacheKey)\s*\('; Message = 'HTML parse cache policy/key helpers belong in UIRenderDirectWriteTextInternal, not UIRender.cpp.' },
            @{ Pattern = '(?is)\bHtmlParseCacheKey\s+parseCacheKey\s*=\s*\{'; Message = 'HTML parse cache key assembly must stay in UIRenderDirectWriteTextInternal; UIRender.cpp should own only cache lookup/writeback and parser state.' },
            @{ Pattern = '(?i)\bhtmlSourceText\.length\s*\(\s*\)\s*<=\s*1024\b'; Message = 'HTML parse cache eligibility must stay in UIRenderDirectWriteTextInternal.' },
            @{ Pattern = '(?im)^\s*(?:std::wstring\s+(?:TrimWideString|ToLowerWideString)|bool\s+(?:TryParseInteger|TryParseHexColor|ParseHtmlAttributeMap|TryReadHtmlTag|TryReadHtmlInlineTagBody))\s*\('; Message = 'Pure HTML parse/string helpers belong in UIRenderHtmlParseInternal, not UIRender.cpp.' },
            @{ Pattern = '(?im)^\s*(?:RECT\s+(?:ToHtmlMetricRect|OffsetHtmlMetricRect|OffsetHtmlRect)(?:Internal)?|bool\s+TryMergeHtmlMetricRect(?:Internal)?|std::vector<RECT>\s+BuildMergedHtmlMetricRects(?:Internal)?)\s*\('; Message = 'Pure HTML hit-test metric helpers belong in UIRenderHtmlMetricsInternal, not UIRender.cpp.' },
            @{ Pattern = '(?im)^\s*(?:bool\s+DrawBitmapSegmentWithDirect2D(?:Internal)?|void\s+DrawTiledBitmapSegmentWithDirect2D(?:Internal)?|RECT\s+GetRotatedBounds(?:Internal)?)\s*\('; Message = 'Pure Direct2D image segment/geometry helpers belong in UIRenderImageD2DInternal, not UIRender.cpp.' },
            @{ Pattern = '(?i)\b(?:CreateBitmapFromHBITMAP|CreateFormatConverter|CreateBitmapFromWicBitmap)\b'; Message = 'WIC-to-D2D bitmap conversion details belong in UIRenderImageD2DInternal, not UIRender.cpp.' }
        )
        foreach ($ban in $renderCppContextizedBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIManager\.h$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $managerHeaderBans = @(
            @{ Pattern = '(?i)\bHDC\b'; Message = 'CPaintManagerUI header must not expose HDC; keep native DC handling inside UIManager.cpp/Core internals.' },
            @{ Pattern = '(?i)\busing\s+namespace\s+Gdiplus\s*;'; Message = 'CPaintManagerUI header must not pollute consumers with using namespace Gdiplus.' }
        )
        foreach ($ban in $managerHeaderBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderSurface\.h$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $surfacePublicBans = @(
            @{ Pattern = '(?i)\bHDC\b'; Message = 'CPaintRenderSurface public header must not expose HDC.' },
            @{ Pattern = '(?i)public:\s*[^}]*\bEnsure\s*\(\s*HDC\b'; Message = 'CPaintRenderSurface must not expose public Ensure(HDC).' },
            @{ Pattern = '(?i)public:\s*[^}]*\bHDC\s+GetDC\s*\(\s*\)\s*const'; Message = 'CPaintRenderSurface must not expose public GetDC().' }
        )
        foreach ($ban in $surfacePublicBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderPrimitiveInternal\.h$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $primitiveHeaderBans = @(
            @{ Pattern = '(?i)\bHDC\b'; Message = 'Primitive internal header must stay CPaintRenderContext-based, not HDC-based.' },
            @{ Pattern = '(?i)\bGdiplusDrawRoundRectInternal\b'; Message = 'GDI+ round-rect helper should remain cpp-local.' }
        )
        foreach ($ban in $primitiveHeaderBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    $contextOnlyInternalHeaders = @(
        @{
            IncludePattern = '(?i)^Core\\UIRenderBatchInternal\.h$'
            Message = 'Direct2D batch internal header must expose CPaintRenderContext-based helpers, not raw HDC batch handles.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderD2DCacheTypesInternal\.h$'
            Message = 'D2D cache type internal header must stay type-only and must not expose raw HDC.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderD2DFrameMetricsInternal\.h$'
            Message = 'D2D frame metrics internal header must stay diagnostics-only and HDC-free.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderD2DResourceInternal\.h$'
            Message = 'D2D resource internal header must expose pure resource creation helpers and stay HDC-free.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderD2DSharedInternal\.h$'
            Message = 'D2D shared internal header must stay pure helper/context-free; do not expose raw HDC.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderHtmlMetricsInternal\.h$'
            Message = 'HTML metrics internal header must stay pure DirectWrite helper/type-only and HDC-free.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderHtmlParseInternal\.h$'
            Message = 'HTML parse internal header must stay pure helper/type-only and HDC-free.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageD2DInternal\.h$'
            Message = 'Image Direct2D internal header must stay pure D2D helper/type-only and HDC-free.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageLegacyInternal\.h$'
            Message = 'Image legacy internal header must expose CPaintRenderContext wrappers; keep raw HDC image fallback in cpp-local/internal islands.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderImageRuntimeInternal\.h$'
            Message = 'Image runtime internal header must expose CPaintRenderContext wrappers; keep raw HDC runtime helpers cpp-local.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderTextLegacyInternal\.h$'
            Message = 'Text legacy internal header must expose CPaintRenderContext wrappers; keep raw HDC text fallback in cpp-local/internal islands.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderTextSharedInternal\.h$'
            Message = 'Text shared internal header must expose CPaintRenderContext/pure helper APIs; keep HtmlText HDC parser state cpp-local.'
        },
        @{
            IncludePattern = '(?i)^Core\\UIRenderSurfaceInternal\.h$'
            Message = 'Render surface internal header must expose CPaintRenderContext/surface operations, not raw HDC accessors.'
        }
    )

    foreach ($item in $projectItems) {
        foreach ($ban in $contextOnlyInternalHeaders) {
            if ($item.Include -notmatch $ban.IncludePattern) {
                continue
            }

            $text = Get-Content -LiteralPath $item.Path -Raw
            $match = [regex]::Match($text, '(?i)\bHDC\b')
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderBatchInternal\.h$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $match = [regex]::Match($text, '(?i)\b(Begin|Flush|End)Direct2DBatchInternal\s*\(')
        if ($match.Success) {
            Add-Failure "Raw Direct2D batch begin/flush/end declarations must stay out of UIRenderBatchInternal.h. ($($item.Include):$((Get-LineNumber $text $match.Index)))"
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIRenderSurfaceInternal\.h$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $surfaceInternalHeaderBans = @(
            @{ Pattern = '(?i)\bGetRenderSurfaceDCInternal\b'; Message = 'Render surface DC getter must stay out of UIRenderSurfaceInternal.h.' },
            @{ Pattern = '(?i)\bEnsureRenderSurfaceForHdcInternal\b'; Message = 'Render surface HDC ensure helper must stay out of UIRenderSurfaceInternal.h.' },
            @{ Pattern = '(?i)\bCreateARGB32BitmapInternal\b'; Message = 'ARGB bitmap creation declaration must stay cpp-local to the surface allocation boundary.' }
        )
        foreach ($ban in $surfaceInternalHeaderBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Core\\UIManager\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $managerContextizedHelperBans = @(
            @{ Pattern = '(?i)\bEnsureNativeWindowBitmapCacheBitmap\s*\([^)]*\bHDC\b'; Message = 'Native window bitmap cache ensure helper must stay CPaintRenderContext-based.' },
            @{ Pattern = '(?i)\bRefreshNativeWindowBitmapCache\s*\([^)]*\bHDC\b'; Message = 'Native window bitmap cache refresh helper must stay CPaintRenderContext-based.' },
            @{ Pattern = '(?i)\bDrawNativeWindowBitmapCache\s*\([^)]*\bHDC\b'; Message = 'Native window bitmap cache draw helper must stay CPaintRenderContext-based.' },
            @{ Pattern = '(?i)\bRenderNativeWindowBitmapCacheEntry\s*\([^)]*\bHDC\b'; Message = 'Native window bitmap cache entry helper must stay CPaintRenderContext-based.' },
            @{ Pattern = '(?is)\b(?:EnsureNativeWindowBitmapCacheBitmap|DrawNativeWindowBitmapCache|RenderNativeWindowBitmapCacheEntry|RenderNativeWindowsToSurface)\b.{0,350}\.GetDC\s*\(\s*\)\s*(?:==|!=)\s*(?:NULL|nullptr)'; Message = 'Native window bitmap cache render-context validity must stay behind CanUseNativeWindowBitmapRenderContext.' },
            @{ Pattern = '(?is)\b(?:EnsureMainRenderSurface|EnsureLayeredBackgroundSurface|ExecutePaintWindowFrame|RenderPrintClientFrame|PrintChildWindows|PresentWindowSurface|PresentMainRenderSurface)\b.{0,350}\.GetDC\s*\(\s*\)\s*(?:==|!=)\s*(?:NULL|nullptr)'; Message = 'UIManager paint/print/present context validity must stay behind HasNativeRenderContext.' },
            @{ Pattern = '(?i)\bCreate(?:Manager|PrintClient|Derived)RenderContext\b'; Message = 'UIManager native render-context bridge helpers must keep explicit Native naming.' },
            @{ Pattern = '(?i)\bCScopedRenderContextDCSave\b|\b(childDCSave|dcSave)\b'; Message = 'Render-context SaveDC/RestoreDC RAII must use explicit NativeDC naming.' },
            @{ Pattern = '(?i)\b(?:referenceDC|screenDC|targetDC|childNativeDCSave)\.GetDC\s*\('; Message = 'Scoped native DC accessors in UIManager.cpp must use GetNativeDC().' },
            @{ Pattern = '(?im)^\s*HDC\s+h(?:Meta|Memory)NativeDC\s*='; Message = 'Metafile/compatible DC locals must use scoped native DC RAII helpers.' },
            @{ Pattern = '(?im)^\s*HGDIOBJ\s+hOldBmp\b'; Message = 'SelectObject bitmap restoration must use CScopedSelectedNativeObject.' },
            @{ Pattern = '(?i)::?\s*DeleteDC\s*\(\s*hMemoryNativeDC\s*\)|::?\s*CloseEnhMetaFile\s*\(\s*hMetaNativeDC\s*\)'; Message = 'Metafile/compatible DC cleanup must stay behind scoped native DC RAII helpers.' },
            @{ Pattern = '(?i)\b(pdsrc|pdobj)\b|(?:CIDropSource|CIDataObject)\s*\*\s*\w+\s*=\s*new\s+(?:CIDropSource|CIDataObject)|(?:CIDropSource|CIDataObject)[^;\r\n]*->\s*(?:AddRef|Release)\s*\(|delete\s+pds(?:rc|obj)\b'; Message = 'UIManager drag/drop COM objects must stay behind CScopedComObjectRef.' },
            @{ Pattern = '(?im)^\s*(?:LPBITMAPINFOHEADER|HDROP)\s+\w+\s*=\s*.*\bGlobalLock\s*\(|(?i)\bGlobalUnlock\s*\(\s*medium\.hGlobal\s*\)'; Message = 'UIManager drag/drop HGLOBAL locks must stay behind CScopedGlobalLock.' },
            @{ Pattern = '(?i)\bUpdateFontTextMetrics\s*\(\s*HDC\b'; Message = 'Font metrics update helper must stay CPaintRenderContext-based.' },
            @{ Pattern = '(?i)::\s*GetTextMetrics\s*\('; Message = 'Font metrics HDC details must stay in text shared internals, not UIManager.cpp.' },
            @{ Pattern = '(?i)\bm_hUpdateRectPen\b|::\s*CreatePen\s*\(\s*PS_SOLID\s*,\s*1\s*,\s*RGB\s*\(\s*220\s*,\s*0\s*,\s*0\s*\)'; Message = 'Update-rect debug drawing must stay on CRenderEngine/CPaintRenderContext, not a cached GDI pen.' },
            @{ Pattern = '(?i)\b(hReferenceDC|hMetaDC|hdcMem|hTargetDC)\b|(?im)^\s*HDC\s+hDC\s*='; Message = 'UIManager.cpp native DC locals must use explicit NativeDC names.' },
            @{ Pattern = '(?im)^\s*(?:const\s+)?HDC\s+\w+\s*=\s*::GetDC\s*\('; Message = 'UIManager.cpp window/screen GetDC acquisition must stay wrapped by CScopedWindowDC.' },
            @{ Pattern = '(?im)^\s*auto\s+\w+\s*=\s*::GetDC\s*\('; Message = 'UIManager.cpp window/screen GetDC acquisition must stay wrapped by CScopedWindowDC.' }
        )
        foreach ($ban in $managerContextizedHelperBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }

        if ($text -notmatch '(?s)class\s+CScopedWindowDC\b') {
            Add-Failure "UIManager.cpp must keep window/screen GetDC acquisition behind CScopedWindowDC. ($($item.Include):1)"
        }

        if ($text -notmatch '(?s)\bbool\s+HasNativeRenderContext\s*\(\s*const\s+CPaintRenderContext&\s+renderContext\s*\)') {
            Add-Failure "UIManager.cpp must keep CPaintRenderContext native-target checks centralized in HasNativeRenderContext. ($($item.Include):1)"
        }

        if ($text -notmatch '(?s)class\s+CScopedRenderContextNativeDCSave\b') {
            Add-Failure "UIManager.cpp must keep SaveDC/RestoreDC behind CScopedRenderContextNativeDCSave. ($($item.Include):1)"
        }

        foreach ($requiredScopedNativeHelper in @(
            'CScopedCompatibleNativeDC',
            'CScopedEnhMetaFileNativeDC',
            'CScopedSelectedNativeObject',
            'CScopedNativeGdiObject',
            'CScopedComObjectRef',
            'CScopedGlobalLock'
        )) {
            if ($text -notmatch "(?s)class\s+$requiredScopedNativeHelper\b") {
                Add-Failure "UIManager.cpp must keep Win32/OLE resources behind $requiredScopedNativeHelper. ($($item.Include):1)"
            }
        }

        if ($text -notmatch '(?s)\bHDC\s+GetRenderContextNativeDC\s*\(\s*const\s+CPaintRenderContext&\s+renderContext\s*\)') {
            Add-Failure "UIManager.cpp must keep CPaintRenderContext::GetDC isolated behind GetRenderContextNativeDC. ($($item.Include):1)"
        }

        $setImageMatches = [regex]::Matches($text, '(?i)\bSTM_SETIMAGE\b')
        if ($setImageMatches.Count -ne 1) {
            Add-Failure "UIManager.cpp STM_SETIMAGE ownership transfer must stay isolated in ReplaceNativeStaticBitmapImage. ($($item.Include):1)"
        }
        else {
            $replaceStaticBitmapIndex = $text.IndexOf('void ReplaceNativeStaticBitmapImage', [System.StringComparison]::Ordinal)
            if ($replaceStaticBitmapIndex -lt 0 -or
                $setImageMatches[0].Index -lt $replaceStaticBitmapIndex -or
                ($setImageMatches[0].Index - $replaceStaticBitmapIndex) -gt 800) {
                Add-Failure "UIManager.cpp must keep native static bitmap replacement behind ReplaceNativeStaticBitmapImage. ($($item.Include):1)"
            }
        }

        foreach ($match in [regex]::Matches($text, '(?i)\.GetDC\s*\(')) {
            $lineStart = $text.LastIndexOf("`n", [Math]::Max(0, $match.Index - 1))
            if ($lineStart -lt 0) {
                $lineStart = 0
            }
            $lineEnd = $text.IndexOf("`n", $match.Index)
            if ($lineEnd -lt 0) {
                $lineEnd = $text.Length
            }
            $line = $text.Substring($lineStart, $lineEnd - $lineStart)
            if ($line -notmatch '(?i)return\s+renderContext\.GetDC\s*\(\s*\)\s*;') {
                Add-Failure "UIManager.cpp direct CPaintRenderContext::GetDC access must stay isolated in GetRenderContextNativeDC. ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }

        $allowedHdcParameterHelpers = @(
            'CreateManagerNativeRenderContext',
            'CScopedCompatibleNativeDC',
            'CScopedEnhMetaFileNativeDC',
            'CScopedSelectedNativeObject'
        )
        $hdcParameterHelperPattern = '(?im)^\s*(?!return\b)(?:static\s+)?(?:[\w:<>*&]+\s+)+(?<Name>[A-Za-z_]\w*)\s*\((?<Params>[^\r\n;{)]*\bHDC\b[^\r\n;{)]*)\)'
        foreach ($match in [regex]::Matches($text, $hdcParameterHelperPattern)) {
            $name = $match.Groups['Name'].Value
            if ($allowedHdcParameterHelpers -notcontains $name) {
                Add-Failure "UIManager.cpp HDC parameter helper is not whitelisted; use CPaintRenderContext or document the native boundary. ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }

        foreach ($match in [regex]::Matches($text, '(?i)reinterpret_cast\s*<\s*HDC\s*>\s*\(')) {
            $lineStart = $text.LastIndexOf("`n", [Math]::Max(0, $match.Index - 1))
            if ($lineStart -lt 0) {
                $lineStart = 0
            }
            $lineEnd = $text.IndexOf("`n", $match.Index)
            if ($lineEnd -lt 0) {
                $lineEnd = $text.Length
            }
            $line = $text.Substring($lineStart, $lineEnd - $lineStart)
            if ($line -notmatch '(?i)CreateManagerNativeRenderContext\s*\(\s*pManager\s*,\s*reinterpret_cast\s*<\s*HDC\s*>\s*\(\s*paintNativeDC\s*\)') {
                Add-Failure "WM_PRINTCLIENT WPARAM-to-HDC cast must stay isolated in CreatePrintClientNativeRenderContext. ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Utils\\DragDropImpl\.cpp$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $dragDropImplBans = @(
            @{ Pattern = '(?i)pmedium->hGlobal\s*=\s*NULL\s*;'; Message = 'CIDataObject::GetData must zero the full STGMEDIUM output.' },
            @{ Pattern = '(?im)^\s*STGMEDIUM\s+medium\s*;'; Message = 'CIDropTarget::Drop must value-initialize STGMEDIUM before GetData.' },
            @{ Pattern = '(?is)case\s+TYMED_FILE\s*:\s*pMedSrc->lpszFileName\s*='; Message = 'TYMED_FILE duplication must write to the destination medium.' },
            @{ Pattern = '(?i)\bnew\s*(?:\([^)]*\)\s*)?(?:FORMATETC|STGMEDIUM)\b'; Message = 'CIDataObject must keep FORMATETC/STGMEDIUM storage value-based, not heap nodes.' },
            @{ Pattern = '(?i)delete\s+m_(?:ArrFormatEtc|StgMedium)\s*\['; Message = 'CIDataObject value arrays must not delete FORMATETC/STGMEDIUM nodes.' },
            @{ Pattern = '(?i)ReleaseStgMedium\s*\(\s*m_StgMedium\s*\['; Message = 'CIDataObject must release stored STGMEDIUM values by address.' },
            @{ Pattern = '(?i)m_ArrFormatEtc\s*\[[^\]]+\]\s*->|m_StgMedium\s*\[[^\]]+\]\s*->'; Message = 'CIDataObject FORMATETC/STGMEDIUM arrays must stay value-based.' }
        )
        foreach ($ban in $dragDropImplBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    foreach ($item in $projectItems) {
        if ($item.Include -notmatch '(?i)^Utils\\DragDropImpl\.h$') {
            continue
        }

        $text = Get-Content -LiteralPath $item.Path -Raw
        $dragDropImplHeaderBans = @(
            @{ Pattern = '(?i)\bPFormatEtcArray\b|\bPStgMediumArray\b'; Message = 'DragDropImpl must not reintroduce pointer-vector typedefs for FORMATETC/STGMEDIUM.' },
            @{ Pattern = '(?i)std::vector\s*<\s*(?:FORMATETC|STGMEDIUM)\s*\*\s*>'; Message = 'DragDropImpl FORMATETC/STGMEDIUM storage must stay value-based.' },
            @{ Pattern = '(?i)CEnumFormatEtc\s*\(\s*const\s+PFormatEtcArray&'; Message = 'CEnumFormatEtc should enumerate value FORMATETC arrays only.' }
        )
        foreach ($ban in $dragDropImplHeaderBans) {
            $match = [regex]::Match($text, $ban.Pattern)
            if ($match.Success) {
                Add-Failure "$($ban.Message) ($($item.Include):$((Get-LineNumber $text $match.Index)))"
            }
        }
    }

    $lowRiskControlHeaders = @(
        'Control\UIButton.h',
        'Control\UIButtonEx.h',
        'Control\UIColorPalette.h',
        'Control\UICombo.h',
        'Control\UIComboBox.h',
        'Control\UICountdown.h',
        'Control\UIFadeButton.h',
        'Control\UIGifAnim.h',
        'Control\UILabel.h',
        'Control\UIList.h',
        'Control\UIListEx.h',
        'Control\UIMenu.h',
        'Control\UIOption.h',
        'Control\UIProgress.h',
        'Control\UIRing.h',
        'Control\UIScrollBar.h',
        'Control\UISlider.h',
        'Layout\UIHorizontalLayout.h',
        'Layout\UIVerticalLayout.h'
    )
    $blockedLowRiskMethods = 'Paint|DoPaint|DoPostPaint|PaintBkColor|PaintBkImage|PaintStatusImage|PaintForeColor|PaintForeImage|PaintText|PaintBorder|PaintBk|PaintButton1|PaintButton2|PaintThumb|PaintRail|PaintPallet|DrawBorder|DrawTextExBtn|DrawItemText|DrawItemBk|DrawItemIcon|DrawItemExpland|DrawCheckBoxImage|DrawFrame|EnsureFrameCache'
    $lowRiskHdcPattern = "(?im)^\s*(?:virtual\s+)?(?:bool|BOOL|void|SIZE)\s+(?:$blockedLowRiskMethods)\s*\([^`r`n;{)]*\bHDC\b"

    foreach ($include in $lowRiskControlHeaders) {
        $path = Join-Path $ProjectRoot $include
        if (!(Test-Path -LiteralPath $path)) {
            continue
        }

        $text = Get-Content -LiteralPath $path -Raw
        foreach ($match in [regex]::Matches($text, $lowRiskHdcPattern)) {
            Add-Failure "Low-risk control HDC paint/helper signature is blocked; use CPaintRenderContext. ($($include):$((Get-LineNumber $text $match.Index)))"
        }
    }
}

if ($failures.Count -gt 0) {
    Write-Error ("FYUI render regression guard failed:`n" + ($failures -join "`n"))
    exit 1
}

Write-Host 'FYUI render regression guard passed.'
