/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "device.hpp"


namespace cemu {

std::string Device::to_string() const
{
    std::ostringstream os{};

    os << Name::to_string() << ", size " << size();

    return os.str();
}

addr_t Device::read_addr(size_t addr, bool is_le) const
{
    uint8_t lo = (is_le ? read(addr) : read(addr + 1));
    uint8_t hi = (is_le ? read(addr + 1) : read(addr));
    addr_t  val = (static_cast<addr_t>(hi) << 8) | lo;

    return val;
}

void Device::write_addr(addr_t addr, addr_t data, bool is_le)
{
    uint8_t lo = static_cast<uint8_t>(data & 0xFF);
    uint8_t hi = static_cast<uint8_t>((data >> 8) & 0xFF);

    if (is_le) {
        write(addr, lo);
        write(addr + 1, hi);
    } else {
        write(addr, hi);
        write(addr + 1, lo);
    }
}

}
