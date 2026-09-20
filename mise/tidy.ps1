$ErrorActionPreference = "Stop"

$compileDb = Get-ChildItem -Path "build","build-*" -Recurse -Filter "compile_commands.json" -ErrorAction SilentlyContinue | Select-Object -First 1 -ExpandProperty FullName

if (-not $compileDb) {
    Write-Host "Error: compile_commands.json not found in build directories. Please build the project first."
    exit 1
}

$compileDir = Split-Path $compileDb -Parent

if ($Args.Count -gt 0) {
    clang-tidy -p $compileDir $Args
} else {
    $files = git diff --name-only HEAD 2>$null | Where-Object { $_ -match '\.(cpp|h|hpp|c|cc|cxx)$' -and $_ -match '^src/' }

    if (-not $files) {
        $files = Get-ChildItem -Path "src/" -File -Recurse -Include "*.cpp","*.cc","*.cxx" | Select-Object -ExpandProperty FullName
    }

    if ($files) {
        clang-tidy -p $compileDir $files
    } else {
        Write-Host "No C++ files found to tidy."
    }
}