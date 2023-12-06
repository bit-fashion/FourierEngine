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
#include "Render/Vulkan/VulkanRenderer.h"

#include "Window/NatureWindow.h"
#include "Utils/IOUtils.h"

// stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define NATURE_SHADER_MODULE_OF_VERTEX_BINARY_FILE "../Engine/Binaries/simple_shader.vert.spv"
#define NATURE_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE "../Engine/Binaries/simple_shader.frag.spv"

/* Get required instance extensions for vulkan. */
void GetRequiredInstanceExtensions(std::vector<const char *> &vec,
                                          std::unordered_map<std::string, VkExtensionProperties> &supports) {
    /* glfw */
    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (int i = 0; i < glfwExtensionCount; i++)
        vec.push_back(glfwExtensions[i]);
}

/* Get required instance layers for vulkan. */
void GetRequiredInstanceLayers(std::vector<const char *> &vec,
                                      std::unordered_map<std::string, VkLayerProperties> &supports) {
#ifdef NATURE_ENGINE_CONFIG_ENABLE_DEBUG
    if (supports.count(VK_LAYER_KHRONOS_validation) != 0)
        vec.push_back(VK_LAYER_KHRONOS_validation);
#endif
}

/**
 * 获取设备必须启用的扩展列表
 */
static void GetRequiredEnableDeviceExtensions(std::unordered_map<std::string, VkExtensionProperties> &supports,
                                                 std::vector<const char *> *pRequired) {
    if (supports.count(VK_KHR_SWAPCHAIN_EXTENSION_NAME) != 0)
        pRequired->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

QueueFamilyIndices VulkanDevice::FindQueueFamilyIndices() {
    QueueFamilyIndices queueFamilyIndices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mVulkanPhysicalDevice.device, &queueFamilyCount, VK_NULL_HANDLE);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mVulkanPhysicalDevice.device, &queueFamilyCount, std::data(queueFamilies));

    int index = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices.graphicsQueueFamily = index;

        VkBool32 isPresentMode = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(mVulkanPhysicalDevice.device, index, mSurfaceKHR, &isPresentMode);
        if (queueFamilyCount > 0 && isPresentMode)
            queueFamilyIndices.presentQueueFamily= index;

        if (queueFamilyIndices.graphicsQueueFamily > 0 && queueFamilyIndices.presentQueueFamily > 0)
            break;

        ++index;
    }

    return queueFamilyIndices;
}

/* Query swapchain details. */
SwapChainSupportDetails QuerySwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

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
VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, NatureWindow *pRIVwindow) {
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
    vkNatureCreate(ShaderModule, device, &shaderModuleCreateInfo, VK_NULL_HANDLE, &shader);

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

/** ---------------------------------------------------------------------------- */
// ====
/**
 * Pipeline
 */
// ====
/** ---------------------------------------------------------------------------- */

VulkanPipeline::VulkanPipeline(VulkanDevice *device, VulkanSwapchainKHR *swapchain, const char *vertex_shader_path, const char *fragment_shader_path)
  : mVulkanDevice(device), mSwapchain(swapchain){
    /* init */
    Init_Graphics_Pipeline();
    /** Create shader of vertex & fragment module. */
    NATURE_LOGGER_INFO("Loading and create vertex shader module from: {}", vertex_shader_path);
    VkShaderModule vertex_shader_module = LoadShaderModule(mVulkanDevice->GetDevice(), vertex_shader_path);
    NATURE_LOGGER_INFO("Loading and create vertex shader module success!");

    NATURE_LOGGER_INFO("Loading and create fragment shader module from: {}", fragment_shader_path);
    VkShaderModule fragment_shader_module = LoadShaderModule(mVulkanDevice->GetDevice(), fragment_shader_path);
    NATURE_LOGGER_INFO("Loading and create fragment shader module success!");

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

    VkVertexInputBindingDescription vertexInputBindingDescription = VulkanPipeline::GetVertexInputBindingDescription();
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;

    auto vertexInputAttributeDescriptions = VulkanPipeline::GetVertexInputAttributeDescriptionArray();
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = std::size(vertexInputAttributeDescriptions);
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = std::data(vertexInputAttributeDescriptions);

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
    pipelineLayoutInfo.pSetLayouts = &mUboDescriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    vkNatureCreate(PipelineLayout, mVulkanDevice->GetDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout);

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

    vkNatureCreate(GraphicsPipelines, mVulkanDevice->GetDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, VK_NULL_HANDLE, &mPipeline);

    /* 销毁着色器模块 */
    vkDestroyShaderModule(mVulkanDevice->GetDevice(), vertex_shader_module, VK_NULL_HANDLE);
    vkDestroyShaderModule(mVulkanDevice->GetDevice(), fragment_shader_module, VK_NULL_HANDLE);
}

VulkanPipeline::~VulkanPipeline() {
    mVulkanDevice->FreeDescriptorSets(1, &mUboDescriptorSet);
    mVulkanDevice->DestroyDescriptorSetLayout(mUboDescriptorSetLayout);
    vkDestroyPipelineLayout(mVulkanDevice->GetDevice(), mPipelineLayout, VK_NULL_HANDLE);
    vkDestroyPipeline(mVulkanDevice->GetDevice(), mPipeline, VK_NULL_HANDLE);
}

void VulkanPipeline::Bind(VkCommandBuffer commandBuffer) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            mPipelineLayout, 0, 1, &mUboDescriptorSet, 0, nullptr);
}

