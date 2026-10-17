//
// Created by zanpisotek on 10/15/26.
//

#pragma once
#include <vulkan/vulkan.h>
#include "Window.h"

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

class Application {
  public:
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
    void InitWindow();
    void InitVulkan();
    void Shutdown();

    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
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

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

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
};
