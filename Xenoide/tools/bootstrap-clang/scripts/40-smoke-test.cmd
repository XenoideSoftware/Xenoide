@echo off
REM ============================================================================
REM 40-smoke-test.cmd
REM
REM Builds Hello World with stage2 clang for the legacy i686 target. Sets the
REM PE subsystem floor to Windows 9x (4.0) so the produced binary advertises
REM compatibility back to Win95/98/Me - which is the whole point of this
REM toolchain.
REM ============================================================================

if "%BOOTSTRAP_ROOT%"=="" (
    echo error: env not initialized. run "call env\stage0-tdm.cmd" first 1>&2
    exit /b 1
)

if not exist "%STAGE2%\bin\clang.exe" (
    echo error: stage2 clang missing - run scripts\30-build-stage2.cmd first 1>&2
    exit /b 1
)

set "TRIPLE=i686-w64-mingw32"
set "SR=%MINGW_SYSROOT%\%TRIPLE%"

if not exist "%SR%\include\windows.h" (
    echo error: i686 sysroot not staged at %SR% 1>&2
    echo        run scripts\10-build-mingw-sysroot.cmd first 1>&2
    exit /b 1
)

set "WORK=%BOOTSTRAP_PREFIX%\_smoke"
if not exist "%WORK%" mkdir "%WORK%"

set "HELLO=%WORK%\hello.c"
> "%HELLO%" echo #include ^<stdio.h^>
>>"%HELLO%" echo int main(void) { puts("hello, legacy"); return 0; }

set "OUT=%WORK%\hello-%TRIPLE%.exe"

echo [40] %TRIPLE%: compile + link (Win9x subsystem floor 4.0)
"%STAGE2%\bin\clang.exe" --target=%TRIPLE% --sysroot="%SR%" -fuse-ld=lld ^
    -Wl,--subsystem,console:4.0 ^
    -Wl,--major-os-version,4 -Wl,--minor-os-version,0 ^
    -Wl,--major-subsystem-version,4 -Wl,--minor-subsystem-version,0 ^
    -o "%OUT%" "%HELLO%"
if errorlevel 1 (
    echo error: build failed 1>&2
    exit /b 1
)

echo [40] running %OUT%
"%OUT%"
if errorlevel 1 (
    echo error: produced binary did not exit cleanly 1>&2
    exit /b 1
)

REM Best-effort PE subsystem version probe. Win9x targets should report 4.x.
where objdump >nul 2>&1
if not errorlevel 1 (
    echo [40] PE header probe:
    objdump -p "%OUT%" | findstr /R /C:"MajorOSystemVersion" /C:"MinorOSystemVersion" /C:"MajorSubsystemVersion" /C:"MinorSubsystemVersion"
)

echo [40] smoke test passed
exit /b 0
