# Shared helpers for the mise build scripts.
#
# Dot-source this file from every mise/*.ps1 script:
#   . "$PSScriptRoot\lib\common.ps1"
#
# This file lives in mise/lib, so the repository root is two levels above it.

$commonPath = if ($PSCommandPath) { $PSCommandPath } else { $MyInvocation.MyCommand.Path }
$MiseLibDir = Split-Path -Parent $commonPath
$RepoRoot = Split-Path -Parent (Split-Path -Parent $MiseLibDir)
$MiseDir = Join-Path $RepoRoot "mise"

# Maps a --project selector to the list of subproject directories (relative to
# the repository root) that a script has to operate on. Accepts either one of
# the canonical project names (engine, ide, pocs), "all" (the default), or an
# explicit directory path.
function Resolve-Projects {
    param([string]$Selector)

    switch ($Selector) {
        "engine" { return @("src/engine") }
        "ide"    { return @("src/ide") }
        "pocs"   { return @("src/pocs") }
        "all"    { return @("src/engine", "src/ide", "src/pocs") }
        default  { return @($Selector) }
    }
}

# Returns the absolute path of the Conan profile matching the host OS. The
# profiles live at the repository root, while scripts operate inside a
# subproject, so they can never be referenced with a relative path.
function Get-ConanProfile {
    if ($PSVersionTable.OS -match 'Linux' -or $PSVersionTable.OS -match 'Darwin') {
        return (Join-Path $RepoRoot "conan/profiles/unix")
    }
    return (Join-Path $RepoRoot "conan/profiles/windows")
}
