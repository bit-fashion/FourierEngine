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
|* File:           VkDefineHandle.h                                                 *|
|* Create Time:    2024/01/03 01:35                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

struct VtxBuffer_T {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize offset;
    VkDeviceSize size;
};

struct VtxTexture2D_T {
    VkImage image;
    VkImageView view;
    VkDeviceMemory memory;
    VkDeviceSize offset;
    VkDeviceSize size;
    VkSampler sampler;
    VkImageLayout layout;
    VkFormat format;
};

struct VtxPipeline_T {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
};

/**
 * 视口对象，这个对象是在 Vulkan 概念之外新封装的对象。这个对象可以简单的理解
 * 为帧缓冲区（Framebuffer）因为 Viewport 是图像最终展示图像的对象。每当一帧
 * 图像被渲染完成后会将图像写入到 Viewport 对象。
 *
 * 然后其他操作可以从 Viewport 中拿到渲染好的图像做后处理。同时也可以理解为
 * 增强版的交换链（Swapchain）
 */
struct VtxViewport_T {
    uint32_t width;
    uint32_t height;
};

/* malloc */
#define VtxMemoryMalloc(object) (object##_T *) malloc(sizeof(object##_T))
#define VtxMemoryFree(ptr) free(ptr)