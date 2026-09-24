set(target "winlambxe")

set(XE_IDE_WIN32_TARGET "10" CACHE STRING
    "Target Windows OS version (9x, xp, vista, 7, 8, 10, 11)")

set_property(CACHE XE_IDE_WIN32_TARGET PROPERTY STRINGS
    "9x" "xp" "vista" "7" "8" "10" "11")

if (XE_IDE_WIN32_TARGET STREQUAL "9x")
    set(XE_IDE_WIN32_TARGET_HEX 0x0410)
elseif (XE_IDE_WIN32_TARGET STREQUAL "xp")
    set(XE_IDE_WIN32_TARGET_HEX 0x0501)
elseif (XE_IDE_WIN32_TARGET STREQUAL "vista")
    set(XE_IDE_WIN32_TARGET_HEX 0x0600)
elseif (XE_IDE_WIN32_TARGET STREQUAL "7")
    set(XE_IDE_WIN32_TARGET_HEX 0x0601)
elseif (XE_IDE_WIN32_TARGET STREQUAL "8")
    set(XE_IDE_WIN32_TARGET_HEX 0x0602)
elseif (XE_IDE_WIN32_TARGET STREQUAL "10")
    set(XE_IDE_WIN32_TARGET_HEX 0x0A00)
elseif (XE_IDE_WIN32_TARGET STREQUAL "11")
    set(XE_IDE_WIN32_TARGET_HEX 0x0A00)
else()
    message(WARNING
        "Xenoide: XE_IDE_WIN32_TARGET ("${XE_IDE_WIN32_TARGET}") must be one of: 9x, xp, vista, 7, 8, 10 or 11")
endif()

message(STATUS "Xenoide: Configuring for Windows ${XE_IDE_WIN32_TARGET} target (${XE_IDE_WIN32_TARGET_HEX}). Unicode: ${XE_ENABLE_IDE_WINLAMB_UNICODE}")

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

message(STATUS "Xenoide: XE_IDE_WIN32_TARGET_HEX: ${XE_IDE_WIN32_TARGET_HEX}")

target_compile_definitions(${target} PUBLIC
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    WINVER=${XE_IDE_WIN32_TARGET_HEX}
    _WIN32_WINNT=${XE_IDE_WIN32_TARGET_HEX}
    _WIN32_WINDOWS=${XE_IDE_WIN32_TARGET_HEX}
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
