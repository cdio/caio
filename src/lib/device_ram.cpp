/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include "device_ram.hpp"

#include "types.hpp"
#include "utils.hpp"


namespace caio {

DeviceRAM::DeviceRAM(const std::string &label, size_t size)
    : Device{TYPE, label},
      _data(size)
{
}

DeviceRAM::DeviceRAM(size_t size)
    : DeviceRAM{{}, size}
{
}

DeviceRAM::DeviceRAM(const std::string &label, const std::vector<uint8_t> &data)
    : Device{TYPE, label},
      _data(data)
{
}

DeviceRAM::DeviceRAM(const std::vector<uint8_t> &data)
    : DeviceRAM{{}, data}
{
}

DeviceRAM::~DeviceRAM()
{
}

void DeviceRAM::reset()
{
}

size_t DeviceRAM::size() const
{
    return _data.size();
}

uint8_t DeviceRAM::read(addr_t addr) const
{
    if (addr < _data.size()) {
        return _data[addr];
    }

    throw InvalidReadAddress{*this, addr};
}

void DeviceRAM::write(addr_t addr, uint8_t data)
{
    if (addr < _data.size()) {
        _data[addr] = data;
    } else {
        throw InvalidWriteAddress{*this, addr};
    }
}

std::ostream &DeviceRAM::dump(std::ostream &os, addr_t base) const
{
    return utils::dump(os, _data, base);
}

}
