param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

$bin = Join-Path $RepoRoot "src/cmake-checker/build-cmake-check/$Configuration/bin/cmake-checker"

if (-not (Test-Path $bin)) {
    Write-Host "Error: cmake-checker binary not found at $bin."
    Write-Host "Run 'mise run install:cmake-check:$($Configuration.ToLower())' first."
    exit 1
}

& $bin --project (Join-Path $RepoRoot "src/engine") --project-build-dir (Join-Path $RepoRoot "src/engine/build-cmake-check/$Configuration")
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }