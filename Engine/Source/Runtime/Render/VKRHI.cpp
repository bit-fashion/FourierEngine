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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ************************************************************************/

/* Creates on 2022/11/23. */
#include "VKRHI.h"

#include "Window/VRRTwindow.h"
#include "Utils/IOUtils.h"

#define VRRT_SHADER_MODULE_OF_VERTEX_BINARY_FILE "../Engine/Binaries/simple_shader.vert.spv"
#define VRRT_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE "../Engine/Binaries/simple_shader.frag.spv"

/* Get required instance extensions for vulkan. */
void VKRHIGetRequiredInstanceExtensions(std::vector<const char *> &vec,
                                          std::unordered_map<std::string, VkExtensionProperties> &supports) {
    /* glfw */
    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (int i = 0; i < glfwExtensionCount; i++)
        vec.push_back(glfwExtensions[i]);
}

/* Get required instance layers for vulkan. */
void VKRHIGetRequiredInstanceLayers(std::vector<const char *> &vec,
                                      std::unordered_map<std::string, VkLayerProperties> &supports) {
#ifdef VRRT_ENGINE_CONFIG_ENABLE_DEBUG
    if (supports.count(VK_LAYER_KHRONOS_validation) != 0)
        vec.push_back(VK_LAYER_KHRONOS_validation);
#endif
}

/**
 * 获取设备必须启用的扩展列表
 */
static void VKRHIGetRequiredEnableDeviceExtensions(std::unordered_map<std::string, VkExtensionProperties> &supports,
                                                 std::vector<const char *> *pRequired) {
    if (supports.count(VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
        pRequired->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VKRHIQueueFamilyIndices _VKRHIDevice::FindQueueFamilyIndices() {
    VKRHIQueueFamilyIndices queueFamilyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_VKRHIGPU.device, &queueFamilyCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_VKRHIGPU.device, &queueFamilyCount, std::data(queueFamilies));

    int index = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices.graphicsQueueFamily = index;

        VkBool32 isPresentMode = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_VKRHIGPU.device, index, m_SurfaceKHR, &isPresentMode);
        if (queueFamilyCount > 0 && isPresentMode)
            queueFamilyIndices.presentQueueFamily= index;

        if (queueFamilyIndices.graphicsQueueFamily > 0 && queueFamilyIndices.presentQueueFamily > 0)
            break;

        ++index;
    }

    return queueFamilyIndices;
}

/* Query swapchain details. */
VKRHISwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VKRHISwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    /* Find surface support formats. */
    {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, VK_NULL_HANDLE);
        if (formatCount > 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, std::data(details.formats));
        }
    }

    /* Find surface support present mode. */
    {
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, VK_NULL_HANDLE);
        if (presentModeCount > 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, std::data(details.presentModes));
        }
    }

    return details;
}

/* Select surface format. */
VkSurfaceFormatKHR SelectSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    if (std::size(formats) == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return formats[0];
}

/* Select surface presentMode. */
VkPresentModeKHR SelectSwapSurfacePresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        } else if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            bestMode = presentMode;
        }
    }

    return bestMode;
}

/* Select swap chain extent. */
VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VRRTwindow *pRIVwindow) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = { static_cast<uint32_t>(pRIVwindow->GetWidth()), static_cast<uint32_t>(pRIVwindow->GetHeight()) };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkShaderModule LoadShaderModule(VkDevice device, const char *file_path) {
    char *buf;
    size_t size;
    VkShaderModule shader;

    /* load shader binaries. */
    buf = vrrt_load_file(file_path, &size);

    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = size;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(buf);
    vkVRRTCreate(ShaderModule, device, &shaderModuleCreateInfo, VK_NULL_HANDLE, &shader);

    /* free binaries buf. */
    vrrt_free_buffer(buf);

    return shader;
}

// ----------------------------------------------------------------------------
// Pipeline
// ----------------------------------------------------------------------------

