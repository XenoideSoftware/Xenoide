
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (target xenoidew)

add_executable(${target} WIN32
    src/main.cpp
    src/main_window.h
    src/main_window.cpp
    src/find_dialog.h
    src/find_dialog.cpp
    src/find_replace_window.h
    src/find_replace_window.cpp
    src/main_window_controller.h
    src/main_window_controller.cpp
    src/main_window_model.h
    src/main_window_model.cpp
    src/left_panel.h
    src/left_panel.cpp
    src/tab_page_richtext.h
    src/tab_page_richtext.cpp
    src/splitter_test_window.h
    src/app_icons.h
    src/app_icons.cpp
    src/sci_editor.h
    src/sci_editor.cpp
    src/observer.h
    src/subject.h
    src/resource.h
    src/xenoidew.rc
)

target_link_libraries (${target} PRIVATE winlambxe)
target_link_libraries (${target} PRIVATE winlamb::manifest)
target_link_libraries (${target} PRIVATE xenoide-core)
target_link_libraries (${target} PRIVATE xenoide-base)
