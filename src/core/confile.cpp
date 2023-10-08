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

#include <regex>

#include "utils.hpp"


namespace caio {
namespace config {

Confile::Confile(const std::string& fname)
    : _sections{}
{
    load(fname);
}

void Confile::load(const std::string& fname)
{
    if (fname.empty()) {
        return;
    }

    std::ifstream ifs{fname};
    if (!ifs) {
        throw IOError{"Can't open configuration file: " + fname + ": " + Error::to_string()};
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
            std::string sname{utils::tolow(result[1])};
            cursect = &_sections[sname];
            continue;
        }

        if (!std::regex_match(str, result, re_param)) {
            /*
             * Invalid entry.
             */
            std::stringstream ss{};
            ss << fname << ": Invalid entry at line #" << line << ": " << std::quoted(str);
            throw ConfigError{ss.str()};
        }

        /*
         * Key-value pair detected.
         */
        if (cursect == nullptr) {
            /*
             * Key-value pair is not valid when a section is not defined (empty sections are not allowed).
             */
            std::stringstream ss{};
            ss << fname << ": Entry without section at line #" << line << ": " << std::quoted(str);
            throw ConfigError{ss.str()};
        }

        const std::string& key = utils::tolow(result[1]);
        const std::string& value = result[2];

        (*cursect)[key] = value;
    }
}

Section& Confile::operator[](const std::string& sname)
{
    return _sections[utils::tolow(sname)];
}

Section Confile::extract(const std::string& sname)
{
    auto nh = _sections.extract(utils::tolow(sname));
    return (nh ? std::move(nh.mapped()) : Section{});
}

std::map<std::string, Section>::const_iterator Confile::find(const std::string& sname) const
{
    return _sections.find(utils::tolow(sname));
}

std::map<std::string, Section>::const_iterator Confile::end() const
{
    return _sections.end();
}

}
}
