find_package(protobuf REQUIRED)

MESSAGE ("protobuf_PROTOC_EXECUTABLE ${protobuf_PROTOC_EXECUTABLE}")

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS person.proto)
add_library(person-proto ${PROTO_SRCS} ${PROTO_HDRS})

MESSAGE ("PROTO_HDRS ${PROTO_HDRS}")
MESSAGE ("PROTO_SRCS ${PROTO_SRCS}")

target_link_libraries(person-proto PUBLIC protobuf::libprotobuf)
target_include_directories(person-proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_executable(protobuf-test protobuf-test-main.cpp)
target_link_libraries(protobuf-test PRIVATE protobuf::libprotobuf person-proto)
