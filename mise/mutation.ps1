$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
Set-Location $ProjectRoot

function Write-Usage {
    Write-Host @"
Usage: powershell -ExecutionPolicy Bypass -File mise/mutation.ps1 [options]

Runs Mull mutation testing against the Xenoide unit test suite.

Options are provided through mise usage flags (environment variables):
  --generate            Discover and generate mutants without running tests (dry run)
  --kill                Execute unit tests against each mutant to kill them (default)
  --config <cfg>        Build type: Debug (default) or Release
  --reporters <list>    Comma-separated report formats: IDE,SQLite,Elements,Patches,Sarif
  --output-dir <dir>    Directory for report files (default: mutation/mull)
  --report-name <name>  Report database base name (default: xenoide)
  --clean               Delete existing build/mutation artifacts before running
  --target <name>       Run against a specific test target (e.g. xe-core-test)
  --timeout <ms>        Maximum test execution timeout per mutant
  --threshold <score>   Minimum required mutation score (0-100)
"@
}

function Fail([string]$Message) {
    Write-Host "Error: $Message"
    exit 1
}

# ---------------------------------------------------------------------------
# mise exposes user-provided usage flags as environment variables.
# ---------------------------------------------------------------------------

$generate = ($env:usage_generate -eq "true")
$kill = ($env:usage_kill -eq "true")
$config = if ($env:usage_config) { $env:usage_config } else { "Debug" }
$reporters = if ($env:usage_reporters) { $env:usage_reporters } else { "IDE,SQLite,Elements" }
$outputDir = if ($env:usage_output_dir) { $env:usage_output_dir } else { "mutation/mull" }
$reportName = if ($env:usage_report_name) { $env:usage_report_name } else { "xenoide" }
$clean = ($env:usage_clean -eq "true")
$target = if ($env:usage_target) { $env:usage_target } else { "" }
$timeout = if ($env:usage_timeout) { $env:usage_timeout } else { "" }
$threshold = if ($env:usage_threshold) { $env:usage_threshold } else { "" }

if ($generate -and $kill) { Fail "--generate and --kill are mutually exclusive" }

switch -Regex ($config) {
    '^debug$'   { $config = "Debug" }
    '^release$' { $config = "Release" }
    default     { Fail "invalid --config '$config' (expected Debug or Release)" }
}

if (-not $outputDir) { Fail "--output-dir cannot be empty" }
if (-not $reportName) { Fail "--report-name cannot be empty" }

$reporterMap = @{
    "ide" = "IDE"; "sqlite" = "SQLite"; "elements" = "Elements"
    "patches" = "Patches"; "sarif" = "Sarif"
}
$reporterList = @()
foreach ($raw in ($reporters -split ',')) {
    $item = $raw.Trim()
    if (-not $item) { continue }
    $key = $item.ToLower()
    if (-not $reporterMap.ContainsKey($key)) {
        Fail "invalid reporter '$item' (expected IDE, SQLite, Elements, Patches or Sarif)"
    }
    $reporterList += $reporterMap[$key]
}
if ($reporterList.Count -eq 0) { Fail "--reporters cannot be empty" }

if ($timeout -and $timeout -notmatch '^[1-9][0-9]*$') {
    Fail "invalid --timeout '$timeout' (expected a positive number of milliseconds)"
}
if ($threshold) {
    if ($threshold -notmatch '^\d+(\.\d+)?$' -or [double]$threshold -gt 100) {
        Fail "invalid --threshold '$threshold' (expected a number between 0 and 100)"
    }
}

foreach ($bin in @("cmake", "ctest", "conan")) {
    if (-not (Get-Command $bin -ErrorAction SilentlyContinue)) {
        Fail "required binary '$bin' not found"
    }
}

# ---------------------------------------------------------------------------
# Mull / Clang version pairing (LLVM pass plugins are bound to one major).
# ---------------------------------------------------------------------------

$mullRunners = @(Get-Command "mull-runner-*" -ErrorAction SilentlyContinue)
$mullVersions = @()
foreach ($runner in $mullRunners) {
    $version = ($runner.Name -split '-')[-1]
    if ($version -match '^\d+$') { $mullVersions += [int]$version }
}
$mullVersions = @($mullVersions | Sort-Object -Descending)
if ($mullVersions.Count -eq 0) {
    Fail "no versioned mull-runner found in PATH (expected mull-runner-<LLVM version>, e.g. mull-runner-19)"
}
$mullVersion = [string]$mullVersions[0]

$mullRunner = "mull-runner-$mullVersion"
$mullReporter = "mull-reporter-$mullVersion"
$ccBin = "clang-$mullVersion"
$cxxBin = "clang++-$mullVersion"

foreach ($bin in @($mullRunner, $mullReporter, $ccBin, $cxxBin)) {
    if (-not (Get-Command $bin -ErrorAction SilentlyContinue)) {
        Fail "version mismatch: found $mullRunner but '$bin' is not available (install matching Mull and Clang $mullVersion packages)"
    }
}

