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
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <Fourier.h>

#include "RIVswapchain.h"

#define vkRIVCreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        throw std::runtime_error("FourierEngine Error: create vulkan object for `{}` handle failed!")
#define vkRIVAllocate(name, ...) \
    if (vkAllocate##name(__VA_ARGS__) != VK_SUCCESS) \
        throw std::runtime_error("FourierEngine Error: allocate vulkan object for `{}` handle failed!")
#define RIV_RENDERER_LOGGER_INFO(fmt, ...) \
  rivulet_logger_info("[RIVULET ENGINE] [INIT_VULKAN_API] IF/ - {}", std::format(fmt, ##__VA_ARGS__))

class RIVwindow;

/** GPU 设备信息 */
struct RIVGPU {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
};

/** 队列族 */
struct RIVQueueFamilyIndices {
    uint32_t graphicsQueueFamily;
    uint32_t presentQueueFamily;
};

/** 查询所有物理设备 */
static void RIVGETGPU(VkInstance instance, std::vector<RIVGPU> *pRIVGPU) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, VK_NULL_HANDLE);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, std::data(devices));

    for (const auto &device: devices) {
        RIVGPU gpu;
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
class RIVdevice {
public:
    /* init and destroy function */
    explicit RIVdevice(VkInstance instance, VkSurfaceKHR surface, RIVwindow *pRIVwindow);
    ~RIVdevice();

public:
    /* public function */
    RIVswapchain *CreateRIVswapchain();
    void DestroyRIVswapchain(RIVswapchain *pRIVswapchain);

public:
    /* Handle */
    VkSurfaceKHR RIVHSurface() { return m_Surface; }
    VkDevice RIVHDevice() { return m_Device; }
    VkPhysicalDevice RIVHPhysicalDevice() const { return m_RIVGPU.device; }
    VkQueue RIVHGraphicsQueue() { return m_GraphicsQueue; }
    VkQueue RIVHPresentQueue() { return m_PresentQueue; }

    /* GET */
    RIVwindow *GetRIVwindow() { return m_RIVwindow; }

private:
    RIVQueueFamilyIndices FindQueueFamilyIndices();

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    RIVGPU m_RIVGPU;
    RIVwindow *m_RIVwindow;
    /* 队列族 */
    RIVQueueFamilyIndices m_RIVQueueFamilyIndices;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;
    /* 设备支持的扩展列表 */
    std::unordered_map<std::string, VkExtensionProperties> m_DeviceSupportedExtensions;
};