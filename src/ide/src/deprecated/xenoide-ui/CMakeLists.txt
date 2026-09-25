
set (target "xenoide-ui")

set (sources
        "include/xenoide/ui/ActionBus.h"
    "include/xenoide/ui/StringUtil.h"
    "include/xenoide/ui/FindReplaceDialog.h"
        "include/xenoide/ui/FindReplaceDialogModel.h"
        "include/xenoide/ui/FindReplaceDialogPresenter.h"
    "include/xenoide/ui/Document.h"
    "include/xenoide/ui/DocumentModel.h"
    "include/xenoide/ui/DocumentPresenter.h"
    "include/xenoide/ui/DocumentManager.h"
    "include/xenoide/ui/DocumentManagerModel.h"
    "include/xenoide/ui/DocumentManagerPresenter.h"
    "include/xenoide/ui/FolderBrowser.h"
        "include/xenoide/ui/FolderBrowserModel.h"
        "include/xenoide/ui/FolderBrowserPresenter.h"
    "include/xenoide/ui/IDEFrame.h"
    "include/xenoide/ui/IDEFrameModel.h"
    "include/xenoide/ui/IDEFramePresenter.h"
    "include/xenoide/ui/FileSearchDialog.h"
        "include/xenoide/ui/FileSearchDialogModel.h"
        "include/xenoide/ui/FileSearchDialogPresenter.h"
    "include/xenoide/ui/DialogManager.h"
    "include/xenoide/ui/Menu.h"
    "include/xenoide/ui/MenuPanel.h"
    "include/xenoide/ui/FileFilter.h"
    "include/xenoide/ui/ObjectTreeView.h"
    "include/xenoide/ui/TreeManagerController.h"
    "include/xenoide/ui/TreeManagerControllerFileSystem.h"

        "src/ActionBus.cpp"
    "src/StringUtil.cpp"
    "src/FindReplaceDialog.cpp"
        "src/FindReplaceDialogModel.cpp"
        "src/FindReplaceDialogPresenter.cpp"
    "src/Document.cpp"
    "src/DocumentModel.cpp"
    "src/DocumentPresenter.cpp"
    "src/DocumentManager.cpp"
    "src/DocumentManagerModel.cpp"
    "src/DocumentManagerPresenter.cpp"
    "src/FolderBrowser.cpp"
        "src/FolderBrowserModel.cpp"
        "src/FolderBrowserPresenter.cpp"
    "src/IDEFrame.cpp"
    "src/IDEFrameModel.cpp"
    "src/IDEFramePresenter.cpp"
    "src/FileSearchDialog.cpp"
        "src/FileSearchDialogModel.cpp"
        "src/FileSearchDialogPresenter.cpp"
    "src/DialogManager.cpp"
    "src/Menu.cpp"
    "src/MenuPanel.cpp"
    "src/FileFilter.cpp"
    "src/ObjectTreeView.cpp"
    "src/TreeManagerControllerFileSystem.cpp"
)

add_library (${target} ${sources})

target_include_directories (${target} PUBLIC "include/")
target_compile_definitions (${target} PRIVATE "XENOIDE_BUILD")

if (UNIX)
    set (DL_LIBRARY dl)
endif ()

target_link_libraries (
    ${target} 
    ${DL_LIBRARY}
    xenoide-core
)

install(TARGETS ${target} DESTINATION "lib")
