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
#include "VulkanContext.h"
#include "Window/Window.h"
#include "VulkanUtils.h"

VulkanContext::VulkanContext(const Window *pWindow) : m_Window(pWindow) {
    InitVulkanDriverContext();
}

VulkanContext::~VulkanContext() {
    vkDestroySwapchainKHR(m_Device, m_SwapchainKHR, VulkanUtils::Allocator);
    vkDestroyDevice(m_Device, VulkanUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_SurfaceKHR, VulkanUtils::Allocator);
    vkDestroyInstance(m_Instance, VulkanUtils::Allocator);
}

void VulkanContext::CreateVulkanSwapchainKHR(VkSwapchainKHR *pSwapchain) {
    VulkanUtils::SwapchainSupportDetail swapchainSupportDetail;
    VulkanUtils::GetVulkanSwapchainSupportDetail(m_PhysicalDevice, m_SurfaceKHR, m_Window, &swapchainSupportDetail);
    
    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.surface = swapchainSupportDetail.surface;
    swapchainCreateInfoKHR.minImageCount = swapchainSupportDetail.minImageCount;
    swapchainCreateInfoKHR.imageFormat = swapchainSupportDetail.surfaceFormat.format;
    swapchainCreateInfoKHR.imageColorSpace = swapchainSupportDetail.surfaceFormat.colorSpace;
    swapchainCreateInfoKHR.imageExtent = { swapchainSupportDetail.width, swapchainSupportDetail.height };
    swapchainCreateInfoKHR.imageArrayLayers = 1;
    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKHR.preTransform = swapchainSupportDetail.capabilities.currentTransform;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.presentMode = swapchainSupportDetail.presentMode;
    swapchainCreateInfoKHR.clipped = VK_TRUE;
    swapchainCreateInfoKHR.oldSwapchain = null;

    vkCreateSwapchainKHR(m_Device, &swapchainCreateInfoKHR, null, pSwapchain);
}

void VulkanContext::InitVulkanDriverContext() {
    /* Create vulkan instance. */
    struct VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_VERSION_1_3;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = SPORTS_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = SPORTS_ENGINE_NAME;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    static Vector<const char *> requiredEnableExtensionsForInstance;
    VulkanUtils::GetVulkanInstanceRequiredExtensions(requiredEnableExtensionsForInstance);
    instanceCreateInfo.enabledExtensionCount = std::size(requiredEnableExtensionsForInstance);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(requiredEnableExtensionsForInstance);

    static Vector<const char *> requiredEnableLayersForInstance;
    VulkanUtils::GetVulkanInstanceRequiredLayers(requiredEnableLayersForInstance);
    instanceCreateInfo.enabledLayerCount = std::size(requiredEnableLayersForInstance);
    instanceCreateInfo.ppEnabledLayerNames = std::data(requiredEnableLayersForInstance);
    vkCreateInstance(&instanceCreateInfo, VulkanUtils::Allocator, &m_Instance);

    /* Create window surface */
#ifdef _glfw3_h_
    VulkanUtils::CreateWindowSurfaceKHR(m_Instance, m_Window->GetHandle(), &m_SurfaceKHR);
#endif

    /* Create vulkan device. */
    VulkanUtils::GetVulkanMostPreferredPhysicalDevice(m_Instance, &m_PhysicalDevice, &m_PhysicalDeviceProperties,
                                                      &m_PhysicalDeviceFeature);
    Vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    VulkanUtils::GetVulkanDeviceCreateRequiredQueueCreateInfo(m_PhysicalDevice, m_SurfaceKHR, deviceQueueCreateInfos);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    static VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;

    static Vector<const char *> requiredEnableExtensions;
    VulkanUtils::GetVulkanDeviceRequiredExtensions(requiredEnableExtensions);
    deviceCreateInfo.enabledExtensionCount = std::size(requiredEnableExtensions);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(requiredEnableExtensions);

    static Vector<const char *> requiredEnableLayers;
    VulkanUtils::GetVulkanDeviceRequiredLayers(requiredEnableLayers);
    deviceCreateInfo.enabledLayerCount = std::size(requiredEnableLayers);
    deviceCreateInfo.ppEnabledLayerNames = std::data(requiredEnableLayers);

    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);

    vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, VulkanUtils::Allocator, &m_Device);

    /* Create swapchain */
    CreateVulkanSwapchainKHR(&m_SwapchainKHR);
}