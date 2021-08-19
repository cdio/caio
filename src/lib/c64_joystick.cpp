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
#include "c64_joystick.hpp"


namespace cemu {
namespace c64 {

/*
 * +------------------------------------------------------------------------------+-------+
 * |                             CIA 1 Port B ($DC01)                             | Joy 2 |
 * +-------------+----------------------------------------------------------------+-------+
 * |             | PB7     PB6     PB5     PB4     PB3     PB2     PB1     PB0    |       |
 * +-------------+----------------------------------------------------------------+-------+
 * | CIA1    PA7 | STOP    Q       C=      SPACE   2       CTRL    <-      1      |       |
 * | Port A  PA6 | /       ^       =       RSHIFT  HOME    ;       *       £      |       |
 * | ($DC00) PA5 | ,       @       :       .       -       L       P       +      |       |
 * |         PA4 | N       O       K       M       0       J       I       9      | Fire  |
 * |         PA3 | V       U       H       B       8       G       Y       7      | Right |
 * |         PA2 | X       T       F       C       6       D       R       5      | Left  |
 * |         PA1 | LSHIFT  E       S       Z       4       A       W       3      | Down  |
 * |         PA0 | CRSR DN F5      F3      F1      F7      CRSR RT RETURN  DELETE | Up    |
 * +-------------+----------------------------------------------------------------+-------+
 * | Joy 1       |                         Fire    Right   Left    Down    Up     |       |
 * +-------------+----------------------------------------------------------------+-------+
 *
 * Source: https://www.c64-wiki.com/wiki/Keyboard
 */
void C64Joystick::position(uint8_t pos)
{
    uint8_t position = C64_JOY_NONE;

    if (pos & JOY_UP) {
        position |= C64_JOY_UP;
    }

    if (pos & JOY_DOWN) {
        position |= C64_JOY_DOWN;
    }

    if (pos & JOY_LEFT) {
        position |= C64_JOY_LEFT;
    }

    if (pos & JOY_RIGHT) {
        position |= C64_JOY_RIGHT;
    }

    if (pos & JOY_FIRE) {
        position |= C64_JOY_FIRE;
    }

    Joystick::position(~position);
}

}
}