void VulkanPipeline::Write(VkDeviceSize offset, VkDeviceSize range, VulkanBuffer buffer, VulkanTexture2D texture) {
    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = buffer.buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = mUboDescriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;
    descriptorWrites[0].pImageInfo = nullptr; // Optional
    descriptorWrites[0].pTexelBufferView = nullptr; // Optional

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.imageView;
    imageInfo.sampler = texture.sampler;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = mUboDescriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = nullptr;
    descriptorWrites[1].pImageInfo = &imageInfo;
    descriptorWrites[1].pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(mVulkanDevice->GetDevice(), std::size(descriptorWrites), std::data(descriptorWrites), 0, nullptr);
}

void VulkanPipeline::Init_Graphics_Pipeline() {
    std::vector<VkDescriptorSetLayoutBinding> uboDescriptorSetLayoutBinding = {
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, VK_NULL_HANDLE },
            { 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE },
    };
    mVulkanDevice->CreateDescriptorSetLayout(uboDescriptorSetLayoutBinding, 0, &mUboDescriptorSetLayout);

    std::vector<VkDescriptorSetLayout> layous = {mUboDescriptorSetLayout};
    mVulkanDevice->AllocateDescriptorSet(layous, &mUboDescriptorSet);
}

/** ---------------------------------------------------------------------------- */
// ====
/**
 * Swapchain
 */
// ====
/** ---------------------------------------------------------------------------- */

VulkanSwapchainKHR::VulkanSwapchainKHR(VulkanDevice *device, NatureWindow *pNatureWindow, VkSurfaceKHR surface)
  : mVulkanDevice(device), mNatureWindow(pNatureWindow), mSurface(surface) {
    /* 查询交换链支持 */
    mSwapChainDetails = QuerySwapChainSupportDetails(mVulkanDevice->GetPhysicalDevice(), mSurface);

    mSurfaceFormatKHR = SelectSwapSurfaceFormat(mSwapChainDetails.formats);
    mSwapchainFormat = mSurfaceFormatKHR.format;
    mSwapchainPresentModeKHR = SelectSwapSurfacePresentMode(mSwapChainDetails.presentModes);
    mSwapchainExtent = SelectSwapExtent(mSwapChainDetails.capabilities, mNatureWindow);

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

    vkNatureCreate(RenderPass, mVulkanDevice->GetDevice(), &renderPassCreateInfo, VK_NULL_HANDLE, &mRenderPass);

    /* 创建交换链 */
    CreateSwapchain();
}

VulkanSwapchainKHR::~VulkanSwapchainKHR() {
    CleanupSwapchain();
}

VkResult VulkanSwapchainKHR::AcquireNextImage(VkSemaphore semaphore, uint32_t *pIndex) {
    return vkAcquireNextImageKHR(mVulkanDevice->GetDevice(), mSwapchain, std::numeric_limits<uint64_t>::max(),
                          semaphore, VK_NULL_HANDLE, pIndex);
}

void VulkanSwapchainKHR::CreateSwapchain() {
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
    vkNatureCreate(SwapchainKHR, mVulkanDevice->GetDevice(), &swapchainCreateInfoKhr, VK_NULL_HANDLE, &mSwapchain);

    vkGetSwapchainImagesKHR(mVulkanDevice->GetDevice(), mSwapchain, &mSwapchainImageCount, VK_NULL_HANDLE);
    mSwapchainImages.resize(mSwapchainImageCount);
    vkGetSwapchainImagesKHR(mVulkanDevice->GetDevice(), mSwapchain, &mSwapchainImageCount, std::data(mSwapchainImages));

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
        vkNatureCreate(ImageView, mVulkanDevice->GetDevice(), &imageViewCreateInfo, VK_NULL_HANDLE, &mSwapchainImageViews[i]);
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

        vkNatureCreate(Framebuffer, mVulkanDevice->GetDevice(), &framebufferCreateInfo, nullptr, &mSwapchainFramebuffers[i]);
    }
}

