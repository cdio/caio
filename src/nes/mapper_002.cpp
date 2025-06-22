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
    : Cartridge{TYPE, fname, hdr, is}
{
    const auto chr_size = hdr.chr_size();
    const auto prg_size = hdr.prg_size();

    if (chr_size != 0) {
        throw InvalidCartridge{"{}: Invalid CHR ROM size: {}. It must be 0", fname.c_str(), chr_size};
    }

    if (prg_size < PRG_BANK_SIZE || (prg_size & PRG_BANK_MASK) != 0) {
        throw InvalidCartridge{"{}: Invalid PRG ROM size: {}. It must be a multiple of {}K",
            fname.c_str(), prg_size, PRG_BANK_SIZE / 1024};
    }

    const auto lo_bank_switcher = [this](size_t addr, uint8_t value) {
        if (addr >= PRG_LO_BASE && addr < PPU_OFFSET) {
            /*
             * PRG access: 4000-BFFF (cpu 8000-FFFF).
             * Bank switching: The written value is the selected bank.
             */
            _prg_lb.bank(value);
        }
    };

    write_observer(lo_bank_switcher);
}

Mapper_002::~Mapper_002()
{
}

}
}
}
