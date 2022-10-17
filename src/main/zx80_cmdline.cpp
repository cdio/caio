/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include "zx80_cmdline.hpp"

#include <iostream>

#include "zx80_config.hpp"


namespace caio {
namespace zx80 {

const std::vector<::option> ZX80Cmdline::lopts = {
//TODO
#if 0
    { "cart",   required_argument,  nullptr,    C64Cmdline::C64_OPTION_CARTFILE },
    { "prg",    required_argument,  nullptr,    C64Cmdline::C64_OPTION_PRGFILE  },
    { "resid",  required_argument,  nullptr,    C64Cmdline::C64_OPTION_RESID    },
    { "swapj",  no_argument,        nullptr,    C64Cmdline::C64_OPTION_SWAPJOY  },
    { "8",      required_argument,  nullptr,    C64Cmdline::C64_OPTION_UNIT_8   },
    { "9",      required_argument,  nullptr,    C64Cmdline::C64_OPTION_UNIT_9   }
#endif
};

ZX80Cmdline::ZX80Cmdline()
    : CaioCmdline{lopts}
{
}

ZX80Cmdline::~ZX80Cmdline()
{
}

Confile ZX80Cmdline::parse(int argc, char *const *argv)
{
    return CaioCmdline::parse(argc, argv);
}

void ZX80Cmdline::usage()
{
    CaioCmdline::usage();

               // 0         1         2         3         4         5         6         7
               // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    std::cerr << std::endl
              << "ZX80 specific:" << std::endl;
}

bool ZX80Cmdline::parse(Confile &conf, int opt, const std::string &arg)
{
//TODO
#if 0
    auto &sec = conf[std::string{ZX80Confile::ZX80_CONFIG_SECTION}];

    switch (opt) {
    default:;
        return false;
    }
#endif
    return true;
}

}
}
