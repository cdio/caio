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
#include "config.hpp"

#include <format>
#include <regex>

#include "utils.hpp"

namespace caio {
namespace config {

Confile::Confile(std::string_view fname)
    : _sections{}
{
    load(fname);
}

void Confile::load(std::string_view fname)
{
    if (fname.empty()) {
        return;
    }

    std::ifstream ifs{std::string{fname}};
    if (!ifs) {
        throw IOError{"Can't open configuration file: {}: {}", fname, Error::to_string()};
    }

    static const std::regex re_comment("^[ \t]*#.*$", std::regex::extended);
    static const std::regex re_section("^[ \t]*\\[[ \t]*([^[ \t\\]]+)[ \t]*\\].*$");
    static const std::regex re_param("^[ \t]*([^ \t=]+)[ \t]*=[ \t]*([^ \t]*)[ \t]*$", std::regex::extended);

    std::smatch result{};
    std::string str{};

    Section* cursect = nullptr;
    size_t line = 0;

    while (std::getline(ifs, str)) {
        ++line;
        if (str.empty() || std::regex_match(str, re_comment)) {
            /*
             * Empty line or comment: Continue with the next line.
             */
            continue;
        }

        if (std::regex_match(str, result, re_section)) {
            /*
             * Section detected.
             */
            auto sname = caio::tolow(result.str(1));
            cursect = &_sections[sname];
            continue;
        }

        if (!std::regex_match(str, result, re_param)) {
            /*
             * Invalid entry.
             */
            throw ConfigError{"{}: Invalid entry at line #{}: \"{}\"", fname, line, str};
        }

        /*
         * Key-value pair detected.
         */
        if (cursect == nullptr) {
            /*
             * Key-value pair is not valid when a section is not defined (empty sections are not allowed).
             */
            throw ConfigError{"{}: Entry without section at line #{}: \"{}\"", fname, line, str};
        }

        const auto key = caio::tolow(result.str(1));
        const auto value = result[2];

        (*cursect)[key] = value;
    }
}

Section& Confile::operator[](std::string_view sname)
{
    return _sections[caio::tolow(sname)];
}

Section Confile::extract(std::string_view sname)
{
    auto nh = _sections.extract(caio::tolow(sname));
    return (nh ? std::move(nh.mapped()) : Section{});
}

std::map<std::string, Section>::const_iterator Confile::find(std::string_view sname) const
{
    return _sections.find(caio::tolow(sname));
}

std::map<std::string, Section>::const_iterator Confile::end() const
{
    return _sections.end();
}

}
}