void VulkanSwapchainKHR::CleanupSwapchain() {
    vkDestroyRenderPass(mVulkanDevice->GetDevice(), mRenderPass, VK_NULL_HANDLE);
    for (const auto &imageView: mSwapchainImageViews)
        vkDestroyImageView(mVulkanDevice->GetDevice(), imageView, VK_NULL_HANDLE);
    for (const auto &framebuffer: mSwapchainFramebuffers)
        vkDestroyFramebuffer(mVulkanDevice->GetDevice(), framebuffer, VK_NULL_HANDLE);
    vkDestroySwapchainKHR(mVulkanDevice->GetDevice(), mSwapchain, VK_NULL_HANDLE);
}

/** ---------------------------------------------------------------------------- */
// ====
/**
 * Device
 */
// ====
/** ---------------------------------------------------------------------------- */

VulkanDevice::VulkanDevice(VkInstance instance, VkSurfaceKHR surface, NatureWindow *pNatureWindow)
  : mInstance(instance), mSurfaceKHR(surface), mNatureWindow(pNatureWindow) {
    /* 选择一个牛逼的 GPU 设备 */
    std::vector<VulkanPhysicalDevice> vGPU;
    EnumerateVulkanPhysicalDevice(mInstance, &vGPU);
    mVulkanPhysicalDevice = vGPU[0];

    /* 获取设备支持的所有扩展列表 */
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(mVulkanPhysicalDevice.device, VK_NULL_HANDLE, &deviceExtensionCount, VK_NULL_HANDLE);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(mVulkanPhysicalDevice.device, VK_NULL_HANDLE, &deviceExtensionCount, std::data(deviceExtensions));
    for (auto &extension : deviceExtensions)
        mDeviceSupportedExtensions.insert({extension.extensionName, extension});

    /* 查询设备支持的队列 */
    mQueueFamilyIndices = FindQueueFamilyIndices();
    std::vector<uint32_t> uniqueQueueFamilies = {mQueueFamilyIndices.graphicsQueueFamily, mQueueFamilyIndices.presentQueueFamily};
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
    GetRequiredEnableDeviceExtensions(mDeviceSupportedExtensions, &vRequiredEnableExtension);
    deviceCreateInfo.enabledExtensionCount = std::size(vRequiredEnableExtension);
    deviceCreateInfo.ppEnabledExtensionNames = std::data(vRequiredEnableExtension);
    vkNatureCreate(Device, mVulkanPhysicalDevice.device, &deviceCreateInfo, nullptr, &mDevice);

    /* 获取队列 */
    vkGetDeviceQueue(mDevice, mQueueFamilyIndices.graphicsQueueFamily, 0, &mGraphicsQueue);
    vkGetDeviceQueue(mDevice, mQueueFamilyIndices.presentQueueFamily, 0, &mPresentQueue);

    /* init */
    InitAllocateDescriptorSetPool();
    InitCommandPool();
}

VulkanDevice::~VulkanDevice() {
    vkDestroyDescriptorPool(mDevice, mDescriptorPool, VK_NULL_HANDLE);
    vkDestroyCommandPool(mDevice, mCommandPool, VK_NULL_HANDLE);
    vkDestroyDevice(mDevice, VK_NULL_HANDLE);
}

void VulkanDevice::CreateSwapchain(VulkanSwapchainKHR **pSwapchain) {
    *pSwapchain = new VulkanSwapchainKHR(this, mNatureWindow, mSurfaceKHR);
}

void VulkanDevice::DestroySwapchain(VulkanSwapchainKHR *swapchain) {
    delete swapchain;
}

void VulkanDevice::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags,
                                           VkDescriptorSetLayout *pDescriptorSetLayout) {
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.flags = flags;
    descriptorSetLayoutCreateInfo.bindingCount = std::size(bindings);
    descriptorSetLayoutCreateInfo.pBindings = std::data(bindings);
    vkNatureCreate(DescriptorSetLayout, mDevice, &descriptorSetLayoutCreateInfo, VK_NULL_HANDLE, pDescriptorSetLayout);
}

