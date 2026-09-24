
set (target Xenoide9X)

message("configuring resource")
configure_file(resources.rc.in ${CMAKE_CURRENT_BINARY_DIR}/resources.rc @ONLY)

set (sources
    main.cpp
    MainWindow.cpp MainWindow.h
    ${CMAKE_CURRENT_BINARY_DIR}/resources.rc
)

find_package(scintilla3 REQUIRED)

add_executable(${target} WIN32 ${sources})

# The source directory must be on the RC compiler's include path so that
# resource.h is found by the generated resources.rc in the binary directory.
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${target} PUBLIC scintilla3)
target_link_libraries(${target} PRIVATE comdlg32)
target_compile_definitions (${target} PUBLIC "WINVER=0x0400")
