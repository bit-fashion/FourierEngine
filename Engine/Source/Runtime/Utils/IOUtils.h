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

/* Creates on 2022/11/23. */
#pragma once

#include <malloc.h>
#include <fstream>
#include <vector>
#include <Fourier.h>

static char *fourier_load_binaries(const char *file_path, size_t *size) {
    std::ifstream file(file_path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        fourier_throw_error("Error: open {} file failed!", file_path);
    *size = file.tellg();
    file.seekg(0);

    /* malloc buffer */
    char *buf = (char *) malloc(*size);
    file.read(buf, *size);
    file.close();

    return buf;
}

static void fourier_free_binaries(char *binaries) {
    free(binaries);
}