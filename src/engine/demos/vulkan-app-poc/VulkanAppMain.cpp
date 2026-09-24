
// Vulkan coding learning exercises from: https://vulkan-tutorial.com/

#include "HostPlatform.h"
#include "VulkanRenderer.h"

int main() {
    const uint32_t SCREEN_WIDTH = 1024;
    const uint32_t SCREEN_HEIGHT = 768;

    HostPlatform platform{"Test", SCREEN_WIDTH, SCREEN_HEIGHT, HostPlatformFlagBits::Debug};
    VulkanRenderer renderer{platform};

    renderer.initialize();
    renderer.renderLoop();
    renderer.terminate();

    return 0;
}
