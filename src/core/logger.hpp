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
#include <cstdint>
#include <format>
#include <fstream>
#include <map>
#include <string>
#include <string_view>

#include "types.hpp"

namespace caio {

#define ANSI_FG                 "\x1b[38;2;"
#define ANSI_BG                 "\x1b[48;2;"
#define ANSI_WHITE              "255;255;255;1m"
#define ANSI_RED                "255;0;0m"
#define ANSI_GREEN              "0;255;0m"
#define ANSI_YELLOW             "255;255;0m"
#define ANSI_RESET              "\x1b[0m"

class Logger {
public:
    constexpr static const char* ERROR_STR        = "error";
    constexpr static const char* WARN_STR         = "warning";
    constexpr static const char* INFO_STR         = "info";
    constexpr static const char* DEBUG_STR        = "debug";
    constexpr static const char* ALL_STR          = "all";
    constexpr static const char* NONE_STR         = "none";

    constexpr static const char* ERROR_COLOR      = ANSI_FG ANSI_WHITE ANSI_BG ANSI_RED;
    constexpr static const char* WARN_COLOR       = ANSI_FG ANSI_YELLOW;
    constexpr static const char* DEBUG_COLOR      = ANSI_FG ANSI_GREEN;
    constexpr static const char* INFO_COLOR       = ANSI_RESET;

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

    void loglevel(std::string_view lvs);

    Level loglevel() const {
        return _lv;
    }

    void logfile(std::string_view fname);

    std::string logfile() const {
        return _logfile;
    }

    Logger& log(std::string_view color, std::string_view fmt, std::format_args args);

    template<typename... Args>
    [[noreturn]]
    Logger& fatal(std::format_string<Args...> fmt, Args&&... args) {
        if (is_error()) {
            log(ERROR_COLOR, fmt.get(), std::make_format_args(args...));
        }
        std::exit(EXIT_FAILURE);
    }

    template<typename... Args>
    Logger& error(std::format_string<Args...> fmt, Args&&... args) {
        if (is_error()) {
            log(ERROR_COLOR, fmt.get(), std::make_format_args(args...));
        }
        return *this;
    }

    template<typename... Args>
    Logger& warn(std::format_string<Args...> fmt, Args&&... args) {
        if (is_warn()) {
            log(WARN_COLOR, fmt.get(), std::make_format_args(args...));
        }
        return *this;
    }

    template<typename... Args>
    Logger& info(std::format_string<Args...> fmt, Args&&... args) {
        if (is_info()) {
            log(INFO_COLOR, fmt.get(), std::make_format_args(args...));
        }
        return *this;
    }

    template<typename... Args>
    Logger& debug(std::format_string<Args...> fmt, Args&&... args) {
        if (is_debug()) {
            log(DEBUG_COLOR, fmt.get(), std::make_format_args(args...));
        }
        return *this;
    }

    /**
     * Convert a string to a level type.
     * @param level Log level string to convert.
     * @return The log level.
     */
    static Level to_loglevel(std::string_view level);

    /**
     * Convert a string with several log level strings to a bitwise combination of levels.
     * @param levels Log level string formatted as "level1|level2|..."
     * @return The bitwise combination of log levels.
     * @exception LoggerError If the specified string is malformed.
     */
    static Level parse_loglevel(std::string_view levels);

private:
    Level         _lv;
    std::ofstream _os;
    std::string   _logfile{};

    static std::map<std::string, Logger::Level> loglevels;
};

using Loglevel = Logger::Level;

extern Logger log;

}
