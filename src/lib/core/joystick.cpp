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
#include "joystick.hpp"

#include <cstdint>
#include <string>


namespace caio {

Joystick::Joystick(const std::string &label)
    : Name{TYPE, label}
{
}

Joystick::~Joystick() {
}

void Joystick::reset(unsigned jid)
{
    _joyid = jid;
    _position = JOY_NONE;
}

void Joystick::position(uint8_t pos)
{
    _position = pos;
}

uint8_t Joystick::position() const
{
    return _position;
}

bool Joystick::is_connected() const
{
    return (_joyid != JOYID_INVALID);
}

}
