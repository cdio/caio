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
#include "zilog_z80.hpp"

namespace caio {
namespace zilog {

int Z80::i_DI(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DI           - F3
     */
    self._IFF1 = false;
    self._IFF2 = false;
    return 0;
}

int Z80::i_EI(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * EI           - FB
     */
    self._IFF1 = true;
    self._IFF2 = true;
    return 0;
}

}
}
