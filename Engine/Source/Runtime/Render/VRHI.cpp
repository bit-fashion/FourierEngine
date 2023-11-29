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
#include "VRHI.h"

#include "Window/VRRTwindow.h"
#include "Utils/IOUtils.h"

#define VRRT_SHADER_MODULE_OF_VERTEX_BINARY_FILE "../Engine/Binaries/simple_shader.vert.spv"
#define VRRT_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE "../Engine/Binaries/simple_shader.frag.spv"

/* Get required instance extensions for vulkan. */
void VRHIGetRequiredInstanceExtensions(std::vector<const char *> &vec,
                                          std::unordered_map<std::string, VkExtensionProperties> &supports) {
    /* glfw */
    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (int i = 0; i < glfwExtensionCount; i++)
        vec.push_back(glfwExtensions[i]);
}

/* Get required instance layers for vulkan. */
void VRHIGetRequiredInstanceLayers(std::vector<const char *> &vec,
                                      std::unordered_map<std::string, VkLayerProperties> &supports) {
#ifdef VRRT_ENGINE_CONFIG_ENABLE_DEBUG
    if (supports.count(VK_LAYER_KHRONOS_validation) != 0)
        vec.push_back(VK_LAYER_KHRONOS_validation);
#endif
}

/**
 * 获取设备必须启用的扩展列表
 */
