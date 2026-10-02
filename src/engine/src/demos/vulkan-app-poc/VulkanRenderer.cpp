
#include "VulkanRenderer.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<Vertex> vertices{
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}, {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}, {{-0.5f, 0.5f}, {0.0f, 0.0f, 0.0f}}
};

const std::vector<uint16_t> indices{0, 1, 2, 2, 3, 0};

static std::ostream &operator<<(std::ostream &os, const vk::PhysicalDeviceType deviceType) {
    switch (deviceType) {
    case vk::PhysicalDeviceType::eCpu:
        os << "vk::PhysicalDeviceType::eCpu";
        break;

    case vk::PhysicalDeviceType::eDiscreteGpu:
        os << "vk::PhysicalDeviceType::eDiscreteGpu";
        break;

    case vk::PhysicalDeviceType::eIntegratedGpu:
        os << "vk::PhysicalDeviceType::eIntegratedGpu";
        break;

    case vk::PhysicalDeviceType::eVirtualGpu:
        os << "vk::PhysicalDeviceType::eVirtualGpu";
        break;

    case vk::PhysicalDeviceType::eOther:
        os << "vk::PhysicalDeviceType::eOther";
        break;
    default:
        os << "<Unknown>:" << (int)deviceType;
    }

    return os;
}

vk::Bool32 debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
    vk::DebugUtilsMessageTypeFlagsEXT /*messageTypes*/,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void * /*pUserData*/
) {

    std::cout << pCallbackData->pMessage << '\n' << '\n';

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger
) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

std::vector<char> readFile(const std::string &filename) {
    std::ifstream fs{filename.c_str(), std::ios::ate | std::ios::binary};

    if (!fs.is_open()) {
        std::cerr << "failed to open file " << filename << '\n';
        throw std::exception();
    }

    const auto fileSize = fs.tellg();
    assert(fileSize >= 0);

    std::vector<char> buffer;
    buffer.resize(static_cast<size_t>(fileSize));

    fs.seekg(0);
    fs.read(buffer.data(), fileSize);
    fs.close();

    return buffer;
}

VulkanRenderer::VulkanRenderer(HostPlatform &platform) : mPlatform(platform) {
}

void VulkanRenderer::initialize() {
    std::vector<const char *> extensions = mPlatform.enumerateRequiredInstanceExtensions();

    mInstance = createInstance(extensions, mPlatform.enumerateValidationLayers());
    assert(mInstance);

    mDebugMessenger = createDebugMessenger(mInstance);

    mSurface = mPlatform.createSurface(mInstance);
    assert(mSurface);

    mPhysicalDevice = pickPhysicalDevice(mInstance.enumeratePhysicalDevices());
    mFamilies = identifyQueueFamilies(mPhysicalDevice, mSurface);
    assert(mFamilies.isComplete());

    mDevice = createDevice(mPhysicalDevice, mFamilies);
    assert(mDevice);

    mDevice.getQueue(mFamilies.graphicsFamily.value(), 0, &mGraphicsQueue);
    assert(mGraphicsQueue);

    mDevice.getQueue(mFamilies.presentFamily.value(), 0, &mPresentationQueue);
    assert(mPresentationQueue);

    /*Swapchain initialization section*/
    const SwapchainDetail swapchainDetail = querySwapchainDetail(mPhysicalDevice, mSurface);
    const uint32_t imageCount = chooseImageCount(swapchainDetail.surfaceCapabilities);

    const std::optional<vk::SurfaceFormatKHR> surfaceFormat = pickSwapchainSurfaceFormat(swapchainDetail.surfaceFormats);
    assert(surfaceFormat);

    mSwapchainFormat = surfaceFormat.value();
    mSwapchainExtent = mPlatform.pickSwapExtent(swapchainDetail.surfaceCapabilities);

    mSwapchain = createSwapchain(
        mDevice,
        mSurface,
        mSwapchainFormat,
        mSwapchainExtent,
        pickPresentMode(swapchainDetail.surfacePresentModes),
        imageCount,
        swapchainDetail.surfaceCapabilities.currentTransform,
        mFamilies
    );
    assert(mSwapchain);

    mSwapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);
    mSwapchainImageViews = createSwapchainImageViews(mDevice, mSwapchainFormat, mSwapchainImages);

    mRenderPass = createRenderPass(mDevice, mSwapchainFormat.format);

    mDescriptorSetLayout = createDescriptorSetLayout();
    mPipelineLayout = createPipelineLayout(mDescriptorSetLayout);
    mGraphicsPipeline = createGraphicsPipeline(mSwapchainExtent, mPipelineLayout, mRenderPass);

    for (const vk::ImageView &imageView : mSwapchainImageViews) {
        mSwapchainFramebuffers.push_back(createFramebuffer(mDevice, imageView, mRenderPass, mSwapchainExtent));
    }

    mCommandPool = createCommandPool(mDevice, mFamilies.graphicsFamily.value());
    mCommandBuffer = allocateCommandBuffer(mDevice, mCommandPool);

    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    mImageAvailableSemaphore = mDevice.createSemaphore({});
    mRenderFinishedSemaphore = mDevice.createSemaphore({});
    mInFlightFence = createFence(mDevice);
}

