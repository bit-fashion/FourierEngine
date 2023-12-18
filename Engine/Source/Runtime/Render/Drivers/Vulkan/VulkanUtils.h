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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, e1ither express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ************************************************************************/

/* Creates on 2022/9/14. */

/*
  ===================================
    @author bit-fashion
  ===================================
*/
#ifndef _SPORTS_VULKAN_UTILS_H_
#define _SPORTS_VULKAN_UTILS_H_

#define VK_LAYER_KHRONOS_validation "VK_LAYER_KHRONOS_validation"

namespace VulkanUtils {

    static VkAllocationCallbacks *Allocator = VK_NULL_HANDLE;

    static HashMap<VkResult, String> _VulkanResultKeyMapping = {
            { VK_SUCCESS, "VK_SUCCESS" },
            { VK_NOT_READY, "VK_NOT_READY" },
            { VK_TIMEOUT, "VK_TIMEOUT" },
            { VK_EVENT_SET, "VK_EVENT_SET" },
            { VK_EVENT_RESET, "VK_EVENT_RESET" },
            { VK_INCOMPLETE, "VK_INCOMPLETE" },
            { VK_ERROR_OUT_OF_HOST_MEMORY, "VK_ERROR_OUT_OF_HOST_MEMORY" },
            { VK_ERROR_OUT_OF_DEVICE_MEMORY, "VK_ERROR_OUT_OF_DEVICE_MEMORY" },
            { VK_ERROR_INITIALIZATION_FAILED, "VK_ERROR_INITIALIZATION_FAILED" },
            { VK_ERROR_DEVICE_LOST, "VK_ERROR_DEVICE_LOST" },
            { VK_ERROR_MEMORY_MAP_FAILED, "VK_ERROR_MEMORY_MAP_FAILED" },
            { VK_ERROR_LAYER_NOT_PRESENT, "VK_ERROR_LAYER_NOT_PRESENT" },
            { VK_ERROR_EXTENSION_NOT_PRESENT, "VK_ERROR_EXTENSION_NOT_PRESENT" },
            { VK_ERROR_FEATURE_NOT_PRESENT, "VK_ERROR_FEATURE_NOT_PRESENT" },
            { VK_ERROR_INCOMPATIBLE_DRIVER, "VK_ERROR_INCOMPATIBLE_DRIVER" },
            { VK_ERROR_TOO_MANY_OBJECTS, "VK_ERROR_TOO_MANY_OBJECTS" },
            { VK_ERROR_FORMAT_NOT_SUPPORTED, "VK_ERROR_FORMAT_NOT_SUPPORTED" },
            { VK_ERROR_FRAGMENTED_POOL, "VK_ERROR_FRAGMENTED_POOL" },
            { VK_ERROR_UNKNOWN, "VK_ERROR_UNKNOWN" },
            { VK_ERROR_OUT_OF_POOL_MEMORY, "VK_ERROR_OUT_OF_POOL_MEMORY" },
            { VK_ERROR_INVALID_EXTERNAL_HANDLE, "VK_ERROR_INVALID_EXTERNAL_HANDLE" },
            { VK_ERROR_FRAGMENTATION, "VK_ERROR_FRAGMENTATION" },
            { VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS, "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS" },
            { VK_PIPELINE_COMPILE_REQUIRED, "VK_PIPELINE_COMPILE_REQUIRED" },
            { VK_ERROR_SURFACE_LOST_KHR, "VK_ERROR_SURFACE_LOST_KHR" },
            { VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" },
            { VK_SUBOPTIMAL_KHR, "VK_SUBOPTIMAL_KHR" },
            { VK_ERROR_OUT_OF_DATE_KHR, "VK_ERROR_OUT_OF_DATE_KHR" },
            { VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" },
            { VK_ERROR_VALIDATION_FAILED_EXT, "VK_ERROR_VALIDATION_FAILED_EXT" },
            { VK_ERROR_INVALID_SHADER_NV, "VK_ERROR_INVALID_SHADER_NV" },
            { VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR, "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR" },
            { VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR, "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR" },
            { VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR, "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR" },
            { VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR, "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR" },
            { VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR, "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR" },
            { VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR, "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR" },
            { VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT, "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT" },
            { VK_ERROR_NOT_PERMITTED_KHR, "VK_ERROR_NOT_PERMITTED_KHR" },
            { VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" },
            { VK_THREAD_IDLE_KHR, "VK_THREAD_IDLE_KHR" },
            { VK_THREAD_DONE_KHR, "VK_THREAD_DONE_KHR" },
            { VK_OPERATION_DEFERRED_KHR, "VK_OPERATION_DEFERRED_KHR" },
            { VK_OPERATION_NOT_DEFERRED_KHR, "VK_OPERATION_NOT_DEFERRED_KHR" },
#ifdef VK_ENABLE_BETA_EXTENSIONS
            { VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR, "VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR" },
#endif
            { VK_ERROR_COMPRESSION_EXHAUSTED_EXT, "VK_ERROR_COMPRESSION_EXHAUSTED_EXT" },
            { VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT, "VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT" },
            { VK_ERROR_OUT_OF_POOL_MEMORY_KHR, "VK_ERROR_OUT_OF_POOL_MEMORY_KHR" },
            { VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR, "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR" },
            { VK_ERROR_FRAGMENTATION_EXT, "VK_ERROR_FRAGMENTATION_EXT" },
            { VK_ERROR_NOT_PERMITTED_EXT, "VK_ERROR_NOT_PERMITTED_EXT" },
            { VK_ERROR_INVALID_DEVICE_ADDRESS_EXT, "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT" },
            { VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR, "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR" },
            { VK_PIPELINE_COMPILE_REQUIRED_EXT, "VK_PIPELINE_COMPILE_REQUIRED_EXT" },
            { VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT, "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT" },
            { VK_RESULT_MAX_ENUM, "VK_RESULT_MAX_ENUM" },
    };
#define VulkanDebugResult(ret) printf("%s\n", VulkanUtils::_VulkanResultKeyMapping[ret].c_str())

    struct QueueFamilyIndices {
        uint32_t graphicsQueueFamily = 0;
        uint32_t presentQueueFamily = 0;
    };

    static VkBool32 _CheckQueueFamilyIndicesComplete(QueueFamilyIndices &queueFamilyIndices) {
        return queueFamilyIndices.graphicsQueueFamily > 0 && queueFamilyIndices.presentQueueFamily > 0;
    }

    static void GetVulkanMostPreferredPhysicalDevice(VkInstance instance, VkPhysicalDevice *pDevice, VkPhysicalDeviceProperties *pProperties,
                                                     VkPhysicalDeviceFeatures *pFeatures) {
        uint32_t vpdCount;
        vkEnumeratePhysicalDevices(instance, &vpdCount, VK_NULL_HANDLE);
        Vector<VkPhysicalDevice> physicalDevices(vpdCount);
        vkEnumeratePhysicalDevices(instance, &vpdCount, std::data(physicalDevices));

        VkPhysicalDevice device = physicalDevices[0];
        *pDevice = device;
        vkGetPhysicalDeviceProperties(device, pProperties);
        vkGetPhysicalDeviceFeatures(device, pFeatures);
    }

    inline static void GetVulkanInstanceRequiredExtensions(Vector<const char *> &required) {
#ifdef _glfw3_h_
        uint32_t glfwRequiredExtensionCount;
        const char **glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionCount);
        for (uint32_t i = 0; i < glfwRequiredExtensionCount; i++) {
            required.push_back(glfwRequiredExtensions[i]);
        }
#endif
    }

    inline static void GetVulkanInstanceRequiredLayers(Vector<const char *> &required) {
#ifdef ENABLE_VULKAN_VALIDATION_LAYER
        required.push_back(VK_LAYER_KHRONOS_validation);
#endif
    }

    inline void GetVulkanDeviceRequiredExtensions(Vector<const char *> &required) {
        required.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    inline void GetVulkanDeviceRequiredLayers(Vector<const char *> &required) {
        // DO NOTHING...
    }

    void FindVulkanDeviceQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface, QueueFamilyIndices *pQueueFamilyIndices) {
        uint32_t queueCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, VK_NULL_HANDLE);
        Vector<VkQueueFamilyProperties> properties(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, std::data(properties));

        uint32_t i = 0;
        for (const auto& queueFamilyProperties : properties) {
            if (queueFamilyProperties.queueCount > 0) {
                if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    pQueueFamilyIndices->graphicsQueueFamily = i;

                VkBool32 isPresentMode;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &isPresentMode);
                if (isPresentMode)
                    pQueueFamilyIndices->presentQueueFamily = i;
            }

            if (_CheckQueueFamilyIndicesComplete(*pQueueFamilyIndices))
                break;

            ++i;
        }
    }

#ifdef _glfw3_h_
    inline static void CreateWindowSurfaceKHR(VkInstance instance, GLFWwindow *window, VkSurfaceKHR *pSurfaceKHR) {
        glfwCreateWindowSurface(instance, window, Allocator, pSurfaceKHR);
    }
#endif

