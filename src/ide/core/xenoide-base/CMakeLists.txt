# virtual target that provides warning checks
set (target xenoide-base)
add_library(${target} INTERFACE)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${target} INTERFACE -Wall)
    target_compile_options(${target} INTERFACE -Wextra)
    target_compile_options(${target} INTERFACE -pedantic)

    if (XENOIDE_WARNINGS_AS_ERRORS)
        target_compile_options(${target} INTERFACE -Werror)
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(${target} INTERFACE /W4)
    target_compile_options(${target} INTERFACE /permissive-)

    if (XENOIDE_WARNINGS_AS_ERRORS)
        target_compile_options(${target} INTERFACE /WX)
    endif ()
endif()
