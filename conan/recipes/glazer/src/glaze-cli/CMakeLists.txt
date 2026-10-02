set (target "glaze")

set (sources
    "src/Main.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target}
    PRIVATE glaze::core
    PRIVATE glaze::interface
)
