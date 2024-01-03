/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|*    Copyright (C) 2023 bit-fashion                                                *|
|*                                                                                  *|
|*    This program is free software: you can redistribute it and/or modify          *|
|*    it under the terms of the GNU General Public License as published by          *|
|*    the Free Software Foundation, either version 3 of the License, or             *|
|*    (at your option) any later version.                                           *|
|*                                                                                  *|
|*    This program is distributed in the hope that it will be useful,               *|
|*    but WITHOUT ANY WARRANTY; without even the implied warranty of                *|
|*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *|
|*    GNU General Public License for more details.                                  *|
|*                                                                                  *|
|*    You should have received a copy of the GNU General Public License             *|
|*    along with this program.  If not, see <https://www.gnu.org/licenses/>.        *|
|*                                                                                  *|
|*    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.    *|
|*    This is free software, and you are welcome to redistribute it                 *|
|*    under certain conditions; type `show c' for details.                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|* File:           VulkanContext.cpp                                                *|
|* Create Time:    2023/12/30 20:21                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#include "VulkanContext.h"
#include "VulkanUtils.h"
// aurora
#include <Aurora/Engine.h>

VulkanContext::VulkanContext(Window *p_win) : m_Window(p_win)
{
    InitVulkanContextInstance();
}

VulkanContext::~VulkanContext()
{
    vkDestroyInstance(m_Instance, VulkanUtils::Allocator);
}

void VulkanContext::InitVulkanContextInstance()
{
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName = AURORA_ENGINE_NAME;
    applicationInfo.engineVersion =  VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = AURORA_ENGINE_NAME;
    applicationInfo.apiVersion = VK_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;

    Vector<const char *> requiredExtensions;
    VulkanUtils::GetVulkanContextInstanceRequiredExtensions(requiredExtensions);
    instanceCreateInfo.enabledExtensionCount = std::size(requiredExtensions);
    instanceCreateInfo.ppEnabledExtensionNames = std::data(requiredExtensions);

    Vector<const char *> requiredLayers;
    VulkanUtils::GetVulkanContextInstanceRequiredLayers(requiredLayers);
    instanceCreateInfo.enabledLayerCount = std::size(requiredLayers);
    instanceCreateInfo.ppEnabledLayerNames = std::data(requiredLayers);

    vkAURACreate(Instance, &instanceCreateInfo, VulkanUtils::Allocator, &m_Instance);
}

