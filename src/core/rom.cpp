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

#include "logger.hpp"
#include "utils.hpp"

namespace caio {

ROM::ROM()
    : RAM{}
{
    type(TYPE);
}

ROM::ROM(std::string_view label, const fs::Path& fname, std::string_view digest)
    : RAM{label, fname, 0}
{
    type(TYPE);
    auto sign = signature();
    if (digest != sign) {
        throw IOError{*this, "{}: Invalid signature: Expected: {}, Calculated: {}", fname.string(), digest, sign};
    }
}

ROM::ROM(std::string_view label, const fs::Path& fname, size_t size)
    : RAM{label, fname, size}
{
    type(TYPE);
    if (size > 0 && _data.size() != size) {
        throw IOError{*this, "{}: Invalid file size: It must be {}", fname.string(), std::to_string(size)};
    }
}

ROM::ROM(std::string_view label, std::istream& is, size_t count)
    : RAM{label, is, count}
{
    type(TYPE);
}

void ROM::dev_write(addr_t addr, uint8_t data)
{
#if 0
    log.warn("{}({}): Write attempt at relative address ${:04x}, data ${:02x}. Ignored\n",
        type(), label(), addr, data);
#endif
}

std::string ROM::signature() const
{
    return utils::sha256(_data);
}

}
