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
#include "zilog_z80.hpp"

#include <chrono>
#include <iomanip>


namespace caio {

int ZilogZ80::i_HALT(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    self._is_halted = true;
    return 0;
}

int ZilogZ80::i_DI(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    self._IFF1 = false;
    self._IFF2 = false;
    return 0;
}

int ZilogZ80::i_EI(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    self._IFF1 = true;
    self._IFF2 = true;
    return 0;
}

}