    static QueueFamilyIndices GetVulkanDeviceCreateRequiredQueueFamilyAndQueueCreateInfo(VkPhysicalDevice device, VkSurfaceKHR surface,
                                                                           Vector<VkDeviceQueueCreateInfo> &deviceQueueCreateInfos) {
        /** Create vulkan device. */
        float queuePriority = 1.0f;
        VulkanUtils::QueueFamilyIndices queueFamilyIndices;
        FindVulkanDeviceQueueFamilyIndices(device, surface, &queueFamilyIndices);

        VkDeviceQueueCreateInfo graphicsDeviceQueueCreateInfo = {};
        graphicsDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicsDeviceQueueCreateInfo.queueCount = 1;
        graphicsDeviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
        graphicsDeviceQueueCreateInfo.pQueuePriorities = &queuePriority;

        VkDeviceQueueCreateInfo presentDeviceQueueCreateInfo = {};
        presentDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        presentDeviceQueueCreateInfo.queueCount = 1;
        presentDeviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.presentQueueFamily;
        presentDeviceQueueCreateInfo.pQueuePriorities = &queuePriority;

        deviceQueueCreateInfos.push_back(graphicsDeviceQueueCreateInfo);
        deviceQueueCreateInfos.push_back(presentDeviceQueueCreateInfo);
    }

