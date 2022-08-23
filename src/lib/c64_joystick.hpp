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
#pragma once

#include "joystick.hpp"
#include "mos_6526.hpp"


namespace caio {
namespace c64 {

/**
 * C64 Joystick.
 */
class C64Joystick : public Joystick {
public:
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
    constexpr static const uint8_t JOY_PORT_PULLUP = 255;
    constexpr static const uint8_t JOY_PORT_UP     = Mos6526::P0;
    constexpr static const uint8_t JOY_PORT_DOWN   = Mos6526::P1;
    constexpr static const uint8_t JOY_PORT_LEFT   = Mos6526::P2;
    constexpr static const uint8_t JOY_PORT_RIGHT  = Mos6526::P3;
    constexpr static const uint8_t JOY_PORT_FIRE   = Mos6526::P4;

    /**
     * Initialise this C64 Joystick.
     * @param label Label assigned to this joystick.
     */
    C64Joystick(const std::string &label = {});

    virtual ~C64Joystick();

    /**
     * @see Joystick::reset().
     */
    void reset(unsigned jid = Joystick::JOYID_INVALID) override;

    /**
     * Set the current joystick position and port status.
     * Set the specified joystick position and translate it to a port value.
     * @param pos Bitwise combination of Joystick::JoyPosition values.
     * @see Joystick::JoyPosition
     * @see Joystick::position(uint8_t)
     * @see Joystick::port()
     */
    void position(uint8_t pos) override;

    /**
     * @return The status of this C64 joystick port.
     */
    uint8_t port() const;

private:
    uint8_t _port{JOY_PORT_PULLUP};
};

}
}
