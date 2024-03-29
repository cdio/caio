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

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <map>
#include <string>

#include "types.hpp"


namespace caio {

class LoggerError : public Error {
    using Error::Error;
};

class Logger {
public:
    constexpr static const char* ERROR_STR        = "error";
    constexpr static const char* WARN_STR         = "warning";
    constexpr static const char* INFO_STR         = "info";
    constexpr static const char* DEBUG_STR        = "debug";
    constexpr static const char* ALL_STR          = "all";
    constexpr static const char* NONE_STR         = "none";

    constexpr static const char* ANSI_FG          = "\x1b[38;2;";
    constexpr static const char* ANSI_BG          = "\x1b[48;2;";
    constexpr static const char* ANSI_WHITE       = "255;255;255;1m";
    constexpr static const char* ANSI_RED         = "255;0;0m";
    constexpr static const char* ANSI_GREEN       = "0;255;0m";
    constexpr static const char* ANSI_YELLOW      = "255;255;0m";
    constexpr static const char* ANSI_RESET       = "\x1b[0m";

    constexpr static const char* DEFAULT_LOGFILE  = "/dev/tty";
    constexpr static const char* DEFAULT_LOGLEVEL = NONE_STR;

    enum Level {
        None    = 0x00,
        Error   = 0x01,
        Warn    = 0x02,
        Info    = 0x04,
        Debug   = 0x08,
        All     = Error | Warn | Info | Debug,
        Invalid = 0xFF
    };

    Logger();

    bool is_level(Level lv) const {
        return (_lv & lv);
    }

    bool is_error() const {
        return is_level(Level::Error);
    }

    bool is_warn() const {
        return is_level(Level::Warn);
    }

    bool is_info() const {
        return is_level(Level::Info);
    }

    bool is_debug() const {
        return is_level(Level::Debug);
    }

    void loglevel(Level lv) {
        _lv = lv;
    }

    void loglevel(const std::string& lvs);

    Level loglevel() const {
        return _lv;
    }

    void logfile(const std::string& fname);

    std::string logfile() const {
        return _logfile;
    }

    Logger& log(Level lv, const std::string& msg);

    Logger& log(Level lv, const char* fmt, va_list ap);

    Logger& log(Level lv, const char* fmt, ...);

    [[noreturn]] void fatal(const char* fmt, va_list ap);

    [[noreturn]] void fatal(const char* fmt, ...);

    Logger& error(const std::string& msg) {
        return log(Level::Error, msg);
    }

    Logger& error(const char* fmt, va_list ap) { //FIXME
        return log(Level::Error, fmt, ap);
    }

    Logger& error(const char* fmt, ...); //FIXME

    Logger& warn(const std::string& msg) {
        return log(Level::Warn, msg);
    }

    Logger& warn(const char* fmt, va_list ap) {
        return log(Level::Warn, fmt, ap);
    }

    Logger& warn(const char* fmt, ...);

    Logger& info(const std::string& msg) {
        return log(Level::Info, msg);
    }

    Logger& info(const char* fmt, va_list ap) {
        return log(Level::Info, fmt, ap);
    }

    Logger& info(const char* fmt, ...);

    Logger& debug(const std::string& msg) {
        return log(Level::Debug, msg);
    }

    Logger& debug(const char* fmt, va_list ap) {
        return log(Level::Debug, fmt, ap);
    }

    Logger& debug(const char* fmt, ...);

    /**
     * Convert a string to a level type.
     * @param level Log level string to convert.
     * @return The log level.
     */
    static Level to_loglevel(const std::string& level);

    /**
     * Convert a string with several log level strings to a bitwise combination of levels.
     * @param levels Log level string formatted as "level1|level2|..."
     * @return The bitwise combination of log levels.
     * @exception LoggerError If the specified string is malformed.
     */
    static Level parse_loglevel(const std::string& levels);

private:
    Level         _lv;
    std::ofstream _os;
    std::string   _logfile{};

    static std::map<std::string, Logger::Level> loglevels;
};

using Loglevel = Logger::Level;

extern Logger log;

}
