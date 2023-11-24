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
#include "VulkanRenderAPI.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <algorithm>
#include <limits>
#include <set>
#include <glm.hpp>

#include "Window/Window.h"
#include "Utils/IOUtils.h"

#define FOURIER_SHADER_MODULE_OF_VERTEX_BINARY_FILE "../Engine/Binaries/simple_shader.vert.spv"
#define FOURIER_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE "../Engine/Binaries/simple_shader.frag.spv"

#define VK_LAYER_KHRONOS_validation "VK_LAYER_KHRONOS_validation"

#define vkFourierCreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        throw std::runtime_error("FourierEngine Error: create vulkan object for `{}` handle failed!")
#define vkFourierAllocate(name, ...) \
    if (vkAllocate##name(__VA_ARGS__) != VK_SUCCESS) \
        throw std::runtime_error("FourierEngine Error: allocate vulkan object for `{}` handle failed!")
#define FOURIER_LOGGER_INIT_VULKAN_API(fmt, ...) \
  fourier_logger_info("[FOURIER ENGINE] [INIT_VULKAN_API] IF/ - {}", std::format(fmt, ##__VA_ARGS__))

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
    if (supports.count(VK_LAYER_KHRONOS_validation) != 0)
        vec.push_back(VK_LAYER_KHRONOS_validation);
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

VkShaderModule LoadShaderModule(VkDevice device, const char *file_path) {
    char *buf;
    size_t size;
    VkShaderModule shader;

    /* load shader binaries. */
    buf = fourier_load_binaries(file_path, &size);

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = size;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buf);
    vkFourierCreate(ShaderModule, device, &shaderModuleCreateInfo, VK_NULL_HANDLE, &shader);

    /* free binaries buf. */
    fourier_free_binaries(buf);

    return shader;
}

uint32_t FindMemoryType(uint32_t typeFilter, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

VulkanRenderAPI::VulkanRenderAPI(FourierWindow *p_window) {
    /* Enumerate instance available extensions. */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, std::data(extensions));
    FOURIER_LOGGER_INIT_VULKAN_API("Vulkan render api available extensions for instance:");
    for (auto &extension : extensions) {
        FOURIER_LOGGER_INIT_VULKAN_API("    {}", extension.extensionName);
        m_VkInstanceExtensionPropertiesSupports.insert({extension.extensionName, extension});
    }

    /* Enumerate instance available layers. */
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(layers));
    FOURIER_LOGGER_INIT_VULKAN_API("Vulkan render api available layer list: ");
    for (auto &layer : layers) {
        FOURIER_LOGGER_INIT_VULKAN_API("    {}", layer.layerName);
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
    FOURIER_LOGGER_INIT_VULKAN_API("Used vulkan extension for instance count: {}", std::size(m_RequiredInstanceExtensions));
    for (const auto& name : m_RequiredInstanceExtensions)
        FOURIER_LOGGER_INIT_VULKAN_API("    {}", name);

    instanceCreateInfo.enabledLayerCount = std::size(m_RequiredInstanceLayers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(m_RequiredInstanceLayers);

    vkFourierCreate(Instance, &instanceCreateInfo, VK_NULL_HANDLE, &m_Instance);

    /** Enumerate physical device. */
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, VK_NULL_HANDLE);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, std::data(devices));
    for (auto &device : devices) {
        PhysicalDeviceProperties physicalDeviceProperties = {};
        physicalDeviceProperties.handle = device;
        /* Get physical device detail properties. */
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);
        /* Set properties to PhysicalDeviceProperties properties struct object. */
        memcpy(physicalDeviceProperties.deviceName, properties.deviceName, FOURIER_ENGINE_MAX_DEVICE_NAME_SIZE);
        /* Save. */
        m_PhysicalDevices.push_back(physicalDeviceProperties);
    }

    if (std::size(m_PhysicalDevices) == 0)
        fourier_throw_error("FourierEngine Error: cannot found physical device for support vulkan api!");

    FOURIER_LOGGER_INIT_VULKAN_API("All physical devices supports for vulkan: ");
    for (auto &device : m_PhysicalDevices)
        FOURIER_LOGGER_INIT_VULKAN_API("    {}", device.deviceName);

    /** Select current using physical device. */
    PhysicalDeviceProperties fourierPhysicalDevice = m_PhysicalDevices[0];
    m_PhysicalDevice = fourierPhysicalDevice.handle;
    FOURIER_LOGGER_INIT_VULKAN_API("Using device: <{}>", fourierPhysicalDevice.deviceName);

    /** Create surface of glfw. */
    if (glfwCreateWindowSurface(m_Instance, p_window->GetWindowHandle(), VK_NULL_HANDLE, &m_Surface) != VK_SUCCESS)
        fourier_throw_error("failed to create window surface!");

    /* Enumerate device extensions. */
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, VK_NULL_HANDLE, &deviceExtensionCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, VK_NULL_HANDLE, &deviceExtensionCount, std::data(deviceExtensions));
    FOURIER_LOGGER_INIT_VULKAN_API("Vulkan render api logical device available extensions: ");
    for (auto &extension : deviceExtensions) {
        FOURIER_LOGGER_INIT_VULKAN_API("    {}", extension.extensionName);
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

    /* get graphics and present queue */
    vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.graphicsFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_QueueFamilyIndices.presentFamily, 0, &m_PresentQueue);

    /** Create swap chain. */
    FourierSwapChainSupportDetails swapChainDetails = QuerySwapChainSupportDetails(m_PhysicalDevice, m_Surface);

    m_SurfaceFormatKHR = SelectSwapSurfaceFormat(swapChainDetails.formats);
    m_SwapChainFormat = m_SurfaceFormatKHR.format;
    m_SurfacePresentModeKHR = SelectSwapSurfacePresentMode(swapChainDetails.presentModes);
    m_SwapChainExtent = SelectSwapExtent(swapChainDetails.capabilities, p_window);

    uint32_t swapchainImageCount = swapChainDetails.capabilities.minImageCount + 1;
    if (swapChainDetails.capabilities.maxImageCount > 0 && swapchainImageCount > swapChainDetails.capabilities.maxImageCount)
        swapchainImageCount = swapChainDetails.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfoKhr = {};
    swapchainCreateInfoKhr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKhr.surface = m_Surface;
    swapchainCreateInfoKhr.minImageCount = swapchainImageCount;
    swapchainCreateInfoKhr.imageFormat = m_SurfaceFormatKHR.format;
    swapchainCreateInfoKhr.imageColorSpace = m_SurfaceFormatKHR.colorSpace;
    swapchainCreateInfoKhr.imageExtent = m_SwapChainExtent;
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
    m_SwapChainImageViews.resize(m_SwapChainImageSize);
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
        vkFourierCreate(ImageView, m_Device, &imageViewCreateInfo, VK_NULL_HANDLE, &m_SwapChainImageViews[i]);
        FOURIER_LOGGER_INIT_VULKAN_API("Create image view for swapchain, image view index: {}", i);
    }

    /** Create render pass. */
    VkAttachmentDescription colorAttachmentDescription = {};
    colorAttachmentDescription.format = m_SwapChainFormat;
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    vkFourierCreate(RenderPass, m_Device, &renderPassCreateInfo, VK_NULL_HANDLE, &m_RenderPass);

    /** Create shader of vertex & fragment module. */
    FOURIER_LOGGER_INIT_VULKAN_API("Loading and create vertex shader module from: {}", FOURIER_SHADER_MODULE_OF_VERTEX_BINARY_FILE);
    VkShaderModule vertex_shader_module = LoadShaderModule(m_Device, FOURIER_SHADER_MODULE_OF_VERTEX_BINARY_FILE);
    FOURIER_LOGGER_INIT_VULKAN_API("Loading and create vertex shader module success!");

    FOURIER_LOGGER_INIT_VULKAN_API("Loading and create fragment shader module from: {}", FOURIER_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE);
    VkShaderModule fragment_shader_module = LoadShaderModule(m_Device, FOURIER_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE);
    FOURIER_LOGGER_INIT_VULKAN_API("Loading and create fragment shader module success!");

    /** Create pipeline phase of vertex and fragment shader */
    VkPipelineShaderStageCreateInfo pipelineVertexShaderStageCreateInfo = {};
    pipelineVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineVertexShaderStageCreateInfo.module = vertex_shader_module;
    pipelineVertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo pipelineFragmentStageCreateInfo = {};
    pipelineFragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineFragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineFragmentStageCreateInfo.module = fragment_shader_module;
    pipelineFragmentStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[] = { pipelineVertexShaderStageCreateInfo, pipelineFragmentStageCreateInfo };

    /* pipeline features */
    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
    pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto vertexInputBindingDescription = Vertex::GetVertexInputBindingDescription();
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

    auto vertexInputAttributeDescriptions = Vertex::GetVertexInputAttributeDescriptions();
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = std::size(vertexInputAttributeDescriptions);
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = std::data(vertexInputAttributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssembly = {};
    pipelineInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_SwapChainExtent.width;
    viewport.height = (float) m_SwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = m_SwapChainExtent;

    /* 视口裁剪 */
    VkPipelineViewportStateCreateInfo pipelineViewportStateCrateInfo = {};
    pipelineViewportStateCrateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportStateCrateInfo.viewportCount = 1;
    pipelineViewportStateCrateInfo.pViewports = &viewport;
    pipelineViewportStateCrateInfo.scissorCount = 1;
    pipelineViewportStateCrateInfo.pScissors = &scissor;

    /* 光栅化阶段 */
    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
    pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // Optional
    pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional

    /* 多重采样 */
    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
    pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f; // Optional
    pipelineMultisampleStateCreateInfo.pSampleMask = nullptr; // Optional
    pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

    /* 颜色混合 */
    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
    pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
    pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
    pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    /* 帧缓冲 */
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    pipelineColorBlendStateCreateInfo.attachmentCount = 1;
    pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
    pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f; // Optional

    /* 动态修改 */
    VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineDynamicStateCreateInfo.dynamicStateCount = 2;
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates;

    /* 管道布局 */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

    vkFourierCreate(PipelineLayout, m_Device, &pipelineLayoutInfo, nullptr, &m_GraphicsPipelineLayout);

    /* 帧缓冲区 */
    m_SwapChainFramebuffers.resize(m_SwapChainImageSize);
    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
        VkImageView attachments[] = { m_SwapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = m_RenderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = m_SwapChainExtent.width;
        framebufferCreateInfo.height = m_SwapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        vkFourierCreate(Framebuffer, m_Device, &framebufferCreateInfo, nullptr, &m_SwapChainFramebuffers[i]);
    }

    /** Create graphics pipeline in vulkan. */
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos;
    graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssembly;
    graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCrateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr; // Optional
    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = nullptr; // Optional
    graphicsPipelineCreateInfo.layout = m_GraphicsPipelineLayout;
    graphicsPipelineCreateInfo.renderPass = m_RenderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    graphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional

    vkFourierCreate(GraphicsPipelines, m_Device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, VK_NULL_HANDLE, &m_GraphicsPipeline);

    /** Destroy shader module. */
    vkDestroyShaderModule(m_Device, vertex_shader_module, VK_NULL_HANDLE);
    vkDestroyShaderModule(m_Device, fragment_shader_module, VK_NULL_HANDLE);

    /** Create command pool. */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily;
    commandPoolCreateInfo.flags = 0; // Optional

    vkFourierCreate(CommandPool, m_Device, &commandPoolCreateInfo, VK_NULL_HANDLE, &m_CommandPool);

    /** Allocate command buffer. */
    m_CommandBuffers.resize(m_SwapChainImageSize);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_CommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t) std::size(m_CommandBuffers);

    vkFourierAllocate(CommandBuffers, m_Device, &commandBufferAllocateInfo, std::data(m_CommandBuffers));

    /** Create semaphores. */
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkFourierCreate(Semaphore, m_Device, &semaphoreCreateInfo, VK_NULL_HANDLE, &m_ImageAvailableSemaphore);
    vkFourierCreate(Semaphore, m_Device, &semaphoreCreateInfo, VK_NULL_HANDLE, &m_RenderFinishedSemaphore);

    FOURIER_LOGGER_INIT_VULKAN_API("The initialize vulkan api success!");

    /** Create vertex buffer */
    CreateVertexBuffer();
}

