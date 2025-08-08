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
#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "fs.hpp"
#include "keyboard.hpp"
#include "logger.hpp"

#include "ui_config.hpp"

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

#ifndef D_SCREENSHOTDIR
#define D_SCREENSHOTDIR     "~/Desktop"
#endif

namespace caio {
namespace config {

constexpr static const char* SYSTEM_CONFDIR         = D_SYSCONFDIR;
constexpr static const char* HOME_CONFDIR           = D_HOMECONFDIR;
constexpr static const char* ROMDIR                 = D_ROMDIR;
constexpr static const char* PALETTEDIR             = D_PALETTEDIR;
constexpr static const char* KEYMAPSDIR             = D_KEYMAPSDIR;
constexpr static const char* SCREENSHOTDIR          = D_SCREENSHOTDIR;

constexpr static const char* PALETTEFILE_EXT        = ".plt";
constexpr static const char* KEYMAPSFILE_EXT        = ".kbd";

constexpr static const char* CONFIG_FILE            = "caio.conf";
constexpr static const char* KEY_CONFIG_FILE        = "conf";

/*
 * Generic configuration file section and default values.
 */
constexpr static const char* SEC_GENERIC            = "generic";

constexpr static const char* KEY_ROMDIR             = "romdir";
constexpr static const char* KEY_PALETTEDIR         = "palettedir";
constexpr static const char* KEY_KEYMAPSDIR         = "keymapsdir";
constexpr static const char* KEY_PALETTE            = "palette";
constexpr static const char* KEY_KEYMAPS            = "keymaps";
constexpr static const char* KEY_CARTRIDGE          = "cart";
constexpr static const char* KEY_FPS                = "fps";
constexpr static const char* KEY_SCALE              = "scale";
constexpr static const char* KEY_ASPECT             = "aspect";
constexpr static const char* KEY_SCANLINES          = "scanlines";
constexpr static const char* KEY_FULLSCREEN         = "fullscreen";
constexpr static const char* KEY_SRESIZE            = "sresize";
constexpr static const char* KEY_AUDIO              = "audio";
constexpr static const char* KEY_DELAY              = "delay";
constexpr static const char* KEY_MONITOR            = "monitor";
constexpr static const char* KEY_LOGFILE            = "logfile";
constexpr static const char* KEY_LOGLEVEL           = "loglevel";
constexpr static const char* KEY_KEYBOARD           = "keyboard";
constexpr static const char* KEY_VJOY               = "vjoy";
constexpr static const char* KEY_VJOY_UP            = "vjoy-up";
constexpr static const char* KEY_VJOY_DOWN          = "vjoy-down";
constexpr static const char* KEY_VJOY_LEFT          = "vjoy-left";
constexpr static const char* KEY_VJOY_RIGHT         = "vjoy-right";
constexpr static const char* KEY_VJOY_FIRE          = "vjoy-fire";
constexpr static const char* KEY_VJOY_A             = "vjoy-a";
constexpr static const char* KEY_VJOY_B             = "vjoy-b";
constexpr static const char* KEY_VJOY_X             = "vjoy-x";
constexpr static const char* KEY_VJOY_Y             = "vjoy-y";
constexpr static const char* KEY_VJOY_BACK          = "vjoy-back";
constexpr static const char* KEY_VJOY_GUIDE         = "vjoy-guide";
constexpr static const char* KEY_VJOY_START         = "vjoy-start";
constexpr static const char* KEY_SCREENSHOTDIR      = "screenshotdir";
constexpr static const char* KEY_STATUSBAR          = "statusbar";

constexpr static const char* DEFAULT_ROMDIR         = ROMDIR;
constexpr static const char* DEFAULT_PALETTEDIR     = PALETTEDIR;
constexpr static const char* DEFAULT_KEYMAPSDIR     = KEYMAPSDIR;
constexpr static const char* DEFAULT_PALETTE        = "";
constexpr static const char* DEFAULT_KEYMAPS        = "";
constexpr static const char* DEFAULT_CARTRIDGE      = "";
constexpr static const char* DEFAULT_FPS            = "50";
constexpr static const char* DEFAULT_SCALE          = "1";
constexpr static const char* DEFAULT_ASPECT         = "system";
constexpr static const char* DEFAULT_SCANLINES      = "n";
constexpr static const char* DEFAULT_FULLSCREEN     = "no";
constexpr static const char* DEFAULT_SRESIZE        = "yes";
constexpr static const char* DEFAULT_AUDIO          = "yes";
constexpr static const char* DEFAULT_DELAY          = "1";
constexpr static const char* DEFAULT_MONITOR        = "no";
constexpr static const char* DEFAULT_LOGFILE        = Logger::DEFAULT_LOGFILE;
constexpr static const char* DEFAULT_LOGLEVEL       = Logger::DEFAULT_LOGLEVEL;
constexpr static const char* DEFAULT_KEYBOARD       = "yes";
constexpr static const char* DEFAULT_VJOY           = "no";
constexpr static const char* DEFAULT_VJOY_UP        = "KEY_NUMPAD_5";
constexpr static const char* DEFAULT_VJOY_DOWN      = "KEY_NUMPAD_2";
constexpr static const char* DEFAULT_VJOY_LEFT      = "KEY_NUMPAD_1";
constexpr static const char* DEFAULT_VJOY_RIGHT     = "KEY_NUMPAD_3";
constexpr static const char* DEFAULT_VJOY_FIRE      = "KEY_LEFT_ALT";
constexpr static const char* DEFAULT_VJOY_A         = "";
constexpr static const char* DEFAULT_VJOY_B         = "";
constexpr static const char* DEFAULT_VJOY_X         = "";
constexpr static const char* DEFAULT_VJOY_Y         = "";
constexpr static const char* DEFAULT_VJOY_BACK      = "";
constexpr static const char* DEFAULT_VJOY_GUIDE     = "";
constexpr static const char* DEFAULT_VJOY_START     = "";
constexpr static const char* DEFAULT_SCREENSHOTDIR  = SCREENSHOTDIR;
constexpr static const char* DEFAULT_STATUSBAR      = "south";

/**
 * Configuration file section.
 * A section contains key-value pairs.
 */
using Section = std::unordered_map<std::string, std::string>;

/**
 * Configuration file.
 * A Configuration file is conformed by one or more sections.
 * Section names are case insensitive. Key names are case sensitive.
 * File format:
 * <pre>
 *      [section_name_1]
 *      key1 = value1
 *      key2 = value2
 *
 *      # comment
 *      [section_name_2]
 *      key1 = value1
 *      key2 = value2
 * </pre>
 * @see Section
 */
class Confile {
public:
    /**
     * Initialise this configuration file.
     * @param fname Name of the configuration file to read or an empty string.
     * @exception ConfigError
     * @exception IOError
     * @see load(const fs::Path&)
     */
    Confile(const fs::Path& fname = {});

