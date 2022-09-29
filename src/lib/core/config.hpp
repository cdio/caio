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
#pragma once

#include "confile.hpp"
#include "logger.hpp"

#ifndef D_PREFIX
#define D_PREFIX            "/opt/caio"
#endif

#ifndef D_SYSCONFDIR
#define D_SYSCONFDIR        D_PREFIX "/etc/caio"
#endif

#ifndef D_HOMECONFDIR
#define D_HOMECONFDIR       "~/.config/caio"
#endif

#ifndef D_DATADIR
#define D_DATADIR           D_PREFIX "/share/caio"
#endif

#ifndef D_ROMDIR
#define D_ROMDIR            D_DATADIR "/roms"
#endif

#ifndef D_PALETTEDIR
#define D_PALETTEDIR        D_DATADIR "/palette"
#endif

#ifndef D_KEYMAPSDIR
#define D_KEYMAPSDIR        D_DATADIR "/keymaps"
#endif


namespace caio {

/**
 * caio base configuation file.
 */
class CaioConfile : public Confile {
public:
    constexpr static const char *SYSTEM_CONFDIR         = D_SYSCONFDIR;
    constexpr static const char *HOME_CONFDIR           = D_HOMECONFDIR;
    constexpr static const char *CWD                    = ".";
    constexpr static const char *ROMDIR                 = D_ROMDIR;
    constexpr static const char *PALETTEDIR             = D_PALETTEDIR;
    constexpr static const char *KEYMAPSDIR             = D_KEYMAPSDIR;

    constexpr static const char *CAIO_CONFIG_FILE       = "caio.conf";

    constexpr static const char *CAIO_CONFIG_SECTION    = "caio";

    constexpr static const char *ROMDIR_CONFIG_KEY      = "romdir";
    constexpr static const char *PALETTEDIR_CONFIG_KEY  = "palettedir";
    constexpr static const char *PALETTE_CONFIG_KEY     = "palette";
    constexpr static const char *KEYMAPSDIR_CONFIG_KEY  = "keymapsdir";
    constexpr static const char *KEYMAPS_CONFIG_KEY     = "keymaps";
    constexpr static const char *CARTDIR_CONFIG_KEY     = "cartdir";
    constexpr static const char *FPS_CONFIG_KEY         = "fps";
    constexpr static const char *SCALE_CONFIG_KEY       = "scale";
    constexpr static const char *SCANLINES_CONFIG_KEY   = "scanlines";
    constexpr static const char *FULLSCREEN_CONFIG_KEY  = "fullscreen";
    constexpr static const char *SRESIZE_CONFIG_KEY     = "sresize";
    constexpr static const char *AUDIO_CONFIG_KEY       = "audio";
    constexpr static const char *DELAY_CONFIG_KEY       = "delay";
    constexpr static const char *MONITOR_CONFIG_KEY     = "monitor";
    constexpr static const char *LOGFILE_CONFIG_KEY     = "logfile";
    constexpr static const char *LOGLEVEL_CONFIG_KEY    = "loglevel";

    CaioConfile() {
    }

    virtual ~CaioConfile() {
    }

    /**
     * @see Confile::parse()
     */
    void parse(const std::string &fname = CAIO_CONFIG_FILE,
        const std::initializer_list<std::string> &spaths = {HOME_CONFDIR, SYSTEM_CONFDIR}) override {
        Confile::parse(fname, spaths);
    }
};


/**
 * Base configuration.
 */
struct Config {
    constexpr static const unsigned DEFAULT_FPS              = 50;
    constexpr static const float    DEFAULT_SCALE            = 1.0f;
    constexpr static const char *   DEFAULT_SCANLINES_EFFECT = "n";
    constexpr static const bool     DEFAULT_FULLSCREEN       = false;
    constexpr static const bool     DEFAULT_SMOOTH_RESIZE    = true;
    constexpr static const bool     DEFAULT_AUDIO_ENABLED    = true;
    constexpr static const float    DEFAULT_DELAY_FACTOR     = 1.0f;
    constexpr static const bool     DEFAULT_MONITOR_ACTIVE   = false;
    constexpr static const char *   DEFAULT_LOGFILE          = Logger::DEFAULT_LOGFILE;
    constexpr static const char *   DEFAULT_LOGLEVEL         = Logger::DEFAULT_LOGLEVEL;

    constexpr static const char *   PALETTEFILE_SUFFIX       = ".plt";
    constexpr static const char *   KEYMAPSFILE_SUFFIX       = ".kbd";

    std::string title{};
    std::string romdir{};
    std::string cartdir{};
    std::string palettedir{};
    std::string palettefile{};
    std::string keymapsdir{};
    std::string keymapsfile{};
    unsigned    fps{DEFAULT_FPS};
    float       scale{DEFAULT_SCALE};
    std::string scanlines{DEFAULT_SCANLINES_EFFECT};
    bool        fullscreen{DEFAULT_FULLSCREEN};
    bool        smooth_resize{DEFAULT_SMOOTH_RESIZE};
    bool        audio_enabled{DEFAULT_AUDIO_ENABLED};
    float       delay{DEFAULT_DELAY_FACTOR};
    bool        monitor{DEFAULT_MONITOR_ACTIVE};
    std::string logfile{DEFAULT_LOGFILE};
    std::string loglevel{DEFAULT_LOGLEVEL};

    Config() {
    }

    virtual ~Config() {
    }

    /**
     * Set this config with values from a configuration file.
     * Only existing values are set, values not defined in the configuration file are left untouched.
     * @param conf Configuration file to load values from.
     * @return This configuration.
     */
    Config &operator=(const Confile &conf);

    /**
     * Get the palette file name given the palette naem.
     * @param palette Palette name.
     * @return The palette file name.
     */
    virtual std::string palette_file(const std::string &palette) const;

    /**
     * Get the keymaps file name given the country code.
     * @param cc Country code.
     * @return The keymaps file name.
     */
    virtual std::string keymaps_file(const std::string &cc) const;

    /**
     * @return A human readable string representation of this configuration.
     */
    virtual std::string to_string() const;
};

}
