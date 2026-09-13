cmake_minimum_required(VERSION 3.20)
project(llvmTest CXX)

# LLVM installs its own CMake config packages; use them directly.
# CMAKE_PREFIX_PATH is populated by Conan's toolchain via cpp_info.builddirs.
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})

# LLVM_DEFINITIONS may contain multiple flags; split into a list first.
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

# Resolve LLVM component libraries.
llvm_map_components_to_libnames(llvm_libs support)

add_executable(llvmTest main.cpp)
target_link_libraries(llvmTest PRIVATE ${llvm_libs})
set_property(TARGET llvmTest PROPERTY CXX_STANDARD 17)