    virtual ~Confile() = default;

    /**
     * Load a configuration file.
     * This configuration is merged with the new data (existing sections are extended
     * with new values and existing values are replaced with new ones).
     * @param fname Name of the configuration file to load or an empty string.
     * @exception ConfigError
     * @exception IOError
     */
    void load(const fs::Path& fname);

    /**
     * Return a configuration section.
     * If the specified section does not exit an empty one is created.
     * @param sname Name of the section (case insensitive).
     * @return The requested section.
     */
    Section& operator[](std::string_view sname);

    /**
     * Extract a section.
     * @param sname Name of the section to extract (case insensitive).
     * @return The extracted section or an empty one if it does not exist.
     */
    Section extract(std::string_view sname);

    /**
     * Find a section.
     * @param sname Name of the section (case insensitive).
     * @return An iterator to the section; or end() if the section does not exist.
     * @see end()
     */
    std::unordered_map<std::string, Section>::const_iterator find(std::string_view sname) const;

    /**
     * Return an iterator following the last section of this configuration file.
     * @return An iterator following the last section.
     */
    std::unordered_map<std::string, Section>::const_iterator end() const;

private:
    std::unordered_map<std::string, Section> _sections;
};

/**
 * Command line option argument specification.
 */
enum class Arg {
    None,
    Required,
    Optional
};

/**
 * Command line option.
 */
struct Option {
    using SetCb = std::function<bool(class Confile&, const Option&, std::string_view)>;

    std::string name{};     /**< Command line option without the "--" prefix.               */
    std::string sname{};    /**< Section name.                                              */
    std::string key{};      /**< Key name.                                                  */
    std::string dvalue{};   /**< Default value.                                             */
    Arg         type{};     /**< Argument requisites.                                       */
    SetCb       fn{};       /**< Value setter.                                              */
    std::string optval{};   /**< Value to set when an optional argument is not provided.    */
};

bool set_value(Confile&, const Option&, std::string_view);
bool set_bool(Confile&, const Option&, std::string_view);
bool set_true(Confile&, const Option&, std::string_view);

/**
 * Detect a "yes" string.
 * @return true if the specified string is equal to "yes", "ye", or "y"; false otherwise.
 */
bool is_true(std::string_view);

/**
 * Detect a "no" string.
 * @return true if the specified string is equal to "no" or "n"; false otherwise.
 */
bool is_false(std::string_view);

/**
 * Command line parser.
 */
class Cmdline {
public:
    Cmdline();

    virtual ~Cmdline();

    std::string_view progname() const
    {
        return _progname;
    }

    /**
     * Build a configuration file with default values.
     * @return A configuration file with the default values.
     * @see Confile
     */
    Confile defaults();

