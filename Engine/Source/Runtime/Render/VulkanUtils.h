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
|* File:           VulkanUtils.h                                                    *|
|* Create Time:    2024/01/03 01:35                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#define vkAURACreate(name, ...) \
    if (vkCreate##name(__VA_ARGS__) != VK_SUCCESS) \
        Logger::ThrowRuntimeError("VulkanContext create vk {} object failed!", #name)

namespace VulkanUtils
{
    /* VK 分配器 */
    static VkAllocationCallbacks *Allocator = null;

    static void GetVulkanContextInstanceRequiredExtensions(Vector<const char *> &required)
    {
        uint32_t count;
        vkEnumerateInstanceExtensionProperties(null, &count, null);
        Vector<VkExtensionProperties> properties(count);
        vkEnumerateInstanceExtensionProperties(null, &count, std::data(properties));

        Logger::Debug("Vulkan instance support extensions: ");
        for (const auto &property : properties)
            Logger::Debug("  %s", property.extensionName);
    }

    static void GetVulkanContextInstanceRequiredLayers(Vector<const char *> &required)
    {
        uint32_t count;
        vkEnumerateInstanceLayerProperties(&count, null);
        Vector<VkLayerProperties> properties(count);
        vkEnumerateInstanceLayerProperties(&count, std::data(properties));

        Logger::Debug("Vulkan instance support layers: ");
        for (const auto &property : properties)
            Logger::Debug("  %s", property.layerName);
    }
}