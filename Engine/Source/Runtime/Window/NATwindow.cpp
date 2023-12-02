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
#include "Window/NATwindow.h"

NATwindow::NATwindow(int width, int height, const char *title) : m_Width(width), m_Height(height) {
    glfwInit();

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(width, height, title, 0, 0);

    if (m_Window == NULL) {
        glfwTerminate();
        VRRT_THROW_ERROR("NatureEngine Error: create window failed, cause window pointer is NULL!");
    }

    glfwSetWindowUserPointer(m_Window, this);

    /* Set resize callback. */
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height) {
        NATwindow *pRIVwindow = ((NATwindow *) glfwGetWindowUserPointer(window));
        pRIVwindow->SetWidth(width);
        pRIVwindow->SetHeight(height);
        if (pRIVwindow->m_FnVRRTResizableWindowCallback != NULL)
            pRIVwindow->m_FnVRRTResizableWindowCallback(pRIVwindow, width, height);
    });
}

NATwindow::~NATwindow() {
    glfwTerminate();
    glfwDestroyWindow(m_Window);
}

bool NATwindow::ShouldClose() {
    return glfwWindowShouldClose(m_Window);
}

void NATwindow::SetWindowHintVisible(bool isVisible) {
    if (isVisible)
        glfwShowWindow(m_Window);
    else
        glfwHideWindow(m_Window);
}