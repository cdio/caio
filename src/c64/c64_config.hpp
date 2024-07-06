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

namespace caio {
namespace commodore {
namespace c64 {

constexpr static const char* SEC_C64            = "c64";
constexpr static const char* KEY_PRGFILE        = "prg";
constexpr static const char* KEY_SWAPJOY        = "swapj";
constexpr static const char* KEY_UNIT_8         = "unit8";
constexpr static const char* KEY_UNIT_9         = "unit9";

constexpr static const char* DEFAULT_PRGFILE    = "";
constexpr static const char* DEFAULT_SWAPJOY    = "no";
constexpr static const char* DEFAULT_UNIT_8     = "";
constexpr static const char* DEFAULT_UNIT_9     = "";

class C64Cmdline : public config::Cmdline {
public:
    using Cmdline::Cmdline;

    virtual ~C64Cmdline() {
    }

    std::vector<config::Option> options() const override;
    std::string usage() const override;
    std::string sname() const override;
};

struct C64Config : public config::Config {
    std::string prgfile{};
    bool        swapj{};
    std::string unit8{};
    std::string unit9{};

    C64Config(config::Section& sec);

    virtual ~C64Config() {
    }

    std::string to_string() const override;
};

}
}
}
