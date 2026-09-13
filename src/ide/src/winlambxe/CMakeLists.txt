set(target "winlambxe")

find_package(scintilla3 REQUIRED)

set(sources_controls
    "src/winlambxe/richedit.h"
    "src/winlambxe/richedit.cpp"
    "src/winlambxe/scintilla.h"
    "src/winlambxe/scintilla.cpp"
    "src/winlambxe/tabctrl.h"
    "src/winlambxe/tabctrl.cpp"
    "src/winlambxe/toolbar.h"
    "src/winlambxe/toolbar.cpp"
    "src/winlambxe/webview.h"
    "src/winlambxe/webview.cpp"
)

set(sources_utils
    "src/winlambxe/utils/defer_window_pos.h"
    "src/winlambxe/utils/defer_window_pos.cpp"
    "src/winlambxe/utils/splitter_base.h"
    "src/winlambxe/utils/splitter_horizontal.h"
    "src/winlambxe/utils/splitter_horizontal.cpp"
    "src/winlambxe/utils/splitter_vertical.h"
    "src/winlambxe/utils/splitter_vertical.cpp"
    "src/winlambxe/utils/dock_window.h"
    "src/winlambxe/utils/dock_window.cpp"
    "src/winlambxe/utils/dock_layout.h"
    "src/winlambxe/utils/dock_layout.cpp"
    "src/winlambxe/utils/document_tabs.h"
    "src/winlambxe/utils/document_tabs.cpp"
    "src/winlambxe/utils/menu_icons.h"
    "src/winlambxe/utils/menu_icons.cpp"
    "src/winlambxe/utils/sys_brush.h"
)

add_library(${target}
    ${sources_controls}
    ${sources_utils}
)

target_include_directories(${target} PUBLIC "src/")

target_compile_definitions(${target} PUBLIC
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    WINVER=${XENOIDE_WIN32_TARGET_HEX}
    _WIN32_WINNT=${XENOIDE_WIN32_TARGET_HEX}
    _WIN32_WINDOWS=${XENOIDE_WIN32_TARGET_HEX}
)

if (XENOIDE_WIN32_UNICODE)
    target_compile_definitions(${target} PUBLIC UNICODE _UNICODE)
    # MinGW's default CRT entry point is WinMainCRTStartup (calls WinMain).
    # Under UNICODE, winlamb's RUN macro expands to wWinMain, which requires
    # the wWinMainCRTStartup entry — selected by passing -municode to the
    # compiler and linker driver.
    if (MINGW)
        target_compile_options(${target} PUBLIC -municode)
        target_link_options(${target} PUBLIC -municode)
    endif()
else()
    target_compile_definitions(${target} PUBLIC _MBCS)
endif()

target_link_libraries(${target} PUBLIC
    winlamb::winlamb
    scintilla3
    user32
    gdi32
    comctl32
    # mctrl::mctrl
    # win32-property-grid::win32-property-grid
    # $<TARGET_NAME_IF_EXISTS:HexCtrl::HexCtrl>
)

target_link_libraries (${target} PRIVATE xenoide-base)