void VulkanDevice::DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) {
    vkDestroyDescriptorSetLayout(mDevice, descriptorSetLayout, VK_NULL_HANDLE);
}

void VulkanDevice::AllocateDescriptorSet(std::vector<VkDescriptorSetLayout> &descriptorSetLayouts, VkDescriptorSet *pDescriptorSet) {
    /** Allocate descriptor set */
    VkDescriptorSetAllocateInfo descriptorAllocateInfo = {};
    descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocateInfo.descriptorPool = mDescriptorPool;
    descriptorAllocateInfo.descriptorSetCount = std::size(descriptorSetLayouts);
    descriptorAllocateInfo.pSetLayouts = std::data(descriptorSetLayouts);
    vkAllocateDescriptorSets(mDevice, &descriptorAllocateInfo, pDescriptorSet);
}

void VulkanDevice::FreeDescriptorSets(uint32_t count, VkDescriptorSet *pDescriptorSet) {
    vkFreeDescriptorSets(mDevice, mDescriptorPool, count, pDescriptorSet);
}

void VulkanDevice::AllocateCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer) {
    /** Allocate command buffer. */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = mCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t) count;

    vkNatureAllocate(CommandBuffers, mDevice, &commandBufferAllocateInfo, pCommandBuffer);
}

void VulkanDevice::FreeCommandBuffer(uint32_t count, VkCommandBuffer *pCommandBuffer) {
    vkFreeCommandBuffers(mDevice, mCommandPool, count, pCommandBuffer);
}

void VulkanDevice::CreateSemaphore(VkSemaphore *pSemaphore) {
    /** Create semaphores. */
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkNatureCreate(Semaphore, mDevice, &semaphoreCreateInfo, VK_NULL_HANDLE, pSemaphore);
}

void VulkanDevice::DestroySemaphore(VkSemaphore semaphore) {
    vkDestroySemaphore(mDevice, semaphore, VK_NULL_HANDLE);
}

void VulkanDevice::AllocateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanBuffer *buffer) {
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    buffer->size = bufferCreateInfo.size;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkNatureCreate(Buffer, mDevice, &bufferCreateInfo, VK_NULL_HANDLE, &buffer->buffer);

    /** Query memory requirements. */
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(mDevice, buffer->buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocInfo = {};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, mVulkanPhysicalDevice.device,properties);

    vkNatureAllocate(Memory, mDevice, &memoryAllocInfo, VK_NULL_HANDLE, &buffer->memory);
    vkBindBufferMemory(mDevice, buffer->buffer, buffer->memory, 0);
}

void VulkanDevice::FreeBuffer(VulkanBuffer buffer) {
    vkFreeMemory(mDevice, buffer.memory, VK_NULL_HANDLE);
    vkDestroyBuffer(mDevice, buffer.buffer, VK_NULL_HANDLE);
}

void VulkanDevice::AllocateVertexBuffer(VkDeviceSize size, const Vertex *pVertices, VulkanBuffer *pVertexBuffer) {
    VulkanBuffer stagingBuffer;
    AllocateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer);
    void *data;
    MapMemory(stagingBuffer, 0, stagingBuffer.size, 0, &data);
    memcpy(data, pVertices, static_cast<VkDeviceSize>(stagingBuffer.size));
    UnmapMemory(stagingBuffer);
    /* vertex buffer */
    AllocateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                   VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, pVertexBuffer);
    CopyBuffer(*pVertexBuffer, stagingBuffer, size);
    FreeBuffer(stagingBuffer);
}

void VulkanDevice::CopyBuffer(VulkanBuffer dest, VulkanBuffer src, VkDeviceSize size) {
    VkCommandBuffer oneTimeCommandBuffer;
    BeginOneTimeCommandBufferSubmit(&oneTimeCommandBuffer);
    {
        /* copy buffer */
        VkBufferCopy copyRegion = {};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(oneTimeCommandBuffer, src.buffer, dest.buffer, 1, &copyRegion);
    }
    EndOneTimeCommandBufferSubmit();
}

void VulkanDevice::AllocateIndexBuffer(VkDeviceSize size, const uint32_t *pIndices, VulkanBuffer *pIndexBuffer) {
    VulkanBuffer stagingBuffer;
    AllocateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer);
    void *data;
    MapMemory(stagingBuffer, 0, stagingBuffer.size, 0, &data);
    memcpy(data, pIndices, static_cast<VkDeviceSize>(stagingBuffer.size));
    UnmapMemory(stagingBuffer);
    /* vertex buffer */
    AllocateBuffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                   VK_MEMORY_HEAP_DEVICE_LOCAL_BIT, pIndexBuffer);
    CopyBuffer(*pIndexBuffer, stagingBuffer, size);
    FreeBuffer(stagingBuffer);
}

