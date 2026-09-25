set (target "xenoide-qt6")

set (sources
        "src/xenoide-qt6-main.cpp"
        "src/QXenoideMainWindow.h"
        "src/QXenoideMainWindow.cpp"
)

add_executable(${target} ${sources})

target_include_directories(${target} PRIVATE "./src")
target_link_libraries(${target} PRIVATE xenoide-ui-codeeditor)

install (TARGETS ${target} 
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
