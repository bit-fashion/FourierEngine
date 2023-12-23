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
#ifndef _VECTRAFLUX_TYPEDEF_H_
#define _VECTRAFLUX_TYPEDEF_H_

#include <vector>
#include <list>
#include <set>
#include <unordered_map>
#include <string>
#include <format>
#include <memory>
#include <array>
#include <algorithm>
#include <stdarg.h>

#if defined(VK_VERSION_1_0)
#  define null VK_NULL_HANDLE
#else
#  define null nullptr
#endif

#define pointer_t void *

template<typename T>
class Vector : public std::vector<T> {
public:
    using std::vector<T>::vector;
    inline void remove(size_t index) { this->erase(this->begin() + index); }
};

template<typename K, typename V>
class HashMap : public std::unordered_map<K, V> {
public:
    using std::unordered_map<K, V>::unordered_map;
};

template <typename T> using List = std::list<T>;
template <typename T> using Set = std::set<T>;
template <typename T, std::size_t N> using Array = std::array<T, N>;
typedef std::string String;

// std::string to const char *
#define getchr(str) (str.c_str())

#define vstrfmt(_fmt, _args) std::vformat(_fmt.get(), std::make_format_args(_args...))

template<typename... Args>
inline static String strfmt(std::format_string<Args...> fmt, Args&&... args) {
    return vstrfmt(fmt, args);
}

#define chrfmt(fmt, ...) ( getchr(strfmt(fmt, __VA_ARGS__)) )

/* Get total byte size of array. */
#define ARRAY_SIZE(a) (sizeof(a[0]) * std::size(a))

/**
 * 获取 va_list 个数
 */
#define va_argc(lval, start, va, type)      \
    {                                       \
        while (va_arg((va), type) != NULL)  \
            lval++;                         \
    }

#endif /* _VECTRAFLUX_TYPEDEF_H_ */
