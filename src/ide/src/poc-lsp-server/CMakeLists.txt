find_package(lsp CONFIG REQUIRED)

add_executable(poc-lsp-server main.cpp)
target_link_libraries(poc-lsp-server PRIVATE lsp::liblsp)
set_property(TARGET poc-lsp-server PROPERTY CXX_STANDARD 20)