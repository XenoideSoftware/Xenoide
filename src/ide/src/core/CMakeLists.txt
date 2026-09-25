
add_subdirectory("libxenoide-core")

if (XE_ENABLE_TESTING)
    set(CMAKE_FOLDER "IDE/Tests")
    add_subdirectory("libxenoide-core-test")
    set(CMAKE_FOLDER "IDE")
endif()

# add_subdirectory("xenoide-ui-codeeditor")
# add_subdirectory("xenoide-ui-folderbrowser")
