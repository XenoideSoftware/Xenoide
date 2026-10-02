
set (target xenoide-wx3)
set (sources
    main.cpp
    MainWindow.cpp MainWindow.h
    FolderExplorerPanel.cpp FolderExplorerPanel.h
    OutputLogPanel.cpp OutputLogPanel.h
    WxTextCtrlSink.h
    LanguageConfig.cpp LanguageConfig.h
    FindReplaceDialog.cpp FindReplaceDialog.h
    FileSearchDialog.cpp FileSearchDialog.h
    Xenoide.manifest resources.xrc
)

configure_file(resources.xrc ${CMAKE_CURRENT_BINARY_DIR}/resources.xrc COPYONLY)

add_executable(${target} WIN32 ${sources})

target_link_libraries (${target} PRIVATE xenoide-base)
target_link_libraries(${target} PRIVATE wxWidgets::wxWidgets)
target_link_libraries(${target} PRIVATE xenoide-core)
