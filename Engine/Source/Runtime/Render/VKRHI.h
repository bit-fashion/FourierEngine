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
#include <VRRT.h>
#include <unordered_map>

class VRRTwindow;

#define VK_LAYER_KHRONOS_validation "VK_LAYER_KHRONOS_validation"

#define vkVRRTCreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        VRRT_THROW_ERROR("[VERIRRT ENGINE] [INIT_VULKAN_API] ERR/ - Create [{}] handle failed!", #name)
#define vkVRRTAllocate(name, ...) \
    if (vkAllocate##name(__VA_ARGS__) != VK_SUCCESS) \
        VRRT_THROW_ERROR("[VERIRRT ENGINE] [INIT_VULKAN_API] ERR/ - Create [{}] handle failed!", #name)

/** GPU 设备信息 */
struct VKRHIGPU {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
};

/** 队列族 */
struct VKRHIQueueFamilyIndices {
    uint32_t graphicsQueueFamily = 0;
    uint32_t presentQueueFamily = 0;
};

/** SwapChain supports details struct. */
struct VKRHISwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/** 查询所有物理设备 */
static void VKRHIGETGPU(VkInstance instance, std::vector<VKRHIGPU> *pRIVGPU) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, VK_NULL_HANDLE);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, std::data(devices));

    for (const auto &device: devices) {
        VKRHIGPU gpu;
        gpu.device = device;
        /* 查询物理设备信息 */
        vkGetPhysicalDeviceProperties(device, &gpu.properties);
        /* 查询物理设备支持的功能列表 */
        vkGetPhysicalDeviceFeatures(device, &gpu.features);
        pRIVGPU->push_back(gpu);
    }
}

typedef class _VKRHISwapchain *VKRHISwapchain;
typedef class _VKRHIDevice *VKRHIDevice;
typedef class _VKRHIPipeline *VKRHIPipeline;

/**
 * 渲染管线
 */
typedef class _VKRHIPipeline {
public:
    explicit _VKRHIPipeline(VKRHIDevice device, VKRHISwapchain swapchain,
                          const char *vertex_shader_path, const char *fragment_shader_path);
    ~_VKRHIPipeline();

public:
    VkPipeline GetPipelineHandle() { return m_Pipeline; }

private:
    VKRHIDevice m_VKRHIDevice;
    VKRHISwapchain m_Swapchain;
    VkPipeline m_Pipeline;
    VkPipelineLayout m_PipelineLayout;
} *VKRHIPipeline;

/**
 * 交换链
 */
typedef class _VKRHISwapchain {
public:
    _VKRHISwapchain(VKRHIDevice device, VRRTwindow *pVRRTwindow, VkSurfaceKHR surface);
    ~_VKRHISwapchain();

    void AcquireNextImage(VkSemaphore semaphore, uint32_t *pIndex);

public:
    uint32_t GetWidth() { return m_SwapchainExtent.width; }
    uint32_t GetHeight() { return m_SwapchainExtent.height; }
    VkRenderPass GetRenderPassHandle() { return m_RenderPass; }
    uint32_t GetImageCount() { return m_SwapchainImageCount; }
    VkFramebuffer GetFramebuffer(uint32_t index) { return m_SwapchainFramebuffers[index]; }
    VkExtent2D GetExtent2D() { return m_SwapchainExtent; }
    VkSwapchainKHR GetSwapchainKHRHandle() { return m_Swapchain; }

private:
    VKRHIDevice m_VKRHIDevice;
    VRRTwindow *m_VRRTwindow;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
    VkRenderPass m_RenderPass = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_SurfaceFormatKHR;
    VkFormat m_SwapchainFormat;
    VkExtent2D m_SwapchainExtent;
    VkPresentModeKHR m_SwapchainPresentModeKHR;
    uint32_t m_SwapchainImageCount;
    std::vector<VkImage > m_SwapchainImages;
    std::vector<VkImageView> m_SwapchainImageViews;
    std::vector<VkFramebuffer> m_SwapchainFramebuffers;
} *VKRHISwapchain;

/**
 * 渲染设备（GPU）
 */
typedef class _VKRHIDevice {
public:
    /* init and destroy function */
    explicit _VKRHIDevice(VkInstance instance, VkSurfaceKHR surface, VRRTwindow *pVRRTwidnow);
    ~_VKRHIDevice();

    void CreateSwapchain(VKRHISwapchain *pSwapchain);
    void DestroySwapchain(VKRHISwapchain swapchain);
    void AllocateDescriptorSet(uint32_t count, VkDescriptorSet *pDescriptorSet);
    void FreeDescriptorSet(uint32_t count, VkDescriptorSet *pDescriptorSet);
    void AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void CreateSemaphore(VkSemaphore *pSemaphore);
    void DestroySemaphore(VkSemaphore semaphore);

public:
    VkPhysicalDevice GetPhysicalDeviceHandle() { return m_VKRHIGPU.device; }
    VkDevice GetDeviceHandle() { return m_Device; }
    VkDescriptorSetLayout GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
    VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
    VkQueue GetPresentQueue() { return m_PresentQueue; }

private:
    void InitAllocateDescriptorSet();
    void InitCommandPool();
    VKRHIQueueFamilyIndices FindQueueFamilyIndices();

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkDevice m_Device = VK_NULL_HANDLE;
    VkSurfaceKHR m_SurfaceKHR = VK_NULL_HANDLE;
    VKRHIGPU m_VKRHIGPU = {};
    VRRTwindow *m_VRRTwindow;
    VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_DescriptorSetLayout = VK_NULL_HANDLE;
    VkCommandPool m_CommandPool = VK_NULL_HANDLE;
    /* 队列族 */
    VKRHIQueueFamilyIndices m_VKRHIQueueFamilyIndices;
    VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
    VkQueue m_PresentQueue = VK_NULL_HANDLE;
    /* 设备支持的扩展列表 */
    std::unordered_map<std::string, VkExtensionProperties> m_DeviceSupportedExtensions;
} *VKRHIDevice;

/**
 * 渲染硬件接口
 */
class VKRHI {
public:
    explicit VKRHI(VRRTwindow *pVRRTwidnow);
    ~VKRHI();

    void BeginRender();
    void Draw();
    void EndRender();

private:
    void Init_Vulkan_Impl();
    void RecordCommandBuffer(uint32_t index, VkCommandBuffer commandBuffer);

private:
    VkInstance m_Instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
    VRRTwindow *m_VRRTwindow;
    std::unique_ptr<_VKRHIDevice> m_VKRHIDevice = NULL;
    std::unique_ptr<_VKRHIPipeline> m_VKRHIPipeline = NULL;
    VKRHISwapchain m_Swapchain = NULL;
    VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;
    /* Vectors. */
    std::vector<const char *> m_RequiredInstanceExtensions;
    std::vector<const char *> m_RequiredInstanceLayers;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    /* Map */
    std::unordered_map<std::string, VkExtensionProperties> m_VkInstanceExtensionPropertiesSupports;
    std::unordered_map<std::string, VkLayerProperties> m_VkInstanceLayerPropertiesSupports;
};