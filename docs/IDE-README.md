
# Xenoide
IDE for Native App and Shader development.

Technical and architectural documentation can be found in the [docs](docs) folder.

## Build & Development (Mise)

We use [mise](https://mise.jdx.dev/) as our unified task runner. For complete documentation on all tasks and options, see [docs/MISE.md](docs/MISE.md).

### Build Steps

1. **Setup Dependencies**: Exports local custom recipes and installs dependencies via Conan:
   ```bash
   mise run setup            # Release (default)
   mise run install:debug      # Debug
   ```

2. **Configure**: Generates CMake build files:
   ```bash
   mise run configure        # Release (default)
   mise run configure:debug  # Debug
   ```

3. **Build**: Compiles binaries in parallel:
   ```bash
   mise run build            # Release (default)
   mise run build:debug      # Debug
   ```

## Conan (Manual Setup)

### One-time setup

#### 1. Install common configuration

Install the XenoideSoftware common Conan configuration (remotes, settings, profiles):

```bash
conan config install git@github.com:XenoideSoftware/conan-common.git
```

#### 2. Register the custom recipes

Three packages used by Xenoide are not in the standard conan-center-index. Clone the XenoideSoftware fork and export each recipe into your local Conan cache:

```bash
git clone git@github.com:XenoideSoftware/conan-center-index.git
conan export conan-center-index/recipes/scintilla/all --version 5.5.7
conan export conan-center-index/recipes/lexilla/all --version 5.4.6
conan export conan-center-index/recipes/lsp-framework/all --version 1.3.1
```

You can also use a local center-index:
```bash
git clone git@github.com:XenoideSoftware/conan-center-index.git
conan remote remove conancenter
conan remote add localcenter ./conan-center-index
```

For more information, visit https://docs.conan.io/2/devops/devops_local_recipes_index.html

### Install dependencies

Pick the profile that matches your toolchain. Available profiles (installed by `conan-common`):

| Profile | Platform |
|---|---|
| `armv8-macos-aclang-17` | macOS ARM64 (Apple Clang 17) |
| `x64-linux-gcc-13` | Linux x86-64 (GCC 13) |
| `x64-win-msvc-194` | Windows x86-64 (MSVC 19.4) |
| `x86-win-gcc-5.1` | Windows x86 (GCC 5.1) |

```bash
# Release build
conan install . --build=missing -s build_type=Release -pr:h <profile> -pr:b <profile>

# Debug build
conan install . --build=missing -s build_type=Debug -pr:h <profile> -pr:b <profile>
```

For example, on macOS ARM64:

```bash
conan install . --build=missing -s build_type=Release -pr:h armv8-macos-aclang-17 -pr:b armv8-macos-aclang-17
```

### Configure and build

```bash
cmake --preset conan-default
cmake --build --preset conan-release  # or conan-debug
```

### Other commands
- `conan cache clean`: Removes temporaries from the cache
- `conan remove <package>`: Removes completely the given recipe and its binaries
- `conan run  .\build\Debug\src\xenoide9x\Xenoide9X.exe --profile=conan/profiles/windows-x86-gcc`: Executes a generated Executable
- `conan remove <package>`: Removes a recipe and its binaries from the local cache

## Troubleshooting:
- macOS: Requires a full Xcode installation (including the IDE?) for qtbase dependency
- macOS: To fix error `'strchrnul' is only available on macOS 15.4 or newer`, use the environment variable `export MACOSX_DEPLOYMENT_TARGET=15.4` before detecting the current system's toolchain profile, and running `conan create . --build=missing`

### macOS environment variables that affects the Conan command.
- `MACOSX_DEPLOYMENT_TARGET`: Sets the minimum macOS version that the binaries will run on (e.g., 10.15). Affects SDK and symbol availability.
- `SDKROOT`: Path to the macOS SDK to use (e.g., `/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk`). Overrides automatic SDK detection.
- `CONAN_CMAKE_OSX_DEPLOYMENT_TARGET`: Conan-specific variable to pass deployment target into CMake projects.
- `CONAN_CMAKE_OSX_ARCHITECTURES`: Controls target architecture in Conan recipes (e.g., `arm64`, `x86_64`, or `arm64;x86_64` for universal builds).
