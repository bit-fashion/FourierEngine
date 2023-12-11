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

#include <GLFW/glfw3.h>
#include <stdexcept>

class GLWIN;
typedef void (* PFN_EngineWindowResizableWindowCallback)(GLWIN *pGLWIN, int width, int height);

class GLWIN {
public:
    /* Init and create window. */
    GLWIN(int width, int height, const char *title);
    ~GLWIN();

    void SetWindowHintVisible(bool isVisible);
    bool WindowShouldClose();

public:
    int GetKey(int key) { return glfwGetKey(m_Window, key); }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    void SetWidth(int width) { m_Width = width; }
    void SetHeight(int height) { m_Height = height; }
    void SetWindowUserPointer(void *pointer) { m_UserPointer = pointer; }
    void *GetWindowUserPointer() const { return m_UserPointer; }
    void SetEngineWindowResizableWindowCallback(PFN_EngineWindowResizableWindowCallback fnEngineWindowResizableWindowCallback) /* Set resizable callback. */
      { m_FnNATUREResizableWindowCallback = fnEngineWindowResizableWindowCallback; };
    GLFWwindow *GetWindowHandle() const { return m_Window; };
#ifdef GLFW_INCLUDE_VULKAN
    void CreateWindowSurfaceKHR(VkInstance instance, const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurfaceKHR) const {
        glfwCreateWindowSurface(instance, GetWindowHandle(), pAllocator, pSurfaceKHR);
    }
#endif

public:
    static void PollEvents() { glfwPollEvents(); }

private:
    int m_Width;
    int m_Height;
    GLFWwindow *m_Window;
    void *m_UserPointer = NULL;
    PFN_EngineWindowResizableWindowCallback m_FnNATUREResizableWindowCallback = NULL;
};