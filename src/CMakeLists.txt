
add_subdirectory("base-target")

if (XE_ENGINE_ENABLE)
    add_subdirectory("engine")
endif()

if (XE_IDE_ENABLE)
    add_subdirectory("ide")
endif()
