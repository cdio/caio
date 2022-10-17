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
#pragma once

#include "aspace.hpp"
#include "device.hpp"


namespace caio {
namespace zx80 {

/**
 * The ZX80 Address Space implements the memory mappings seen by the cpu on a ZX80.
 * @see ASpace
 */
class ZX80ASpace : public ASpace {
public:
    constexpr static const addr_t ADDR_MASK = 0x7FFF;

    /**
     * Initialise this ZX80 Address Space.
     * @param ram Internal RAM (1K);
     * @param rom Internal ROM (4K).
     */
//TODO ioexp + ioports
    ZX80ASpace(const devptr_t &ram, const devptr_t &rom);

    virtual ~ZX80ASpace();
};

}
}
