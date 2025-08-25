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
#include "ram_bank.hpp"

namespace caio {

std::ostream& RAMBank::dump(std::ostream& os, size_t base) const
{
    const auto start = _ram->begin() + _boffset;
    const auto end = start + _bsize;
    return utils::dump(os, start, end, base);
}

Serializer& operator&(Serializer& ser, RAMBank& rb)
{
    ser & static_cast<Device&>(rb)
        & rb._bsize
        & rb._banks
        & rb._bank
        & rb._boffset;

    return ser;
}

}
