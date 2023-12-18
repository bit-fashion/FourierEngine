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
#include "VulkanContext.h"
#include "Window/Window.h"
#include "VulkanUtils.h"

VulkanContext::VulkanContext(Window *window) : m_Window(window) {
    InitVulkanDriverContext();
}

VulkanContext::~VulkanContext() {
    DestroySwapchainContext(&m_SwapchainContext);
    vkDestroyDevice(m_Device, VulkanUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_SurfaceKHR, VulkanUtils::Allocator);
    vkDestroyInstance(m_Instance, VulkanUtils::Allocator);
}

void VulkanContext::_CreateSwapcahinAboutComponents(VkSwapchainContextKHR *pSwapchainContext) {
    VkSwapchainCreateInfoKHR swapchainCreateInfoKHR = {};
    swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKHR.surface = m_SwapchainContext.surface;
    swapchainCreateInfoKHR.minImageCount = m_SwapchainContext.minImageCount;
    swapchainCreateInfoKHR.imageFormat = m_SwapchainContext.surfaceFormat.format;
    swapchainCreateInfoKHR.imageColorSpace = m_SwapchainContext.surfaceFormat.colorSpace;
    swapchainCreateInfoKHR.imageExtent = { m_SwapchainContext.width, m_SwapchainContext.height };
    swapchainCreateInfoKHR.imageArrayLayers = 1;
    swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKHR.preTransform = m_SwapchainContext.capabilities.currentTransform;
    swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKHR.presentMode = m_SwapchainContext.presentMode;
    swapchainCreateInfoKHR.clipped = VK_TRUE;
    swapchainCreateInfoKHR.oldSwapchain = null;

    vkCreateSwapchainKHR(m_Device, &swapchainCreateInfoKHR, VulkanUtils::Allocator,
                         &m_SwapchainContext.swapchain);

    vkGetSwapchainImagesKHR(m_Device, pSwapchainContext->swapchain, &pSwapchainContext->minImageCount, null);
    pSwapchainContext->images.resize(pSwapchainContext->minImageCount);
    vkGetSwapchainImagesKHR(m_Device, pSwapchainContext->swapchain, &pSwapchainContext->minImageCount, std::data(pSwapchainContext->images));

    /* create swapcahin image view and framebuffer */
    pSwapchainContext->imageViews.resize(pSwapchainContext->minImageCount);
    pSwapchainContext->framebuffers.resize(pSwapchainContext->minImageCount);
    for (uint32_t i = 0; i < pSwapchainContext->minImageCount; i++) {
        /* view */
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = pSwapchainContext->images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = pSwapchainContext->surfaceFormat.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(m_Device, &imageViewCreateInfo, VulkanUtils::Allocator, &pSwapchainContext->imageViews[i]);

        /* framebuffer */
        VkImageView attachments[] = { pSwapchainContext->imageViews[i] };
        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = pSwapchainContext->renderpass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = pSwapchainContext->width;
        framebufferCreateInfo.height = pSwapchainContext->height;
        framebufferCreateInfo.layers = 1;

        vkCreateFramebuffer(m_Device, &framebufferCreateInfo, VulkanUtils::Allocator, &pSwapchainContext->framebuffers[i]);
    }
}

void VulkanContext::_CreateRenderpass(VkSwapchainContextKHR *pSwapchainContext) {
    VkAttachmentDescription colorAttachmentDescription = {};
    colorAttachmentDescription.format = pSwapchainContext->surfaceFormat.format;
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentReference = {};
    colorAttachmentReference.attachment = 0;
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorAttachmentReference;

    VkSubpassDependency subpassDependency = {};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = {};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    vkCreateRenderPass(m_Device, &renderPassCreateInfo, VulkanUtils::Allocator, &pSwapchainContext->renderpass);
}

void VulkanContext::_ConfigurationSwapchainContext(VkSwapchainContextKHR *pSwapchainContext) {
    VulkanUtils::ConfigurationVulkanSwapchainContextDetail(m_PhysicalDevice, m_SurfaceKHR, m_Window,
                                                       &m_SwapchainContext);
}

