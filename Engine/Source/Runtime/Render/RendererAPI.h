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
#include <Fourier.h>

struct FourierPhysicalDevice {
    VkPhysicalDevice handle;
    char deviceName[FOURIER_ENGINE_MAX_DEVICE_NAME_SIZE];
};

class RendererAPI {
public:
    /* Init vulkan render api. */
    RendererAPI();
    ~RendererAPI();
private:
    /* Handle object. */
    VkInstance m_Instance = NULL;
    VkPhysicalDevice m_PhysicalDevice = NULL;
    /* Vectors. */
    std::vector<const char *> m_RequiredInstanceExtensions;
    std::vector<const char *> m_RequiredInstanceLayers;
    std::vector<FourierPhysicalDevice> m_FourierPhysicalDevices;
};

