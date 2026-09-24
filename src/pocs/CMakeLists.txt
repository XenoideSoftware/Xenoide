
option (XE_ENABLE_POC_CLANG "Enable pocs that require clang" OFF)
option (XE_ENABLE_POC_LSP "Enable pocs that require (LSP) Language Server Protocol" OFF)
option (XE_ENABLE_POC_PROTOBUF "Enable pocs that require Google Protocol Buffers" OFF)
option (XE_ENABLE_POC_WXWIDGETS "Enable pocs that require wxWidgets" OFF)
option (XE_ENABLE_POC_ZEROMQ "Enable pocs that require ZeroMQ" OFF)

if (XE_ENABLE_POC_CLANG)
    add_subdirectory("clang-test")
endif()

if (XE_ENABLE_POC_LSP)
    add_subdirectory("poc-lsp-client")
    add_subdirectory("poc-lsp-server")
endif()

if (XE_ENABLE_POC_PROTOBUF)
    add_subdirectory("poc-protobuf-protocol")
    add_subdirectory("poc-protobuf-test")
endif()

if (XE_ENABLE_POC_WXWIDGETS)
    add_subdirectory("poc-wxminimal")
endif ()

if (XE_ENABLE_POC_ZEROMQ)
    add_subdirectory("poc-zeromq-client")
    add_subdirectory("poc-zeromq-server")
endif()
