set(target "xenoide-ui-folderbrowser")

if (XENOIDE_UI_QT6)
    set(sources_qt6
        "src/xenoide/qt6/QFolderBrowser.h"
        "src/xenoide/qt6/QFolderBrowser.cpp"
    )
endif()

set(sources
    "src/xenoide/FolderBrowser.h"
    "src/xenoide/FolderBrowserPresenter.h"
    "src/xenoide/FolderBrowserPresenter.cpp"
    ${sources_qt6}
)

add_library(${target} ${sources})

target_include_directories(${target} PUBLIC "src/")
target_link_libraries(${target} PUBLIC xenoide-core)

if (XENOIDE_UI_QT6)
    target_link_libraries(${target} PUBLIC Qt6::Core Qt6::Widgets Qt6::Gui)
endif()

install(TARGETS ${target} DESTINATION "lib")

if (XENOIDE_TESTS)
    set(test_target ${target}-test)
    add_executable(${test_target} "test/${test_target}.cpp")
    target_link_libraries(${test_target} PRIVATE Catch2::Catch2WithMain ${target})
endif()

add_subdirectory(testapp-qt6)
