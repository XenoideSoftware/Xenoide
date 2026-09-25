set (target xe-interface)

add_library(${target} INTERFACE)
add_library(xe::interface ALIAS ${target})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set (XE_CXXFLAGS_WALL -Wall -Wextra)
    set (XE_CXXFLAGS_WERROR -Werror)
    set (XE_CXXFLAGS_COVERAGE --coverage)
    set (XE_LDFLAGS_COVERAGE --coverage)

elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set (XE_CXXFLAGS_WALL -Wall -Wextra)
    set (XE_CXXFLAGS_WERROR -Werror)
    set (XE_CXXFLAGS_COVERAGE -fprofile-instr-generate -fcoverage-mapping)
    set (XE_LDFLAGS_COVERAGE -fprofile-instr-generate)

elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set (XE_CXXFLAGS_WALL /W4)
    set (XE_CXXFLAGS_WERROR /WX)
    set (XE_CXXFLAGS_COVERAGE )
    set (XE_LDFLAGS_COVERAGE )
endif()

target_compile_options(${target} INTERFACE ${XE_CXXFLAGS_WALL})
if (XE_ENABLE_WERROR)
    target_compile_options(${target} INTERFACE ${XE_CXXFLAGS_WERROR})
endif()

if (XE_ENABLE_COVERAGE)
    target_compile_options(${target} INTERFACE ${XE_CXXFLAGS_COVERAGE})
    target_link_options(${target} INTERFACE ${XE_LDFLAGS_COVERAGE})
endif() 

if (XE_SANITIZER)
    target_compile_options(${target} INTERFACE -fsanitize=${XE_SANITIZER})
    target_link_options(${target} INTERFACE -fsanitize=${XE_SANITIZER})
endif()
