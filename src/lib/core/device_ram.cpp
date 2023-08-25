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
#include "device_ram.hpp"

#include "types.hpp"
#include "utils.hpp"


namespace caio {

DeviceRAM::DeviceRAM(const std::string& label, size_t size, const init_cb_t& initcb)
    : Device{TYPE, label},
      _data(size),
      _initcb{initcb}
{
    if (_initcb) {
        gsl::span data{_data.data(), _data.size()};
        _initcb(data);
    }
}

void DeviceRAM::copy(const DeviceROM& rom, size_t offset)
{
    if (offset + rom.size() > size()) {
        throw Error{*this, "Can't copy: ROM at offset exceeds RAM size"};
    }

    std::copy(rom._data.begin(), rom._data.end(), _data.begin() + offset);
}

uint8_t DeviceRAM::read(addr_t addr, ReadMode)
{
    if (addr < _data.size()) {
        return _data[addr];
    }

    throw InvalidReadAddress{*this, addr};
}

void DeviceRAM::write(addr_t addr, uint8_t data)
{
    if (addr >= _data.size()) {
        throw InvalidWriteAddress{*this, addr};
    }

    _data[addr] = data;
}

}