    static void _SelectVulkanSwapchainSurfaceFormatKHR(Vector<VkSurfaceFormatKHR> &formats, VkSurfaceFormatKHR *pSurfaceFormat) {
        if (std::size(formats) == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            *pSurfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
            return;
        }

        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                *pSurfaceFormat = format;
                return;
            }
        }

        *pSurfaceFormat = formats[0];
    }

    static void _SelectVulkanSwapchainPresentModeKHR(Vector<VkPresentModeKHR> &presentModes, VkPresentModeKHR *pPresentMode) {
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

        for (const auto& presentMode : presentModes) {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                bestMode = presentMode;
                goto _SelectVulkanSwapchainPresentModeKHREndTag;
            } else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                bestMode = presentMode;
                goto _SelectVulkanSwapchainPresentModeKHREndTag;
            }
        }
    _SelectVulkanSwapchainPresentModeKHREndTag:
        *pPresentMode = bestMode;
    }

    static void ConfigurationVulkanSwapchainContextDetail(VkPhysicalDevice device, VkSurfaceKHR surface, const Window *pWindow,
                                                          VkSwapchainContextKHR *pSwapchainContext) {
        pSwapchainContext->surface = surface;
        pSwapchainContext->window = pWindow;
        auto windowExtent2D = pWindow->GetWindowExtent2D();
        pSwapchainContext->width = windowExtent2D.width;
        pSwapchainContext->height = windowExtent2D.height;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &pSwapchainContext->capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, null);
        Vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, std::data(formats));
        _SelectVulkanSwapchainSurfaceFormatKHR(formats, &pSwapchainContext->surfaceFormat);

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, null);
        Vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, std::data(presentModes));
        _SelectVulkanSwapchainPresentModeKHR(presentModes, &pSwapchainContext->presentMode);

        if (pSwapchainContext->capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            pSwapchainContext->width = pSwapchainContext->capabilities.currentExtent.width;
            pSwapchainContext->height = pSwapchainContext->capabilities.currentExtent.height;
        } else {
            VkExtent2D actualExtent = { static_cast<uint32_t>(windowExtent2D.width), static_cast<uint32_t>(windowExtent2D.height) };

            actualExtent.width = std::max(pSwapchainContext->capabilities.minImageExtent.width, std::min(pSwapchainContext->capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(pSwapchainContext->capabilities.minImageExtent.height, std::min(pSwapchainContext->capabilities.maxImageExtent.height, actualExtent.height));

            pSwapchainContext->width = actualExtent.width;
            pSwapchainContext->height = actualExtent.height;
        }

        /* 设置三重缓冲 */
        pSwapchainContext->minImageCount = pSwapchainContext->capabilities.minImageCount + 1;
        if (pSwapchainContext->capabilities.maxImageCount > 0 && pSwapchainContext->minImageCount > pSwapchainContext->capabilities.maxImageCount)
            pSwapchainContext->minImageCount = pSwapchainContext->capabilities.maxImageCount;
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

}

#endif /* _SPORTS_VULKAN_UTILS_H_ */
