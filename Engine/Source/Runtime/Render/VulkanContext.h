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
|* File:           VulkanContext.h                                                  *|
|* Create Time:    2023/12/30 20:21                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <vulkan/vulkan.h>
#include <Aurora/Logger.h>
#include "Window/Window.h"

typedef struct VctxBuffer_T {
    VkBuffer hbuffer;
    VkDeviceMemory hmemory;
    VkDeviceSize size;
} *VctxBuffer;

/**
 * vulkan 上下文
 */
class VulkanContext {
public:
    VulkanContext(Window *p_win);
   ~VulkanContext();

   /**
    * device
    */
   void AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VctxBuffer *pBuffer);
   void FreeBuffer(VctxBuffer buffer);

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