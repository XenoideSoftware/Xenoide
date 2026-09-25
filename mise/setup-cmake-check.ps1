param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

$profile = Get-ConanProfile

Write-Host ""
Write-Host "=== [src/cmake-checker] Building cmake-checker ($Configuration) ==="
Push-Location (Join-Path $RepoRoot "src/cmake-checker")
try {
    conan install . --build=missing -s build_type=$Configuration -pr:h $profile -pr:b $profile `
        -of "build-cmake-check/$Configuration" `
        -c "tools.cmake.cmaketoolchain:user_presets="
    if ($LASTEXITCODE -ne 0) { exit 1 }

    cmake -S . -B "build-cmake-check/$Configuration" `
        -DCMAKE_TOOLCHAIN_FILE="build-cmake-check/$Configuration/conan_toolchain.cmake" `
        -DCMAKE_BUILD_TYPE=$Configuration
    if ($LASTEXITCODE -ne 0) { exit 1 }

    cmake --build "build-cmake-check/$Configuration"
    if ($LASTEXITCODE -ne 0) { exit 1 }
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "=== [src/engine] Installing CMake checker dependencies ($Configuration) ==="
Push-Location (Join-Path $RepoRoot "src/engine")
try {
    conan install . --build=missing -s build_type=$Configuration -pr:h $profile -pr:b $profile `
        -of "build-cmake-check/$Configuration" `
        -c "tools.cmake.cmaketoolchain:user_presets="
    if ($LASTEXITCODE -ne 0) { exit 1 }
} finally {
    Pop-Location
}