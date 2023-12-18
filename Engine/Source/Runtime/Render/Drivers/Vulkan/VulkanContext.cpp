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
        vkCreateImageView(m_Device, &imageViewCreateInfo, VK_NULL_HANDLE, &pSwapchainContext->imageViews[i]);

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

        vkCreateFramebuffer(m_Device, &framebufferCreateInfo, nullptr, &pSwapchainContext->framebuffers[i]);
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
    m_Window->SetWindowResizeableEventCallback([](Window *window, uint32_t width, uint32_t height){
        VulkanContext *context = (VulkanContext *) window->GetWindowUserPointer();
        context->RecreateSwapchainContext(&context->m_SwapchainContext, width, height);
    });
}

void VulkanContext::DeviceWaitIdle() {
    vkDeviceWaitIdle(m_Device);
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
    m_Window->SetWindowUserPointer(this);
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
    VulkanUtils::CreateWindowSurfaceKHR(m_Instance, m_Window->GetHandle(), &m_SurfaceKHR);
#endif

    /* Create vulkan device. */
    VulkanUtils::GetVulkanMostPreferredPhysicalDevice(m_Instance, &m_PhysicalDevice, &m_PhysicalDeviceProperties,
                                                      &m_PhysicalDeviceFeature);
    Vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    VulkanUtils::GetVulkanDeviceCreateRequiredQueueCreateInfo(m_PhysicalDevice, m_SurfaceKHR, deviceQueueCreateInfos);

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

    /* Create swapchain */
    CreateSwapchainContext(&m_SwapchainContext);
}

void VulkanContext::DestroySwapchainContext(VkSwapchainContextKHR *pSwapchainContext) {
    vkDestroyRenderPass(m_Device, pSwapchainContext->renderpass, VulkanUtils::Allocator);
    for (int i = 0; i < pSwapchainContext->minImageCount; i++) {
        vkDestroyImageView(m_Device, pSwapchainContext->imageViews[i], VulkanUtils::Allocator);
        vkDestroyFramebuffer(m_Device, pSwapchainContext->framebuffers[i], VulkanUtils::Allocator);
    }
    vkDestroySwapchainKHR(m_Device, pSwapchainContext->swapchain, VulkanUtils::Allocator);
}