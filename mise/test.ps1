param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration,

    [string]$Project = "all"
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

$isUnix = $PSVersionTable.OS -match 'Linux' -or $PSVersionTable.OS -match 'Darwin'
$isMSystem = $env:MSYSTEM

$preset = "conan-$($Configuration.ToLower())"

foreach ($folder in @(Resolve-Projects $Project)) {
    Write-Host ""
    Write-Host "=== [$folder] Running tests ($Configuration) ==="
    Push-Location (Join-Path $RepoRoot $folder)
    try {
        if ($isMSystem -or $isUnix) {
            ctest --preset $preset --output-on-failure
        } else {
            ctest --preset $preset -C $Configuration --output-on-failure
        }
        if ($LASTEXITCODE -ne 0) { exit 1 }
    } finally {
        Pop-Location
    }
}
