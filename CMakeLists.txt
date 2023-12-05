# ************************************************************************ #
#
# Copyright (C) 2022 Vincent Luo All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ************************************************************************ #

# Creates on 2022/9/14. #
cmake_minimum_required(VERSION 3.26)
project(NatureEngine)

set(CMAKE_CXX_STANDARD 23)

# Configure third party include of system.
include_directories(SYSTEM
        "${PROJECT_SOURCE_DIR}/Engine/Source/Include"
        "${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty"
        "${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/vulkan/include"
        "${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/glfw/include"
        "${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/glad/include"
        "${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/glm"
        "${PROJECT_SOURCE_DIR}/Engine/Source/ThirdParty/imgui"
)

include_directories(
        "${CMAKE_SOURCE_DIR}/Engine/Source/Include"
)

# Configure link directories.
link_directories(
        "Engine/Source/ThirdParty/vulkan"
        "Engine/Source/ThirdParty/glfw/lib-mingw-w64"
)

add_executable(${PROJECT_NAME}
        "Engine/Source/Runtime/Main.cpp"
        "Engine/Source/Runtime/Window/NatureWindow.cpp"
        "Engine/Source/Runtime/Render/Vulkan/VulkanRenderer.cpp"
        "Engine/Source/Runtime/Editor/DearImGuiEditor.cpp"
        "Engine/Source/Runtime/Render/Camera/OrthoCamera.cpp"
        #[[ Dear ImGUI ]]
        "Engine/Source/ThirdParty/imgui/imgui.cpp"
        "Engine/Source/ThirdParty/imgui/imgui_draw.cpp"
        "Engine/Source/ThirdParty/imgui/imgui_demo.cpp"
        "Engine/Source/ThirdParty/imgui/imgui_tables.cpp"
        "Engine/Source/ThirdParty/imgui/imgui_widgets.cpp"
        "Engine/Source/ThirdParty/imgui/backends/imgui_impl_vulkan.cpp"
        "Engine/Source/ThirdParty/imgui/backends/imgui_impl_glfw.cpp"
)

target_link_libraries(${PROJECT_NAME}
        vulkan-1
        glfw3
        imm32
)