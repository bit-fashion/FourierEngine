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

namespace System
{
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
    __always_inline
    static void VaConsoleWrite(const char *fmt, va_list va)
    {
        vprintf(fmt, va);
        fflush(stdout); /* 立即刷新缓冲区 */
    }

    /** 向控制台格式化打印输出 */
    static void ConsoleWrite(const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        VaConsoleWrite(fmt, va);
        va_end(va);
    }
}