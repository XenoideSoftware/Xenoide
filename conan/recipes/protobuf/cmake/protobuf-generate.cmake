# Minimal protobuf integration for consumers of the Xenoide-local
# protobuf recipe. Provides the two API surfaces the project actually
# uses today:
#
#   - protobuf_PROTOC_EXECUTABLE  : path to the protoc binary
#   - protobuf_generate_cpp()      : generate .pb.cc / .pb.h pairs at
#                                    build time
#
# This file is shipped under <pkg>/lib/cmake/protobuf/ and is auto-loaded
# by CMakeDeps via cpp_info.cmake_build_modules.

if(NOT DEFINED protobuf_PROTOC_EXECUTABLE)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(_protobuf_protoc_name "protoc.exe")
    else()
        set(_protobuf_protoc_name "protoc")
    endif()

    # CMAKE_CURRENT_LIST_DIR == <pkg>/lib/cmake/protobuf
    set(protobuf_PROTOC_EXECUTABLE
        "${CMAKE_CURRENT_LIST_DIR}/../../../bin/${_protobuf_protoc_name}"
        CACHE FILEPATH "Path to the protoc binary"
    )
    unset(_protobuf_protoc_name)
endif()

if(NOT COMMAND protobuf_generate_cpp)
    function(protobuf_generate_cpp SRCS HDRS)
        set(${SRCS})
        set(${HDRS})

        foreach(_proto IN LISTS ARGN)
            get_filename_component(_abs "${_proto}" ABSOLUTE)
            get_filename_component(_dir "${_abs}" DIRECTORY)
            get_filename_component(_we  "${_proto}" NAME_WE)

            set(_cc "${CMAKE_CURRENT_BINARY_DIR}/${_we}.pb.cc")
            set(_h  "${CMAKE_CURRENT_BINARY_DIR}/${_we}.pb.h")

            list(APPEND ${SRCS} "${_cc}")
            list(APPEND ${HDRS} "${_h}")

            add_custom_command(
                OUTPUT  "${_cc}" "${_h}"
                COMMAND "${protobuf_PROTOC_EXECUTABLE}"
                ARGS    --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
                        -I "${_dir}"
                        "${_abs}"
                DEPENDS "${_abs}"
                COMMENT "protobuf_generate_cpp: ${_proto}"
                VERBATIM
            )
        endforeach()

        set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)

        set(${SRCS} ${${SRCS}} PARENT_SCOPE)
        set(${HDRS} ${${HDRS}} PARENT_SCOPE)
    endfunction()
endif()
