param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration
)

$ErrorActionPreference = "Stop"

$isUnix = $PSVersionTable.OS -match 'Linux' -or $PSVersionTable.OS -match 'Darwin'
$isMSystem = $env:MSYSTEM

$preset = "conan-$($Configuration.ToLower())"

if ($isMSystem -or $isUnix) {
    cmake --preset $preset
} else {
    cmake --preset $preset
}
