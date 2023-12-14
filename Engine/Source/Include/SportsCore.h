//
// Created by 39898 on 2023/12/13.
//

#ifndef NATUREENGINE_SPORTS_CORE_H
#define NATUREENGINE_SPORTS_CORE_H

#include <vector>
#include <unordered_map>
#include <string>
#include <format>
#include <memory>

template <typename T> using Vector = std::vector<T>;
template <typename K, typename V> using HashMap = std::unordered_map<K, V>;
typedef std::string String;

#if defined(_WIN32)
#  define SPORTS_API_ATTR
#  define SPORTS_API_CALL __stdcall
#else
#  define SPORTS_API_ATTR
#  define SPORTS_API_CALL
#endif

#endif //NATUREENGINE_CORE_H
