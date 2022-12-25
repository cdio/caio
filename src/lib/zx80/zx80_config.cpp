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

#include <iomanip>
#include <sstream>

#include "utils.hpp"


namespace caio {
namespace zx80 {

ZX80Confile::ZX80Confile()
{
}

ZX80Confile::ZX80Confile(Confile &&other)
{
    *this = std::move(other);
}

ZX80Confile::~ZX80Confile()
{
}

ZX80Confile &ZX80Confile::operator=(Confile &&other)
{
    static_cast<Confile &>(*this) = std::move(other);
    return *this;
}

ZX80Config::ZX80Config()
{
}

ZX80Config::ZX80Config(const Confile &conf)
{
    *this = conf;
}

ZX80Config::~ZX80Config()
{
}

ZX80Config &ZX80Config::operator=(const Confile &conf)
{
    Config::operator=(conf);

    title += " - ZX80";

    const auto secit = conf.find(ZX80Confile::ZX80_CONFIG_SECTION);
    if (secit != conf.end()) {
        const auto &sec = secit->second;

        auto it = sec.find(ZX80Confile::ZX80_16K_RAM_CONFIG_KEY);
        if (it != sec.end()) {
            std::string str = utils::tolow(it->second);
            if (!str.empty()) {
                ram16 = (str == "yes" || str == "ye" || str == "y");
            }
        }

        it = sec.find(ZX80Confile::ZX80_8K_ROM_CONFIG_KEY);
        if (it != sec.end()) {
            std::string str = utils::tolow(it->second);
            if (!str.empty()) {
                rom8 = (str == "yes" || str == "ye" || str == "y");
            }
        }

        /* Palette entry in the zx80 section, overrides any palette set in the caio section */
        it = sec.find(CaioConfile::PALETTE_CONFIG_KEY);
        if (it != sec.end()) {
            Config::palettefile = palette_file(it->second);
        }
    }

    return *this;
}

std::string ZX80Config::palette_file(const std::string &palette) const
{
    return "zx80_" + Config::palette_file(palette);
}

std::string ZX80Config::keymaps_file(const std::string &cc) const
{
    return "zx80_" + Config::keymaps_file(cc);
}

std::string ZX80Config::to_string() const
{
    std::ostringstream os{};

    os << Config::to_string() << std::endl
       << "  Attach 16K RAM   :  " << (ram16 ? "yes" : "no") << std::endl
       << "  Use 8K ROM:         " << (rom8  ? "yes" : "no");

    return os.str();
}

}
}
