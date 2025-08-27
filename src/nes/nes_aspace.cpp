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
#include "nes_aspace.hpp"

#include "logger.hpp"
#include "nes_params.hpp"

namespace caio {
namespace nintendo {
namespace nes {

NESASpace::NESASpace(std::string_view label, const sptr_t<RAM>& ram, const sptr_t<RP2C02>& ppu,
    const sptr_t<Cartridge>& cart)
    : ASpace{TYPE, label},
      _mmap{{
          { ram,  0x0000 },     /* 0000 - 07FF - 2K RAM                                             */
          { ram,  0x0000 },     /* 0800 - 0FFF - 2K RAM (mirror)                                    */
          { ram,  0x0000 },     /* 1000 - 17FF - 2K RAM (mirror)                                    */
          { ram,  0x0000 },     /* 1800 - 1FFF - 2K RAM (mirror)                                    */
          { ppu,  0x0000 },     /* 2000 - 27FF - PPU MMIO registers                                 */
          { ppu,  0x0000 },     /* 2800 - 2FFF - PPU MMIO registers (mirror)                        */
          { ppu,  0x0000 },     /* 3000 - 37FF - PPU MMIO registers (mirror)                        */
          { ppu,  0x0000 },     /* 3800 - 3FFF - PPU MMIO registers (mirror)                        */
          { cart, 0x0000 },     /* 4000 - 47FF - Cartridge (4000-401F overrided internally by CPU)  */
          { cart, 0x0800 },     /* 4800 - 4FFF - Cartridge                                          */
          { cart, 0x1000 },     /* 5000 - 57FF - Cartridge                                          */
          { cart, 0x1800 },     /* 5800 - 5FFF - Cartridge                                          */
          { cart, 0x2000 },     /* 6000 - 67FF - Cartridge                                          */
          { cart, 0x2800 },     /* 6800 - 6FFF - Cartridge                                          */
          { cart, 0x3000 },     /* 7000 - 77FF - Cartridge                                          */
          { cart, 0x3800 },     /* 7800 - 7FFF - Cartridge                                          */
          { cart, 0x4000 },     /* 8000 - 87FF - Cartridge                                          */
          { cart, 0x4800 },     /* 8800 - 8FFF - Cartridge                                          */
          { cart, 0x5000 },     /* 9000 - 97FF - Cartridge                                          */
          { cart, 0x5800 },     /* 9800 - 9FFF - Cartridge                                          */
          { cart, 0x6000 },     /* A000 - A7FF - Cartridge                                          */
          { cart, 0x6800 },     /* A800 - AFFF - Cartridge                                          */
          { cart, 0x7000 },     /* B000 - B7FF - Cartridge                                          */
          { cart, 0x7800 },     /* B800 - BFFF - Cartridge                                          */
          { cart, 0x8000 },     /* C000 - C7FF - Cartridge                                          */
          { cart, 0x8800 },     /* C800 - CFFF - Cartridge                                          */
          { cart, 0x9000 },     /* D000 - D7FF - Cartridge                                          */
          { cart, 0x9800 },     /* D800 - DFFF - Cartridge                                          */
          { cart, 0xA000 },     /* E000 - E7FF - Cartridge                                          */
          { cart, 0xA800 },     /* E800 - EFFF - Cartridge                                          */
          { cart, 0xB000 },     /* F000 - F7FF - Cartridge                                          */
          { cart, 0xB800 }      /* F800 - FFFF - Cartridge                                          */
      }}
{
    CAIO_ASSERT(ram && ppu && cart && ram->size() >= RAM_SIZE);
    ASpace::reset(_mmap, _mmap, ADDR_MASK);
}

}
}
}
