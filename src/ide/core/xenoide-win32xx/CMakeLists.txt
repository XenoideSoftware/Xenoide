
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(target xenoide-win32xx)

add_executable(${target} WIN32
    src/main.cpp
    src/main_window.h
    src/main_window.cpp
    src/documents_view.h
    src/documents_view.cpp
    src/left_panel.h
    src/left_panel.cpp
    src/tab_page_richtext.h
    src/tab_page_richtext.cpp

    src/resource.h
    src/app.rc
)

target_compile_definitions(${target} PRIVATE
    UNICODE
    _UNICODE
)

target_link_libraries(${target} PRIVATE
    win32xx::win32xx
)

if (WIN32)
    target_link_libraries(${target} PRIVATE
        advapi32
        ole32
    )
endif()
