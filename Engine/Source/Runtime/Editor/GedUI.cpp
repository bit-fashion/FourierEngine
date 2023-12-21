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
#include "GedUI.h"

static VkApplicationContext *s_DriverApplicationContext = null;
GedUI *_GECTX = null;

GedUI::GedUI(const Window *window, VulkanContext *context) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    InitGameEditorContext(window, context);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);
}

GedUI::~GedUI() {
    ImGui_ImplVulkan_DestroyFontsTexture();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}

void GedUI::InitGameEditorContext(const Window *window, VulkanContext *context) {
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // set default font.
    io.Fonts->AddFontFromFileTTF("../Engine/Assets/Fonts/smiley-sans-v1.1.1/SmileySans-Oblique.ttf", 21.0f,
                                 null, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    io.FontDefault = io.Fonts->Fonts.back();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // default theme.
    _ThemeEmbraceTheDarkness();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window->GetWindowPointer(), true);

    context->GetApplicationContext(&s_DriverApplicationContext);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = s_DriverApplicationContext->Instance;
    init_info.PhysicalDevice = s_DriverApplicationContext->PhysicalDevice;
    init_info.Device = s_DriverApplicationContext->Device;
    init_info.QueueFamily = s_DriverApplicationContext->GraphicsQueueFamily;
    init_info.Queue = s_DriverApplicationContext->GraphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = s_DriverApplicationContext->DescriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = s_DriverApplicationContext->MinImageCount;
    init_info.ImageCount = s_DriverApplicationContext->MinImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = VK_NULL_HANDLE;
    ImGui_ImplVulkan_Init(&init_info, s_DriverApplicationContext->RenderPass);
}

void GedUI::BeginGameEditorFrame() {
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // docking
    ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_PassthruCentralNode);
}

void GedUI::EndGameEditorFrame() {
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Rendering
    ImGui::Render();
    ImDrawData* main_draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(main_draw_data, s_DriverApplicationContext->FrameContext->commandBuffer);

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void GedUI::_MenuItemShowDemoWindow() {
    if (this->state.ShowDemoWindowFlag) {
        if (ImGui::MenuItem("关闭 ImGui Demo"))
            this->state.ShowDemoWindowFlag = false;
    } else {
        if (ImGui::MenuItem("显示 ImGui Demo"))
            this->state.ShowDemoWindowFlag = true;
    }
}

void GedUI::_MenuItemShowDemoWatchWindow() {
    if (this->state.ShowDebugWatchWindowFlag) {
        if (ImGui::MenuItem("关闭开发者调试器"))
            this->state.ShowDebugWatchWindowFlag = false;
    } else {
        if (ImGui::MenuItem("显示开发者调试器"))
            this->state.ShowDebugWatchWindowFlag = true;
    }
}

void GedUI::_ShowDebugWatchWindow() {
    ImGui::Begin("开发者调试器");
    {
        SportsDebugWatchIteration([](const SportsDebugWatchInfo &watch) {
            switch (watch.type) {
                case SPORTS_DEBUG_WATCH_TYPE_INT:
                    ImGui::Text("%s: %d", _chars(watch.name), *((int *) watch.value));
                    break;
                case SPORTS_DEBUG_WATCH_TYPE_UINT32:
                    ImGui::Text("%s: %u", _chars(watch.name), *((uint32_t *) watch.value));
                    break;
                case SPORTS_DEBUG_WATCH_TYPE_FLOAT:
                    ImGui::Text("%s: %f", _chars(watch.name), *((float *) watch.value));
                    break;
                case SPORTS_DEBUG_WATCH_TYPE_POINTER:
                    ImGui::Text("%s: %p", _chars(watch.name), watch.value);
                    break;
            }
        });
    }
    ImGui::End();
}

void GedUI::_ThemeEmbraceTheDarkness() {
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border]                 = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding                     = ImVec2(8.00f, 8.00f);
    style.FramePadding                      = ImVec2(5.00f, 2.00f);
    style.CellPadding                       = ImVec2(6.00f, 6.00f);
    style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
    style.IndentSpacing                     = 25;
    style.ScrollbarSize                     = 15;
    style.GrabMinSize                       = 10;
    style.WindowBorderSize                  = 1;
    style.ChildBorderSize                   = 1;
    style.PopupBorderSize                   = 1;
    style.FrameBorderSize                   = 1;
    style.TabBorderSize                     = 1;
    style.WindowRounding                    = 7;
    style.ChildRounding                     = 4;
    style.FrameRounding                     = 3;
    style.PopupRounding                     = 4;
    style.ScrollbarRounding                 = 9;
    style.GrabRounding                      = 3;
    style.LogSliderDeadzone                 = 4;
    style.TabRounding                       = 4;
}

//
// GameEditor
//
void GedUI::Init(const Window *window, VulkanContext *context) {
    _GECTX = new GedUI(window, context);
}

void GedUI::Destroy() {
    delete _GECTX;
}

void GedUI::BeginNewFrame() {
    _GECTX->BeginGameEditorFrame();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("帮助")) {
            _GECTX->_MenuItemShowDemoWindow();
            _GECTX->_MenuItemShowDemoWatchWindow();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    /* 显示 Demo 窗口 */
    if (_GECTX->state.ShowDemoWindowFlag)
        ImGui::ShowDemoWindow(&_GECTX->state.ShowDemoWindowFlag);

    /* 显示 Debug 窗口 */
    if (_GECTX->state.ShowDebugWatchWindowFlag)
        _GECTX->_ShowDebugWatchWindow();
}

void GedUI::EndNewFrame() {
    _GECTX->EndGameEditorFrame();
}

ImTextureID GedUI::AddTexture2D(VkTexture2D &texture) {
    return (ImTextureID) ImGui_ImplVulkan_AddTexture(texture.sampler, texture.imageView, texture.layout);
}

void GedUI::RemoveTexture2D(const ImTextureID &texture) {
    ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet) texture);
}

ImVec2 GedUI::GetWindowSize() {
    return ImGui::GetWindowSize();
}

void GedUI::DrawTexture2D(const ImTextureID &texture, const ImVec2 &size) {
    ImGui::Image(texture, size);
}

void GedUI::DrawTexture2DFill(const ImTextureID &texture, int *width, int *height) {
    ImVec2 windowSize = ImGui::GetContentRegionAvail();
    if (width != null)
        *width = windowSize.x;
    if (height != null)
        *height = windowSize.y;
    ImGui::Image(texture, ImVec2(*width, *height));
}

void GedUI::BeginViewport(const char *name) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin(name);
}

void GedUI::EndViewport() {
    ImGui::End();
    ImGui::PopStyleVar();
}