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
    Write-Host "=== [$folder] Installing tidy Conan dependencies ($Configuration) ==="
    Push-Location (Join-Path $RepoRoot $folder)
    try {
        # The tidy tree does not use presets; skip the root CMakeUserPresets.json
        # update so duplicate preset names do not break `cmake --preset`.
        conan install . --build=missing -s build_type=$Configuration -pr:h $profile -pr:b $profile `
            -of "build-tidy/$Configuration" `
            -c "tools.cmake.cmaketoolchain:user_presets="
        if ($LASTEXITCODE -ne 0) { exit 1 }
    } finally {
        Pop-Location
    }
}