static void VRHIGetRequiredEnableDeviceExtensions(std::unordered_map<std::string, VkExtensionProperties> &supports,
                                                 std::vector<const char *> *pRequired) {
    if (supports.count(VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
        pRequired->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VRHIQueueFamilyIndices VRHIdevice::FindQueueFamilyIndices() {
    VRHIQueueFamilyIndices queueFamilyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mVRHIGPU.device, &queueFamilyCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mVRHIGPU.device, &queueFamilyCount, std::data(queueFamilies));

    int index = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices.graphicsQueueFamily = index;

        VkBool32 isPresentMode = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(mVRHIGPU.device, index, mSurfaceKHR, &isPresentMode);
        if (queueFamilyCount > 0 && isPresentMode)
            queueFamilyIndices.presentQueueFamily= index;

        if (queueFamilyIndices.graphicsQueueFamily > 0 && queueFamilyIndices.presentQueueFamily > 0)
            break;

        ++index;
    }

    return queueFamilyIndices;
}

/* Query swapchain details. */
VRHISwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VRHISwapChainSupportDetails details;

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

uint32_t FindMemoryType(uint32_t typeFilter, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

// ----------------------------------------------------------------------------
// Pipeline
// ----------------------------------------------------------------------------

VRHIpipeline::VRHIpipeline(VRHIdevice *device, VRHIswapchain *swapchain, const char *vertex_shader_path, const char *fragment_shader_path)
  : mVRHIdevice(device), mSwapchain(swapchain){
    /** Create shader of vertex & fragment module. */
    VRRT_LOGGER_INFO("Loading and create vertex shader module from: {}", vertex_shader_path);
    VkShaderModule vertex_shader_module = LoadShaderModule(mVRHIdevice->GetDeviceHandle(), vertex_shader_path);
    VRRT_LOGGER_INFO("Loading and create vertex shader module success!");

    VRRT_LOGGER_INFO("Loading and create fragment shader module from: {}", fragment_shader_path);
    VkShaderModule fragment_shader_module = LoadShaderModule(mVRHIdevice->GetDeviceHandle(), fragment_shader_path);
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

    vkVRRTCreate(PipelineLayout, mVRHIdevice->GetDeviceHandle(), &pipelineLayoutInfo, nullptr, &mPipelineLayout);

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
    graphicsPipelineCreateInfo.layout = mPipelineLayout;
    graphicsPipelineCreateInfo.renderPass = mSwapchain->GetRenderPass();
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    graphicsPipelineCreateInfo.basePipelineIndex = -1; // Optional

    vkVRRTCreate(GraphicsPipelines, mVRHIdevice->GetDeviceHandle(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, VK_NULL_HANDLE, &mPipeline);

    /* 销毁着色器模块 */
    vkDestroyShaderModule(mVRHIdevice->GetDeviceHandle(), vertex_shader_module, VK_NULL_HANDLE);
    vkDestroyShaderModule(mVRHIdevice->GetDeviceHandle(), fragment_shader_module, VK_NULL_HANDLE);
}

VRHIpipeline::~VRHIpipeline() {
    vkDestroyPipelineLayout(mVRHIdevice->GetDeviceHandle(), mPipelineLayout, VK_NULL_HANDLE);
    vkDestroyPipeline(mVRHIdevice->GetDeviceHandle(), mPipeline, VK_NULL_HANDLE);
}

// ----------------------------------------------------------------------------
// Swapchain
// ----------------------------------------------------------------------------

VRHIswapchain::VRHIswapchain(VRHIdevice *device, VRRTwindow *pVRRTwindow, VkSurfaceKHR surface)
  : mVRHIdevice(device), mVRRTwindow(pVRRTwindow), mSurface(surface) {
    /* 查询交换链支持 */
    mSwapChainDetails = QuerySwapChainSupportDetails(mVRHIdevice->GetPhysicalDeviceHandle(), mSurface);

    mSurfaceFormatKHR = SelectSwapSurfaceFormat(mSwapChainDetails.formats);
    mSwapchainFormat = mSurfaceFormatKHR.format;
    mSwapchainPresentModeKHR = SelectSwapSurfacePresentMode(mSwapChainDetails.presentModes);
    mSwapchainExtent = SelectSwapExtent(mSwapChainDetails.capabilities, mVRRTwindow);

    /** Create render pass. */
    VkAttachmentDescription colorAttachmentDescription = {};
    colorAttachmentDescription.format = mSwapchainFormat;
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

    vkVRRTCreate(RenderPass, mVRHIdevice->GetDeviceHandle(), &renderPassCreateInfo, VK_NULL_HANDLE, &mRenderPass);

    /* 创建交换链 */
    CreateSwapchain();
}

VRHIswapchain::~VRHIswapchain() {
    CleanupSwapchain();
}

VkResult VRHIswapchain::AcquireNextImage(VkSemaphore semaphore, uint32_t *pIndex) {
    return vkAcquireNextImageKHR(mVRHIdevice->GetDeviceHandle(), mSwapchain, std::numeric_limits<uint64_t>::max(),
                          semaphore, VK_NULL_HANDLE, pIndex);
}

void VRHIswapchain::CreateSwapchain() {
    /* 设置三重缓冲 */
    mSwapchainImageCount = mSwapChainDetails.capabilities.minImageCount + 1;
    if (mSwapChainDetails.capabilities.maxImageCount > 0 && mSwapchainImageCount > mSwapChainDetails.capabilities.maxImageCount)
        mSwapchainImageCount = mSwapChainDetails.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfoKhr = {};
    swapchainCreateInfoKhr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKhr.surface = mSurface;
    swapchainCreateInfoKhr.minImageCount = mSwapchainImageCount;
    swapchainCreateInfoKhr.imageFormat = mSurfaceFormatKHR.format;
    swapchainCreateInfoKhr.imageColorSpace = mSurfaceFormatKHR.colorSpace;
    swapchainCreateInfoKhr.imageExtent = mSwapchainExtent;
    swapchainCreateInfoKhr.imageArrayLayers = 1;
    swapchainCreateInfoKhr.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKhr.preTransform = mSwapChainDetails.capabilities.currentTransform;
    swapchainCreateInfoKhr.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKhr.presentMode = mSwapchainPresentModeKHR;
    swapchainCreateInfoKhr.clipped = VK_TRUE;
    swapchainCreateInfoKhr.oldSwapchain = VK_NULL_HANDLE;
    vkVRRTCreate(SwapchainKHR, mVRHIdevice->GetDeviceHandle(), &swapchainCreateInfoKhr, VK_NULL_HANDLE, &mSwapchain);

    vkGetSwapchainImagesKHR(mVRHIdevice->GetDeviceHandle(), mSwapchain, &mSwapchainImageCount, VK_NULL_HANDLE);
    mSwapchainImages.resize(mSwapchainImageCount);
    vkGetSwapchainImagesKHR(mVRHIdevice->GetDeviceHandle(), mSwapchain, &mSwapchainImageCount, std::data(mSwapchainImages));

    /** Create image views. */
    mSwapchainImageViews.resize(mSwapchainImageCount);
    for (uint32_t i = 0; i < mSwapchainImageCount; i++) {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = mSwapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = mSurfaceFormatKHR.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        vkVRRTCreate(ImageView, mVRHIdevice->GetDeviceHandle(), &imageViewCreateInfo, VK_NULL_HANDLE, &mSwapchainImageViews[i]);
    }

    /* 帧缓冲区 */
    mSwapchainFramebuffers.resize(mSwapchainImageCount);
    for (size_t i = 0; i < mSwapchainImageCount; i++) {
        VkImageView attachments[] = { mSwapchainImageViews[i] };

        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = mRenderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
        framebufferCreateInfo.width = mSwapchainExtent.width;
        framebufferCreateInfo.height = mSwapchainExtent.height;
        framebufferCreateInfo.layers = 1;

        vkVRRTCreate(Framebuffer, mVRHIdevice->GetDeviceHandle(), &framebufferCreateInfo, nullptr, &mSwapchainFramebuffers[i]);
    }
}

void VRHIswapchain::CleanupSwapchain() {
    vkDestroyRenderPass(mVRHIdevice->GetDeviceHandle(), mRenderPass, VK_NULL_HANDLE);
    for (const auto &imageView: mSwapchainImageViews)
        vkDestroyImageView(mVRHIdevice->GetDeviceHandle(), imageView, VK_NULL_HANDLE);
    for (const auto &framebuffer: mSwapchainFramebuffers)
        vkDestroyFramebuffer(mVRHIdevice->GetDeviceHandle(), framebuffer, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(mVRHIdevice->GetDeviceHandle(), mSwapchain, VK_NULL_HANDLE);
}

// ----------------------------------------------------------------------------
// Device
// ----------------------------------------------------------------------------

VRHIdevice::VRHIdevice(VkInstance instance, VkSurfaceKHR surface, VRRTwindow *pVRRTwindow)
  : mInstance(instance), mSurfaceKHR(surface), mVRRTwindow(pVRRTwindow) {
    /* 选择一个牛逼的 GPU 设备 */
    std::vector<VRHIGPU> vGPU;
    VRHIGETGPU(mInstance, &vGPU);
    mVRHIGPU = vGPU[0];

    /* 获取设备支持的所有扩展列表 */
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(mVRHIGPU.device, VK_NULL_HANDLE, &deviceExtensionCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(mVRHIGPU.device, VK_NULL_HANDLE, &deviceExtensionCount, std::data(deviceExtensions));
    for (auto &extension : deviceExtensions)
        mDeviceSupportedExtensions.insert({extension.extensionName, extension});

    /* 查询设备支持的队列 */
    mVRHIQueueFamilyIndices = FindQueueFamilyIndices();
    std::vector<uint32_t> uniqueQueueFamilies = {mVRHIQueueFamilyIndices.graphicsQueueFamily, mVRHIQueueFamilyIndices.presentQueueFamily};
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
    VRHIGetRequiredEnableDeviceExtensions(mDeviceSupportedExtensions, &vRequiredEnableExtension);
    deviceCreateInfo.enabledExtensionCount = std::size(vRequiredEnableExtension);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(vRequiredEnableExtension);
    vkVRRTCreate(Device, mVRHIGPU.device, &deviceCreateInfo, nullptr, &mDevice);

    /* 获取队列 */
    vkGetDeviceQueue(mDevice, mVRHIQueueFamilyIndices.graphicsQueueFamily, 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, mVRHIQueueFamilyIndices.presentQueueFamily, 0, &mPresentQueue);

    /* init */
    InitAllocateDescriptorSet();
    InitCommandPool();
}

VRHIdevice::~VRHIdevice() {
    vkDestroyDescriptorPool(mDevice, mDescriptorPool, VK_NULL_HANDLE);
    vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, VK_NULL_HANDLE);
    vkDestroyCommandPool(mDevice, mCommandPool, VK_NULL_HANDLE);
    vkDestroyDevice(mDevice, VK_NULL_HANDLE);
}

void VRHIdevice::CreateSwapchain(VRHIswapchain **pSwapchain) {
    *pSwapchain = new VRHIswapchain(this, mVRRTwindow, mSurfaceKHR);
}

void VRHIdevice::DestroySwapchain(VRHIswapchain *swapchain) {
    delete swapchain;
}

void VRHIdevice::AllocateDescriptorSet(uint32_t count, VkDescriptorSet *pDescriptorSet) {
    /** Allocate descriptor set */
    VkDescriptorSetLayout descriptorSetLayouts[] = {mDescriptorSetLayout};
    VkDescriptorSetAllocateInfo descriptorAllocateInfo = {};
    descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocateInfo.descriptorPool = mDescriptorPool;
    descriptorAllocateInfo.descriptorSetCount = count;
    descriptorAllocateInfo.pSetLayouts = descriptorSetLayouts;
    vkAllocateDescriptorSets(mDevice, &descriptorAllocateInfo, pDescriptorSet);
}

void VRHIdevice::FreeDescriptorSet(uint32_t count, VkDescriptorSet *pDescriptorSet) {
    vkFreeDescriptorSets(mDevice, mDescriptorPool, count, pDescriptorSet);
}

void VRHIdevice::AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer) {
    /** Allocate command buffer. */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = mCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t) count;

    vkVRRTAllocate(CommandBuffers, mDevice, &commandBufferAllocateInfo, pCommandBuffer);
}

void VRHIdevice::FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer) {
    vkFreeCommandBuffers(mDevice, mCommandPool, count, pCommandBuffer);
}

void VRHIdevice::CreateSemaphore(VkSemaphore *pSemaphore) {
    /** Create semaphores. */
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkVRRTCreate(Semaphore, mDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, pSemaphore);
}

void VRHIdevice::DestroySemaphore(VkSemaphore semaphore) {
    vkDestroySemaphore(mDevice, semaphore, VK_NULL_HANDLE);
}

void VRHIdevice::AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VRHIbuffer *buffer) {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkVRRTCreate(Buffer, mDevice, &bufferCreateInfo, VK_NULL_HANDLE, &buffer->buffer);

    /** Query memory requirements. */
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(mDevice, buffer->buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, mVRHIGPU.device,properties);

    vkVRRTAllocate(Memory, mDevice, &memoryAllocInfo, VK_NULL_HANDLE, &buffer->memory);
    vkBindBufferMemory(mDevice, buffer->buffer, buffer->memory, 0);
}

