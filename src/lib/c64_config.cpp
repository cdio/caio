/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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

#include <iomanip>
#include <sstream>

#include "utils.hpp"


namespace caio {
namespace c64 {

C64Config &C64Config::operator=(const Confile &conf)
{
    Config::operator=(conf);

    title += " - C64";

    const auto secit = conf.find(C64Confile::C64_CONFIG_SECTION);
    if (secit != conf.end()) {
        const auto &sec = secit->second;

        auto it = sec.find(C64Confile::C64_CARTFILE_CONFIG_KEY);
        if (it != sec.end()) {
            cartfile = it->second;
        }

        it = sec.find(C64Confile::C64_PRGFILE_CONFIG_KEY);
        if (it != sec.end()) {
            prgfile = it->second;
        }

        it = sec.find(C64Confile::C64_RESID_CONFIG_KEY);
        if (it != sec.end()) {
            std::string str = utils::tolow(it->second);
            if (!str.empty()) {
                resid = (str == "yes" || str == "ye" || str == "y");
            }
        }

        it = sec.find(C64Confile::C64_SWAPJOY_CONFIG_KEY);
        if (it != sec.end()) {
            std::string str = utils::tolow(it->second);
            if (!str.empty()) {
                swapj = (str == "yes" || str == "ye" || str == "y");
            }
        }

        /* Palette entry in the c64 section, overrides any palette set in the caio section */
        it = sec.find(CaioConfile::PALETTE_CONFIG_KEY);
        if (it != sec.end()) {
            Config::palettefile = palette_file(it->second);
        }

        it = sec.find(C64Confile::C64_UNIT_8_CONFIG_KEY);
        if (it != sec.end()) {
            unit8 = it->second;
        }

        it = sec.find(C64Confile::C64_UNIT_9_CONFIG_KEY);
        if (it != sec.end()) {
            unit9 = it->second;
        }
    }

    return *this;
}

std::string C64Config::to_string() const
{
    std::ostringstream os{};

    os << Config::to_string() << std::endl
       << "  Use MOS6581 reSID:  " << (resid ? "yes" : "no") << std::endl
       << "  Swap Joysticks:     " << (swapj ? "yes" : "no") << std::endl
       << "  Attached Cartridge: " << std::quoted(cartfile)  << std::endl
       << "  Attached PRG:       " << std::quoted(prgfile)   << std::endl
       << "  Unit-8:             " << std::quoted(unit8)     << std::endl
       << "  Unit-9:             " << std::quoted(unit9);

    return os.str();
}

}
}
