cmake_minimum_required(VERSION 3.15)
project(protobuf_test CXX)

find_package(protobuf REQUIRED CONFIG)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS test.proto)

add_executable(test_protobuf main.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(test_protobuf PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(test_protobuf PRIVATE protobuf::libprotobuf)
