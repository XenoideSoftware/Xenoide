@echo off
REM ============================================================================
REM 00-fetch-sources.cmd
REM
REM Shallow-clones llvm-project at the pinned tag, applies any local patches,
REM and prepares src/. The mingw-w64 sysroot fetch is handled by 10-...
REM ============================================================================

if "%BOOTSTRAP_ROOT%"=="" (
    echo error: env not initialized. run "call env\stage0-tdm.cmd" first 1>&2
    exit /b 1
)

set "LLVM_TAG=llvmorg-7.0.1"
set "LLVM_REPO=https://github.com/llvm/llvm-project.git"
set "LLVM_SRC=%BOOTSTRAP_SRC%\llvm-project"

if exist "%LLVM_SRC%\.git" (
    echo [00] llvm-project already cloned at %LLVM_SRC% - skipping
    goto :patches
)

echo [00] cloning %LLVM_REPO% @ %LLVM_TAG% (shallow)
git clone --depth 1 --branch "%LLVM_TAG%" "%LLVM_REPO%" "%LLVM_SRC%"
if errorlevel 1 (
    echo error: git clone failed 1>&2
    exit /b 1
)

:patches
set "PATCHES_DIR=%BOOTSTRAP_ROOT%\patches"
if not exist "%PATCHES_DIR%" (
    echo [00] no patches/ directory - nothing to apply
    goto :done
)

REM Apply patches in lexical order. Re-running is safe: git apply --check
REM gates each patch against the current tree state before applying.
echo [00] applying patches from %PATCHES_DIR%
pushd "%LLVM_SRC%" >nul
for %%P in ("%PATCHES_DIR%\*.patch") do (
    git apply --check "%%P" >nul 2>&1
    if errorlevel 1 (
        echo [00]   skip "%%~nxP" (already applied or does not apply cleanly^)
    ) else (
        echo [00]   apply "%%~nxP"
        git apply "%%P"
        if errorlevel 1 (
            popd >nul
            echo error: patch "%%~nxP" failed mid-apply 1>&2
            exit /b 1
        )
    )
)
popd >nul

:done
echo [00] sources ready under %LLVM_SRC%
exit /b 0
