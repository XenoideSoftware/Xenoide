
option (XE_ENABLE_TESTING       "Enable unit test suite" ON)

if (XE_ENABLE_TESTING)
    include(CTest)
    enable_testing()
    find_package(Catch2 3 CONFIG REQUIRED)
endif()
