/* ************************************************************************
 *
 * Copyright (C) 2022 Vincent Luo All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ************************************************************************/

/* Creates on 2023/11/21. */
#include "RendererAPI.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <algorithm>
#include <limits>
#include <set>

#include "Window/Window.h"

#define VK_LAYER_LUNARG_standard_validation "VK_LAYER_LUNARG_standard_validation"

#define vkFourierCreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        throw std::runtime_error("FourierEngine Error: create vulkan object for `{}` handle failed!")

/* Get required instance extensions for vulkan. */
void FourierGetRequiredInstanceExtensions(std::vector<const char *> &vec,
                                          std::unordered_map<std::string, VkExtensionProperties> &supports) {
    /* glfw */
    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (int i = 0; i < glfwExtensionCount; i++)
        vec.push_back(glfwExtensions[i]);
}

/* Get required instance layers for vulkan. */
void FourierGetRequiredInstanceLayers(std::vector<const char *> &vec,
                                      std::unordered_map<std::string, VkLayerProperties> &supports) {
#ifdef FOURIER_DEBUG
    if (supports.count(VK_LAYER_LUNARG_standard_validation) != 0)
        vec.push_back(VK_LAYER_LUNARG_standard_validation);
#endif
}

/* Get required device extensions for vulkan. */
void FourierGetRequiredDeviceExtensions(std::vector<const char *> &vec,
                                      std::unordered_map<std::string, VkExtensionProperties> &supports) {
    if (supports.count(VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
        vec.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

/* Query swapchain details. */
FourierSwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
    FourierSwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    /* Find surface support formats. */
    {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, VK_NULL_HANDLE);
        if (formatCount > 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, std::data(details.formats));
        }
    }

    /* Find surface support present mode. */
    {
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, VK_NULL_HANDLE);
        if (presentModeCount > 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, std::data(details.presentModes));
        }
    }

    return details;
}

/* Find queue family. */
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices queueFamilyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, std::data(queueFamilies));

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices.graphicsFamily = i;

        VkBool32 supportPresentMode = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &supportPresentMode);
        if (queueFamilyCount > 0 && supportPresentMode)
            queueFamilyIndices.presentFamily= i;

        if (queueFamilyIndices.isComplete())
            break;

        ++i;
    }

    return queueFamilyIndices;
}

/* Select surface format. */
VkSurfaceFormatKHR SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    if (std::size(formats) == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return formats[0];
}

/* Select surface presentMode. */
VkPresentModeKHR SelectSwapSurfacePresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        } else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            bestMode = presentMode;
        }
    }

    return bestMode;
}

/* Select swap chain extent. */
VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, FourierWindow *p_window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = { static_cast<uint32_t>(p_window->GetWidth()), static_cast<uint32_t>(p_window->GetHeight()) };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

