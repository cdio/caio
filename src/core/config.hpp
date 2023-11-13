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
#include <map>
#include <string>

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
namespace config {

constexpr static const char* SYSTEM_CONFDIR     = D_SYSCONFDIR;
constexpr static const char* HOME_CONFDIR       = D_HOMECONFDIR;
constexpr static const char* CWD                = ".";
constexpr static const char* ROMDIR             = D_ROMDIR;
constexpr static const char* PALETTEDIR         = D_PALETTEDIR;
constexpr static const char* KEYMAPSDIR         = D_KEYMAPSDIR;

constexpr static const char* PALETTEFILE_EXT    = ".plt";
constexpr static const char* KEYMAPSFILE_EXT    = ".kbd";

constexpr static const char* CONFIG_FILE        = "caio.conf";
constexpr static const char* KEY_CONFIG_FILE    = "conf";

/*
 * Generic configuration file section and default values.
 */
constexpr static const char* SEC_GENERIC        = "generic";

constexpr static const char* KEY_ROMDIR         = "romdir";
constexpr static const char* KEY_PALETTEDIR     = "palettedir";
constexpr static const char* KEY_KEYMAPSDIR     = "keymapsdir";
constexpr static const char* KEY_PALETTE        = "palette";
constexpr static const char* KEY_KEYMAPS        = "keymaps";
constexpr static const char* KEY_CARTRIDGE      = "cart";
constexpr static const char* KEY_FPS            = "fps";
constexpr static const char* KEY_SCALE          = "scale";
constexpr static const char* KEY_SCANLINES      = "scanlines";
constexpr static const char* KEY_FULLSCREEN     = "fullscreen";
constexpr static const char* KEY_SRESIZE        = "sresize";
constexpr static const char* KEY_AUDIO          = "audio";
constexpr static const char* KEY_DELAY          = "delay";
constexpr static const char* KEY_MONITOR        = "monitor";
constexpr static const char* KEY_LOGFILE        = "logfile";
constexpr static const char* KEY_LOGLEVEL       = "loglevel";

constexpr static const char* DEFAULT_ROMDIR     = ROMDIR;
constexpr static const char* DEFAULT_PALETTEDIR = PALETTEDIR;
constexpr static const char* DEFAULT_KEYMAPSDIR = KEYMAPSDIR;
constexpr static const char* DEFAULT_PALETTE    = "";
constexpr static const char* DEFAULT_KEYMAPS    = "";
constexpr static const char* DEFAULT_CARTRIDGE  = "";
constexpr static const char* DEFAULT_FPS        = "50";
constexpr static const char* DEFAULT_SCALE      = "1";
constexpr static const char* DEFAULT_SCANLINES  = "n";
constexpr static const char* DEFAULT_FULLSCREEN = "no";
constexpr static const char* DEFAULT_SRESIZE    = "yes";
constexpr static const char* DEFAULT_AUDIO      = "yes";
constexpr static const char* DEFAULT_DELAY      = "1";
constexpr static const char* DEFAULT_MONITOR    = "no";
constexpr static const char* DEFAULT_LOGFILE    = Logger::DEFAULT_LOGFILE;
constexpr static const char* DEFAULT_LOGLEVEL   = Logger::DEFAULT_LOGLEVEL;

/**
 * Configuration file section.
 * A section contains key-value pairs.
 */
using Section = std::map<std::string, std::string>;

/**
 * Configuration file.
 * A Configuration file is conformed by one or more sections.
 * Section names are case insensitive. Key names are case sensitive.
 * <pre>
 * File format:
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
     * @param fname Name of the configuration file to read or empty string.
     * @exception ConfigError
     * @exception IOError
     * @see load(const std::string&)
     */
    Confile(const std::string& fname = {});

    virtual ~Confile() {
    }

    /**
     * Load a configuration file.
     * This configuration is overrided with the new data (existing sections are extended
     * with new values and existing values are replaced with new ones).
     * @param fname Name of the configuration file to load or empty string.
     * @exception ConfigError
     * @exception IOError
     */
    void load(const std::string& fname);

    /**
     * Retrieve a configuration section.
     * If the specified section does not exit an empty one is created.
     * @param sname Name of the section (case insensitive).
     * @return The requested section.
     */
    Section& operator[](const std::string& sname);