void VulkanRenderer::terminate() {
}

void VulkanRenderer::renderLoop() {
    while (!mPlatform.closeWasRequested()) {
        mPlatform.pollEvents();
        drawFrame();
    }
}

vk::ApplicationInfo VulkanRenderer::createAppInfo() const {
    // Identifica a la aplicacion / motor en uso, para que pueda ser (potencialmente)
    // considerado por el implementador de Hardware (nVidia, Intel, etc), para aplicar
    // ciertas optimizaciones a nivel del Driver.
    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("VulkanTestApp");
    appInfo.setApplicationVersion(VK_MAKE_VERSION(1, 0, 0));
    appInfo.setPEngineName("XE");
    appInfo.setEngineVersion(VK_MAKE_VERSION(1, 0, 0));

    // La version de la API de vulkan a usar.
    // Se deberia detectar previamente la maxima version de vulkan soportada por el driver.
    appInfo.apiVersion = VK_API_VERSION_1_1;

    return appInfo;
}

std::vector<const char *> VulkanRenderer::getRequiredExtensions() const {
    uint32_t extensionCount = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    std::vector<const char *> result = {extensions, extensions + extensionCount};

    result.push_back("VK_KHR_get_physical_device_properties2");

    return result;
}

vk::DebugUtilsMessengerCreateInfoEXT VulkanRenderer::createDebugMessengerInfo() const {
    vk::DebugUtilsMessengerCreateInfoEXT msgInfo;

    msgInfo.setMessageSeverity(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
    );

    msgInfo.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

    msgInfo.pfnUserCallback = debugCallback;
    msgInfo.pUserData = nullptr;

    return msgInfo;
}

vk::DebugUtilsMessengerEXT VulkanRenderer::createDebugMessenger(vk::Instance instance) const {
    const auto debugInfo = static_cast<VkDebugUtilsMessengerCreateInfoEXT>(createDebugMessengerInfo());

    VkDebugUtilsMessengerEXT messenger = nullptr;
    const VkResult result = CreateDebugUtilsMessengerEXT(static_cast<VkInstance>(instance), &debugInfo, nullptr, &messenger);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to setup the debug messenger");
    }

    return vk::DebugUtilsMessengerEXT{messenger};
}

vk::Instance VulkanRenderer::createInstance(const std::vector<const char *> &extensions, const std::vector<const char *> &validationLayers) const {

    std::cout << "Creating Vulkan instance with these specified features" << '\n';

    std::cout << '\n' << "Extensions:" << '\n';
    std::for_each(extensions.begin(), extensions.end(), std::puts);

    std::cout << '\n' << "Validation Layers:" << '\n';
    std::for_each(validationLayers.begin(), validationLayers.end(), std::puts);

    const vk::ApplicationInfo appInfo = createAppInfo();

    // crear la instancia con una seria de extensiones / capas de validacion requeridas
    vk::InstanceCreateInfo info{};
    info.setPApplicationInfo(&appInfo);
    // establecer que requerimos layers referentes a la validacion del uso de la API
    info.setPEnabledLayerNames(validationLayers);
    // establecer las extensiones requeridas por GLFW
    info.setPEnabledExtensionNames(extensions);

#if defined(XE_OS_MACOS) || defined(XE_OS_IOS)
    info.flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

    return vk::createInstance(info);
}

vk::PhysicalDevice VulkanRenderer::pickPhysicalDevice(const std::vector<vk::PhysicalDevice> &devices) {
    for (const auto &device : devices) {
        showPhysicalDeviceInformation(device);
    }

    return devices[0];
}

