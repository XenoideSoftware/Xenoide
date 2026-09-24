
set(CMAKE_FOLDER "Common")
add_subdirectory("base-target")

if (XE_ENABLE_ENGINE)
    set(CMAKE_FOLDER "Engine")
    add_subdirectory("engine")
endif()

if (XE_ENABLE_IDE)
    set(CMAKE_FOLDER "IDE")
    add_subdirectory("ide")
endif()

set(CMAKE_FOLDER "Pocs")
add_subdirectory("pocs")

unset(CMAKE_FOLDER)
