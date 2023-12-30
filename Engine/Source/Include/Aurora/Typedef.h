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
|* File:           Typedef.h                                                        *|
|* Create Time:    2023/12/30 21:07                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <format>
#include <stdarg.h>
#include <iostream>

/* 强制内联 */
#ifndef __always_inline
#  define __always_inline inline __attribute__((__always_inline__))
#endif

/** std::vector<T> 标准库封装 */
template<typename T>
class Vector : public std::vector<T>{
public:
    using std::vector<T>::vector;
    __always_inline void remove(size_t index)
    {
      this->erase(this->begin() + index);
    }
};

/** std::unordered_map<K, V> 标准库封装 */
template<typename K, typename V>
class HashMap : public std::unordered_map<K, V> {
public:
    using std::unordered_map<K, V>::unordered_map;
};

/** 字符串格式化 */
template<typename ...Args>
__always_inline static std::string strifmt(std::string fmt, Args&& ...args)
{
    return std::vformat(fmt, std::make_format_args(args...));
}

#define strifmtc(fmt, ...) ( strifmt(fmt, __VA_ARGS__).c_str() )