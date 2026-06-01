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
#include "mapper_002.hpp"

namespace caio {
namespace nintendo {
namespace nes {

Mapper_002::Mapper_002(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Cartridge{TYPE, fname, hdr, is},
      _bmask{(_prg.size() >> 13) - 1}
{
    using namespace std::placeholders;
    write_observer(std::bind(&Mapper_002::bank_select, this, _1, _2));
}

void Mapper_002::bank_select(size_t addr, uint8_t value)
{
    if (addr >= PRG_BASE_ADDR && addr < PRG_BASE_ADDR + PRG_ASPACE_SIZE) {
        /*
         * PRG bank select register: 4000-BFFF (CPU 8000-FFFF).
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    x  x  x  x  +--+--+--+-> Select 16K PRG ROM bank for CPU (8000-BFFF)
         *                             (UNROM uses bits 2-0, UOROM uses bits 3-0)
         */
        const size_t bank = (value << 1) & _bmask;
        _prg_banks[0].bank(bank);
        _prg_banks[1].bank(bank + 1);
    }
}

}
}
}
