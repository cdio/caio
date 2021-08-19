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
#include "c64_cmdline.hpp"

#include <iostream>

#include "c64_config.hpp"


namespace cemu {
namespace c64 {

const std::vector<::option> C64Cmdline::lopts = {
    { "cart",   required_argument,  nullptr,    C64Cmdline::C64_OPTION_CARTFILE },
    { "prg",    required_argument,  nullptr,    C64Cmdline::C64_OPTION_PRGFILE  },
    { "resid",  required_argument,  nullptr,    C64Cmdline::C64_OPTION_RESID    },
    { "swapj",  no_argument,        nullptr,    C64Cmdline::C64_OPTION_SWAPJOY  },
    { "8",      required_argument,  nullptr,    C64Cmdline::C64_OPTION_UNIT_8   },
    { "9",      required_argument,  nullptr,    C64Cmdline::C64_OPTION_UNIT_9   }
};


void C64Cmdline::usage()
{
    CemuCmdline::usage();

               // 0         1         2         3         4         5         6         7
               // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    std::cerr << std::endl
              << "C64 specific:"                                                                    << std::endl
              << " --cart <cart>          Attach a ROM image (RAW or CRT formats, 8K or 16K only)"  << std::endl
              << " --prg <prg>            Load a PRG file as soon as the basic is ready"            << std::endl
              << " --8 <path>             Attach a disk drive unit 8"                               << std::endl
              << " --9 <path>             Attach a disk drive unit 9"                               << std::endl
              << " --resid <yes|no>       Use the MOS6581 reSID library"                            << std::endl
              << "                        (default is no; cemu implementation is used)"             << std::endl
              << " --swapj                Swap Joysticks"                                           << std::endl;
}

bool C64Cmdline::parse(Confile &conf, int opt, const std::string &arg)
{
    auto &sec = conf[std::string{C64Confile::C64_CONFIG_SECTION}];

    switch (opt) {
    case C64_OPTION_CARTFILE:   /* Attach a Cartridge image */
        sec[C64Confile::C64_CARTFILE_CONFIG_KEY] = arg;
        break;

    case C64_OPTION_PRGFILE:    /* Load a PRG file */
        sec[C64Confile::C64_PRGFILE_CONFIG_KEY] = arg;
        break;

    case C64_OPTION_UNIT_8:
        sec[C64Confile::C64_UNIT_8_CONFIG_KEY] = arg;
        break;

    case C64_OPTION_UNIT_9:
        sec[C64Confile::C64_UNIT_9_CONFIG_KEY] = arg;
        break;

    case C64_OPTION_RESID:      /* reSID library */
        sec[C64Confile::C64_RESID_CONFIG_KEY] = arg;
        break;

    case C64_OPTION_SWAPJOY:    /* Swap Joysticks */
        sec[C64Confile::C64_SWAPJOY_CONFIG_KEY] = "yes";
        break;

    default:
        return false;
    }

    return true;
}

}
}
