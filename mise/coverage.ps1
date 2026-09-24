param(
    [Parameter(Mandatory)]
    [ValidateSet('gcov', 'llvm-cov')]
    [string]$Tool,
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release', 'all')]
    [string]$Configuration
)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)

# mise exposes user-provided usage flags as environment variables; let them
# override the parameters baked into the mise task definitions.
$config = if ($env:usage_config) { $env:usage_config } else { $Configuration }
$export = if ($env:usage_export) { $env:usage_export } else { "text" }
$check = if ($env:usage_check) { $env:usage_check } else { $null }
$outputDir = if ($env:usage_output_dir) { $env:usage_output_dir } else { $null }
$clean = if ($env:usage_clean) { $env:usage_clean } else { "false" }

switch -Regex ($config) {
    '^debug$'   { $config = "Debug" }
    '^release$' { $config = "Release" }
    '^all$'     { $config = "all" }
    default     { Write-Host "Error: invalid --config '$config' (expected Debug, Release or all)"; exit 1 }
}

switch -Regex ($export) {
    '^text$' { $export = "text" }
    '^html$' { $export = "html" }
    '^csv$'  { $export = "csv" }
    default  { Write-Host "Error: invalid --export '$export' (expected text, html or csv)"; exit 1 }
}

if ($check) {
    if ($check -eq "true") { $check = "90" }
    if ($check -notmatch '^\d+(\.\d+)?$') {
        Write-Host "Error: invalid --check threshold '$check' (expected a number)"
        exit 1
    }
}

if (-not $outputDir) {
    if ($config -eq "all") { $outputDir = "coverage/$Tool" }
    else { $outputDir = "coverage/$Tool/$config" }
}
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$os = $PSVersionTable.OS
if ($os -match 'Linux' -or $os -match 'Darwin') {
    $profile = "conan/profiles/unix"
} else {
    $profile = "conan/profiles/windows"
}

$configs = @()
if ($config -eq "all") { $configs = @("Debug", "Release") } else { $configs = @($config) }

$llvmBinaries = @()

