cmake_minimum_required(VERSION 3.15)
project(zeromq_test C CXX)

find_package(ZeroMQ REQUIRED CONFIG)

add_executable(test_zeromq main.c)
target_link_libraries(test_zeromq PRIVATE libzmq)

# libzmq is a C++ static archive (libstdc++ operator new/delete, std::nothrow,
# SJLJ exception personality on TDM-GCC, etc.). A C-only test_package would
# link with gcc and miss libstdc++/libsupc++; force the C++ linker driver.
set_target_properties(test_zeromq PROPERTIES LINKER_LANGUAGE CXX)