void VulkanContext::_ConfigurationWindowResizeableEventCallback() {
    m_Window->AddWindowResizeableCallback([](Window *window, int width, int height){
        VulkanContext *context = (VulkanContext *) window->GetWindowUserPointer("VulkanContext");
        context->RecreateSwapchainContext(&context->m_SwapchainContext, width, height);
    });
}

void VulkanContext::DeviceWaitIdle() {
    vkDeviceWaitIdle(m_Device);
}

void VulkanContext::CopyBuffer(VkDeviceBuffer dest, VkDeviceBuffer src, VkDeviceSize size) {
    VkCommandBuffer oneTimeCommandBuffer;
    BeginOnceTimeCommandBufferSubmit(&oneTimeCommandBuffer);
    {
        /* copy buffer */
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(oneTimeCommandBuffer, src.buffer, dest.buffer, 1, &copyRegion);
    }
    EndOnceTimeCommandBufferSubmit();
}

void VulkanContext::MapMemory(VkDeviceBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData) {
    vkMapMemory(m_Device, buffer.memory, offset, size, flags, ppData);
}

void VulkanContext::UnmapMemory(VkDeviceBuffer buffer) {
    vkUnmapMemory(m_Device, buffer.memory);
}

void VulkanContext::BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags) {
    /* start command buffers record. */
    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = usageFlags;
    commandBufferBeginInfo.pInheritanceInfo = null; // Optional
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
}