_VKRHIPipeline::_VKRHIPipeline(VKRHIDevice device, VKRHISwapchain swapchain, const char *vertex_shader_path, const char *fragment_shader_path)
  : m_VKRHIDevice(device), m_Swapchain(swapchain){
    /** Create shader of vertex & fragment module. */
    VRRT_LOGGER_INFO("Loading and create vertex shader module from: {}", vertex_shader_path);
    VkShaderModule vertex_shader_module = LoadShaderModule(m_VKRHIDevice->GetDeviceHandle(), vertex_shader_path);
    VRRT_LOGGER_INFO("Loading and create vertex shader module success!");

    VRRT_LOGGER_INFO("Loading and create fragment shader module from: {}", fragment_shader_path);
    VkShaderModule fragment_shader_module = LoadShaderModule(m_VKRHIDevice->GetDeviceHandle(), fragment_shader_path);
    VRRT_LOGGER_INFO("Loading and create fragment shader module success!");

    /** Create pipeline phase of vertex and fragment shader */
    VkPipelineShaderStageCreateInfo pipelineVertexShaderStageCreateInfo = {};
    pipelineVertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineVertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    pipelineVertexShaderStageCreateInfo.module = vertex_shader_module;
    pipelineVertexShaderStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo pipelineFragmentStageCreateInfo = {};
    pipelineFragmentStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineFragmentStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineFragmentStageCreateInfo.module = fragment_shader_module;
    pipelineFragmentStageCreateInfo.pName = "main";

    VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[] = { pipelineVertexShaderStageCreateInfo, pipelineFragmentStageCreateInfo };

    /* pipeline features */
    VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
    pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssembly = {};
    pipelineInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) swapchain->GetWidth();
    viewport.height = (float) swapchain->GetHeight();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = { swapchain->GetWidth(), swapchain->GetHeight() };

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
    pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vkVRRTCreate(PipelineLayout, m_VKRHIDevice->GetDeviceHandle(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout);

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
    graphicsPipelineCreateInfo.layout = m_PipelineLayout;
    graphicsPipelineCreateInfo.renderPass = m_Swapchain->GetRenderPassHandle();
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    graphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional

    vkVRRTCreate(GraphicsPipelines, m_VKRHIDevice->GetDeviceHandle(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, VK_NULL_HANDLE, &m_Pipeline);

    /* 销毁着色器模块 */
    vkDestroyShaderModule(m_VKRHIDevice->GetDeviceHandle(), vertex_shader_module, VK_NULL_HANDLE);
    vkDestroyShaderModule(m_VKRHIDevice->GetDeviceHandle(), fragment_shader_module, VK_NULL_HANDLE);
}

_VKRHIPipeline::~_VKRHIPipeline() {
    vkDestroyPipelineLayout(m_VKRHIDevice->GetDeviceHandle(), m_PipelineLayout, VK_NULL_HANDLE);
    vkDestroyPipeline(m_VKRHIDevice->GetDeviceHandle(), m_Pipeline, VK_NULL_HANDLE);
}

// ----------------------------------------------------------------------------
// Swapchain
// ----------------------------------------------------------------------------

_VKRHISwapchain::_VKRHISwapchain(VKRHIDevice device, VRRTwindow *pVRRTwindow, VkSurfaceKHR surface)
  : m_VKRHIDevice(device), m_VRRTwindow(pVRRTwindow), m_Surface(surface) {
    /* 查询交换链支持 */
    VKRHISwapChainSupportDetails swapChainDetails = QuerySwapChainSupportDetails(m_VKRHIDevice->GetPhysicalDeviceHandle(), m_Surface);

    m_SurfaceFormatKHR = SelectSwapSurfaceFormat(swapChainDetails.formats);
    m_SwapchainFormat = m_SurfaceFormatKHR.format;
    m_SwapchainPresentModeKHR = SelectSwapSurfacePresentMode(swapChainDetails.presentModes);
    m_SwapchainExtent = SelectSwapExtent(swapChainDetails.capabilities, pVRRTwindow);

    /** Create render pass. */
    VkAttachmentDescription colorAttachmentDescription = {};
    colorAttachmentDescription.format = m_SwapchainFormat;
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

    vkVRRTCreate(RenderPass, m_VKRHIDevice->GetDeviceHandle(), &renderPassCreateInfo, VK_NULL_HANDLE, &m_RenderPass);

    /* 设置三重缓冲 */
    m_SwapchainImageCount = swapChainDetails.capabilities.minImageCount + 1;
    if (swapChainDetails.capabilities.maxImageCount > 0 && m_SwapchainImageCount > swapChainDetails.capabilities.maxImageCount)
        m_SwapchainImageCount = swapChainDetails.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfoKhr = {};
    swapchainCreateInfoKhr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKhr.surface = m_Surface;
    swapchainCreateInfoKhr.minImageCount = m_SwapchainImageCount;
    swapchainCreateInfoKhr.imageFormat = m_SurfaceFormatKHR.format;
    swapchainCreateInfoKhr.imageColorSpace = m_SurfaceFormatKHR.colorSpace;
    swapchainCreateInfoKhr.imageExtent = m_SwapchainExtent;
    swapchainCreateInfoKhr.imageArrayLayers = 1;
    swapchainCreateInfoKhr.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKhr.preTransform = swapChainDetails.capabilities.currentTransform;
    swapchainCreateInfoKhr.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKhr.presentMode = m_SwapchainPresentModeKHR;
    swapchainCreateInfoKhr.clipped = VK_TRUE;
    swapchainCreateInfoKhr.oldSwapchain = VK_NULL_HANDLE;
    vkVRRTCreate(SwapchainKHR, m_VKRHIDevice->GetDeviceHandle(), &swapchainCreateInfoKhr, VK_NULL_HANDLE, &m_Swapchain);

    vkGetSwapchainImagesKHR(m_VKRHIDevice->GetDeviceHandle(), m_Swapchain, &m_SwapchainImageCount, VK_NULL_HANDLE);
    m_SwapchainImages.resize(m_SwapchainImageCount);
    vkGetSwapchainImagesKHR(m_VKRHIDevice->GetDeviceHandle(), m_Swapchain, &m_SwapchainImageCount, std::data(m_SwapchainImages));

    /** Create image views. */
    m_SwapchainImageViews.resize(m_SwapchainImageCount);
    for (uint32_t i = 0; i < m_SwapchainImageCount; i++) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_SwapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = m_SurfaceFormatKHR.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        vkVRRTCreate(ImageView, m_VKRHIDevice->GetDeviceHandle(), &imageViewCreateInfo, VK_NULL_HANDLE, &m_SwapchainImageViews[i]);
    }

    /* 帧缓冲区 */
    m_SwapchainFramebuffers.resize(m_SwapchainImageCount);
    for (size_t i = 0; i < m_SwapchainImageCount; i++) {
        VkImageView attachments[] = { m_SwapchainImageViews[i] };

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = m_RenderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = m_SwapchainExtent.width;
        framebufferCreateInfo.height = m_SwapchainExtent.height;
        framebufferCreateInfo.layers = 1;

        vkVRRTCreate(Framebuffer, m_VKRHIDevice->GetDeviceHandle(), &framebufferCreateInfo, nullptr, &m_SwapchainFramebuffers[i]);
    }
}

