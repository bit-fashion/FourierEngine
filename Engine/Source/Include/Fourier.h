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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ************************************************************************/

/* Creates on 2023/11/21. */
#pragma once

#include <iostream>
#include <stdexcept>
#include <format>

#define FOURIER_DEBUG
#define FOURIER_ENGINE "FourierEngine"
#define FOURIER_ENGINE_MAX_DEVICE_NAME_SIZE 256U

#define fourier_logger_info(fmt, ...) printf("%s\n", std::format(fmt, __VA_ARGS__).c_str())
#define fourier_logger_error(fmt, ...)
#define fourier_throw_error(fmt, ...) throw std::runtime_error(std::format(fmt, ##__VA_ARGS__))

namespace fourier {

}