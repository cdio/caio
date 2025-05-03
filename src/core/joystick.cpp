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
#include "joystick.hpp"

#include <cstddef>
#include <unordered_map>

#include "utils.hpp"

namespace caio {
namespace joystick {

static const std::unordered_map<std::string, ssize_t> gamepad_name_to_port{
    { "UP",     offsetof(Port, up)    },
    { "DOWN",   offsetof(Port, down)  },
    { "LEFT",   offsetof(Port, left)  },
    { "RIGHT",  offsetof(Port, right) },
    { "FIRE",   offsetof(Port, fire)  },
    { "A",      offsetof(Port, a)     },
    { "B",      offsetof(Port, b)     },
    { "X",      offsetof(Port, x)     },
    { "Y",      offsetof(Port, y)     },
    { "START",  offsetof(Port, start) },
    { "BACK",   offsetof(Port, back)  },
    { "GUIDE",  offsetof(Port, guide) }
};

ssize_t port_name_to_offset(std::string_view name)
{
    const auto it = gamepad_name_to_port.find(utils::toup(name));
    return (it == gamepad_name_to_port.end() ? -1 : it->second);
}

std::string port_offset_to_string(ssize_t offset)
{
    const auto it = std::find_if(std::begin(gamepad_name_to_port), std::end(gamepad_name_to_port),
        [offset](const auto& pair) {
            return (offset == pair.second);
    });

    return (it == std::end(gamepad_name_to_port) ? "" : it->first);
}

}
}
