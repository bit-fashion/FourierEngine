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
#ifndef _SPORTS_VULKAN_CONTEXT_H_
#define _SPORTS_VULKAN_CONTEXT_H_

#include <vulkan/vulkan.h>
#include <Typedef.h>
#include <Engine.h>
#include <stdexcept>

class Window;

class VulkanContext {
public:
    struct SwapchainContext {
        VkSwapchainKHR swapchain;
        Vector<VkImage> images;
        Vector<VkImageView> imageViews;
        Vector<VkFramebuffer> framebuffers;
        VkRenderPass renderpass;
        uint32_t minImageCount;
        VkSurfaceFormatKHR surfaceFormat;
        VkSurfaceKHR surface;
        const Window *window;
        uint32_t width;
        uint32_t height;
        VkSurfaceCapabilitiesKHR capabilities;
        VkPresentModeKHR presentMode;
    };

public:
    VulkanContext(Window *window);
   ~VulkanContext();

private:
    void _CreateSwapcahinAboutComponents(SwapchainContext *pSwapchainContext);
    void _CreateRenderpass(SwapchainContext *pSwapchainContext);
    void _ConfigurationSwapchainContext(SwapchainContext *pSwapchainContext);
    void _ConfigurationWindowResizeableEventCallback();

    void DeviceWaitIdle();
    void RecreateSwapchainContext(SwapchainContext *pSwapchainContext, uint32_t width, uint32_t height);

    void CreateSwapchainContext(SwapchainContext *pSwapchainContext);
    void InitVulkanDriverContext();

    void DestroySwapchainContext(SwapchainContext *pSwapchainContext);

private:
    VkInstance m_Instance;
    VkSurfaceKHR m_SurfaceKHR;
    VkDevice m_Device;
    SwapchainContext m_SwapchainContext;

    Window *m_Window;
    VkPhysicalDevice m_PhysicalDevice;
    VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures m_PhysicalDeviceFeature;
};

#endif /* _SPORTS_VULKAN_CONTEXT_H_ */
