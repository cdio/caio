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
static const std::string
spectrum_to_utf8(char spec)
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
        { _ram,     0x0000 },
        { _ram,     0x4000 },
        { _rom,     0x0000 },
        { _ram,     0xC000 }
      }}
{
    ASpace::reset(_mmap, _mmap, 0xFFFF);

    /*
     * Intercept calls to $0005 (bdos)
     */
    _ram->write(5, 0xD3); /* OUT (0), A */
    _ram->write(6, 0x00);
    _ram->write(7, 0xC9); /* RET */
}

void Z80TestASpace::write(addr_t addr, uint8_t value)
{
    if (_cpu->iorq_pin()) {
        switch (addr & 255) {
        case 255:
            /*
             * out (ff), A
             */
            _out << spectrum_to_utf8(value) << std::flush;
            break;

        case 0:
            /*
             * CPM BDOS call replaced by out (0), A
             */
            bdos(_cpu->regs().C, _cpu->regs().DE());
            break;

        default:
            log.error("z80-test: Invalid out command: %04X. Ignored\n", addr);
        }

    } else {
        ASpace::write(addr, value);
    }
}

uint8_t Z80TestASpace::read(addr_t addr, ReadMode rmode)
{
    if (_cpu->iorq_pin()) {
//        if (addr == 0xFE) {
            /* Spectrum stuff, this is related to the MIC input */
            return 0xBF;
//        }
    }

    return ASpace::read(addr);
}

void Z80TestASpace::bdos(uint8_t cmd, addr_t arg)
{
    /*
     * CPM BDOS call emulator.
     */
    switch (cmd) {
    case 0: /* System reset */
        _cpu->ebreak();
        break;

    case 2: /* Console ascii character output */
        _out << static_cast<char>(arg & 255) << std::flush;
        break;

    case 9: /* Console ascii string output (ended with '$') */
        while (1) {
            char c = ASpace::read(arg);
            if (c == '$') {
                break;
            }
            _out << c << std::flush;
            ++arg;
        }
        break;

    default:
        log.error("Unsupported bdos call: %02X. Ignored\n", cmd);
    }
}

}
}
