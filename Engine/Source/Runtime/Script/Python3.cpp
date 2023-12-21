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
#include "Python3.h"
#include <sstream>

namespace Python3 {

    static HashMap<const char *, PyObject *> s_ObjectCache;

    void Initialize() {
        Py_Initialize();

        if (!Py_IsInitialized())
            throw std::runtime_error("Python interpreter initialize failed!");

        PyRun_SimpleString("import sys, os");
    }

    void AddPath(const String &path) {
        std::stringstream stream(path); String token;
        while (std::getline(stream, token, ';'))
            PyRun_SimpleString(chrfmt("sys.path.append('{}')", token));
    }

    PyObject *ImportModule(const char *name) {
        PyObject *py_module = null;

        if (s_ObjectCache.contains(name))
            goto ImportPyModuleObjectEndTag;

        py_module = PyImport_ImportModule(name);
        s_ObjectCache[name] = py_module;

    ImportPyModuleObjectEndTag:
        return s_ObjectCache[name];
    }

    void Invoke(const char *py_object, const char *fn, PyObject *args) {
        PyObject *py_module = ImportModule(py_object);
        PyObject *py_func = PyObject_GetAttrString(py_module, fn);
        PyObject_CallObject(py_func, args);
    }

    void Finalize() {
        Py_Finalize();
    }
}