void VulkanRenderer::showPhysicalDeviceInformation(const vk::PhysicalDevice &device) {
    const auto properties = device.getProperties();

    std::cout << "\"" << properties.deviceName << "\"" << '\n';
    std::cout << "  deviceID: " << "\"" << properties.deviceID << "\"" << '\n';
    std::cout << "  deviceName: " << "\"" << properties.deviceName << "\"" << '\n';
    std::cout << "  vendorID: " << "\"" << properties.vendorID << "\"" << '\n';
    std::cout << "  apiVersion: " << "\"" << properties.apiVersion << "\"" << '\n';
    std::cout << "  deviceType: " << "\"" << properties.deviceType << "\"" << '\n';
    std::cout << "  driverVersion: " << "\"" << properties.driverVersion << "\"" << '\n';

    std::cout << '\n';
}

std::vector<vk::DeviceQueueCreateInfo> VulkanRenderer::mapQueueCreateInfo(const QueryFamilyIndices &familyIndices) const {
    const float priority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> infos;

    for (const uint32_t familyIndex : familyIndices.uniques()) {
        vk::DeviceQueueCreateInfo info;

        info.queueFamilyIndex = familyIndex;
        info.setPQueuePriorities(&priority);
        info.setQueueCount(1);

        infos.push_back(info);
    }

    return infos;
}

vk::Device VulkanRenderer::createDevice(const vk::PhysicalDevice &physicalDevice, const QueryFamilyIndices &familyIndices) const {
    const std::vector<vk::DeviceQueueCreateInfo> queueInfos = mapQueueCreateInfo(familyIndices);

    vk::DeviceCreateInfo info;

    // set the required queues (graphics and presentation, for now).
    info.setPQueueCreateInfos(queueInfos.data());
    info.setQueueCreateInfoCount(static_cast<uint32_t>(queueInfos.size()));

    // set the required device extensions
    const auto deviceExtensions = mPlatform.enumerateRequiredDeviceExtensions();

    if (deviceExtensions.size() > 0) {
        info.setPEnabledExtensionNames(deviceExtensions);
        info.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()));
    }

    // set the validation layers
    auto validationLayers = mPlatform.enumerateValidationLayers();

    if (validationLayers.size() > 0) {
        info.setPEnabledLayerNames(validationLayers);
        info.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));
    }

    return physicalDevice.createDevice(info);
}

QueryFamilyIndices VulkanRenderer::identifyQueueFamilies(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) const {
    QueryFamilyIndices familyIndices;

    const auto queueProperties = physicalDevice.getQueueFamilyProperties();

    for (uint32_t i = 0; i < static_cast<uint32_t>(queueProperties.size()); i++) {
        const auto &props = queueProperties[i];

        // grab the family index for graphics
        if (props.queueFlags & vk::QueueFlagBits::eGraphics) {
            familyIndices.graphicsFamily = i;
        }

        // check if the current queue has presentation capabilities
        vk::Bool32 presentationSuport = VK_FALSE;
        const vk::Result result = physicalDevice.getSurfaceSupportKHR(i, surface, &presentationSuport);

        if (result == vk::Result::eSuccess && presentationSuport == VK_TRUE) {
            familyIndices.presentFamily = i;
        }
    }

    return familyIndices;
}

vk::SwapchainKHR VulkanRenderer::createSwapchain(
    const vk::Device &device,
    const vk::SurfaceKHR &surface,
    const vk::SurfaceFormatKHR &swapchainFormat,
    const vk::Extent2D &swapchainExtent,
    const vk::PresentModeKHR presentMode,
    const uint32_t imageCount,
    const vk::SurfaceTransformFlagBitsKHR preTransform,
    const QueryFamilyIndices &familyIndices
) const {
    vk::SwapchainCreateInfoKHR info = {};
    info.surface = surface;
    info.minImageCount = imageCount;
    info.imageFormat = swapchainFormat.format;
    info.imageColorSpace = swapchainFormat.colorSpace;
    info.imageExtent = swapchainExtent;
    info.imageArrayLayers = 1;
    info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    if (familyIndices.graphicsFamily != familyIndices.presentFamily) {
        const uint32_t queueFamilyIndices[] = {familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value()};

        info.imageSharingMode = vk::SharingMode::eConcurrent;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        info.imageSharingMode = vk::SharingMode::eExclusive;
        info.queueFamilyIndexCount = 0;
        info.pQueueFamilyIndices = nullptr;
    }

    info.preTransform = preTransform;
    info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    info.presentMode = presentMode;
    info.oldSwapchain = nullptr;

    return device.createSwapchainKHR(info);
}

SwapchainDetail VulkanRenderer::querySwapchainDetail(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) const {
    SwapchainDetail detail;

    detail.surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    detail.surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface);
    detail.surfacePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

    return detail;
}

