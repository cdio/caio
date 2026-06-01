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
#include "mapper_071.hpp"

namespace caio {
namespace nintendo {
namespace nes {

Mapper_071::Mapper_071(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Cartridge{TYPE, fname, hdr, is}
{
    using namespace std::placeholders;
    write_observer(std::bind(&Mapper_071::bank_select, this, _1, _2));
}

void Mapper_071::bank_select(size_t addr, uint8_t value)
{
    if (addr >= MIRROR_REG_ADDR && addr < MIRROR_REG_END_ADDR) {
        /*
         * "Fire Hawk only writes this register at the address $9000,
         *  and other games like Micro Machines and Ultimate Stuntman
         *  write $00 to $8000 on startup. For compatibility without
         *  using a submapper, FCEUX begins all games with fixed mirroring,
         *  and applies single screen mirroring only once $9000-9FFF
         *  is written, ignoring writes to $8000-8FFF."
         */
        const auto type = ((value & D4) ? MirrorType::OneScreenLower : MirrorType::OneScreenUpper);
        vram_mirror(type);

    } else if (addr >= BANKSEL_REG_START_ADDR && addr < BANKSEL_REG_END_ADDR) {
        /*
         * Bank Select register (CPU C000-FFFF):
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    x  x  x  x  +--+--+--+--> Select 16 KB PRG ROM bank for CPU $8000-$BFFF
         */
        const size_t bank = (value & (D3 | D2 | D1 | D0)) << 1;
        _prg_banks[0].bank(bank);
        _prg_banks[1].bank(bank + 1);
    }
}

}
}
}
