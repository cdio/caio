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
#include "c64_config.hpp"

#include "utils.hpp"

namespace caio {
namespace commodore {
namespace c64 {

static const config::Option c64_options[] = {
    { KEY_PRGFILE,  SEC_C64, KEY_PRGFILE, DEFAULT_PRGFILE, config::Arg::Required, config::set_value         },
    { KEY_SWAPJOY,  SEC_C64, KEY_SWAPJOY, DEFAULT_SWAPJOY, config::Arg::Optional, config::set_bool, "yes"   },
    { KEY_UNIT_8,   SEC_C64, KEY_UNIT_8,  DEFAULT_UNIT_8,  config::Arg::Required, config::set_value         },
    { KEY_UNIT_9,   SEC_C64, KEY_UNIT_9,  DEFAULT_UNIT_9,  config::Arg::Required, config::set_value         }
};

std::string C64Cmdline::usage() const
{
      // 0         1         2         3         4         5         6         7
      // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    return std::format("{}\n\n"
        "Commodore 64 specific:\n"
        " --prg <prg>             Load a PRG file as soon as the basic is ready\n"
        " --swapj [yes|no]        Swap Joysticks (default is {})\n"
        " --unit8 <dir>           Attach a disk drive as unit 8\n"
        " --unit9 <dir>           Attach a disk drive as unit 9",
        config::Cmdline::usage(),
        DEFAULT_SWAPJOY);
}

std::vector<config::Option> C64Cmdline::options() const
{
    auto opts = Cmdline::options();
    opts.insert(opts.end(), &c64_options[0], &c64_options[std::size(c64_options)]);
    return opts;
}

std::string C64Cmdline::sname() const
{
    return SEC_C64;
}

C64Config::C64Config(config::Section& sec)
    : Config{sec, "c64_"},
      prgfile{sec[KEY_PRGFILE]},
      swapj{config::is_true(sec[KEY_SWAPJOY])},
      unit8{sec[KEY_UNIT_8]},
      unit9{sec[KEY_UNIT_9]}
{
    title += " - Commodore 64";
}

bool C64Config::operator==(const C64Config& other) const
{
    return (static_cast<const Config&>(*this) == static_cast<const Config&>(other) &&
       prgfile == other.prgfile &&
       swapj == other.swapj &&
       unit8 == other.unit8 &&
       unit9 == other.unit9);
}

void C64Config::to_section(config::Section& sec) const
{
    Config::to_section(sec);
    sec[KEY_PRGFILE] = prgfile;
    sec[KEY_SWAPJOY] = (swapj ? "yes" : "no");
    sec[KEY_UNIT_8] = unit8;
    sec[KEY_UNIT_9] = unit9;
}

std::string C64Config::to_string() const
{
    return std::format("{}\n"
        "  Swap Joysticks:     {}\n"
        "  Attached PRG:       \"{}\"\n"
        "  Unit-8:             \"{}\"\n"
        "  Unit-9:             \"{}\"",
        Config::to_string(),
        (swapj ? "yes" : "no"),
        prgfile,
        unit8,
        unit9);
}

}
}
}
