
find_package(glaze CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(DevIL CONFIG REQUIRED)
find_package(Vulkan 1.2 REQUIRED)
find_package(cxxopts REQUIRED)
find_package(Backport CONFIG REQUIRED)

set (target xe-ktxc)
set (sources src/xe-ktxc-main.cpp)

add_executable(${target} ${sources})
target_include_directories(${target} PUBLIC include)
target_link_libraries(${target} PRIVATE xe::interface PUBLIC xe::core PUBLIC DevIL::IL PUBLIC KTX::ktx PUBLIC Vulkan::Vulkan PUBLIC cxxopts::cxxopts PRIVATE Backport::Backport)
