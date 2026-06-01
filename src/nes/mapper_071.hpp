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
 * Cartridge Mapper 071.
 *
 * ### Banks:
 *
 *     8000-BFFF: Switchable
 *     C000-FFFF: Fixed to last bank
 *
 * ### Mirror register (8000-9FFF):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      x  x  x  |  x  x  x  x
 *               +-------------> Select 1 KB CIRAM bank for PPU (2000-2FFF)
 *
 * "Fire Hawk only writes this register at the address $9000,
 *  and other games like Micro Machines and Ultimate Stuntman
 *  write $00 to $8000 on startup. For compatibility without
 *  using a submapper, FCEUX begins all games with fixed mirroring,
 *  and applies single screen mirroring only once $9000-9FFF
 *  is written, ignoring writes to $8000-8FFF."
 *
 * ### Bank select register (C000-FFFF):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      x  x  x  x  +--+--+--+-> Select 16K PRG ROM bank for CPU (8000-BFFF)
 *
 * @see Cartridge
 * @see https://www.nesdev.org/wiki/INES_Mapper_071
 */
class Mapper_071 : public Cartridge {
public:
    constexpr static const char* TYPE                    = "CART_Codemasters";
    constexpr static const addr_t MIRROR_REG_START_ADDR  = PRG_BASE_ADDR;
    constexpr static const addr_t MIRROR_REG_END_ADDR    = MIRROR_REG_START_ADDR + 0x2000;
    constexpr static const addr_t MIRROR_REG_ADDR        = PRG_BASE_ADDR + 0x1000;
    constexpr static const addr_t BANKSEL_REG_START_ADDR = PRG_BASE_ADDR + 0x4000;
    constexpr static const addr_t BANKSEL_REG_END_ADDR   = BANKSEL_REG_START_ADDR + 0x4000;

    /**
     * @see Cartridge::Cartridge(std::string_view, const fs::Path&, const iNES::Header& std::ifstream&)
     */
    Mapper_071(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

private:
    void bank_select(size_t addr, uint8_t value);
};

}
}
}
