
add_subdirectory("base-target")

if (XE_ENABLE_ENGINE)
    add_subdirectory("engine")
endif()

if (XE_IDE)
    add_subdirectory("ide")
endif()
