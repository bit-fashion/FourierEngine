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
#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <NEDEF.h>
#include <unordered_map>

class VRRTwindow;

#define VK_LAYER_KHRONOS_validation "VK_LAYER_KHRONOS_validation"

#define vkVRHICreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        VRRT_THROW_ERROR("[NANORIV ENGINE] [INIT_VULKAN_API] ERR/ - Create [{}] handle failed!", #name)
#define VRHI_LOGGER_VULKAN_API_INFO(fmt, ...) \
  VRRT_LOGGER_INFO("[NANORIV ENGINE] [INIT_VULKAN_API] IF/ - {}", fmt, ##__VA_ARGS__)

/** GPU 设备信息 */
struct RHIGPU {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
};

/** 队列族 */
struct RHIQueueFamilyIndices {
    uint32_t graphicsQueueFamily = 0;
    uint32_t presentQueueFamily = 0;
};

/** 查询所有物理设备 */
static void RHIGETGPU(VkInstance instance, std::vector<RHIGPU> *pRIVGPU) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, VK_NULL_HANDLE);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, std::data(devices));

    for (const auto &device: devices) {
        RHIGPU gpu;
        gpu.device = device;
        /* 查询物理设备信息 */
        vkGetPhysicalDeviceProperties(device, &gpu.properties);
        /* 查询物理设备支持的功能列表 */
        vkGetPhysicalDeviceFeatures(device, &gpu.features);
        pRIVGPU->push_back(gpu);
    }
}

/**
 * 渲染设备（GPU）
 */
class VRHIDevice {
public:
    /* init and destroy function */
    explicit VRHIDevice(VkInstance instance, VkSurfaceKHR surface, VRRTwindow *pVRRTwidnow);
    ~VRHIDevice();

private:
    RHIQueueFamilyIndices FindQueueFamilyIndices();

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkSurfaceKHR m_SurfaceKHR = VK_NULL_HANDLE;
    RHIGPU m_RHIGPU = {};
    VRRTwindow *m_NRIVwindow;
    /* 队列族 */
    RHIQueueFamilyIndices m_RHIQueueFamilyIndices;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;
    /* 设备支持的扩展列表 */
    std::unordered_map<std::string, VkExtensionProperties> m_DeviceSupportedExtensions;
};

class VRHI {
public:
    explicit VRHI(VRRTwindow *pVRRTwidnow);
    ~VRHI();

private:
    void Init_Vulkan_Impl();

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VRRTwindow *m_NRIVwindow;
    std::unique_ptr<VRHIDevice> m_VRHIDevice = NULL;
    /* Vectors. */
    std::vector<const char *> m_RequiredInstanceExtensions;
    std::vector<const char *> m_RequiredInstanceLayers;
    /* Map */
    std::unordered_map<std::string, VkExtensionProperties> m_VkInstanceExtensionPropertiesSupports;
    std::unordered_map<std::string, VkLayerProperties> m_VkInstanceLayerPropertiesSupports;
};