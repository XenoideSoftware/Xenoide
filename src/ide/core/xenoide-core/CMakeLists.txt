
find_package(gsl-lite CONFIG REQUIRED)

set (target "xenoide-core")

set (sources
    "src/xenoide/core/Predef.h"
    "src/xenoide/core/FileService.h"
    "src/xenoide/core/FileService.cpp"
    "src/xenoide/core/StringUtil.h"
    "src/xenoide/core/StringUtil.cpp"
) 

add_library(${target} ${sources})

target_include_directories( ${target} PUBLIC "src/")
target_link_libraries (${target} PUBLIC gsl-lite::gsl-lite)

install(TARGETS ${target} DESTINATION "lib")