void VulkanContext::EndCommandBuffer(VkCommandBuffer commandBuffer) {
    /* end command buffer record. */
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void VulkanContext::SyncSubmitQueueWithSubmitInfo(uint32_t commandBufferCount, VkCommandBuffer *pCommandBuffers,
                                                 uint32_t waitSemaphoreCount, VkSemaphore *pWaitSemaphores,
                                                 uint32_t signalSemaphoreCount, VkSemaphore *pSignalSemaphores,
                                                 VkPipelineStageFlags *pWaitDstStageMask) {
    /* submit command buffer */
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = waitSemaphoreCount;
    submitInfo.pWaitSemaphores = pWaitSemaphores;
    submitInfo.pWaitDstStageMask = pWaitDstStageMask;
    submitInfo.commandBufferCount = commandBufferCount;
    submitInfo.pCommandBuffers = pCommandBuffers;

    submitInfo.signalSemaphoreCount = signalSemaphoreCount;
    submitInfo.pSignalSemaphores = pSignalSemaphores;

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    vkQueueWaitIdle(m_GraphicsQueue);
}

void VulkanContext::BeginOnceTimeCommandBufferSubmit(VkCommandBuffer *pCommandBuffer) {
    AllocateCommandBuffer(1, &m_SingleTimeCommandBuffer);
    *pCommandBuffer = m_SingleTimeCommandBuffer;
    /* begin */
    BeginCommandBuffer(m_SingleTimeCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

void VulkanContext::EndOnceTimeCommandBufferSubmit() {
    EndCommandBuffer(m_SingleTimeCommandBuffer);
    /* submit */
    SyncSubmitQueueWithSubmitInfo(1, &m_SingleTimeCommandBuffer, 0, NULL, 0, NULL, NULL);
    FreeCommandBuffer(1, &m_SingleTimeCommandBuffer);
}

void VulkanContext::BeginRenderPass(VkRenderPass renderPass) {
    /* start render pass. */
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = m_FrameContext.framebuffer;
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = { m_SwapchainContext.width, m_SwapchainContext.height };

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(m_FrameContext.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanContext::EndRenderPass() {
    /* end render pass */
    vkCmdEndRenderPass(m_FrameContext.commandBuffer);
}

void VulkanContext::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags,
                                             VkDescriptorSetLayout *pDescriptorSetLayout) {
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.flags = flags;
    descriptorSetLayoutCreateInfo.bindingCount = std::size(bindings);
    descriptorSetLayoutCreateInfo.pBindings = std::data(bindings);
    vkCreateDescriptorSetLayout(m_Device, &descriptorSetLayoutCreateInfo, VulkanUtils::Allocator, pDescriptorSetLayout);
}

void VulkanContext::CreateGraphicsPipeline(const String &shaderfolder, const String &shadername, VkDescriptorSetLayout descriptorSetLayout,
                                           VkDriverGraphicsPipeline *pDriverGraphicsPipeline) {
    /** Create shader of vertex & fragment module. */
    VkShaderModule vertexShaderModule =
            VulkanUtils::LoadShaderModule(m_Device, shaderfolder, shadername, VK_SHADER_STAGE_VERTEX_BIT);
    VkShaderModule fragmentShaderModule =
            VulkanUtils::LoadShaderModule(m_Device, shaderfolder, shadername, VK_SHADER_STAGE_FRAGMENT_BIT);

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

    VkVertexInputBindingDescription vertexInputBindingDescription = VulkanUtils::GetVertexInputBindingDescription();
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

    auto vertexInputAttributeDescriptions = VulkanUtils::GetVertexInputAttributeDescriptionArray();
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = std::size(vertexInputAttributeDescriptions);
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = std::data(vertexInputAttributeDescriptions);

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssembly = {};
    pipelineInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_SwapchainContext.width;
    viewport.height = (float) m_SwapchainContext.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = { m_SwapchainContext.width, m_SwapchainContext.height };

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
    VkDynamicState dynamicStates[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
    pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    pipelineDynamicStateCreateInfo.dynamicStateCount = 2;
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates;

    /* 管道布局 */
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, VulkanUtils::Allocator, &pDriverGraphicsPipeline->pipelineLayout);

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
    graphicsPipelineCreateInfo.pDynamicState = nullptr; // Optional
    graphicsPipelineCreateInfo.layout = pDriverGraphicsPipeline->pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = m_SwapchainContext.renderpass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    graphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional

    vkCreateGraphicsPipelines(m_Device, null, 1, &graphicsPipelineCreateInfo,
                              VulkanUtils::Allocator, &pDriverGraphicsPipeline->pipeline);

    /* 销毁着色器模块 */
    vkDestroyShaderModule(m_Device, vertexShaderModule, VulkanUtils::Allocator);
    vkDestroyShaderModule(m_Device, fragmentShaderModule, VulkanUtils::Allocator);
}

void VulkanContext::AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer) {
    /** Allocate command buffer. */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_CommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t) count;

    vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, pCommandBuffer);
}

void VulkanContext::AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                                   VkDeviceBuffer *buffer) {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    buffer->size = bufferCreateInfo.size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkCreateBuffer(m_Device, &bufferCreateInfo, VulkanUtils::Allocator, &buffer->buffer);
    vkCreateBuffer(m_Device, &bufferCreateInfo, VulkanUtils::Allocator, &buffer->buffer);

    /** Query memory requirements. */
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_Device, buffer->buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = VulkanUtils::FindMemoryType(memoryRequirements.memoryTypeBits, m_PhysicalDevice, properties);

    vkAllocateMemory(m_Device, &memoryAllocInfo, VulkanUtils::Allocator, &buffer->memory);
    vkBindBufferMemory(m_Device, buffer->buffer, buffer->memory, 0);
}

void VulkanContext::RecreateSwapchainContext(VkSwapchainContextKHR *pSwapchainContext, uint32_t width, uint32_t height) {
    if (width <= 0 || height <= 0)
        return;
    DeviceWaitIdle();
    DestroySwapchainContext(pSwapchainContext);
    CreateSwapchainContext(pSwapchainContext);
}

void VulkanContext::CreateSwapchainContext(VkSwapchainContextKHR *pSwapchainContext) {
    _ConfigurationSwapchainContext(pSwapchainContext);
    _CreateRenderpass(pSwapchainContext);
    _CreateSwapcahinAboutComponents(pSwapchainContext);
}

