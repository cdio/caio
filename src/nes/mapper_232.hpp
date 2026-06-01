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
 * Cartridge Mapper 232.
 *
 * ### Block select register (8000-BFFF):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      x  x  x  |  |  x  x  x
 *               +--+----------> 64KB PRG block select
 *
 * ### Bank select register (C000-FFFF):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      x  x  x  x  x  x  |  |
 *                        +--+-> 16KB PRG page select (within the selected block)
 *
 * @see Cartridge
 * @see https://www.nesdev.org/wiki/INES_Mapper_232
 */
class Mapper_232 : public Cartridge {
public:
    constexpr static const char* TYPE = "CART_Codemasters";
    constexpr static const size_t BLOCK_SELECT_START_ADDR = PRG_BASE_ADDR;
    constexpr static const size_t BLOCK_SELECT_END_ADDR   = BLOCK_SELECT_START_ADDR + 0x4000;
    constexpr static const size_t PAGE_SELECT_START_ADDR  = BLOCK_SELECT_END_ADDR;
    constexpr static const size_t PAGE_SELECT_END_ADDR    = PAGE_SELECT_START_ADDR + 0x4000;

    /**
     * @see Cartridge::Cartridge(std::string_view, const fs::Path&, const iNES::Header& std::ifstream&)
     */
    Mapper_232(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

    void reset() override;

private:
    void bank_select(size_t addr, uint8_t value);

    size_t _block{};
    size_t _page{};

    friend Serializer& operator&(Serializer&, Mapper_232&);
};

}
}
}
