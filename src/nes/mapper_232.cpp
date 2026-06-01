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
#include "mapper_232.hpp"

namespace caio {
namespace nintendo {
namespace nes {

Mapper_232::Mapper_232(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Cartridge{TYPE, fname, hdr, is}
{
    using namespace std::placeholders;
    write_observer(std::bind(&Mapper_232::bank_select, this, _1, _2));
    Mapper_232::reset();
}

void Mapper_232::reset()
{
    Cartridge::reset();
    _block = 0;
    _page = 0;
    _prg_banks[2].bank(6);
    _prg_banks[3].bank(7);
}

void Mapper_232::bank_select(size_t addr, uint8_t value)
{
    if (addr >= BLOCK_SELECT_START_ADDR && addr < BLOCK_SELECT_END_ADDR) {
        /*
         * Block select register (CPU 8000-BFFF):
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    x  x  x  |  |  x  x  x
         *             +--+----------> 64KB PRG block select.
         */
        _block = (value & (D4 | D3));   /* This is the base 8K bank */

        const size_t bank = _block + _page;
        _prg_banks[0].bank(bank);
        _prg_banks[1].bank(bank + 1);

        _prg_banks[2].bank(_block + 6);   /* Last bank of the selected 64K block */
        _prg_banks[3].bank(_block + 7);

    } else if (addr >= PAGE_SELECT_START_ADDR && addr < PAGE_SELECT_END_ADDR) {
        /*
         * Bank select register (CPU C000-FFFF):
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    x  x  x  x  x  x  +--+--> 16KB PRG bank select (within current block)
         */
        _page = (value & 3) << 1;

        const size_t bank = _block + _page;
        _prg_banks[0].bank(bank);
        _prg_banks[1].bank(bank + 1);

        _prg_banks[2].bank(_block + 6);   /* Last bank of the selected 64K block */
        _prg_banks[3].bank(_block + 7);
    }
}

Serializer& operator&(Serializer& ser, Mapper_232& cart)
{
    ser & static_cast<Cartridge&>(cart)
        & cart._block
        & cart._page;

    if (ser.is_deserializer()) {
        cart.bank_select(Mapper_232::BLOCK_SELECT_START_ADDR, cart._block);
    }

    return ser;
}

}
}
}
