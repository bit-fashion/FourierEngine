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
#include "Window.h"

Window::Window(const String &title, uint32_t width, uint32_t height)
  : m_Title(std::move(title)), m_WindowExtent2D(width, height) {

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    m_HWINDOW = glfwCreateWindow(m_WindowExtent2D.width, m_WindowExtent2D.height, getchr(m_Title), null, null);
    if (m_HWINDOW == null)
        throw std::runtime_error("Create glfw window failed!");

    glfwSetWindowUserPointer(m_HWINDOW, this);

    glfwSetWindowSizeCallback(m_HWINDOW, [](GLFWwindow *window, int w, int h) {
        Window *win = (Window *) glfwGetWindowUserPointer(window);
        win->m_WindowExtent2D.width = w;
        win->m_WindowExtent2D.height = h;

        for (const auto &item: win->m_WindowResizeableEventCallbacks)
            item(win, w, h);
    });
}

Window::~Window() {
    glfwDestroyWindow(m_HWINDOW);
    glfwTerminate();
}

WindowExtent2D Window::GetWindowExtent2D() const {
    return m_WindowExtent2D;
}

void Window::PutWindowUserPointer(const String &key, pointer_t pointer) {
    m_WindowUserPointers[key] = pointer;
}

pointer_t Window::GetWindowUserPointer(const String &key) const {
    if (m_WindowUserPointers.find(key) != m_WindowUserPointers.end())
        return m_WindowUserPointers.at(key);
    return null;
}

size_t Window::AddWindowResizeableCallback(PFN_WindowResizeableEventCallback callback) {
    m_WindowResizeableEventCallbacks.push_back(callback);
    return std::size(m_WindowResizeableEventCallbacks);
}

void Window::RemoveWindowResizeableCallback(size_t n) {
    auto iter = m_WindowResizeableEventCallbacks.begin() + n;
    m_WindowResizeableEventCallbacks.erase(iter);
}

bool Window::is_close() const {
    return glfwWindowShouldClose(m_HWINDOW);
}

void Window::SetWindowHintVisible(bool isVisible) const {
    isVisible ? glfwShowWindow(m_HWINDOW) : glfwHideWindow(m_HWINDOW);
}

int Window::GetKey(int key) const {
    return glfwGetKey(m_HWINDOW, key);
}