_VKRHISwapchain::~_VKRHISwapchain() {
    vkDestroyRenderPass(m_VKRHIDevice->GetDeviceHandle(), m_RenderPass, VK_NULL_HANDLE);
    for (const auto &imageView: m_SwapchainImageViews)
        vkDestroyImageView(m_VKRHIDevice->GetDeviceHandle(), imageView, VK_NULL_HANDLE);
    for (const auto &framebuffer: m_SwapchainFramebuffers)
        vkDestroyFramebuffer(m_VKRHIDevice->GetDeviceHandle(), framebuffer, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(m_VKRHIDevice->GetDeviceHandle(), m_Swapchain, VK_NULL_HANDLE);
}

void _VKRHISwapchain::AcquireNextImage(VkSemaphore semaphore, uint32_t *pIndex) {
    vkAcquireNextImageKHR(m_VKRHIDevice->GetDeviceHandle(), m_Swapchain, std::numeric_limits<uint64_t>::max(),
                          semaphore, VK_NULL_HANDLE, pIndex);
}

// ----------------------------------------------------------------------------
// Device
// ----------------------------------------------------------------------------

_VKRHIDevice::_VKRHIDevice(VkInstance instance, VkSurfaceKHR surface, VRRTwindow *pVRRTwindow)
  : m_Instance(instance), m_SurfaceKHR(surface), m_VRRTwindow(pVRRTwindow) {
    /* 选择一个牛逼的 GPU 设备 */
    std::vector<VKRHIGPU> vGPU;
    VKRHIGETGPU(m_Instance, &vGPU);
    m_VKRHIGPU = vGPU[0];

    /* 获取设备支持的所有扩展列表 */
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(m_VKRHIGPU.device, VK_NULL_HANDLE, &deviceExtensionCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(m_VKRHIGPU.device, VK_NULL_HANDLE, &deviceExtensionCount, std::data(deviceExtensions));
    for (auto &extension : deviceExtensions)
        m_DeviceSupportedExtensions.insert({extension.extensionName, extension});

    /* 查询设备支持的队列 */
    m_VKRHIQueueFamilyIndices = FindQueueFamilyIndices();
    std::vector<uint32_t> uniqueQueueFamilies = {m_VKRHIQueueFamilyIndices.graphicsQueueFamily, m_VKRHIQueueFamilyIndices.presentQueueFamily};
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo deviceQueueCreateInfo = {};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueFamilyIndex = queueFamily;
        deviceQueueCreateInfo.queueCount = 1;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    /** 创建 Vulkan 逻辑设备句柄对象 */
    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = std::size(deviceQueueCreateInfos);
    deviceCreateInfo.pQueueCreateInfos = std::data(deviceQueueCreateInfos);
    VkPhysicalDeviceFeatures features = {};
    deviceCreateInfo.pEnabledFeatures = &features;
    /* 选择设备启用扩展 */
    std::vector<const char *> vRequiredEnableExtension;
    VKRHIGetRequiredEnableDeviceExtensions(m_DeviceSupportedExtensions, &vRequiredEnableExtension);
    deviceCreateInfo.enabledExtensionCount = std::size(vRequiredEnableExtension);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(vRequiredEnableExtension);
    vkVRRTCreate(Device, m_VKRHIGPU.device, &deviceCreateInfo, nullptr, &m_Device);

    /* 获取队列 */
    vkGetDeviceQueue(m_Device, m_VKRHIQueueFamilyIndices.graphicsQueueFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_VKRHIQueueFamilyIndices.presentQueueFamily, 0, &m_PresentQueue);

    /* init */
    InitAllocateDescriptorSet();
    InitCommandPool();
}

_VKRHIDevice::~_VKRHIDevice() {
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, VK_NULL_HANDLE);
    vkDestroyCommandPool(m_Device, m_CommandPool, VK_NULL_HANDLE);
    vkDestroyDevice(m_Device, VK_NULL_HANDLE);
}

void _VKRHIDevice::CreateSwapchain(VKRHISwapchain *pSwapchain) {
    *pSwapchain = new _VKRHISwapchain(this, m_VRRTwindow, m_SurfaceKHR);
}

void _VKRHIDevice::DestroySwapchain(VKRHISwapchain swapchain) {
    delete swapchain;
}

void _VKRHIDevice::AllocateDescriptorSet(uint32_t count, VkDescriptorSet *pDescriptorSet) {
    /** Allocate descriptor set */
    VkDescriptorSetLayout descriptorSetLayouts[] = {m_DescriptorSetLayout};
    VkDescriptorSetAllocateInfo descriptorAllocateInfo = {};
    descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocateInfo.descriptorPool = m_DescriptorPool;
    descriptorAllocateInfo.descriptorSetCount = count;
    descriptorAllocateInfo.pSetLayouts = descriptorSetLayouts;
    vkAllocateDescriptorSets(m_Device, &descriptorAllocateInfo, pDescriptorSet);
}

void _VKRHIDevice::FreeDescriptorSet(uint32_t count, VkDescriptorSet *pDescriptorSet) {
    vkFreeDescriptorSets(m_Device, m_DescriptorPool, count, pDescriptorSet);
}

void _VKRHIDevice::AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer) {
    /** Allocate command buffer. */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = m_CommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t) count;

    vkVRRTAllocate(CommandBuffers, m_Device, &commandBufferAllocateInfo, pCommandBuffer);
}

