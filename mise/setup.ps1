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

conan install . --build=missing -s build_type=$Configuration -pr:h $profile -pr:b $profile
