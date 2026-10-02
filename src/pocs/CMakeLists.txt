cmake_minimum_required (VERSION 3.16)

project (XEPOCS)

option (XE_ENABLE_POC_CLANG "Enable pocs that require clang" OFF)
option (XE_ENABLE_POC_LSP "Enable pocs that require (LSP) Language Server Protocol" OFF)
option (XE_ENABLE_POC_PROTOBUF "Enable pocs that require Google Protocol Buffers" OFF)
option (XE_ENABLE_POC_WXWIDGETS "Enable pocs that require wxWidgets" OFF)
option (XE_ENABLE_POC_ZEROMQ "Enable pocs that require ZeroMQ" OFF)

add_subdirectory("src")
