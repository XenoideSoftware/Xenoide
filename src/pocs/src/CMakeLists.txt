
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
