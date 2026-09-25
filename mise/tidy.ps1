param(
    [Parameter(Mandatory)]
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration,

    [string]$Project = "all"
)

$ErrorActionPreference = "Stop"

. "$PSScriptRoot\lib\common.ps1"

$fix = if ($env:usage_fix) { $env:usage_fix } else { "false" }
$full = if ($env:usage_full) { $env:usage_full } else { "false" }
$format = if ($env:usage_format) { $env:usage_format } else { "false" }
$file = if ($env:usage_file) { $env:usage_file } else { $null }

if ($file -and $full -eq "true") {
    Write-Host "Error: --file and --full are mutually exclusive."
    exit 1
}

if ($format -eq "true" -and $fix -ne "true") {
    Write-Host "Warning: --format has no effect without --fix. Ignoring."
}

$modified = @(git -C $RepoRoot diff --name-only HEAD 2>$null |
    Where-Object { $_ -match '\.(cpp|h|hpp|c|cc|cxx)$' })

foreach ($folder in @(Resolve-Projects $Project)) {
    $BuildDir = Join-Path $RepoRoot "$folder/build-tidy/$Configuration"

    if (-not (Test-Path "$BuildDir/compile_commands.json")) {
        Write-Host "Error: compile_commands.json not found in $BuildDir."
        Write-Host "Please run 'mise run configure:tidy:$($Configuration.ToLower())' first."
        exit 1
    }

    Write-Host ""
    Write-Host "=== [$folder] Running clang-tidy ($Configuration) ==="

    # Build run-clang-tidy arguments
    $RctArgs = @("-p", $BuildDir, "-use-color=1")

    if ($fix -eq "true") {
        $RctArgs += "-fix"

        if ($format -eq "true") {
            $RctArgs += "-format"
        }
    }

    if ($file) {
        $fileRegex = [regex]::Escape($file)

        $db = Get-Content "$BuildDir/compile_commands.json" -Raw
        if ($db -notmatch "\"file\": \".*$fileRegex\"") {
            Write-Host "Error: '$file' not found in compile database $BuildDir/compile_commands.json"
            exit 1
        }

        Write-Host "Running clang-tidy on $file ($Configuration)..."
        run-clang-tidy @RctArgs $fileRegex
    } elseif ($full -eq "true") {
        Write-Host "Running clang-tidy on all files in compile database ($Configuration)..."
        run-clang-tidy @RctArgs
    } else {
        $files = @($modified | Where-Object { $_ -match "^$([regex]::Escape($folder))/" })

        if ($files.Count -eq 0) {
            Write-Host "No modified C++ source files found in $folder. Nothing to tidy."
            Write-Host "Tip: Use --full to check all files."
            continue
        }

        # Convert file list to a regex alternation for run-clang-tidy
        $escaped = $files | ForEach-Object { [regex]::Escape($_) }
        $fileRegex = $escaped -join '|'

        Write-Host "Running clang-tidy on $($files.Count) modified file(s) ($Configuration)..."
        run-clang-tidy @RctArgs $fileRegex
    }
}
