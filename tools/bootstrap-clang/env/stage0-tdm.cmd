@echo off
REM ============================================================================
REM stage0-tdm.cmd - Detect TDM-GCC-5 (32-bit) and export bootstrap env vars.
REM
REM Call this from a stock cmd.exe BEFORE running any of the numbered scripts:
REM
REM     call env\stage0-tdm.cmd
REM
REM Override TDM_GCC_ROOT before calling if your install lives elsewhere:
REM
REM     set TDM_GCC_ROOT=D:\dev\TDM-GCC-32
REM     call env\stage0-tdm.cmd
REM
REM Variables exported to the caller:
REM   TDM_GCC_ROOT       absolute path to the TDM-GCC-5 install root
REM   BOOTSTRAP_ROOT     absolute path to tools\bootstrap-clang
REM   BOOTSTRAP_SRC      %BOOTSTRAP_ROOT%\src
REM   BOOTSTRAP_PREFIX   %BOOTSTRAP_ROOT%\prefix
REM   BOOTSTRAP_DL       %BOOTSTRAP_PREFIX%\_dl
REM   MINGW_SYSROOT      %BOOTSTRAP_PREFIX%\mingw-sysroot
REM   STAGE1             %BOOTSTRAP_PREFIX%\stage1
REM   STAGE2             %BOOTSTRAP_PREFIX%\stage2
REM   CC, CXX            point at TDM-GCC-5
REM   PATH               TDM-GCC-5\bin prepended (idempotent)
REM ============================================================================

if "%TDM_GCC_ROOT%"=="" set "TDM_GCC_ROOT=C:\TDM-GCC-32"

if not exist "%TDM_GCC_ROOT%\bin\gcc.exe" (
    echo error: gcc.exe not found under "%TDM_GCC_ROOT%\bin" 1>&2
    echo        set TDM_GCC_ROOT to your TDM-GCC-5 32-bit install root 1>&2
    exit /b 1
)

REM Anchor BOOTSTRAP_ROOT to this script's parent directory (env\ -> bootstrap-clang\)
for %%I in ("%~dp0..") do set "BOOTSTRAP_ROOT=%%~fI"

set "BOOTSTRAP_SRC=%BOOTSTRAP_ROOT%\src"
set "BOOTSTRAP_PREFIX=%BOOTSTRAP_ROOT%\prefix"
set "BOOTSTRAP_DL=%BOOTSTRAP_PREFIX%\_dl"
set "MINGW_SYSROOT=%BOOTSTRAP_PREFIX%\mingw-sysroot"
set "STAGE1=%BOOTSTRAP_PREFIX%\stage1"
set "STAGE2=%BOOTSTRAP_PREFIX%\stage2"

set "CC=%TDM_GCC_ROOT%\bin\gcc.exe"
set "CXX=%TDM_GCC_ROOT%\bin\g++.exe"

REM Idempotent PATH prepend - skip if TDM bin is already first.
echo %PATH% | findstr /b /c:"%TDM_GCC_ROOT%\bin" >nul
if errorlevel 1 set "PATH=%TDM_GCC_ROOT%\bin;%PATH%"

REM Verify required external tools are reachable.
for %%T in (git cmake ninja curl tar) do (
    where %%T >nul 2>&1
    if errorlevel 1 (
        echo error: required tool "%%T" not found on PATH 1>&2
        exit /b 1
    )
)

echo [stage0] TDM_GCC_ROOT     = %TDM_GCC_ROOT%
echo [stage0] BOOTSTRAP_ROOT   = %BOOTSTRAP_ROOT%
echo [stage0] BOOTSTRAP_PREFIX = %BOOTSTRAP_PREFIX%

if not exist "%BOOTSTRAP_SRC%"    mkdir "%BOOTSTRAP_SRC%"
if not exist "%BOOTSTRAP_PREFIX%" mkdir "%BOOTSTRAP_PREFIX%"
if not exist "%BOOTSTRAP_DL%"     mkdir "%BOOTSTRAP_DL%"
if not exist "%MINGW_SYSROOT%"    mkdir "%MINGW_SYSROOT%"

exit /b 0
