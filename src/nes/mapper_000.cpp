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
#include "mapper_000.hpp"

namespace caio {
namespace nintendo {
namespace nes {

Mapper_000::Mapper_000(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Cartridge{TYPE, fname, hdr, is}
{
    const size_t prg_size = hdr.prg_size();
    const size_t chr_size = hdr.chr_size();

    if (prg_size != 32768 && prg_size != 16384) {
        throw InvalidCartridge{"{}: Invalid PRG ROM size: {}. It must be 32K or 16K\n{}",
            fname.c_str(), prg_size, iNES::to_string(hdr)};
    }

    if (chr_size != 0 && chr_size != 8192) {
        throw InvalidCartridge{"{}: Invalid CHR ROM size: {}. It must be 8K\n{}",
            fname.c_str(), chr_size, iNES::to_string(hdr)};
    }
}

void Mapper_000::reset()
{
    Cartridge::reset();
}

}
}
}
