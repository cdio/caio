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
#include "c64_aspace.hpp"

#include "device_none.hpp"
#include "mos_6510.hpp"
#include "types.hpp"


namespace cemu {
namespace c64 {

void C64ASpace::reset(devptr_t ram, devptr_t basic, devptr_t kernal, devptr_t chargen, devptr_t io, devptr_t cart)
{
    if (!cart) {
        cart = none;
    }

    /*
     * Fixed mappings to accelerate bank switching.
     *
     * Mappigns source: https://www.c64-wiki.com/wiki/Bank_Switching.
     */
    auto mode_00 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { ram,      0xA000 },
        { ram,      0xB000 },
        { ram,      0xC000 },
        { ram,      0xD000 },
        { ram,      0xE000 },
        { ram,      0xF000 }
    });

    auto mode_01 = mode_00;

    auto mode_02 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { cart,     0x2000 },
        { cart,     0x3000 },
        { ram,      0xC000 },
        { chargen,  0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_03 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { cart,     0x0000 },
        { cart,     0x1000 },
        { cart,     0x2000 },
        { cart,     0x3000 },
        { ram,      0xC000 },
        { chargen,  0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_04 = mode_00;

    auto mode_05 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { ram,      0xA000 },
        { ram,      0xB000 },
        { ram,      0xC000 },
        { io,       0x0000 },
        { ram,      0xE000 },
        { ram,      0xF000 },
    });

    auto mode_06 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { cart,     0x2000 },
        { cart,     0x3000 },
        { ram,      0xC000 },
        { io,       0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_07 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { cart,     0x0000 },
        { cart,     0x1000 },
        { cart,     0x2000 },
        { cart,     0x3000 },
        { ram,      0xC000 },
        { io,       0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_08 = mode_00;

    auto mode_09 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { ram,      0xA000 },
        { ram,      0xB000 },
        { ram,      0xC000 },
        { chargen,  0x0000 },
        { ram,      0xE000 },
        { ram,      0xF000 },
    });

