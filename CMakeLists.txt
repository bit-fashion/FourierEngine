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
CMAKE_MINIMUM_REQUIRED(VERSION 3.26)
PROJECT(SportsEngine)

SET(CMAKE_CXX_STANDARD 23)
SET(ENGINE_SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/Engine/Source")
SET(ENGINE_THIRD_PARTY_SOURCE_DIRECTORY "${ENGINE_SOURCE_DIRECTORY}/ThirdParty")
SET(ENGINE_RUNTIME_SOURCE_DIRECTORY "${ENGINE_SOURCE_DIRECTORY}/Runtime")

##
## Python3
##
FIND_PACKAGE(Python3 COMPONENTS Interpreter Development)
IF(Python3_FOUND)
    MESSAGE(STATUS "Python3 library found: ${Python3_FOUND}")
    SET(PYTHON3_INCLUDE "${Python3_INCLUDE_DIRS}")
    SET(PYTHON3_EXECUTABLE "${Python3_EXECUTABLE}")
    SET(PYTHON3_LIBRARIES "${Python3_LIBRARIES}")
    SET(PYTHON3_VERSION "${Python3_VERSION}")
    # debug info
    MESSAGE(STATUS "Python3 include directory: ${PYTHON3_INCLUDE}")
    MESSAGE(STATUS "Python3 library directory: ${PYTHON3_LIBRARIES}")
    MESSAGE(STATUS "Python3 executable directory: ${PYTHON3_EXECUTABLE}")
    MESSAGE(STATUS "Python3 version: ${PYTHON3_VERSION}")
ENDIF()

#[[ Configure third party include of system ]]
INCLUDE_DIRECTORIES(SYSTEM
  #"${PYTHON3_INCLUDE}"
  "${ENGINE_SOURCE_DIRECTORY}/Include"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/vulkan/include"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/glfw/include"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/glad/include"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/glm"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/imgui"
)

INCLUDE_DIRECTORIES(
  "${ENGINE_SOURCE_DIRECTORY}/Runtime"
)

#[[ Configure link directories ]]
LINK_DIRECTORIES(
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/vulkan"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/glfw/lib-mingw-w64"
)

ADD_EXECUTABLE(${PROJECT_NAME}
  "${ENGINE_SOURCE_DIRECTORY}/Include/pch.cpp"
  "${ENGINE_SOURCE_DIRECTORY}/Include/Debug.cpp"
  "${ENGINE_RUNTIME_SOURCE_DIRECTORY}/Main.cpp"
  "${ENGINE_RUNTIME_SOURCE_DIRECTORY}/Editor/GedUI.cpp"
  "${ENGINE_RUNTIME_SOURCE_DIRECTORY}/Window/Window.cpp"
  #[[ Render ]]
  "${ENGINE_RUNTIME_SOURCE_DIRECTORY}/Render/Camera/OrthoCamera.cpp"
  "${ENGINE_RUNTIME_SOURCE_DIRECTORY}/Render/Drivers/Vulkan/VulkanContext.cpp"
  #[[ Script ]]
  #"${ENGINE_RUNTIME_SOURCE_DIRECTORY}/Scripter/Python3.cpp"
  #[[ Dear ImGUI ]]
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/imgui/imgui.cpp"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/imgui/imgui_draw.cpp"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/imgui/imgui_demo.cpp"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/imgui/imgui_tables.cpp"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/imgui/imgui_widgets.cpp"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/imgui/backends/imgui_impl_vulkan.cpp"
  "${ENGINE_THIRD_PARTY_SOURCE_DIRECTORY}/imgui/backends/imgui_impl_glfw.cpp"
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  vulkan-1
  glfw3
  imm32
  #${PYTHON3_LIBRARIES}
)