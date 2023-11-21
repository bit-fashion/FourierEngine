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

#define VK_LAYER_LUNARG_standard_validation "VK_LAYER_LUNARG_standard_validation"

#define vkFourierCreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        fourier::error("FourierEngine Error: create vulkan object handle failed!")

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

/* Find queue family. */
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices queueFamilyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, std::data(queueFamilies));

    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices.graphicsFamily = i;

        if (queueFamilyIndices.isComplete())
            break;

        ++i;
    }

    return queueFamilyIndices;
}

RendererAPI::RendererAPI() {
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
        fourier::error("FourierEngine Error: cannot found physical device for support vulkan api!");

    std::cout << "FourierEngine Renderer API: All physical devices supports for vulkan: " << std::endl;
    for (auto &device : m_FourierPhysicalDevices)
        std::cout << "    " << device.deviceName << std::endl;

    /** Select current using physical device. */
    FourierPhysicalDevice fourierPhysicalDevice = m_FourierPhysicalDevices[0];
    m_PhysicalDevice = fourierPhysicalDevice.handle;
    std::cout << "FourierEngine Renderer API: Using device: " << "<" << fourierPhysicalDevice.deviceName << ">" << std::endl;

    /** Find queue for graphics family and build create device queue struct. */
    m_QueueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);
    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily.value();
    deviceQueueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

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

    /* Check device is support swapchain. */
    FourierGetRequiredDeviceExtensions(m_RequiredDeviceExtensions, m_VkDeviceExtensionPropertiesSupports);

    /** Create logical device object handle. */
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;
    deviceCreateInfo.enabledExtensionCount = std::size(m_RequiredDeviceExtensions);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(m_RequiredDeviceExtensions);
    vkFourierCreate(Device, m_PhysicalDevice, &deviceCreateInfo, VK_NULL_HANDLE, &m_Device);

    std::cout << "FourierEngine Renderer API: The initialize vulkan api success! " << std::endl;
}

RendererAPI::~RendererAPI() {
    vkDestroyDevice(m_Device, VK_NULL_HANDLE);
    vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
}