    auto mode_10 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { ram,      0xA000 },
        { ram,      0xB000 },
        { ram,      0xC000 },
        { chargen,  0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_11 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { cart,     0x0000 },
        { cart,     0x1000 },
        { basic,    0x0000 },
        { basic,    0x1000 },
        { ram,      0xC000 },
        { chargen,  0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_12 = mode_00;
    auto mode_13 = mode_05;

    auto mode_14 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { ram,      0xA000 },
        { ram,      0xB000 },
        { ram,      0xC000 },
        { io,       0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_15 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { cart,     0x0000 },
        { cart,     0x1000 },
        { basic,    0x0000 },
        { basic,    0x1000 },
        { ram,      0xC000 },
        { io,       0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_16 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { cart,     0x0000 },
        { cart,     0x1000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { io,       0x0000 },
        { cart,     0x2000 },
        { cart,     0x3000 },
    });

    auto mode_17 = mode_16;
    auto mode_18 = mode_16;
    auto mode_19 = mode_16;
    auto mode_20 = mode_16;
    auto mode_21 = mode_16;
    auto mode_22 = mode_16;
    auto mode_23 = mode_16;
    auto mode_24 = mode_00;
    auto mode_25 = mode_09;
    auto mode_26 = mode_10;

    auto mode_27 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { basic,    0x0000 },
        { basic,    0x1000 },
        { ram,      0xC000 },
        { chargen,  0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    auto mode_28 = mode_00;
    auto mode_29 = mode_05;
    auto mode_30 = mode_14;

    auto mode_31 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { basic,    0x0000 },
        { basic,    0x1000 },
        { ram,      0xC000 },
        { io,       0x0000 },
        { kernal,   0x0000 },
        { kernal,   0x1000 },
    });

    _rmodes = std::array<addrmap_t, 32>{
        mode_00, mode_01, mode_02, mode_03, mode_04, mode_05, mode_06, mode_07,
        mode_08, mode_09, mode_10, mode_11, mode_12, mode_13, mode_14, mode_15,
        mode_16, mode_17, mode_18, mode_19, mode_20, mode_21, mode_22, mode_23,
        mode_24, mode_25, mode_26, mode_27, mode_28, mode_29, mode_30, mode_31
    };

    auto wmode_00 = mode_00;
    auto wmode_01 = mode_00;
    auto wmode_02 = mode_00;
    auto wmode_03 = mode_00;
    auto wmode_04 = mode_00;
    auto wmode_05 = mode_13;
    auto wmode_06 = mode_13;
    auto wmode_07 = mode_13;
    auto wmode_08 = mode_00;
    auto wmode_09 = mode_00;
    auto wmode_10 = mode_00;
    auto wmode_11 = mode_00;
    auto wmode_12 = mode_00;
    auto wmode_13 = mode_13;
    auto wmode_14 = mode_13;
    auto wmode_15 = mode_13;

    auto wmode_16 = addrmap_t(new std::vector<devmap_t>{
        { ram,      0x0000 },
        { ram,      0x1000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { ram,      0x8000 },
        { ram,      0x9000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { none,     0x0000 },
        { io,       0x0000 },
        { ram,      0xE000 },
        { ram,      0xF000 },
    });

    auto wmode_17 = wmode_16;
    auto wmode_18 = wmode_16;
    auto wmode_19 = wmode_16;
    auto wmode_20 = wmode_16;
    auto wmode_21 = wmode_16;
    auto wmode_22 = wmode_16;
    auto wmode_23 = wmode_16;
    auto wmode_24 = mode_00;
    auto wmode_25 = mode_00;
    auto wmode_26 = mode_00;
    auto wmode_27 = mode_00;
    auto wmode_28 = mode_00;
    auto wmode_29 = mode_13;
    auto wmode_30 = mode_13;
    auto wmode_31 = mode_13;

    _wmodes = std::array<addrmap_t, 32>{
        wmode_00, wmode_01, wmode_02, wmode_03, wmode_04, wmode_05, wmode_06, wmode_07,
        wmode_08, wmode_09, wmode_10, wmode_11, wmode_12, wmode_13, wmode_14, wmode_15,
        wmode_16, wmode_17, wmode_18, wmode_19, wmode_20, wmode_21, wmode_22, wmode_23,
        wmode_24, wmode_25, wmode_26, wmode_27, wmode_28, wmode_29, wmode_30, wmode_31
    };

    _port0 = LORAM | HIRAM | CHAREN | CASSETTE_DATA_OUT | CASSETTE_MOTOR_CONTROL;
    _port1 = LORAM | HIRAM | CHAREN | CASSETTE_MOTOR_CONTROL | GAME | EXROM;

    if (cart) {
        /*
         * ROM Cartridge configurations:
         *
         * 8K cartridge:  GAME = 1, EXROM = 0, ROML = 0, ROMH = 1: ROM-LO at $8000/$9FFF
         * 16K cartridge: GAME = 0, EXROM = 0, ROML = 0, ROMH = 0: ROM-LO at $8000/$9FFF, ROM-HI at $A000/$BFFFF
         *
         * Ultimax Modes:
         * 8K cartridge:  GAME = 0, EXROM = 1, ROML = 1, ROMH = 0: ROM-LO at $E000/$FFFF
         * 16K cartridge: GAME = 0, EXROM = 1, ROML = 0, ROMH = 0: ROM-LO at $8000/$AFFF, ROM-HI at $E000/$FFFF
         *
         * Source: http://blog.worldofjani.com/?p=879
         *
         * This implementation does not support ultimax mode (ROMH and ROML expansion port lines are not considered),
         * mappings for these modes (16 to 23) are defined, though.
         */
        switch (cart->size()) {
        case 8192:
            _port1 &= ~EXROM;
            break;
        case 16384:
            _port1 &= ~(GAME | EXROM);
            break;
        default:;
        }
    }

    remap();
    ASpace::write(Mos6510::PORT_0, _port0);
    ASpace::write(Mos6510::PORT_1, _port1 & 255);
}

void C64ASpace::write(addr_t addr, uint8_t value)
{
    switch (addr) {
    case Mos6510::PORT_0:
        _port0 = value;
        break;
    case Mos6510::PORT_1:
        _port1 = (value & _port0) | (_port1 & (GAME | EXROM));
        remap();    /* Can remap() before ASpace::write() because addresses < 0x1000 are always in ram */
        break;
    default:;
    }

    ASpace::write(addr, value);
}

void C64ASpace::write_addr(addr_t addr, addr_t value)
{
    if (addr <= Mos6510::PORT_1) {
        write(addr, value & 0xff);
        write(addr + 1, (value >> 8));
    } else {
        ASpace::write(addr, value);
    }
}

}
}
