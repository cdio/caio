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

#include <cstdint>
#include <string>

#include "name.hpp"


namespace caio {
namespace joystick {

/**
 * Joystick port.
 * Port values cannot be 0 and must not share bits.
 */
struct Port {
    uint8_t up{};
    uint8_t down{};
    uint8_t left{};
    uint8_t right{};
    uint8_t fire{};
};

/**
 * Emulated joystick.
 * This class implements the basics of a joystick and
 * it should be derived by a specific emulated joystick.
 */
class Joystick : public Name {
public:
    constexpr static const char* TYPE          = "JOY";
    constexpr static unsigned JOYID_INVALID    = static_cast<unsigned>(-1);
    constexpr static unsigned JOYID_UNASSIGNED = JOYID_INVALID;
    constexpr static unsigned JOYID_VIRTUAL    = 255;

    /**
     * Initialise this joystick.
     * @param jport Joystick port;
     * @param label Label assigned to this joystick.
     * @see JoyPort
     */
    Joystick(const Port& port, const std::string& label = {})
        : Name{TYPE, label},
          _port{port} {
    }

    virtual ~Joystick() {
    }

    /**
     * Reset this joystick.
     * This method must be called by the UI to associate
     * this joystick to a real game controller.
     * @param jid Identifier assigned to this joystick.
     */
    void reset(unsigned jid = JOYID_INVALID) {
        _joyid = jid;
        _position = 0;
    }

    /**
     * Set the current joystick position.
     * @param pos A bitwise combination of the joystick port values.
     * @see JoyPort
     */
    void position(uint8_t pos) {
        _position = pos;
    }

    /**
     * Get the joystick positions.
     * @return A bitwise combination of the current joystick port values.
     * @see JoyPort
     */
    uint8_t position() const {
        return _position;
    }

    /**
     * Get the connection status of this joystick.
     * @return True if there is a real game controller behind this joystick; false otherwise.
     */
    bool is_connected() const {
        return (_joyid != JOYID_INVALID);
    }

    /**
     * Return the identifier of this joystick.
     * @return The joystick id.
     */
    unsigned joyid() const {
        return _joyid;
    }

    /**
     * Get the joystick port.
     * @return A constant reference to the joystick port.
     */
    const Port& port() const {
        return _port;
    }

private:
    Port     _port;
    unsigned _joyid{JOYID_INVALID};
    uint8_t  _position{};
};

}

using Joystick = joystick::Joystick;
using JoystickPort = joystick::Port;

}
