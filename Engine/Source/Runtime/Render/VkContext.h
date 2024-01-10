/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|*    Copyright (C) 2023 bit-fashion                                                *|
|*                                                                                  *|
|*    This program is free software: you can redistribute it and/or modify          *|
|*    it under the terms of the GNU General Public License as published by          *|
|*    the Free Software Foundation, either version 3 of the License, or             *|
|*    (at your option) any later version.                                           *|
|*                                                                                  *|
|*    This program is distributed in the hope that it will be useful,               *|
|*    but WITHOUT ANY WARRANTY; without even the implied warranty of                *|
|*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *|
|*    GNU General Public License for more details.                                  *|
|*                                                                                  *|
|*    You should have received a copy of the GNU General Public License             *|
|*    along with this program.  If not, see <https://www.gnu.org/licenses/>.        *|
|*                                                                                  *|
|*    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.    *|
|*    This is free software, and you are welcome to redistribute it                 *|
|*    under certain conditions; type `show c' for details.                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|* File:           VkContext.h                                                      *|
|* Create Time:    2023/12/30 20:21                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <vulkan/vulkan.h>
#include <Aurora/Logger.h>
#include "Window/Window.h"
#include "Renderable.h"

#define VTX_DEFINE_HANDLE(object) typedef struct object##_T *object;
VTX_DEFINE_HANDLE(VtxBuffer)
VTX_DEFINE_HANDLE(VtxPipeline)
VTX_DEFINE_HANDLE(VtxTexture2D)
VTX_DEFINE_HANDLE(VtxViewport)

/**
 * vulkan 上下文
 */
class VkContext {
public:
    VkContext(Window *p_win);
    ~VkContext();

    /**
     * 创建以及分配 Vulkan 对象
     */
    void AllocateDescriptorSet(VkDescriptorSet *pDescriptorSet);
    void FreeDescriptorSet(VkDescriptorSet descriptorSet);
    void CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VtxTexture2D *pTexture2D);
    void DestroyTexture2D(VtxTexture2D texture2D);
    void AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VtxBuffer *pBuffer);
    void FreeBuffer(VtxBuffer buffer);
    void CopyBuffer(VtxBuffer dst, VtxBuffer src, VkDeviceSize size);
    void WriteMemory(void *buf, VkDeviceSize offset, VkDeviceSize size, VtxBuffer buffer);
    void MapMemory(VtxBuffer buffer, VkDeviceSize offset, VkDeviceSize size, void **pBuf);
    void UnmapMemory(VtxBuffer buffer);
    void AllocateCommandBuffer(VkCommandBuffer *pCommandBuffer);
    void FreeCommandBuffer(VkCommandBuffer commandBuffer);

    void BeginOneTimeCommandBuffer(VkCommandBuffer *pCommandBuffer);
    void EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer);
    void BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags);
    void EndCommandBuffer(VkCommandBuffer commandBuffer);

    /**
     * Cmd
     */
     void CmdBindPipeline(VkCommandBuffer commandBuffer, VtxPipeline pipeline);

private:
    void InitVulkanContextInstance();
    void InitVulkanContextSurface();
    void InitVulkanContextDevice();
    void InitVulkanContextCommandPool();
    void InitVulkanContextDescriptorPool();

private:
    Window *m_Window;
    VkPhysicalDevice m_PhysicalDevice;
    VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
    uint32_t m_GraphicsQueueFamilyIndex;
    VkQueue m_GraphicsQueue;
    uint32_t m_PresentQueueFamilyIndex;
    VkQueue m_PresentQueue;

    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    VkInstance m_Instance;
    VkCommandPool m_CommandPool;
    VkDescriptorPool m_DescriptorPool;
};