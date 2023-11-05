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
#include "zx80_config.hpp"

#include <sstream>


namespace caio {
namespace sinclair {
namespace zx80 {

static const config::Option zx80_options[] = {
    { "ram16",  SEC_ZX80,   KEY_RAM_16K,    DEFAULT_RAM_16K,    config::Arg::None,  config::set_true    },
    { "rom8",   SEC_ZX80,   KEY_ROM_8K,     DEFAULT_ROM_8K,     config::Arg::None,  config::set_true    }
};

std::string ZX80Cmdline::usage() const
{
    std::ostringstream os{};

    os << config::Cmdline::usage() << std::endl << std::endl
       << "Sinclair ZX80 specific:" << std::endl
       << " --ram16                 Attach a 16K RAM instead of the default 1K RAM" << std::endl
       << " --rom8                  Attach the 8K ROM instead of the default 4K ROM";

    return os.str();
}

std::vector<config::Option> ZX80Cmdline::options() const
{
    auto opts = Cmdline::options();
    opts.insert(opts.end(), &zx80_options[0], &zx80_options[std::size(zx80_options)]);
    return opts;
}

std::string ZX80Cmdline::sname() const
{
    return SEC_ZX80;
}

ZX80Config::ZX80Config(config::Section& sec)
    : Config{sec, "zx80_"},
      ram16{config::is_true(sec[KEY_RAM_16K])},
      rom8{config::is_true(sec[KEY_ROM_8K])}
{
    title += " - ZX80";
}

std::string ZX80Config::to_string() const
{
    std::ostringstream os{};

    os << Config::to_string()                                   << std::endl
       << "  16K RAM:            " << (ram16 ? "yes" : "no")    << std::endl
       << "  8K ROM:             " << (rom8  ? "yes" : "no");

    return os.str();
}

}
}
}
