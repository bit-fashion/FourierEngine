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
#pragma once

#include <vulkan/vulkan.h>
#include <SportsCore.h>

struct _VRACWindow_t {
    const GLWIN *Context = VK_NULL_HANDLE;
    VkSurfaceKHR SurfaceKHR = VK_NULL_HANDLE;
};

struct _VRACDevice_t {
    VkInstance Instance = VK_NULL_HANDLE;
    VkDevice Device = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties Properties = SPORTS_MEMORY_INIT;
    VkPhysicalDeviceFeatures Features = SPORTS_MEMORY_INIT;
};

struct _VRACSwapchainKHR_t {
    VkSwapchainKHR SwapchainKHR = VK_NULL_HANDLE;
    _VRACWindow_t *Window = VK_NULL_HANDLE;
    VkSurfaceFormatKHR SurfaceFormatKHR = SPORTS_MEMORY_INIT; /* auto */
    VkPresentModeKHR PresentModeKHR = SPORTS_MEMORY_INIT; /* auto */
    VkSurfaceCapabilitiesKHR SurfaceCapabilitiesKHR; /* auto */
};

struct _VRACBuffer_t {
    VkBuffer Buffer = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    VkDeviceSize Size = 0;
};

struct _VRACTexture2D_t {
    VkImage Image = VK_NULL_HANDLE;
    VkImageView ImageView = VK_NULL_HANDLE;
};

static VkSurfaceFormatKHR _GetBeastPreferredSwapchainSurfaceFormatKHR(const Vector<VkSurfaceFormatKHR> &surfaceFormatKHRs) {
    if (std::size(surfaceFormatKHRs) == 0 && surfaceFormatKHRs[0].format == VK_FORMAT_UNDEFINED)
        return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

    for (const auto& surfaceFormatKHR : surfaceFormatKHRs) {
        if (surfaceFormatKHR.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormatKHR.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return surfaceFormatKHR;
    }

    return surfaceFormatKHRs[0];
}

static VkPresentModeKHR _GetBeastPreferredSwapchainPresentModeKHR(Vector<VkPresentModeKHR> &presentModeKHRs) {
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto &presentModeKHR: presentModeKHRs) {
        if (presentModeKHR == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentModeKHR;
        } else if (presentModeKHR == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            bestMode = presentModeKHR;
        }
    }

    return bestMode;
}

static void ConfigurationSwapchainKHRSupportDetails(_VRACDevice_t *pVRACDevice, _VRACSwapchainKHR_t *pVRACSwapchainKHR) {
    /* 查询交换链支持 */
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pVRACDevice->PhysicalDevice, pVRACSwapchainKHR->Window->SurfaceKHR,
                                              &pVRACSwapchainKHR->SurfaceCapabilitiesKHR);

    /* 查询 Surface 支持的格式 */
    uint32_t surfaceFormatKHRCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pVRACDevice->PhysicalDevice, pVRACSwapchainKHR->Window->SurfaceKHR, &surfaceFormatKHRCount, VK_NULL_HANDLE);
    if (surfaceFormatKHRCount > 0) {
        Vector<VkSurfaceFormatKHR> surfaceFormatKHRs(surfaceFormatKHRCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pVRACDevice->PhysicalDevice, pVRACSwapchainKHR->Window->SurfaceKHR,
                                             &surfaceFormatKHRCount, std::data(surfaceFormatKHRs));
        pVRACSwapchainKHR->SurfaceFormatKHR = _GetBeastPreferredSwapchainSurfaceFormatKHR(surfaceFormatKHRs);
    }

    /* 查询 Surface 展示模式支持 */
    uint32_t presentModeKHRCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pVRACDevice->PhysicalDevice, pVRACSwapchainKHR->Window->SurfaceKHR, &presentModeKHRCount, VK_NULL_HANDLE);
    if (presentModeKHRCount > 0) {
        Vector<VkPresentModeKHR> presentModeKHRs(presentModeKHRCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pVRACDevice->PhysicalDevice, pVRACSwapchainKHR->Window->SurfaceKHR, &presentModeKHRCount, std::data(presentModeKHRs));
        pVRACSwapchainKHR->PresentModeKHR = _GetBeastPreferredSwapchainPresentModeKHR(presentModeKHRs);
    }
}