void VRHIdevice::FreeBuffer(VRHIbuffer buffer) {
    vkFreeMemory(mDevice, buffer.memory, VK_NULL_HANDLE);
    vkDestroyBuffer(mDevice, buffer.buffer, VK_NULL_HANDLE);
}

void VRHIdevice::MapMemory(VRHIbuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData) {
    vkMapMemory(mDevice, buffer.memory, offset, size, flags, ppData);
}

void VRHIdevice::UnmapMemory(VRHIbuffer buffer) {
    vkUnmapMemory(mDevice, buffer.memory);
}

void VRHIdevice::WaitIdle() {
    vkDeviceWaitIdle(mDevice);
}

void VRHIdevice::InitAllocateDescriptorSet() {
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

    vkVRRTCreate(DescriptorSetLayout, mDevice, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, &mDescriptorSetLayout);

    /** Create descriptor set pool */
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo descriptorPoolCrateInfo = {};
    descriptorPoolCrateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCrateInfo.poolSizeCount = 1;
    descriptorPoolCrateInfo.pPoolSizes = &poolSize;
    descriptorPoolCrateInfo.maxSets = 1;

    vkVRRTCreate(DescriptorPool, mDevice, &descriptorPoolCrateInfo, VK_NULL_HANDLE, &mDescriptorPool);
}

