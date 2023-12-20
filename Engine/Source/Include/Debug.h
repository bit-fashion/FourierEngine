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

#include <Typedef.h>

/**
 * 调试数据类型枚举
 */
enum SportsDebugWatchType {
    SPORTS_DEBUG_WATCH_TYPE_STRING,
    SPORTS_DEBUG_WATCH_TYPE_POINTER,
    SPORTS_DEBUG_WATCH_TYPE_INT,
    SPORTS_DEBUG_WATCH_TYPE_FLOAT,
    SPORTS_DEBUG_WATCH_TYPE_DOUBLE,
    SPORTS_DEBUG_WATCH_TYPE_UINT32,
    SPORTS_DEBUG_WATCH_TYPE_FLOAT2,
    SPORTS_DEBUG_WATCH_TYPE_FLOAT3,
};

/**
 * 调试数据结构体定义
 */
struct SportsDebugWatchInfo {
    String name;
    SportsDebugWatchType valueType;
    const pointer_t value; /* value pointer */
    bool editable; /* 是否可编辑 */
};

/** 全局列表，存放调试数据结构 */
static Vector<SportsDebugWatchInfo> _GLOBAL_DEBUG_WATCHER;

/**
 * 推送一个调试数据到监听器
 */
inline
static void SportsDebugAddWatch(const String &name, SportsDebugWatchType type, pointer_t ptr,
                                bool editable = false) {
    _GLOBAL_DEBUG_WATCHER.push_back({name, type, ptr, editable});
}

/**
 * 推送一个调试数据到监听器
 */
static void SportsDebugRemoveWatch(const String &name) {
    size_t index = -1;
    size_t len = std::size(_GLOBAL_DEBUG_WATCHER);

    for (int i = 0; i < len; i++)
        if (_GLOBAL_DEBUG_WATCHER[(index = i)].name == name)
            break;

    if (index != -1)
        _GLOBAL_DEBUG_WATCHER.remove(index);
}