param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

$buildDir = Join-Path $RepoRoot "src/engine/build-cmake-check/$Configuration"

$toolchain = Get-ChildItem -Path $buildDir -Recurse -Filter "conan_toolchain.cmake" -ErrorAction SilentlyContinue |
    Select-Object -First 1 -ExpandProperty FullName

if (-not $toolchain) {
    Write-Host "Error: conan_toolchain.cmake not found under $buildDir. Run install:cmake-check first."
    exit 1
}

$queryDir = Join-Path $buildDir ".cmake/api/v1/query/client-xe-cmake-check"
New-Item -ItemType Directory -Force -Path $queryDir | Out-Null
@'
{"requests": [{"kind": "codemodel", "version": 2}, {"kind": "cmakeFiles", "version": 1}]}
'@ | Set-Content -Path (Join-Path $queryDir "query.json") -NoNewline

Remove-Item -Path (Join-Path $buildDir "trace.json") -ErrorAction SilentlyContinue

Push-Location (Join-Path $RepoRoot "src/engine")
try {
    cmake -B $buildDir `
        -DCMAKE_TOOLCHAIN_FILE=$toolchain `
        -DCMAKE_BUILD_TYPE=$Configuration `
        --trace-format=json-v1 `
        --trace-redirect=$(Join-Path $buildDir "trace.json")
    if ($LASTEXITCODE -ne 0) { exit 1 }
} finally {
    Pop-Location
}