void VulkanDevice::MapMemory(VulkanBuffer buffer, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData) {
    vkMapMemory(mDevice, buffer.memory, offset, size, flags, ppData);
}

void VulkanDevice::UnmapMemory(VulkanBuffer buffer) {
    vkUnmapMemory(mDevice, buffer.memory);
}

void VulkanDevice::DeviceWaitIdle() {
    vkDeviceWaitIdle(mDevice);
}

void VulkanDevice::QueueWaitIdle(VkQueue queue) {
    vkQueueWaitIdle(queue);
}

void VulkanDevice::BeginCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferUsageFlags usageFlags) {
    /* start command buffers record. */
    vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = usageFlags;
    commandBufferBeginInfo.pInheritanceInfo = nullptr; // Optional
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
}

void VulkanDevice::EndCommandBuffer(VkCommandBuffer commandBuffer) {
    /* end command buffer record. */
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");
}

void VulkanDevice::SyncSubmitQueueWithSubmitInfo(uint32_t commandBufferCount, VkCommandBuffer *pCommandBuffers,
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

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    vkQueueWaitIdle(mGraphicsQueue);
}

void VulkanDevice::SyncSubmitQueueWithPresentInfoKHR(const VkPresentInfoKHR *presentInfoKHR) {
    vkQueuePresentKHR(mPresentQueue, presentInfoKHR);
    QueueWaitIdle(mPresentQueue);
}

