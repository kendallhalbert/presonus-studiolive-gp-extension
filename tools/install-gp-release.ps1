# Copy Release PreSonusStudioLive.dll into Gig Performer Extensions.
# Usage: .\tools\install-gp-release.ps1 [-ExtensionsDir "C:\path\to\Extensions"]

param(
    [string]$ExtensionsDir = ""
)

$ErrorActionPreference = "Stop"
$repoRoot = Split-Path $PSScriptRoot -Parent
$dll = Join-Path $repoRoot "build-rel\bin\Release\PreSonusStudioLive.dll"

if (-not (Test-Path $dll)) {
    Write-Error "Build the DLL first: cmake --build build-rel --config Release"
}

$candidates = @(
    $ExtensionsDir,
    "C:\Users\Public\Documents\Gig Performer\Extensions",
    "$env:USERPROFILE\Documents\Gig Performer\Extensions",
    "$env:USERPROFILE\OneDrive\Documents\Gig Performer\Extensions"
) | Where-Object { $_ -ne "" }

$destDir = $null
foreach ($c in $candidates) {
    if (Test-Path $c) {
        $destDir = $c
        break
    }
}

if (-not $destDir) {
    $destDir = "C:\Users\Public\Documents\Gig Performer\Extensions"
    New-Item -ItemType Directory -Force -Path $destDir | Out-Null
    Write-Host "Created Extensions folder: $destDir"
}

$dest = Join-Path $destDir "PreSonusStudioLive.dll"
Copy-Item -Force $dll $dest
Write-Host "Installed: $dest"
Get-Item $dest | Format-List FullName, Length, LastWriteTime

# Remove any external panel override — it bypasses the embedded template and has
# caused blank panels (UTF-8 BOM) and GP crashes (knob GPSCALE clones).
$panelOverride = Join-Path $destDir "PreSonusScenePicker.gppanel"
if (Test-Path $panelOverride) {
    Remove-Item -Force $panelOverride
    Write-Host "Removed panel override: $panelOverride"
}
