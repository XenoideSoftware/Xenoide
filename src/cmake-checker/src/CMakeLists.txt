
add_subdirectory(libcmake-checker-core)
add_subdirectory(cmake-checker)

if (XE_ENABLE_TESTING)
    add_subdirectory(cmake-checker-test)
endif()
