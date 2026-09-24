$ErrorActionPreference = "Stop"

$packagesDir = "conan/recipes"

Get-ChildItem -Path $packagesDir -Directory | Sort-Object Name | ForEach-Object {
    $pkg = $_.Name
    $pkgDir = $_.FullName
    $conanfile = Join-Path $pkgDir "conanfile.py"
    $conandata = Join-Path $pkgDir "conandata.yml"

    if (-not (Test-Path $conanfile)) {
        return
    }

    $conanfileContent = Get-Content $conanfile -Raw
    $hasVersion = $conanfileContent -match '^\s*version\s*='

    if ($hasVersion) {
        Write-Host "==> Exporting $pkg ($pkgDir)"
        conan export $pkgDir
    } else {
        $versions = @()
        if (Test-Path $conandata) {
            $inSources = $false
            Get-Content $conandata | ForEach-Object {
                $line = $_
                $stripped = $line.Trim()
                if ($stripped -match '^sources:') {
                    $inSources = $true
                    return
                }
                if ($inSources) {
                    if ($line -match '^\s{2}[^ ]' -and $line -notmatch '^\s{4}') {
                        if ($line -match "^\s*['`"']?([^\`"':]+)['`"']?\s*:") {
                            $version = $matches[1]
                            if ($version) {
                                $versions += $version
                            }
                        }
                    } elseif ($stripped -and $stripped -notmatch '^\s' -and $stripped -notmatch '^#') {
                        $inSources = $false
                    }
                }
            }
        }

        if ($versions.Count -gt 0) {
            foreach ($v in $versions) {
                Write-Host "==> Exporting $pkg/$v ($pkgDir)"
                conan export $pkgDir --version $v
            }
        } else {
            Write-Host "==> Exporting $pkg ($pkgDir)"
            conan export $pkgDir
        }
    }
}