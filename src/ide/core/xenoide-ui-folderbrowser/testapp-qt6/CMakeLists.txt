
set (target "xenoide-ui-folderbrowser-testapp-qt6")
set (sources  "main.cpp")

add_executable(${target} ${sources})

target_link_libraries(${target} PRIVATE xenoide-ui-folderbrowser)

install (TARGETS ${target}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
