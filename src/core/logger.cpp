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
#include <cstdlib>
#include <iomanip>
#include <iterator>
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
             * Malformed levels string.
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
    _os.open(DEFAULT_LOGFILE);
}

void Logger::loglevel(std::string_view lvs)
{
    _lv = Logger::parse_loglevel(lvs);
}

void Logger::logfile(const fs::Path& fname)
{
    if (!fname.empty()) {
        std::ofstream ofs{fname};
        if (!ofs) {
            throw LoggerError{"Can't open logfile: {}", fname.string()};
        }

        _logfile = fname;
        _os = std::move(ofs);
    }
}

Logger& Logger::log(std::string_view color, std::string_view fmt, std::format_args args)
{
    _os << color;
    std::vformat_to(std::ostream_iterator<char>(_os), fmt, args);
    (_os << ANSI_RESET).flush();
    return *this;
}

}
