param(
    [string]$Project = "all"
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

foreach ($folder in @(Resolve-Projects $Project)) {
    Write-Host "=== [$folder] Formatting C/C++ sources ==="
    $srcDir = Join-Path $RepoRoot "$folder/src"
    if (-not (Test-Path $srcDir)) { continue }
    Get-ChildItem -Path $srcDir -Recurse -File -Include "*.cpp","*.h","*.hpp","*.c","*.cc","*.cxx" | ForEach-Object {
        clang-format -i $_.FullName
    }
}
