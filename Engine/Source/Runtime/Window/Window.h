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
#ifndef _SPORTS_WINDOW_H_
#define _SPORTS_WINDOW_H_

#include <GLFW/glfw3.h>
#include <Typedef.h>

class Window {
public:
    Window(const String &title, uint32_t width, uint32_t height);
   ~Window();

public:
    GLFWwindow *GetHandle() const { return HWIN; }
    String GetTitle() const { return m_Title; }
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    bool ShouldClose() const { return glfwWindowShouldClose(HWIN); }

public:
    static void PollEvents() { glfwPollEvents(); }

private:
    GLFWwindow *HWIN;
    String m_Title;
    uint32_t m_Width;
    uint32_t m_Height;
};

#endif /* _SPORTS_WINDOW_H_ */
