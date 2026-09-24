param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration
)

$ErrorActionPreference = "Stop"

$os = $PSVersionTable.OS
if ($os -match 'Linux' -or $os -match 'Darwin') {
    $profile = "conan/profiles/unix"
} elseif ($env:MSYSTEM -or $os -match 'CYGWIN' -or $os -match 'MINGW' -or $os -match 'MSYS') {
    $profile = "conan/profiles/windows"
} else {
    $profile = "conan/profiles/windows"
}

# The tidy tree does not use presets; skip the root CMakeUserPresets.json
# update so duplicate preset names do not break `cmake --preset`.
conan install . --build=missing -s build_type=$Configuration -pr:h $profile -pr:b $profile -of "build-tidy/$Configuration" `
    -c "tools.cmake.cmaketoolchain:user_presets="
if ($LASTEXITCODE -ne 0) { exit 1 }