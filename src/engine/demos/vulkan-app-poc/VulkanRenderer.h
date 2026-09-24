
#pragma once

#include <xe/Predef.h>
#include <xe/math/Matrix.h>
#include <xe/math/Vector.h>

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>

#if defined(_MSC_VER)
#pragma warning(pop, 0)
#endif

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "HostPlatform.h"

struct Vertex {
    xe::Vector2 pos;
    xe::Vector3 color;

    static vk::VertexInputBindingDescription createBindingDescription() {
        vk::VertexInputBindingDescription desc;

        // all the vertex data is packed in just one array.
        desc.binding = 0;

        // size, in bytes, of each vertex
        desc.stride = sizeof(Vertex);

        // no instancing
        desc.inputRate = vk::VertexInputRate::eVertex;

        return desc;
    }
};

struct QueryFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }

    std::set<uint32_t> uniques() const {
        return {
            graphicsFamily.value(),
            presentFamily.value(),
        };
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainDetail {
    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<vk::SurfaceFormatKHR> surfaceFormats;
    std::vector<vk::PresentModeKHR> surfacePresentModes;
};

//! struct used to transfer data from the host to the shader program
//! all the descriptor stuff will be used for that
struct UniformBufferObject {
    xe::Matrix4 model;
    xe::Matrix4 view;
    xe::Matrix4 proj;
};

class HostPlatform;
class VulkanRenderer {
public:
    explicit VulkanRenderer(HostPlatform &platform);

    void initialize();

    void terminate();

    void renderLoop();

private:
    HostPlatform &mPlatform;
    vk::Instance mInstance;
    vk::DebugUtilsMessengerEXT mDebugMessenger;
    vk::PhysicalDevice mPhysicalDevice;
    vk::SurfaceKHR mSurface;
    vk::Device mDevice;
    QueryFamilyIndices mFamilies;
    vk::Queue mGraphicsQueue;
    vk::Queue mPresentationQueue;
    vk::SwapchainKHR mSwapchain;
    std::vector<vk::Image> mSwapchainImages;
    std::vector<vk::ImageView> mSwapchainImageViews;
    vk::Extent2D mSwapchainExtent;
    vk::SurfaceFormatKHR mSwapchainFormat;
    vk::RenderPass mRenderPass;

    //! the resource descriptor set layout describes the types of resources
    //! that are going to be accessed by the pipeline
    vk::DescriptorSetLayout mDescriptorSetLayout;

    vk::PipelineLayout mPipelineLayout;
    vk::Pipeline mGraphicsPipeline;
    std::vector<vk::Framebuffer> mSwapchainFramebuffers;

    vk::CommandPool mCommandPool;
    vk::CommandBuffer mCommandBuffer;

    //! signals when an image is available from the swapchain
    vk::Semaphore mImageAvailableSemaphore;

    //! signals when an image is ready for presentation (rendering process finished)
    vk::Semaphore mRenderFinishedSemaphore;

    //! pending
    vk::Fence mInFlightFence;

    // buffer that holds vertex information
    vk::Buffer mVertexBuffer;

    // memory that holds the vertex buffer data
    vk::DeviceMemory mVertexBufferMemory;

    vk::Buffer mIndexBuffer;
    vk::DeviceMemory mIndexBufferMemory;

    std::vector<vk::Buffer> mUniformBuffers;
    std::vector<vk::DeviceMemory> mUniformBuffersMemory;
    std::vector<void *> mUniformBuffersMapped;

    //! allocation object for descriptor sets
    vk::DescriptorPool mDescriptorPool;

    //! the descriptor sets specifies the *actual* buffer or image resources
    //! that will be bound to the descriptors, so it needs to be bound to be
    //! for the drawing commands.
    std::vector<vk::DescriptorSet> mDescriptorSets;

    //! image index, in the mSwapchainImage member array
    //! used for indicaing which swapchain image use
    //! when presenting to the user
    uint32_t imageIndex = 0;

    //!
    uint32_t currentFrame = 0;

    vk::ApplicationInfo createAppInfo() const;

    std::vector<const char *> getRequiredExtensions() const;

    vk::DebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo() const;

    vk::DebugUtilsMessengerEXT createDebugMessenger(vk::Instance instance) const;

    vk::Instance createInstance(const std::vector<const char *> &extensions, const std::vector<const char *> &validationLayers) const;

    vk::PhysicalDevice pickPhysicalDevice(const std::vector<vk::PhysicalDevice> &devices);

    void showPhysicalDeviceInformation(const vk::PhysicalDevice &device);

    std::vector<vk::DeviceQueueCreateInfo> mapQueueCreateInfo(const QueryFamilyIndices &indices) const;

    vk::Device createDevice(const vk::PhysicalDevice &physicalDevice, const QueryFamilyIndices &indices) const;

    QueryFamilyIndices identifyQueueFamilies(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) const;

    vk::SwapchainKHR createSwapchain(
        const vk::Device &device,
        const vk::SurfaceKHR &surface,
        const vk::SurfaceFormatKHR &swapchainFormat,
        const vk::Extent2D &swapchainExtent,
        const vk::PresentModeKHR presentMode,
        const uint32_t imageCount,
        const vk::SurfaceTransformFlagBitsKHR preTransform,
        const QueryFamilyIndices &indices
    ) const;

    SwapchainDetail querySwapchainDetail(const vk::PhysicalDevice &physicalDevice, const vk::SurfaceKHR &surface) const;

    //! picks an required surface format
    std::optional<vk::SurfaceFormatKHR> pickSwapchainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &surfaceFormats) const;

    vk::PresentModeKHR pickPresentMode(const std::vector<vk::PresentModeKHR> &presentModes) const;

    uint32_t chooseImageCount(const vk::SurfaceCapabilitiesKHR &caps) const;

    std::vector<vk::ImageView>
    createSwapchainImageViews(const vk::Device &device, const vk::SurfaceFormatKHR &swapchainFormat, const std::vector<vk::Image> &swapchainImages) const;

    std::vector<char> loadBinaryFile(const std::string &filename) const;

    vk::Pipeline createGraphicsPipeline(const vk::Extent2D &swapchainExtent, const vk::PipelineLayout &pipelineLayout, const vk::RenderPass &renderPass);

    vk::ShaderModule createShaderModule(const vk::Device &device, const std::vector<char> &shaderCode) const;

    vk::PipelineLayout createPipelineLayout(const vk::DescriptorSetLayout &descriptorSetLayout);

    vk::RenderPass createRenderPass(const vk::Device &device, const vk::Format &swapchainFormat) const;

    vk::Framebuffer createFramebuffer(const vk::Device &device, const vk::ImageView &imageView, const vk::RenderPass &renderPass, const vk::Extent2D &swapchainExtent) const;

    vk::CommandPool createCommandPool(const vk::Device &device, const uint32_t queueFamily) const;

    vk::CommandBuffer allocateCommandBuffer(const vk::Device &device, const vk::CommandPool &commandPool) const;

    vk::RenderPassBeginInfo
    createRenderPassBeginInfo(const vk::Framebuffer &framebuffer, const vk::ClearValue &clearValue, const vk::RenderPass &renderPass, const vk::Extent2D &swapchainExtent) const;

    void recordCommandBuffer(
        const vk::CommandBuffer &commandBuffer,
        const vk::Framebuffer &framebuffer,
        const vk::RenderPass &renderPass,
        const vk::Extent2D &swapchainExtent,
        const vk::Pipeline &graphicsPipeline
    ) const;

    vk::Fence createFence(const vk::Device &device) const;

    void drawFrame();

    void createVertexBuffer();

    void createIndexBuffer();

    // find a suitable GPU memory type for assign a portion of GPU memory for the buffer
    // this allows better control and performance
    std::optional<uint32_t> findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags propertyFlags) const;

    // creates both a buffer and a buffer memory object, based on supplied specs
    std::tuple<vk::Buffer, vk::DeviceMemory> createBuffer(const vk::DeviceSize size, const vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const;

    //! copies data between two buffers
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, const vk::DeviceSize size);

    //! craetes a new descriptor set layout that specifies the details of every descriptor binding
    //! this needs to be called right before createGraphicsPipeline
    vk::DescriptorSetLayout createDescriptorSetLayout();

    void createUniformBuffers();

    void updateUniformBuffer(const uint32_t currentImage);

    // descriptor sets must be created from a pool (like the command buffers)
    void createDescriptorPool();

    void createDescriptorSets();
    void configureDescriptors();
};