    /**
     * Parse the command line.
     * @param argc argc as received by the main() function;
     * @param argv argv as received by the main() function.
     * @return A pair containing: configuration file with all the parsed values,
     * and a string containing the name of a program to launch (can be empty).
     * @exception InvalidArgument if an unknown command line option is detected or
     * a mandatory argument is missing.
     * @see Confile
     * @see options()
     * @see usage()
     */
    std::pair<Confile, std::string> parse(int argc, const char** argv);

    /**
     * Return a vector with the command line options.
     * This method must be re-implemented by platforms in order to add new command line options.
     * @return A vector with all the command line options.
     * @see Option
     */
    virtual std::vector<Option> options() const;

    /**
     * Return the command line usage string.
     * This method must be re-implemented by platforms in order to add new usgae information.
     * @return A string with usage information.
     */
    virtual std::string usage() const;

    /**
     * Return the platform's section name.
     * This method must be implemented by the specific emulated platform.
     * @return The platform's section name.
     */
    virtual std::string sname() const = 0;

private:
    std::string _progname{""};
};

/**
 * Configuration parser.
 * Parse the command line options and an eventual configuartion file
 * and generate a section containing all the configuration values.
 * The section is built using the following precedence:
 *   1. Command line options
 *   2. Configuration file values
 *   3. Default values
 * @param argc        argc as received by main();
 * @param argv        argv as received by main();
 * @param cmdline     Command line parser;
 * @param search_conf If the configuration file is not defined search it in standard directories (default is true)
 * @return A pair containing: A section with all the configuration values,
 * and a string containing the name of a program to launch (can be empty).
 */
std::pair<Section, std::string> parse(int argc, const char** argv, Cmdline& cmdline, bool search_conf = true);

/**
 * Save a configuration section to file.
 * If the file exists it is overwritten.
 * @param fname File name;
 * @param sname Section name;
 * @param sec   Configuration section.
 * @exception IOError
 */
void save(const fs::Path& fname, std::string_view sname, const Section& sec);

/**
 * Virtual joystick configuration.
 */
struct VJoyConfig : VJoyKeys {
    bool enabled{};

    VJoyConfig() = default;

    /**
     * Initialise this virtual joystick configuration structure.
     * @param sec Section containing configuration key-value pairs.
     * @exception InvalidArgument
     */
    VJoyConfig(Section& sec);

    bool operator==(const VJoyConfig& other) const;

    bool operator!=(const VJoyConfig& other) const
    {
        return !operator==(other);
    }

    /**
     * Fill a section with the configuration parameters.
     * @param sec Destination section.
     */
    void to_section(Section& sec) const;
};

/**
 * Generic configuration.
 */
struct Config {
    using SLEffect = ui::SLEffect;
    using AspectRatio = ui::AspectRatio;

    std::string title{};
    std::string romdir{};
    std::string palette{};
    std::string keymaps{};
    std::string cartridge{};
    unsigned    fps{};
    unsigned    scale{};
    AspectRatio aspect{};
    SLEffect    scanlines{};
    bool        fullscreen{};
    bool        sresize{};
    bool        audio{};
    float       delay{};
    bool        monitor{};
    std::string logfile{};
    std::string loglevel{};
    bool        keyboard{};
    VJoyConfig  vjoy{};
    std::string screenshotdir{};
    std::string statusbar{};

    /**
     * Initialise this configuration structure.
     * Configuration values are taken from the specified section.
     * @param sec    Section containing configuration key-value pairs;
     * @param prefix Platform prefix (see resolve()).
     * @exception InvalidArgument
     * @exception IOError
     * @see resolve(std::string_view, std::string_view, std::string_view, std::string_view)
     */
    Config(Section& sec, std::string_view prefix);

    virtual ~Config();

    bool operator==(const Config& other) const;

    bool operator!=(const Config& other) const
    {
        return !operator==(other);
    }

    /**
     * Fill a section with the configuration parameters.
     * @param sec Destination section.
     */
    virtual void to_section(Section& sec) const;

    /**
     * Return a human readable string representation of this configuration.
     * @return A string representation of this configuration.
     */
    virtual std::string to_string() const;

private:
    /**
     * Resolve the fullpath for a palette or a keymaps file.
     * @param name   Name of the palette or keymaps or a full path;
     * @param path   Directory where the file is intended to be;
     * @param prefix Platform prefix;
     * @param ext    File extension.
     * @return The resolved fullpath if the file is found (name or path + "/" + prefix + name + ext);
     * otherwise the parameter name is returned.
     */
    fs::Path resolve(const fs::Path& name, const fs::Path& path, const fs::Path& prefix, const fs::Path& ext);
};

/**
 * Persistent data directory.
 * The persistent data directory is where caio stores information
 * than can be read/written during emulation by devices, such as
 * EEPROM data used by various cartridges (high scores, player names, etc.).
 * @return A path to the persistent data directory.
 */
fs::Path storage_path();

}
}
