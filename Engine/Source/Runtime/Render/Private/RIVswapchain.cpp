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

/* Creates on 2023/11/21. */
#include "RIVswapchain.h"
#include "RIVdevice.h"
#include "Window/RIVwindow.h"
#include <limits>

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
VkExtent2D SelectSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, int width, int height) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

RIVSwapchainSupportedDetails RIVswapchain::QueryRIVSwapchainSupportedDetails() {
    RIVSwapchainSupportedDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_RIVdevice->RIVHPhysicalDevice(), m_RIVdevice->RIVHSurface(), &details.capabilities);

    /* Find surface support formats. */
    {
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_RIVdevice->RIVHPhysicalDevice(), m_RIVdevice->RIVHSurface(), &formatCount, VK_NULL_HANDLE);
        if (formatCount > 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_RIVdevice->RIVHPhysicalDevice(), m_RIVdevice->RIVHSurface(), &formatCount, std::data(details.formats));
        }
    }

    /* Find surface support present mode. */
    {
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_RIVdevice->RIVHPhysicalDevice(), m_RIVdevice->RIVHSurface(), &presentModeCount, VK_NULL_HANDLE);
        if (presentModeCount > 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_RIVdevice->RIVHPhysicalDevice(), m_RIVdevice->RIVHSurface(), &presentModeCount, std::data(details.presentModes));
        }
    }

    return details;
}

RIVswapchain::RIVswapchain(RIVdevice *pRIVdevice, int width, int height) : m_RIVdevice(pRIVdevice) {
    /* 初始化交换链支持参数 */
    RIVSwapchainSupportedDetails swapChainDetails = QueryRIVSwapchainSupportedDetails();
    m_SurfaceFormatKHR = SelectSwapSurfaceFormat(swapChainDetails.formats);
    m_SwapChainFormat = m_SurfaceFormatKHR.format;
    m_SurfacePresentModeKHR = SelectSwapSurfacePresentMode(swapChainDetails.presentModes);
    m_SwapChainExtent = SelectSwapExtent(swapChainDetails.capabilities, width, height);

    uint32_t swapchainImageCount = swapChainDetails.capabilities.minImageCount + 1;
    if (swapChainDetails.capabilities.maxImageCount > 0 && swapchainImageCount > swapChainDetails.capabilities.maxImageCount)
        swapchainImageCount = swapChainDetails.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfoKhr = {};
    swapchainCreateInfoKhr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfoKhr.surface = m_RIVdevice->RIVHSurface();
    swapchainCreateInfoKhr.minImageCount = swapchainImageCount;
    swapchainCreateInfoKhr.imageFormat = m_SurfaceFormatKHR.format;
    swapchainCreateInfoKhr.imageColorSpace = m_SurfaceFormatKHR.colorSpace;
    swapchainCreateInfoKhr.imageExtent = m_SwapChainExtent;
    swapchainCreateInfoKhr.imageArrayLayers = 1;
    swapchainCreateInfoKhr.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfoKhr.preTransform = swapChainDetails.capabilities.currentTransform;
    swapchainCreateInfoKhr.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfoKhr.presentMode = m_SurfacePresentModeKHR;
    swapchainCreateInfoKhr.clipped = VK_TRUE;
    swapchainCreateInfoKhr.oldSwapchain = VK_NULL_HANDLE;
    vkRIVCreate(SwapchainKHR, pRIVdevice->RIVHDevice(), &swapchainCreateInfoKhr, VK_NULL_HANDLE, &m_Swapchain);
}

RIVswapchain::~RIVswapchain() {
    vkDestroySwapchainKHR(m_RIVdevice->RIVHDevice(), m_Swapchain, VK_NULL_HANDLE);
}