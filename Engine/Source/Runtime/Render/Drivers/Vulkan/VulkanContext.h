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
#include "Render/GameModel.h"

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
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
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
    VkImage image;
    VkImageView imageView;
};

struct VkRenderPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

struct VkTexture2D {
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VkFormat format;
    VkImageLayout layout;
    VkDeviceMemory memory;
};

struct VkRenderContext {
    VkInstance Instance;
    VkSurfaceKHR Surface;
    VkPhysicalDevice PhysicalDevice;
    VkDevice Device;
    VkQueue GraphicsQueue;
    uint32_t GraphicsQueueFamily;
    VkSwapchainKHR Swapchain;
    VkCommandPool CommandPool;
    VkDescriptorPool DescriptorPool;
    uint32_t MinImageCount;
    VkRenderPass RenderPass;
    struct VkFrameContext *FrameContext;
};
/**
 * Vulkan context class.
 */
class VulkanContext {
public:
    VulkanContext(Window *window);
   ~VulkanContext();

    void GetRenderContext(VkRenderContext **pRenderContext) { *pRenderContext = &m_RenderContext; }
    void GetFrameContext(VkFrameContext **pContext) { *pContext = &m_FrameContext; }
    void DeviceWaitIdle();
    VkDevice GetDevice() const { return m_Device; }

    //
    // About vulkan device buffer.
    //
    void CopyBuffer(VkDeviceBuffer dest, VkDeviceBuffer src, VkDeviceSize size);
    void MapMemory(VkDeviceBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData);
    void UnmapMemory(VkDeviceBuffer buffer);

    //
    // About vulkan command buffer.
    //
    void BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags);
    void EndCommandBuffer(VkCommandBuffer commandBuffer);
    void SyncSubmitQueueWithSubmitInfo(uint32_t commandBufferCount, VkCommandBuffer *pCommandBuffers,
                                       uint32_t waitSemaphoreCount, VkSemaphore *pWaitSemaphores,
                                       uint32_t signalSemaphoreCount, VkSemaphore *pSignalSemaphores,
                                       VkPipelineStageFlags *pWaitDstStageMask);
    void BeginOnceTimeCommandBufferSubmit(VkCommandBuffer *pCommandBuffer);
    void EndOnceTimeCommandBufferSubmit();
    void BeginRecordCommandBuffer();
    void EndRecordCommandBuffer();
    void BeginRenderPass(VkRenderPass renderPass);
    void EndRenderPass();
    void QueueWaitIdle(VkQueue queue);
    void BeginRender(VkFrameContext **ppFrameContext = null);
    void EndRender();

    //
    // Bind
    //
    void BindRenderPipeline(VkRenderPipeline &pipeline);
    void BindDescriptorSets(VkRenderPipeline &pipeline, uint32_t count, VkDescriptorSet *pDescriptorSets);
    void WriteDescriptorSet(VkDeviceBuffer &buffer, VkTexture2D &texture, VkDescriptorSet descriptorSet);
    void DrawIndexed(uint32_t indexCount);

    //
    // Allocate and create buffer etc...
    //
    void AllocateVertexBuffer(VkDeviceSize size, const Vertex *pVertices, VkDeviceBuffer *pVertexBuffer);
    void AllocateIndexBuffer(VkDeviceSize size, const uint32_t *pIndices, VkDeviceBuffer *pIndexBuffer);
    void TransitionTextureLayout(VkTexture2D *texture, VkImageLayout newLayout);
    void CopyTextureBuffer(VkDeviceBuffer &buffer, VkTexture2D &texture, uint32_t width, uint32_t height);
    void CreateTexture2D(const String &path, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkTexture2D *pTexture2D);
    void CreateTextureSampler2D(VkSampler *pSampler);
    void CreateSemaphore(VkSemaphore *semaphore);
    void CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags,
                                   VkDescriptorSetLayout *pDescriptorSetLayout);
    void AllocateDescriptorSet(Vector<VkDescriptorSetLayout> &layouts, VkDescriptorSet *pDescriptorSet);
    void CreateRenderPipeline(const String &shaderfolder, const String &shadername, VkDescriptorSetLayout descriptorSetLayout,
                                VkRenderPipeline *pDriverGraphicsPipeline);
    void AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceBuffer *buffer);
    void RecreateSwapchainContext(VkSwapchainContextKHR *pSwapchainContext, uint32_t width, uint32_t height);
    void CreateSwapchainContext(VkSwapchainContextKHR *pSwapchainContext);

    //
    // Destroy components.
    //
    void DestroyTexture2D(VkTexture2D &texture);
    void FreeDescriptorSets(uint32_t count, VkDescriptorSet *pDescriptorSet);
    void DestroyDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout);
    void DestroyRenderPipeline(VkRenderPipeline &pipeline);
    void FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void FreeBuffer(VkDeviceBuffer &buffer);
    void DestroySwapchainContext(VkSwapchainContextKHR *pSwapchainContext);

private:
    void InitVulkanDriverContext(); /* Init VulkanContext main */
    void _InitVulkanContextInstance();
    void _InitVulkanContextSurface();
    void _InitVulkanContextDevice();
    void _InitVulkanContextQueue();
    void _InitVulkanContextCommandPool();
    void _InitVulkanContextSwapchain();
    void _InitVulkanContextCommandBuffers();
    void _InitVulkanContextDescriptorPool();

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
    Vector<VkCommandBuffer> m_CommandBuffers;
    VkSwapchainContextKHR m_SwapchainContext;

    Window *m_Window;
    VkPhysicalDevice m_PhysicalDevice;
    VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures m_PhysicalDeviceFeature;
    uint32_t m_GraphicsQueueFamily;
    uint32_t m_PresentQueueFamily;
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    VkCommandBuffer m_SingleTimeCommandBuffer;
    VkFrameContext m_FrameContext;
    VkDescriptorPool m_DescriptorPool;
    VkRenderContext m_RenderContext;
};

#endif /* _SPORTS_VULKAN_CONTEXT_H_ */
