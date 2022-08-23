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
#include "c64_joystick.hpp"


namespace caio {
namespace c64 {

C64Joystick::C64Joystick(const std::string &label)
    : Joystick{label}
{
    C64Joystick::reset();
}

C64Joystick::~C64Joystick()
{
}

void C64Joystick::reset(unsigned jid)
{
    Joystick::reset(jid);
    _port = JOY_PORT_PULLUP;
}

void C64Joystick::position(uint8_t pos)
{
    Joystick::position(pos);

    uint8_t port = JOY_PORT_PULLUP;

    if (pos & JOY_UP) {
        port &= ~JOY_PORT_UP;
    }

    if (pos & JOY_DOWN) {
        port &= ~JOY_PORT_DOWN;
    }

    if (pos & JOY_LEFT) {
        port &= ~JOY_PORT_LEFT;
    }

    if (pos & JOY_RIGHT) {
        port &= ~JOY_PORT_RIGHT;
    }

    if (pos & JOY_FIRE) {
        port &= ~JOY_PORT_FIRE;
    }

    _port = port;
}

uint8_t C64Joystick::port() const
{
    return _port;
}

}
}
