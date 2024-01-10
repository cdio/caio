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
    { KEY_RAM_16K, SEC_ZX80, KEY_RAM_16K, DEFAULT_RAM_16K, config::Arg::Optional, config::set_bool, "yes" },
    { KEY_ROM_8K,  SEC_ZX80, KEY_ROM_8K,  DEFAULT_ROM_8K,  config::Arg::Optional, config::set_bool, "yes" },
    { KEY_RVIDEO,  SEC_ZX80, KEY_RVIDEO,  DEFAULT_RVIDEO,  config::Arg::Optional, config::set_bool, "yes" },
    { KEY_CASSDIR, SEC_ZX80, KEY_CASSDIR, DEFAULT_CASSDIR, config::Arg::Required, config::set_value       },
    { KEY_PRGFILE, SEC_ZX80, KEY_PRGFILE, DEFAULT_PRGFILE, config::Arg::Required, config::set_value       }
};

std::string ZX80Cmdline::usage() const
{
    std::ostringstream os{};

    os << config::Cmdline::usage() << "\n\n"
        "Sinclair ZX-80 specific:\n"
        " --ram16 [yes|no]        Attach a 16K RAM instead of the default 1K RAM\n"
        " --rom8 [yes|no]         Attach the 8K ROM instead of the default 4K ROM\n"
        " --rvideo [yes|no]       Reverse video output\n"
        " --cassdir <dir>         Set the basic save/load directory (default is " << DEFAULT_CASSDIR << ")\n"
        " --prg <.o|.p>           Load a .o/.p file as soon as the basic is started";

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
      rom8{config::is_true(sec[KEY_ROM_8K])},
      rvideo{config::is_true(sec[KEY_RVIDEO])},
      cassdir{sec[KEY_CASSDIR]},
      prgfile{sec[KEY_PRGFILE]}
{
    title += " - Sinclair ZX-80";
}

std::string ZX80Config::to_string() const
{
    std::ostringstream os{};

    os << Config::to_string() << "\n"
        "  16K RAM:            " << (ram16  ? "yes" : "no") << "\n"
        "  8K ROM:             " << (rom8   ? "yes" : "no") << "\n"
        "  Reverse video:      " << (rvideo ? "yes" : "no") << "\n"
        "  Cassette directory: " << std::quoted(cassdir)    << "\n"
        "  Attached PRG:       " << std::quoted(prgfile);

    return os.str();
}

}
}
}
