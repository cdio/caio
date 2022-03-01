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
#include "device_gpio.hpp"

#include <algorithm>

#include "utils.hpp"


namespace cemu {

DeviceGpio::DeviceGpio(const std::string &label, addr_t banks)
    : Device{TYPE, label},
     _size{static_cast<unsigned>(banks * BSIZE)},
     _mask{static_cast<addr_t>(_size - 1)},
     _ports(banks)
{
}

DeviceGpio::~DeviceGpio()
{
}

size_t DeviceGpio::size() const
{
    return _size;
}

uint8_t DeviceGpio::read(addr_t addr) const
{
    auto [bank, offset] = decode(addr);
    return _ports[bank].ior(offset);
}

void DeviceGpio::write(addr_t addr, uint8_t value)
{
    auto [bank, offset] = decode(addr);
    return _ports[bank].iow(offset, value);
}

std::ostream &DeviceGpio::dump(std::ostream &os, addr_t base) const
{
    std::vector<uint8_t> data(_size);
    addr_t addr{};

    std::generate(data.begin(), data.end(), [this, &addr]() {
        return read(addr++);
    });

    return utils::dump(os, data, base);
}

void DeviceGpio::add_ior(const ior_t &ior, addr_t bank, uint8_t mask)
{
    try {
        _ports.at(bank).add_ior(ior, mask);
    } catch (const std::out_of_range &) {
        throw InvalidArgument{"Invalid bank: " + std::to_string(bank)};
    }
}

void DeviceGpio::add_iow(const iow_t &iow, addr_t bank, uint8_t mask)
{
    try {
        _ports.at(bank).add_iow(iow, mask);
    } catch (const std::out_of_range &) {
        throw InvalidArgument{"Invalid bank: " + std::to_string(bank)};
    }
}

inline std::pair<addr_t, uint8_t> DeviceGpio::decode(addr_t addr) const
{
    return {(addr & _mask) >> 8, addr & BSIZE};
}

}