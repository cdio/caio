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

#include "config.hpp"
#include "nes_joystick.hpp"

namespace caio {
namespace nintendo {
namespace nes {

constexpr static const char* SEC_NES            = "nes";
constexpr static const char* KEY_NTSC           = "ntsc";
constexpr static const char* KEY_SWAPJOY        = "swapj";
constexpr static const char* KEY_JOY_UP         = "button-up";
constexpr static const char* KEY_JOY_DOWN       = "button-down";
constexpr static const char* KEY_JOY_LEFT       = "button-left";
constexpr static const char* KEY_JOY_RIGHT      = "button-right";
constexpr static const char* KEY_JOY_A          = "button-a";
constexpr static const char* KEY_JOY_B          = "button-b";
constexpr static const char* KEY_JOY_START      = "button-start";
constexpr static const char* KEY_JOY_SELECT     = "button-select";

constexpr static const char* DEFAULT_NTSC       = "yes";
constexpr static const char* DEFAULT_SWAPJOY    = "no";
constexpr static const char* DEFAULT_JOY_UP     = "UP";
constexpr static const char* DEFAULT_JOY_DOWN   = "DOWN";
constexpr static const char* DEFAULT_JOY_LEFT   = "LEFT";
constexpr static const char* DEFAULT_JOY_RIGHT  = "RIGHT";
constexpr static const char* DEFAULT_JOY_A      = "A";
constexpr static const char* DEFAULT_JOY_B      = "B";
constexpr static const char* DEFAULT_JOY_START  = "START";
#ifdef __APPLE__    /* WTF XXX */
constexpr static const char* DEFAULT_JOY_SELECT = "GUIDE";
#else
constexpr static const char* DEFAULT_JOY_SELECT = "BACK";
#endif

class NESCmdline : public config::Cmdline {
public:
    using Cmdline::Cmdline;

    virtual ~NESCmdline() {
    }

    std::vector<config::Option> options() const override;
    std::string usage() const override;
    std::string sname() const override;
};

struct NESConfig : public config::Config {
    bool            ntsc{};
    bool            swapj{};
    NESButtonConfig buttons{};

    NESConfig(config::Section& sec);

    virtual ~NESConfig()
    {
    }

    bool operator==(const NESConfig& other) const;

    bool operator!=(const NESConfig& other) const
    {
        return !operator==(other);
    }

    void to_section(config::Section& sec) const override;
    std::string to_string() const override;
};

}
}
}
