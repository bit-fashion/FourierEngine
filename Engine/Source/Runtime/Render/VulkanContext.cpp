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
|* File:           VulkanContext.cpp                                                *|
|* Create Time:    2023/12/30 20:21                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "VulkanContext.h"
#include "VkUtils.h"

VulkanContext::VulkanContext(Window *p_win) : m_Window(p_win)
{
    InitVulkanContextInstance();
    InitVulkanContextSurface();
    InitVulkanContextDevice();
}

VulkanContext::~VulkanContext()
{
    vkDestroyDevice(m_Device, VkUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VkUtils::Allocator);
    vkDestroyInstance(m_Instance, VkUtils::Allocator);
}

void VulkanContext::InitVulkanContextInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = AURORA_ENGINE_NAME;
    applicationInfo.engineVersion =  VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = AURORA_ENGINE_NAME;
    applicationInfo.apiVersion = VK_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    Vector<VkExtensionProperties> extensionProperties;
    VkUtils::EnumerateInstanceExtensionProperties(extensionProperties);

    /* enable extension properties */
    Vector<const char *> enableExtensionProperties;
    uint32_t glfwRequiredInstanceExtensionCount;

    /* 获取 glfw 必要扩展 */
    const char **glfwRequiredInstanceExtensions = glfwGetRequiredInstanceExtensions(&glfwRequiredInstanceExtensionCount);
    for (int i = 0; i < glfwRequiredInstanceExtensionCount; ++i)
        enableExtensionProperties.push_back(glfwRequiredInstanceExtensions[i]);

    instanceCreateInfo.enabledExtensionCount = std::size(enableExtensionProperties);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(enableExtensionProperties);

    Vector<VkLayerProperties> layerProperties;
    VkUtils::EnumerateInstanceLayerProperties(layerProperties);

    /* enable layer properties */
    Vector<const char *> enableLayerProperties;
#ifdef AURORA_ENGINE_ENABLE_DEBUG
    enableLayerProperties.push_back("VK_LAYER_KHRONOS_validation");
#endif
    instanceCreateInfo.enabledLayerCount = std::size(enableLayerProperties);
    instanceCreateInfo.ppEnabledLayerNames = std::data(enableLayerProperties);

    vkCheckCreate(Instance, &instanceCreateInfo, VkUtils::Allocator, &m_Instance);
}

void VulkanContext::InitVulkanContextSurface()
{
    glfwCreateWindowSurface(m_Instance, m_Window->GetHWIN(), VkUtils::Allocator, &m_Surface);
}

void VulkanContext::InitVulkanContextDevice()
{
    VkUtils::GetBestPerformancePhysicalDevice(m_Instance, &m_PhysicalDevice);
    Logger::Debug("Vulkan context physical device using: {}", m_PhysicalDevice.properties.deviceName);

    VkUtils::QueueFamilyIndices queueFamilyIndices;
    VkUtils::FindQueueFamilyIndices(m_PhysicalDevice.device, m_Surface, &queueFamilyIndices);
    m_GraphicsQueueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    m_PresentQueueFamilyIndex = queueFamilyIndices.presentQueueFamily;

    float priorities = 1.0f;
    std::array<VkDeviceQueueCreateInfo, 2> deviceQueueCreateInfos = {};
    deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[0].queueCount = 1;
    deviceQueueCreateInfos[0].queueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    deviceQueueCreateInfos[0].pQueuePriorities = &priorities;

    deviceQueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[1].queueCount = 1;
    deviceQueueCreateInfos[1].queueFamilyIndex = queueFamilyIndices.presentQueueFamily;
    deviceQueueCreateInfos[1].pQueuePriorities = &priorities;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);
    static VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;

    Vector<VkExtensionProperties> deviceExtensionProperties;
    VkUtils::EnumerateDeviceExtensionProperties(m_PhysicalDevice.device, deviceExtensionProperties);

    Vector<const char *> enableDeviceExtensionProperties;
    deviceCreateInfo.enabledExtensionCount = std::size(enableDeviceExtensionProperties);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(enableDeviceExtensionProperties);

    Vector<VkLayerProperties> layerExtensionProperties;
    VkUtils::EnumerateDeviceLayerProperties(m_PhysicalDevice.device, layerExtensionProperties);

    Vector<const char *> enableDeviceLayerProperties;
#ifdef AURORA_ENGINE_ENABLE_DEBUG
    enableDeviceLayerProperties.push_back("VK_LAYER_KHRONOS_validation");
#endif
    deviceCreateInfo.enabledLayerCount = std::size(enableDeviceLayerProperties);
    deviceCreateInfo.ppEnabledLayerNames = std::data(enableDeviceLayerProperties);

    vkCheckCreate(Device, m_PhysicalDevice.device, &deviceCreateInfo, VkUtils::Allocator, &m_Device);

    /* 获取队列 */
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
}
