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

/* Creates on 2023/12/14. */

/*
 ===============================
   @author bit-fashion
 ===============================
*/
#pragma once

#include <Python.h>
#include <Aurora/Typedef.h>
#include <stdarg.h>

#define Python3_Build_Value(signature, ...) ( Py_BuildValue(signature, __VA_ARGS__) )
#define Python3_Decref_Object(ref) \
    Py_DECREF(ref); \
    ref = null;

namespace Python3 {
    void        Initialize();
    void        AddPath(const String &path); // 添加脚本路径
    PyObject*   ImportModule(const char *name);
    PyObject*   BuildValue(const char *sign, ...);
    PyObject*   VaBuildValue(const char *sign, va_list va);
    void        Decref(PyObject *ref);
    PyObject*   Invoke(const char *py_object, const char *fn, PyObject *args = null);
    PyObject*   Invoke(const char *py_object, const char *fn, const char *sign, ...);
    void        Finalize();
}