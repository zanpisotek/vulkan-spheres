//
// Created by zanpisotek on 10/15/26.
//
#include "Application.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <ostream>

Application::Application() {
    InitWindow();
    InitVulkan();
}
Application::~Application() {
    delete m_Window;
    Shutdown();
}
void Application::Run() {
    while (m_Window->IsOpen()) {
        m_Window->PollEvents();
    }
}
void Application::InitWindow() { m_Window = new Window(1600, 900, "Vulkan Spheres"); }
void Application::InitVulkan() {
    CreateInstance();
    SetupDebugMessenger();
}
void Application::Shutdown() {
    vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}
void Application::CreateInstance() {
    VkApplicationInfo appInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkan Spheres",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_4};

    auto requiredExtensions = GetRequiredInstanceExtensions();

    uint32_t extensionCount = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));

    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data()));
    extensionProperties.resize(extensionCount);

    // check if there are any unsupported extensions
    auto unsupportedPropertyIt =
        std::ranges::find_if(requiredExtensions, [&extensionProperties](const auto& requiredExtension) {
        return std::ranges::none_of(extensionProperties, [&requiredExtension](const auto& extensionProperty) {
            return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
        });
    });

    if (unsupportedPropertyIt != requiredExtensions.end()) {
        throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
    }

    // assign validation layers if we are in debug
    std::vector<const char*> requiredLayers;
    if (EnableValidationLayers) {
        requiredLayers.assign(ValidationLayers.begin(), ValidationLayers.end());
    }

    uint32_t layerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

    std::vector<VkLayerProperties> layerProperties(layerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data()));
    layerProperties.resize(layerCount);

    // check if layers are supported
    auto unsupportedLayerIt = std::ranges::find_if(requiredLayers, [&layerProperties](const auto& requiredLayer) {
        return std::ranges::none_of(layerProperties, [requiredLayer](const auto& layerProperty) {
            return strcmp(layerProperty.layerName, requiredLayer) == 0;
        });
    });

    if (unsupportedLayerIt != requiredLayers.end()) {
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
    };

    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance));
}
void Application::SetupDebugMessenger() {
    if constexpr (!EnableValidationLayers) {
        return;
    }

    VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &DebugCallback};

    vkCreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger);
}
std::vector<const char*> Application::GetRequiredInstanceExtensions() {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (EnableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}
void Application::vkCreateDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerCreateInfoEXT* createInfo,
    VkAllocationCallbacks* allocator, VkDebugUtilsMessengerEXT* debugMessenger) {
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));

    if (!func) {
        throw std::runtime_error("Failed to load vkCreateDebugUtilsMessengerEXT");
    }
    if (func(m_Instance, createInfo, allocator, debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create debug messenger");
    }
}
void Application::vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, VkAllocationCallbacks* allocator) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (!func) {
        throw std::runtime_error("Failed to load vkDestroyDebugUtilsMessengerEXT");
    }
    func(m_Instance, debugMessenger, allocator);
}
VkBool32 Application::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "Validation layer: type " << std::to_string(messageType) << " message: " << pCallbackData->pMessage
              << std::endl;
    return VK_FALSE;
}