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
#ifndef _SPORTS_TYPEDEF_H_
#define _SPORTS_TYPEDEF_H_

#include <vector>
#include <unordered_map>
#include <string>
#include <format>
#include <memory>
#include <array>

template <typename T> using Vector = std::vector<T>;
template <typename T, std::size_t N> using Array = std::array<T, N>;
template <typename K, typename V> using HashMap = std::unordered_map<K, V>;
typedef std::string String;

#endif /* _SPORTS_TYPEDEF_H_ */
