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
#include <string_view>

#include "name.hpp"

namespace caio {
namespace joystick {

/**
 * Joystick port.
 */
struct Port {
    uint16_t up{};
    uint16_t down{};
    uint16_t left{};
    uint16_t right{};
    uint16_t fire{};
    uint16_t a{};
    uint16_t b{};
    uint16_t x{};
    uint16_t y{};
    uint16_t back{};
    uint16_t guide{};
    uint16_t start{};
};

/**
 * Get the offset of a port member given its name.
 * @param name Name of the port member.
 * @return The offset to the member within the port structure;
 * -1 if the name does not correspond to a port member.
 * @see Port
 */
ssize_t port_name_to_offset(std::string_view name);

/**
 * Get the name of a port member given its offset.
 * @param offset Offset to the member.
 * @return The name of the member if the offset is valid; otherwise an empty string.
 * @see Port
 */
std::string port_offset_to_string(ssize_t offset);

/**
 * Joystick interface.
 * Interface between a physical gamepad connected to the
 * host platform and a joystick as the emulated system sees it.
 */
class Joystick : public Name {
public:
    constexpr static const char* TYPE          = "JOY";
    constexpr static unsigned JOYID_INVALID    = static_cast<unsigned>(-1);
    constexpr static unsigned JOYID_UNASSIGNED = JOYID_INVALID;
    constexpr static unsigned JOYID_VIRTUAL    = 255;
    constexpr static const char* VJOY_NAME     = "Virtual Joystick";

    /**
     * Initialise this joystick.
     * @param label Label assigned to this joystick;
     * @param jport Joystick port.
     * @see Port
     */
    Joystick(std::string_view label, const Port& port)
        : Name{TYPE, label},
          _port{port}
    {
    }

    virtual ~Joystick()
    {
    }

    /**
     * Reset this joystick.
     * This method must be called by the UI to associate
     * this joystick to a real game controller.
     * @param jid Identifier assigned to this joystick.
     */
    void reset(unsigned jid = JOYID_INVALID, std::string_view name = "")
    {
        _joyid = jid;
        _position = 0;
        _name = name;
    }

    /**
     * Set the current joystick position.
     * @param pos A bitwise combination of the joystick port values.
     * @see Port
     */
    void position(uint16_t pos)
    {
        _position = pos;
    }

    /**
     * Get the joystick positions.
     * @return A bitwise combination of the current joystick port values.
     * @see Port
     */
    uint16_t position() const
    {
        return _position;
    }

    /**
     * Get the connection status of this joystick.
     * @return True if there is a real game controller behind this joystick; false otherwise.
     */
    bool is_connected() const
    {
        return (_joyid != JOYID_INVALID);
    }

    /**
     * Return the identifier of this joystick.
     * @return The joystick id.
     */
    unsigned joyid() const
    {
        return _joyid;
    }

    /**
     * Get the joystick port.
     * @return A constant reference to the joystick port.
     */
    const Port& port() const
    {
        return _port;
    }

    /*
     * Get the name of the associated gamepad.
     * @return The name of the associated gamepad if this joystick is connected to one; otherwise an empty string.
     */
    const std::string& name() const
    {
        return _name;
    }

private:
    Port        _port;
    unsigned    _joyid{JOYID_INVALID};
    uint16_t    _position{};
    std::string _name{};
};

}

using Joystick = joystick::Joystick;
using JoystickPort = joystick::Port;

}
