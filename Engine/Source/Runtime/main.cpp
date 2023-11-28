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
#include "Window/VRRTwindow.h"

 #include "Render/VRHI.h"
//#include "Render/Demo/VulkanRenderAPI.h"

int main() {
    auto window = VRRTwindow(1280, 1000, "NE-1");
    /* Create VRHI */
     std::unique_ptr<VRHI> pVRHI = std::make_unique<VRHI>(&window);
    window.ShowWindowInScreen();

    while (!window.WindowShouldClose()) {
        NRIVPollEvents();
        pVRHI->BeginRender();
        {
            pVRHI->Draw();
        }
        pVRHI->EndRender();
    }

    return 0;
}
