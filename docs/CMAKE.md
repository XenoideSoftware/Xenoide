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
    target_link_library(${target} PUBLIC library::component)
    target_link_library(${target} PUBLIC library::component1)
```


## Static Library Target Specification

```cmake
    find_package(Dependency REQUIRED)

    set (target "library-name")
    set (sources "src/source.cpp" "src/source2.cpp" ...)

    add_library(${target} ${sources})

    target_include_directories(${target} PUBLIC "src")

    # one line per dependency
    target_link_library(${target} PUBLIC library::component)
    target_link_library(${target} PUBLIC library::component1)
```

## Catch2 v3 Executable Target Specification
```cmake
    find_package(Catch2 REQUIRED)
    find_package(Dependency REQUIRED)

    set (target "library-name-test")
    set (sources "src/source.cpp" "src/source2.cpp" ...)

    add_library(${target} ${sources})

    target_include_directories(${target} PUBLIC "src")

    # one line per dependency
    target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
    target_link_library(${target} PUBLIC library::component)
    target_link_library(${target} PUBLIC library::component1)
    target_link_library(${target} PUBLIC library-name)

    # Enable autodiscovering    
    include(Catch)
    catch_discover_tests(${target})
```

## TODOs
- Formalize this guidelines in a kind of CMake checker / formatter
