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
#include "z80_test_aspace.hpp"

namespace caio {
namespace test {

//FIXME
static const std::string spectrum_to_utf8(char spec)
{
    switch (spec) {
    case 0x0D: return "\r\n";
    case 0x09: return " ";
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16: break;
    case 0x17: return "\t";
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E: break;
    case 0x1F: return "";
    case 0x5E: return "^";
    case 0x60: return "£";
    case 0x7F: return "©";
    default:;
    }

    return std::string{spec};
}

Z80TestASpace::Z80TestASpace(const sptr_t<Z80>& cpu, const devptr_t& ram, const devptr_t& rom, std::ostream& out)
    : _cpu{cpu},
      _ram{ram},
      _rom{rom},
      _out{out},
      _mmap{{
        { _ram, 0x0000 },
        { _ram, 0x4000 },
        { _rom, 0x0000 },
        { _ram, 0xC000 }
      }}
{
    ASpace::reset(_mmap, _mmap, 0xFFFF);
}

uint8_t Z80TestASpace::read(addr_t addr, ReadMode rmode)
{
    if (_cpu->iorq_pin()) {
//        if (addr == 0xFE) {
            /* Spectrum stuff, this is related to the MIC input */
            return 0xBF;
//        }
    }

    return ASpace::read(addr, rmode);
}

void Z80TestASpace::write(addr_t addr, uint8_t value)
{
    if (_cpu->iorq_pin()) {
        if ((addr & 255) == 255) {
            /*
             * out (ff), A
             */
            _out << spectrum_to_utf8(value) << std::flush;
        }
    } else {
        ASpace::write(addr, value);
    }
}

}
}
