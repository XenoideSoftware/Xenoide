set(target "xe-core-test")

set(sources
    "DataType.Test.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE
    xe::core
    xe::interface
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(${target})
