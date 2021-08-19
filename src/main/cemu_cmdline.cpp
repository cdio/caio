/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "cemu_cmdline.hpp"

#include <iomanip>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "fs.hpp"
#include "logger.hpp"
#include "version.hpp"


namespace cemu {

const std::vector<::option> CemuCmdline::lopts = {
    { "conf",       required_argument,  nullptr, CemuCmdline::OPTION_CONF       },

    { "romdir",     required_argument,  nullptr, CemuCmdline::OPTION_ROMDIR     },
    { "cartdir",    required_argument,  nullptr, CemuCmdline::OPTION_CARTDIR    },
    { "palettedir", required_argument,  nullptr, CemuCmdline::OPTION_PALETTEDIR },
    { "keymapsdir", required_argument,  nullptr, CemuCmdline::OPTION_KEYMAPSDIR },

    { "palette",    required_argument,  nullptr, CemuCmdline::OPTION_PALETTE    },
    { "keymaps",    required_argument,  nullptr, CemuCmdline::OPTION_KEYMAPS    },

    { "fps",        required_argument,  nullptr, CemuCmdline::OPTION_FPS        },
    { "scale",      required_argument,  nullptr, CemuCmdline::OPTION_SCALE      },
    { "scanlines",  required_argument,  nullptr, CemuCmdline::OPTION_SCANLINES  },
    { "fullscreen", no_argument,        nullptr, CemuCmdline::OPTION_FULLSCREEN },
    { "sresize",    required_argument,  nullptr, CemuCmdline::OPTION_SRESIZE    },
    { "audio",      required_argument,  nullptr, CemuCmdline::OPTION_AUDIO      },

    { "delay",      required_argument,  nullptr, CemuCmdline::OPTION_DELAY      },
    { "monitor",    no_argument,        nullptr, CemuCmdline::OPTION_MONITOR    },

    { "logfile",    required_argument,  nullptr, CemuCmdline::OPTION_LOGFILE    },
    { "loglevel",   required_argument,  nullptr, CemuCmdline::OPTION_LOGLEVEL   },

    { "panel",      required_argument,  nullptr, CemuCmdline::OPTION_PANEL      },

    { "version",    no_argument,        nullptr, CemuCmdline::OPTION_VERSION    },
    { "help",       no_argument,        nullptr, CemuCmdline::OPTION_HELP       }
};


CemuCmdline::CemuCmdline(const std::vector<::option> &ext_lopts)
    : _lopts{lopts.begin(), lopts.end()}
{
    _lopts.insert(_lopts.end(), ext_lopts.begin(), ext_lopts.end());
    _lopts.push_back({nullptr, 0, nullptr, 0});
}

void CemuCmdline::usage()
{
               // 0         1         2         3         4         5         6         7
               // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    std::cerr << "usage: " << _progname << " <options>"                                                 << std::endl
              << "where <options> are:"                                                                 << std::endl
              << " --conf <confile>       Configuration file"                                           << std::endl
              << "                        (default is " << CemuConfile::CEMU_CONFIG_FILE << ")"         << std::endl
              << " --romdir <romdir>      ROMs directory"                                               << std::endl
              << " --cartdir <cdir>       Cartridge directory"                                          << std::endl
              << " --palettedir <pdir>    Colour palette directory"                                     << std::endl
              << " --palette <palette>    Colour palette file"                                          << std::endl
              << " --keymapsdir <kdir>    Key mappings directory"                                       << std::endl
              << " --keymaps <keymaps>    Key mappings file"                                            << std::endl
              << " --fps <rate>           Frame rate"                                                   << std::endl
              << "                        (default is " << Config::DEFAULT_FPS << ")"                   << std::endl
              << " --scale <scale>        Graphics scale factor"                                        << std::endl
              << "                        (default is " << std::setprecision(2) << Config::DEFAULT_SCALE << ")"
                                                                                                        << std::endl
              << " --scanlines <v|h|n>    Scanlines effect (horizontal, vertical or none)"              << std::endl
              << "                        (default is " << std::quoted(Config::DEFAULT_SCANLINES_EFFECT)
                                                                                                        << ")"
                                                                                                        << std::endl
              << " --fullscreen           Launch in fullscreen mode"                                    << std::endl
              << " --sresize <yes|no>     Smooth window resize"                                         << std::endl
              << "                        (default is " << (Config::DEFAULT_SMOOTH_RESIZE ? "yes" : "no") << ")"
                                                                                                        << std::endl
              << " --audio <yes|no>       Enable (disable) audio"                                       << std::endl
              << "                        (default is " << (Config::DEFAULT_AUDIO_ENABLED ? "yes" : "no") << ")"
                                                                                                        << std::endl
              << " --delay <delay>        Speed delay factor"                                           << std::endl
              << "                        (default is " << std::setprecision(2) << Config::DEFAULT_DELAY_FACTOR << ")"
                                                                                                        << std::endl
              << " --monitor              Activate the CPU monitor"                                     << std::endl
              << " --logfile <file>       Write log information into the specified file"                << std::endl
              << "                        (default is " << Config::DEFAULT_LOGFILE << ")"               << std::endl
              << " --loglevel <lv>        Use the specified loglevel; a combination of:"                << std::endl
              << "                            E: Show errors messages"                                  << std::endl
              << "                            W: Show warning messages"                                 << std::endl
              << "                            I: Show info messages"                                    << std::endl
              << "                            D: Show debug messages"                                   << std::endl
              << "                        (default is " << std::quoted(Config::DEFAULT_LOGLEVEL) << ")" << std::endl
              << " --panel <yes|no>       Panel visibility"                                             << std::endl
              << "                        (default is " << (Config::DEFAULT_PANEL ? "yes" : "no") << ")"
                                                                                                        << std::endl
              << " -v|--version           Show version information and exit"                            << std::endl
              << " -h|--help              Print this message and exit"                                  << std::endl;
}

Confile CemuCmdline::parse(int argc, char *const *argv)
{
    _progname = fs::basename(argv[0]);

    CemuConfile conf{};
    std::string cfile{};
    int c{};

    while ((c = ::getopt_long(argc, argv, "c:hv", &_lopts[0], nullptr)) != -1) {
        switch (c) {
        case OPTION_CONF:
            cfile = optarg;
            break;

        case 'v':
        case OPTION_VERSION:
            std::cout << "CEMU v" << version() << std::endl;
            std::exit(EXIT_SUCCESS);

        case 'h':
        case OPTION_HELP:
            usage();
            std::exit(EXIT_FAILURE);

        default:;
        }
    }

    if (cfile.empty()) {
        try {
            /*
             * Search for the configuration file in the default search path.
             */
            conf.parse();
        } catch (const ConfileError &) {
        }
    } else {
        try {
            /*
             * The user specified a configuration file name.
             */
            conf.parse(cfile, { CemuConfile::CWD });
        } catch (const std::exception &err) {
            std::cerr << _progname << ": " << err.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    auto &sec = conf[std::string{conf.CEMU_CONFIG_SECTION}];
    bool terminate{};
    optind = 0;

    while (!terminate) {
        c = ::getopt_long(argc, argv, "", &_lopts[0], nullptr);
        if (c == -1) {
            break;
        }

        switch (c) {
        case OPTION_ROMDIR:         /* ROM directory */
            sec[conf.ROMDIR_CONFIG_KEY] = optarg;
            break;

        case OPTION_CARTDIR:        /* Cartridge directory */
            sec[conf.CARTDIR_CONFIG_KEY] = optarg;
            break;

        case OPTION_PALETTEDIR:     /* Palette directory */
            sec[conf.PALETTEDIR_CONFIG_KEY] = optarg;
            break;

        case OPTION_PALETTE:        /* Palette file */
            sec[conf.PALETTE_CONFIG_KEY] = optarg;
            break;

        case OPTION_KEYMAPSDIR:     /* Keymaps directory */
            sec[conf.KEYMAPSDIR_CONFIG_KEY] = optarg;
            break;

        case OPTION_KEYMAPS:        /* Keymaps file */
            sec[conf.KEYMAPS_CONFIG_KEY] = optarg;
            break;

        case OPTION_FPS:            /* Frame rate */
            sec[conf.FPS_CONFIG_KEY] = optarg;
            break;

        case OPTION_SCALE:          /* Scale factor */
            sec[conf.SCALE_CONFIG_KEY] = optarg;
            break;

        case OPTION_SCANLINES:      /* Scanlines effect */
            sec[conf.SCANLINES_CONFIG_KEY] = optarg;
            break;

        case OPTION_FULLSCREEN:     /* Fullscreen mode */
            sec[conf.FULLSCREEN_CONFIG_KEY] = "yes";
            break;

        case OPTION_SRESIZE:        /* Smooth window resize */
            sec[conf.SRESIZE_CONFIG_KEY] = optarg;
            break;

        case OPTION_AUDIO:          /* Enable Audio */
            sec[conf.AUDIO_CONFIG_KEY] = optarg;
            break;

        case OPTION_DELAY:          /* Speed delay */
            sec[conf.DELAY_CONFIG_KEY] = optarg;
            break;

        case OPTION_MONITOR:        /* Monitor */
            sec[conf.MONITOR_CONFIG_KEY] = "yes";
            break;

        case OPTION_PANEL:          /* Panel position */
            sec[conf.PANEL_CONFIG_KEY] = optarg;
            break;

        case OPTION_LOGFILE:        /* Logfile */
            sec[conf.LOGFILE_CONFIG_KEY] = optarg;
            break;

        case OPTION_LOGLEVEL:       /* Loglevel */
            sec[conf.LOGLEVEL_CONFIG_KEY] = optarg;
            break;

        case OPTION_CONF:
        case OPTION_HELP:
            break;

        default:
            terminate = !parse(conf, c, (optarg == nullptr ? "" : optarg));
        }
    }

    if (optind != argc) {
        std::cerr << _progname << ": Invalid option -- '" << argv[optind] << "'" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    return conf;
}

}
