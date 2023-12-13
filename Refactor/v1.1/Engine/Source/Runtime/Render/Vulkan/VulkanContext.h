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
#pragma once

#include <vulkan/vulkan.h>
#include "SportsCore.h"

#define VK_LAYER_KHRONOS_validation "VK_LAYER_KHRONOS_validation"
#define VRAC_DEFINE_HANDLE(name) typedef struct _##name##_t *name;

#define vkVRACCreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        SPORTS_THROW_ERROR("[NATURE ENGINE] [INIT_VULKAN_API] ERR/ - Create [{}] handle failed!", #name)

VRAC_DEFINE_HANDLE(VRACWindow)
VRAC_DEFINE_HANDLE(VRACSwapchainKHR)
VRAC_DEFINE_HANDLE(VRACTexture2D)
VRAC_DEFINE_HANDLE(VRACBuffer)
VRAC_DEFINE_HANDLE(VRACDevice)

class GLWIN;
class VulkanContext;

class VulkanContext {
private:
    struct QueueFamilyIndices {
        uint32_t GraphicsQueueFamily = 0;
        uint32_t PresentQueueFamily = 0;
    };

    static VkBool32 CompleteQueueFamilyIndices(VulkanContext::QueueFamilyIndices &queueFamilyIndices) {
        return queueFamilyIndices.GraphicsQueueFamily > 0 && queueFamilyIndices.PresentQueueFamily > 0;
    }

private:
    static void _GetVulkanContextInstanceRequiredExtensions(Vector<const char *> &extensions);
    static void _GetVulkanContextInstanceRequiredLayers(Vector<const char *> &layers);
    static void _GetVulkanContextDeviceRequiredExtensions(Vector<const char *> &extensions);
    static void _GetVulkanContextDeviceRequiredLayers(Vector<const char *> &layers);
    static void _GetVulkanContextMostPreferredPhysicalDevice(VRACDevice device);
    static void _FindVulkanContextDeviceQueueFamilyIndices(const VRACDevice device, const VRACWindow window, VulkanContext::QueueFamilyIndices* pQueueFamilyIndices);

public:
    explicit VulkanContext(const GLWIN *pGLWIN);
    ~VulkanContext();

    void CreateSwapchainKHR(VRACSwapchainKHR *pVRACSwapchainKHR);
    void DestroySwpachainKHR(VRACSwapchainKHR swapchainKHR);

    void AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VRACBuffer *pBuffer);
    void FreeBuffer(VRACBuffer buffer);
    void CopyBuffer(VRACBuffer dest, VRACBuffer src, VkDeviceSize size);
    void CreateTexture2D(VRACTexture2D *pTexture2D);
    void DestroyTexture2D(VRACTexture2D texture2D);

private:
    void Init_VulkanR_Context(void);

private:
    const GLWIN *m_GLWIN;
    VRACDevice m_VRACDevice;
    VRACWindow m_VRACWindow;
};