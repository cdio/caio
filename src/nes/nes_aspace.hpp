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
#include "ram.hpp"
#include "ricoh_2c02.hpp"

#include "nes_cartridge.hpp"

#include <string_view>

namespace caio {
namespace nintendo {
namespace nes {

/**
 * NES address space.
 *
 * ### Address space:
 *
 *   CPU Address Range  Size    Description             Provider
 *   ------------------------------------------------------------------
 *   0000-07FF          800     2K RAM                  NES board
 *   0800-0FFF          800     Mirror of 0000-07FF
 *   1000-17FF          800     Mirror of 0000-07FF
 *   1800-1FFF          800     Mirror of 0000-07FF
 *   2000-2007          8       PPU registers           PPU
 *   2008-3FFF          1FF8    Mirror of 2000-2007
 *   4000-401F          20      APU/IO registers        Internal to CPU
 *   4020-5FFF          1FE0    Cartridge defined       Cartridge
 *   6000-7FFF          2000    8K RAM (usually)        Cartridge
 *   8000-FFFF          8000    32K ROM (usually)       Cartridge
 *
 * @see ASpace
 * @see NESCartridge
 * @see RAM
 * @see RP2C02
 */
class NESASpace : public ASpace {
public:
    constexpr static const char* TYPE       = "NES-ASPACE";
    constexpr static const addr_t ADDR_MASK = 0xFFFF;
    constexpr static const size_t BLOCKS    = 32;       /* 2K sized blocks */

    /**
     * Initialise this address space.
     * @param label Label assigned to this address space.
     * @param ram   Board's RAM (2K);
     * @param ppu   PPU;
     * @param cart  Cartridge.
     */
    NESASpace(std::string_view label, const sptr_t<RAM>& ram, const sptr_t<RP2C02>& ppu, const sptr_t<Cartridge>& cart);

    virtual ~NESASpace() = default;

private:
    bank_t<BLOCKS> _mmap;
};

}
}
}
