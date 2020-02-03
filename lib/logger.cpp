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
#include "logger.hpp"

#include <climits>

#include "utils.hpp"


namespace cemu {

Logger log{};


void Logger::loglevel(const std::string &lvs)
{
    auto llvs = utils::tolow(lvs);
    _lv = static_cast<Level>(
        (llvs.find("e") == std::string::npos ? 0 : ERROR) |
        (llvs.find("w") == std::string::npos ? 0 : WARN)  |
        (llvs.find("i") == std::string::npos ? 0 : INFO)  |
        (llvs.find("d") == std::string::npos ? 0 : DEBUG));
}

void Logger::logfile(const std::string &fname)
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

Logger &Logger::log(Level lv, const std::string &msg)
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
        case ERROR:
            color += std::string{ANSI_WHITE} + ANSI_BG + ANSI_RED;
            break;

        case WARN:
            color += ANSI_YELLOW;
            break;

        case DEBUG:
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

Logger &Logger::log(Level lv, const char *fmt, va_list ap)
{
    char buf[LINE_MAX];

    std::vsnprintf(buf, sizeof(buf), fmt, ap);

    return log(lv, std::string{buf});
}

Logger &Logger::log(Level lv, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log(lv, fmt, ap);
    va_end(ap);

    return *this;
}

Logger &Logger::error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    error(fmt, ap);
    va_end(ap);

    return *this;
}

Logger &Logger::warn(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    warn(fmt, ap);
    va_end(ap);

    return *this;
}

Logger &Logger::info(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    info(fmt, ap);
    va_end(ap);

    return *this;
}

Logger &Logger::debug(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    debug(fmt, ap);
    va_end(ap);

    return *this;
}

}
