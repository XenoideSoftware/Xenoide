param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration,

    [string]$Project = "all"
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

foreach ($folder in @(Resolve-Projects $Project)) {
    Write-Host ""
    Write-Host "=== [$folder] Configuring tidy analysis ($Configuration) ==="
    $sourceDir = Join-Path $RepoRoot $folder
    $BuildDir = Join-Path $sourceDir "build-tidy/$Configuration"

    $toolchain = Get-ChildItem -Path $BuildDir -Recurse -Filter "conan_toolchain.cmake" -ErrorAction SilentlyContinue |
        Select-Object -First 1 -ExpandProperty FullName

    if (-not $toolchain) {
        Write-Host "Error: conan_toolchain.cmake not found under $BuildDir. Run setup-tidy first."
        exit 1
    }

    # The subprojects resolve `CMAKE_MODULE_PATH` entries such as "../cmake"
    # against the working directory, so CMake must run from the subproject.
    Push-Location $sourceDir
    try {
        cmake -B $BuildDir `
            -DCMAKE_TOOLCHAIN_FILE=$toolchain `
            -DCMAKE_BUILD_TYPE=$Configuration `
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        if ($LASTEXITCODE -ne 0) { exit 1 }
    } finally {
        Pop-Location
    }
}
