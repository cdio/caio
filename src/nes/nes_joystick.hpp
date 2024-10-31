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

#include <string_view>

#include "joystick.hpp"
#include "types.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/**
 * NES controller button configuration
 * Members point to an offset within a Port.
 * @see joystick::Port
 */
struct NESButtonConfig {
    ssize_t up{};
    ssize_t down{};
    ssize_t left{};
    ssize_t right{};
    ssize_t a{};
    ssize_t b{};
    ssize_t select{};
    ssize_t start{};
};

/**
 * NES Joystick interface.
 *
 * ### Controller internal connections:
 *
 * The controller buttons are connected to the
 * parallel inputs of a shift register as follows:
 *
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
 *
 * - The latch command (that loads the shift register with parallel data)
 *   is connected to the processor's OUT-0 pin ($4016.0).
 *
 * - The clock's LO-HI transition triggers a bit shift:
 *   + The content of PI1 is moved to PI2 and so-forth.
 *   + The content of PI8 is sent to the serial output.
 *   + The content of the register's serial-input pin is feeded
 *     into PI1 (in the case of NES this pin is connected to GND).
 *   + The clock input is connected to the CPU's R/W output pin
 *     which transitions from LO to HI after a read.
 *
 * ### Data read from the CPU:
 *
 * The controller status is read from input ports $4016 (joy#1) and $4017 (joy#2):
 *
 *   D7 D6 D5 D4 D3 D2 D1 D0
 *    |  |  |  |  |  |  |  |
 *    x  x  x  x  x  |  |  +-> Status bit (0: button released, 1: button pressed)
 *                   |  +----> NES: 0, Famicom: Expansion controller status bit
 *                   +-------> NES: 0, Famicom: Microphone status bit (port $4016 only)
 *
 * The status bit is the output from the shift register.
 *
 * @see https://www.nesdev.org/wiki/Standard_controller
 */
class NESJoystick : public Joystick {
public:
    /**
     * Initialise this joystick.
     * @param label Label assigned to this joystick.
     * @see Joystick
     */
    NESJoystick(std::string_view label, const NESButtonConfig& buttons)
        : Joystick{label, to_port(buttons)}
    {
    }

    /**
     * Initialise this joystick.
     * @param label Label assigned to this joystick.
     * @see Joystick
     */
    NESJoystick(std::string_view label)
        : Joystick{label, default_port}
    {
    }

    ~NESJoystick()
    {
    }

    /**
     * Load the shift register with the joystick status.
     */
    void load()
    {
        _shreg = position();
    }

    /**
     * Shift one bit.
     */
    void clk()
    {
        _shreg >>= 1;
    }

    /**
     * Retrieve the output bit value.
     * @return The output bit value.
     */
    bool data() const
    {
        return (_shreg & 1);
    }

    static JoystickPort to_port(const NESButtonConfig& buttons);

private:
    uint16_t _shreg{};

    static JoystickPort default_port;
};

}

using NESJoystick = nes::NESJoystick;

}
}