void _VKRHIDevice::FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer) {
    vkFreeCommandBuffers(m_Device, m_CommandPool, count, pCommandBuffer);
}

void _VKRHIDevice::CreateSemaphore(VkSemaphore *pSemaphore) {
    /** Create semaphores. */
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkVRRTCreate(Semaphore, m_Device, &semaphoreCreateInfo, VK_NULL_HANDLE, pSemaphore);
}

void _VKRHIDevice::DestroySemaphore(VkSemaphore semaphore) {
    vkDestroySemaphore(m_Device, semaphore, VK_NULL_HANDLE);
}

void _VKRHIDevice::InitAllocateDescriptorSet() {
    /** Descriptor set layout */
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
    descriptorSetLayoutBinding.binding = 0;
    descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorSetLayoutBinding.descriptorCount = 1;
    descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptorSetLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = 1;
    descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

    vkVRRTCreate(DescriptorSetLayout, m_Device, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &m_DescriptorSetLayout);

    /** Create descriptor set pool */
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCrateInfo = {};
    descriptorPoolCrateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCrateInfo.poolSizeCount = 1;
    descriptorPoolCrateInfo.pPoolSizes = &poolSize;
    descriptorPoolCrateInfo.maxSets = 1;

    vkVRRTCreate(DescriptorPool, m_Device, &descriptorPoolCrateInfo, VK_NULL_HANDLE, &m_DescriptorPool);
}

