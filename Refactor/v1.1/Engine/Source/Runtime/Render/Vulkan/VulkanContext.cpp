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
#include "VulkanContext.h"
#include "VulkanTypedef.h"
#define GLFW_INCLUDE_VULKAN
#include "Window/GLWIN.h"

void VulkanContext::_GetVulkanContextInstanceRequiredExtensions(Vector<const char *> &extensions) {
#ifdef _glfw3_h_
    uint32_t glfwRequiredExtensionCount;
    const char **glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredExtensionCount);
    for (uint32_t i = 0; i < glfwRequiredExtensionCount; i++)
        extensions.push_back(glfwRequiredExtensions[i]);
#endif
}

void VulkanContext::_GetVulkanContextInstanceRequiredLayers(Vector<const char *> &layers) {
#ifdef SPORTS_ENGINE_CONFIG_ENABLE_DEBUG
    layers.push_back(VK_LAYER_KHRONOS_validation);
#endif
}

void VulkanContext::_GetVulkanContextDeviceRequiredExtensions(Vector<const char *> &extensions) {
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void VulkanContext::_GetVulkanContextDeviceRequiredLayers(Vector<const char *> &layers) {
// do nothing...
}

void VulkanContext::_GetVulkanContextMostPreferredPhysicalDevice(VRACDevice device) {
    uint32_t vpdCount;
    vkEnumeratePhysicalDevices(device->Instance, &vpdCount, VK_NULL_HANDLE);
    Vector<VkPhysicalDevice> physicalDevices(vpdCount);
    vkEnumeratePhysicalDevices(device->Instance, &vpdCount, std::data(physicalDevices));

    VkPhysicalDevice pd = physicalDevices[0];
    device->PhysicalDevice = pd;
    vkGetPhysicalDeviceProperties(pd, &device->Properties);
    vkGetPhysicalDeviceFeatures(pd, &device->Features);
}

void VulkanContext::_FindVulkanContextDeviceQueueFamilyIndices(const VRACDevice device, const VRACWindow window, VulkanContext::QueueFamilyIndices *pQueueFamilyIndices) {
    uint32_t queueCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device->PhysicalDevice, &queueCount, VK_NULL_HANDLE);
    Vector<VkQueueFamilyProperties> properties(queueCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device->PhysicalDevice, &queueCount, std::data(properties));

    uint32_t i = 0;
    for (const auto& queueFamilyProperties : properties) {
        if (queueFamilyProperties.queueCount > 0) {
            if (queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                pQueueFamilyIndices->GraphicsQueueFamily = i;

            VkBool32 isPresentMode;
            vkGetPhysicalDeviceSurfaceSupportKHR(device->PhysicalDevice, i, window->SurfaceKHR, &isPresentMode);
            if (isPresentMode)
                pQueueFamilyIndices->PresentQueueFamily = i;
        }

        if (CompleteQueueFamilyIndices(*pQueueFamilyIndices))
            break;

        ++i;
    }
}

VulkanContext::VulkanContext(const GLWIN *pWIN) : m_GLWIN(pWIN) {
    /* init vulkan render context */
    Init_VulkanR_Context();
}

VulkanContext::~VulkanContext() {
    vkDestroyDevice(m_VRACDevice->Device, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(m_VRACDevice->Instance, m_VRACWindow->SurfaceKHR, VK_NULL_HANDLE);
    vkDestroyInstance(m_VRACDevice->Instance, VK_NULL_HANDLE);
    /* free */
    free(m_VRACWindow);
    free(m_VRACDevice);
}

void VulkanContext::CreateSwapchainKHR(VRACSwapchainKHR *pVRACSwapchainKHR) {
    VRACSwapchainKHR swapchain = (_VRACSwapchainKHR_t *) malloc(sizeof(_VRACSwapchainKHR_t));
    swapchain->Window = m_VRACWindow;
    ConfigurationSwapchainKHRSupportDetails(m_VRACDevice, swapchain);

    /* build create info */
    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = SPORTS_MEMORY_INIT;
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    vkVRACCreate(SwapchainKHR, m_VRACDevice->Device, &swapchainCreateInfoKHR, VK_NULL_HANDLE, &swapchain->SwapchainKHR);
    *pVRACSwapchainKHR = swapchain;
}

void VulkanContext::DestroySwpachainKHR(VRACSwapchainKHR swapchainKHR) {
    vkDestroySwapchainKHR(m_VRACDevice->Device, swapchainKHR->SwapchainKHR, VK_NULL_HANDLE);
    free(swapchainKHR);
}

void VulkanContext::Init_VulkanR_Context(void) {
    m_VRACWindow = (_VRACWindow_t *) malloc(sizeof(_VRACWindow_t));
    m_VRACDevice = (_VRACDevice_t *) malloc(sizeof(_VRACDevice_t));

    /** Create vulkan instance. */
    struct VkApplicationInfo applicationInfo = SPORTS_MEMORY_INIT;
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_VERSION_1_3;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = SPORTS_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = SPORTS_ENGINE_NAME;

    struct VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    Vector<const char *> requiredEnableExtensionsForInstance;
    VulkanContext::_GetVulkanContextInstanceRequiredExtensions(requiredEnableExtensionsForInstance);
    instanceCreateInfo.enabledExtensionCount = std::size(requiredEnableExtensionsForInstance);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(requiredEnableExtensionsForInstance);

    Vector<const char *> requiredEnableLayersForInstance;
    VulkanContext::_GetVulkanContextInstanceRequiredLayers(requiredEnableLayersForInstance);
    instanceCreateInfo.enabledLayerCount = std::size(requiredEnableLayersForInstance);
    instanceCreateInfo.ppEnabledLayerNames = std::data(requiredEnableLayersForInstance);

    vkVRACCreate(Instance, &instanceCreateInfo, VK_NULL_HANDLE, &m_VRACDevice->Instance);

    m_VRACWindow->Context = m_GLWIN;
    m_GLWIN->CreateWindowSurfaceKHR(m_VRACDevice->Instance, VK_NULL_HANDLE, &m_VRACWindow->SurfaceKHR);

    /** Create vulkan device. */
    _GetVulkanContextMostPreferredPhysicalDevice(m_VRACDevice);

    float queuePriority = 1.0f;
    VulkanContext::QueueFamilyIndices queueFamilyIndices;
    _FindVulkanContextDeviceQueueFamilyIndices(m_VRACDevice, m_VRACWindow, &queueFamilyIndices);

    VkDeviceQueueCreateInfo graphicsDeviceQueueCreateInfo = {};
	graphicsDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsDeviceQueueCreateInfo.queueCount = 1;
	graphicsDeviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.GraphicsQueueFamily;
	graphicsDeviceQueueCreateInfo.pQueuePriorities = &queuePriority;

	VkDeviceQueueCreateInfo presentDeviceQueueCreateInfo = {};
	presentDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	presentDeviceQueueCreateInfo.queueCount = 1;
	presentDeviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.PresentQueueFamily;
	presentDeviceQueueCreateInfo.pQueuePriorities = &queuePriority;

    Array<VkDeviceQueueCreateInfo, 2> deviceQueueCreateInfos = {
        graphicsDeviceQueueCreateInfo, presentDeviceQueueCreateInfo
    };

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    VkPhysicalDeviceFeatures features = SPORTS_MEMORY_INIT;
    deviceCreateInfo.pEnabledFeatures = &features;

    Vector<const char *> requiredEnableExtensions;
    VulkanContext::_GetVulkanContextDeviceRequiredExtensions(requiredEnableExtensions);
    deviceCreateInfo.enabledExtensionCount = std::size(requiredEnableExtensions);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(requiredEnableExtensions);

    Vector<const char *> requiredEnableLayers;
    VulkanContext::_GetVulkanContextDeviceRequiredLayers(requiredEnableLayers);
    deviceCreateInfo.enabledLayerCount = std::size(requiredEnableLayers);
    deviceCreateInfo.ppEnabledLayerNames = std::data(requiredEnableLayers);

    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);

    vkVRACCreate(Device, m_VRACDevice->PhysicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &m_VRACDevice->Device);
}