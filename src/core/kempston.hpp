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


namespace caio {
namespace kempston {

constexpr static const uint8_t JOY_PORT_UP    = 0x08;
constexpr static const uint8_t JOY_PORT_DOWN  = 0x04;
constexpr static const uint8_t JOY_PORT_LEFT  = 0x02;
constexpr static const uint8_t JOY_PORT_RIGHT = 0x01;
constexpr static const uint8_t JOY_PORT_FIRE  = 0x10;
constexpr static const uint8_t JOY_MASK       = 0x1F;

/**
 * Kempston interface joystick port.
 */
extern JoystickPort joystick_port;

}
}
