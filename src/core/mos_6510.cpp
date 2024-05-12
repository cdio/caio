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
#include "mos_6510.hpp"

namespace caio {

uint8_t Mos6510::read(addr_t addr, Device::ReadMode mode)
{
    switch (addr) {
    case PORT_0:
        return _iodir;

    case PORT_1:
        return _ioport.ior(0);

    default:;
    }

    return Mos6502::read(addr, mode);
}

void Mos6510::write(addr_t addr, uint8_t value)
{
    switch (addr) {
    case PORT_0:
        _iodir = value;
        return;

    case PORT_1:
        value = (value & _iodir) | (_ioport.ior(0) & ~_iodir);
        _ioport.iow(0, value);
        return;

    default:;
    }

    Mos6502::write(addr, value);
}

}
