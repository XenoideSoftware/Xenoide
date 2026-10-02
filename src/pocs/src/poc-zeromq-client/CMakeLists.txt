find_package(zmqpp REQUIRED)

add_executable(zeromq-client zeromq-client-main.cpp)
target_link_libraries(zeromq-client zmqpp::zmqpp protobuf-protocol)
