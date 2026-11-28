set (target "xenoide-qt6")

set (sources
    "src/xenoide-qt6-main.cpp"

    "src/MainWindowQt.h"
    "src/MainWindowQt.cpp"

    "src/FindReplaceDialogQt.h"
    "src/FindReplaceDialogQt.cpp"

    "src/FileSearchDialogQt.h"
    "src/FileSearchDialogQt.cpp"

    "src/FileSearchDialogItemWidgetQt.h"
    "src/FileSearchDialogItemWidgetQt.cpp"

    "src/FolderBrowserQt.h"
    "src/FolderBrowserQt.cpp"

    "src/DocumentManagerMdiQt.h"
    "src/DocumentManagerMdiQt.cpp"

    "src/DocumentMdiSubWindowQt.h"
    "src/DocumentMdiSubWindowQt.cpp"
)

add_executable(${target} ${sources})

target_include_directories(${target} PRIVATE "./src")

target_link_libraries(${target} PRIVATE Qt6::Core)
target_link_libraries(${target} PRIVATE Qt6::Widgets)
target_link_libraries(${target} PRIVATE Qt6::Gui)

install (TARGETS ${target} 
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