void VRHIdevice::InitCommandPool() {
    /** Create command pool. */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = mVRHIQueueFamilyIndices.graphicsQueueFamily;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkVRRTCreate(CommandPool, mDevice, &commandPoolCreateInfo, VK_NULL_HANDLE, &mCommandPool);
}

// ----------------------------------------------------------------------------
// VRHI
// ----------------------------------------------------------------------------

VRHI::VRHI(VRRTwindow *pVRRTwindow) : mVRRTwindow(pVRRTwindow) {
    /* Enumerate instance available extensions. */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, std::data(extensions));
    VRRT_LOGGER_INFO("Vulkan render api available extensions for instance:");
    for (auto &extension : extensions) {
        VRRT_LOGGER_INFO("    {}", extension.extensionName);
        mVkInstanceExtensionPropertiesSupports.insert({extension.extensionName, extension});
    }

    /* Enumerate instance available layers. */
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(layers));
    VRRT_LOGGER_INFO("Vulkan render api available layer list: ");
    for (auto &layer : layers) {
        VRRT_LOGGER_INFO("    {}", layer.layerName);
        mVkInstanceLayerPropertiesSupports.insert({layer.layerName, layer});
    }

    /* Get extensions & layers. */
    VRHIGetRequiredInstanceExtensions(mRequiredInstanceExtensions, mVkInstanceExtensionPropertiesSupports);
    VRHIGetRequiredInstanceLayers(mRequiredInstanceLayers, mVkInstanceLayerPropertiesSupports);

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

    instanceCreateInfo.enabledExtensionCount = std::size(mRequiredInstanceExtensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(mRequiredInstanceExtensions);
    VRRT_LOGGER_INFO("Used vulkan extension for instance count: {}", std::size(mRequiredInstanceExtensions));
    for (const auto& name : mRequiredInstanceExtensions)
        VRRT_LOGGER_INFO("    {}", name);

    instanceCreateInfo.enabledLayerCount = std::size(mRequiredInstanceLayers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(mRequiredInstanceLayers);

    vkVRRTCreate(Instance, &instanceCreateInfo, VK_NULL_HANDLE, &mInstance);

    /** 创建 Surface 接口对象 */
    if (glfwCreateWindowSurface(mInstance, pVRRTwindow->GetWindowHandle(),VK_NULL_HANDLE,
                                &mSurface) != VK_SUCCESS) {
        VRRT_LOGGER_INFO("Create glfw surface failed!");
    }

    /* init */
    Init_Vulkan_Impl();
}

VRHI::~VRHI() {
    mVRHIdevice->DestroySemaphore(mImageAvailableSemaphore);
    mVRHIdevice->DestroySemaphore(mRenderFinishedSemaphore);
    CleanupSwapchain();
    VRRT_FREE_POINTER(mVRHIdevice);
    vkDestroySurfaceKHR(mInstance, mSurface, VK_NULL_HANDLE);
    vkDestroyInstance(mInstance, VK_NULL_HANDLE);
}

void VRHI::Init_Vulkan_Impl() {
    mVRHIdevice = std::make_unique<VRHIdevice>(mInstance, mSurface, mVRRTwindow);
    CreateSwapchain();
    mCommandBuffers.resize(mSwapchain->GetImageCount());
    mVRHIdevice->AllocateCommandBuffer(std::size(mCommandBuffers), std::data(mCommandBuffers));
    mVRHIdevice->CreateSemaphore(&mImageAvailableSemaphore);
    mVRHIdevice->CreateSemaphore(&mRenderFinishedSemaphore);
    /* 设置监听窗口变化回调 */
    mVRRTwindow->SetWindowUserPointer(this);
    mVRRTwindow->SetVRRTwindowResizableWindowCallback([](VRRTwindow *pVRRTwindow, int width, int height) {
        VRHI *pVRHI = (VRHI *) pVRRTwindow->GetWindowUserPointer();
        pVRHI->RecreateSwapchain();
    });

}

void VRHI::CleanupSwapchain() {
    VRRT_FREE_POINTER(mVRHIpipeline);
    mVRHIdevice->DestroySwapchain(mSwapchain);
}

void VRHI::CreateSwapchain() {
    mVRHIdevice->CreateSwapchain(&mSwapchain);
    mVRHIpipeline = std::make_unique<VRHIpipeline>(mVRHIdevice.get(), mSwapchain,
                                                   VRRT_SHADER_MODULE_OF_VERTEX_BINARY_FILE,
                                                   VRRT_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE);
}

void VRHI::RecreateSwapchain() {
    mVRHIdevice->WaitIdle();
    CleanupSwapchain();
    CreateSwapchain();
    mVRHIdevice->FreeCommandBuffer(std::size(mCommandBuffers), std::data(mCommandBuffers));
    mVRHIdevice->AllocateCommandBuffer(std::size(mCommandBuffers), std::data(mCommandBuffers));
}

void VRHI::BeginRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index) {
    mCurrentContextCommandBuffer = commandBuffer;
    mCurrentContextImageIndex = index;
    /* start command buffers record. */
    vkResetCommandBuffer(mCurrentContextCommandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional
    vkBeginCommandBuffer(mCurrentContextCommandBuffer, &commandBufferBeginInfo);
}

void VRHI::EndRecordCommandBuffer() {
    /* end command buffer record. */
    if (vkEndCommandBuffer(mCurrentContextCommandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void VRHI::BeginRenderPass(VkRenderPass renderPass) {
    mCurrentContextRenderPass = renderPass;
    /* start render pass. */
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = mCurrentContextRenderPass;
    renderPassBeginInfo.framebuffer = mSwapchain->GetFramebuffer(mCurrentContextImageIndex);
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = mSwapchain->GetExtent2D();

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(mCurrentContextCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    {
        /* bind graphics pipeline. */
        vkCmdBindPipeline(mCurrentContextCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mVRHIpipeline->GetPipeline());
        /* draw call */
        vkCmdDraw(mCurrentContextCommandBuffer, 3, 1, 0, 0);
    }
}

void VRHI::EndRenderPass() {
    /* end render pass */
    vkCmdEndRenderPass(mCurrentContextCommandBuffer);
}

void VRHI::BeginRender() {
    uint32_t index;
    mSwapchain->AcquireNextImage(mImageAvailableSemaphore, &index);

    BeginRecordCommandBuffer(mCommandBuffers[index], index);
    BeginRenderPass(mSwapchain->GetRenderPass());
}

void VRHI::Draw() {
    /* bind graphics pipeline. */
    vkCmdBindPipeline(mCurrentContextCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      mVRHIpipeline->GetPipeline());
    /* draw call */
    vkCmdDraw(mCurrentContextCommandBuffer, 3, 1, 0, 0);
}

void VRHI::EndRender() {
    EndRenderPass();
    EndRecordCommandBuffer();

    VkSemaphore waitSemaphores[] = { mImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    /* submit command buffer */
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &mCurrentContextCommandBuffer;

    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(mVRHIdevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { mSwapchain->GetSwapchainKHRHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &mCurrentContextImageIndex;
    presentInfo.pResults = nullptr; // Optional

    vkQueuePresentKHR(mVRHIdevice->GetPresentQueue(), &presentInfo);
    vkQueueWaitIdle(mVRHIdevice->GetPresentQueue());
}