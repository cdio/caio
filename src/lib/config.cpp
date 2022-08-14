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
#include "config.hpp"

#include "utils.hpp"
#include "logger.hpp"


namespace caio {

Config &Config::operator=(const Confile &conf)
{
    title = "caio";

    const auto secit = conf.find(CaioConfile::CAIO_CONFIG_SECTION);
    if (secit != conf.end()) {
        const auto &sec = secit->second;

        auto it = sec.find(CaioConfile::ROMDIR_CONFIG_KEY);
        romdir = (it != sec.end() ? it->second : CaioConfile::ROMDIR);

        it = sec.find(CaioConfile::CARTDIR_CONFIG_KEY);
        if (it != sec.end()) {
            cartdir = it->second;
        }

        it = sec.find(CaioConfile::PALETTEDIR_CONFIG_KEY);
        palettedir = (it != sec.end() ? it->second : CaioConfile::PALETTEDIR);

        it = sec.find(CaioConfile::PALETTE_CONFIG_KEY);
        if (it != sec.end()) {
            const auto &palette = it->second;
            palettefile = palette_file(palette);
        }

        it = sec.find(CaioConfile::KEYMAPSDIR_CONFIG_KEY);
        keymapsdir = (it != sec.end() ? it->second : CaioConfile::KEYMAPSDIR);

        it = sec.find(CaioConfile::KEYMAPS_CONFIG_KEY);
        if (it != sec.end()) {
            const auto &cc = it->second;
            keymapsfile = keymaps_file(cc);
        }

        it = sec.find(CaioConfile::FPS_CONFIG_KEY);
        if (it != sec.end()) {
            try {
                auto val = std::stoi(it->second);
                fps = (val > 0 ? val : DEFAULT_FPS);
            } catch (...) {
                fps = DEFAULT_FPS;
            }
        }

        it = sec.find(CaioConfile::SCALE_CONFIG_KEY);
        if (it != sec.end()) {
            const std::string &str = it->second;
            if (!str.empty()) {
                scale = std::atof(str.c_str());
            }
        }

        it = sec.find(CaioConfile::SCANLINES_CONFIG_KEY);
        if (it != sec.end()) {
            scanlines = utils::tolow(it->second);
        }

        it = sec.find(CaioConfile::FULLSCREEN_CONFIG_KEY);
        if (it != sec.end()) {
            const std::string str = utils::tolow(it->second);
            fullscreen = (str == "yes" || str == "ye" || str == "y");
        }

        it = sec.find(CaioConfile::SRESIZE_CONFIG_KEY);
        if (it != sec.end()) {
            const std::string str = utils::tolow(it->second);
            smooth_resize = (str == "yes" || str == "ye" || str == "y");
        }

        it = sec.find(CaioConfile::AUDIO_CONFIG_KEY);
        if (it != sec.end()) {
            const std::string str = utils::tolow(it->second);
            audio_enabled = (str == "yes" || str == "ye" || str == "y");
        }

        it = sec.find(CaioConfile::DELAY_CONFIG_KEY);
        if (it != sec.end()) {
            const std::string &str = it->second;
            if (!str.empty()) {
                delay = std::atof(str.c_str());
            }
        }

        it = sec.find(CaioConfile::MONITOR_CONFIG_KEY);
        if (it != sec.end()) {
            const std::string str = utils::tolow(it->second);
            monitor = (str == "yes" || str == "ye" || str == "y");
        }

        it = sec.find(CaioConfile::PANEL_CONFIG_KEY);
        if (it != sec.end()) {
            const std::string str = utils::tolow(it->second);
            panel = (str == "yes" || str == "ye" || str == "y");
        }

        it = sec.find(CaioConfile::LOGFILE_CONFIG_KEY);
        logfile = (it != sec.end() ? logfile = it->second : DEFAULT_LOGFILE);

        it = sec.find(CaioConfile::LOGLEVEL_CONFIG_KEY);
        loglevel = (it != sec.end() ? utils::tolow(it->second) : DEFAULT_LOGLEVEL);
    }

    return *this;
}

std::string Config::palette_file(const std::string &palette) const
{
    return (palette.ends_with(PALETTEFILE_SUFFIX) ? palette : palette + PALETTEFILE_SUFFIX);
}

std::string Config::keymaps_file(const std::string &cc) const
{
    return (cc.ends_with(KEYMAPSFILE_SUFFIX) ? cc : cc + KEYMAPSFILE_SUFFIX);
}

std::string Config::to_string() const
{
    std::ostringstream os{};

    os << "  Title:              " << std::quoted(title) << std::endl
       << "  ROMs path:          " << std::quoted(romdir) << std::endl
       << "  Cartridge path:     " << std::quoted(cartdir) << std::endl
       << "  Palette path:       " << std::quoted(palettedir) << std::endl
       << "  Palette file:       " << std::quoted(palettefile.empty() ? "" : palettefile) << std::endl
       << "  Keymaps path:       " << std::quoted(keymapsdir) << std::endl
       << "  Keymaps file:       " << std::quoted(keymapsfile.empty() ? "" : keymapsfile) << std::endl
       << "  FPS:                " << fps << std::endl
       << "  Scale:              " << scale << "x" << std::endl
       << "  Scanlines effect:   " << scanlines << std::endl
       << "  Fullscreen:         " << (fullscreen ? "yes" : "no") << std::endl
       << "  Smooth resize:      " << (smooth_resize ? "yes" : "no") << std::endl
       << "  Audio enabled:      " << (audio_enabled ? "yes" : "no") << std::endl
       << "  Speed Delay         " << delay << "x" << std::endl
       << "  CPU Monitor:        " << (monitor ? "yes" : "no") << std::endl
       << "  Panel visible:      " << (panel ? "yes" : "no") << std::endl
       << "  Log file:           " << logfile << std::endl
       << "  Log level:          " << loglevel;

    return os.str();
}

}
