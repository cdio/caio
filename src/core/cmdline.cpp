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
#include "config.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>

#include <gsl/assert>

#include "fs.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "version.hpp"


namespace caio {
namespace config {

bool is_true(const std::string& value)
{
    auto val = utils::tolow(value);
    return (val == "yes" || val == "ye" || val == "y");
}

bool is_false(const std::string& value)
{
    auto val = utils::tolow(value);
    return (val == "no" || val == "n");
}

bool set_value(Confile& cf, const Option& opt, const std::string& value)
{
    cf[opt.sname][opt.key] = value;
    return true;
}

bool set_true(Confile& cf, const Option& opt, const std::string&)
{
    cf[opt.sname][opt.key] = "yes";
    return true;
}

bool set_bool(Confile& cf, const Option& opt, const std::string& value)
{
    auto& cfvalue = cf[opt.sname][opt.key];
    if (is_true(value)) {
        cfvalue = "yes";
    } else if (is_false(value)) {
        cfvalue = "no";
    } else {
        return false;
    }
    return true;
}

static Option generic_options[] = {
    { "conf",       SEC_GENERIC,    KEY_CONFIG_FILE,    CONFIG_FILE,        Arg::Required,  set_value           },
    { "romdir",     SEC_GENERIC,    KEY_ROMDIR,         DEFAULT_ROMDIR,     Arg::Required,  set_value           },
    { "palettedir", SEC_GENERIC,    KEY_PALETTEDIR,     DEFAULT_PALETTEDIR, Arg::Required,  set_value           },
    { "keymapsdir", SEC_GENERIC,    KEY_KEYMAPSDIR,     DEFAULT_KEYMAPSDIR, Arg::Required,  set_value           },
    { "palette",    SEC_GENERIC,    KEY_PALETTE,        DEFAULT_PALETTE,    Arg::Required,  set_value           },
    { "keymaps",    SEC_GENERIC,    KEY_KEYMAPS,        DEFAULT_KEYMAPS,    Arg::Required,  set_value           },
    { "cart",       SEC_GENERIC,    KEY_CARTRIDGE,      DEFAULT_CARTRIDGE,  Arg::Required,  set_value           },
    { "fps",        SEC_GENERIC,    KEY_FPS,            DEFAULT_FPS,        Arg::Required,  set_value           },
    { "scale",      SEC_GENERIC,    KEY_SCALE,          DEFAULT_SCALE,      Arg::Required,  set_value           },
    { "scanlines",  SEC_GENERIC,    KEY_SCANLINES,      DEFAULT_SCANLINES,  Arg::Required,  set_value           },
    { "fullscreen", SEC_GENERIC,    KEY_FULLSCREEN,     DEFAULT_FULLSCREEN, Arg::Optional,  set_bool,   "yes"   },
    { "sresize",    SEC_GENERIC,    KEY_SRESIZE,        DEFAULT_SRESIZE,    Arg::Optional,  set_bool,   "yes"   },
    { "audio",      SEC_GENERIC,    KEY_AUDIO,          DEFAULT_AUDIO,      Arg::Optional,  set_bool,   "yes"   },
    { "delay",      SEC_GENERIC,    KEY_DELAY,          DEFAULT_DELAY,      Arg::Required,  set_value           },
    { "monitor",    SEC_GENERIC,    KEY_MONITOR,        DEFAULT_MONITOR,    Arg::Optional,  set_bool,   "yes"   },
    { "logfile",    SEC_GENERIC,    KEY_LOGFILE,        DEFAULT_LOGFILE,    Arg::Required,  set_value           },
    { "loglevel",   SEC_GENERIC,    KEY_LOGLEVEL,       DEFAULT_LOGLEVEL,   Arg::Required,  set_value           },
    { "vjoy",       SEC_GENERIC,    KEY_VJOY,           DEFAULT_VJOY,       Arg::Optional,  set_bool,   "yes"   },
    { "vjoy-up",    SEC_GENERIC,    KEY_VJOY_UP,        DEFAULT_VJOY_UP,    Arg::Required,  set_value           },
    { "vjoy-down",  SEC_GENERIC,    KEY_VJOY_DOWN,      DEFAULT_VJOY_DOWN,  Arg::Required,  set_value           },
    { "vjoy-left",  SEC_GENERIC,    KEY_VJOY_LEFT,      DEFAULT_VJOY_LEFT,  Arg::Required,  set_value           },
    { "vjoy-right", SEC_GENERIC,    KEY_VJOY_RIGHT,     DEFAULT_VJOY_RIGHT, Arg::Required,  set_value           },
    { "vjoy-fire",  SEC_GENERIC,    KEY_VJOY_FIRE,      DEFAULT_VJOY_FIRE,  Arg::Required,  set_value           }
};

std::string Cmdline::usage() const
{
    std::ostringstream os{};

        // 0         1         2         3         4         5         6         7
        // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    os << "usage: " << _progname << " <options>"                                                          << std::endl
       << "where <options> are:"                                                                          << std::endl
       << " --conf <cfile>          Configuration file"                                                   << std::endl
       << " --romdir <romdir>       ROMs directory"                                                       << std::endl
       << " --palettedir <pdir>     Colour palette directory"                                             << std::endl
       << " --palette <palette>     Colour palette name or filename"                                      << std::endl
       << " --keymapsdir <kdir>     Key mappings directory"                                               << std::endl
       << " --keymaps <keymaps>     Key mappings name or filename"                                        << std::endl
       << " --cart <cfile>          Cartridge filename"                                                   << std::endl
       << " --fps <rate>            Frame rate (default is " << DEFAULT_FPS << ")"                        << std::endl
       << " --scale <scale>         Window scale factor (default is " << DEFAULT_SCALE << ")"             << std::endl
       << " --scanlines <n|h|v|H|V> Scanlines effect: (n)one, (h)orizontal, (v)ertical,"                  << std::endl
       << "                         advanced (H)orizontal, advanced (V)ertical"                           << std::endl
       << "                         (default is " << DEFAULT_SCANLINES << ")"                             << std::endl
       << " --fullscreen [yes|no]   Start in fullscreen mode"                                             << std::endl
       << " --sresize [yes|no]      Smooth window resize (default is " << DEFAULT_SRESIZE << ")"          << std::endl
       << " --audio [yes|no]        Enable audio (default is " << DEFAULT_AUDIO << ")"                    << std::endl
       << " --delay <delay>         Clock delay factor (default is " << DEFAULT_DELAY << ")"              << std::endl
       << " --monitor [yes|no]      Activate the CPU monitor (default is " << DEFAULT_MONITOR << ")"      << std::endl
       << " --logfile <file>        Send log information to the specified destination"                    << std::endl
       << "                         (default is " << DEFAULT_LOGFILE << ")"                               << std::endl
       << " --loglevel <lv>         Loglevel, bitwise combination of: "                                   << std::endl
       << "                         none|error|warn|info|debug|all (default is " << DEFAULT_LOGLEVEL << ")" << std::endl
       << " --vjoy [yes|no]         Enable virtual joystick (default is " << DEFAULT_VJOY << ")"          << std::endl
       << " --vjoy-up <keyname>     Virtual joystick UP key (default is " << DEFAULT_VJOY_UP << ")"       << std::endl
       << " --vjoy-down <keyname>   Virtual joystick DOWN key (default is " << DEFAULT_VJOY_DOWN << ")"   << std::endl
       << " --vjoy-left <keyname>   Virtual joystick LEFT key (default is " << DEFAULT_VJOY_LEFT << ")"   << std::endl
       << " --vjoy-right <keyname>  Virtual joystick RIGHT key (default is " << DEFAULT_VJOY_RIGHT << ")" << std::endl
       << " --vjoy-fire <keyname>   Virtual joystick FIRE key (default is " << DEFAULT_VJOY_FIRE << ")"   << std::endl
       << " -v|--version            Show version information and exit"                                    << std::endl
       << " -h|--help               Print this message and exit";

    return os.str();
}

Confile Cmdline::defaults()
{
    Confile cf{};
    const auto& opts = options();

    std::for_each(opts.begin(), opts.end(), [&cf](const Option& opt) {
        cf[opt.sname][opt.key] = opt.dvalue;
    });

    return cf;
}

Confile Cmdline::parse(int argc, const char** argv)
{
    using namespace gsl;
    Expects(argc > 0 && argv != nullptr && *argv != nullptr);

    _progname = fs::basename(argv[0]);
    const auto& opts = options();

    Confile cf{};

    /*
     * Process special cases --help and --version.
     */
    for (int i = 1; i < argc; ++i) {
        const std::string arg{argv[i]};
        if (arg == "?" || arg == "-?" || arg == "--?" || arg == "-h" || arg == "--h" ||
            arg == "-help" || arg == "--help") {
            std::cerr << usage() << std::endl;
            std::exit(EXIT_FAILURE);
            /* NOTREACHED */
        }

        if (arg == "-v" || arg == "--v" || arg == "-version" || arg == "--version") {
            std::cerr << full_version() << std::endl;
            std::exit(EXIT_SUCCESS);
            /* NOTREACHED */
        }
    }

    /*
     * No special cases, parse the command line normally.
     */
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == nullptr) {
            break;
        }

