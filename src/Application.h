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

    Application();
    ~Application();

    void Run();

  private:
    void InitWindow();
    void InitVulkan();
    void Shutdown();

    void CreateInstance();
    void SetupDebugMessenger();

    std::vector<const char*> GetRequiredInstanceExtensions();

    // Loaded extension functions
    void vkCreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* createInfo,
        VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger);
    void vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* allocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

  private:
    Window* m_Window = nullptr;

    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
};
