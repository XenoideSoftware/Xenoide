@echo off
REM ============================================================================
REM 30-build-stage2.cmd
REM
REM Rebuilds clang + lld 7.0.1 using stage1's clang and lld. Same source tree,
REM same target triples - the point is to validate stage1 works as a C++
REM compiler, not just that GCC-5 could compile the LLVM tree.
REM
REM With lld in play we drop LLVM_PARALLEL_LINK_JOBS=1; lld handles parallel
REM links without the 32-bit ld memory wall.
REM ============================================================================

if "%BOOTSTRAP_ROOT%"=="" (
    echo error: env not initialized. run "call env\stage0-tdm.cmd" first 1>&2
    exit /b 1
)

if not exist "%STAGE1%\bin\clang.exe" (
    echo error: stage1 clang missing at %STAGE1%\bin\clang.exe 1>&2
    echo        run scripts\20-build-stage1.cmd first 1>&2
    exit /b 1
)

set "LLVM_SRC=%BOOTSTRAP_SRC%\llvm-project"
set "BUILD_DIR=%BOOTSTRAP_PREFIX%\_build\stage2"
set "I686_SYSROOT=%MINGW_SYSROOT%\i686-w64-mingw32"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Point the stage1 clang at the staged i686 sysroot so it picks up mingw
REM headers/libs instead of guessing TDM's install path. --target keeps the
REM driver consistent with the host triple.
set "STAGE2_CFLAGS=--target=i686-w64-mingw32 --sysroot=%I686_SYSROOT%"

echo [30] configuring stage2 in %BUILD_DIR%
cmake -S "%LLVM_SRC%\llvm" -B "%BUILD_DIR%" ^
    -G Ninja ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_C_COMPILER="%STAGE1%\bin\clang.exe" ^
    -DCMAKE_CXX_COMPILER="%STAGE1%\bin\clang++.exe" ^
    -DCMAKE_C_FLAGS="%STAGE2_CFLAGS%" ^
    -DCMAKE_CXX_FLAGS="%STAGE2_CFLAGS%" ^
    -DCMAKE_INSTALL_PREFIX="%STAGE2%" ^
    -DLLVM_USE_LINKER=lld ^
    -DLLVM_ENABLE_PROJECTS="clang;lld" ^
    -DLLVM_TARGETS_TO_BUILD=X86 ^
    -DLLVM_DEFAULT_TARGET_TRIPLE=i686-w64-mingw32 ^
    -DLLVM_HOST_TRIPLE=i686-w64-mingw32 ^
    -DLLVM_OPTIMIZED_TABLEGEN=ON ^
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

echo [30] building stage2
cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo error: stage2 build failed 1>&2
    exit /b 1
)

echo [30] installing stage2 to %STAGE2%
cmake --install "%BUILD_DIR%"
if errorlevel 1 (
    echo error: stage2 install failed 1>&2
    exit /b 1
)

echo [30] stage2 ready: "%STAGE2%\bin\clang.exe"
exit /b 0
