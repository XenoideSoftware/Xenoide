
set (target "libxenoide-core-test")
set (sources "src/xenoide-test-main.cpp")

add_executable(${target} ${sources})

target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)
target_link_libraries(${target} PRIVATE libxenoide-core)
