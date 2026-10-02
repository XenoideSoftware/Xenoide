@echo off
REM ============================================================================
REM 20-build-stage1.cmd
REM
REM Builds clang + lld 7.0.1 with TDM-GCC-5 (32-bit). Output: stage1 clang/lld
REM hosted on i686-w64-mingw32, capable of cross-targeting x86_64-w64-mingw32.
REM
REM Notes:
REM   - LLVM_PARALLEL_LINK_JOBS=1: TDM ld.bfd is 32-bit, can OOM during link.
REM   - LLVM_OPTIMIZED_TABLEGEN=ON: speeds up the long stage1 link cycle.
REM   - LLVM_BUILD_LLVM_DYLIB=OFF: MinGW's DLL export of LLVM internals is
REM     historically unreliable; keep it static here.
REM ============================================================================

if "%BOOTSTRAP_ROOT%"=="" (
    echo error: env not initialized. run "call env\stage0-tdm.cmd" first 1>&2
    exit /b 1
)

set "LLVM_SRC=%BOOTSTRAP_SRC%\llvm-project"
set "BUILD_DIR=%BOOTSTRAP_PREFIX%\_build\stage1"

if not exist "%LLVM_SRC%\llvm\CMakeLists.txt" (
    echo error: llvm sources not found at %LLVM_SRC% 1>&2
    echo        run scripts\00-fetch-sources.cmd first 1>&2
    exit /b 1
)

if not exist "%MINGW_SYSROOT%\i686-w64-mingw32\include\windows.h" (
    echo error: i686 mingw sysroot missing 1>&2
    echo        run scripts\10-build-mingw-sysroot.cmd first 1>&2
    exit /b 1
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo [20] configuring stage1 in %BUILD_DIR%
cmake -S "%LLVM_SRC%\llvm" -B "%BUILD_DIR%" ^
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_C_COMPILER="%CC%" ^
    -DCMAKE_CXX_COMPILER="%CXX%" ^
    -DCMAKE_INSTALL_PREFIX="%STAGE1%" ^
    -DLLVM_ENABLE_PROJECTS="clang;lld" ^
    -DLLVM_TARGETS_TO_BUILD=X86 ^
    -DLLVM_DEFAULT_TARGET_TRIPLE=i686-w64-mingw32 ^
    -DLLVM_HOST_TRIPLE=i686-w64-mingw32 ^
    -DLLVM_OPTIMIZED_TABLEGEN=ON ^
    -DLLVM_PARALLEL_LINK_JOBS=1 ^
    -DLLVM_ENABLE_ASSERTIONS=OFF ^
    -DLLVM_INCLUDE_TESTS=OFF ^
    -DLLVM_INCLUDE_EXAMPLES=OFF ^
    -DLLVM_INCLUDE_BENCHMARKS=OFF ^
    -DLLVM_BUILD_LLVM_DYLIB=OFF ^
    -DLLVM_ENABLE_LIBXML2=OFF ^
    -DLLVM_ENABLE_ZLIB=OFF ^
    -DLLVM_ENABLE_TERMINFO=OFF ^
    -DLLVM_ENABLE_LIBEDIT=OFF ^
    -DCLANG_DEFAULT_LINKER=lld
if errorlevel 1 (
    echo error: cmake configure failed 1>&2
    exit /b 1
)

echo [20] building stage1 (this is the slow step - 32-bit ld serializes links)
cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo error: stage1 build failed 1>&2
    exit /b 1
)

echo [20] installing stage1 to %STAGE1%
cmake --install "%BUILD_DIR%"
if errorlevel 1 (
    echo error: stage1 install failed 1>&2
    exit /b 1
)

echo [20] stage1 ready: "%STAGE1%\bin\clang.exe"
exit /b 0
