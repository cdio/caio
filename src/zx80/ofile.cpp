/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of caio.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
#include "ofile.hpp"

#include "endian.hpp"
#include "fs.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

void OFile::load(const std::string& fname)
{
    *static_cast<std::vector<uint8_t>*>(this) = fs::load(fname);

    uint16_t size = le16toh(*reinterpret_cast<uint16_t*>(data() + SIZE_OFFSET));
    if (size < LOAD_ADDR) {
        throw IOError{"Invalid cassette file: " + fname};
    }

    size -= LOAD_ADDR;
    if (size > MAX_SIZE) {
        throw IOError{"Invalid cassette file size: " + fname + ": " + std::to_string(size)};
    }
}

void OFile::save(const std::string& fname)
{
    fs::save(fname, *this);
}

}
}
}