//! picks an required surface format
std::optional<vk::SurfaceFormatKHR> VulkanRenderer::pickSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &surfaceFormats) const {
    for (const vk::SurfaceFormatKHR &surfaceFormat : surfaceFormats) {
        if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return surfaceFormat;
        }
    }

    return {};
}

vk::PresentModeKHR VulkanRenderer::pickPresentMode(const std::vector<vk::PresentModeKHR> &presentModes) const {
    const auto it = std::find(presentModes.begin(), presentModes.end(), vk::PresentModeKHR::eMailbox);

    if (it != presentModes.end()) {
        return vk::PresentModeKHR::eMailbox;
    }

    return vk::PresentModeKHR::eFifo;
}

uint32_t VulkanRenderer::chooseImageCount(const vk::SurfaceCapabilitiesKHR &caps) const {
    const uint32_t imageCount = caps.minImageCount + 1;

    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) {
        return caps.maxImageCount;
    }

    return imageCount;
}

std::vector<vk::ImageView>
VulkanRenderer::createSwapchainImageViews(const vk::Device &device, const vk::SurfaceFormatKHR &swapchainFormat, const std::vector<vk::Image> &swapchainImages) const {
    std::vector<vk::ImageView> imageViews;

    imageViews.reserve(swapchainImages.size());

    for (const vk::Image &image : swapchainImages) {
        vk::ImageViewCreateInfo info = {};

        info.image = image;
        info.viewType = vk::ImageViewType::e2D;
        info.format = swapchainFormat.format;
        info.components.r = vk::ComponentSwizzle::eIdentity;
        info.components.g = vk::ComponentSwizzle::eIdentity;
        info.components.b = vk::ComponentSwizzle::eIdentity;
        info.components.a = vk::ComponentSwizzle::eIdentity;
        info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        imageViews.push_back(device.createImageView(info));
    }

    return imageViews;
}

std::vector<char> VulkanRenderer::loadBinaryFile(const std::string &filename) const {
    const std::string path = XE_MEDIA_PATH + filename;
    return readFile(path);
}

vk::Pipeline VulkanRenderer::createGraphicsPipeline(const vk::Extent2D &swapchainExtent, const vk::PipelineLayout &pipelineLayout, const vk::RenderPass &renderPass) {
    vk::ShaderModule vertModule = createShaderModule(mDevice, loadBinaryFile("/shaders/triangle/vert.spv"));
    vk::ShaderModule fragModule = createShaderModule(mDevice, loadBinaryFile("/shaders/triangle/frag.spv"));

    vk::PipelineShaderStageCreateInfo vertexShaderStageInfo;
    vertexShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    vertexShaderStageInfo.module = vertModule;
    vertexShaderStageInfo.pName = "main";

    vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo;
    fragmentShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    fragmentShaderStageInfo.module = fragModule;
    fragmentShaderStageInfo.pName = "main";

    // configured shader stages
    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{vertexShaderStageInfo, fragmentShaderStageInfo};

    // vertex binding description
    vk::VertexInputBindingDescription vertexBindingDesc = Vertex::createBindingDescription();

    // vertex attribute description
    std::array<vk::VertexInputAttributeDescription, 2> vertexAttributeDescs;

    // vertex position
    vertexAttributeDescs[0].binding = 0;
    vertexAttributeDescs[0].location = 0;
    vertexAttributeDescs[0].format = vk::Format::eR32G32Sfloat;
    vertexAttributeDescs[0].offset = offsetof(Vertex, pos);

    // vertex color
    vertexAttributeDescs[1].binding = 0;
    vertexAttributeDescs[1].location = 1;
    vertexAttributeDescs[1].format = vk::Format::eR32G32B32Sfloat;
    vertexAttributeDescs[1].offset = offsetof(Vertex, color);

    // vertex layout
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;

    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescs.size());
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescs.data();

    // primitive used to draw
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    // viewport specification
    // region of the framebuffer
    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchainExtent.width);
    viewport.height = static_cast<float>(swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D{0, 0};
    scissor.extent = swapchainExtent;

    vk::PipelineViewportStateCreateInfo viewportStateInfo;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &scissor;

    // rasterization state
    // equivalent to RenderState in Direct3D 7-ish
    vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo;
    rasterizationStateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateInfo.polygonMode = vk::PolygonMode::eFill;
    rasterizationStateInfo.lineWidth = 1.0f;
    rasterizationStateInfo.cullMode = vk::CullModeFlagBits::eBack;
    rasterizationStateInfo.frontFace = vk::FrontFace::eCounterClockwise;
    rasterizationStateInfo.depthBiasEnable = VK_FALSE;

    // multisampling
    vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
    multisampleStateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

    // fragment color blending parameters
    // describes how to combine the fragment's final color with its previous color
    vk::PipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    colorBlendAttachment.blendEnable = VK_FALSE;

    vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;
    colorBlendStateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateInfo.attachmentCount = 1;
    colorBlendStateInfo.pAttachments = &colorBlendAttachment;

    vk::GraphicsPipelineCreateInfo pipelineInfo{};

    // reference all the description states, created earlier in this method

    // shader stages used
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizationStateInfo;
    pipelineInfo.pMultisampleState = &multisampleStateInfo;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlendStateInfo;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = pipelineLayout;

    // the render pass that this pipeline will use
    pipelineInfo.renderPass = renderPass;

    // the index of the sub pass within the sub pass that will be used
    pipelineInfo.subpass = 0;

    auto pipelineResult = mDevice.createGraphicsPipeline({}, pipelineInfo);

    if (pipelineResult.result != vk::Result::eSuccess) {
        throw std::runtime_error("Can't create a GraphicsPipeline with those parameters.");
    }

    return pipelineResult.value;
}

