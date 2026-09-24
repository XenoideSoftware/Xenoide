
# TODO: These don't build either of a missing std::filesystem reference
add_subdirectory("xenoide-core")
# add_subdirectory("xenoide-ui-codeeditor")
# add_subdirectory("xenoide-ui-folderbrowser")

if (XE_ENABLE_IDE_WINLAMB)
    add_subdirectory("winlambxe")
    # add_subdirectory("xenoide9x")
    # add_subdirectory("xenoide10x")
    add_subdirectory("xenoidew")
endif ()

if (XENOIDE_UI_QT6)
    add_subdirectory("xenoide-qt6")
endif()

if (XENOIDE_UI_WX)
    set(CMAKE_FOLDER "IDE")
    add_subdirectory("xenoide-wx3")
endif()

if (XENOIDE_TESTS)
    set(CMAKE_FOLDER "IDE/Tests")
    add_subdirectory("xenoide-test")
    set(CMAKE_FOLDER "IDE")
endif()
