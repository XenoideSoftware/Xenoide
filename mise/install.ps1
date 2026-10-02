param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration,

    [string]$Project = "all"
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

$profile = Get-ConanProfile

foreach ($folder in @(Resolve-Projects $Project)) {
    Write-Host ""
    Write-Host "=== [$folder] Installing Conan dependencies ($Configuration) ==="
    Push-Location (Join-Path $RepoRoot $folder)
    try {
        conan install . --build=missing -s build_type=$Configuration -pr:h $profile -pr:b $profile
        if ($LASTEXITCODE -ne 0) { exit 1 }
    } finally {
        Pop-Location
    }
}
