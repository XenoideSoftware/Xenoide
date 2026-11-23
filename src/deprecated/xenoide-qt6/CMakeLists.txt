set (target "XenoideQt")

set (sources
        "src/main.cpp"

        "src/MainWindowQt.h"
        "src/MainWindowQt.cpp"

        "src/ActionBusQt.h"
        "src/ActionBusQt.cpp"

        "src/UtilitiesQt.h"
        "src/UtilitiesQt.cpp"

        "src/dialogs/FindReplaceDialogQt.h"
        "src/dialogs/FindReplaceDialogQt.cpp"

        "src/dialogs/DialogManagerQt.h"
        "src/dialogs/DialogManagerQt.cpp"

        "src/dialogs/FileSearchDialogQt.h"
        "src/dialogs/FileSearchDialogQt.cpp"

        "src/dialogs/FileSearchDialogItemWidgetQt.h"
        "src/dialogs/FileSearchDialogItemWidgetQt.cpp"

        "src/widgets/FolderBrowserQt.h"
        "src/widgets/FolderBrowserQt.cpp"

        "src/widgets/DocumentManagerMdiQt.h"
        "src/widgets/DocumentManagerMdiQt.cpp"

        "src/widgets/DocumentMdiSubWindowQt.h"
        "src/widgets/DocumentMdiSubWindowQt.cpp"
)

add_executable(${target} ${sources})

target_include_directories(${target} PRIVATE "./src")
target_link_libraries(${target} PRIVATE scintilla Qt6::Core Qt6::Widgets Qt6::Gui xenoide-ui)

if(WIN32)
#    qt_deploy_runtime_dependencies(EXECUTABLE ${CMAKE_BINARY_DIR}/bin/${target})
endif()

install (TARGETS ${target} 
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
