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

#define HWINDOW GLFWwindow *

class Window;

struct WindowExtent2D {
    int width;
    int height;
};

typedef void (*PFN_WindowResizeableEventCallback)(Window *window, int width, int height);

class Window {
public:
    Window(const String &title, uint32_t width, uint32_t height);
   ~Window();

   GLFWwindow *GetWindowPointer() const { return m_HWINDOW; }
   WindowExtent2D GetWindowExtent2D() const;
   void PutWindowUserPointer(const String &key, pointer_t pointer);
   pointer_t GetWindowUserPointer(const String &key) const;
   size_t AddWindowResizeableCallback(PFN_WindowResizeableEventCallback callback);
   void RemoveWindowResizeableCallback(size_t n);
   bool is_close() const;
   void SetWindowHintVisible(bool isVisible) const;
   int GetKey(int key) const; /* GLFW_KEY_* */

public:
    static void PollEvents() { glfwPollEvents(); }

private:
    HWINDOW m_HWINDOW;
    WindowExtent2D m_WindowExtent2D;
    const String m_Title;
    HashMap<String, pointer_t> m_WindowUserPointers;
    Vector<PFN_WindowResizeableEventCallback> m_WindowResizeableEventCallbacks;
};

#endif /* _SPORTS_WINDOW_H_ */
