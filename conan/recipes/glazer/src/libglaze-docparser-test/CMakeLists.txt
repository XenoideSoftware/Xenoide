set (target "glaze-docparser-test")

set (sources
    "DocParser.Test.cpp"
)

add_executable(${target} ${sources})
target_link_libraries(${target} PRIVATE
    glaze::docparser
    glaze::interface
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(${target})
