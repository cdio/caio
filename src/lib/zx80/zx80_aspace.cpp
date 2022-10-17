/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include "zx80_aspace.hpp"

#include "device_none.hpp"


namespace caio {
namespace zx80 {

ZX80ASpace::ZX80ASpace(const devptr_t &ram, const devptr_t &rom)
{
    /*
     * The 16K address space of is subdivided into 16 1K mappings.
     *   0000-0FFF  BIOS ROM (4KBytes)
     *   1000-3FFF  Not used
     *   4000-43FF  Internal RAM (1 KByte)
     *   4000-7FFF  External RAM (16 KBytes)
     */
    addrmap_t map = {
        { rom,          0x0000 },
        { rom,          0x0400 },
        { rom,          0x0800 },
        { rom,          0x0C00 },

        { device_none,  0x1000 },
        { device_none,  0x1400 },
        { device_none,  0x1800 },
        { device_none,  0x1C00 },

        { device_none,  0x2000 },
        { device_none,  0x2400 },
        { device_none,  0x2800 },
        { device_none,  0x2C00 },

        { device_none,  0x3000 },
        { device_none,  0x3400 },
        { device_none,  0x3800 },
        { device_none,  0x3C00 },

        { ram,          0x0000 },
        { device_none,  0x4400 },
        { device_none,  0x4800 },
        { device_none,  0x4C00 },

        { device_none,  0x5000 },
        { device_none,  0x5400 },
        { device_none,  0x5800 },
        { device_none,  0x5C00 },

        { device_none,  0x6000 },
        { device_none,  0x6400 },
        { device_none,  0x6800 },
        { device_none,  0x6C00 },

        { device_none,  0x7000 },
        { device_none,  0x7400 },
        { device_none,  0x7800 },
        { device_none,  0x7C00 },
    };

    ASpace::reset(map, map, ADDR_MASK);
}

ZX80ASpace::~ZX80ASpace()
{
}

}
}
