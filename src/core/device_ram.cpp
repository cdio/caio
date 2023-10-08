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

#include <gsl/assert>

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
    using namespace gsl;
    Expects(rom.size() <= offset + size());
    std::copy(rom._data.begin(), rom._data.end(), _data.begin() + offset);
}

uint8_t DeviceRAM::read(addr_t addr, ReadMode)
{
    using namespace gsl;
    Expects(addr < _data.size());
    return _data[addr];
}

void DeviceRAM::write(addr_t addr, uint8_t data)
{
    using namespace gsl;
    Expects(addr < _data.size());
    _data[addr] = data;
}

}
