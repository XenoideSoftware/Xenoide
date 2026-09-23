param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration
)

$ErrorActionPreference = "Stop"

$BuildDir = "build-tidy/$Configuration"

$toolchain = Get-ChildItem -Path $BuildDir -Recurse -Filter "conan_toolchain.cmake" -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName

if (-not $toolchain) {
    Write-Host "Error: conan_toolchain.cmake not found under $BuildDir. Run setup-tidy first."
    exit 1
}

cmake -B $BuildDir `
    -DCMAKE_TOOLCHAIN_FILE=$toolchain `
    -DCMAKE_BUILD_TYPE=$Configuration `
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON