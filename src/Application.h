//
// Created by zanpisotek on 10/15/26.
//

#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include "Window.h"
#include "Object.h"

#include <array>
#include <vector>
#include <stdexcept>
#include <string>

// clang-format off
#define VK_CHECK(expression)                                               \
    do {                                                                   \
        const VkResult vkCheckResult = (expression);                       \
        if (vkCheckResult != VK_SUCCESS) {                                 \
            throw std::runtime_error(                                      \
                std::string("Vulkan call failed: ") + #expression +         \
                " | VkResult: " +                                          \
                std::to_string(static_cast<int>(vkCheckResult)) +           \
                " | " + __FILE__ + ":" + std::to_string(__LINE__));         \
        }                                                                  \
    } while (false)
// clang-format on

// mat4 requires 16-byte alignment
struct UniformBufferObject {
    alignas(16) glm::mat4 Model;
    alignas(16) glm::mat4 View;
    alignas(16) glm::mat4 Projection;
};
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Color;
    glm::vec2 TexCoord;

    static VkVertexInputBindingDescription GetBindingDescription() {
        return {
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
        return {{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, Position)},
            {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, Color)},
            {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, TexCoord)},
        }};
    }
};
class Application {
  public:
    const std::string TEXTURE_PATH = "assets/textures/texture.jpg";
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
    static constexpr uint32_t NUM_SPHERES = 5;
#ifdef DEBUG
    static constexpr bool EnableValidationLayers = true;
#else
    static constexpr bool EnableValidationLayers = false;
#endif

    const std::vector<const char*> ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> RequiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    Application();
    ~Application();

    void Run();

  private:
    void DrawFrame();

    void InitWindow();
    void InitVulkan();
    void Shutdown();

    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void SetupGameObjects();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChoosSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    uint32_t ChooseSwapMinImageCount(const VkSurfaceCapabilitiesKHR& capabilities);
    void CreateSwapChain();
    void RecreateSwapChain();
    void CleanupSwapChain();
    void CreateImageViews();
    void CreateDescriptorSetLayout();
    void CreateGraphicsPipeline();
    void CreateCommandPool();
    void CreateColorResources();
    void CreateDepthResources();
    void CreateTextureImage();
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateVertexBuffer();
    void GenerateSphere(float radius, uint32_t sectors, uint32_t stacks);
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateCommandBuffers();
    void RecordCommandBuffer(uint32_t imageIndex);
    void CreateSyncObjects();

    std::vector<const char*> GetRequiredInstanceExtensions();

    // Loaded extension functions
    void vkCreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* createInfo,
        VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger);
    void vkDestroyDebugUtilsMessengerEXT(
        VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* allocator);

    // Helpers
    bool IsPhysicalDeviceSuitable(const VkPhysicalDevice& device);

    VkImageView CreateImageView(
        const VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    std::vector<char> ReadFile(const std::string& filename);

    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    VkFormat FindSupportedFormat(
        const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat FindDepthFormat();

    void TransitionImageLayout(const VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout,
        VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask, VkPipelineStageFlags2 srcStageMask,
        VkPipelineStageFlags2 dstStageMask, VkImageAspectFlags imageAspectFlags);

    void TransitionImageLayout(VkCommandBuffer commandBuffer, const VkImage& image, VkImageLayout oldLayout,
        VkImageLayout newLayout, uint32_t mipLevels);

    std::pair<VkImage, VkDeviceMemory> CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels,
        VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties);

    std::pair<VkBuffer, VkDeviceMemory> CreateBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    void CopyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size);

    void CopyBufferToImage(
        VkCommandBuffer& commandBuffer, const VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height);

    void GenerateMipmaps(VkCommandBuffer& commandBuffer, VkImage& image, VkFormat format, int32_t texWidth,
        int32_t texHeight, uint32_t mipLevels);

    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkSampleCountFlagBits GetMaxUsableSampleCount();

    void UpdateUniformBuffers(uint32_t currentImage);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

  private:
    Window* m_Window = nullptr;

    VkInstance m_Instance = nullptr;
    VkDebugUtilsMessengerEXT m_DebugMessenger = nullptr;
    VkSurfaceKHR m_Surface = nullptr;
    VkPhysicalDevice m_PhysicalDevice = nullptr;
    VkDevice m_Device = nullptr;
    uint32_t m_QueueIndex;
    VkQueue m_Queue = nullptr;
    VkSurfaceFormatKHR m_SwapChainSurfaceFormat;
    VkExtent2D m_SwapChainExtent;
    VkSwapchainKHR m_SwapChain = nullptr;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;
    VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
    VkPipelineLayout m_PipelineLayout = nullptr;
    VkPipeline m_GraphicsPipeline = nullptr;
    VkCommandPool m_CommandPool = nullptr;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    uint32_t m_MipLevels = 0;
    VkImage m_TextureImage = nullptr;
    VkImageView m_TextureImageView = nullptr;
    VkDeviceMemory m_TextureImageMemory = nullptr;
    VkSampler m_TextureSampler = nullptr;
    VkImage m_ColorImage = nullptr;
    VkImageView m_ColorImageView = nullptr;
    VkDeviceMemory m_ColorImageMemory = nullptr;
    VkSampleCountFlagBits m_MsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkImage m_DepthImage = nullptr;
    VkImageView m_DepthImageView = nullptr;
    VkDeviceMemory m_DepthImageMemory = nullptr;

    VkBuffer m_VertexBuffer = nullptr;
    VkDeviceMemory m_VertexBufferMemory = nullptr;
    VkBuffer m_IndexBuffer = nullptr;
    VkDeviceMemory m_IndexBufferMemory = nullptr;
    VkDescriptorPool m_DescriptorPool = nullptr;

    std::vector<VkSemaphore> m_PresentCompleteSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;

    uint32_t m_FrameIndex = 0;
    bool m_FramebufferResized = false;

    std::vector<Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;

    std::array<Object, NUM_SPHERES> m_Spheres;
};
