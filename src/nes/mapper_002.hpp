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
 * ### Address range accessed by the CPU:
 *
 *     Mapper Address     CPU Address     Descripton
 *     -----------------------------------------------------------------------------
 *     2000-3FFF          6000-7FFF       PRG RAM (size depends on specified value)
 *     4000-7FFF          8000-BFFF       Switchable 16K PRG ROM
 *     8000-BFFF          C000-FFFF       16K PRG ROM, fixed to the last bank
 *
 * ### Address range accessed by the PPU:
 *
 *     Mapper Address     PPU Address     Descripton
 *     -----------------------------------------------------------------------------
 *     C000-DFFF          0000-1FFF       8K CHR RAM
 *     E000-E7Ff          2000-27FF       2K VRAM
 *     E800-FFFF          2800-2FFF       2K VRAM (Nametable mirroring)
 *     F000-F7FF          3000-37FF       Mirror of 2000-27FF
 *     F800-FFFF          3800-3FFF       Mirror of 2800-2FFF
 *
 * ### Bank select register (8000-FFFF):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      x  x  x  x  +--+--+--+-> Select 16K PRG ROM bank for CPU (8000-BFFF)
 *                               (UNROM uses bits 2-0, UOROM uses bits 3-0)
 *
 * @see https://www.nesdev.org/wiki/UxROM
 */
class Mapper_002 : public Cartridge {
public:
    constexpr static const char* TYPE = "CART_UxROM";

    /**
     * @see Cartridge::Cartridge(std::string_view, const fs::Path&, const iNES::Header& std::ifstream&)
     */
    Mapper_002(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

    virtual ~Mapper_002() = default;
};

}
}
}
