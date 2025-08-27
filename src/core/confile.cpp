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

#include "utils.hpp"

#include <format>
#include <fstream>
#include <regex>

namespace caio {
namespace config {

Confile::Confile(const fs::Path& fname)
    : _sections{}
{
    load(fname);
}

void Confile::load(const fs::Path& fname)
{
    if (fname.empty()) {
        return;
    }

    std::ifstream ifs{fname};
    if (!ifs) {
        throw IOError{"Can't open configuration file: {}: {}", fname.string(), Error::to_string()};
    }

    static const std::regex re_comment("^[ \t]*#.*$", std::regex::extended);
    static const std::regex re_section("^[ \t]*\\[[ \t]*([^[ \t\\]]+)[ \t]*\\].*$");
    static const std::regex re_param("^[ \t]*([^ \t=]+)[ \t]*=[ \t]*(.*)[ \t]*$", std::regex::extended);

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
            auto sname = utils::tolow(result.str(1));
            cursect = &_sections[sname];
            continue;
        }

        if (!std::regex_match(str, result, re_param)) {
            /*
             * Invalid entry.
             */
            throw ConfigError{"{}: Invalid entry at line #{}: \"{}\"", fname.string(), line, str};
        }

        /*
         * Key-value pair detected.
         */
        if (cursect == nullptr) {
            /*
             * Key-value pair is not valid when a section is not defined (empty sections are not allowed).
             */
            throw ConfigError{"{}: Entry without section at line #{}: \"{}\"", fname.string(), line, str};
        }

        const auto key = utils::tolow(result.str(1));
        const auto value = result.str(2);
        (*cursect)[key] = value;
    }
}

Section& Confile::operator[](std::string_view sname)
{
    return _sections[utils::tolow(sname)];
}

Section Confile::extract(std::string_view sname)
{
    auto nh = _sections.extract(utils::tolow(sname));
    return (nh ? std::move(nh.mapped()) : Section{});
}

std::unordered_map<std::string, Section>::const_iterator Confile::find(std::string_view sname) const
{
    return _sections.find(utils::tolow(sname));
}

std::unordered_map<std::string, Section>::const_iterator Confile::end() const
{
    return _sections.end();
}

}
}
