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
#pragma once

#include "joystick.hpp"
#include "types.hpp"

namespace caio {
namespace kempston {

/*
 * Kempston joystick port:
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 * |  D7   |  D6   |  D5   |  D4   |  D3   |  D2   |  D1   |  D0   |
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 * |       |       |       | Fire  | Up    | Down  | Left  | Right |
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 */
constexpr static const uint16_t JOY_PORT_UP    = D3;
constexpr static const uint16_t JOY_PORT_DOWN  = D2;
constexpr static const uint16_t JOY_PORT_LEFT  = D1;
constexpr static const uint16_t JOY_PORT_RIGHT = D0;
constexpr static const uint16_t JOY_PORT_FIRE  = D4;
constexpr static const uint16_t JOY_MASK       = D0 | D1 | D2 | D3 | D4;

/**
 * Kempston interface joystick port.
 */
extern JoystickPort joystick_port;

}
}
