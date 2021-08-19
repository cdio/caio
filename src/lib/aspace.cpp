/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "aspace.hpp"


namespace cemu {

void ASpace::reset(addrmap_t rmaps, addrmap_t wmaps)
{
    auto banks = wmaps->size();
    if (!banks) {
        throw InternalError{"ASpace::reset(): 0 banks specified"};
    }

    auto bsize = (static_cast<size_t>(std::numeric_limits<addr_t>::max()) + 1) / banks;
    auto bmask = bsize - 1;
    if ((bsize & bmask) != 0) {
        throw InternalError{"ASpace::reset(): Bank size is not a power of 2: " + std::to_string(bsize)};
    }

    _rmaps = rmaps;
    _wmaps = wmaps;
    _bsize = bsize;
    _bmask = bmask;
    for (_bshift = 0; bmask != 0; bmask >>= 1, ++_bshift);
}

}
