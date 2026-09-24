# sanitizer target
set (target xenoide-sanitize)
add_library(${target} INTERFACE)
if (XENOIDE_SANITIZER)
    target_compile_options(${target} INTERFACE -fsanitize=${XENOIDE_SANITIZER})
    target_link_options(${target} INTERFACE -fsanitize=${XENOIDE_SANITIZER})
endif()
