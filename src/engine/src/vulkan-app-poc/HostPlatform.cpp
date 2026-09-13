
#include "HostPlatform.h"

#include <iostream>
#include <xe/Predef.h>

HostPlatform::HostPlatform(const std::string &title, const uint32_t screenWidth, const uint32_t screenHeight, const HostPlatformFlagBits flags)
    : mScreenWidth(screenWidth),
      mScreenHeight(screenHeight),
      mFlags(flags) {
    assert(screenWidth > 0);
    assert(screenHeight > 0);

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(mScreenWidth, mScreenHeight, title.c_str(), nullptr, nullptr);
}

HostPlatform::~HostPlatform() {
    destroyWindow();
    glfwTerminate();
}

bool HostPlatform::closeWasRequested() const {
    return glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(mWindow) == GLFW_TRUE;
}

uint32_t HostPlatform::getScreenWidth() const {
    return mScreenWidth;
}

uint32_t HostPlatform::getScreenHeight() const {
    return mScreenHeight;
}

std::vector<const char *> HostPlatform::enumerateRequiredInstanceExtensions() const {
    uint32_t extensionCount = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    std::vector<const char *> result = {extensions, extensions + extensionCount};

    result.push_back("VK_KHR_get_physical_device_properties2");

    if (mFlags == HostPlatformFlagBits::Debug) {
        result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // SDK 1.3.216 for macOS now requires the enablement of the VK_KHR_portability_enumeration instance extension
#if defined(XE_OS_MACOS) || defined(XE_OS_IOS)
    result.push_back("VK_KHR_portability_enumeration");
#endif

    std::cout << "These " << result.size() << " extensions are required:" << '\n';
    std::for_each(result.begin(), result.end(), std::puts);

    return result;
}

std::vector<const char *> HostPlatform::enumerateRequiredDeviceExtensions() const {
    return {
        // these extensions provide all the objects required for swapchains
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,

    // required for platforms where Vulkan isn't supported directly by the OS
#if defined(XE_OS_MACOS) || defined(XE_OS_IOS)
        "VK_KHR_portability_subset"
#endif
    };
}

std::vector<const char *> HostPlatform::enumerateValidationLayers() const {
    if (mFlags == HostPlatformFlagBits::Debug) {
        return {"VK_LAYER_KHRONOS_validation"};
    }

    return {};
}

void HostPlatform::pollEvents() {
    glfwPollEvents();
}

vk::Extent2D HostPlatform::pickSwapExtent(const vk::SurfaceCapabilitiesKHR &surfaceCaps) const {
    assert(mWindow);

    if (surfaceCaps.currentExtent.width != UINT32_MAX) {
        return surfaceCaps.currentExtent;
    }

    int width = 0, height = 0;
    glfwGetFramebufferSize(mWindow, &width, &height);

    const vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    return {
        std::clamp(actualExtent.width, surfaceCaps.minImageExtent.width, surfaceCaps.maxImageExtent.width),
        std::clamp(actualExtent.height, surfaceCaps.minImageExtent.height, surfaceCaps.maxImageExtent.height),
    };
}

vk::SurfaceKHR HostPlatform::createSurface(vk::Instance &instance) const {
    VkSurfaceKHR rawsurf = nullptr;

    if (glfwCreateWindowSurface(instance, mWindow, nullptr, &rawsurf) != VK_SUCCESS) {
        return {};
    }

    return rawsurf;
}

void HostPlatform::destroyWindow() {
    if (mWindow) {
        glfwDestroyWindow(mWindow);
        mWindow = nullptr;
    }
}
