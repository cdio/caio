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
#include "c64_io.hpp"

#include "utils.hpp"


namespace cemu {
namespace c64 {

uint8_t C64IO::read(addr_t addr) const
{
    if (addr < SID_ADDR) {
        /*
         * VIC2 registers repeat in several blocks of 64 bytes.
         * Addresses between 47 and 64 of each block contain 255 as fixed value.
         */
        addr = (addr - VIC2_ADDR) % 64;
        return ((addr < _vic2->size()) ? _vic2->read(addr) : 255);
    }

    if (addr < VCOLOR_ADDR) {
        /*
         * SID.
         */
        return _sid->read((addr - SID_ADDR) % 32);
    }

    if (addr < CIA1_ADDR) {
        /*
         * Colour RAM.
         */
        return _vcolor->read(addr - VCOLOR_ADDR);
    }

    if (addr < CIA2_ADDR) {
        /*
         * CIA1.
         */
        return _cia1->read((addr - CIA1_ADDR) % 0x10);
    }

    if (addr < RESERVED_ADDR) {
        /*
         * CIA2.
         */
        return _cia2->read((addr - CIA2_ADDR) % 0x10);
    }

    /* Reserved */
    return 255;
}

void C64IO::write(addr_t addr, uint8_t value)
{
    if (addr < SID_ADDR) {
        /*
         * VIC2 registers repeat in several blocks of 64 bytes.
         */
        addr = (addr - VIC2_ADDR) % 64;
        if (addr < _vic2->size()) {
            _vic2->write(addr, value);
        }

        return;
    }

    if (addr < VCOLOR_ADDR) {
        /*
         * SID.
         */
        return _sid->write((addr - SID_ADDR) % 32, value);
    }

    if (addr < CIA1_ADDR) {
        /*
         * Colour RAM.
         */
        _vcolor->write(addr - VCOLOR_ADDR, value);
        return;
    }

    if (addr < CIA2_ADDR) {
        /*
         * CIA1.
         */
        _cia1->write((addr - CIA1_ADDR) % 0x10, value);
        return;
    }

    if (addr < RESERVED_ADDR) {
        /*
         * CIA2.
         */
        _cia2->write((addr - CIA2_ADDR) % 0x10, value);
        return;
    }
}

std::ostream &C64IO::dump(std::ostream &os, addr_t base) const
{
    /*
     * $D000 - $D02E -- $0000 - $002E   VIC2 registers
     * $D02F - $D03F -- $002F - $003F   Fixed #$FF
     * $D040 - $D3FF -- $0040 - $03FF   Previous 2 repeated as 64 bytes blocks
     * $D400 - $D7FF -- $0400 - $07FF   SID registers (repeated as 32 bytes blocks)
     * $D800 - $DBFF -- $0800 - $0BFF   Color RAM (only low nibbles are settable)
     * $DC00 - $DCFF -- $0C00 - $0CFF   CIA #1 registers
     * $DD00 - $DDFF -- $0D00 - $0DFF   CIA #2 registers
     * $DE00 - $DEFF -- $0E00 - $0EFF   Reserved for future I/O expansion
     * $DF00 - $DFFF -- $0F00 - $0FFF   Reserved for future I/O expansion
     */
    constexpr static const addr_t ff_area_size = 0x40 - 0x2f;
    static const std::array<uint8_t, SIZE - RESERVED_ADDR> reserved{255};

    for (addr_t offset = VIC2_ADDR; offset < SID_ADDR; offset += 0x40) {
        _vic2->dump(os, base + offset);
        utils::dump(os, reserved.begin(), reserved.begin() + ff_area_size, base + offset + _cia2->size());
    }

    for (addr_t offset = 0; offset < 1024; offset += 32) {
        _sid->dump(os, base + SID_ADDR + offset);
    }

    _vcolor->dump(os, base + VCOLOR_ADDR);
    _cia1->dump(os, base + CIA1_ADDR);
    _cia2->dump(os, base + CIA2_ADDR);
    utils::dump(os, reserved.begin(), reserved.end(), base + RESERVED_ADDR);

    return os;
}

}
}
