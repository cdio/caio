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

#include "fs.hpp"
#include "types.hpp"
#include "utils.hpp"
#include "version.hpp"

namespace caio {
namespace config {

bool is_true(std::string_view value)
{
    auto val = caio::tolow(value);
    return (val == "yes" || val == "ye" || val == "y");
}

bool is_false(std::string_view value)
{
    auto val = caio::tolow(value);
    return (val == "no" || val == "n");
}

bool set_value(Confile& cf, const Option& opt, std::string_view value)
{
    cf[opt.sname][opt.key] = value;
    return true;
}

bool set_true(Confile& cf, const Option& opt, std::string_view value)
{
    cf[opt.sname][opt.key] = "yes";
    return true;
}

bool set_bool(Confile& cf, const Option& opt, std::string_view value)
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
    { "conf",          SEC_GENERIC, KEY_CONFIG_FILE, CONFIG_FILE,         Arg::Required, set_value       },
    { KEY_ROMDIR,      SEC_GENERIC, KEY_ROMDIR,      DEFAULT_ROMDIR,      Arg::Required, set_value       },
    { KEY_PALETTEDIR,  SEC_GENERIC, KEY_PALETTEDIR,  DEFAULT_PALETTEDIR,  Arg::Required, set_value       },
    { KEY_KEYMAPSDIR,  SEC_GENERIC, KEY_KEYMAPSDIR,  DEFAULT_KEYMAPSDIR,  Arg::Required, set_value       },
    { KEY_PALETTE,     SEC_GENERIC, KEY_PALETTE,     DEFAULT_PALETTE,     Arg::Required, set_value       },
    { KEY_KEYMAPS,     SEC_GENERIC, KEY_KEYMAPS,     DEFAULT_KEYMAPS,     Arg::Required, set_value       },
    { KEY_CARTRIDGE,   SEC_GENERIC, KEY_CARTRIDGE,   DEFAULT_CARTRIDGE,   Arg::Required, set_value       },
    { KEY_FPS,         SEC_GENERIC, KEY_FPS,         DEFAULT_FPS,         Arg::Required, set_value       },
    { KEY_SCALE,       SEC_GENERIC, KEY_SCALE,       DEFAULT_SCALE,       Arg::Required, set_value       },
    { KEY_SCANLINES,   SEC_GENERIC, KEY_SCANLINES,   DEFAULT_SCANLINES,   Arg::Required, set_value       },
    { KEY_FULLSCREEN,  SEC_GENERIC, KEY_FULLSCREEN,  DEFAULT_FULLSCREEN,  Arg::Optional, set_bool, "yes" },
    { KEY_SRESIZE,     SEC_GENERIC, KEY_SRESIZE,     DEFAULT_SRESIZE,     Arg::Optional, set_bool, "yes" },
    { KEY_AUDIO,       SEC_GENERIC, KEY_AUDIO,       DEFAULT_AUDIO,       Arg::Optional, set_bool, "yes" },
    { KEY_DELAY,       SEC_GENERIC, KEY_DELAY,       DEFAULT_DELAY,       Arg::Required, set_value       },
    { KEY_MONITOR,     SEC_GENERIC, KEY_MONITOR,     DEFAULT_MONITOR,     Arg::Optional, set_bool, "yes" },
    { KEY_LOGFILE,     SEC_GENERIC, KEY_LOGFILE,     DEFAULT_LOGFILE,     Arg::Required, set_value       },
    { KEY_LOGLEVEL,    SEC_GENERIC, KEY_LOGLEVEL,    DEFAULT_LOGLEVEL,    Arg::Required, set_value       },
    { KEY_VJOY,        SEC_GENERIC, KEY_VJOY,        DEFAULT_VJOY,        Arg::Optional, set_bool, "yes" },
    { KEY_VJOY_UP,     SEC_GENERIC, KEY_VJOY_UP,     DEFAULT_VJOY_UP,     Arg::Required, set_value       },
    { KEY_VJOY_DOWN,   SEC_GENERIC, KEY_VJOY_DOWN,   DEFAULT_VJOY_DOWN,   Arg::Required, set_value       },
    { KEY_VJOY_LEFT,   SEC_GENERIC, KEY_VJOY_LEFT,   DEFAULT_VJOY_LEFT,   Arg::Required, set_value       },
    { KEY_VJOY_RIGHT,  SEC_GENERIC, KEY_VJOY_RIGHT,  DEFAULT_VJOY_RIGHT,  Arg::Required, set_value       },
    { KEY_VJOY_FIRE_A, SEC_GENERIC, KEY_VJOY_FIRE_A, DEFAULT_VJOY_FIRE_A, Arg::Required, set_value       },
    { KEY_VJOY_FIRE_B, SEC_GENERIC, KEY_VJOY_FIRE_B, DEFAULT_VJOY_FIRE_B, Arg::Required, set_value       }
};

std::string Cmdline::usage() const
{
      // 0         1         2         3         4         5         6         7
      // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    return std::format("usage: {} <options> [<file>]\n"
        "where <file> is the name of a program, cartridge or\n"
        "snapshot to launch (the file format is auto-detected)\n"
        "and <options> are:\n"
        " --conf <cfile>          Configuration file\n"
        " --romdir <romdir>       ROMs directory\n"
        " --palettedir <pdir>     Colour palette directory\n"
        " --palette <palette>     Colour palette name or filename\n"
        " --keymapsdir <kdir>     Key mappings directory\n"
        " --keymaps <keymaps>     Key mappings name or filename\n"
        " --cart <cfile>          Cartridge filename\n"
        " --fps <rate>            Frame rate (default is {})\n"
        " --scale <scale>         Window scale factor (default is {})\n"
        " --scanlines <n|h|v|H|V> Scanlines effect: (n)one, (h)orizontal, (v)ertical,\n"
        "                         advanced (H)orizontal, advanced (V)ertical\n"
        "                         (default is {})\n"
        " --fullscreen [yes|no]   Start in fullscreen mode\n"
        " --sresize [yes|no]      Smooth window resize (default is {})\n"
        " --audio [yes|no]        Enable audio (default is {})\n"
        " --delay <delay>         Clock delay factor (default is {})\n"
        " --monitor [yes|no]      Activate the CPU monitor (default is {})\n"
        " --logfile <file>        Send log information to the specified destination\n"
        "                         (default is {})\n"
        " --loglevel <lv>         Loglevel, bitwise combination of:\n"
        "                         none|error|warn|info|debug|all (default is {})\n"
        " --vjoy [yes|no]         Enable virtual joystick (default is {})\n"
        " --vjoy-up <keyname>     Virtual joystick UP key (default is {})\n"
        " --vjoy-down <keyname>   Virtual joystick DOWN key (default is {})\n"
        " --vjoy-left <keyname>   Virtual joystick LEFT key (default is {})\n"
        " --vjoy-right <keyname>  Virtual joystick RIGHT key (default is {})\n"
        " --vjoy-fire <keyname>   Virtual joystick FIRE-A key (default is {})\n"
        " --vjoy-fire-b <keyname> Virtual joystick FIRE-B key (default is {})\n"
        " -v|--version            Show version information and exit\n"
        " -h|--help               Print this message and exit",
        _progname,
        DEFAULT_FPS,
        DEFAULT_SCALE,
        DEFAULT_SCANLINES,
        DEFAULT_SRESIZE,
        DEFAULT_AUDIO,
        DEFAULT_DELAY,
        DEFAULT_MONITOR,
        DEFAULT_LOGFILE,
        DEFAULT_LOGLEVEL,
        DEFAULT_VJOY,
        DEFAULT_VJOY_UP,
        DEFAULT_VJOY_DOWN,
        DEFAULT_VJOY_LEFT,
        DEFAULT_VJOY_RIGHT,
        DEFAULT_VJOY_FIRE_A,
        DEFAULT_VJOY_FIRE_B);
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

std::pair<Confile, std::string> Cmdline::parse(int argc, const char** argv)
{
    CAIO_ASSERT(argc > 0 && argv != nullptr && *argv != nullptr);

    _progname = fs::basename(argv[0]);
    const auto& opts = options();

    Confile cf{};
    std::string pname{};

    /*
     * Process special cases --help and --version.
     */
    for (int i = 1; i < argc; ++i) {
        const std::string arg{argv[i]};
        if (arg == "?" || arg == "-?" || arg == "--?" || arg == "-h" || arg == "--h" ||
            arg == "-help" || arg == "--help") {
            std::cerr << usage() << "\n";
            std::exit(EXIT_FAILURE);
            /* NOTREACHED */
        }

        if (arg == "-v" || arg == "--v" || arg == "-version" || arg == "--version") {
            std::cerr << full_version() << "\n";
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
            if (i + 1 == argc) {
                pname = argv[i];
                continue;
            }
            throw InvalidArgument{"Invalid command line option: {}", optstr};
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
                        throw InvalidArgument{"Invalid optional parameter: option: {}, parameter: {}", optstr, optval};
                    }
                    break;
                case Arg::Required:
                    if (i + 1 >= argc) {
                        throw InvalidArgument{"Missing parameter: {}", optstr};
                    }
                    ++i;
                    if (!opt.fn(cf, opt, argv[i])) {
                        throw InvalidArgument{"Invalid parameter: option: {}, parameter: {}", optstr, argv[i]};
                    }
                    break;
                }
                break;
            }

            ++j;
        }

        if (j == opts.size()) {
            throw InvalidArgument{"Invalid option: {}", optstr};
        }
    }

    return {cf, pname};
}

std::vector<Option> Cmdline::options() const
{
    return {&generic_options[0], &generic_options[std::size(generic_options)]};
}

}
}
