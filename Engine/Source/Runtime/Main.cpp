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
#include "Window/Window.h"
#include "Render/Drivers/Vulkan/VulkanContext.h"
#include <System.h>
#include "Editor/GedUI.h"

int main(int argc, const char **argv) {
    system("chcp 65001");

    //
    // 初始化
    //
    Window window("VectrafluxEngine", 1280, 1200);
    std::unique_ptr<VulkanContext> p_vctx = std::make_unique<VulkanContext>(&window);
    window.SetWindowHintVisible(true);
    GedUI::Init(&window, p_vctx.get());

    int rec_frame_count = 0, final_frame_count = 0;
    timestamp64_t rec_frame_start_time = System::GetTimeMillis();

#ifdef ENGINE_CONFIG_ENABLE_DEBUG
    Vectraflux::AddDebuggerWatch("FPS", VFLUX_DEBUGGER_WATCH_TYPE_INT, &final_frame_count);
#endif

    VkTexture2D texture;
    p_vctx->CreateTexture2D("../Document/EngineEditor.png", &texture);

    ImTextureID imTextureID = GedUI::AddTexture2D(texture);

    while (!window.is_close()) {

        //
        // 渲染 ImGui
        //
        VkGraphicsFrameContext *frameContext;
        p_vctx->BeginGraphicsRender(&frameContext);
            GedUI::BeginNewFrame();
                GedUI::BeginViewport("视口");
                    GedUI::Image(imTextureID);
                GedUI::EndViewport();
            GedUI::EndNewFrame();
        p_vctx->EndGraphicsRender();

        Window::PollEvents();

        ++rec_frame_count;
        timestamp64_t end = System::GetTimeMillis();
#ifdef ENGINE_CONFIG_ENABLE_DEBUG
        if (end - rec_frame_start_time >= 1000.0) {
            final_frame_count = rec_frame_count;
            rec_frame_count = 0;
            rec_frame_start_time = end;
        }
#endif
    }

    //
    // 资源释放
    //
    p_vctx->DestroyTexture2D(texture);
    GedUI::Destroy();
}
