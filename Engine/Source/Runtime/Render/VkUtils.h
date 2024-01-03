/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|*    Copyright (C) 2023 bit-fashion                                                *|
|*                                                                                  *|
|*    This program is free software: you can redistribute it and/or modify          *|
|*    it under the terms of the GNU General Public License as published by          *|
|*    the Free Software Foundation, either version 3 of the License, or             *|
|*    (at your option) any later version.                                           *|
|*                                                                                  *|
|*    This program is distributed in the hope that it will be useful,               *|
|*    but WITHOUT ANY WARRANTY; without even the implied warranty of                *|
|*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *|
|*    GNU General Public License for more details.                                  *|
|*                                                                                  *|
|*    You should have received a copy of the GNU General Public License             *|
|*    along with this program.  If not, see <https://www.gnu.org/licenses/>.        *|
|*                                                                                  *|
|*    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.    *|
|*    This is free software, and you are welcome to redistribute it                 *|
|*    under certain conditions; type `show c' for details.                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|* File:           VkUtils.h                                                        *|
|* Create Time:    2024/01/03 01:35                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

// aurora
#include <Aurora/Engine.h>

/* 检查 vulkan 对象是否创建成功 */
#define vkCheckCreate(name, ...) \
    { \
        VkResult ret = vkCreate##name(__VA_ARGS__); \
        if (ret != VK_SUCCESS) \
            Logger::Error("VulkanContext create vulkan {} object failed! VkResult status: {}", #name, VkUtils::GetVkResultStatusName(ret)); \
    }

namespace VkUtils
{
    /* VK 分配器 */
    static VkAllocationCallbacks *Allocator = null;

    static HashMap<VkResult, std::string> _VkResultKeyMap = {
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

    struct QueueFamilyIndices {
        uint32_t graphicsQueueFamily = 0;
        uint32_t presentQueueFamily = 0;
    };

    /* 获取 Result 映射的值 */
    static std::string_view GetVkResultStatusName(VkResult result)
    {
        return _VkResultKeyMap[result];
    }

    static void EnumerateInstanceExtensionProperties(Vector<VkExtensionProperties> &properties)
    {
        uint32_t count;
        vkEnumerateInstanceExtensionProperties(null, &count, null);
        properties.resize(count);
        vkEnumerateInstanceExtensionProperties(null, &count, std::data(properties));

        /* 遍历 extension 属性列表 */
        Logger::Debug("Vulkan instance support extension properties: ");
        for (const auto &property : properties)
            Logger::Debug("  name: {}, version: {}.{}.{}", property.extensionName, VK_VERSION_MAJOR(property.specVersion),VK_VERSION_MINOR(property.specVersion),
                          VK_VERSION_PATCH(property.specVersion));
    }

    static void EnumerateInstanceLayerProperties(Vector<VkLayerProperties> &properties)
    {
        uint32_t count;
        vkEnumerateInstanceLayerProperties(&count, null);
        properties.resize(count);
        vkEnumerateInstanceLayerProperties(&count, std::data(properties));

        /* 遍历 layer 属性列表 */
        Logger::Debug("Vulkan instance support layer properties: ");
        for (const auto &property : properties)
            Logger::Debug("  name: {}, version: {}.{}.{}", property.layerName, VK_VERSION_MAJOR(property.specVersion),VK_VERSION_MINOR(property.specVersion),
                          VK_VERSION_PATCH(property.specVersion));
    }

    static void GetBestPerformancePhysicalDevice(VkInstance instance, VkContextPhysicalDevice *pPhysicalDevice)
    {
        uint32_t count;
        Vector<VkPhysicalDevice> tmp;
        vkEnumeratePhysicalDevices(instance, &count, null);
        tmp.resize(count);
        vkEnumeratePhysicalDevices(instance, &count, std::data(tmp));

        Vector<VkContextPhysicalDevice> devices;
        for (const auto &device : tmp) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);
            devices.emplace_back(device, properties, features);
        }

        /* 遍历 layer 属性列表 */
        Logger::Debug("Vulkan available physical device properties: ");
        for (const auto &device : devices)
            Logger::Debug("  device name: {}", device.properties.deviceName);

        *pPhysicalDevice = devices[0];
    }

    static void EnumerateDeviceExtensionProperties(VkPhysicalDevice device, Vector<VkExtensionProperties> &properties)
    {
        uint32_t count;
        vkEnumerateDeviceExtensionProperties(device, null, &count, null);
        properties.resize(count);
        vkEnumerateDeviceExtensionProperties(device, null, &count, std::data(properties));

        /* 遍历 extension 属性列表 */
        Logger::Debug("Vulkan physical device support extension properties: ");
        for (const auto &property : properties)
            Logger::Debug("  name: {}, version: {}.{}.{}", property.extensionName, VK_VERSION_MAJOR(property.specVersion),VK_VERSION_MINOR(property.specVersion),
                          VK_VERSION_PATCH(property.specVersion));
    }

    static void EnumerateDeviceLayerProperties(VkPhysicalDevice device, Vector<VkLayerProperties> &properties)
    {
        uint32_t count;
        vkEnumerateDeviceLayerProperties(device, &count, null);
        properties.resize(count);
        vkEnumerateDeviceLayerProperties(device, &count, std::data(properties));

        /* 遍历 layer 属性列表 */
        Logger::Debug("Vulkan physical device support layer properties: ");
        for (const auto &property : properties)
            Logger::Debug("  name: {}, version: {}.{}.{}", property.layerName, VK_VERSION_MAJOR(property.specVersion),VK_VERSION_MINOR(property.specVersion),
                          VK_VERSION_PATCH(property.specVersion));
    }

    static void FindQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, QueueFamilyIndices *pQueueFamilyIndices)
    {
        QueueFamilyIndices queueFamilyIndices;

        uint32_t queueCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, null);
        Vector<VkQueueFamilyProperties> properties(queueCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, std::data(properties));

        uint32_t index = 0;
        for(const auto &queue : properties) {
            if (queue.queueCount > 0) {
                if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    queueFamilyIndices.graphicsQueueFamily = index;

                VkBool32 isPresetQueue;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, surface, &isPresetQueue);
                if (isPresetQueue)
                    queueFamilyIndices.presentQueueFamily = index;
            }

            if (queueFamilyIndices.graphicsQueueFamily > 0 && queueFamilyIndices.presentQueueFamily > 0)
                break;

            ++index;
        }

        *pQueueFamilyIndices = queueFamilyIndices;
    }

}