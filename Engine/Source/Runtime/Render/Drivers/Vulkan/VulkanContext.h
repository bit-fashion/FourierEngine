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
};

struct VkRenderPipeline {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
};

struct VkTexture2D {
    VkImage image;
    VkImageView imageView;
    VkSampler sampler;
    VkFormat format;
    VkImageLayout layout;
    VkDeviceMemory memory;
};

class VulkanContext {
public:
    VulkanContext(Window *window);
   ~VulkanContext();

   void GetFrameContext(VkFrameContext *pContext) { *pContext = m_FrameContext; }
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
    void BeginRecordCommandBuffer();
    void EndRecordCommandBuffer();
    void BeginRenderPass(VkRenderPass renderPass);
    void EndRenderPass();
    void QueueWaitIdle(VkQueue queue);
    void BeginRender();
    void EndRender();
    VkDevice GetDevice() const { return m_Device; }
    void BindRenderPipeline(VkRenderPipeline *pPipeline);
    void BindDescriptorSet(VkRenderPipeline *pPipeline, uint32_t count, VkDescriptorSet *pDescriptorSets);

    void AllocateVertexBuffer(VkDeviceSize size, const Vertex *pVertices, VkDeviceBuffer *pVertexBuffer);
    void AllocateIndexBuffer(VkDeviceSize size, const uint32_t *pIndices, VkDeviceBuffer *pIndexBuffer);
    void TransitionTextureLayout(VkTexture2D *texture, VkImageLayout newLayout);
    void CopyTextureBuffer(VkDeviceBuffer &buffer, VkTexture2D &texture, uint32_t width, uint32_t height);
    void CreateTexture2D(const String &path, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkTexture2D *pTexture2D);
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

    void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    void DestroyRenderPipeline(VkRenderPipeline driverGraphicsPipeline);
    void FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer);
    void FreeBuffer(VkDeviceBuffer buffer);
    void DestroySwapchainContext(VkSwapchainContextKHR *pSwapchainContext);

private:
    void InitVulkanDriverContext(); /* Init Vulkan Context */
    void InitDescriptorPool();

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
    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;
    VkCommandBuffer m_SingleTimeCommandBuffer;
    VkFrameContext m_FrameContext;
    VkDescriptorPool m_DescriptorPool;
};

#endif /* _SPORTS_VULKAN_CONTEXT_H_ */