    /**
     * Extract a section.
     * @param sname Name of the section to extract (case insensitive).
     * @return The extracted section or an empty one if it does not exist.
     */
    Section extract(const std::string& sname);

    /**
     * Find a section.
     * @param sname Name of the section (case insensitive).
     * @return An iterator to the section; or end() if the section does not exist.
     */
    std::map<std::string, Section>::const_iterator find(const std::string& sname) const;

    /**
     * @return An iterator to the last section plus one.
     */
    std::map<std::string, Section>::const_iterator end() const;

private:
    std::map<std::string, Section> _sections;
};

/**
 * Command line option argument specification.
 */
enum class Arg {
    None,
    Required
};

/**
 * Command line option.
 */
struct Option {
    using set_cb = std::function<bool(class Confile&, const Option&, const std::string&)>;

    std::string name{};     /* Command line option without the "--" prefix  */
    std::string sname{};    /* Section name                                 */
    std::string key{};      /* Key name                                     */
    std::string dvalue{};   /* Default value                                */
    Arg         type{};     /* Argument requisites                          */
    set_cb      fn{};       /* Value setter                                 */
};

bool set_value(Confile&, const Option&, const std::string&);
bool set_bool(Confile&, const Option&, const std::string&);
bool set_true(Confile&, const Option&, const std::string&);

/**
 * @return true if the specified string is equal to "yes", "ye", or "y"; false otherwise.
 */
bool is_true(const std::string&);

/**
 * @return true if the specified string is equal to "no" or "n"; false otherwise.
 */
bool is_false(const std::string&);

/**
 * Command line parser.
 */
class Cmdline {
public:
    /**
     * Setup this commmand line parser.
     */
    Cmdline() {
    }

    virtual ~Cmdline() {
    }

    const std::string progname() const {
        return _progname;
    }

    /**
     * @return Default configuration values.
     */
    Confile defaults();

    /**
     * Parse the command line.
     * @param argc argc as received by the main() function;
     * @param argv argv as received by the main() function.
     * @return A configuration file with all the parsed values.
     * @exception InvalidArgument if an unknown command line option is detected or
     * a required argument is missing.
     * @see Confile
     * @see options()
     * @see usage()
     */
    Confile parse(int argc, const char** argv);

    /**
     * @return A vector with all the command line options.
     */
    virtual std::vector<Option> options() const;

    /**
     * @return A string with usage information.
     */
    virtual std::string usage() const;

    /**
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
 * The section is build using the following precedence:
 *   1. Command line options
 *   2. Configuration file values
 *   3. Default values
 * @param argc    argc as received by main();
 * @param argv    argv as received by main();
 * @param cmdline Command line parser.
 * @return A section containing all the configuration values.
 */
Section parse(int argc, const char** argv, Cmdline& cmdline);

/**
 * Generic configuration.
 */
struct Config {
    std::string title{};
    std::string romdir{};
    std::string palette{};
    std::string keymaps{};
    std::string cartridge{};
    unsigned    fps{};
    unsigned    scale{};
    std::string scanlines{};
    bool        fullscreen{};
    bool        sresize{};
    bool        audio{};
    float       delay{};
    bool        monitor{};
    std::string logfile{};
    std::string loglevel{};

    /**
     * Initialise this configuration structure.
     * Configuration values are taken from the specified section.
     * @param sec    Section containing configuration key-value pairs;
     * @param prefix Platform prefix (see resolve()).
     * @exception InvalidArgument
     * @exception IOError
     * @see resolve(const std::string&, const std::string&, const std::string&, const std::string&)
     */
    Config(Section& sec, const std::string& prefix);

    virtual ~Config() {
    }

    /**
     * @return A human readable string representation of this configuration.
     */
    virtual std::string to_string() const;

private:
    /**
     * Resolve the fullpath for a palette or a keymaps file.
     * @param name   Name of the palette or keymaps or a full path;
     * @param path   Directory where the file is intended to be;
     * @param prefix Platform prefix;
     * @param ext    File extension.
     * @return The fullpath if the file is found (name or path + "/" + prefix + name + ext).
     * @exception IOError if the file does not exist in the specified path.
     */
    std::string resolve(const std::string& name, const std::string& path, const std::string& prefix,
        const std::string& ext);
};

}
}
