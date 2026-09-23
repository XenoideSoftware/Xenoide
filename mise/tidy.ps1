param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration
)

$ErrorActionPreference = "Stop"

$BuildDir = "build-tidy/$Configuration"

if (-not (Test-Path "$BuildDir/compile_commands.json")) {
    Write-Host "Error: compile_commands.json not found in $BuildDir."
    Write-Host "Please run 'mise run configure:tidy:$($Configuration.ToLower())' first."
    exit 1
}

Write-Host "=== Running clang-tidy ($Configuration) ==="

# Build run-clang-tidy arguments
$RctArgs = @("-p", $BuildDir, "-use-color")

$fix = if ($env:usage_fix) { $env:usage_fix } else { "false" }
$full = if ($env:usage_full) { $env:usage_full } else { "false" }
$format = if ($env:usage_format) { $env:usage_format } else { "false" }

if ($fix -eq "true") {
    $RctArgs += "-fix"

    if ($format -eq "true") {
        $RctArgs += "-format"
    }
}

if ($format -eq "true" -and $fix -ne "true") {
    Write-Host "Warning: --format has no effect without --fix. Ignoring."
}

if ($full -eq "true") {
    Write-Host "Running clang-tidy on all files in compile database ($Configuration)..."
    run-clang-tidy @RctArgs
} else {
    $files = git diff --name-only HEAD 2>$null |
        Where-Object { $_ -match '\.(cpp|h|hpp|c|cc|cxx)$' -and $_ -match '^src/' }

    if (-not $files -or $files.Count -eq 0) {
        Write-Host "No modified C++ source files found. Nothing to tidy."
        Write-Host "Tip: Use --full to check all files."
        exit 0
    }

    # Convert file list to a regex alternation for run-clang-tidy
    $escaped = $files | ForEach-Object { [regex]::Escape($_) }
    $fileRegex = $escaped -join '|'

    Write-Host "Running clang-tidy on $($files.Count) modified file(s) ($Configuration)..."
    run-clang-tidy @RctArgs $fileRegex
}