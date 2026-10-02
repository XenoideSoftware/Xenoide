param(
    [string]$Project = "all"
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

# Legacy build trees left at the repository root by the pre-split layout.
Remove-Item -Recurse -Force (Join-Path $RepoRoot "build"), (Join-Path $RepoRoot "build-*") -ErrorAction SilentlyContinue

foreach ($folder in @(Resolve-Projects $Project)) {
    Write-Host "=== [$folder] Removing build directories ==="
    Remove-Item -Recurse -Force (Join-Path $RepoRoot "$folder/build"), (Join-Path $RepoRoot "$folder/build-*") -ErrorAction SilentlyContinue
}
