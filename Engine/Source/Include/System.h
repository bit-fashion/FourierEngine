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
#ifndef _VECTRAFLUX_ENGINE_SYSTEM_H_
#define _VECTRAFLUX_ENGINE_SYSTEM_H_

#include <chrono>
#include <Typedef.h>

typedef std::chrono::milliseconds::rep timestamp64_t;

namespace System {
    /* define Properties type and statement global
     * properties object. */
    typedef HashMap<String, String> Properties;
    static  Properties __props__;

    /**
    * @return get current time million seconds.
    */
    static timestamp64_t GetTimeMillis() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    }

    static timestamp64_t GetTimeNanos() {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    }

    /**
     * set property to global properties variable.
     */
    static void SetProperty(const String &key, const String &value) {
        __props__[key] = value;
    }

    /**
     * get property from global properties variable.
     */
    static String GetProperty(const String &key) {
        return __props__[key];
    }

    static void FlushStdout() {
        fflush(stdout);
    }

    /**
     * 打印文本到控制台，支持字符串格式化
     */
    template<typename ...Args>
    static void ConsoleWrite(std::format_string<Args...> fmt, Args&&... args) {
        printf("%s\n", vstrfmt(fmt, args).c_str());
        fflush(stdout);
    }

    /**
     * 获取程序运行时间
     */
    typedef void (*PFN_GetRunTimeFunc) (void);
    inline static timestamp64_t GetRunTime(PFN_GetRunTimeFunc fn) {
        auto start = GetTimeMillis();
        fn();
        auto end = GetTimeMillis();
        return end - start;
    }

}

#endif /* _VECTRAFLUX_ENGINE_SYSTEM_H_ */
