find_package(zmqpp REQUIRED)

add_executable(zeromq-server zeromq-server-main.cpp)
target_link_libraries(zeromq-server zmqpp::zmqpp protobuf-protocol)