vk::ShaderModule VulkanRenderer::createShaderModule(const vk::Device &device, const std::vector<char> &shaderCode) const {
    vk::ShaderModuleCreateInfo info;

    info.codeSize = shaderCode.size();
    info.pCode = reinterpret_cast<const uint32_t *>(shaderCode.data());

    return device.createShaderModule(info);
}

vk::PipelineLayout VulkanRenderer::createPipelineLayout(const vk::DescriptorSetLayout &descriptorSetLayout) {
    // pipeline layout
    // used for passing values to the uniforms found in the current shader program
    vk::PipelineLayoutCreateInfo info;
    info.setLayoutCount = 1;
    info.pSetLayouts = &descriptorSetLayout;

    return mDevice.createPipelineLayout(info);
}

vk::RenderPass VulkanRenderer::createRenderPass(const vk::Device &device, const vk::Format &swapchainFormat) const {
    // Color Buffer Attachment
    // its represented from by one of the images in the Swapchain
    vk::AttachmentDescription colorAttachment;

    // format should match the format of the swapchain images
    colorAttachment.format = swapchainFormat;

    // no multisampling.
    colorAttachment.samples = vk::SampleCountFlagBits::e1;

    // what to do before rendering
    // in this case, clear the color buffer
    // apply for color and depth data
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;

    // what to do after rendering
    // in this case, rendered content will be stored in memory
    // apply for color and depth data
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;

    // what to do before and after rendering, this time, for the stencil buffer.
    // dont-care for now.
    colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

    // what change to make with respect to the layout of the pixels in memory
    // describes wich layout the image will have before the render pass begins
    colorAttachment.initialLayout = vk::ImageLayout::eUndefined;

    // specified wich layout the image will have after the render pass finish.
    // present with the swapchain.
    colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

    // subpass specification

    // Attachment reference
    // specifies references to use within a subpass
    vk::AttachmentReference colorAttachmentRef;

    // attachment index from the attachment description array
    colorAttachmentRef.attachment = 0;

    // we will use the attachment as a Color Buffer
    colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

    // describes a render subpass
    vk::SubpassDescription subpass;

    // this is a graphics subpass
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // subpass dependency
    // controls how the transitions of the image layouts are made.
    vk::SubpassDependency dependency;

    // refers to te implicit subpass dependency
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

    // refers to our only subpass
    // dstSubpass must always be greater than srcSubpass to prevent cycles in the dependency graph
    dependency.dstSubpass = 0;

    // specify that we need to wait for the swapchain from the image, before we can make use of.
    // this is implicit (in this case), as this is the result of the color attachment output stage itself.
    dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    // the color attachment output stage must wait on this
    dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

    // render pass creation
    // use the attachment and the sub pass specification
    // subpasses uses internally the attachments specified here, via the attachment reference, specified earlier.
    vk::RenderPassCreateInfo renderPassInfo;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    return device.createRenderPass(renderPassInfo);
}