        const std::string optstr{argv[i]};
        size_t pos = 0;
        if (optstr.starts_with("--")) {
            pos = 2;
        } else if (optstr.starts_with("-")) {
            pos = 1;
        } else {
            throw InvalidArgument{"Invalid command line option: " + optstr};
        }

        std::string optval{};
        const auto& useropt = optstr.substr(pos);
        size_t j = 0;
        while (j < opts.size()) {
            const auto& opt = opts[j];
            if (useropt == opt.name) {
                switch (opt.type) {
                case Arg::None:
                    opt.fn(cf, opt, "");
                    break;
                case Arg::Optional:
                    if ((i + 1 >= argc) || std::string{argv[i + 1]}.starts_with("--")) {
                        optval = opt.optval;
                    } else {
                        ++i;
                        optval = argv[i];
                    }
                    if (!opt.fn(cf, opt, optval)) {
                        throw InvalidArgument{"Invalid optional parameter: option " + optstr + ", parameter " +
                            optval};
                    }
                    break;
                case Arg::Required:
                    if (i + 1 >= argc) {
                        throw InvalidArgument{"Missing parameter: " + optstr};
                    }
                    ++i;
                    if (!opt.fn(cf, opt, argv[i])) {
                        throw InvalidArgument{"Invalid parameter: option " + optstr + ", parameter " + argv[i]};
                    }
                    break;
                }
                break;
            }

            ++j;
        }

        if (j == opts.size()) {
            throw InvalidArgument{"Invalid option: " + optstr};
        }
    }

    return cf;
}

std::vector<Option> Cmdline::options() const
{
    return {&generic_options[0], &generic_options[std::size(generic_options)]};
}

}
}
