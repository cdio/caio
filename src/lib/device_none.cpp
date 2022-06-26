/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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
#include "device_none.hpp"

#include <memory>


namespace caio {

devptr_t device_none = std::make_shared<DeviceNone>();

DeviceNone::DeviceNone()
    : Device{TYPE, TYPE}
{
}

DeviceNone::~DeviceNone()
{
}

void DeviceNone::reset()
{
}

size_t DeviceNone::size() const
{
    return 0;
}

uint8_t DeviceNone::read(addr_t) const
{
    return 0;
}

void DeviceNone::write(addr_t, uint8_t)
{
}

std::ostream &DeviceNone::dump(std::ostream &os, addr_t) const
{
    return os;
}

}