vk::Framebuffer
VulkanRenderer::createFramebuffer(const vk::Device &device, const vk::ImageView &imageView, const vk::RenderPass &renderPass, const vk::Extent2D &swapchainExtent) const {
    vk::FramebufferCreateInfo info;

    // render pass to be compatible with
    info.renderPass = renderPass;

    // image views to bind with
    info.attachmentCount = 1;
    info.pAttachments = &imageView;

    // size of the framebuffer
    info.width = swapchainExtent.width;
    info.height = swapchainExtent.height;

    // our image views from the swapchain are single-layered, so we put 1 one
    info.layers = 1;

    return device.createFramebuffer(info);
}

vk::CommandPool VulkanRenderer::createCommandPool(const vk::Device &device, const uint32_t queueFamily) const {
    // a commandPool is an Object that records different types of commands (like rendering and memory transfer),
    // to be executed by the GPU.

    vk::CommandPoolCreateInfo info;

    // specifies we will be recretating the command pool in every render frame
    info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    // put this command pool in the specified Queue Family
    info.queueFamilyIndex = queueFamily;

    return device.createCommandPool(info);
}

vk::CommandBuffer VulkanRenderer::allocateCommandBuffer(const vk::Device &device, const vk::CommandPool &commandPool) const {
    vk::CommandBufferAllocateInfo info;

    info.commandPool = commandPool;
    // a primary buffer is a buffer that can be submitted to a queue for execution directly, but can't be called from another buffers
    // a secondary buffer is a buffer that can't be submitted to a queue for execution, but can be called from primary buffers
    info.level = vk::CommandBufferLevel::ePrimary;

    // the number of buffers to allocate
    info.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;

    const vk::Result result = device.allocateCommandBuffers(&info, &commandBuffer);

    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Can't allocate the CommandBuffer with the supplied parameters.");
    }

    return commandBuffer;
}

vk::RenderPassBeginInfo VulkanRenderer::createRenderPassBeginInfo(
    const vk::Framebuffer &framebuffer, const vk::ClearValue &clearValue, const vk::RenderPass &renderPass, const vk::Extent2D &swapchainExtent
) const {

    // render pass
    vk::RenderPassBeginInfo info;

    info.renderPass = renderPass;
    info.framebuffer = framebuffer;
    info.renderArea.offset = vk::Offset2D{0, 0};
    info.renderArea.extent = swapchainExtent;
    info.clearValueCount = 1;
    info.pClearValues = &clearValue;

    return info;
}

void VulkanRenderer::recordCommandBuffer(
    const vk::CommandBuffer &commandBuffer,
    const vk::Framebuffer &framebuffer,
    const vk::RenderPass &renderPass,
    const vk::Extent2D &swapchainExtent,
    const vk::Pipeline &graphicsPipeline
) const {

    vk::ClearValue clearColor;
    clearColor.color.setFloat32({0.2f, 0.2f, 0.2f, 1.0f});

    vk::CommandBufferBeginInfo beginInfo;

    // implicitily, resets the command buffer
    commandBuffer.begin(beginInfo);
    commandBuffer.beginRenderPass(createRenderPassBeginInfo(framebuffer, clearColor, renderPass, swapchainExtent), vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    // bind the vertex buffer
    const vk::Buffer vertexBuffers[] = {mVertexBuffer};
    const vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

    // bind the index buffer
    commandBuffer.bindIndexBuffer(mIndexBuffer, 0, vk::IndexType::eUint16);

    // render
    // commandBuffer.draw(3, 1, 0, 0);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mPipelineLayout, 0, 1, &mDescriptorSets[imageIndex], 0, nullptr);
    commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.end();
}

vk::Fence VulkanRenderer::createFence(const vk::Device &device) const {
    vk::FenceCreateInfo info;

    info.flags = vk::FenceCreateFlagBits::eSignaled;

    return device.createFence(info);
}

void VulkanRenderer::drawFrame() {
    // first, wait for the previous frame to render
    const auto waitResult = mDevice.waitForFences(1, &mInFlightFence, VK_TRUE, UINT64_MAX);

    if (waitResult != vk::Result::eSuccess) {
        throw std::runtime_error("Fence Wait operation failed.");
    }

    // reset the frame-rendering fence
    const auto resetResult = mDevice.resetFences(1, &mInFlightFence);

    if (resetResult != vk::Result::eSuccess) {
        throw std::runtime_error("Fence Reset operation failed.");
    }

    // acquire an image from the swapchain
    const auto acquireResult = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, mImageAvailableSemaphore, nullptr, &imageIndex);
    if (acquireResult != vk::Result::eSuccess) {
        throw std::runtime_error("Acquire image from the swapchain operation failed.");
    }

    assert(imageIndex < mSwapchainFramebuffers.size());

    updateUniformBuffer(currentFrame);

    // record the necesary commands render the frame
    mCommandBuffer.reset();
    recordCommandBuffer(mCommandBuffer, mSwapchainFramebuffers[imageIndex], mRenderPass, mSwapchainExtent, mGraphicsPipeline);

    // submit the command buffer to the graphics pipeline queue
    const vk::Semaphore waitSemaphores[] = {mImageAvailableSemaphore};
    const vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    const vk::Semaphore signalSemaphores[] = {mRenderFinishedSemaphore};

    vk::SubmitInfo submitInfo;

    // configure the semaphores to use for waiting the execution in specific
    // pipeline stages (color attachment output, in this case), begins.
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // the commands to be submitted
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCommandBuffer;

    // the semaphores to signal when the command finish execution
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    const auto resultSubmit = mGraphicsQueue.submit(1, &submitInfo, mInFlightFence);
    if (resultSubmit != vk::Result::eSuccess) {
        throw std::runtime_error("Graphics queue submit operation failed.");
    }

    // presentation info
    vk::PresentInfoKHR presentInfo;

    // wich semaphores wait on, before presentation can occur
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    // specify wich swapchain use to present images to.
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mSwapchain;

    // the index of the image, for each swapchain used.
    presentInfo.pImageIndices = &imageIndex;

    // results to use
    presentInfo.pResults = nullptr;

    const vk::Result presentResult = mPresentationQueue.presentKHR(presentInfo);
    if (presentResult != vk::Result::eSuccess) {
        throw std::runtime_error("Graphics queue submit operation failed.");
    }
}

