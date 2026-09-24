
find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan 1.2 REQUIRED)

set (target "vulkan-app-poc")
set (src VulkanAppMain.cpp HostPlatform.h HostPlatform.cpp VulkanRenderer.h VulkanRenderer.cpp)

if (NOT Vulkan_FOUND)
	message(STATUS "Vulkan libraries not detected. ${target} is not available")
	return()
endif()

add_executable(${target} ${src})

target_include_directories(${target} PUBLIC ${Vulkan_INCLUDE_DIRS})

target_link_libraries(
  ${target}
  PUBLIC xe::core
  PUBLIC xe::math
  PUBLIC glfw
  PRIVATE xe::interface
  PUBLIC Vulkan::Vulkan
)
