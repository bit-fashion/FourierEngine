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
#include <Aurora/IOUtils.h>

/* malloc */
#define VtxMemoryMalloc(object) (object) MemoryMalloc(sizeof(object##_T))
#define VtxMemoryFree(ptr) MemoryFree(ptr)

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

void VkContext::CreatePipeline(VtxPipelineCreateConfiguration *pConfiguration, VtxPipeline *pPipeline)
{
    char *buf;
    size_t size;
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    *pPipeline = VtxMemoryMalloc(VtxPipeline);

    /* 创建顶点着色器 */
    char vertex_shader_file[255];
    snprintf(vertex_shader_file, sizeof(vertex_shader_file), "%s.vert", pConfiguration->ShaderFileName);

    buf = IOUtils::ReadBuf(vertex_shader_file, &size);
    VkShaderModuleCreateInfo vertexShaderModuleCreateInfo = {};
    vertexShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertexShaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t *>(buf);
    vertexShaderModuleCreateInfo.codeSize = size;
    vkCreateShaderModule(m_Device, &vertexShaderModuleCreateInfo, VkUtils::Allocator, &vertexShaderModule);
    IOUtils::FreeBuf(buf);

    /* 创建片段着色器 */
    char fragment_shader_file[255];
    snprintf(fragment_shader_file, sizeof(fragment_shader_file), "%s.frag", pConfiguration->ShaderFileName);

    buf = IOUtils::ReadBuf(vertex_shader_file, &size);
    VkShaderModuleCreateInfo fragmentShaderModuleCreateInfo = {};
    fragmentShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragmentShaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t *>(buf);
    fragmentShaderModuleCreateInfo.codeSize = size;
    vkCreateShaderModule(m_Device, &fragmentShaderModuleCreateInfo, VkUtils::Allocator, &fragmentShaderModule);
    IOUtils::FreeBuf(buf);

    /** Create pipeline phase of vertex and fragment shader */
    VkPipelineShaderStageCreateInfo pipelineVertexShaderStageCreateInfo = {};
    pipelineVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineVertexShaderStageCreateInfo.module = vertexShaderModule;
    pipelineVertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo pipelineFragmentStageCreateInfo = {};
    pipelineFragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineFragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineFragmentStageCreateInfo.module = fragmentShaderModule;
    pipelineFragmentStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[] = { pipelineVertexShaderStageCreateInfo, pipelineFragmentStageCreateInfo };

    /* pipeline features */
    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
    pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    std::array<VkVertexInputAttributeDescription, 3> vertexInputAttributeDescription = {};
    vertexInputAttributeDescription[0].binding  = 0;
    vertexInputAttributeDescription[0].location = 0;
    vertexInputAttributeDescription[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributeDescription[0].offset   = offsetof(Vertex, position);

    vertexInputAttributeDescription[1].binding  = 0;
    vertexInputAttributeDescription[1].location = 1;
    vertexInputAttributeDescription[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributeDescription[1].offset   = offsetof(Vertex, color);

    vertexInputAttributeDescription[2].binding  = 0;
    vertexInputAttributeDescription[2].location = 2;
    vertexInputAttributeDescription[2].format   = VK_FORMAT_R32G32_SFLOAT;
    vertexInputAttributeDescription[2].offset   = offsetof(Vertex, uv);

    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = std::size(vertexInputAttributeDescription);
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = std::data(vertexInputAttributeDescription);

    std::array<VkVertexInputBindingDescription, 1> vertexInputBindingDescription = {};
    vertexInputBindingDescription[0].binding = 0;
    vertexInputBindingDescription[0].stride = sizeof(Vertex);
    vertexInputBindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = std::size(vertexInputBindingDescription);
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = std::data(vertexInputBindingDescription);

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssembly = {};
    pipelineInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) pConfiguration->Width;
    viewport.height = (float) pConfiguration->Height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = { pConfiguration->Width, pConfiguration->Height };

    /* 视口裁剪 */
    VkPipelineViewportStateCreateInfo pipelineViewportStateCrateInfo = {};
    pipelineViewportStateCrateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipelineViewportStateCrateInfo.viewportCount = 1;
    pipelineViewportStateCrateInfo.pViewports = &viewport;
    pipelineViewportStateCrateInfo.scissorCount = 1;
    pipelineViewportStateCrateInfo.pScissors = &scissor;

    /* 光栅化阶段 */
    VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
    pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; // Optional
    pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; // Optional
    pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; // Optional

    /* 多重采样 */
    VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = {};
    pipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    pipelineMultisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f; // Optional
    pipelineMultisampleStateCreateInfo.pSampleMask = nullptr; // Optional
    pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    pipelineMultisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

    /* 颜色混合 */
    VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
    pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
    pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
    pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    /* 帧缓冲 */
    VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
    pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    pipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
    pipelineColorBlendStateCreateInfo.attachmentCount = 1;
    pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
    pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f; // Optional
    pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f; // Optional

    /* 动态修改 */
    Vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_LINE_WIDTH,
    };

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineDynamicStateCreateInfo.dynamicStateCount = std::size(dynamicStates);
    pipelineDynamicStateCreateInfo.pDynamicStates = std::data(dynamicStates);

    /* 管道布局 */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &pConfiguration->DescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, VkUtils::Allocator, &(*pPipeline)->pipelineLayout);

    /** Create graphics pipeline in vulkan. */
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos;
    graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssembly;
    graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCrateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr; // Optional
    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo; // Optional
    graphicsPipelineCreateInfo.layout = (*pPipeline)->pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = pConfiguration->RenderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    graphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional

    vkCreateGraphicsPipelines(m_Device, null, 1, &graphicsPipelineCreateInfo,
                              VkUtils::Allocator, &(*pPipeline)->pipeline);

    /* 销毁着色器模块 */
    vkDestroyShaderModule(m_Device, vertexShaderModule, VkUtils::Allocator);
    vkDestroyShaderModule(m_Device, fragmentShaderModule, VkUtils::Allocator);

}

void VkContext::DestroyPipeline(VtxPipeline pipeline)
{
    vkDestroyPipelineLayout(m_Device, pipeline->pipelineLayout, VkUtils::Allocator);
    vkDestroyPipeline(m_Device, pipeline->pipeline, VkUtils::Allocator);
    VtxMemoryFree(pipeline);
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
    vkFreeDescriptorSets(m_Device, m_DescriptorPool, 1, &descriptorSet);
}

void VkContext::WriteTexture2D(VkDeviceSize offset, VkDeviceSize size, void *buf, VtxTexture2D texture2D)
{

}

void VkContext::CreateTexture2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                VkMemoryPropertyFlags properties, VtxTexture2D *pTexture2D)
{
    *pTexture2D = VtxMemoryMalloc(VtxTexture2D);

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
    (*pTexture2D)->size = width * height;

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
    VtxMemoryFree(texture2D);
}

void VkContext::AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VtxBuffer *pBuffer)
{
    *pBuffer = VtxMemoryMalloc(VtxBuffer);

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
    VtxMemoryFree(buffer);
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

void VkContext::CmdBindPipeline(VkCommandBuffer commandBuffer, VtxPipeline pipeline)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
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
            { VK_DESCRIPTOR_TYPE_SAMPLER,                1024 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1024 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1024 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1024 }
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = std::size(poolSizes);
    descriptorPoolCreateInfo.pPoolSizes = std::data(poolSizes);
    descriptorPoolCreateInfo.maxSets = 1024 * std::size(poolSizes);
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    vkCheckCreate(DescriptorPool, m_Device, &descriptorPoolCreateInfo, VkUtils::Allocator, &m_DescriptorPool);
}