void _VKRHIDevice::InitCommandPool() {
    /** Create command pool. */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = m_VKRHIQueueFamilyIndices.graphicsQueueFamily;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkVRRTCreate(CommandPool, m_Device, &commandPoolCreateInfo, VK_NULL_HANDLE, &m_CommandPool);
}

// ----------------------------------------------------------------------------
// VKRHI
// ----------------------------------------------------------------------------

VKRHI::VKRHI(VRRTwindow *pVRRTwindow) : m_VRRTwindow(pVRRTwindow) {
    /* Enumerate instance available extensions. */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, std::data(extensions));
    VRRT_LOGGER_INFO("Vulkan render api available extensions for instance:");
    for (auto &extension : extensions) {
        VRRT_LOGGER_INFO("    {}", extension.extensionName);
        m_VkInstanceExtensionPropertiesSupports.insert({extension.extensionName, extension});
    }

    /* Enumerate instance available layers. */
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(layers));
    VRRT_LOGGER_INFO("Vulkan render api available layer list: ");
    for (auto &layer : layers) {
        VRRT_LOGGER_INFO("    {}", layer.layerName);
        m_VkInstanceLayerPropertiesSupports.insert({layer.layerName, layer});
    }

    /* Get extensions & layers. */
    VKRHIGetRequiredInstanceExtensions(m_RequiredInstanceExtensions, m_VkInstanceExtensionPropertiesSupports);
    VKRHIGetRequiredInstanceLayers(m_RequiredInstanceLayers, m_VkInstanceLayerPropertiesSupports);

    /** Create vulkan instance. */
    struct VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_VERSION_1_3;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = VRRT_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = VRRT_ENGINE_NAME;

    struct VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    instanceCreateInfo.enabledExtensionCount = std::size(m_RequiredInstanceExtensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(m_RequiredInstanceExtensions);
    VRRT_LOGGER_INFO("Used vulkan extension for instance count: {}", std::size(m_RequiredInstanceExtensions));
    for (const auto& name : m_RequiredInstanceExtensions)
        VRRT_LOGGER_INFO("    {}", name);

    instanceCreateInfo.enabledLayerCount = std::size(m_RequiredInstanceLayers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(m_RequiredInstanceLayers);

    vkVRRTCreate(Instance, &instanceCreateInfo, VK_NULL_HANDLE, &m_Instance);

    /** 创建 Surface 接口对象 */
    if (glfwCreateWindowSurface(m_Instance, pVRRTwindow->GetWindowHandle(),VK_NULL_HANDLE,
                                &m_Surface) != VK_SUCCESS) {
        VRRT_LOGGER_INFO("Create glfw surface failed!");
    }

    /* init */
    Init_Vulkan_Impl();
}

VKRHI::~VKRHI() {
    m_VKRHIDevice->DestroySemaphore(m_ImageAvailableSemaphore);
    m_VKRHIDevice->DestroySemaphore(m_RenderFinishedSemaphore);
    VRRT_FREE_POINTER(m_VKRHIPipeline);
    m_VKRHIDevice->DestroySwapchain(m_Swapchain);
    VRRT_FREE_POINTER(m_VKRHIDevice);
    vkDestroySurfaceKHR(m_Instance, m_Surface, VK_NULL_HANDLE);
    vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
}

void VKRHI::Init_Vulkan_Impl() {
    m_VKRHIDevice = std::make_unique<_VKRHIDevice>(m_Instance, m_Surface, m_VRRTwindow);
    m_VKRHIDevice->CreateSwapchain(&m_Swapchain);
    m_VKRHIPipeline = std::make_unique<_VKRHIPipeline>(m_VKRHIDevice.get(), m_Swapchain,
                                                   VRRT_SHADER_MODULE_OF_VERTEX_BINARY_FILE,
                                                   VRRT_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE);
    m_CommandBuffers.resize(m_Swapchain->GetImageCount());
    m_VKRHIDevice->AllocateCommandBuffer(std::size(m_CommandBuffers), std::data(m_CommandBuffers));
    m_VKRHIDevice->CreateSemaphore(&m_ImageAvailableSemaphore);
    m_VKRHIDevice->CreateSemaphore(&m_RenderFinishedSemaphore);
}

void VKRHI::RecordCommandBuffer(uint32_t index, VkCommandBuffer commandBuffer) {
    /* start command buffers record. */
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    {
        /* start render pass. */
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_Swapchain->GetRenderPassHandle();
        renderPassBeginInfo.framebuffer = m_Swapchain->GetFramebuffer(index);
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = m_Swapchain->GetExtent2D();

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            /* bind graphics pipeline. */
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              m_VKRHIPipeline->GetPipelineHandle());
            /* draw call */
            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        }
        /* end render pass */
        vkCmdEndRenderPass(commandBuffer);
    }
    /* end command buffer record. */
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void VKRHI::BeginRender() {

}

void VKRHI::Draw() {
    uint32_t imageIndex;
    m_Swapchain->AcquireNextImage(m_ImageAvailableSemaphore, &imageIndex);

    auto commandBuffer = m_CommandBuffers[imageIndex];
    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    RecordCommandBuffer(imageIndex, commandBuffer);

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    /* submit command buffer */
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_VKRHIDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { m_Swapchain->GetSwapchainKHRHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(m_VKRHIDevice->GetPresentQueue(), &presentInfo);
    vkQueueWaitIdle(m_VKRHIDevice->GetPresentQueue());
}

void VKRHI::EndRender() {

}