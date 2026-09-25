
set (target cmake-checker)
set (sources
    "src/main.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE cmake-checker-core xe::interface cxxopts::cxxopts)
