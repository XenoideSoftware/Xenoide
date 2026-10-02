@echo off
REM ============================================================================
REM 99-emit-conan-profile.cmd
REM
REM Writes a Conan 2 profile pointing at the stage2 clang/lld toolchain:
REM
REM   conan\profiles\windows-x86-clang-legacy   (i686-w64-mingw32, Win9x floor)
REM
REM Profile uses ${PROFILE_DIR} so absolute paths resolve relative to the
REM checkout, not the machine that ran the bootstrap.
REM ============================================================================

if "%BOOTSTRAP_ROOT%"=="" (
    echo error: env not initialized. run "call env\stage0-tdm.cmd" first 1>&2
    exit /b 1
)

if not exist "%STAGE2%\bin\clang.exe" (
    echo error: stage2 clang missing - run scripts\30-build-stage2.cmd first 1>&2
    exit /b 1
)

REM Resolve repo root: BOOTSTRAP_ROOT = <repo>\tools\bootstrap-clang
for %%I in ("%BOOTSTRAP_ROOT%\..\..") do set "REPO_ROOT=%%~fI"
set "PROFILES_DIR=%REPO_ROOT%\conan\profiles"

if not exist "%PROFILES_DIR%" (
    echo error: conan\profiles directory not found at %PROFILES_DIR% 1>&2
    exit /b 1
)

set "TRIPLE=i686-w64-mingw32"
set "OUT=%PROFILES_DIR%\windows-x86-clang-legacy"

setlocal EnableDelayedExpansion
set "PD=$"
set "PD=!PD!{PROFILE_DIR}"
set "STAGE2_REF=!PD!/../../tools/bootstrap-clang/prefix/stage2"
set "SYSROOT_REF=!PD!/../../tools/bootstrap-clang/prefix/mingw-sysroot/%TRIPLE%"

REM Win9x subsystem/OS floor baked into LDFLAGS so anything Conan builds with
REM this profile inherits Win98-era PE compatibility.
set "WIN9X_LDFLAGS=-Wl,--subsystem,console:4.0 -Wl,--major-os-version,4 -Wl,--minor-os-version,0 -Wl,--major-subsystem-version,4 -Wl,--minor-subsystem-version,0"

echo [99] writing %OUT%
> "%OUT%" echo [settings]
>>"%OUT%" echo os=Windows
>>"%OUT%" echo arch=x86
>>"%OUT%" echo build_type=Release
>>"%OUT%" echo compiler=clang
>>"%OUT%" echo compiler.version=7
>>"%OUT%" echo compiler.cppstd=gnu14
>>"%OUT%" echo compiler.libcxx=libstdc++11
>>"%OUT%" echo.
>>"%OUT%" echo [buildenv]
>>"%OUT%" echo PATH=+(path)!STAGE2_REF!/bin
>>"%OUT%" echo CC=clang
>>"%OUT%" echo CXX=clang++
>>"%OUT%" echo CFLAGS=--target=%TRIPLE% --sysroot=!SYSROOT_REF!
>>"%OUT%" echo CXXFLAGS=--target=%TRIPLE% --sysroot=!SYSROOT_REF!
>>"%OUT%" echo LDFLAGS=--target=%TRIPLE% --sysroot=!SYSROOT_REF! -fuse-ld=lld !WIN9X_LDFLAGS!
>>"%OUT%" echo.
>>"%OUT%" echo [conf]
>>"%OUT%" echo tools.build:compiler_executables={"c": "clang", "cpp": "clang++"}
>>"%OUT%" echo tools.cmake.cmaketoolchain:generator=Ninja
endlocal

echo [99] profile written: %OUT%
exit /b 0
