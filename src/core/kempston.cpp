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
#include "kempston.hpp"

namespace caio {
namespace kempston {

JoystickPort joystick_port{
    .up     = JOY_PORT_UP,
    .down   = JOY_PORT_DOWN,
    .left   = JOY_PORT_LEFT,
    .right  = JOY_PORT_RIGHT,
    .fire   = JOY_PORT_FIRE,
    .a      = 0,
    .b      = 0,
    .x      = 0,
    .y      = 0,
    .back   = 0,
    .guide  = 0,
    .start  = 0
};

}
}
