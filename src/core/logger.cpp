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
#include "logger.hpp"

#include <climits>
#include <iomanip>
#include <regex>

#include "utils.hpp"


namespace caio {

std::map<std::string, Logger::Level> Logger::loglevels = {
    { ERROR_STR,    Logger::Error   },
    { WARN_STR,     Logger::Warn    },
    { INFO_STR,     Logger::Info    },
    { DEBUG_STR,    Logger::Debug   },
    { ALL_STR,      Logger::All     },
    { NONE_STR,     Logger::None    }
};

Logger log{};

Logger::Level Logger::to_loglevel(const std::string& level)
{
    if (level.empty()) {
        return Level::None;
    }

    auto it = loglevels.find(level);
    return (it == loglevels.end() ? Level::Invalid : it->second);
}

Logger::Level Logger::parse_loglevel(const std::string& levels)
{
    static const std::regex re_loglevel("([^\\|]+)", std::regex::extended);
    int loglevel = Level::None;

    for (auto it = std::sregex_iterator(levels.begin(), levels.end(), re_loglevel);
        it != std::sregex_iterator(); ++it) {

        const std::string& lstr = utils::trim(it->str());
        Level l = Logger::to_loglevel(lstr);
        if (l == Level::Invalid) {
            /*
             * Malformed levels string.
             */
            std::ostringstream ss{};
            ss << "Invalid log level: " << std::quoted(lstr)
               << ", complete log level specification: " << std::quoted(levels);
            throw LoggerError{ss.str()};
        }

        loglevel |= l;
    }

    return static_cast<Level>(loglevel);
}

Logger::Logger()
    : _lv{Logger::parse_loglevel(DEFAULT_LOGLEVEL)}
{
    _os.open(DEFAULT_LOGFILE);
}

void Logger::loglevel(const std::string& lvs)
{
    _lv = Logger::parse_loglevel(lvs);
}

void Logger::logfile(const std::string& fname)
{
    if (!fname.empty()) {
        std::ofstream ofs{fname};
        if (!ofs) {
            throw LoggerError{"Can't open logfile: " + fname};
        }

        _logfile = fname;
        _os = std::move(ofs);
    }
}

Logger& Logger::log(Level lv, const std::string& msg)
{
    if (_os && is_level(lv) && !msg.empty()) {
        std::string m{msg};
        char nl = msg.back();
        if (nl == '\n') {
            m.erase(m.end() - 1);
        } else {
            nl = '\0';
        }

        std::string color{ANSI_FG};
        switch (lv) {
        case Level::Error:
            color += std::string{ANSI_WHITE} + ANSI_BG + ANSI_RED;
            break;

        case Level::Warn:
            color += ANSI_YELLOW;
            break;

        case Level::Debug:
            color += ANSI_GREEN;
            break;

        default:
            color = ANSI_RESET;
        }

        _os << color << m << ANSI_RESET << nl;
        _os.flush();
    }

    return *this;
}

Logger& Logger::log(Level lv, const char* fmt, va_list ap)
{
    char buf[LINE_MAX];

    std::vsnprintf(buf, sizeof(buf), fmt, ap);

    return log(lv, std::string{buf});
}

Logger& Logger::log(Level lv, const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log(lv, fmt, ap);
    va_end(ap);

    return *this;
}

[[noreturn]]
void Logger::fatal(const char* fmt, va_list ap)
{
    error(fmt, ap);
    std::terminate();
}

[[noreturn]]
void Logger::fatal(const char* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fatal(fmt, ap);
    va_end(ap);
}

Logger& Logger::error(const char* fmt, ...)
{
    if (is_error()) {
        va_list ap;

        va_start(ap, fmt);
        error(fmt, ap);
        va_end(ap);
    }

    return *this;
}

Logger& Logger::warn(const char* fmt, ...)
{
    if (is_warn()) {
        va_list ap;

        va_start(ap, fmt);
        warn(fmt, ap);
        va_end(ap);
    }

    return *this;
}

Logger& Logger::info(const char* fmt, ...)
{
    if (is_info()) {
        va_list ap;

        va_start(ap, fmt);
        info(fmt, ap);
        va_end(ap);
    }

    return *this;
}

Logger& Logger::debug(const char* fmt, ...)
{
    if (is_debug()) {
        va_list ap;

        va_start(ap, fmt);
        debug(fmt, ap);
        va_end(ap);
    }

    return *this;
}

}
