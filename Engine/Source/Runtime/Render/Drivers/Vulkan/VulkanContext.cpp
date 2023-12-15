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

VulkanContext::VulkanContext(const Window *pWindow) : m_Window(pWindow) {
    InitVulkanDriverContext();
}

VulkanContext::~VulkanContext() {
    vkDestroyDevice(m_Device, VulkanUtils::Allocator);
    vkDestroySurfaceKHR(m_Instance, m_SurfaceKHR, VulkanUtils::Allocator);
    vkDestroyInstance(m_Instance, VulkanUtils::Allocator);
}

void VulkanContext::InitVulkanDriverContext() {
    /* Create vulkan instance. */
    VkInstanceCreateInfo instanceCreateInfo = {};
    VulkanUtils::ConfigureVulkanInstanceCreateInfo(&instanceCreateInfo);
    vkCreateInstance(&instanceCreateInfo, VulkanUtils::Allocator, &m_Instance);

    /* Create window surface */
#ifdef _glfw3_h_
    VulkanUtils::CreateWindowSurfaceKHR(m_Instance, m_Window->GetHandle(), &m_SurfaceKHR);
#endif

    /* Create vulkan device. */
    VulkanUtils::GetVulkanMostPreferredPhysicalDevice(m_Instance, &m_DriverPhysicalDevice);
    VkDeviceCreateInfo deviceCreateInfo = {};
    Vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    VulkanUtils::GetVulkanDeviceCreateRequiredQueueCreateInfo(m_DriverPhysicalDevice.device, m_SurfaceKHR, deviceQueueCreateInfos);
    VulkanUtils::ConfigureVulkanDeviceCreateInfo(deviceQueueCreateInfos, &deviceCreateInfo);
    vkCreateDevice(m_DriverPhysicalDevice.device, &deviceCreateInfo, VulkanUtils::Allocator, &m_Device);
}