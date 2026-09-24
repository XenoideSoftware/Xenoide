$ErrorActionPreference = "Stop"

$packagesDir = "conan/recipes"
$exportArgs = @("--user", "xenoide", "--channel", "xenoide")

Get-ChildItem -Path $packagesDir -Directory | Sort-Object Name | ForEach-Object {
    $pkg = $_.Name
    $pkgDir = $_.FullName
    $conanfile = Join-Path $pkgDir "conanfile.py"
    $conandata = Join-Path $pkgDir "conandata.yml"

    $config = Join-Path $pkgDir "config.yml"

    if (-not (Test-Path $conanfile)) {
        if (Test-Path $config) {
            $versions = @()
            $folders = @()
            $inVersions = $false
            $version = ""
            Get-Content $config | ForEach-Object {
                $line = $_
                $stripped = $line.Trim()
                if ($stripped -match '^versions:') {
                    $inVersions = $true
                    return
                }
                if ($inVersions) {
                    if ($line -match '^\s{2}["`'']?[^"''\s:]+["`'']?\s*:\s*$') {
                        if ($line -match "^\s*['`"']?([^'''':]+)['`"']?\s*:\s*$") {
                            $version = $matches[1]
                        }
                    } elseif ($version) {
                        if ($line -match '^\s*folder:\s*["`'']?([^"`'']+)["`'']?\s*$') {
                            $folder = $matches[1]
                            if ($folder) {
                                $versions += $version
                                $folders += $folder
                                $version = ""
                            }
                        }
                    }
                    if ($stripped -and $line -notmatch '^\s' -and $stripped -notmatch '^#') {
                        $inVersions = $false
                    }
                }
            }

            if ($versions.Count -gt 0) {
                for ($i = 0; $i -lt $versions.Count; $i++) {
                    $v = $versions[$i]
                    $folder = $folders[$i]
                    Write-Host "==> Exporting $pkg/$v ($pkgDir/$folder)"
                    conan export "$pkgDir/$folder" --version $v @exportArgs
                }
                return
            }
        }
        return
    }

    $conanfileContent = Get-Content $conanfile -Raw
    $hasVersion = $conanfileContent -match '^\s*version\s*='

    if ($hasVersion) {
        Write-Host "==> Exporting $pkg ($pkgDir)"
        conan export $pkgDir @exportArgs
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
                conan export $pkgDir --version $v @exportArgs
            }
        } else {
            Write-Host "==> Exporting $pkg ($pkgDir)"
            conan export $pkgDir @exportArgs
        }
    }
}