RendererAPI::RendererAPI(FourierWindow *p_window) {
    /* Enumerate instance available extensions. */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, std::data(extensions));
    std::cout << "FourierEngine Renderer API: Vulkan render api available extensions for instance: " << std::endl;
    for (auto &extension : extensions) {
        std::cout << "    " << extension.extensionName << std::endl;
        m_VkInstanceExtensionPropertiesSupports.insert({extension.extensionName, extension});
    }

    /* Enumerate instance available layers. */
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(layers));
    std::cout << "FourierEngine Renderer API: Vulkan render api available layer list: " << std::endl;
    for (auto &layer : layers) {
        std::cout << "    " << layer.layerName << std::endl;
        m_VkInstanceLayerPropertiesSupports.insert({layer.layerName, layer});
    }

    /* Get extensions & layers. */
    FourierGetRequiredInstanceExtensions(m_RequiredInstanceExtensions, m_VkInstanceExtensionPropertiesSupports);
    FourierGetRequiredInstanceLayers(m_RequiredInstanceLayers, m_VkInstanceLayerPropertiesSupports);

    /** Create vulkan instance. */
    struct VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_VERSION_1_3;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = FOURIER_ENGINE;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = FOURIER_ENGINE;

    struct VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    instanceCreateInfo.enabledExtensionCount = std::size(m_RequiredInstanceExtensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(m_RequiredInstanceExtensions);

    instanceCreateInfo.enabledLayerCount = std::size(m_RequiredInstanceLayers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(m_RequiredInstanceLayers);

    vkFourierCreate(Instance, &instanceCreateInfo, VK_NULL_HANDLE, &m_Instance);

    /** Enumerate physical device. */
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, VK_NULL_HANDLE);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, std::data(devices));
    for (auto &device : devices) {
        FourierPhysicalDevice fourierPhysicalDevice = {};
        fourierPhysicalDevice.handle = device;
        /* Get physical device detail properties. */
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        /* Set properties to FourierPhysicalDevice properties struct object. */
        memcpy(fourierPhysicalDevice.deviceName, properties.deviceName, FOURIER_ENGINE_MAX_DEVICE_NAME_SIZE);
        /* Save. */
        m_FourierPhysicalDevices.push_back(fourierPhysicalDevice);
    }

    if (std::size(m_FourierPhysicalDevices) == 0)
        fourier_throw_error("FourierEngine Error: cannot found physical device for support vulkan api!");

    std::cout << "FourierEngine Renderer API: All physical devices supports for vulkan: " << std::endl;
    for (auto &device : m_FourierPhysicalDevices)
        std::cout << "    " << device.deviceName << std::endl;

    /** Select current using physical device. */
    FourierPhysicalDevice fourierPhysicalDevice = m_FourierPhysicalDevices[0];
    m_PhysicalDevice = fourierPhysicalDevice.handle;
    std::cout << "FourierEngine Renderer API: Using device: " << "<" << fourierPhysicalDevice.deviceName << ">" << std::endl;

    /** Create surface of glfw. */
    if (glfwCreateWindowSurface(m_Instance, p_window->GetWindowHandle(), VK_NULL_HANDLE, &m_Surface) != VK_SUCCESS)
        fourier_throw_error("failed to create window surface!");

    /* Enumerate device extensions. */
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, VK_NULL_HANDLE, &deviceExtensionCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, VK_NULL_HANDLE, &deviceExtensionCount, std::data(deviceExtensions));
    std::cout << "FourierEngine Renderer API: Vulkan render api logical device available extensions: " << std::endl;
    for (auto &extension : deviceExtensions) {
        std::cout << "    " << extension.extensionName << std::endl;
        m_VkDeviceExtensionPropertiesSupports.insert({extension.extensionName, extension});
    }

    /* Get required extensions for device. */
    FourierGetRequiredDeviceExtensions(m_RequiredDeviceExtensions, m_VkDeviceExtensionPropertiesSupports);

    /** Find queue for graphics family and build create device queue struct. */
    m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice, m_Surface);
    std::set<uint32_t> uniqueQueueFamilies = { m_QueueFamilyIndices.graphicsFamily, m_QueueFamilyIndices.presentFamily };

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueFamilyIndex = queueFamily;
        deviceQueueCreateInfo.queueCount = 1;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    /** Create logical device object handle. */
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);
    VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.enabledExtensionCount = std::size(m_RequiredDeviceExtensions);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(m_RequiredDeviceExtensions);
    vkFourierCreate(Device, m_PhysicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &m_Device);

    vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.presentFamily, 0, &presentQueue);

    /** Create swap chain. */
    FourierSwapChainSupportDetails swapChainDetails = QuerySwapChainSupportDetails(m_PhysicalDevice, m_Surface);

    m_SurfaceFormatKHR = SelectSwapSurfaceFormat(swapChainDetails.formats);
    m_SurfacePresentModeKHR = SelectSwapSurfacePresentMode(swapChainDetails.presentModes);
    m_SurfaceExtent = SelectSwapExtent(swapChainDetails.capabilities, p_window);

    uint32_t swapchainImageCount = swapChainDetails.capabilities.minImageCount + 1;
    if (swapChainDetails.capabilities.maxImageCount > 0 && swapchainImageCount > swapChainDetails.capabilities.maxImageCount)
        swapchainImageCount = swapChainDetails.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfoKhr = {};
    swapchainCreateInfoKhr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKhr.surface = m_Surface;
    swapchainCreateInfoKhr.minImageCount = swapchainImageCount;
    swapchainCreateInfoKhr.imageFormat = m_SurfaceFormatKHR.format;
    swapchainCreateInfoKhr.imageColorSpace = m_SurfaceFormatKHR.colorSpace;
    swapchainCreateInfoKhr.imageExtent = m_SurfaceExtent;
    swapchainCreateInfoKhr.imageArrayLayers = 1;
    swapchainCreateInfoKhr.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKhr.preTransform = swapChainDetails.capabilities.currentTransform;
    swapchainCreateInfoKhr.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKhr.presentMode = m_SurfacePresentModeKHR;
    swapchainCreateInfoKhr.clipped = VK_TRUE;
    swapchainCreateInfoKhr.oldSwapchain = VK_NULL_HANDLE;
    vkFourierCreate(SwapchainKHR, m_Device, &swapchainCreateInfoKhr, VK_NULL_HANDLE, &m_SwapChain);

    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &m_SwapChainImageSize, VK_NULL_HANDLE);
    m_SwapChainImages.resize(m_SwapChainImageSize);
    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &m_SwapChainImageSize, std::data(m_SwapChainImages));

    /** Create image views. */
    std::vector<VkImageView> swapChainImageViews(m_SwapChainImageSize);
    for (uint32_t i = 0; i < m_SwapChainImageSize; i++) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_SwapChainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = m_SurfaceFormatKHR.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        vkFourierCreate(ImageView, m_Device, &imageViewCreateInfo, VK_NULL_HANDLE, &swapChainImageViews[i]);
        fourier_logger_info("FourierEngine Renderer API: Create image view for swapchain, image view index: {}", i);
    }

    /** Create graphics pipeline in vulkan. */
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    // TODO vkFourierCreate(GraphicsPipelines, m_Device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, VK_NULL_HANDLE, &m_GraphicsPipeline);

    std::cout << "FourierEngine Renderer API: The initialize vulkan api success! " << std::endl;
}

RendererAPI::~RendererAPI() {
    vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
    vkDestroyDevice(m_Device, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VK_NULL_HANDLE);
    vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
}
