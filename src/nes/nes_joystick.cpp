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
#include "nes_joystick.hpp"

#include <map>

#include "utils.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/*
 *   PI1 PI2 PI3 PI4 PI5 PI6 PI7 P8I
 *    |   |   |   |   |   |   |   |
 *    |   |   |   |   |   |   |   +-> A
 *    |   |   |   |   |   |   +-----> B
 *    |   |   |   |   |   +---------> Select
 *    |   |   |   |   +-------------> Start
 *    |   |   |   +-----------------> Up
 *    |   |   +---------------------> Down
 *    |   +-------------------------> Left
 *    +-----------------------------> Right
 */
constexpr static const uint16_t JOY_PORT_A      = D0;
constexpr static const uint16_t JOY_PORT_B      = D1;
constexpr static const uint16_t JOY_PORT_SELECT = D2;
constexpr static const uint16_t JOY_PORT_START  = D3;
constexpr static const uint16_t JOY_PORT_UP     = D4;
constexpr static const uint16_t JOY_PORT_DOWN   = D5;
constexpr static const uint16_t JOY_PORT_LEFT   = D6;
constexpr static const uint16_t JOY_PORT_RIGHT  = D7;

JoystickPort NESJoystick::default_port{
    .up     = JOY_PORT_UP,
    .down   = JOY_PORT_DOWN,
    .left   = JOY_PORT_LEFT,
    .right  = JOY_PORT_RIGHT,
    .fire   = 0,
    .a      = JOY_PORT_A,
    .b      = JOY_PORT_B,
    .x      = 0,
    .y      = 0,
    .back   = JOY_PORT_SELECT,
    .guide  = 0,
    .start  = JOY_PORT_START
};

JoystickPort NESJoystick::to_port(const NESButtonConfig& buttons)
{
    JoystickPort port{};
    uint8_t* p = reinterpret_cast<uint8_t*>(&port);

    *reinterpret_cast<uint16_t*>(p + buttons.up) = JOY_PORT_UP;
    *reinterpret_cast<uint16_t*>(p + buttons.down) = JOY_PORT_DOWN;
    *reinterpret_cast<uint16_t*>(p + buttons.left) = JOY_PORT_LEFT;
    *reinterpret_cast<uint16_t*>(p + buttons.right) = JOY_PORT_RIGHT;
    *reinterpret_cast<uint16_t*>(p + buttons.a) = JOY_PORT_A;
    *reinterpret_cast<uint16_t*>(p + buttons.b) = JOY_PORT_B;
    *reinterpret_cast<uint16_t*>(p + buttons.start) = JOY_PORT_START;
    *reinterpret_cast<uint16_t*>(p + buttons.select) = JOY_PORT_SELECT;

    return port;
}

}
}
}
