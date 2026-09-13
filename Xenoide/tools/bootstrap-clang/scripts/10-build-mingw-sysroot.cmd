@echo off
REM ============================================================================
REM 10-build-mingw-sysroot.cmd
REM
REM Stages the i686 mingw-w64 sysroot by mirroring TDM-GCC-5's own runtime
REM into prefix\mingw-sysroot\i686-w64-mingw32\. No downloads.
REM
REM This bootstrap is i686-only (Win9x / Win98 era target). x86_64 is not in
REM scope - if you need it, it's a separate facility.
REM ============================================================================

if "%BOOTSTRAP_ROOT%"=="" (
    echo error: env not initialized. run "call env\stage0-tdm.cmd" first 1>&2
    exit /b 1
)

set "I686_DST=%MINGW_SYSROOT%\i686-w64-mingw32"

if exist "%I686_DST%\include\windows.h" (
    echo [10] i686 sysroot already staged - skipping
    goto :done
)

echo [10] staging i686 sysroot from %TDM_GCC_ROOT%

if not exist "%I686_DST%" mkdir "%I686_DST%"

REM TDM-GCC-32 layout: top-level include/ + lib/ hold the mingw-w64 runtime,
REM and i686-w64-mingw32/ holds the target-specific subtree. Mirror both into
REM the staged sysroot so clang's mingw driver finds headers and libs at
REM either lookup location.
for %%D in (include lib i686-w64-mingw32) do (
    if exist "%TDM_GCC_ROOT%\%%D" (
        echo [10]   xcopy %%D
        xcopy /E /I /Q /Y "%TDM_GCC_ROOT%\%%D" "%I686_DST%\%%D" >nul
        if errorlevel 1 (
            echo error: failed to copy %TDM_GCC_ROOT%\%%D 1>&2
            exit /b 1
        )
    ) else (
        echo warning: %TDM_GCC_ROOT%\%%D not present - skipping 1>&2
    )
)

REM Mirror libgcc / libstdc++ static archives from gcc's lib/gcc tree so
REM clang can link a self-contained C++ binary without -L overrides.
if exist "%TDM_GCC_ROOT%\lib\gcc" (
    echo [10]   xcopy lib\gcc
    xcopy /E /I /Q /Y "%TDM_GCC_ROOT%\lib\gcc" "%I686_DST%\lib\gcc" >nul
    if errorlevel 1 (
        echo error: failed to copy lib\gcc 1>&2
        exit /b 1
    )
)

:done
echo [10] i686 sysroot ready at %I686_DST%
exit /b 0
