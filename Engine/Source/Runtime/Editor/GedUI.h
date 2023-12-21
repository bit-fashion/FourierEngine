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

/* Creates on 2023/12/14. */

/*
 ===============================
   @author bit-fashion
 ===============================
*/
#pragma once

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include "Window/Window.h"
#include "Render/Drivers/Vulkan/VulkanContext.h"
#include <Debug.h>

/**
 * Game Editor
 */
class GedUI {
public:
    //
    // 公共函数
    //
    static void Init(const Window *window, VulkanContext *context);
    static void Destroy();
    static void BeginNewFrame();
    static void EndNewFrame();
    static ImTextureID AddTexture2D(VkTexture2D &texture);
    static void RemoveTexture2D(const ImTextureID &texture);
    static ImVec2 GetWindowSize();
    static void DrawTexture2D(const ImTextureID &texture, const ImVec2 &size);
    static void DrawTexture2DFill(const ImTextureID &texture, int *width = null, int *height = null);
    static void BeginViewport(const char *name);
    static void EndViewport();

private:
    struct State {
        bool ShowDemoWindowFlag = false;
    };

private:
    GedUI(const Window *window, VulkanContext *context);
    ~GedUI();
    void InitGameEditorContext(const Window *window, VulkanContext *context);

    //
    // start gui render
    //
    void BeginGameEditorFrame();
    void EndGameEditorFrame();

    //
    // 组件
    //
    void _MenuItemShowDemoWindow();
    void _ThemeEmbraceTheDarkness(); /* 设置主题 */

private:
    State state;
};
