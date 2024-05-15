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
#include "rom.hpp"

#include <fstream>

#include "fs.hpp"
#include "logger.hpp"
#include "utils.hpp"

namespace caio {

ROM::ROM(std::string_view fname, std::string_view digest, std::string_view label)
    : RAM{fname, 0, label}
{
    type(TYPE);
    auto sign = signature();
    if (digest != sign) {
        throw IOError{*this, "{}: Invalid signature: Expected: {}, Calculated: {}", fname, digest, sign};
    }
}

ROM::ROM(std::string_view fname, size_t size, std::string_view label)
    : RAM{fname, size, label}
{
    type(TYPE);
    if (size > 0 && _data.size() != size) {
        throw IOError{*this, "{}: Invalid file size: It must be {}", fname, std::to_string(size)};
    }
}

ROM::ROM(std::istream& is, size_t count)
    : RAM{is, count}
{
    type(TYPE);
}

void ROM::write(addr_t addr, uint8_t data)
{
#if 0
    log.warn("{}({}): Write attempt at relative address ${:04x}, data ${:02x}. Ignored\n",
        type(), label(), addr, data);
#endif
}

std::string ROM::signature() const
{
    return caio::sha256(_data);
}

}
