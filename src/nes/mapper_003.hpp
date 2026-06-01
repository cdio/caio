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
#pragma once

#include "nes_cartridge.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/**
 * Cartridge Mapper 003 (CNROM).
 *
 * ### Characteristics:
 *
 *   - CPU 6000-7FFF: 2 KB of PRG-RAM, mirrored three times (Hayauchi Super Igo only)
 *   - CPU 8000-FFFF: 32 KB unbanked PRG-ROM
 *   - PPU 0000-1FFF: 8 KB switchable window into 32 KB CHR-ROM
 *   - Nametable arrangement: Fixed; solder pad selects between Horizontal and Vertical
 *
 * ### Bank select register (8000-FFFF):
 *
 *   D7 D6 D5 D4 D3 D2 D1 D0
 *    |  |  |  |  |  |  |  |
 *    x  x  x  x  +--+--+--+-> Up to 128K CHR 8K bank.
 *
 * @see Cartridge
 * @see https://www.nesdev.org/wiki/CNROM
 */
class Mapper_003 : public Cartridge {
public:
    constexpr static const char* TYPE              = "CART_CNROM";
    constexpr static const size_t M3_ROM_SIZE      = 32768;
    constexpr static const size_t M3_CHR_BANK_SIZE = 8192;

    /**
     * @see Cartridge::Cartridge(std::string_view, const fs::Path&, const iNES::Header& std::ifstream&)
     */
    Mapper_003(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

private:
    void bank_select(size_t addr, uint8_t value);
};

}
}
}
