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
#include "device.hpp"

namespace caio {

uint8_t Device::read(addr_t addr, ReadMode mode)
{
    if (_read_cb) {
        _read_cb(addr, mode);
    }

    return dev_read(addr, mode);
}

void Device::write(addr_t addr, uint8_t data)
{
    if (_write_cb) {
        _write_cb(addr, data);
    }

    dev_write(addr, data);
}

void Device::read_observer(const ReadObserverCb& cb)
{
    _read_cb = cb;
}

void Device::write_observer(const WriteObserverCb& cb)
{
    _write_cb = cb;
}

std::string Device::to_string() const
{
    return std::format("{}, size {}", Name::to_string(), size());
}

}
