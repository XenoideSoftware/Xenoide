find_package(protobuf REQUIRED)

MESSAGE ("protobuf_PROTOC_EXECUTABLE ${protobuf_PROTOC_EXECUTABLE}")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS message.proto)
add_library(protobuf-protocol ${PROTO_SRCS} ${PROTO_HDRS})

MESSAGE ("PROTO_HDRS ${PROTO_HDRS}")
MESSAGE ("PROTO_SRCS ${PROTO_SRCS}")

target_include_directories(protobuf-protocol PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(protobuf-protocol PUBLIC protobuf::libprotobuf)