void VulkanDevice::BeginOneTimeCommandBufferSubmit(VkCommandBuffer *pCommandBuffer) {
    AllocateCommandBuffer(1, &mSingleTimeCommandBuffer);
    *pCommandBuffer = mSingleTimeCommandBuffer;
    /* begin */
    BeginCommandBuffer(mSingleTimeCommandBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

void VulkanDevice::EndOneTimeCommandBufferSubmit() {
    EndCommandBuffer(mSingleTimeCommandBuffer);
    /* submit */
    SyncSubmitQueueWithSubmitInfo(1, &mSingleTimeCommandBuffer, 0, NULL, 0, NULL, NULL);
    FreeCommandBuffer(1, &mSingleTimeCommandBuffer);
}

void VulkanDevice::CreateTexture(const char *path, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VulkanTexture2D *pTexture) {
    /* load image. */
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;
    if (!pixels)
        NATURE_THROW_ERROR("failed to load texture image!");

    VulkanBuffer stagingBuffer;
    AllocateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer);

    void *data;
    MapMemory(stagingBuffer, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    UnmapMemory(stagingBuffer);

    stbi_image_free(pixels);

    /* Create image */
    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = static_cast<uint32_t>(texWidth);
    imageCreateInfo.extent.height = static_cast<uint32_t>(texHeight);
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.flags = 0; // Optional

    pTexture->format = imageCreateInfo.format;
    pTexture->layout = imageCreateInfo.initialLayout;

    vkNatureCreate(Image, mDevice, &imageCreateInfo, VK_NULL_HANDLE, &pTexture->image);

    /* bind memory */
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(mDevice, pTexture->image, &requirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = requirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(requirements.memoryTypeBits, mVulkanPhysicalDevice.device, properties);

    vkNatureAllocate(Memory, mDevice, &allocInfo, VK_NULL_HANDLE, &pTexture->memory);

    vkBindImageMemory(mDevice, pTexture->image, pTexture->memory, 0);

    /* create image view */
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = pTexture->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vkNatureCreate(ImageView, mDevice, &viewInfo, nullptr, &pTexture->imageView);

    /* create texture  sampler */
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

    vkNatureCreate(Sampler, mDevice, &samplerInfo, VK_NULL_HANDLE, &pTexture->sampler);

    /* copy buffer to image */
    TransitionTextureLayout(pTexture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyTextureBuffer(stagingBuffer, *pTexture, texWidth, texHeight);
    TransitionTextureLayout(pTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    FreeBuffer(stagingBuffer);
}

void VulkanDevice::DestroyTexture(VulkanTexture2D texture) {
    vkDestroySampler(mDevice, texture.sampler, VK_NULL_HANDLE);
    vkDestroyImageView(mDevice, texture.imageView, VK_NULL_HANDLE);
    vkDestroyImage(mDevice, texture.image, VK_NULL_HANDLE);
    vkFreeMemory(mDevice, texture.memory, VK_NULL_HANDLE);
}

void VulkanDevice::TransitionTextureLayout(VulkanTexture2D *texture, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer;
    BeginOneTimeCommandBufferSubmit(&commandBuffer);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = texture->layout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = texture->image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (texture->layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (texture->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
    );

    EndOneTimeCommandBufferSubmit();

    texture->layout = newLayout;
}

void VulkanDevice::CopyTextureBuffer(VulkanBuffer buffer, VulkanTexture2D texture, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer;
    BeginOneTimeCommandBufferSubmit(&commandBuffer);

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
            width,
            height,
            1
    };

    vkCmdCopyBufferToImage(
            commandBuffer,
            buffer.buffer,
            texture.image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
    );

    EndOneTimeCommandBufferSubmit();
}

void VulkanDevice::InitAllocateDescriptorSetPool() {
    /** Create descriptor set pool */
    std::vector<VkDescriptorPoolSize> poolSizes = {
            {VK_DESCRIPTOR_TYPE_SAMPLER,                1024},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1024}
    };

    VkDescriptorPoolCreateInfo descriptorPoolCrateInfo = {};
    descriptorPoolCrateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCrateInfo.poolSizeCount = static_cast<uint32_t>(std::size(poolSizes));
    descriptorPoolCrateInfo.pPoolSizes = std::data(poolSizes);
    descriptorPoolCrateInfo.maxSets = 1024 * std::size(poolSizes);
    descriptorPoolCrateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    vkNatureCreate(DescriptorPool, mDevice, &descriptorPoolCrateInfo, VK_NULL_HANDLE, &mDescriptorPool);
}

void VulkanDevice::InitCommandPool() {
    /** Create command pool. */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = mQueueFamilyIndices.graphicsQueueFamily;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkNatureCreate(CommandPool, mDevice, &commandPoolCreateInfo, VK_NULL_HANDLE, &mCommandPool);
}

/** ---------------------------------------------------------------------------- */
// ====
/**
 * VulkanRenderer
 */
// ====
/** ---------------------------------------------------------------------------- */

VulkanRenderer::VulkanRenderer(NatureWindow *pNatureWindow) : mNatureWindow(pNatureWindow) {
    /* Enumerate instance available extensions. */
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, std::data(extensions));
    NATURE_LOGGER_INFO("Vulkan render api available extensions for instance:");
    for (auto &extension : extensions) {
        NATURE_LOGGER_INFO("    {}", extension.extensionName);
        mVkInstanceExtensionPropertiesSupports.insert({extension.extensionName, extension});
    }

    /* Enumerate instance available layers. */
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, std::data(layers));
    NATURE_LOGGER_INFO("Vulkan render api available layer list: ");
    for (auto &layer : layers) {
        NATURE_LOGGER_INFO("    {}", layer.layerName);
        mVkInstanceLayerPropertiesSupports.insert({layer.layerName, layer});
    }

    /* Get extensions & layers. */
    GetRequiredInstanceExtensions(mRequiredInstanceExtensions, mVkInstanceExtensionPropertiesSupports);
    GetRequiredInstanceLayers(mRequiredInstanceLayers, mVkInstanceLayerPropertiesSupports);

    /** Create vulkan instance. */
    struct VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion = VK_VERSION_1_3;
    applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = NATURE_ENGINE_NAME;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = NATURE_ENGINE_NAME;

    struct VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    instanceCreateInfo.enabledExtensionCount = std::size(mRequiredInstanceExtensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(mRequiredInstanceExtensions);
    NATURE_LOGGER_INFO("Used vulkan extension for instance count: {}", std::size(mRequiredInstanceExtensions));
    for (const auto& name : mRequiredInstanceExtensions)
        NATURE_LOGGER_INFO("    {}", name);

    instanceCreateInfo.enabledLayerCount = std::size(mRequiredInstanceLayers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(mRequiredInstanceLayers);

    vkNatureCreate(Instance, &instanceCreateInfo, VK_NULL_HANDLE, &mInstance);

    /** 创建 Surface 接口对象 */
    if (glfwCreateWindowSurface(mInstance, pNatureWindow->GetWindowHandle(),VK_NULL_HANDLE,
                                &mSurface) != VK_SUCCESS) {
        NATURE_LOGGER_INFO("Create glfw surface failed!");
    }

    /* init */
    Init_Vulkan_Impl();
}

VulkanRenderer::~VulkanRenderer() {
    mVulkanDevice->DestroyTexture(mTexture);
    mVulkanDevice->FreeBuffer(mUniformBuffer);
    mVulkanDevice->FreeBuffer(mVertexBuffer);
    mVulkanDevice->FreeBuffer(mIndexBuffer);
    mVulkanDevice->DestroySemaphore(mImageAvailableSemaphore);
    mVulkanDevice->DestroySemaphore(mRenderFinishedSemaphore);
    CleanupSwapchain();
    NATURE_FREE_POINTER(mVulkanDevice);
    vkDestroySurfaceKHR(mInstance, mSurface, VK_NULL_HANDLE);
    vkDestroyInstance(mInstance, VK_NULL_HANDLE);
}

void VulkanRenderer::Init_Vulkan_Impl() {
    mVulkanDevice = std::make_unique<VulkanDevice>(mInstance, mSurface, mNatureWindow);
    CreateSwapchain();
    mCommandBuffers.resize(mVulkanSwapchainKHR->GetImageCount());
    mVulkanDevice->AllocateCommandBuffer(std::size(mCommandBuffers), std::data(mCommandBuffers));
    mVulkanDevice->CreateSemaphore(&mImageAvailableSemaphore);
    mVulkanDevice->CreateSemaphore(&mRenderFinishedSemaphore);
    /* 设置监听窗口变化回调 */
    mNatureWindow->SetWindowUserPointer(this);
    mNatureWindow->SetEngineWindowResizableWindowCallback([](NatureWindow *pNatureWindow, int width, int height) {
        auto *pVulkanRenderer = (VulkanRenderer *) pNatureWindow->GetWindowUserPointer();
        pVulkanRenderer->RecreateSwapchain();
    });
    /* 初始化 vulkan 渲染实例上下文 */
    Init_VulkanR_Instance_Context();
    /* 创建 Vertex buffer */
    mVulkanDevice->AllocateVertexBuffer(ARRAY_TOTAL_SIZE(mVertices), std::data(mVertices), &mVertexBuffer);
    /* 创建 Index buffer */
    mVulkanDevice->AllocateIndexBuffer(ARRAY_TOTAL_SIZE(mIndices), std::data(mIndices), &mIndexBuffer);
    /* 创建 Uniform buffer */
    mVulkanDevice->AllocateBuffer(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &mUniformBuffer);
    /* 创建 Texture */
    mVulkanDevice->CreateTexture("../Engine/Resource/VulkanHomePage.png", VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mTexture);
}

void VulkanRenderer::Init_VulkanR_Instance_Context() {
    mVulkanRenderInstanceContext.Instance = mInstance;
    mVulkanRenderInstanceContext.Device = mVulkanDevice->GetDevice();
    mVulkanRenderInstanceContext.PhysicalDevice = mVulkanDevice->GetPhysicalDevice();
    mVulkanRenderInstanceContext.GraphicsQueue = mVulkanDevice->GetGraphicsQueue();
    mVulkanRenderInstanceContext.GraphicsQueueFamily = mVulkanDevice->GetGraphicsQueueFamily();
    mVulkanRenderInstanceContext.PresentQueue = mVulkanDevice->GetPresentQueue();
    mVulkanRenderInstanceContext.PresentQueueFamily = mVulkanDevice->GetPresentQueueFamily();
    mVulkanRenderInstanceContext.DescriptorPool = mVulkanDevice->GetDescriptorPool();
    mVulkanRenderInstanceContext.MinImageCount = mVulkanSwapchainKHR->GetImageCount();
    mVulkanRenderInstanceContext.RenderPass = mVulkanSwapchainKHR->GetRenderPass();
    mVulkanRenderInstanceContext.RenderContext = &mVulkanRenderContext;
}

void VulkanRenderer::CleanupSwapchain() {
    NATURE_FREE_POINTER(mVulkanPipeline);
    mVulkanDevice->DestroySwapchain(mVulkanSwapchainKHR);
}

void VulkanRenderer::CreateSwapchain() {
    mVulkanDevice->CreateSwapchain(&mVulkanSwapchainKHR);
    mVulkanPipeline = std::make_unique<VulkanPipeline>(mVulkanDevice.get(), mVulkanSwapchainKHR,
                                                   NATURE_SHADER_MODULE_OF_VERTEX_BINARY_FILE,
                                                   NATURE_SHADER_MODULE_OF_FRAGMENT_BINARY_FILE);
}

void VulkanRenderer::RecreateSwapchain() {
    mVulkanDevice->DeviceWaitIdle();
    CleanupSwapchain();
    CreateSwapchain();
    mVulkanDevice->FreeCommandBuffer(std::size(mCommandBuffers), std::data(mCommandBuffers));
    mVulkanDevice->AllocateCommandBuffer(std::size(mCommandBuffers), std::data(mCommandBuffers));
}

void VulkanRenderer::BeginRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t index) {
    mVulkanRenderContext.commandBuffer = commandBuffer;
    mVulkanRenderContext.index = index;
    mVulkanDevice->BeginCommandBuffer(mVulkanRenderContext.commandBuffer, VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
}

void VulkanRenderer::EndRecordCommandBuffer() {
    mVulkanDevice->EndCommandBuffer(mVulkanRenderContext.commandBuffer);
}

void VulkanRenderer::BeginRenderPass(VkRenderPass renderPass) {
    mVulkanRenderContext.renderPass = renderPass;
    /* start render pass. */
    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = mVulkanRenderContext.renderPass;
    renderPassBeginInfo.framebuffer = mVulkanSwapchainKHR->GetFramebuffer(mVulkanRenderContext.index);
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = mVulkanSwapchainKHR->GetExtent2D();

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;
    vkCmdBeginRenderPass(mVulkanRenderContext.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::EndRenderPass() {
    /* end render pass */
    vkCmdEndRenderPass(mVulkanRenderContext.commandBuffer);
}

void VulkanRenderer::BeginRender() {
    uint32_t index;
    mVulkanSwapchainKHR->AcquireNextImage(mImageAvailableSemaphore, &index);
    BeginRecordCommandBuffer(mCommandBuffers[index], index);
    BeginRenderPass(mVulkanSwapchainKHR->GetRenderPass());
}

void VulkanRenderer::QueueSubmitBuffer() {
    VkSemaphore waitSemaphores[] = { mImageAvailableSemaphore };
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphore };

    mVulkanDevice->SyncSubmitQueueWithSubmitInfo(1, &mVulkanRenderContext.commandBuffer,
                                                 1, waitSemaphores,
                                                 1, signalSemaphores,
                                                 waitStages);

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { mVulkanSwapchainKHR->GetSwapchainKHR() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &mVulkanRenderContext.index;
    presentInfo.pResults = nullptr; // Optional

    mVulkanDevice->SyncSubmitQueueWithPresentInfoKHR(&presentInfo);
}

void VulkanRenderer::UpdateUniformBuffer() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
    UniformBufferObject ubo = {};
    ubo.m = glm::rotate(glm::mat4(rotateM), time * glm::radians(rotateRadians), rotateV);
    ubo.v = glm::lookAt(lookAtEye, lookAtCenter, lookAtUp);
    ubo.p = glm::perspective(glm::radians(45.0f), mVulkanSwapchainKHR->GetWidth() / (float) mVulkanSwapchainKHR->GetHeight(), 0.1f, 10.0f);
    ubo.p[1][1] *= -1;
    ubo.t = (float) glfwGetTime();
    void* data;
    mVulkanDevice->MapMemory(mUniformBuffer, 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    mVulkanDevice->UnmapMemory(mUniformBuffer);
}

void VulkanRenderer::Draw() {
    UpdateUniformBuffer();
    /* bind graphics pipeline. */
    mVulkanPipeline->Bind(mVulkanRenderContext.commandBuffer);
    mVulkanPipeline->Write(0, sizeof(UniformBufferObject), mUniformBuffer, mTexture);
    /* bind vertex buffer */
    VkBuffer buffers[] = {mVertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(mVulkanRenderContext.commandBuffer, 0, 1, buffers, offsets);
    vkCmdBindIndexBuffer(mVulkanRenderContext.commandBuffer, mIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    /* draw call */
    // vkCmdDraw(mVulkanRenderContext.commandBuffer, 3, 1, 0, 0);
    vkCmdDrawIndexed(mVulkanRenderContext.commandBuffer, static_cast<uint32_t>(std::size(mIndices)), 1, 0, 0, 0);
}

void VulkanRenderer::EndRender() {
    EndRenderPass();
    EndRecordCommandBuffer();
    /* final submit */
    QueueSubmitBuffer();
}

const VulkanRenderInstanceContext *VulkanRenderer::GetVulkanRenderInstanceContext() const {
    return &mVulkanRenderInstanceContext;
}