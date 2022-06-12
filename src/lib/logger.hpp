/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <map>
#include <string>

#include "types.hpp"


namespace cemu {

class LoggerError : public Error {
    using Error::Error;
};

class Logger {
public:
    constexpr static const char *DEFAULT_LOGFILE  = "/dev/tty";
    constexpr static const char *DEFAULT_LOGLEVEL = "";

    constexpr static const char *ERROR_STR        = "error";
    constexpr static const char *WARN_STR         = "warning";
    constexpr static const char *INFO_STR         = "info";
    constexpr static const char *DEBUG_STR        = "debug";
    constexpr static const char *ALL_STR          = "all";
    constexpr static const char *NONE_STR         = "none";

    constexpr static const char *ANSI_FG          = "\x1b[38;2;";
    constexpr static const char *ANSI_BG          = "\x1b[48;2;";
    constexpr static const char *ANSI_WHITE       = "255;255;255;1m";
    constexpr static const char *ANSI_RED         = "255;0;0m";
    constexpr static const char *ANSI_GREEN       = "0;255;0m";
    constexpr static const char *ANSI_YELLOW      = "255;255;0m";
    constexpr static const char *ANSI_RESET       = "\x1b[0m";

    enum Level {
        NONE    = 0x0,
        ERROR   = 0x1,
        WARN    = 0x2,
        INFO    = 0x4,
        DEBUG   = 0x8,
        ALL     = (ERROR | WARN | INFO | DEBUG),
        INVALID = -1
    };

    Logger();

    virtual ~Logger();

    bool is_level(Level lv) const {
        return (_lv & lv);
    }

    bool is_error() const {
        return is_level(ERROR);
    }

    bool is_warn() const {
        return is_level(WARN);
    }

    bool is_info() const {
        return is_level(INFO);
    }

    bool is_debug() const {
        return is_level(DEBUG);
    }

    void loglevel(Level lv) {
        _lv = lv;
    }

    void loglevel(const std::string &lvs);

    Level loglevel() const {
        return _lv;
    }

    void logfile(const std::string &fname);

    std::string logfile() const {
        return _logfile;
    }

    Logger &log(Level lv, const std::string &msg);

    Logger &log(Level lv, const char *fmt, va_list ap);

    Logger &log(Level lv, const char *fmt, ...);

    [[noreturn]] void fatal(const char *fmt, va_list ap);

    [[noreturn]] void fatal(const char *fmt, ...);

    Logger &error(const std::string &msg) {
        return log(ERROR, msg);
    }

    Logger &error(const char *fmt, va_list ap) {
        return log(ERROR, fmt, ap);
    }

    Logger &error(const char *fmt, ...);

    Logger &warn(const std::string &msg) {
        return log(WARN, msg);
    }

    Logger &warn(const char *fmt, va_list ap) {
        return log(WARN, fmt, ap);
    }

    Logger &warn(const char *fmt, ...);

    Logger &info(const std::string &msg) {
        return log(INFO, msg);
    }

    Logger &info(const char *fmt, va_list ap) {
        return log(INFO, fmt, ap);
    }

    Logger &info(const char *fmt, ...);

    Logger &debug(const std::string &msg) {
        return log(DEBUG, msg);
    }

    Logger &debug(const char *fmt, va_list ap) {
        return log(DEBUG, fmt, ap);
    }

    Logger &debug(const char *fmt, ...);

    /**
     * Convert a string to a level type.
     * @param level Log level string to convert.
     * @return The log level.
     */
    static Level to_loglevel(const std::string &level);

    /**
     * Convert a string with several log level strings to a bitwise combination of levels.
     * @param levels Log level string formatted as "level1|level2|..."
     * @return The bitwise combination of log levels.
     * @exception LoggerError If the specified string is malformed.
     */
    static Level parse_loglevel(const std::string &levels);

private:
    Level         _lv;
    std::ofstream _os;
    std::string   _logfile{};

    static std::map<std::string, Level> loglevel_map;
};

extern Logger log;

}
