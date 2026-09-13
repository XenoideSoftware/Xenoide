
if (NOT XE_BUILD_VERSION_MAJOR)
    message (WARNING "[Xenoide] XE_BUILD_VERSION_MAJOR not set, defaulting to 0")
    set(XE_BUILD_VERSION_MAJOR "0")
endif()

if (NOT XE_BUILD_VERSION_MINOR)
    message (WARNING "[Xenoide] XE_BUILD_VERSION_MINOR not set, defaulting to 0")
    set(XE_BUILD_VERSION_MINOR "0")
endif()

if (NOT XE_BUILD_VERSION_REV)
    message (WARNING "[Xenoide] XE_BUILD_VERSION_REV not set, defaulting to 0")
    set(XE_BUILD_VERSION_REV "0")
endif()

if (NOT XE_BUILD_COMMIT)
    message (WARNING "[Xenoide] XE_BUILD_COMMIT not set, defaulting to 0")
    set(XE_BUILD_COMMIT "local")
endif()

if (NOT XE_BUILD_TIMESTAMP)
    message (WARNING "[Xenoide] XE_BUILD_TIMESTAMP not set, defaulting to 0")
    set(XE_BUILD_TIMESTAMP "now")
endif()

project (xenoide VERSION ${XE_BUILD_VERSION_MAJOR}.${XE_BUILD_VERSION_MINOR}.${XE_BUILD_VERSION_REV})

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

option (XE_ENGINE "Build with engine" ON)
option (XE_IDE "Build with IDE" ON)

add_subdirectory(src)
