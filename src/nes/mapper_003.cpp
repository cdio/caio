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
#include "mapper_003.hpp"

namespace caio {
namespace nintendo {
namespace nes {

Mapper_003::Mapper_003(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Cartridge{TYPE, fname, hdr, is}
{
    const size_t prg_size = hdr.prg_size();

    if (prg_size != M3_ROM_SIZE) {
        throw InvalidCartridge{"{}: Invalid PRG ROM size: {}. It must be {}",
            fname.c_str(), prg_size, M3_ROM_SIZE};
    }

    using namespace std::placeholders;
    write_observer(std::bind(&Mapper_003::bank_select, this, _1, _2));
}

void Mapper_003::bank_select(size_t addr, uint8_t value)
{
    if (addr >= PRG_BASE_ADDR && addr < PRG_BASE_ADDR + PRG_ASPACE_SIZE) {
        /*
         * CHR bank select register:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    x  x  x  x  +--+--+--+-> Up to 128K CHR 8K bank.
         *
         * Submapper 0: Bus conflict behavior unknown
         * Submapper 1: No bus conflicts
         * Submapper 2: AND-type bus conflicts
         *
         * The original CNROM board is always subject to AND-type bus conflicts:
         * the effective value is the value being written bitwise-AND'd with the
         * PRG-ROM content at the address being written to.
         * iNES Mapper 3's submapper indicates whether bus conflicts should
         * actually be emulated.
         */
        const auto pvalue = _prg.read(addr - PRG_BASE_ADDR, ReadMode::Peek);
        const size_t bank = (value & pvalue) << 3;

        _chr_banks[0].bank(bank);
        _chr_banks[1].bank(bank + 1);
        _chr_banks[2].bank(bank + 2);
        _chr_banks[3].bank(bank + 3);
        _chr_banks[4].bank(bank + 4);
        _chr_banks[5].bank(bank + 5);
        _chr_banks[6].bank(bank + 6);
        _chr_banks[7].bank(bank + 7);
    }
}

}
}
}
