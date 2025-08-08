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
#include <fstream>

#include "fs.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "ui_config.hpp"

namespace caio {
namespace config {

std::pair<Section, std::string> parse(int argc, const char** argv, Cmdline& cmdline, bool search_conf)
{
    Confile def{cmdline.defaults()};
    auto [cline, pname] = cmdline.parse(argc, argv);
    Confile cfile{};

    const auto& gsec = cline[SEC_GENERIC];
    const auto& it = gsec.find(KEY_CONFIG_FILE);
    std::string fname{};
    if (it != gsec.end()) {
        /*
         * A configuration file is specified in the command line.
         */
        fname = it->second;
        log.debug("Configuration file: {}\n", fname);
        cfile.load(fname);
    } else if (search_conf) {
        /*
         * Searh for the configuration file in standard directories.
         */
        fname = fs::search(CONFIG_FILE, {HOME_CONFDIR, SYSTEM_CONFDIR});
        if (fname.empty()) {
            log.debug("Configuration file not found. Using default values\n");
        } else {
            log.debug("Configuration file found: {}\n", fname);
            cfile.load(fname);
        }
    } else {
        log.debug("Configuration file not defined. Using default values\n");
    }

    const auto& sname = cmdline.sname();

    Section merged{};
    merged.merge(cline.extract(sname));
    merged.merge(cline.extract(SEC_GENERIC));

    merged.merge(cfile.extract(sname));
    merged.merge(cfile.extract(SEC_GENERIC));

    merged.merge(def.extract(sname));
    merged.merge(def.extract(SEC_GENERIC));

    return {merged, pname};
}

void save(const fs::Path& fname, std::string_view sname, const Section& sec)
{
    std::ofstream os{fname, std::ios_base::out | std::ios_base::trunc};
    if (!os) {
        throw IOError{"Can't open file: {}: {}", fname.string(), Error::to_string(errno)};
    }

    try {
        os << "[" << sname << "]\n";
        std::for_each(sec.begin(), sec.end(), [&os](const std::pair<std::string, std::string>& kv) {
            const auto& [key, value] = kv;
            os << key << " =";
            if (!value.empty()) {
                os << " " << value;
            }
            os << "\n";
        });
    } catch (const std::exception& err) {
        throw IOError{"Can't write file: {}: {}", fname.string(), err.what()};
    }
}

VJoyConfig::VJoyConfig(Section& sec)
    : VJoyKeys{
        .up     = keyboard::to_key(sec[KEY_VJOY_UP]),
        .down   = keyboard::to_key(sec[KEY_VJOY_DOWN]),
        .left   = keyboard::to_key(sec[KEY_VJOY_LEFT]),
        .right  = keyboard::to_key(sec[KEY_VJOY_RIGHT]),
        .fire   = keyboard::to_key(sec[KEY_VJOY_FIRE]),
        .a      = keyboard::to_key(sec[KEY_VJOY_A]),
        .b      = keyboard::to_key(sec[KEY_VJOY_B]),
        .x      = keyboard::to_key(sec[KEY_VJOY_X]),
        .y      = keyboard::to_key(sec[KEY_VJOY_Y]),
        .back   = keyboard::to_key(sec[KEY_VJOY_BACK]),
        .guide  = keyboard::to_key(sec[KEY_VJOY_GUIDE]),
        .start  = keyboard::to_key(sec[KEY_VJOY_START])},
      enabled{is_true(sec[KEY_VJOY])}
{
    if (up == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick up key: {}", sec[KEY_VJOY_UP]};
    }

    if (down == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick down key: {}", sec[KEY_VJOY_DOWN]};
    }

    if (left == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick left key: {}", sec[KEY_VJOY_LEFT]};
    }

    if (right == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick right key: {}", sec[KEY_VJOY_RIGHT]};
    }

    if (fire == keyboard::KEY_NONE) {
        throw InvalidArgument{"Invalid virtual joystick fire key: {}", sec[KEY_VJOY_FIRE]};
    }

    if (a == keyboard::KEY_NONE && !sec[KEY_VJOY_A].empty()) {
        throw InvalidArgument{"Invalid virtual joystick A key: {}", sec[KEY_VJOY_A]};
    }

    if (b == keyboard::KEY_NONE && !sec[KEY_VJOY_B].empty()) {
        throw InvalidArgument{"Invalid virtual joystick B key: {}", sec[KEY_VJOY_B]};
    }

    if (x == keyboard::KEY_NONE && !sec[KEY_VJOY_X].empty()) {
        throw InvalidArgument{"Invalid virtual joystick X key: {}", sec[KEY_VJOY_X]};
    }

    if (y == keyboard::KEY_NONE && !sec[KEY_VJOY_Y].empty()) {
        throw InvalidArgument{"Invalid virtual joystick Y key: {}", sec[KEY_VJOY_Y]};
    }

    if (back == keyboard::KEY_NONE && !sec[KEY_VJOY_BACK].empty()) {
        throw InvalidArgument{"Invalid virtual joystick BACK key: {}", sec[KEY_VJOY_BACK]};
    }

    if (guide == keyboard::KEY_NONE && !sec[KEY_VJOY_GUIDE].empty()) {
        throw InvalidArgument{"Invalid virtual joystick GUIDE key: {}", sec[KEY_VJOY_GUIDE]};
    }

    if (start == keyboard::KEY_NONE && !sec[KEY_VJOY_START].empty()) {
        throw InvalidArgument{"Invalid virtual joystick START key: {}", sec[KEY_VJOY_START]};
    }
}

bool VJoyConfig::operator==(const VJoyConfig& other) const
{
    return enabled == other.enabled &&
           up == other.up &&
           down == other.down &&
           left == other.left &&
           right == other.right &&
           fire == other.fire &&
           a == other.a &&
           b == other.b &&
           x == other.x &&
           y == other.y &&
           back == other.back &&
           guide == other.guide &&
           start == other.start;
}

void VJoyConfig::to_section(Section& sec) const
{
    sec[KEY_VJOY] = (enabled ? "yes" : "no");
    sec[KEY_VJOY_UP] = keyboard::to_string(up);
    sec[KEY_VJOY_DOWN] = keyboard::to_string(down);
    sec[KEY_VJOY_LEFT] = keyboard::to_string(left);
    sec[KEY_VJOY_RIGHT] = keyboard::to_string(right);
    sec[KEY_VJOY_FIRE] = keyboard::to_string(fire);
    sec[KEY_VJOY_A] = keyboard::to_string(a);
    sec[KEY_VJOY_B] = keyboard::to_string(b);
    sec[KEY_VJOY_X] = keyboard::to_string(x);
    sec[KEY_VJOY_Y] = keyboard::to_string(y);
    sec[KEY_VJOY_BACK] = keyboard::to_string(back);
    sec[KEY_VJOY_GUIDE] = keyboard::to_string(guide);
    sec[KEY_VJOY_START] = keyboard::to_string(start);
}

Config::Config(Section& sec, std::string_view prefix)
    : title{"caio"},
      romdir{sec[KEY_ROMDIR]},
      palette{(sec[KEY_PALETTE].empty() ? "" : resolve(sec[KEY_PALETTE], sec[KEY_PALETTEDIR], prefix, PALETTEFILE_EXT))},
      keymaps{(sec[KEY_KEYMAPS].empty() ? "" : resolve(sec[KEY_KEYMAPS], sec[KEY_KEYMAPSDIR], prefix, KEYMAPSFILE_EXT))},
      cartridge{sec[KEY_CARTRIDGE]},
      fps{static_cast<unsigned>(std::atoi(sec[KEY_FPS].c_str()))},
      scale{static_cast<unsigned>(std::atoi(sec[KEY_SCALE].c_str()))},
      aspect{ui::to_aspect_ratio(sec[KEY_ASPECT])},
      scanlines{ui::to_sleffect(sec[KEY_SCANLINES])},
      fullscreen{is_true(sec[KEY_FULLSCREEN])},
      sresize{is_true(sec[KEY_SRESIZE])},
      audio{is_true(sec[KEY_AUDIO])},
      delay{static_cast<float>(std::atof(sec[KEY_DELAY].c_str()))},
      monitor{is_true(sec[KEY_MONITOR])},
      logfile{sec[KEY_LOGFILE]},
      loglevel{sec[KEY_LOGLEVEL]},
      keyboard{is_true(sec[KEY_KEYBOARD])},
      vjoy{sec},
      screenshotdir{fs::fix_home(sec[KEY_SCREENSHOTDIR])},
      statusbar{sec[KEY_STATUSBAR]}
{
    if (scale < 1) {
        scale = 1;
    }

    if (!fs::exists(screenshotdir)) {
        /*
         * Default to $HOME when the screenshot directory is invalid.
         */
        screenshotdir = fs::home();
    }
}

Config::~Config()
{
}

bool Config::operator==(const Config& other) const
{
    return (title == other.title &&
        romdir == other.romdir &&
        palette == other.palette &&
        keymaps == other.keymaps &&
        cartridge == other.cartridge &&
        fps == other.fps &&
        scale == other.scale &&
        aspect == other.aspect &&
        scanlines == other.scanlines &&
        fullscreen == other.fullscreen &&
        sresize == other.sresize &&
        audio == other.audio &&
        delay == other.delay &&
        monitor == other.monitor &&
        logfile == other.logfile &&
        loglevel == other.loglevel &&
        keyboard == other.keyboard &&
        vjoy == other.vjoy &&
        screenshotdir == other.screenshotdir &&
        statusbar == other.statusbar);
}

void Config::to_section(Section& sec) const
{
    sec[KEY_ROMDIR]         = romdir;
    sec[KEY_PALETTE]        = palette;
    sec[KEY_KEYMAPS]        = keymaps;
    sec[KEY_CARTRIDGE]      = cartridge;
    sec[KEY_FPS]            = std::format("{}", fps);
    sec[KEY_SCALE]          = std::format("{}", scale);
    sec[KEY_ASPECT]         = ui::to_string(aspect);
    sec[KEY_SCANLINES]      = ui::to_string(scanlines);
    sec[KEY_FULLSCREEN]     = (fullscreen ? "yes" : "no");
    sec[KEY_SRESIZE]        = (sresize ? "yes" : "no");
    sec[KEY_AUDIO]          = (audio ? "yes" : "no");
    sec[KEY_DELAY]          = std::format("{:.1f}", delay);
    sec[KEY_MONITOR]        = (monitor ? "yes" : "no");
    sec[KEY_LOGFILE]        = logfile;
    sec[KEY_LOGLEVEL]       = loglevel;
    sec[KEY_KEYBOARD]       = (keyboard ? "yes" : "no");
    sec[KEY_SCREENSHOTDIR]  = screenshotdir;
    sec[KEY_STATUSBAR]      = statusbar;

    vjoy.to_section(sec);
}

fs::Path Config::resolve(const fs::Path& name, const fs::Path& path, const fs::Path& prefix, const fs::Path& ext)
{
    fs::Path fname = fs::search(name);
    if (!fname.empty()) {
        /*
         * name is referencing an existing file.
         */
        return fname;
    }

    /*
     * Build the basename and search for the file in the specified path.
     */
    fname = std::format("{}{}{}", prefix.string(), name.string(), ext.string());
    fs::Path fullpath = fs::search(fname, {path});
    if (!fullpath.empty()) {
        return fullpath;
    }

    return name;
}

std::string Config::to_string() const
{
    return std::format(
        "  Title:              \"{}\"\n"
        "  ROMs path:          \"{}\"\n"
        "  Palette:            \"{}\"\n"
        "  Keymaps:            \"{}\"\n"
        "  Cartridge:          \"{}\"\n"
        "  FPS:                {}\n"
        "  Scale:              {}x\n"
        "  Aspect Ratio:       {}\n"
        "  Scanlines effect:   {}\n"
        "  Fullscreen:         {}\n"
        "  Smooth resize:      {}\n"
        "  Audio enabled:      {}\n"
        "  Clock delay:        {:1.1f}x\n"
        "  CPU Monitor:        {}\n"
        "  Log file:           \"{}\"\n"
        "  Log level:          {}\n"
        "  Keyboard enabled:   {}\n"
        "  Virtual Joystick:   {}\n"
        "                up:   {}\n"
        "              down:   {}\n"
        "              left:   {}\n"
        "             right:   {}\n"
        "              fire:   {}\n"
        "                 A:   {}\n"
        "                 B:   {}\n"
        "                 X:   {}\n"
        "                 Y:   {}\n"
        "              back:   {}\n"
        "             guide:   {}\n"
        "             start:   {}\n"
        "  Screenshots path:   \"{}\"\n"
        "  Status bar:         \"{}\"",
        title, romdir, palette, keymaps, cartridge, fps, scale,
        ui::to_string(aspect),
        ui::to_string(scanlines),
        (fullscreen ? "yes" : "no"),
        (sresize ? "yes" : "no"),
        (audio ? "yes" : "no"),
        delay,
        (monitor ? "yes" : "no"),
        logfile,
        loglevel,
        (keyboard ? "yes" : "no"),
        (vjoy.enabled ? "yes" : "no"),
        keyboard::to_string(vjoy.up),
        keyboard::to_string(vjoy.down),
        keyboard::to_string(vjoy.left),
        keyboard::to_string(vjoy.right),
        keyboard::to_string(vjoy.fire),
        keyboard::to_string(vjoy.a),
        keyboard::to_string(vjoy.b),
        keyboard::to_string(vjoy.x),
        keyboard::to_string(vjoy.y),
        keyboard::to_string(vjoy.back),
        keyboard::to_string(vjoy.guide),
        keyboard::to_string(vjoy.start),
        screenshotdir,
        statusbar);
}

fs::Path storage_path()
{
    static fs::Path dir{};
    if (dir.empty()) {
        dir = fs::fix_home(D_HOMECONFDIR);
    }
    return dir;
}

}
}
