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
|* File:           Logger.h                                                         *|
|* Create Time:    2023/12/30 21:07                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include "Color.h"
#include "System.h"
#include "Date.h"
// std
#include <stdexcept>

namespace Logger
{
    __always_inline
    static void __VaLoggerConsoleWrite(const char *level, const char *fmt, const char *color, va_list va)
    {
        /*
         * color
         * datetime
         * function
         * fmt
         * level
         */
        char time[32];
        Date::Format(time, "%Y-%m-%d %H:%M:%S", sizeof(time));
        System::VaConsoleWrite(strifmtc("{} [{}{}{}] - {}\n",
                                        time,
                                        color,
                                        level,
                                        ASCII_COLOR_RESET,
                                        fmt), va);

    }

    /** Info 日志打印 */
    static void Info(const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        __VaLoggerConsoleWrite("INFO ", fmt, ASCII_COLOR_BLUE, va);
        va_end(va);
    }

    /** Debug 日志打印 */
    static void Debug(const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        __VaLoggerConsoleWrite("DEBUG", fmt, ASCII_COLOR_BLUE, va);
        va_end(va);
    }

    /** Warning 日志打印 */
    static void Warn(const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        __VaLoggerConsoleWrite("WARN ", fmt, ASCII_COLOR_YELLOW, va);
        va_end(va);
    }

    /** Error 日志打印 */
    static void Error(const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        __VaLoggerConsoleWrite("ERROR", fmt, ASCII_COLOR_RED, va);
        va_end(va);
    }

    static void ThrowRuntimeError(const char *fmt, ...)
    {
        char message[512];
        va_list va;
        va_start(va, fmt);
        vsnprintf(message, sizeof(message), fmt, va);
        throw std::runtime_error(std::string(message));
        va_end(va);
    }
}