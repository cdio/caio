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

#include <fcntl.h>
#include <unistd.h>

#include <climits>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <regex>

#include "utils.hpp"

namespace caio {

std::unordered_map<std::string, Logger::Level> Logger::loglevels = {
    { ERROR_STR,    Logger::Error   },
    { WARN_STR,     Logger::Warn    },
    { INFO_STR,     Logger::Info    },
    { DEBUG_STR,    Logger::Debug   },
    { ALL_STR,      Logger::All     },
    { NONE_STR,     Logger::None    }
};

Logger log{};

Logger::Level Logger::to_loglevel(std::string_view level)
{
    if (level.empty()) {
        return Level::None;
    }

    auto it = loglevels.find(std::string{level});
    return (it == loglevels.end() ? Level::Invalid : it->second);
}

Logger::Level Logger::parse_loglevel(std::string_view levels)
{
    static const std::regex re_loglevel("([^\\|]+)", std::regex::extended);
    int loglevel = Level::None;

    const  std::string lvls{levels};
    for (auto it = std::sregex_iterator(lvls.begin(), lvls.end(), re_loglevel);
        it != std::sregex_iterator(); ++it) {

        auto lstr = utils::trim(it->str());
        Level l = Logger::to_loglevel(lstr);
        if (l == Level::Invalid) {
            /*
             * Malformed level string.
             */
            throw LoggerError{"Invalid log level: \"{}\", complete log level argument: \"{}\"", lstr, levels};
        }

        loglevel |= l;
    }

    return static_cast<Level>(loglevel);
}

Logger::Logger()
    : _lv{Logger::parse_loglevel(DEFAULT_LOGLEVEL)}
{
    Logger::logfile(DEFAULT_LOGFILE);
}

Logger::~Logger()
{
    if (_fd >= 0) {
        ::close(_fd);
    }
}

void Logger::logfile(const fs::Path& fname)
{
    const int fd = ::open(fname.c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        throw LoggerError{"Can't open logfile: {}: {}\n", fname.c_str(), Error::to_string()};
    }

    try {
        logfile(fd);
    } catch (const LoggerError& err) {
        ::close(fd);
        throw err;
    }
}

void Logger::logfile(int fd)
{
    if (_fd >= 0) {
        ::close(_fd);
    }

    _fd = ::dup(fd);
    if (_fd < 0) {
        throw LoggerError{"Can't duplicate file descriptor: {}", Error::to_string()};
    }
}

void Logger::loglevel(std::string_view lvs)
{
    _lv = Logger::parse_loglevel(lvs);
}

Logger& Logger::log(std::string_view color, std::string_view fmt, std::format_args args)
{
    ::write(_fd, color.data(), color.size());
    const auto msg = std::vformat(fmt, args);
    ::write(_fd, msg.data(), msg.size());
    ::write(_fd, ANSI_RESET, std::strlen(ANSI_RESET));
    return *this;
}

}
