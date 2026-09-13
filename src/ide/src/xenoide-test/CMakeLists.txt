
add_executable(xenoide-test xenoide-test-main.cpp)

target_link_libraries(xenoide-test PRIVATE Catch2::Catch2WithMain xenoide-core)
