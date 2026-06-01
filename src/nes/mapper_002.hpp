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
 * Cartridge Mapper 002 (UxROM).
 *
 * ### Characteristics:
 *
 *   - PRG ROM capacity 256K/4096K
 *   - PRG ROM window 16K + 16K fixed
 *   - PRG RAM capacity None
 *   - CHR capacity 8K
 *
 *   - CPU 8000-BFFF: 16 KB switchable PRG ROM bank
 *   - CPU C000-FFFF: 16 KB PRG ROM bank, fixed to the last bank
 *
 * ### Bank select register (8000-FFFF):
 *
 *   D7 D6 D5 D4 D3 D2 D1 D0
 *    |  |  |  |  |  |  |  |
 *    x  x  x  x  +--+--+--+-> Select 16K PRG ROM bank for CPU (8000-BFFF)
 *                             (UNROM uses bits 2-0, UOROM uses bits 3-0)
 *
 * Emulator implementations of iNES mapper 2 treat this as a
 * full 8-bit bank select register, without bus conflicts.
 *
 * @see Cartridge
 * @see https://www.nesdev.org/wiki/UxROM
 */
class Mapper_002 : public Cartridge {
public:
    constexpr static const char* TYPE = "CART_UxROM";

    /**
     * @see Cartridge::Cartridge(std::string_view, const fs::Path&, const iNES::Header& std::ifstream&)
     */
    Mapper_002(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

private:
    void bank_select(size_t addr, uint8_t value);

    size_t _bmask;
};

}
}
}
