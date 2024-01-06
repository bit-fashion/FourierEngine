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
|* File:           VkContext.cpp                                                    *|
|* Create Time:    2023/12/30 20:21                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "VkContext.h"
#include "VkUtils.h"

VkContext::VkContext(Window *p_win) : m_Window(p_win)
{
    Logger::Info("Begin initialize vulkan context");
    InitVulkanContextInstance();
    InitVulkanContextSurface();
    InitVulkanContextDevice();
    InitVulkanContextCommandPool();
    InitVulkanContextDescriptorPool();
    Logger::Info("End initialize vulkan context");
}

VkContext::~VkContext()
{
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VkUtils::Allocator);
    vkDestroyCommandPool(m_Device, m_CommandPool, VkUtils::Allocator);
    vkDestroyDevice(m_Device, VkUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VkUtils::Allocator);
    vkDestroyInstance(m_Instance, VkUtils::Allocator);
}

void VkContext::AllocateDescriptorSet(VkDescriptorSet *pDescriptorSet)
{
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts = 0;
}

void VkContext::FreeDescriptorSet(VkDescriptorSet descriptorSet)
{

}

void VkContext::CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                VkMemoryPropertyFlags properties, VtxTexture2D *pTexture2D)
{
    *pTexture2D = (VtxTexture2D_T *) vmalloc(sizeof(pTexture2D));

    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = format;
    imageCreateInfo.extent = { width, height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.flags = 0;

    (*pTexture2D)->format = imageCreateInfo.format;
    (*pTexture2D)->layout = imageCreateInfo.initialLayout;

    vkCheckCreate(Image, m_Device, &imageCreateInfo, VkUtils::Allocator, &(*pTexture2D)->image);

    /* 分配内存 */
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(m_Device, (*pTexture2D)->image, &requirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = requirements.size;
    memoryAllocateInfo.memoryTypeIndex = VkUtils::FindMemoryType(requirements.memoryTypeBits, m_PhysicalDevice, properties);
    vkCheckAllocate(Memory, m_Device, &memoryAllocateInfo, VkUtils::Allocator, &(*pTexture2D)->memory);

    vkBindImageMemory(m_Device, (*pTexture2D)->image, (*pTexture2D)->memory, 0);

    /* 视图 */
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = (*pTexture2D)->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(m_Device, &viewInfo, nullptr, &(*pTexture2D)->view);

    /* 采样器 */
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    vkCreateSampler(m_Device, &samplerInfo, VK_NULL_HANDLE, &(*pTexture2D)->sampler);
}

void VkContext::DestroyTexture2D(VtxTexture2D texture2D)
{
    vkDestroyImage(m_Device, texture2D->image, VkUtils::Allocator);
    vkDestroyImageView(m_Device, texture2D->view, VkUtils::Allocator);
    vkFreeMemory(m_Device, texture2D->memory, VkUtils::Allocator);
    vkDestroySampler(m_Device, texture2D->sampler, VkUtils::Allocator);
    vfree(texture2D);
}

void VkContext::AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VtxBuffer *pBuffer)
{
    *pBuffer = (VtxBuffer_T *) vmalloc(sizeof(VtxBuffer_T));

    /* 创建缓冲区 */
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCheckCreate(Buffer, m_Device, &bufferCreateInfo, VkUtils::Allocator, &(*pBuffer)->buffer);

    /* 分配内存 */
    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(m_Device, (*pBuffer)->buffer, &requirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = size;
    memoryAllocateInfo.memoryTypeIndex = VkUtils::FindMemoryType(requirements.memoryTypeBits, m_PhysicalDevice, properties);
    vkCheckAllocate(Memory, m_Device, &memoryAllocateInfo, VkUtils::Allocator, &(*pBuffer)->memory);
    vkBindBufferMemory(m_Device, (*pBuffer)->buffer, (*pBuffer)->memory, 0);
}

void VkContext::FreeBuffer(VtxBuffer buffer)
{
    vkDestroyBuffer(m_Device, buffer->buffer, VkUtils::Allocator);
    vkFreeMemory(m_Device, buffer->memory, VkUtils::Allocator);
    vfree(buffer);
}

void VkContext::CopyBuffer(VtxBuffer dst, VtxBuffer src, VkDeviceSize size)
{
    VkCommandBuffer oneTimeCommandBuffer;
    BeginOneTimeCommandBuffer(&oneTimeCommandBuffer);
    {
        VkBufferCopy region = {};
        region.dstOffset = 0;
        region.srcOffset = 0;
        region.size = size;
        vkCmdCopyBuffer(oneTimeCommandBuffer, src->buffer, dst->buffer, 1, &region);
    }
    EndOneTimeCommandBuffer(oneTimeCommandBuffer);
}

void VkContext::WriteMemory(VtxBuffer buffer, VkDeviceSize offset, VkDeviceSize size, void *buf)
{
    void *dst;
    MapMemory(buffer, offset, size, &dst);
    memcpy(dst, buf, size);
    UnmapMemory(buffer);
}

void VkContext::MapMemory(VtxBuffer buffer, VkDeviceSize offset, VkDeviceSize size, void **pBuf)
{
    vkMapMemory(m_Device, buffer->memory, offset, size, 0, pBuf);
}

void VkContext::UnmapMemory(VtxBuffer buffer)
{
    vkUnmapMemory(m_Device, buffer->memory);
}

void VkContext::AllocateCommandBuffer(VkCommandBuffer *pCommandBuffer)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_CommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;
    vkCheckAllocate(CommandBuffers, m_Device, &commandBufferAllocateInfo, pCommandBuffer);
}

void VkContext::FreeCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}

