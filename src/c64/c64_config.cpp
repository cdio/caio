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

#include <sstream>

#include "utils.hpp"


namespace caio {
namespace commodore {
namespace c64 {

static const config::Option c64_options[] = {
    { "prg",    SEC_C64,  KEY_PRGFILE,  DEFAULT_PRGFILE,    config::Arg::Required,  config::set_value   },
    { "resid",  SEC_C64,  KEY_RESID,    DEFAULT_RESID,      config::Arg::Required,  config::set_bool    },
    { "swapj",  SEC_C64,  KEY_SWAPJOY,  DEFAULT_SWAPJOY,    config::Arg::None,      config::set_true    },
    { "8",      SEC_C64,  KEY_UNIT_8,   DEFAULT_UNIT_8,     config::Arg::Required,  config::set_value   },
    { "9",      SEC_C64,  KEY_UNIT_9,   DEFAULT_UNIT_9,     config::Arg::Required,  config::set_value   }
};

std::string C64Cmdline::usage() const
{
    std::ostringstream os{};

        // 0         1         2         3         4         5         6         7
        // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    os << config::Cmdline::usage() << std::endl << std::endl
       << "Commodore C64 specific:"                                                                     << std::endl
       << " --prg <prg>             Load a PRG file as soon as the basic is ready"                      << std::endl
       << " --resid <yes|no>        Use the MOS6581 reSID library (default is " << DEFAULT_RESID << ")" << std::endl
       << " --swapj                 Swap Joysticks"                                                     << std::endl
       << " --8 <path>              Attach a disk drive unit 8"                                         << std::endl
       << " --9 <path>              Attach a disk drive unit 9";

    return os.str();
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
      resid{config::is_true(sec[KEY_RESID])},
      swapj{config::is_true(sec[KEY_SWAPJOY])},
      unit8{sec[KEY_UNIT_8]},
      unit9{sec[KEY_UNIT_9]}
{
    title += " - C64";
}

std::string C64Config::to_string() const
{
    std::ostringstream os{};

    os << Config::to_string()                                   << std::endl
       << "  Swap Joysticks:     " << (swapj ? "yes" : "no")    << std::endl
       << "  Attached PRG:       " << std::quoted(prgfile)      << std::endl
       << "  Unit-8:             " << std::quoted(unit8)        << std::endl
       << "  Unit-9:             " << std::quoted(unit9)        << std::endl
       << "  Use reSID library:  " << (resid ? "yes" : "no");

    return os.str();
}

}
}
}
