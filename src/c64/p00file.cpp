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
#include "p00file.hpp"

#include <fstream>

#include "endian.hpp"

namespace caio {
namespace commodore {
namespace c64 {

void P00File::load(const std::string& fname)
{
    if (!fname.empty()) {
        std::ifstream is{fname, std::ios::binary | std::ios::in};
        if (!is) {
            throw IOError{"Can't open P00 file: {}: {}", fname, Error::to_string()};
        }

        if (!is.read(reinterpret_cast<char*>(&_hdr), sizeof(_hdr))) {
            throw IOError{"Can't read P00 header: {}: {}", fname, Error::to_string()};
        }

        if (be64toh(_hdr.magic) != P00_MAGIC) {
            throw IOError{"Invalid magic number: {}", fname};
        }

        if (_hdr.rsize != 0) {
            throw IOError{"REL file type is not unsupported: {}", fname};
        }

        PrgFile::load(is);
    }
}

void P00File::save(const std::string& fname, addr_t addr)
{
    if (!fname.empty()) {
        std::ofstream os{fname, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc};
        if (!os) {
            throw IOError{"Can't create P00 file: {}: {}", fname, Error::to_string()};
        }

        if (!os.write(reinterpret_cast<char*>(&_hdr), sizeof(_hdr))) {
            throw IOError{"Can't write P00 header: {}: {}", fname, Error::to_string()};
        }

        PrgFile::save(os, addr);
    }
}

}
}
}
