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
#include "confile.hpp"

#include <regex>

#include "fs.hpp"
#include "logger.hpp"


namespace cemu {

std::string &ConfileSection::operator()(const std::string &key, const std::string &dvalue)
{
    if (find(key) != end()) {
        return operator[](key);
    }

    std::string &value = operator[](key);
    value = dvalue;

    return value;
}

std::string &ConfileSection::at(const std::string &key)
{
    try {
        return std::map<std::string, std::string>::at(key);
    } catch (std::out_of_range &) {
        throw MissingKeyError{key};
    }
}

std::ifstream Confile::open(const std::string &fname, const std::initializer_list<std::string> &spaths)
{
    std::string fullpath = fs::search(fname, spaths);
    if (fullpath.empty()) {
        throw ConfileError{fname + ": " + Error::to_string(ENOENT)};
    }

    if (fullpath != fname) {
        log.debug("Loading configuration from " + fullpath + "...\n");
    }

    std::ifstream ifs{fullpath};
    if (!ifs) {
        return {};
    }

    _fullpath = fullpath;
    _sections.clear();

    return ifs;
}

void Confile::load(std::ifstream &ifs)
{
    static const std::regex re_comment("^[ \t]*#.*$", std::regex::extended);
    static const std::regex re_section("^[ \t]*\\[[ \t]*([^[ \t\\]]+)[ \t]*\\].*$");
    static const std::regex re_param("^[ \t]*([^ \t=]+)[ \t]*=[ \t]*([^ \t]*)[ \t]*$", std::regex::extended);

    std::smatch result{};
    std::string str{};

    ConfileSection *cursect = nullptr;
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
            std::string secname{utils::tolow(result[1])};
            cursect = &_sections[secname];
            continue;
        }

        if (!std::regex_match(str, result, re_param)) {
            /*
             * Invalid entry.
             */
            std::stringstream ss;
            ss << fullpath() << ": Invalid entry at line #" << line << ": " << std::quoted(str);
            throw ConfileError{ss.str()};
        }

        /*
         * Key=value pair detected.
         */
        if (cursect == nullptr) {
            /*
             * Key-value pair is not valid when a section is not defined (empty sections are not supported).
             */
            std::stringstream ss;
            ss << fullpath() << ": Entry without section at line #" << line << ": " << std::quoted(str);
            throw ConfileError{ss.str()};
        }

        const std::string &key = result[1];
        const std::string &value = result[2];
        (*cursect)[key] = value;
    }
}

void Confile::parse(const std::string &fname, const std::initializer_list<std::string> &spaths)
{
    std::ifstream ifs{open(fname, spaths)};
    load(ifs);
}

ConfileSection &Confile::at(const std::string &sname)
{
    try {
        return (_sections.at(utils::tolow(sname)));
    } catch (const std::out_of_range &) {
        throw MissingSectionError{sname};
    }
}

std::string Confile::to_string() const
{
    std::stringstream ss;

    ss << "config file=" << std::quoted(fullpath()) << std::endl;
    for (const auto &sec : _sections) {
        const std::string &secname = sec.first;
        const ConfileSection &secdata = sec.second;
        ss << "[ " << secname << " ]" << std::endl;
        for (const auto &elem : secdata) {
            ss << "\t" << elem.first << " = " << elem.second << std::endl;
        }
    }

    return ss.str();
}

}
