/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|*    Copyright (C) 2023 bit-fashion                                                *|
|*                                                                                  *|
|*    This program is free software: you can redistribute it and/or modify          *|
|*    it under the terms of the GNU General Public License as published by          *|
|*    the Free Software Foundation, either version 3 of the License, or             *|
|*    (at your option) any later version.                                           *|
|*                                                                                  *|
|*    This program is distributed in the hope that it will be useful,               *|
|*    but WITHOUT ANY WARRANTY; without even the implied warranty of                *|
|*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *|
|*    GNU General Public License for more details.                                  *|
|*                                                                                  *|
|*    You should have received a copy of the GNU General Public License             *|
|*    along with this program.  If not, see <https://www.gnu.org/licenses/>.        *|
|*                                                                                  *|
|*    This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.    *|
|*    This is free software, and you are welcome to redistribute it                 *|
|*    under certain conditions; type `show c' for details.                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------------- *\
|*                                                                                  *|
|* File:           System.h                                                         *|
|* Create Time:    2023/12/30 21:07                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include "Typedef.h"
// std
#include <chrono>

#if defined(_WIN32)
#  include <windows.h>
#  include <dbghelp.h>
#endif

enum DebuggerPropertyFlags {
    SYSTEM_DEBUG_PROPERTY_TYPE_UNSIGNED,
    SYSTEM_DEBUG_PROPERTY_TYPE_INT,
    SYSTEM_DEBUG_PROPERTY_TYPE_LONG,
    SYSTEM_DEBUG_PROPERTY_TYPE_UINT32,
    SYSTEM_DEBUG_PROPERTY_TYPE_FLOAT,
    SYSTEM_DEBUG_PROPERTY_TYPE_FLOAT2,
    SYSTEM_DEBUG_PROPERTY_TYPE_FLOAT3,
    SYSTEM_DEBUG_PROPERTY_TYPE_DOUBLE,
    SYSTEM_DEBUG_PROPERTY_TYPE_STRING,
};

namespace System
{
    struct DebuggerProperty {
        std::string name;
        DebuggerPropertyFlags flags;
        void *vpointer;
    };

    /* 全局对象 */
    static HashMap<std::string, std::string> g_Properties;
    static HashMap<std::string, DebuggerProperty> g_DebuggerProperties;

    /** 获取当前时间戳（毫秒） */
    static uint64_t GetTimeMillis()
    {
        auto currentTime = std::chrono::system_clock::now();
        auto currentTimeMs = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTime);
        return currentTimeMs.time_since_epoch().count();
    }

    /** 获取当前时间戳（纳秒） */
    static uint64_t GetTimeNanos()
    {
        auto currentTime = std::chrono::system_clock::now();
        auto currentTimeNs = std::chrono::time_point_cast<std::chrono::nanoseconds>(currentTime);
        return currentTimeNs.time_since_epoch().count();
    }

    /** 以可变参数的形式传参，向控制台格式化打印输出 */
    static void VaConsoleWrite(std::string_view fmt, std::format_args args)
    {
        std::cout << vstrifmt(fmt, args) << std::endl;
    }

    template<typename ...Args>
    static void ConsoleWrite(std::string_view fmt, Args&& ...args)
    {
        VConsoleWrite(fmt, std::make_format_args(args...));
    }

    /** 设置全局属性 */
    static void SetProperty(const std::string &&key, const std::string value)
    {
        g_Properties.insert(std::pair<std::string, std::string>(std::move(key), std::move(value)));
    }

    /** 获取全局属性 */
    static std::string& GetProperty(const std::string &key)
    {
        return g_Properties[key];
    }

    /** 设置全局调试属性 */
    static void SetDebuggerProperty(const std::string &&name, DebuggerPropertyFlags flags, void *vpointer)
    {
        DebuggerProperty property = {};
        property.name = std::move(name);
        property.flags = flags;
        property.vpointer = vpointer;

        g_DebuggerProperties.insert(std::pair<std::string, DebuggerProperty>(std::move(name), property));
    }

    /** 获取全局调试属性 */
    static const DebuggerProperty& GetDebuggerProperty(const std::string &key)
    {
        return g_DebuggerProperties[key];
    }
}