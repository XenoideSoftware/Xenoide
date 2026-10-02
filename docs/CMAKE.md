# CMake guidelines

- A single CMake target should be stored in a given folder. 
- Target name should have the same name as the folder.
- A Target can exist in either `src/engine` or `src/ide`. 

## Basic Folder Structure

```
xe-executable-name
    src
    CMakeLists.txt
```

## Executable Target Specification


```cmake
    find_package(Dependency REQUIRED)

    set (target "executable-name")
    set (sources "src/source.cpp" "src/source2.cpp" ...)

    add_executable(${target} ${sources})

    # one line per dependency
    target_link_libraries(${target} PUBLIC library::component)
    target_link_libraries(${target} PUBLIC library::component1)
```


## Static Library Target Specification

```cmake
    find_package(Dependency REQUIRED)

    set (target "libprefix-name")
    set (sources "src/source.cpp" "src/source2.cpp" ...)

    add_library(${target} ${sources})
    add_library(prefix::library-name ALIAS ${target})

    target_include_directories(${target} PUBLIC "src")

    # one line per dependency
    target_link_libraries(${target} PUBLIC library::component)
    target_link_libraries(${target} PUBLIC library::component1)
```

## Catch2 v3 Executable Target Specification
```cmake
    find_package(Catch2 REQUIRED)
    find_package(Dependency REQUIRED)

    set (target "prefix-library-name-test")
    set (sources "src/source.cpp" "src/source2.cpp" ...)

    add_executable(${target} ${sources})

    target_include_directories(${target} PUBLIC "src")

    # one line per dependency
    target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
    target_link_libraries(${target} PUBLIC library::component)
    target_link_libraries(${target} PUBLIC library::component1)
    target_link_libraries(${target} PUBLIC library-name)

    # Enable autodiscovering    
    include(Catch)
    catch_discover_tests(${target})
```

## Style Checker
CMake files under `src/engine` are validated by the in-tree `cmake-checker`; see `docs/plans/CMAKE_STYLE_CHECKER_V2.2.md` ->    WE ARE REFINING AND IMPLEMENTING THIS PLAN!

