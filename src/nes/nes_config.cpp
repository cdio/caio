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
#include "nes_config.hpp"

#include "utils.hpp"

namespace caio {
namespace nintendo {
namespace nes {

static const config::Option nes_options[] = {
    { KEY_NTSC,       SEC_NES, KEY_NTSC,       DEFAULT_NTSC,       config::Arg::Optional, config::set_bool, "yes"   },
    { KEY_SWAPJOY,    SEC_NES, KEY_SWAPJOY,    DEFAULT_SWAPJOY,    config::Arg::Optional, config::set_bool, "yes"   },
    { KEY_JOY_UP,     SEC_NES, KEY_JOY_UP,     DEFAULT_JOY_UP,     config::Arg::Required, config::set_value         },
    { KEY_JOY_DOWN,   SEC_NES, KEY_JOY_DOWN,   DEFAULT_JOY_DOWN,   config::Arg::Required, config::set_value         },
    { KEY_JOY_LEFT,   SEC_NES, KEY_JOY_LEFT,   DEFAULT_JOY_LEFT,   config::Arg::Required, config::set_value         },
    { KEY_JOY_RIGHT,  SEC_NES, KEY_JOY_RIGHT,  DEFAULT_JOY_RIGHT,  config::Arg::Required, config::set_value         },
    { KEY_JOY_A,      SEC_NES, KEY_JOY_A,      DEFAULT_JOY_A,      config::Arg::Required, config::set_value         },
    { KEY_JOY_B,      SEC_NES, KEY_JOY_B,      DEFAULT_JOY_B,      config::Arg::Required, config::set_value         },
    { KEY_JOY_SELECT, SEC_NES, KEY_JOY_SELECT, DEFAULT_JOY_SELECT, config::Arg::Required, config::set_value         },
    { KEY_JOY_START,  SEC_NES, KEY_JOY_START,  DEFAULT_JOY_START,  config::Arg::Required, config::set_value         }
};

std::string NESCmdline::usage() const
{
      // 0         1         2         3         4         5         6         7
      // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    return std::format("{}\n\n"
        "NES (Nintendo Entertainment System) specific:\n"
        " --ntsc [yes|no]         Use the resolution of an old NTSC TV set\n"
        "                         (default is {})\n"
        " --swapj [yes|no]        Swap Joysticks (default is {})\n"
        " --button-up <bt>        Map the controller UP button to a gamepad button\n"
        "                         (default is {})\n"
        " --button-down <bt>      Map the controller DOWN button to a gamepad button\n"
        "                         (default is {})\n"
        " --button-left <bt>      Map the controller LEFT button to a gamepad button\n"
        "                         (default is {})\n"
        " --button-right <bt>     Map the controller RIGHT button to a gamepad button\n"
        "                         (default is {})\n"
        " --button-a <bt>         Map the controller A button to a gamepad button\n"
        "                         (default is {})\n"
        " --button-b <bt>         Map the controller B button to a gamepad button\n"
        "                         (default is {})\n"
        " --button-select <bt>    Map the controller SELECT button to a gamepad button\n"
        "                         (default is {})\n"
        " --button-start <bt>     Map the controller START button to a gamepad button\n"
        "                         (default is {})\n"
        " where <bt> is one of:\n"
        " UP, DOWN, LEFT, RIGHT, FIRE, A, B, X, Y, START, BACK, GUIDE\n",
        config::Cmdline::usage(),
        DEFAULT_NTSC,
        DEFAULT_SWAPJOY,
        DEFAULT_JOY_UP,
        DEFAULT_JOY_DOWN,
        DEFAULT_JOY_LEFT,
        DEFAULT_JOY_RIGHT,
        DEFAULT_JOY_A,
        DEFAULT_JOY_B,
        DEFAULT_JOY_SELECT,
        DEFAULT_JOY_START);
}

std::vector<config::Option> NESCmdline::options() const
{
    auto opts = Cmdline::options();
    opts.insert(opts.end(), &nes_options[0], &nes_options[std::size(nes_options)]);
    return opts;
}

std::string NESCmdline::sname() const
{
    return SEC_NES;
}

NESConfig::NESConfig(config::Section& sec)
    : Config{sec, "nes_"},
      ntsc{config::is_true(sec[KEY_NTSC])},
      swapj{config::is_true(sec[KEY_SWAPJOY])},
      buttons{
        .up     = joystick::port_name_to_offset(sec[KEY_JOY_UP].empty()     ? DEFAULT_JOY_UP     : sec[KEY_JOY_UP]),
        .down   = joystick::port_name_to_offset(sec[KEY_JOY_DOWN].empty()   ? DEFAULT_JOY_DOWN   : sec[KEY_JOY_DOWN]),
        .left   = joystick::port_name_to_offset(sec[KEY_JOY_LEFT].empty()   ? DEFAULT_JOY_LEFT   : sec[KEY_JOY_LEFT]),
        .right  = joystick::port_name_to_offset(sec[KEY_JOY_RIGHT].empty()  ? DEFAULT_JOY_RIGHT  : sec[KEY_JOY_RIGHT]),
        .a      = joystick::port_name_to_offset(sec[KEY_JOY_A].empty()      ? DEFAULT_JOY_A      : sec[KEY_JOY_A]),
        .b      = joystick::port_name_to_offset(sec[KEY_JOY_B].empty()      ? DEFAULT_JOY_B      : sec[KEY_JOY_B]),
        .select = joystick::port_name_to_offset(sec[KEY_JOY_SELECT].empty() ? DEFAULT_JOY_SELECT : sec[KEY_JOY_SELECT]),
        .start  = joystick::port_name_to_offset(sec[KEY_JOY_START].empty()  ? DEFAULT_JOY_START  : sec[KEY_JOY_START])
      }
{
    title += " - NES";

    if (buttons.up == -1) {
        throw InvalidArgument{"Invalid gamepad key name for controller UP button: {}", sec[KEY_JOY_UP]};
    }

    if (buttons.down == -1) {
        throw InvalidArgument{"Invalid gamepad key name for controller DOWN button: {}", sec[KEY_JOY_DOWN]};
    }

    if (buttons.left == -1) {
        throw InvalidArgument{"Invalid gamepad key name for controller LEFT button: {}", sec[KEY_JOY_LEFT]};
    }

    if (buttons.right == -1) {
        throw InvalidArgument{"Invalid gamepad key name for controller RIGHT button: {}", sec[KEY_JOY_RIGHT]};
    }

    if (buttons.a == -1) {
        throw InvalidArgument{"Invalid gamepad key name for controller A button: {}", sec[KEY_JOY_A]};
    }

    if (buttons.b == -1) {
        throw InvalidArgument{"Invalid gamepad key name for controller B button: {}", sec[KEY_JOY_B]};
    }

    if (buttons.select == -1) {
        throw InvalidArgument{"Invalid gamepad key name for controller SELECT button: {}", sec[KEY_JOY_SELECT]};
    }

    if (buttons.start == -1) {
        throw InvalidArgument{"Invalid gamepad key name for controller START button: {}", sec[KEY_JOY_START]};
    }
}

bool NESConfig::operator==(const NESConfig& other) const
{
    return (static_cast<const Config&>(*this) == static_cast<const Config&>(other) &&
        ntsc == other.ntsc &&
        swapj == other.swapj &&
        buttons.up == other.buttons.up &&
        buttons.down == other.buttons.down &&
        buttons.left == other.buttons.left &&
        buttons.right == other.buttons.right &&
        buttons.a == other.buttons.a &&
        buttons.b == other.buttons.b &&
        buttons.select == other.buttons.select &&
        buttons.start == other.buttons.start);
}

void NESConfig::to_section(config::Section& sec) const
{
    Config::to_section(sec);
    sec[KEY_NTSC] = (ntsc ? "yes" : "no");
    sec[KEY_SWAPJOY] = (swapj ? "yes" : "no");
    sec[KEY_JOY_UP] = joystick::port_offset_to_string(buttons.up);
    sec[KEY_JOY_DOWN] = joystick::port_offset_to_string(buttons.down);
    sec[KEY_JOY_LEFT] = joystick::port_offset_to_string(buttons.left);
    sec[KEY_JOY_RIGHT] = joystick::port_offset_to_string(buttons.right);
    sec[KEY_JOY_A] = joystick::port_offset_to_string(buttons.a);
    sec[KEY_JOY_B] = joystick::port_offset_to_string(buttons.b);
    sec[KEY_JOY_SELECT] = joystick::port_offset_to_string(buttons.select);
    sec[KEY_JOY_START] = joystick::port_offset_to_string(buttons.start);
}

std::string NESConfig::to_string() const
{
    return std::format("{}\n"
        "  NTSC mode:          {}\n"
        "  Swap controllers:   {}\n"
        "  UP button:          \"{}\"\n"
        "  DOWN button:        \"{}\"\n"
        "  LEFT button:        \"{}\"\n"
        "  RIGHT button:       \"{}\"\n"
        "  A button:           \"{}\"\n"
        "  B button:           \"{}\"\n"
        "  SELECT button:      \"{}\"\n"
        "  START button:       \"{}\"",
        Config::to_string(),
        (ntsc ? "yes" : "no"),
        (swapj ? "yes" : "no"),
        joystick::port_offset_to_string(buttons.up),
        joystick::port_offset_to_string(buttons.down),
        joystick::port_offset_to_string(buttons.left),
        joystick::port_offset_to_string(buttons.right),
        joystick::port_offset_to_string(buttons.a),
        joystick::port_offset_to_string(buttons.b),
        joystick::port_offset_to_string(buttons.select),
        joystick::port_offset_to_string(buttons.start));
}

}
}
}
