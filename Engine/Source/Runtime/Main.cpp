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
#include "Window/NatureWindow.h"

#include "Render/Vulkan/VulkanRenderer.h"
#include "Editor/DearImGuiEditor.h"

int main() {
    system("chcp 65001");
    system("cd ../Bin & spvc.cmd");
    auto window = NatureWindow(1280, 1000, "NatureEngine(vulkan)");
    /* Create VulkanRenderer */
    std::unique_ptr<VulkanRenderer> pVulkanRenderer = std::make_unique<VulkanRenderer>(&window);
    std::unique_ptr<DearImGuiEditor> pDearImGuiEditor = std::make_unique<DearImGuiEditor>(&window, pVulkanRenderer.get());
    window.SetWindowHintVisible(true);

    while (!window.is_close()) {
        NatureWindow::PollEvents();
        pVulkanRenderer->BeginRender();
        {
            /* Draw */
            pVulkanRenderer->Draw();
            /* Editor */
            pDearImGuiEditor->BeginEditorFrameRender();
            {
                ImGui::Begin("Draw");
                {
                    ImGui::DragFloat3("Lookat Center", glm::value_ptr(pVulkanRenderer->lookAtCenter));
                    ImGui::DragFloat3("Lookat Eye", glm::value_ptr(pVulkanRenderer->lookAtEye));
                    ImGui::DragFloat3("Lookat Up", glm::value_ptr(pVulkanRenderer->lookAtUp));

                    ImGui::DragFloat3("Rotate V", glm::value_ptr(pVulkanRenderer->rotateV));
                    ImGui::DragFloat("Rotate Radians", &pVulkanRenderer->rotateRadians);
                    ImGui::DragFloat("Rotate M", &pVulkanRenderer->rotateM);
                }
                ImGui::End();
            }
            pDearImGuiEditor->EndEditorFrameRender();
        }
        pVulkanRenderer->EndRender();
    }

    return 0;
}
