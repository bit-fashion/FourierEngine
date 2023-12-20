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

struct VkWindowContext {
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    const Window *win;
    /* only create once */
    VkRenderPass renderpass;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
};

struct VkSwapchainContextKHR {
    VkSwapchainKHR swapchain;
    Vector<VkImage> images;
    Vector<VkImageView> imageViews;
    Vector<VkFramebuffer> framebuffers;
    const VkWindowContext *winctx;
    uint32_t minImageCount;
    uint32_t width;
    uint32_t height;
    VkPresentModeKHR presentMode;
    VkFormat format;
    VkColorSpaceKHR colorSpace;
    VkSurfaceCapabilitiesKHR capabilities;
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

struct VkOffScreenRenderContext {
    VkRenderPass renderpass;
    VkTexture2D texture;
    VkFramebuffer framebuffer;
    VkCommandBuffer commandBuffer;
    uint32_t width;
    uint32_t height;
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
    void BeginRecordCommandBuffer(VkCommandBuffer commandBuffer);
    void EndRecordCommandBuffer(VkCommandBuffer commandBuffer);
    void BeginRenderPass(VkCommandBuffer commandBuffer, uint32_t w, uint32_t h, VkRenderPass renderPass, VkFramebuffer framebuffer);
    void EndRenderPass(VkCommandBuffer commandBuffer);
    void QueueWaitIdle(VkQueue queue);

    void BeginRender(VkFrameContext **ppFrameContext = null);
    void EndRender();

    void BeginOffScreenRender(VkCommandBuffer *pCommandBuffer, uint32_t width, uint32_t height);
    void EndOffScreenRender();
    void RecreateOffScreenRenderContext(uint32_t width, uint32_t height);
    void AcquireOffScreenRenderTexture2D(VkTexture2D **ppTexture2D);

    //
    // Bind
    //
    void BindRenderPipeline(VkCommandBuffer commandBuffer, uint32_t width, uint32_t height, VkRenderPipeline &pipeline);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, VkRenderPipeline &pipeline, uint32_t count, VkDescriptorSet *pDescriptorSets);
    void WriteDescriptorSet(VkDeviceBuffer &buffer, VkTexture2D &texture, VkDescriptorSet descriptorSet);
    void DrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount);

    //
    // Allocate and create buffer etc...
    //
    void CreateOffScreenRenderContext(uint32_t width, uint32_t height, VkOffScreenRenderContext *pContext);
    void AllocateVertexBuffer(VkDeviceSize size, const Vertex *pVertices, VkDeviceBuffer *pVertexBuffer);
    void AllocateIndexBuffer(VkDeviceSize size, const uint32_t *pIndices, VkDeviceBuffer *pIndexBuffer);
    void TransitionTextureLayout(VkTexture2D *texture, VkImageLayout newLayout);
    void CopyTextureBuffer(VkDeviceBuffer &buffer, VkTexture2D &texture, uint32_t width, uint32_t height);
    void CreateTexture2D(const String &path, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkTexture2D *pTexture2D);
    void CreateTexture2D(int texWidth, int texHeight, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkTexture2D *pTexture2D);
    void CreateFramebuffer(VkRenderPass renderpass, VkImageView imageView, int width, int height, VkFramebuffer *pFramebuffer);
    void CreateTextureSampler2D(VkSampler *pSampler);
    void CreateSemaphore(VkSemaphore *semaphore);
    void CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags, VkDescriptorSetLayout *pDescriptorSetLayout);
    void AllocateDescriptorSet(Vector<VkDescriptorSetLayout> &layouts, VkDescriptorSet *pDescriptorSet);
    void CreateRenderPipeline(const String &shaderfolder, const String &shadername, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout, VkRenderPipeline *pDriverGraphicsPipeline);
    void AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceBuffer *buffer);
    void RecreateSwapchainContextKHR(VkSwapchainContextKHR *pSwapchainContext, uint32_t width, uint32_t height);
    void CreateSwapchainContextKHR(VkSwapchainContextKHR *pSwapchainContext);
    void CreateRenderpass(VkFormat format, VkImageLayout imageLayout, VkRenderPass *pRenderPass);

    VkRenderPass GetOffScreenRenderPass() { return m_OffScreenRenderContext.renderpass; }

    //
    // Destroy components.
    //
    void DestroyFramebuffer(VkFramebuffer &framebuffer);
    void DestroyOffScreenRenderContext(VkOffScreenRenderContext &context);
    void DestroyTexture2D(VkTexture2D &texture);
    void FreeDescriptorSets(uint32_t count, VkDescriptorSet *pDescriptorSet);
    void DestroyDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout);
    void DestroyRenderPipeline(VkRenderPipeline &pipeline);
    void FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void FreeBuffer(VkDeviceBuffer &buffer);
    void DestroySwapchainContextKHR(VkSwapchainContextKHR *pSwapchainContext);
    void DestroyRenderPass(VkRenderPass renderPass);

private:
    void InitVulkanDriverContext(); /* Init VulkanContext main */
    void _InitVulkanContextInstance();
    void _InitVulkanContextSurface();
    void _InitVulkanContextWindowContext();
    void _InitVulkanContextDevice();
    void _InitVulkanContextQueue();
    void _InitVulkanContextCommandPool();
    void _InitVulkanContextMainSwapchain();
    void _InitVulkanContextCommandBuffers();
    void _InitVulkanContextDescriptorPool();
    void _InitVulkanContextOffScreenRenderContext();

private:
    void _CreateSwapcahinAboutComponents(VkSwapchainContextKHR *pSwapchainContext);
    void _ConfigurationSwapchainContext(VkSwapchainContextKHR *pSwapchainContext);
    void _ConfigurationWindowResizeableEventCallback();

private:
    VkInstance m_Instance;
    VkSurfaceKHR m_SurfaceKHR;
    VkDevice m_Device;
    VkCommandPool m_CommandPool;
    Vector<VkCommandBuffer> m_CommandBuffers;
    VkSwapchainContextKHR m_MainSwapchainContext;
    VkOffScreenRenderContext m_OffScreenRenderContext;

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
    VkWindowContext m_WindowContext;
};

#endif /* _SPORTS_VULKAN_CONTEXT_H_ */
