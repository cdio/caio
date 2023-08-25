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
#include "zx80_cmdline.hpp"

#include <iostream>

#include "zx80_config.hpp"


namespace caio {
namespace zx80 {

const std::vector<::option> ZX80Cmdline::lopts = {
    { "ram16",   required_argument,  nullptr,    ZX80Cmdline::ZX80_OPTION_16K_RAM },
    { "rom8",    required_argument,  nullptr,    ZX80Cmdline::ZX80_OPTION_8K_ROM  }
};

void ZX80Cmdline::usage()
{
    CaioCmdline::usage();

               // 0         1         2         3         4         5         6         7
               // 01234567890123456789012345678901234567890123456789012345678901234567890123456789
    std::cerr << std::endl
              << "ZX80 specific:" << std::endl
              << " --ram16                Attach a 16K RAM instead of the default 1K RAM"   << std::endl
              << " --rom8                 Attach the 8K ROM instead of the default 4K ROM"  << std::endl;
}

bool ZX80Cmdline::parse(Confile& conf, int opt, const std::string& arg)
{
    auto& sec = conf[std::string{ZX80Confile::ZX80_CONFIG_SECTION}];

    switch (opt) {
    case ZX80_OPTION_16K_RAM:   /* Attach a 16K (external) RAM */
        sec[ZX80Confile::ZX80_16K_RAM_CONFIG_KEY] = "yes";
        break;

    case ZX80_OPTION_8K_ROM:    /* Attach the 8K ROM */
        sec[ZX80Confile::ZX80_8K_ROM_CONFIG_KEY] = "yes";
        break;

    default:
        return false;
    }

    return true;
}

}
}
