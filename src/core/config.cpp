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

#include "fs.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "ui_config.hpp"


namespace caio {
namespace config {

Section parse(int argc, const char** argv, Cmdline& cmdline)
{
    Confile def{cmdline.defaults()};
    Confile cline{cmdline.parse(argc, argv)};
    Confile cfile{};

    const auto& gsec = cline[SEC_GENERIC];
    const auto& it = gsec.find(KEY_CONFIG_FILE);
    std::string fname{};
    if (it != gsec.end()) {
        /*
         * A configuration file is specified in the command line.
         */
        fname = it->second;
        log.debug("Configuration file: %s\n", fname.c_str());
        cfile.load(fname);
    } else {
        /*
         * Searh for the configuration file in standard directories.
         */
        fname = fs::search(CONFIG_FILE, {HOME_CONFDIR, SYSTEM_CONFDIR});
        if (fname.empty()) {
            log.debug("Configuration file not found. Using default values\n");
        } else {
            log.debug("Configuration file found: %s\n", fname.c_str());
            cfile.load(fname);
        }
    }

    const auto& sname = cmdline.sname();

    Section merged{};
    merged.merge(cline.extract(sname));
    merged.merge(cline.extract(SEC_GENERIC));

    merged.merge(cfile.extract(sname));
    merged.merge(cfile.extract(SEC_GENERIC));

    merged.merge(def.extract(sname));
    merged.merge(def.extract(SEC_GENERIC));

    return merged;
}

VJoyConfig::VJoyConfig(Section& sec)
    : VJoyKeys{.up    = keyboard::to_key(sec[KEY_VJOY_UP]),
               .down  = keyboard::to_key(sec[KEY_VJOY_DOWN]),
               .left  = keyboard::to_key(sec[KEY_VJOY_LEFT]),
               .right = keyboard::to_key(sec[KEY_VJOY_RIGHT]),
               .fire  = keyboard::to_key(sec[KEY_VJOY_FIRE])},
      enabled{is_true(sec[KEY_VJOY])}
{
    if (up == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick up key: " + sec[KEY_VJOY_UP]};
    }

    if (down == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick down key: " + sec[KEY_VJOY_DOWN]};
    }

    if (left == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick left key: " + sec[KEY_VJOY_LEFT]};
    }

    if (right == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick right key: " + sec[KEY_VJOY_RIGHT]};
    }

    if (fire == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick fire key: " + sec[KEY_VJOY_FIRE]};
    }
}

Config::Config(Section& sec, const std::string& prefix)
    : title{"caio"},
      romdir{sec[KEY_ROMDIR]},
      palette{(sec[KEY_PALETTE].empty() ? "" : resolve(sec[KEY_PALETTE], sec[KEY_PALETTEDIR], prefix, PALETTEFILE_EXT))},
      keymaps{(sec[KEY_KEYMAPS].empty() ? "" : resolve(sec[KEY_KEYMAPS], sec[KEY_KEYMAPSDIR], prefix, KEYMAPSFILE_EXT))},
      cartridge{sec[KEY_CARTRIDGE]},
      fps{static_cast<unsigned>(std::atoi(sec[KEY_FPS].c_str()))},
      scale{static_cast<unsigned>(std::atoi(sec[KEY_SCALE].c_str()))},
      scanlines{static_cast<char>(ui::to_sleffect(sec[KEY_SCANLINES]))},
      fullscreen{is_true(sec[KEY_FULLSCREEN])},
      sresize{is_true(sec[KEY_SRESIZE])},
      audio{is_true(sec[KEY_AUDIO])},
      delay{static_cast<float>(std::atof(sec[KEY_DELAY].c_str()))},
      monitor{is_true(sec[KEY_MONITOR])},
      logfile{sec[KEY_LOGFILE]},
      loglevel{sec[KEY_LOGLEVEL]},
      vjoy{sec}
{
    if (scale < 1) {
        scale = 1;
    }
}

std::string Config::resolve(const std::string& name, const std::string& path, const std::string& prefix,
    const std::string& ext)
{
    std::string fname{fs::search(name)};
    if (!fname.empty()) {
        /*
         * name is referencing an existing file.
         */
        return fname;
    }

    /*
     * Build the basename and search for the file in the specified path.
     */
    fname = prefix + name + ext;
    std::string fullpath{fs::search(fname, {path})};
    if (!fullpath.empty()) {
        return fullpath;
    }

    throw IOError{"File not found: name " + fname + ", path " + path};
}

std::string Config::to_string() const
{
    std::ostringstream os{};

    os << "  Title:              " << std::quoted(title)              << std::endl
       << "  ROMs path:          " << std::quoted(romdir)             << std::endl
       << "  Palette:            " << std::quoted(palette)            << std::endl
       << "  Keymaps:            " << std::quoted(keymaps)            << std::endl
       << "  Cartridge:          " << std::quoted(cartridge)          << std::endl
       << "  FPS:                " << fps                             << std::endl
       << "  Scale:              " << scale << "x"                    << std::endl
       << "  Scanlines effect:   " << scanlines                       << std::endl
       << "  Fullscreen:         " << (fullscreen ? "yes" : "no")     << std::endl
       << "  Smooth resize:      " << (sresize ? "yes" : "no")        << std::endl
       << "  Audio enabled:      " << (audio ? "yes" : "no")          << std::endl
       << "  Clock delay:        " << delay << "x"                    << std::endl
       << "  CPU Monitor:        " << (monitor ? "yes" : "no")        << std::endl
       << "  Log file:           " << std::quoted(logfile)            << std::endl
       << "  Log level:          " << loglevel                        << std::endl
       << "  Virtual Joystick:   " << (vjoy.enabled ? "yes" : "no")   << std::endl
       << "                up:   " << keyboard::to_string(vjoy.up)    << std::endl
       << "              down:   " << keyboard::to_string(vjoy.down)  << std::endl
       << "              left:   " << keyboard::to_string(vjoy.left)  << std::endl
       << "             right:   " << keyboard::to_string(vjoy.right) << std::endl
       << "              fire:   " << keyboard::to_string(vjoy.fire);

    return os.str();
}

}
}
