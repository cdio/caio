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
#include "ram.hpp"

#include "types.hpp"

namespace caio {

RAM::RAM()
    : Device{TYPE, {}},
      _data{}
{
}

RAM::RAM(std::string_view label, size_t size)
    : Device{TYPE, label},
      _data(size)
{
}

RAM::RAM(std::string_view label, const fs::Path& fname, size_t count)
    : Device{TYPE, label},
      _data{fs::load(fname, count)}
{
}

RAM::RAM(std::string_view label, std::istream& is, size_t count)
    : Device{TYPE, label},
      _data{fs::load(is, count)}
{
    if (_data.size() != count) {
        throw IOError{"Unexpected EOF: Read bytes: {}, required: {}", _data.size(), count};
    }
}

RAM::RAM(RAM&& other)
    : Device{other.type(), other.label()},
      _data{std::move(other._data)}
{
}

RAM::~RAM()
{
}

RAM& RAM::operator=(RAM&& other)
{
    type(other.type());
    label(other.label());
    _data = std::move(other._data);
    return *this;
}

uint8_t RAM::dev_read(size_t addr, ReadMode)
{
    return _data[addr];
}

void RAM::dev_write(size_t addr, uint8_t data)
{
    _data[addr] = data;
}

std::ostream& RAM::dump(std::ostream& os, size_t base) const
{
    return utils::dump(os, _data, base);
}

}
