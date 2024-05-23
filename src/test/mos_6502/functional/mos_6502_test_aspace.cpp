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
#include "mos_6502_test_aspace.hpp"

#include <unistd.h>

namespace caio {
namespace test {

constexpr static const addr_t KOWALSKI_OUTPUT_CHAR_ADDR = 0xF001;
constexpr static const addr_t KOWALSKI_INPUT_CHAR_ADDR  = 0xF004;

Mos6502TestASpace::Mos6502TestASpace(const sptr_t<Mos6502>& cpu, const devptr_t& ram, Readline& io)
    : _cpu{cpu},
      _ram{ram},
      _io{io},
      _mmap{{{ _ram, 0x0000 }}}
{
    ASpace::reset(_mmap, _mmap, 0xFFFF);
}

void Mos6502TestASpace::write(addr_t addr, uint8_t value)
{
    if (addr == KOWALSKI_OUTPUT_CHAR_ADDR) {
        _io.write(_cpu->regs().A);
    } else {
        ASpace::write(addr, value);
    }
}

uint8_t Mos6502TestASpace::read(addr_t addr, ReadMode rmode)
{
    return (addr == KOWALSKI_INPUT_CHAR_ADDR ? _io.getc() : ASpace::read(addr, rmode));
}

}
}
