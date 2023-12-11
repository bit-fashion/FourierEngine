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

struct _VRACWindow_t {
    const GLWIN *pWIN = VK_NULL_HANDLE;
    VkSurfaceKHR SurfaceKHR = VK_NULL_HANDLE;
};

struct _VRACDevice_t {
    VkInstance Instance = VK_NULL_HANDLE;
    VkDevice Device = VK_NULL_HANDLE;
    VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties Properties = NATURE_OBJECT_INIT;
    VkPhysicalDeviceFeatures Features = NATURE_OBJECT_INIT;
};

struct _VRACSwapchainKHR_t {
    VkSwapchainKHR SwapchainKHR = VK_NULL_HANDLE;
    _VRACWindow_t *pVRACWindow = VK_NULL_HANDLE;
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