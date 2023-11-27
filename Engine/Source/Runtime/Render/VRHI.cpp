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

/* Creates on 2022/11/23. */
#include "VRHI.h"

#include "Window/VRRTwindow.h"

/* Get required instance extensions for vulkan. */
void RHIGetRequiredInstanceExtensions(std::vector<const char *> &vec,
                                          std::unordered_map<std::string, VkExtensionProperties> &supports) {
    /* glfw */
    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (int i = 0; i < glfwExtensionCount; i++)
        vec.push_back(glfwExtensions[i]);
}

/* Get required instance layers for vulkan. */
void RHIGetRequiredInstanceLayers(std::vector<const char *> &vec,
                                      std::unordered_map<std::string, VkLayerProperties> &supports) {
#ifdef VRRT_ENGINE_CONFIG_ENABLE_DEBUG
    if (supports.count(VK_LAYER_KHRONOS_validation) != 0)
        vec.push_back(VK_LAYER_KHRONOS_validation);
#endif
}

/**
 * 获取设备必须启用的扩展列表
 */
static void RHIGetRequiredEnableDeviceExtensions(std::unordered_map<std::string, VkExtensionProperties> &supports,
                                                 std::vector<const char *> *pRequired) {
    if (supports.count(VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
        pRequired->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

RHIQueueFamilyIndices VRHIDevice::FindQueueFamilyIndices() {
    RHIQueueFamilyIndices queueFamilyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_RHIGPU.device, &queueFamilyCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_RHIGPU.device, &queueFamilyCount, std::data(queueFamilies));

    int index = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices.graphicsQueueFamily = index;

        VkBool32 isPresentMode = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_RHIGPU.device, index, m_SurfaceKHR, &isPresentMode);
        if (queueFamilyCount > 0 && isPresentMode)
            queueFamilyIndices.presentQueueFamily= index;

        if (queueFamilyIndices.graphicsQueueFamily > 0 && queueFamilyIndices.presentQueueFamily > 0)
            break;

        ++index;
    }

    return queueFamilyIndices;
}

VRHIDevice::VRHIDevice(VkInstance instance, VkSurfaceKHR surface, VRRTwindow *pRHIwindow)
  : m_Instance(instance), m_SurfaceKHR(surface), m_NRIVwindow(pRHIwindow) {
    /* 选择一个牛逼的 GPU 设备 */
    std::vector<RHIGPU> vGPU;
    RHIGETGPU(m_Instance, &vGPU);
    m_RHIGPU = vGPU[0];

    /* 获取设备支持的所有扩展列表 */
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(m_RHIGPU.device, VK_NULL_HANDLE, &deviceExtensionCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(m_RHIGPU.device, VK_NULL_HANDLE, &deviceExtensionCount, std::data(deviceExtensions));
    for (auto &extension : deviceExtensions)
        m_DeviceSupportedExtensions.insert({extension.extensionName, extension});

    /* 查询设备支持的队列 */
    m_RHIQueueFamilyIndices = FindQueueFamilyIndices();
    std::vector<uint32_t> uniqueQueueFamilies = {m_RHIQueueFamilyIndices.graphicsQueueFamily, m_RHIQueueFamilyIndices.presentQueueFamily};
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

    /** 创建 Vulkan 逻辑设备句柄对象 */
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);
    VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;
    /* 选择设备启用扩展 */
    std::vector<const char *> vRequiredEnableExtension;
    RHIGetRequiredEnableDeviceExtensions(m_DeviceSupportedExtensions, &vRequiredEnableExtension);
    deviceCreateInfo.enabledExtensionCount = std::size(vRequiredEnableExtension);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(vRequiredEnableExtension);
    vkVRHICreate(Device, m_RHIGPU.device, &deviceCreateInfo, nullptr, &m_Device);

    /* 获取队列 */
    vkGetDeviceQueue(m_Device, m_RHIQueueFamilyIndices.graphicsQueueFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_RHIQueueFamilyIndices.graphicsQueueFamily, 0, &m_PresentQueue);
}

VRHIDevice::~VRHIDevice() noexcept {
    vkDestroyDevice(m_Device, VK_NULL_HANDLE);
}

VRHI::VRHI(VRRTwindow *pRHIwindow) : m_NRIVwindow(pRHIwindow) {
    /* Enumerate instance available extensions. */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, std::data(extensions));
    VRHI_LOGGER_VULKAN_API_INFO("Vulkan render api available extensions for instance:");
    for (auto &extension : extensions) {
        VRHI_LOGGER_VULKAN_API_INFO("    {}", extension.extensionName);
        m_VkInstanceExtensionPropertiesSupports.insert({extension.extensionName, extension});
    }

    /* Enumerate instance available layers. */
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(layers));
    VRHI_LOGGER_VULKAN_API_INFO("Vulkan render api available layer list: ");
    for (auto &layer : layers) {
        VRHI_LOGGER_VULKAN_API_INFO("    {}", layer.layerName);
        m_VkInstanceLayerPropertiesSupports.insert({layer.layerName, layer});
    }

    /* Get extensions & layers. */
    RHIGetRequiredInstanceExtensions(m_RequiredInstanceExtensions, m_VkInstanceExtensionPropertiesSupports);
    RHIGetRequiredInstanceLayers(m_RequiredInstanceLayers, m_VkInstanceLayerPropertiesSupports);

    /** Create vulkan instance. */
    struct VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_VERSION_1_3;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = VRRT_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = VRRT_ENGINE_NAME;

    struct VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    instanceCreateInfo.enabledExtensionCount = std::size(m_RequiredInstanceExtensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(m_RequiredInstanceExtensions);
    VRHI_LOGGER_VULKAN_API_INFO("Used vulkan extension for instance count: {}", std::size(m_RequiredInstanceExtensions));
    for (const auto& name : m_RequiredInstanceExtensions)
        VRHI_LOGGER_VULKAN_API_INFO("    {}", name);

    instanceCreateInfo.enabledLayerCount = std::size(m_RequiredInstanceLayers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(m_RequiredInstanceLayers);

    vkVRHICreate(Instance, &instanceCreateInfo, VK_NULL_HANDLE, &m_Instance);

    /** 创建 Surface 接口对象 */
    if (glfwCreateWindowSurface(m_Instance, pRHIwindow->GetWindowHandle(),VK_NULL_HANDLE,
                                &m_Surface) != VK_SUCCESS) {
        VRHI_LOGGER_VULKAN_API_INFO("Create glfw surface failed!");
    }

    /* init */
    Init_Vulkan_Impl();
}

VRHI::~VRHI() {
    VRRT_FREE_POINTER(m_VRHIDevice);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VK_NULL_HANDLE);
    vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
}

void VRHI::Init_Vulkan_Impl() {
    m_VRHIDevice = std::make_unique<VRHIDevice>(m_Instance, m_Surface, m_NRIVwindow);
}