VulkanRenderAPI::~VulkanRenderAPI() {
    vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore, nullptr);
    vkDestroySemaphore(m_Device, m_RenderFinishedSemaphore, nullptr);

    vkFreeCommandBuffers(m_Device, m_CommandPool, std::size(m_CommandBuffers), std::data(m_CommandBuffers));
    vkDestroyCommandPool(m_Device, m_CommandPool, VK_NULL_HANDLE);

    for (auto &framebuffer : m_SwapChainFramebuffers)
        vkDestroyFramebuffer(m_Device, framebuffer, VK_NULL_HANDLE);

    vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_GraphicsPipelineLayout, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPass, VK_NULL_HANDLE);

    for (auto &imageView : m_SwapChainImageViews)
        vkDestroyImageView(m_Device, imageView, VK_NULL_HANDLE);

    vkDestroyBuffer(m_Device, m_VertexBuffer, VK_NULL_HANDLE);
    vkFreeMemory(m_Device, m_VertexBufferMemory, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(m_Device, m_SwapChain, VK_NULL_HANDLE);
    vkDestroyDevice(m_Device, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VK_NULL_HANDLE);
    vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
}

void VulkanRenderAPI::Draw() {
    /** Begin vulkan render. */
    for (uint32_t i = 0; i < m_SwapChainImageSize; i++) {
        /* start command buffers record. */
        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional

        vkBeginCommandBuffer(m_CommandBuffers[i], &commandBufferBeginInfo);

        /* start render pass. */
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_RenderPass;
        renderPassBeginInfo.framebuffer = m_SwapChainFramebuffers[i];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = m_SwapChainExtent;

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        /* bind graphics pipeline. */
        vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

        /* bind vertex buffer */
        VkBuffer buffers[] = {m_VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, buffers, offsets);

        /* draw call */
        vkCmdDraw(m_CommandBuffers[i], std::size(triangleVertices), 1, 0, 0);

        /* end render pass */
        vkCmdEndRenderPass(m_CommandBuffers[i]);

        /* end command buffer record. */
        if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to record command buffer!");
    }

    uint32_t imageIndex;
    vkAcquireNextImageKHR(m_Device, m_SwapChain, std::numeric_limits<uint64_t>::max(),
                          m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    /* submit command buffer */
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_SwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(m_PresentQueue, &presentInfo);
    vkQueueWaitIdle(m_PresentQueue);
}

void VulkanRenderAPI::CreateVertexBuffer() {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = sizeof(triangleVertices[0]) * std::size(triangleVertices);
    bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkFourierCreate(Buffer, m_Device, &bufferCreateInfo, VK_NULL_HANDLE, &m_VertexBuffer);

    /** Query memory requirements. */
    vkGetBufferMemoryRequirements(m_Device, m_VertexBuffer, &m_MemoryRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = m_MemoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = FindMemoryType(m_MemoryRequirements.memoryTypeBits, m_PhysicalDevice,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkFourierAllocate(Memory, m_Device, &memoryAllocInfo, VK_NULL_HANDLE, &m_VertexBufferMemory);

    vkBindBufferMemory(m_Device, m_VertexBuffer, m_VertexBufferMemory, 0);

    /* 填充顶点缓冲区 */
    void *data;
    vkMapMemory(m_Device, m_VertexBufferMemory, 0, bufferCreateInfo.size, 0, &data);
    memcpy(data, std::data(triangleVertices), (size_t) bufferCreateInfo.size);
    vkUnmapMemory(m_Device, m_VertexBufferMemory);
}