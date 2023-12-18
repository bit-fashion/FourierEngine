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

/*
  ===================================
    @author bit-fashion
  ===================================
*/
#ifndef _SPORTS_VULKAN_CONTEXT_H_
#define _SPORTS_VULKAN_CONTEXT_H_

#include <vulkan/vulkan.h>
#include <Typedef.h>
#include <Engine.h>
#include <stdexcept>
#include <glm/glm.hpp>

class Window;

struct VkSwapchainContextKHR {
    VkSwapchainKHR swapchain;
    Vector<VkImage> images;
    Vector<VkImageView> imageViews;
    Vector<VkFramebuffer> framebuffers;
    VkRenderPass renderpass;
    uint32_t minImageCount;
    VkSurfaceFormatKHR surfaceFormat;
    VkSurfaceKHR surface;
    const Window *window;
    uint32_t width;
    uint32_t height;
    VkSurfaceCapabilitiesKHR capabilities;
    VkPresentModeKHR presentMode;
};

struct VkDeviceBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;
};

struct VkFrameContext {
    uint32_t index;
    VkCommandBuffer commandBuffer;
    VkFramebuffer framebuffer;
};

struct VkDriverGraphicsPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
};

class VulkanContext {
public:
    VulkanContext(Window *window);
   ~VulkanContext();

    void DeviceWaitIdle();
    void CopyBuffer(VkDeviceBuffer dest, VkDeviceBuffer src, VkDeviceSize size);
    void MapMemory(VkDeviceBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);
    void UnmapMemory(VkDeviceBuffer buffer);
    void BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags);
    void EndCommandBuffer(VkCommandBuffer commandBuffer);
    void SyncSubmitQueueWithSubmitInfo(uint32_t commandBufferCount, VkCommandBuffer *pCommandBuffers,
                                       uint32_t waitSemaphoreCount, VkSemaphore *pWaitSemaphores,
                                       uint32_t signalSemaphoreCount, VkSemaphore *pSignalSemaphores,
                                       VkPipelineStageFlags *pWaitDstStageMask);
    void BeginOnceTimeCommandBufferSubmit(VkCommandBuffer *pCommandBuffer);
    void EndOnceTimeCommandBufferSubmit();
    void BeginRenderPass(VkRenderPass renderPass);
    void EndRenderPass();

    void CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags,
                                   VkDescriptorSetLayout *pDescriptorSetLayout);
    void CreateGraphicsPipeline(const String &shaderfolder, const String &shadername, VkDescriptorSetLayout descriptorSetLayout,
                                VkDriverGraphicsPipeline *pDriverGraphicsPipeline);
    void AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceBuffer *buffer);
    void RecreateSwapchainContext(VkSwapchainContextKHR *pSwapchainContext, uint32_t width, uint32_t height);
    void CreateSwapchainContext(VkSwapchainContextKHR *pSwapchainContext);

    void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    void DestroyGraphicsPipeline(VkDriverGraphicsPipeline driverGraphicsPipeline);
    void FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void FreeBuffer(VkDeviceBuffer buffer);
    void DestroySwapchainContext(VkSwapchainContextKHR *pSwapchainContext);

private:
    void InitVulkanDriverContext(); /* Init Vulkan Context */

private:
    void _CreateSwapcahinAboutComponents(VkSwapchainContextKHR *pSwapchainContext);
    void _CreateRenderpass(VkSwapchainContextKHR *pSwapchainContext);
    void _ConfigurationSwapchainContext(VkSwapchainContextKHR *pSwapchainContext);
    void _ConfigurationWindowResizeableEventCallback();

private:
    VkInstance m_Instance;
    VkSurfaceKHR m_SurfaceKHR;
    VkDevice m_Device;
    VkCommandPool m_CommandPool;
    VkSwapchainContextKHR m_SwapchainContext;

    Window *m_Window;
    VkPhysicalDevice m_PhysicalDevice;
    VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures m_PhysicalDeviceFeature;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    VkCommandBuffer m_SingleTimeCommandBuffer;
    VkFrameContext m_FrameContext;
};

#endif /* _SPORTS_VULKAN_CONTEXT_H_ */