void VulkanRenderer::createVertexBuffer() {
    const vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // the staging buffer will be visible for the host
    const auto stagingBufferUsage = vk::BufferUsageFlagBits::eTransferSrc;
    const auto stagingMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    auto [stagingBuffer, stagingBufferMemory] = createBuffer(bufferSize, stagingBufferUsage, stagingMemoryProperties);

    // copy the vertex data to the staging buffer
    void *data = mDevice.mapMemory(stagingBufferMemory, 0, VK_WHOLE_SIZE);
    std::memcpy(data, vertices.data(), sizeof(vertices[0]) * vertices.size());
    mDevice.unmapMemory(stagingBufferMemory);

    // create the vertex buffer and its memory
    const auto bufferUsage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    const auto bufferMemory = vk::MemoryPropertyFlagBits::eDeviceLocal;

    std::tie(mVertexBuffer, mVertexBufferMemory) = createBuffer(bufferSize, bufferUsage, bufferMemory);

    copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

    mDevice.destroyBuffer(stagingBuffer);
    mDevice.freeMemory(stagingBufferMemory);
}

std::optional<uint32_t> VulkanRenderer::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags) const {
    // query the available memory types in the physical device
    const vk::PhysicalDeviceMemoryProperties properties = mPhysicalDevice.getMemoryProperties();

    // vk::PhysicalDeviceMemoryProperties has two fields: types and heaps
    // heaps are memory regions (RAM, VRAM or a combination of two)
    for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {

        // filter for the specified typeFilter
        const bool matchTypeFilter = typeFilter & (i << 1);
        const bool matchPropertyFlags = (properties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags;

        if (matchTypeFilter && matchPropertyFlags) {
            return i;
        }
    }

    return {};
}

std::tuple<vk::Buffer, vk::DeviceMemory> VulkanRenderer::createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    // access to any range or image subresource of the object will be exclusive to an unique family at a time
    // this may be more performant than a concurrent sharing mode
    bufferInfo.sharingMode = vk::SharingMode::eExclusive;

    auto buffer = mDevice.createBuffer(bufferInfo);

    // retrieve the memory requirements for the buffer
    auto requirements = mDevice.getBufferMemoryRequirements(buffer);

    // allocate the memory
    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, properties).value();

    auto memory = mDevice.allocateMemory(allocInfo);

    mDevice.bindBufferMemory(buffer, memory, 0);

    return {buffer, memory};
}

void VulkanRenderer::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, const vk::DeviceSize size) {
    vk::CommandBufferAllocateInfo allocInfo{};

    allocInfo.level = vk::CommandBufferLevel::ePrimary; // will be executed directly by a command queue
    allocInfo.commandPool = mCommandPool;
    allocInfo.commandBufferCount = 1;

    const auto commandBuffers = mDevice.allocateCommandBuffers(allocInfo);
    const auto &commandBuffer = commandBuffers[0];

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    commandBuffer.begin(beginInfo);
    const auto regions = std::array<vk::BufferCopy, 1>{vk::BufferCopy{0, 0, size}};
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, regions);
    commandBuffer.end();

    // submit the command buffer for immediate execution
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    mGraphicsQueue.submit(submitInfo);

    mGraphicsQueue.waitIdle();

    mDevice.freeCommandBuffers(mCommandPool, commandBuffer);
}

