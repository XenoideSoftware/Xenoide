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
    ctest --preset $preset --output-on-failure
} else {
    ctest --preset conan-default -C $Configuration --output-on-failure
}
