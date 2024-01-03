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
    static void _vprintlog(std::string_view level, std::string_view fmt, std::string_view color, std::format_args args)
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
        std::string preprocessor = strifmtc("{} [{}{}{}] - {}",
                                            time,
                                            color,
                                            level,
                                            ASCII_COLOR_RESET,
                                            fmt);
        /* println */
        System::VaConsoleWrite(preprocessor, args);
    }

    template<typename ...Args>
    static void Info(std::string_view fmt, Args&& ...args)
    {
        _vprintlog("INFO ", fmt, ASCII_COLOR_BLUE, std::make_format_args(args...));
    }

    template<typename ...Args>
    static void Debug(std::string_view fmt, Args&& ...args)
    {
        _vprintlog("DEBUG", fmt, ASCII_COLOR_BLUE, std::make_format_args(args...));
    }

    template<typename ...Args>
    static void Warn(std::string_view fmt, Args&& ...args)
    {
        _vprintlog("WARN ", fmt, ASCII_COLOR_YELLOW, std::make_format_args(args...));
    }

    template<typename ...Args>
    static void Error(std::string_view fmt, Args&& ...args)
    {
        _vprintlog("ERROR", fmt, ASCII_COLOR_RED, std::make_format_args(args...));
    }
}