void VulkanRenderer::createIndexBuffer() {
    const vk::DeviceSize size = indices.size() * sizeof(indices[0]);
    const auto stagingUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;
    const auto stagingMemoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    auto [stagingBuffer, stagingBufferMemory] = createBuffer(size, stagingUsageFlags, stagingMemoryProperties);

    void *data = mDevice.mapMemory(stagingBufferMemory, 0, size);
    std::memcpy(data, indices.data(), size);
    mDevice.unmapMemory(stagingBufferMemory);

    const auto usageFlags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
    const auto memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    std::tie(mIndexBuffer, mIndexBufferMemory) = createBuffer(size, usageFlags, memoryProperties);

    copyBuffer(stagingBuffer, mIndexBuffer, size);

    mDevice.destroyBuffer(stagingBuffer);
    mDevice.freeMemory(stagingBufferMemory);
}

vk::DescriptorSetLayout VulkanRenderer::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding layout{};

    // binding location. must match the one indicated in the shader
    layout.binding = 0;

    // the descriptor will just an unique uniform buffer
    // this uniform buffer will hold the matrix mvp pack
    layout.descriptorType = vk::DescriptorType::eUniformBuffer;
    layout.descriptorCount = 1;

    // this descriptor will apply at the vertex shader stage
    layout.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
    descriptorSetLayoutInfo.bindingCount = 1;
    descriptorSetLayoutInfo.pBindings = &layout;

    return mDevice.createDescriptorSetLayout(descriptorSetLayoutInfo);
}

void VulkanRenderer::createUniformBuffers() {
    const vk::DeviceSize size = sizeof(UniformBufferObject);

    mUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    mUniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    mUniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    const auto flags = vk::BufferUsageFlagBits::eUniformBuffer;
    const auto properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::tie(mUniformBuffers[i], mUniformBuffersMemory[i]) = createBuffer(size, flags, properties);

        mUniformBuffersMapped[i] = mDevice.mapMemory(mUniformBuffersMemory[i], 0, size);
    }

    assert(!mUniformBuffers.empty());
}

void VulkanRenderer::updateUniformBuffer(const uint32_t currentImage) {
    assert(currentImage < MAX_FRAMES_IN_FLIGHT);

    static auto startTime = std::chrono::high_resolution_clock::now();
    const auto currentTime = std::chrono::high_resolution_clock::now();
    const auto diff = currentTime - startTime;

    float time = std::chrono::duration<float, std::chrono::seconds::period>(diff).count();

    UniformBufferObject ubo{};

    ubo.model = xe::mat4RotationZ(time * xe::radians(90.0f));
    ubo.view = xe::mat4LookAtRH({2.0f, 2.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});

    const auto aspectRatio = static_cast<float>(mSwapchainExtent.width) / static_cast<float>(mSwapchainExtent.height);
    ubo.proj = xe::mat4Perspective(xe::radians(45.0f), aspectRatio, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    assert(mUniformBuffersMapped[currentImage] != nullptr);
    std::memcpy(mUniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanRenderer::createDescriptorPool() {
    vk::DescriptorPoolSize poolSize{};
    poolSize.type = vk::DescriptorType::eUniformBuffer;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.poolSizeCount = 1;
    poolCreateInfo.pPoolSizes = &poolSize;
    poolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    mDescriptorPool = mDevice.createDescriptorPool(poolCreateInfo);
}

void VulkanRenderer::createDescriptorSets() {
    assert(!mUniformBuffers.empty());

    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);

    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.descriptorPool = mDescriptorPool; // we will allocate from mDescriptorPool
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    mDescriptorSets = mDevice.allocateDescriptorSets(allocInfo);
    assert(mDescriptorSets.size() == MAX_FRAMES_IN_FLIGHT);
    assert(mDescriptorSets[0]);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo info{};
        info.buffer = mUniformBuffers[i];
        info.offset = 0;
        info.range = sizeof(UniformBufferObject); // or VK_WHOLE_SIZE

        vk::WriteDescriptorSet descriptorWrite{};
        descriptorWrite.dstSet = mDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &info;

        mDevice.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }
}
