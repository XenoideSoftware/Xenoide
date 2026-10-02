
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

enum class HostPlatformFlagBits { None = 0x00, Debug = 0x01 };

class HostPlatform {
public:
    explicit HostPlatform(const std::string &title, const uint32_t screenWidth, const uint32_t screenHeight, const HostPlatformFlagBits flags);

    ~HostPlatform();

    bool closeWasRequested() const;

    uint32_t getScreenWidth() const;

    uint32_t getScreenHeight() const;

    std::vector<const char *> enumerateRequiredInstanceExtensions() const;

    std::vector<const char *> enumerateRequiredDeviceExtensions() const;

    std::vector<const char *> enumerateValidationLayers() const;

    void pollEvents();

    vk::Extent2D pickSwapExtent(const vk::SurfaceCapabilitiesKHR &surfaceCaps) const;

    vk::SurfaceKHR createSurface(vk::Instance &instance) const;

private:
    void destroyWindow();

    GLFWwindow *mWindow = nullptr;
    uint32_t mScreenWidth = 1024;
    uint32_t mScreenHeight = 768;
    HostPlatformFlagBits mFlags;
};
