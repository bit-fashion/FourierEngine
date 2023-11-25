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
#include "Render/RivuletRenderer.h"

#include "Window/RIVwindow.h"

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
#ifdef RIVULET_ENABLE_DEBUG
    if (supports.count(VK_LAYER_KHRONOS_validation) != 0)
        vec.push_back(VK_LAYER_KHRONOS_validation);
#endif
}

RivuletRenderer::RivuletRenderer(RIVwindow *pRIVwindow) {
    /* Enumerate instance available extensions. */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, std::data(extensions));
    RIV_RENDERER_LOGGER_INFO("Vulkan render api available extensions for instance:");
    for (auto &extension : extensions) {
        RIV_RENDERER_LOGGER_INFO("    {}", extension.extensionName);
        m_VkInstanceExtensionPropertiesSupports.insert({extension.extensionName, extension});
    }

    /* Enumerate instance available layers. */
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(layers));
    RIV_RENDERER_LOGGER_INFO("Vulkan render api available layer list: ");
    for (auto &layer : layers) {
        RIV_RENDERER_LOGGER_INFO("    {}", layer.layerName);
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
    RIV_RENDERER_LOGGER_INFO("Used vulkan extension for instance count: {}", std::size(m_RequiredInstanceExtensions));
    for (const auto& name : m_RequiredInstanceExtensions)
        RIV_RENDERER_LOGGER_INFO("    {}", name);

    instanceCreateInfo.enabledLayerCount = std::size(m_RequiredInstanceLayers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(m_RequiredInstanceLayers);

    vkRIVCreate(Instance, &instanceCreateInfo, VK_NULL_HANDLE, &m_Instance);

    /* 创建 Surface 接口对象 */
    if (glfwCreateWindowSurface(m_Instance, pRIVwindow->GetWindowHandle(),VK_NULL_HANDLE,
                                &m_Surface) != VK_SUCCESS) {
        RIV_RENDERER_LOGGER_INFO("Create glfw surface failed!");
    }

    InitRIVrenderer();
}

void RivuletRenderer::InitRIVrenderer() {
    m_RIVdevice = std::make_unique<RIVdevice>(m_Instance, m_Surface, m_RIVwindow);
    m_RIVswapchain = m_RIVdevice->CreateRIVswapchain();
}

RivuletRenderer::~RivuletRenderer() {
    m_RIVdevice->DestroyRIVswapchain(m_RIVswapchain);
    RIVdevice *pRIVdevice = m_RIVdevice.release();
    delete pRIVdevice;
    vkDestroySurfaceKHR(m_Instance, m_Surface, VK_NULL_HANDLE);
    vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
}