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
    CreateSurface();
    PickPhysicalDevice();
}
void Application::Shutdown() {
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
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
void Application::CreateSurface() {
    if (glfwCreateWindowSurface(m_Instance, m_Window->GetHandle(), nullptr, &m_Surface) != 0) {
        throw std::runtime_error("Failed to create window surface");
    }
}
void Application::PickPhysicalDevice() {
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr));

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.data());
    physicalDevices.resize(physicalDeviceCount);

    if (physicalDevices.empty()) {
        throw std::runtime_error("Faild to find GPU's with Vulkan support");
    }

    for (const auto& physicalDevice : physicalDevices) {
        if (IsPhysicalDeviceSuitable(physicalDevice)) {
            m_PhysicalDevice = physicalDevice;

            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
#ifdef DEBUG
                std::cout << "Picked GPU: " << properties.deviceName << std::endl;
#endif
                break;
            }
        }
    }
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
// Helpers
bool Application::IsPhysicalDeviceSuitable(const VkPhysicalDevice& device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    bool supportsVulkan1_3 = properties.apiVersion >= VK_API_VERSION_1_3;

    // Queue support
    uint32_t queuePropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queuePropertiesCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queuePropertiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queuePropertiesCount, queueFamilyProperties.data());
    queueFamilyProperties.resize(queuePropertiesCount);

    bool supportsGraphicsAndPresent = false;
    for (uint32_t i = 0; i < queuePropertiesCount; ++i) {
        VkBool32 presentSupport = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport));

        if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
            supportsGraphicsAndPresent = true;
            break;
        }
    }

    if (!supportsGraphicsAndPresent) {
        throw std::runtime_error("Failed to find graphics and present queue support");
    }

    uint32_t deviceExtensionCount = 0;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr));

    std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
    VK_CHECK(
        vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, availableDeviceExtensions.data()));
    availableDeviceExtensions.resize(deviceExtensionCount);

    bool supportsAllExtensions = std::ranges::all_of(
        RequiredDeviceExtensions, [&availableDeviceExtensions](const auto& requiredDeviceExtension) {
        return std::ranges::any_of(
            availableDeviceExtensions, [requiredDeviceExtension](const auto& availableDeviceExtension) {
            return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension);
        });
    });

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicStateFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT};

    VkPhysicalDeviceVulkan13Features vulkan13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &dynamicStateFeatures};

    VkPhysicalDeviceVulkan11Features vulkan11Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, .pNext = &vulkan13Features};

    VkPhysicalDeviceFeatures2 features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &vulkan11Features};

    vkGetPhysicalDeviceFeatures2(device, &features);

    bool supportsRequiredFeatures = vulkan11Features.shaderDrawParameters && vulkan13Features.dynamicRendering &&
                                    dynamicStateFeatures.extendedDynamicState;

    return supportsVulkan1_3 && supportsGraphicsAndPresent && supportsRequiredFeatures && supportsAllExtensions;
}


// Callbacks
VkBool32 Application::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ||
        messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "Validation layer: type " << std::to_string(messageType) << " message: " << pCallbackData->pMessage
                  << std::endl;
    } else {
        std::cout << "Validation layer: type " << std::to_string(messageType) << " message: " << pCallbackData->pMessage
                  << std::endl;
    }
    return VK_FALSE;
}