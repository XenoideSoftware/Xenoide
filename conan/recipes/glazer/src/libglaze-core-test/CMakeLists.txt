set (target "glaze-core-test")

set (sources
    "BindingGenerator.E2E.Test.cpp"
    "BindingGenerator.DocBrief.Test.cpp"
    "ArgParse.Test.cpp"
)

add_executable(${target} ${sources})
# Reuse the MiniRegistry.h fixture from the producer test suite.
target_include_directories(${target} PRIVATE
    "${CMAKE_SOURCE_DIR}/src/libglaze-producer-test"
)
target_link_libraries(${target} PRIVATE
    glaze::core
    glaze::producer
    glaze::model
    glaze::interface
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(${target})
