set (target "libxe-cmake-checker-script")

find_package(chaiscript CONFIG REQUIRED)

# ChaiScript is a template-heavy header-only library that trips -Werror on
# stock build configurations; treat its headers as system headers.
if (TARGET chaiscript::chaiscript)
    get_target_property(_chai_includes chaiscript::chaiscript INTERFACE_INCLUDE_DIRECTORIES)
    if (NOT _chai_includes)
        set_property(TARGET chaiscript::chaiscript PROPERTY INTERFACE_INCLUDE_DIRECTORIES
            "${chaiscript_INCLUDE_DIRS_RELEASE}")
    endif()
    set_property(TARGET chaiscript::chaiscript PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
        "${chaiscript_INCLUDE_DIRS_RELEASE}")
endif()

set (headers
    "src/xe/cmake/script/ScriptEngineFacade.h"
    "src/xe/cmake/script/ScriptBindings.h"
    "src/xe/cmake/script/ScriptRuleLoader.h"
)

set (sources
    ${headers}
    "src/xe/cmake/script/ScriptEngineFacade.cpp"
    "src/xe/cmake/script/ScriptBindings.cpp"
    "src/xe/cmake/script/ScriptRuleLoader.cpp"
)

add_library(${target} ${sources})
add_library(xe::cmake-checker-script ALIAS ${target})

# ChaiScript's operator-precedence table trips a known GCC/Clang 13
# -Warray-bounds false positive; suppress it locally (Werror is global).
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${target} PRIVATE -Wno-array-bounds)
endif()

target_include_directories(${target} PUBLIC "src")

# one line per dependency
target_link_libraries(${target} PUBLIC xe::cmake-checker-core)
target_link_libraries(${target} PUBLIC xe::cmake-checker-analysis)
target_link_libraries(${target} PRIVATE chaiscript::chaiscript)