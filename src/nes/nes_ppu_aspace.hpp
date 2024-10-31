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

#include "aspace.hpp"

#include "nes_cartridge.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/**
 * NES PPU address space.
 *
 * ### Address space:
 *
 *   PPU Address Range  Size    Description             Provider
 *   ------------------------------------------------------------------------
 *   0000-0FFF          1000    Pattern table 0 (CHR)   Cartridge (*1)
 *   1000-1FFF          1000    Pattern table 1 (CHR)   Cartridge (*1)
 *   2000-23BF          0400    Nametable 0             VRAM / Cartridge (*2)
 *   2400-27FF          0400    Nametable 1             VRAM / Cartridge (*2)
 *   2800-2BFF          0400    Nametable 2             VRAM / Cartridge (*2)
 *   2C00-2FFF          0400    Nametable 3             VRAM / Cartridge (*2)
 *   3000-3EFF          0F00    Unused                  Cartridge
 *   3F00-3F1F          0020    Palette RAM indexes     Internal to PPU
 *   3F20-3FFF          00E0    Mirror of 3F00-3F1F     Internal to PPU
 *
 * (*1) CHR 8K RAM or ROM
 * (*2) 2K VRAM on NES board is mapped on these addresses but the cartridge
 *      is able to redirect these lines.
 *
 * @see ASpace
 */
class NESPPUASpace : public ASpace {
public:
    constexpr static const addr_t ADDR_MASK = 0x3FFF;
    constexpr static const size_t BLOCKS    = 8;

    /**
     * Initialise this address space.
     * @param cart Cartridge.
     */
    NESPPUASpace(const sptr_t<Cartridge>& cart);

    virtual ~NESPPUASpace();

private:
    bank_t<BLOCKS> _mmap;
};

}
}
}
