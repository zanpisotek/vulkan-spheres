//
// Created by zanpisotek on 10/15/26.
//
#include "Application.h"

#include <algorithm>
#include <cassert>
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
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
}
void Application::Shutdown() {
    CleanupSwapChain();
    vkDestroyDevice(m_Device, nullptr);
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
void Application::CreateLogicalDevice() {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());
    queueFamilies.resize(queueFamilyCount);

    uint32_t queueIndex = ~0;
    for (uint32_t qfpIndex = 0; qfpIndex < queueFamilies.size(); ++qfpIndex) {
        VkBool32 supportsPresent = VK_FALSE;
        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, qfpIndex, m_Surface, &supportsPresent));
        if ((queueFamilies[qfpIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) && supportsPresent) {
            queueIndex = qfpIndex;
            break;
        }
    }

    if (queueIndex == ~0) {
        throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
    }

    m_QueueIndex = queueIndex;

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicStateFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .extendedDynamicState = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features vulkan13Features = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &dynamicStateFeatures,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE};

    VkPhysicalDeviceVulkan11Features vulkan11Features = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &vulkan13Features,
        .shaderDrawParameters = VK_TRUE};

    VkPhysicalDeviceFeatures2 features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vulkan11Features,
        .features = {.sampleRateShading = VK_TRUE, .samplerAnisotropy = true},
    };

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_QueueIndex,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority};

    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<uint32_t>(RequiredDeviceExtensions.size()),
        .ppEnabledExtensionNames = RequiredDeviceExtensions.data(),
    };

    VK_CHECK(vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device));
    vkGetDeviceQueue(m_Device, m_QueueIndex, 0, &m_Queue);
}
VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    assert(!availableFormats.empty());

    const auto formatIt = std::ranges::find_if(availableFormats, [](const auto& format) {
        return format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    });

    return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
}
VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    // assert that at least fifo-khr is supported
    assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) {
        return presentMode == VK_PRESENT_MODE_FIFO_KHR;
    }));

    VkPresentModeKHR chosenPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto presentMode : availablePresentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            chosenPresentMode = presentMode;
        }
    }
    return chosenPresentMode;
}
VkExtent2D Application::ChoosSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(m_Window->GetHandle(), &width, &height);

    return {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height),
    };
}
uint32_t Application::ChooseSwapMinImageCount(const VkSurfaceCapabilitiesKHR& capabilities) {
    auto minImageCount = std::max(3u, capabilities.minImageCount);
    if ((capabilities.maxImageCount > 0) && (capabilities.maxImageCount < minImageCount)) {
        minImageCount = capabilities.maxImageCount;
    }

    return minImageCount;
}
void Application::CreateSwapChain() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &surfaceCapabilities));
    m_SwapChainExtent = ChoosSwapExtent(surfaceCapabilities);
    uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

    uint32_t surfaceFormatCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &surfaceFormatCount, nullptr));

    std::vector<VkSurfaceFormatKHR> availableFormats(surfaceFormatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
        m_PhysicalDevice, m_Surface, &surfaceFormatCount, availableFormats.data()));
    availableFormats.resize(surfaceFormatCount);

    m_SwapChainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &presentModeCount, nullptr);

    std::vector<VkPresentModeKHR> availablePresentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        m_PhysicalDevice, m_Surface, &presentModeCount, availablePresentModes.data());
    availablePresentModes.resize(presentModeCount);

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_Surface,
        .minImageCount = minImageCount,
        .imageFormat = m_SwapChainSurfaceFormat.format,
        .imageColorSpace = m_SwapChainSurfaceFormat.colorSpace,
        .imageExtent = m_SwapChainExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = ChooseSwapPresentMode(availablePresentModes),
        .clipped = true,
        .oldSwapchain = nullptr,
    };

    VK_CHECK(vkCreateSwapchainKHR(m_Device, &swapChainCreateInfo, nullptr, &m_SwapChain));

    uint32_t imageCount = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr));

    m_SwapChainImages.resize(imageCount);
    VK_CHECK(vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data()));
    m_SwapChainImages.resize(imageCount);
}
void Application::RecreateSwapChain() {
    int width, height;
    glfwGetFramebufferSize(m_Window->GetHandle(), &width, &height);

    // Minimization
    while ((width == 0 || height == 0) && !glfwWindowShouldClose(m_Window->GetHandle())) {
        glfwGetFramebufferSize(m_Window->GetHandle(), &width, &height);
        glfwWaitEvents();
    }
    VK_CHECK(vkDeviceWaitIdle(m_Device));

    CleanupSwapChain();

    CreateSwapChain();
}
void Application::CleanupSwapChain() {
    vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
    m_SwapChain = nullptr;

    for (auto& imageView : m_SwapChainImageViews) {
        vkDestroyImageView(m_Device, imageView, nullptr);
    }
}
void Application::CreateImageViews() {
    assert(m_SwapChainImageViews.empty());

    for (const auto& image : m_SwapChainImages) {
        m_SwapChainImageViews.push_back(
            CreateImageView(image, m_SwapChainSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1));
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
VkImageView Application::CreateImageView(
    const VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {

    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange =
            {
                .aspectMask = aspectFlags,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    VkImageView imageView;
    VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView));

    return imageView;
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