$pluginCandidates = @(
    "/usr/lib/mull-ir-frontend-$mullVersion",
    "/usr/local/lib/mull-ir-frontend-$mullVersion",
    "$env:ProgramFiles\mull\mull-ir-frontend-$mullVersion",
    "$env:ProgramFiles\mull\bin\mull-ir-frontend-$mullVersion.dll",
    "${env:ProgramFiles(x86)}\mull\mull-ir-frontend-$mullVersion"
)
$mullPlugin = $null
foreach ($candidate in $pluginCandidates) {
    if ($candidate -and (Test-Path $candidate)) { $mullPlugin = $candidate; break }
}
if (-not $mullPlugin) {
    foreach ($root in @("$env:ProgramFiles\mull", "/usr/lib", "/usr/local/lib")) {
        if (-not (Test-Path $root)) { continue }
        $found = Get-ChildItem -Path $root -Filter "mull-ir-frontend-$mullVersion*" -Recurse -Depth 2 -ErrorAction SilentlyContinue |
            Select-Object -First 1 -ExpandProperty FullName
        if ($found) { $mullPlugin = $found; break }
    }
}
if (-not $mullPlugin) { Fail "mull-ir-frontend-$mullVersion not found" }

$mode = if ($generate) { "generate (dry run)" } else { "kill" }
Write-Host "=== Mull toolchain ==="
Write-Host "  mull-runner  : $mullRunner"
Write-Host "  mull-reporter: $mullReporter"
Write-Host "  compiler     : $cxxBin / $ccBin"
Write-Host "  ir frontend  : $mullPlugin"
Write-Host "  mode         : $mode"
Write-Host "  config       : $config"

# ---------------------------------------------------------------------------

if ($clean) {
    Write-Host ""
    Write-Host "=== Cleaning build and mutation artifacts ==="
    if (Test-Path "build-mutation-mull") { Remove-Item -Recurse -Force "build-mutation-mull" }
    if (Test-Path $outputDir) { Remove-Item -Recurse -Force $outputDir }
}

New-Item -ItemType Directory -Force -Path $outputDir | Out-Null

$os = $PSVersionTable.OS
if ($os -match 'Linux' -or $os -match 'Darwin') {
    $profile = "conan/profiles/unix"
} else {
    $profile = "conan/profiles/windows"
}

$buildDir = "build-mutation-mull/$config"

# ---------------------------------------------------------------------------
# 1. Configure and build the instrumented test targets in an isolated tree.
# ---------------------------------------------------------------------------