void VulkanContext::InitVulkanDriverContext() {
    m_Window->PutWindowUserPointer("VulkanContext", this);
    _ConfigurationWindowResizeableEventCallback();

    /* Create vulkan instance. */
    struct VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_VERSION_1_3;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = SPORTS_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = SPORTS_ENGINE_NAME;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    static Vector<const char *> requiredEnableExtensionsForInstance;
    VulkanUtils::GetVulkanInstanceRequiredExtensions(requiredEnableExtensionsForInstance);
    instanceCreateInfo.enabledExtensionCount = std::size(requiredEnableExtensionsForInstance);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(requiredEnableExtensionsForInstance);

    static Vector<const char *> requiredEnableLayersForInstance;
    VulkanUtils::GetVulkanInstanceRequiredLayers(requiredEnableLayersForInstance);
    instanceCreateInfo.enabledLayerCount = std::size(requiredEnableLayersForInstance);
    instanceCreateInfo.ppEnabledLayerNames = std::data(requiredEnableLayersForInstance);
    vkCreateInstance(&instanceCreateInfo, VulkanUtils::Allocator, &m_Instance);

    /* Create window surface */
#ifdef _glfw3_h_
    VulkanUtils::CreateWindowSurfaceKHR(m_Instance, m_Window->GetWindowPointer(), &m_SurfaceKHR);
#endif

    /* Create vulkan device. */
    VulkanUtils::GetVulkanMostPreferredPhysicalDevice(m_Instance, &m_PhysicalDevice, &m_PhysicalDeviceProperties,
                                                      &m_PhysicalDeviceFeature);
    Vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    VulkanUtils::QueueFamilyIndices queueFamilyIndices =
            VulkanUtils::GetVulkanDeviceCreateRequiredQueueFamilyAndQueueCreateInfo(m_PhysicalDevice, m_SurfaceKHR, deviceQueueCreateInfos);

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    static VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;

    static Vector<const char *> requiredEnableExtensions;
    VulkanUtils::GetVulkanDeviceRequiredExtensions(requiredEnableExtensions);
    deviceCreateInfo.enabledExtensionCount = std::size(requiredEnableExtensions);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(requiredEnableExtensions);

    static Vector<const char *> requiredEnableLayers;
    VulkanUtils::GetVulkanDeviceRequiredLayers(requiredEnableLayers);
    deviceCreateInfo.enabledLayerCount = std::size(requiredEnableLayers);
    deviceCreateInfo.ppEnabledLayerNames = std::data(requiredEnableLayers);

    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);

    vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, VulkanUtils::Allocator, &m_Device);

    /* get queue */
    vkGetDeviceQueue(m_Device, queueFamilyIndices.graphicsQueueFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, queueFamilyIndices.presentQueueFamily, 0, &m_PresentQueue);

    /* Create command pool. */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsQueueFamily;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool(m_Device, &commandPoolCreateInfo, VulkanUtils::Allocator, &m_CommandPool);

    /* Create swapchain */
    CreateSwapchainContext(&m_SwapchainContext);
}

void VulkanContext::DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
    vkDestroyDescriptorSetLayout(m_Device, descriptorSetLayout, VulkanUtils::Allocator);
}

void VulkanContext::DestroyGraphicsPipeline(VkDriverGraphicsPipeline driverGraphicsPipeline) {
    vkDestroyPipeline(m_Device, driverGraphicsPipeline.pipeline, VulkanUtils::Allocator);
    vkDestroyPipelineLayout(m_Device, driverGraphicsPipeline.pipelineLayout, VulkanUtils::Allocator);
}

void VulkanContext::FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer) {
    vkFreeCommandBuffers(m_Device, m_CommandPool, count, pCommandBuffer);
}

void VulkanContext::FreeBuffer(VkDeviceBuffer buffer) {
    vkFreeMemory(m_Device, buffer.memory, VulkanUtils::Allocator);
    vkDestroyBuffer(m_Device, buffer.buffer, VulkanUtils::Allocator);
}

void VulkanContext::DestroySwapchainContext(VkSwapchainContextKHR *pSwapchainContext) {
    vkDestroyRenderPass(m_Device, pSwapchainContext->renderpass, VulkanUtils::Allocator);
    for (int i = 0; i < pSwapchainContext->minImageCount; i++) {
        vkDestroyImageView(m_Device, pSwapchainContext->imageViews[i], VulkanUtils::Allocator);
        vkDestroyFramebuffer(m_Device, pSwapchainContext->framebuffers[i], VulkanUtils::Allocator);
    }
    vkDestroySwapchainKHR(m_Device, pSwapchainContext->swapchain, VulkanUtils::Allocator);
}