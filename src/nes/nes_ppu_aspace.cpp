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
#include "nes_ppu_aspace.hpp"

namespace caio {
namespace nintendo {
namespace nes {

NESPPUASpace::NESPPUASpace(const sptr_t<Cartridge>& cart)
    : ASpace{},
      _mmap{{
          { cart, 0x0000 + Cartridge::PPU_OFFSET },     /* 0000 - 07FF - Cartridge 8K CHR RAM                   */
          { cart, 0x0800 + Cartridge::PPU_OFFSET },     /* 0800 - 0FFF - Cartridge 8K CHR RAM                   */
          { cart, 0x1000 + Cartridge::PPU_OFFSET },     /* 1000 - 17FF - Cartridge 8K CHR RAM                   */
          { cart, 0x1800 + Cartridge::PPU_OFFSET },     /* 1000 - 17FF - Cartridge 8K CHR RAM                   */
          { cart, 0x2000 + Cartridge::PPU_OFFSET },     /* 2000 - 27FF - Cartridge 2K VRAM                      */
          { cart, 0x2800 + Cartridge::PPU_OFFSET },     /* 2800 - 2FFF - Cartridge 2K VRAM, Nametable mirroring */
          { cart, 0x3000 + Cartridge::PPU_OFFSET },     /* 3000 - 37FF - Mirror of 2000-27FF                    */
          { cart, 0x3800 + Cartridge::PPU_OFFSET }      /* 3800 - 3FFF - Mirror of 2800-2FFF                    */
      }}
{
    CAIO_ASSERT(cart.get() != nullptr);
    ASpace::reset(_mmap, _mmap, ADDR_MASK);
}

NESPPUASpace::~NESPPUASpace()
{
}

}
}
}
