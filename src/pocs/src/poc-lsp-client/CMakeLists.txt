find_package(lsp CONFIG REQUIRED)

add_executable(poc-lsp-client main.cpp)
target_link_libraries(poc-lsp-client PRIVATE lsp::liblsp)
set_property(TARGET poc-lsp-client PROPERTY CXX_STANDARD 20)
