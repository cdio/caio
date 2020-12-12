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
#pragma once

#include "joystick.hpp"


namespace cemu {
namespace c64 {

/**
 * C64 Joystick.
 */
class C64Joystick : public Joystick {
public:
    enum C64JoystickPosition {
        C64_JOY_NONE  = 0x00,
        C64_JOY_UP    = 0x01,
        C64_JOY_DOWN  = 0x02,
        C64_JOY_LEFT  = 0x04,
        C64_JOY_RIGHT = 0x08,
        C64_JOY_FIRE  = 0x10
    };

    /**
     * Initialise this C64 Joystick.
     * @param label Label assigned to this joystick.
     */
    C64Joystick(const std::string &label = {})
        : Joystick{label} {
        C64Joystick::reset();
    }

    virtual ~C64Joystick() {
    }

    /**
     * @see Joystick::reset().
     */
    void reset(unsigned jid = Joystick::JOYID_INVALID) override {
        Joystick::reset(jid);
        Joystick::position(~C64_JOY_NONE);
    }

    /**
     * Set the current joystick position.
     * This method converts the CEMU's joystick position coding into C64's joystick coding.
     * Receives a JoystickPosition and translates it into a C64JoystickPosition.
     * @param pos A bitwise combination of the positions to set (see JoystickPosition).
     * @see C64JoystickPosition
     * @see JoystickPosition
     * @see Joystick::position(uint8_t)
     * @see Joystick::position()
     */
    void position(uint8_t pos) override;
};

}
}
