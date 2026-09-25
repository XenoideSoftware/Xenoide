set(target "clang-test")

# LLVM installs its own CMake config packages; Conan exposes their directories
# via cpp_info.builddirs so that find_package() can locate them.
find_package(LLVM  REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

add_executable(${target} WIN32 "main.cpp")

target_include_directories(${target} PRIVATE ${CLANG_INCLUDE_DIRS})
target_link_libraries(${target} PRIVATE libclang)

set_property(TARGET ${target} PROPERTY CXX_STANDARD 20)
install(TARGETS ${target} DESTINATION "bin")
