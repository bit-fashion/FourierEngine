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
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class RIVdevice;

struct RIVSwapchainSupportedDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/**
 * 交换链
 */
class RIVswapchain {
public:
    RIVswapchain(RIVdevice *pRIVdevice, int width, int height);
    ~RIVswapchain();

private:
    RIVSwapchainSupportedDetails QueryRIVSwapchainSupportedDetails();

private:
    VkSwapchainKHR m_Swapchain;
    RIVdevice *m_RIVdevice;
    VkSurfaceFormatKHR m_SurfaceFormatKHR;
    VkFormat m_SwapChainFormat;
    VkPresentModeKHR m_SurfacePresentModeKHR;
    VkExtent2D m_SwapChainExtent;
};