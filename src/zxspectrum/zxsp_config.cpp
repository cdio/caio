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
#include "zxsp_config.hpp"

#include <sstream>

namespace caio {
namespace sinclair {
namespace zxspectrum {

static const config::Option zxspectrum_options[] = {
    { KEY_OTAPE,    SEC_ZXSPECTRUM, KEY_OTAPE,    DEFAULT_OTAPE,    config::Arg::Required, config::set_value        },
    { KEY_ITAPE,    SEC_ZXSPECTRUM, KEY_ITAPE,    DEFAULT_ITAPE,    config::Arg::Required, config::set_value        },
    { KEY_FASTLOAD, SEC_ZXSPECTRUM, KEY_FASTLOAD, DEFAULT_FASTLOAD, config::Arg::Optional, config::set_bool, "yes"  },
    { KEY_SNAPSHOT, SEC_ZXSPECTRUM, KEY_SNAPSHOT, DEFAULT_SNAPSHOT, config::Arg::Required, config::set_value        }
};

std::string ZXSpectrumCmdline::usage() const
{
    std::ostringstream os{};

    os << config::Cmdline::usage() << "\n\n"
        "Sinclair ZX-Spectrum 48K specific:\n"
        " --tape <fname|dir>      Set the input tape file (TAP) or directory\n"
        " --otape <fname|dir>     Set the output tape file (TAP) or directory\n"
        "                         (default is " << DEFAULT_OTAPE << ")\n"
        " --fastload [yes|no]     Fast tape loading (default is " << DEFAULT_FASTLOAD << ")\n"
        " --snap <fname>          Load a snapshot image (Z80 or SNA formats)\n";

    return os.str();
}

std::vector<config::Option> ZXSpectrumCmdline::options() const
{
    auto opts = Cmdline::options();
    opts.insert(opts.end(), &zxspectrum_options[0], &zxspectrum_options[std::size(zxspectrum_options)]);
    return opts;
}

std::string ZXSpectrumCmdline::sname() const
{
    return SEC_ZXSPECTRUM;
}

ZXSpectrumConfig::ZXSpectrumConfig(config::Section& sec)
    : Config{sec, "zxspectrum_"},
      otape{sec[KEY_OTAPE]},
      itape{sec[KEY_ITAPE]},
      fastload{config::is_true(sec[KEY_FASTLOAD])},
      snap{sec[KEY_SNAPSHOT]}
{
    title += " - Sinclair ZX-Spectrum 48K";
}

std::string ZXSpectrumConfig::to_string() const
{
    std::ostringstream os{};

    os << Config::to_string() << "\n"
        "  Output tape:        " << std::quoted(otape)          << "\n"
        "  Input tape:         " << std::quoted(itape)          << "\n"
        "  Tape fastload:      " << (fastload ? "yes" : "no")   << "\n"
        "  Snapshot:           " << std::quoted(snap);

    return os.str();
}

}
}
}