foreach ($cfg in $configs) {
    $buildDir = "build/coverage-$Tool/$cfg"

    Write-Host ""
    Write-Host "=== [$Tool / $cfg] Installing Conan dependencies ==="
    # The coverage tree does not use presets; skip the root CMakeUserPresets.json
    # update so duplicate preset names do not break `cmake --preset`.
    conan install . --build=missing -s build_type=$cfg -pr:h $profile -pr:b $profile -of $buildDir `
        -c "tools.cmake.cmaketoolchain:user_presets="
    if ($LASTEXITCODE -ne 0) { exit 1 }

    $toolchain = Get-ChildItem -Path $buildDir -Recurse -Filter "conan_toolchain.cmake" -ErrorAction SilentlyContinue |
        Select-Object -First 1 -ExpandProperty FullName
    if (-not $toolchain) {
        Write-Host "Error: conan_toolchain.cmake not found under $buildDir"
        exit 1
    }

    Write-Host "=== [$Tool / $cfg] Configuring CMake ==="
    $cmakeArgs = @(
        "-B", $buildDir,
        "-DCMAKE_TOOLCHAIN_FILE=$toolchain",
        "-DCMAKE_BUILD_TYPE=$cfg",
        "-DXE_ENABLE_COVERAGE=ON"
    )
    if ($Tool -eq "llvm-cov") {
        $cmakeArgs += "-DCMAKE_C_COMPILER=clang"
        $cmakeArgs += "-DCMAKE_CXX_COMPILER=clang++"
    }
    cmake @cmakeArgs
    if ($LASTEXITCODE -ne 0) { exit 1 }

    $testTargets = @()
    try {
        $json = ctest --test-dir $buildDir --show-only=json-v1 2>$null | ConvertFrom-Json
        foreach ($t in $json.tests) {
            if ($t.command -and $t.command.Count -gt 0) {
                $testTargets += Split-Path -Leaf $t.command[0]
            } else {
                $name = [string]$t.name
                if ($name -match '^(.*?)_NOT_BUILT-') {
                    $testTargets += $Matches[1]
                } elseif ($name) {
                    $testTargets += $name
                }
            }
        }
    } catch {
        $testTargets = @()
    }
    $testTargets = @($testTargets | Select-Object -Unique)

    Write-Host "=== [$Tool / $cfg] Building test targets ==="
    if ($testTargets.Count -gt 0) {
        cmake --build $buildDir --parallel --target $testTargets
    } else {
        Write-Host "Warning: no test targets discovered, building the full tree"
        cmake --build $buildDir --parallel
    }
    if ($LASTEXITCODE -ne 0) { exit 1 }

    Write-Host "=== [$Tool / $cfg] Running CTest ==="
    if ($clean -eq "true") {
        Get-ChildItem -Path $buildDir -Recurse -Include *.gcda, *.profraw -ErrorAction SilentlyContinue |
            Remove-Item -Force -ErrorAction SilentlyContinue
    }

    if ($Tool -eq "llvm-cov") {
        $profileDir = Join-Path $buildDir "profiles"
        New-Item -ItemType Directory -Force -Path $profileDir | Out-Null
        $env:LLVM_PROFILE_FILE = Join-Path $ProjectRoot "$profileDir/%p.profraw"
        ctest --test-dir $buildDir --output-on-failure
        if ($LASTEXITCODE -ne 0) { exit 1 }
        Remove-Item Env:\LLVM_PROFILE_FILE

        try {
            $json = ctest --test-dir $buildDir --show-only=json-v1 2>$null | ConvertFrom-Json
            foreach ($t in $json.tests) {
                if ($t.command -and $t.command.Count -gt 0) {
                    $exe = $t.command[0]
                    if ($llvmBinaries -notcontains $exe) { $llvmBinaries += $exe }
                }
            }
        } catch {
            $llvmBinaries = @()
        }
    } else {
        ctest --test-dir $buildDir --output-on-failure
        if ($LASTEXITCODE -ne 0) { exit 1 }
    }
}

$reporterArgs = @("--tool", $Tool, "--export", $export, "--output-dir", $outputDir)
if ($check) { $reporterArgs += @("--check", $check) }

if ($Tool -eq "llvm-cov") {
    Write-Host ""
    Write-Host "=== Merging LLVM profiles ==="
    $profraws = Get-ChildItem -Path "build/coverage-$Tool" -Recurse -Filter *.profraw -ErrorAction SilentlyContinue
    if (-not $profraws) {
        Write-Host "Error: no .profraw profile files found under build/coverage-$Tool"
        exit 1
    }
    $merged = Join-Path $outputDir "merged.profdata"
    llvm-profdata merge -o $merged $profraws.FullName
    if ($LASTEXITCODE -ne 0) { exit 1 }
    Write-Host "Merged profiles into $merged"

    if ($llvmBinaries.Count -eq 0) {
        Write-Host "Error: no instrumented test executables found"
        exit 1
    }

    $exportDir = Join-Path $outputDir ".llvm-exports"
    New-Item -ItemType Directory -Force -Path $exportDir | Out-Null
    Write-Host "=== Exporting coverage from $($llvmBinaries.Count) test executable(s) ==="
    $index = 0
    foreach ($exe in ($llvmBinaries | Select-Object -Unique)) {
        $outJson = Join-Path $exportDir "report-$index.json"
        llvm-cov export -instr-profile=$merged $exe 2>$null | Set-Content -Path $outJson
        if ($LASTEXITCODE -eq 0) {
            $index++
        } else {
            Write-Host "  (skipped '$(Split-Path -Leaf $exe)': no coverage data)"
        }
    }
    if ($index -eq 0) {
        Write-Host "Error: no llvm-cov export JSON files could be generated"
        exit 1
    }
    Get-ChildItem -Path $exportDir -Filter "report-*.json" | ForEach-Object { $reporterArgs += $_.FullName }
} else {
    Write-Host ""
    Write-Host "=== Aggregating gcov data ==="
    $gcovDir = Join-Path $outputDir ".gcov-json"
    New-Item -ItemType Directory -Force -Path $gcovDir | Out-Null
    foreach ($cfg in $configs) {
        $cfgDir = Join-Path $gcovDir $cfg
        New-Item -ItemType Directory -Force -Path $cfgDir | Out-Null
        $buildDir = "build/coverage-$Tool/$cfg"
        $count = 0
        $gcnos = Get-ChildItem -Path (Join-Path $ProjectRoot $buildDir) -Recurse -Filter *.gcno -ErrorAction SilentlyContinue
        foreach ($gcno in $gcnos) {
            $gcda = $gcno.FullName -replace '\.gcno$', '.gcda'
            if (-not (Test-Path $gcda)) { continue }
            Push-Location $cfgDir
            gcov -j $gcno.FullName *> $null
            Pop-Location
            $count++
        }
        Write-Host "  [$cfg] generated gcov data for $count executed translation unit(s)"
    }
    $reporterArgs += @("--gcov-dir", $gcovDir)
}

Write-Host ""
Write-Host "=== Generating $export report ==="
python mise/coverage_reporter.py @reporterArgs
exit $LASTEXITCODE