void VkContext::BeginOneTimeCommandBuffer(VkCommandBuffer *pCommandBuffer)
{
    AllocateCommandBuffer(pCommandBuffer);
    BeginCommandBuffer(*pCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

void VkContext::EndOneTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
    EndCommandBuffer(commandBuffer);
}

void VkContext::BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = flags;
    commandBufferBeginInfo.pInheritanceInfo = null;

    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
}

void VkContext::EndCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);
}

void VkContext::InitVulkanContextInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = AURORA_ENGINE_NAME;
    applicationInfo.engineVersion =  VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = AURORA_ENGINE_NAME;
    applicationInfo.apiVersion = VK_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    Vector<const char *> enableExtensionProperties;
    VkUtils::GetInstanceRequiredEnableExtensionProperties(enableExtensionProperties);
    Logger::Info("Vulkan context instance enabled extensions:");
    for (const auto &enableExtensionProperty : enableExtensionProperties)
        Logger::Info("  - {}", enableExtensionProperty);
    instanceCreateInfo.enabledExtensionCount = std::size(enableExtensionProperties);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(enableExtensionProperties);

    /* enable layer properties */
    Vector<const char *> enableLayerProperties;
    VkUtils::GetInstanceRequiredEnableLayerProperties(enableLayerProperties);
    Logger::Info("Vulkan context instance enabled layers:");
    for (const auto &enableLayerProperty : enableLayerProperties)
        Logger::Info("  - {}", enableLayerProperty);
    instanceCreateInfo.enabledLayerCount = std::size(enableLayerProperties);
    instanceCreateInfo.ppEnabledLayerNames = std::data(enableLayerProperties);

    vkCheckCreate(Instance, &instanceCreateInfo, VkUtils::Allocator, &m_Instance);
}

void VkContext::InitVulkanContextSurface()
{
    glfwCreateWindowSurface(m_Instance, m_Window->GetHWIN(), VkUtils::Allocator, &m_Surface);
}

void VkContext::InitVulkanContextDevice()
{
    VkUtils::GetBestPerformancePhysicalDevice(m_Instance, &m_PhysicalDevice);
    VkUtils::GetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties, &m_PhysicalDeviceFeatures);
    Logger::Debug("Vulkan context physical device using: {}", m_PhysicalDeviceProperties.deviceName);

    VkUtils::QueueFamilyIndices queueFamilyIndices;
    VkUtils::FindQueueFamilyIndices(m_PhysicalDevice, m_Surface, &queueFamilyIndices);
    m_GraphicsQueueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    m_PresentQueueFamilyIndex = queueFamilyIndices.presentQueueFamily;

    float priorities = 1.0f;
    std::array<VkDeviceQueueCreateInfo, 2> deviceQueueCreateInfos = {};
    deviceQueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[0].queueCount = 1;
    deviceQueueCreateInfos[0].queueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    deviceQueueCreateInfos[0].pQueuePriorities = &priorities;

    deviceQueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfos[1].queueCount = 1;
    deviceQueueCreateInfos[1].queueFamilyIndex = queueFamilyIndices.presentQueueFamily;
    deviceQueueCreateInfos[1].pQueuePriorities = &priorities;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);
    static VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;

    Vector<const char *> enableDeviceExtensionProperties;
    VkUtils::GetDeviceRequiredEnableExtensionProperties(m_PhysicalDevice, enableDeviceExtensionProperties);
    deviceCreateInfo.enabledExtensionCount = std::size(enableDeviceExtensionProperties);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(enableDeviceExtensionProperties);

    Vector<const char *> enableDeviceLayerProperties;
    VkUtils::GetDeviceRequiredEnableLayerProperties(m_PhysicalDevice, enableDeviceLayerProperties);
    deviceCreateInfo.enabledLayerCount = std::size(enableDeviceLayerProperties);
    deviceCreateInfo.ppEnabledLayerNames = std::data(enableDeviceLayerProperties);

    vkCheckCreate(Device, m_PhysicalDevice, &deviceCreateInfo, VkUtils::Allocator, &m_Device);

    /* 获取队列 */
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamilyIndex, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamilyIndex, 0, &m_PresentQueue);
}

void VkContext::InitVulkanContextCommandPool()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCheckCreate(CommandPool, m_Device, &commandPoolCreateInfo, VkUtils::Allocator, &m_CommandPool);
}

void VkContext::InitVulkanContextDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                64},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          64},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          64},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   64},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   64},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         64},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         64},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 64},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 64},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       64}
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = std::size(poolSizes);
    descriptorPoolCreateInfo.pPoolSizes = std::data(poolSizes);
    descriptorPoolCreateInfo.maxSets = 1024 * std::size(poolSizes);
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    vkCheckCreate(DescriptorPool, m_Device, &descriptorPoolCreateInfo, VkUtils::Allocator, &m_DescriptorPool);
}