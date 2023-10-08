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
#include "zilog_z80.hpp"


namespace caio {
namespace zilog {

uint8_t Z80::io_in(uint8_t port)
{
    iorq_pin(true);
    auto value = read(port);
    iorq_pin(false);
    return value;
}

void Z80::io_out(uint8_t port, uint8_t value)
{
    iorq_pin(true);
    write(port, value);
    iorq_pin(false);
}

int Z80::i_IN_A_n(Z80& self, uint8_t op, addr_t arg)
{
    self._regs.memptr = (static_cast<uint16_t>(self._regs.A) << 8) + arg + 1;
    self._regs.A = self.io_in(arg);
    return 0;
}

int Z80::i_OUT_n_A(Z80& self, uint8_t op, addr_t arg)
{
    self._regs.memptr = (static_cast<uint16_t>(self._regs.A) << 8) + ((arg + 1) & 255);
    self.io_out(arg & 255, self._regs.A);
    return 0;
}

}
}
