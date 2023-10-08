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
#include "prgfile.hpp"

#include <fstream>

#include "endian.hpp"


namespace caio {

void PrgFile::load(const std::string& fname)
{
    if (!fname.empty()) {
        std::ifstream is{fname, std::ios::binary | std::ios::in};
        if (!is) {
            throw IOError{"Can't open PRG file: " + fname + ": " + Error::to_string()};
        }

        load(is);
    }
}

std::istream& PrgFile::load(std::istream& is)
{
    if (!is.read(reinterpret_cast<char*>(&_hdr), sizeof(_hdr))) {
        throw IOError{"Can't read PRG header: " + Error::to_string()};
    }

    _hdr.addr = le16toh(_hdr.addr);

    clear();
    uint8_t c{};
    while (is.read(reinterpret_cast<char*>(&c), sizeof(c))) {
        push_back(c);
    }

    return is;
}

void PrgFile::save(const std::string& fname, addr_t addr)
{
    if (!fname.empty()) {
        std::ofstream os{fname, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc};
        if (!os) {
            throw IOError{"Can't create PRG file: " + fname + ": " + Error::to_string()};
        }

        save(os, addr);
    }
}

std::ostream& PrgFile::save(std::ostream& os, addr_t addr)
{
    return PrgFile::save(os, (addr == 0 ? address() : addr), {data(), size()});
}

std::ostream& PrgFile::save(std::ostream& os, addr_t addr, const gsl::span<uint8_t>& data)
{
    addr_t leaddr = htole16(addr);
    if (!os.write(reinterpret_cast<char*>(&leaddr), sizeof(leaddr)) ||
        !os.write(reinterpret_cast<char*>(data.data()), data.size())) {
            throw IOError{"Can't write file data: " + Error::to_string()};
    }

    return os;
}

}
