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
|* File:           IOUtils.h                                                        *|
|* Create Time:    2024/01/09 10:32                                                 *|
|* Author:         bit-fashion                                                      *|
|* EMail:          bit-fashion@hotmail.com                                          *|
|*                                                                                  *|
\* -------------------------------------------------------------------------------- */
#pragma once

#include <malloc.h>
#include <fstream>
#include <vector>
#include <stdexcept>

namespace IOUtils {

    static char *ReadBuf(const char *filepath, size_t *size) {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Error: open file failed!");
        *size = file.tellg();
        file.seekg(0);

        /* malloc buffer */
        char *buf = (char *) malloc(*size);
        file.read(buf, *size);
        file.close();

        return buf;
    }

    static void FreeBuf(char *binaries) {
        free(binaries);
    }

}