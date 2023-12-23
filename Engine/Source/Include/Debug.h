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
enum VfluxDebugWatchValueType {
    VFLUX_DEBUG_WATCH_TYPE_STRING,
    VFLUX_DEBUG_WATCH_TYPE_POINTER,
    VFLUX_DEBUG_WATCH_TYPE_INT,
    VFLUX_DEBUG_WATCH_TYPE_FLOAT,
    VFLUX_DEBUG_WATCH_TYPE_DOUBLE,
    VFLUX_DEBUG_WATCH_TYPE_UINT32,
    VFLUX_DEBUG_WATCH_TYPE_FLOAT2,
    VFLUX_DEBUG_WATCH_TYPE_FLOAT3,
};

namespace Vectraflux {

    /**
     * 调试数据结构体定义
     */
    struct DebugWatchInfo {
        String name;
        VfluxDebugWatchValueType type;
        const pointer_t value; /* value pointer */
        bool editable; /* 是否可编辑 */
    };

    typedef void (*PFN_DebugWatchMapIteration)(const DebugWatchInfo &watch);

    /** 全局列表，存放调试数据结构 */
    extern HashMap<String, DebugWatchInfo> __gdwp__;

    /**
     * 推送一个调试数据到监听器
     */
    inline
    static void AddDebugWatch(const String &name, VfluxDebugWatchValueType type, pointer_t ptr,
                                    bool editable = false) {
        __gdwp__[name] = {name, type, ptr, editable};
    }

    /**
     * 推送一个调试数据到监听器
     */
    inline
    static void RemoveDebugWatch(const String &name) {
        __gdwp__.erase(name);
    }

    inline
    static void GetDebugWatchIterator(PFN_DebugWatchMapIteration fn) {
        for (const auto &item: __gdwp__)
            fn(item.second);
    }

}