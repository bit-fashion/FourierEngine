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
struct VRHIGPU {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
};

/** 队列族 */
struct VRHIQueueFamilyIndices {
    uint32_t graphicsQueueFamily = 0;
    uint32_t presentQueueFamily = 0;
};

/** SwapChain supports details struct. */
struct VRHISwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/** 查询所有物理设备 */
static void VRHIGETGPU(VkInstance instance, std::vector<VRHIGPU> *pRIVGPU) {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, VK_NULL_HANDLE);
    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance, &count, std::data(devices));

    for (const auto &device: devices) {
        VRHIGPU gpu;
        gpu.device = device;
        /* 查询物理设备信息 */
        vkGetPhysicalDeviceProperties(device, &gpu.properties);
        /* 查询物理设备支持的功能列表 */
        vkGetPhysicalDeviceFeatures(device, &gpu.features);
        pRIVGPU->push_back(gpu);
    }
}

struct VRHIbuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};

class VRHIswapchain;
class VRHIdevice;
class VRHIpipeline;

/**
 * 渲染管线
 */
class VRHIpipeline {
public:
    explicit VRHIpipeline(VRHIdevice *device, VRHIswapchain *swapchain,
                          const char *vertex_shader_path, const char *fragment_shader_path);
    ~VRHIpipeline();

public:
    VkPipeline GetPipeline() { return mPipeline; }

private:
    VRHIdevice *mVRHIdevice;
    VRHIswapchain *mSwapchain;
    VkPipeline mPipeline;
    VkPipelineLayout mPipelineLayout;
};

/**
 * 交换链
 */
class VRHIswapchain {
public:
    VRHIswapchain(VRHIdevice *device, VRRTwindow *pVRRTwindow, VkSurfaceKHR surface);
    ~VRHIswapchain();

    VkResult AcquireNextImage(VkSemaphore semaphore, uint32_t *pIndex);

public:
    uint32_t GetWidth() { return mSwapchainExtent.width; }
    uint32_t GetHeight() { return mSwapchainExtent.height; }
    VkRenderPass GetRenderPass() { return mRenderPass; }
    uint32_t GetImageCount() { return mSwapchainImageCount; }
    VkFramebuffer GetFramebuffer(uint32_t index) { return mSwapchainFramebuffers[index]; }
    VkExtent2D GetExtent2D() { return mSwapchainExtent; }
    VkSwapchainKHR GetSwapchainKHRHandle() { return mSwapchain; }

private:
    void CreateSwapchain();
    void CleanupSwapchain();

private:
    VRHIdevice *mVRHIdevice;
    VRRTwindow *mVRRTwindow;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
    VkRenderPass mRenderPass = VK_NULL_HANDLE;
    VkSurfaceFormatKHR mSurfaceFormatKHR;
    VkFormat mSwapchainFormat;
    VkExtent2D mSwapchainExtent;
    VkPresentModeKHR mSwapchainPresentModeKHR;
    uint32_t mSwapchainImageCount;
    std::vector<VkImage > mSwapchainImages;
    std::vector<VkImageView> mSwapchainImageViews;
    std::vector<VkFramebuffer> mSwapchainFramebuffers;
    VRHISwapChainSupportDetails mSwapChainDetails;
};

/**
 * 渲染设备（GPU）
 */
class VRHIdevice {
public:
    /* init and destroy function */
    explicit VRHIdevice(VkInstance instance, VkSurfaceKHR surface, VRRTwindow *pVRRTwidnow);
    ~VRHIdevice();

    void CreateSwapchain(VRHIswapchain **pSwapchain);
    void DestroySwapchain(VRHIswapchain *swapchain);
    void AllocateDescriptorSet(uint32_t count, VkDescriptorSet *pDescriptorSet);
    void FreeDescriptorSet(uint32_t count, VkDescriptorSet *pDescriptorSet);
    void AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void CreateSemaphore(VkSemaphore *pSemaphore);
    void DestroySemaphore(VkSemaphore semaphore);
    void AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VRHIbuffer *buffer);
    void FreeBuffer(VRHIbuffer buffer);
    void MapMemory(VRHIbuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);
    void UnmapMemory(VRHIbuffer buffer);
    void WaitIdle();

public:
    VkPhysicalDevice GetPhysicalDeviceHandle() { return mVRHIGPU.device; }
    VkDevice GetDeviceHandle() { return mDevice; }
    VkDescriptorSetLayout GetDescriptorSetLayout() { return mDescriptorSetLayout; }
    VkQueue GetGraphicsQueue() { return mGraphicsQueue; }
    VkQueue GetPresentQueue() { return mPresentQueue; }

private:
    void InitAllocateDescriptorSet();
    void InitCommandPool();
    VRHIQueueFamilyIndices FindQueueFamilyIndices();

private:
    VkInstance mInstance = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;
    VkSurfaceKHR mSurfaceKHR = VK_NULL_HANDLE;
    VRHIGPU mVRHIGPU = {};
    VRRTwindow *mVRRTwindow;
    VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
    VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
    VkCommandPool mCommandPool = VK_NULL_HANDLE;
    VkMemoryRequirements mMemoryRequirements;
    /* 队列族 */
    VRHIQueueFamilyIndices mVRHIQueueFamilyIndices;
    VkQueue mGraphicsQueue = VK_NULL_HANDLE;
    VkQueue mPresentQueue = VK_NULL_HANDLE;
    /* 设备支持的扩展列表 */
    std::unordered_map<std::string, VkExtensionProperties> mDeviceSupportedExtensions;
};

/**
 * 渲染硬件接口
 */
class VRRTrenderer {
public:
    explicit VRRTrenderer(VRRTwindow *pVRRTwidnow);
    ~VRRTrenderer();

    void BeginRender();
    void Draw();
    void EndRender();

private:
    void Init_Vulkan_Impl();
    void CleanupSwapchain();
    void CreateSwapchain();
    void RecreateSwapchain();
    void BeginRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index);
    void EndRecordCommandBuffer();
    void BeginRenderPass(VkRenderPass renderPass);
    void EndRenderPass();
    void SubmitCommandBuffer();

private:
    VkInstance mInstance = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    VRRTwindow *mVRRTwindow;
    std::unique_ptr<VRHIdevice> mVRHIdevice = NULL;
    std::unique_ptr<VRHIpipeline> mVRHIpipeline = NULL;
    VRHIswapchain *mSwapchain = NULL;
    VkSemaphore mImageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore mRenderFinishedSemaphore = VK_NULL_HANDLE;
    /* Vectors. */
    std::vector<const char *> mRequiredInstanceExtensions;
    std::vector<const char *> mRequiredInstanceLayers;
    std::vector<VkCommandBuffer> mCommandBuffers;
    /* Map */
    std::unordered_map<std::string, VkExtensionProperties> mVkInstanceExtensionPropertiesSupports;
    std::unordered_map<std::string, VkLayerProperties> mVkInstanceLayerPropertiesSupports;
    /* Context */
    VkCommandBuffer mCurrentContextCommandBuffer = VK_NULL_HANDLE;
    VkRenderPass mCurrentContextRenderPass = VK_NULL_HANDLE;
    uint32_t mCurrentContextImageIndex = 0;
};