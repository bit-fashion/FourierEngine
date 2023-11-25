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

#include "Render/Private/RIVdevice.h"
#include "Render/Private/RIVswapchain.h"
#include <memory>

class RIVwindow;

#define VK_LAYER_KHRONOS_validation "VK_LAYER_KHRONOS_validation"

class RivuletRenderer {
public:
    /* init */
    explicit RivuletRenderer(RIVwindow *pRIVwindow);
    ~RivuletRenderer();
private:
    void InitRIVrenderer();

private:
    VkInstance m_Instance;
    VkSurfaceKHR m_Surface;
    std::unique_ptr<RIVdevice> m_RIVdevice;
    RIVswapchain *m_RIVswapchain;
    RIVwindow *m_RIVwindow;
    /* Vectors. */
    std::vector<const char *> m_RequiredInstanceExtensions;
    std::vector<const char *> m_RequiredInstanceLayers;
    /* Map */
    std::unordered_map<std::string, VkExtensionProperties> m_VkInstanceExtensionPropertiesSupports;
    std::unordered_map<std::string, VkLayerProperties> m_VkInstanceLayerPropertiesSupports;
};