Write-Host ""
Write-Host "=== [$config] Installing Conan dependencies ==="
# Disable the root CMakeUserPresets.json update: this isolated tree does not
# use presets, and accumulating include entries would break `cmake --preset`
# with duplicate preset names.
conan install . --build=missing -s build_type=$config -pr:h $profile -pr:b $profile -of $buildDir `
    -c "tools.cmake.cmaketoolchain:user_presets="
if ($LASTEXITCODE -ne 0) { exit 1 }

$toolchain = Get-ChildItem -Path $buildDir -Recurse -Filter "conan_toolchain.cmake" -ErrorAction SilentlyContinue |
    Select-Object -First 1 -ExpandProperty FullName
if (-not $toolchain) { Fail "conan_toolchain.cmake not found under $buildDir" }

Write-Host "=== [$config] Configuring CMake with Mull instrumentation ==="
$mullFlags = "-fpass-plugin=$mullPlugin -O0 -g -grecord-command-line"
$cmakeArgs = @(
    "-B", $buildDir,
    "-DCMAKE_TOOLCHAIN_FILE=$toolchain",
    "-DCMAKE_BUILD_TYPE=$config",
    "-DCMAKE_C_COMPILER=$ccBin",
    "-DCMAKE_CXX_COMPILER=$cxxBin"
)
if ($config -eq "Release") {
    $cmakeArgs += "-DCMAKE_C_FLAGS_RELEASE=-O0 -g -DNDEBUG"
    $cmakeArgs += "-DCMAKE_CXX_FLAGS_RELEASE=-O0 -g -DNDEBUG"
}

$prevCFLAGS = $env:CFLAGS
$prevCXXFLAGS = $env:CXXFLAGS
$env:CFLAGS = $mullFlags
$env:CXXFLAGS = $mullFlags
cmake @cmakeArgs
$env:CFLAGS = $prevCFLAGS
$env:CXXFLAGS = $prevCXXFLAGS
if ($LASTEXITCODE -ne 0) { exit 1 }

$cacheFile = Join-Path $buildDir "CMakeCache.txt"
if (-not (Test-Path $cacheFile) -or -not (Select-String -Path $cacheFile -Pattern "fpass-plugin" -Quiet)) {
    Fail "build tree at $buildDir was not configured with Mull flags; re-run with --clean"
}

# Extract buildable test target names from ctest. catch_discover_tests()
# registers "<target>_NOT_BUILT-<hash>" placeholders until the binary exists.
$testTargets = @()
try {
    $json = ctest --test-dir $buildDir --show-only=json-v1 2>$null | ConvertFrom-Json
    foreach ($t in $json.tests) {
        $command = if ($t.command -and $t.command.Count -gt 0) { $t.command[0] } else { $null }
        $name = [string]$t.name
        $raw = if ($command) { Split-Path -Leaf $command } else { $name }
        if ($raw -match '^(.*?)_NOT_BUILT') {
            $testTargets += $Matches[1]
        } elseif ($command) {
            $testTargets += $raw
        } elseif ($name) {
            $testTargets += $name
        }
    }
} catch {
    $testTargets = @()
}
$testTargets = @($testTargets | Select-Object -Unique)

if ($target -and $testTargets.Count -gt 0) {
    if ($testTargets -contains $target) {
        $testTargets = @($target)
    } else {
        # Target may not be registered yet; let CMake decide whether it exists.
        $testTargets = @($target)
    }
}

Write-Host "=== [$config] Building test targets ==="
if ($testTargets.Count -gt 0) {
    cmake --build $buildDir --parallel --target $testTargets
} else {
    Write-Host "Warning: no test targets discovered, building the full tree"
    cmake --build $buildDir --parallel
}
if ($LASTEXITCODE -ne 0) { exit 1 }

# ---------------------------------------------------------------------------
# 2. Discover the built test executables.
# ---------------------------------------------------------------------------

$testExecutables = @()
try {
    $json = ctest --test-dir $buildDir --show-only=json-v1 2>$null | ConvertFrom-Json
    foreach ($t in $json.tests) {
        if (-not $t.command -or $t.command.Count -eq 0) { continue }
        $exe = $t.command[0]
        if ($exe -like "*_NOT_BUILT*") { continue }
        if ($testExecutables -notcontains $exe) { $testExecutables += $exe }
    }
} catch {
    $testExecutables = @()
}

$runExecutables = @()
foreach ($exe in $testExecutables) {
    if ($target) {
        $base = [System.IO.Path]::GetFileNameWithoutExtension($exe)
        if ($base -ne $target) { continue }
    }
    if (Test-Path $exe) { $runExecutables += $exe }
}

if ($runExecutables.Count -eq 0) {
    if ($target) {
        $available = ($testExecutables | ForEach-Object { Split-Path -Leaf $_ } | Sort-Object -Unique) -join " "
        Fail "test target '$target' not found (available: $(if ($available) { $available } else { 'none' }))"
    }
    Fail "no test executables discovered under $buildDir"
}

# ---------------------------------------------------------------------------
# 3. Execute (or dry-run) every target, accumulating results in SQLite.
# ---------------------------------------------------------------------------

$dbPath = Join-Path $outputDir "$reportName.sqlite"
if (Test-Path $dbPath) { Remove-Item -Force $dbPath }

$runnerArgs = @(
    "--reporters", "SQLite",
    "--report-dir", $outputDir,
    "--report-name", $reportName,
    "--allow-surviving"
)
if ($generate) { $runnerArgs += "--dry-run" }
if ($timeout) { $runnerArgs += @("--timeout", $timeout) }

Write-Host ""
Write-Host "=== Running mull-runner on $($runExecutables.Count) test executable(s) ==="
foreach ($exe in $runExecutables) {
    Write-Host ""
    Write-Host "--- $(Split-Path -Leaf $exe) ---"
    & $mullRunner @runnerArgs $exe
    if ($LASTEXITCODE -ne 0) { exit 1 }
}

if (-not (Test-Path $dbPath)) { Fail "expected SQLite report at $dbPath was not created" }

# ---------------------------------------------------------------------------
# 4. Consolidate the aggregated database with mull-reporter.
# ---------------------------------------------------------------------------

$reporterCliArgs = @()
foreach ($reporter in $reporterList) {
    if ($reporter -eq "SQLite") {
        Write-Host "Note: SQLite database already written by mull-runner at $dbPath"
        continue
    }
    $reporterCliArgs += @("--reporters", $reporter)
}
if ($reporterCliArgs.Count -eq 0) { $reporterCliArgs = @("--reporters", "IDE") }

$reporterArgs = @("--report-dir", $outputDir, "--report-name", $reportName)
if ($threshold) {
    $reporterArgs += @("--mutation-score-threshold", $threshold)
} else {
    $reporterArgs += "--allow-surviving"
}

Write-Host ""
Write-Host "=== Generating consolidated reports ==="
& $mullReporter @reporterArgs @reporterCliArgs $dbPath
if ($LASTEXITCODE -ne 0) { exit 1 }

Write-Host ""
Write-Host "=== Mutation testing artifacts ==="
Write-Host "  database : $dbPath"
Write-Host "  reports  : $outputDir (report name: $reportName)"
exit 0
