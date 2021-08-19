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
#include <string>

#include "types.hpp"


namespace cemu {

class LoggerError : public Error {
    using Error::Error;
};


class Logger {
public:
    constexpr static const char *ANSI_FG     = "\x1b[38;2;";
    constexpr static const char *ANSI_BG     = "\x1b[48;2;";
    constexpr static const char *ANSI_WHITE  = "255;255;255;1m";
    constexpr static const char *ANSI_RED    = "255;0;0m";
    constexpr static const char *ANSI_GREEN  = "0;255;0m";
    constexpr static const char *ANSI_YELLOW = "255;255;0m";
    constexpr static const char *ANSI_RESET  = "\x1b[0m";

    enum Level {
        NONE  = 0x0,
        ERROR = 0x1,
        WARN  = 0x2,
        INFO  = 0x4,
        DEBUG = 0x8,
        ALL   = (ERROR | WARN | INFO | DEBUG)
    };

    constexpr static const char *DEFAULT_LOGFILE         = "/dev/null";
    constexpr static const char *DEFAULT_LOGLEVEL_STRING = "";
    constexpr static Level DEFAULT_LOGLEVEL              = Level::NONE;


    Logger() {
        _os.open(DEFAULT_LOGFILE);
    }

    explicit Logger(Level lv)
        : _lv{lv} {
    }

    virtual ~Logger() {
    }

    bool is_level(Level lv) const {
        return (_lv & lv);
    }

    bool is_level_error() const {
        return is_level(ERROR);
    }

    bool is_level_warn() const {
        return is_level(WARN);
    }

    bool is_level_info() const {
        return is_level(INFO);
    }

    bool is_level_debug() const {
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

private:
    Level         _lv{DEFAULT_LOGLEVEL};
    std::string   _logfile{};
    std::ofstream _os;
};


extern Logger log;

}
