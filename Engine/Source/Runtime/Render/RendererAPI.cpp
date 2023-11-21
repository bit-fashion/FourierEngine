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

#define vkFourierCreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        fourier::error("[FourierEngine Error] - create vulkan object handle failed!")

/* Get required instance extensions for vulkan. */
void FourierGetRequiredInstanceExtensions(std::vector<const char *> *p_vext) {
    /* glfw */
    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (int i = 0; i < glfwExtensionCount; i++)
        p_vext->push_back(glfwExtensions[i]);
}

/* Get required instance extensions for vulkan. */
void FourierGetRequiredInstanceLayers(std::vector<const char *> *p_vext) {
#ifndef FOURIER_NDEBUG
    p_vext->push_back("VK_LAYER_LUNARG_standard_validation");
#endif
}

RendererAPI::RendererAPI() {
    /* Get extensions & layers. */
    FourierGetRequiredInstanceExtensions(&m_RequiredInstanceExtensions);
    FourierGetRequiredInstanceLayers(&m_RequiredInstanceLayers);

    /* Enumerate instance available extensions. */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, std::data(extensions));
    std::cout << "[FourierEngine Renderer API] - Vulkan render api available extensions for instance: " << std::endl;
    for (auto &extension : extensions)
        std::cout << "    " << extension.extensionName << std::endl;

    /* Enumerate instance available extensions. */
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(layers));
    std::cout << "[FourierEngine Renderer API] - Vulkan render api available layer list: " << std::endl;
    for (auto &layer : layers)
        std::cout << "    " << layer.layerName << std::endl;

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
        fourier::error("[FourierEngine Error] - cannot found physical device for support vulkan api!");

    std::cout << "[FourierEngine Renderer API] - All physical devices supports for vulkan: " << std::endl;
    for (auto &device : m_FourierPhysicalDevices)
        std::cout << "    " << device.deviceName << std::endl;

    /** Select current using physical device. */
    FourierPhysicalDevice fourierPhysicalDevice = m_FourierPhysicalDevices[0];
    m_PhysicalDevice = fourierPhysicalDevice.handle;
    std::cout << "[FourierEngine Renderer API] - Using device: " << "<" << fourierPhysicalDevice.deviceName << ">" << std::endl;
}

RendererAPI::~RendererAPI() {
    vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
}
