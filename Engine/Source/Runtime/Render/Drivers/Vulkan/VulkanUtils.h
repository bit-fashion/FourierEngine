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

    struct QueueFamilyIndices {
        uint32_t graphicsQueueFamily = 0;
        uint32_t presentQueueFamily = 0;
    };

    static VkBool32 _CheckQueueFamilyIndicesComplete(QueueFamilyIndices &queueFamilyIndices) {
        return queueFamilyIndices.graphicsQueueFamily > 0 && queueFamilyIndices.presentQueueFamily > 0;
    }

    static void GetVulkanMostPreferredPhysicalDevice(VkInstance instance, VulkanContext::DriverPhysicalDevice *pDriverPhysicalDevice) {
        uint32_t vpdCount;
        vkEnumeratePhysicalDevices(instance, &vpdCount, VK_NULL_HANDLE);
        Vector<VkPhysicalDevice> physicalDevices(vpdCount);
        vkEnumeratePhysicalDevices(instance, &vpdCount, std::data(physicalDevices));

        VkPhysicalDevice device = physicalDevices[0];
        pDriverPhysicalDevice->device = device;
        vkGetPhysicalDeviceProperties(device, &pDriverPhysicalDevice->properties);
        vkGetPhysicalDeviceFeatures(device, &pDriverPhysicalDevice->features);
    }

    inline static void _GetVulkanInstanceRequiredExtensions(Vector<const char *> &required) {
#ifdef _glfw3_h_
        uint32_t glfwRequiredExtensionCount;
    const char **glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionCount);
    for (uint32_t i = 0; i < glfwRequiredExtensionCount; i++)
        required.push_back(glfwRequiredExtensions[i]);
#endif
    }

    inline static void _GetVulkanInstanceRequiredLayers(Vector<const char *> &required) {
#ifdef SPORTS_ENGINE_CONFIG_ENABLE_DEBUG
        required.push_back(VK_LAYER_KHRONOS_validation);
#endif
    }

    inline void _GetVulkanDeviceRequiredExtensions(Vector<const char *> &required) {
        required.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    inline void _GetVulkanDeviceRequiredLayers(Vector<const char *> &required) {
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

    /**
     * Configuration vulkan instance create info struct.
     */
    static void ConfigureVulkanInstanceCreateInfo(VkInstanceCreateInfo *pInstanceCreateInfo) {
        struct VkApplicationInfo applicationInfo = {};
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.apiVersion = VK_VERSION_1_3;
        applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pApplicationName = SPORTS_ENGINE_NAME;
        applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName = SPORTS_ENGINE_NAME;

        pInstanceCreateInfo->sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        pInstanceCreateInfo->pApplicationInfo = &applicationInfo;

        Vector<const char *> requiredEnableExtensionsForInstance;
        _GetVulkanInstanceRequiredExtensions(requiredEnableExtensionsForInstance);
        pInstanceCreateInfo->enabledExtensionCount = std::size(requiredEnableExtensionsForInstance);
        pInstanceCreateInfo->ppEnabledExtensionNames = std::data(requiredEnableExtensionsForInstance);

        Vector<const char *> requiredEnableLayersForInstance;
        _GetVulkanInstanceRequiredLayers(requiredEnableLayersForInstance);
        pInstanceCreateInfo->enabledLayerCount = std::size(requiredEnableLayersForInstance);
        pInstanceCreateInfo->ppEnabledLayerNames = std::data(requiredEnableLayersForInstance);
    }

#ifdef _glfw3_h_
    inline static void CreateWindowSurfaceKHR(VkInstance instance, GLFWwindow *pGLFWwindow, VkSurfaceKHR *pSurfaceKHR) {
        glfwCreateWindowSurface(instance, pGLFWwindow, Allocator, pSurfaceKHR);
    }
#endif

    static void GetVulkanDeviceCreateRequiredQueueCreateInfo(VkPhysicalDevice device, VkSurfaceKHR surface, Vector<VkDeviceQueueCreateInfo> &deviceQueueCreateInfos) {
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

    /**
     * Configuration vulkan device create info struct.
     */
    static void ConfigureVulkanDeviceCreateInfo(Vector<VkDeviceQueueCreateInfo> &queueCreateInfos, VkDeviceCreateInfo *pDeviceCreateInfo) {
        pDeviceCreateInfo->sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        VkPhysicalDeviceFeatures features = {};
        pDeviceCreateInfo->pEnabledFeatures = &features;

        Vector<const char *> requiredEnableExtensions;
        _GetVulkanDeviceRequiredExtensions(requiredEnableExtensions);
        pDeviceCreateInfo->enabledExtensionCount = std::size(requiredEnableExtensions);
        pDeviceCreateInfo->ppEnabledExtensionNames = std::data(requiredEnableExtensions);

        Vector<const char *> requiredEnableLayers;
        _GetVulkanDeviceRequiredLayers(requiredEnableLayers);
        pDeviceCreateInfo->enabledLayerCount = std::size(requiredEnableLayers);
        pDeviceCreateInfo->ppEnabledLayerNames = std::data(requiredEnableLayers);

        pDeviceCreateInfo->queueCreateInfoCount = std::size(queueCreateInfos);
        pDeviceCreateInfo->pQueueCreateInfos = std::data(queueCreateInfos);
    }

}

#endif /* _SPORTS_VULKAN_UTILS_H_ */
