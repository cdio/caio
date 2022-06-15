/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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

#include <fstream>
#include <initializer_list>
#include <map>
#include <string>

#include "utils.hpp"


namespace caio {

class ConfileError : public Error {
    using Error::Error;
};


/**
 * Section of a configuration file.
 * @see Confile
 */
class ConfileSection : public std::map<std::string, std::string> {
public:
    ConfileSection() {
    }

    virtual ~ConfileSection() {
    }

    /**
     * Get a configuration value.
     * If the specified key does not exit an empty one is created.
     * @param key Key name.
     * @return The value associated with the specified key.
     */
    std::string &operator[](const std::string &key) {
        return (std::map<std::string, std::string>::operator[](key));
    }

    /**
     * Get a configuration value.
     * If the specified key does not exist a new one is created.
     * @param key    Key name.
     * @param dvalue Default value to set in case the specified key does not exist.
     * @return The value associated to the specified key.
     */
    std::string &operator()(const std::string &key, const std::string &dvalue = {});

    /**
     * Get a configuration value.
     * @param key Name of the key.
     * @return The value associated to the specified key.
     * @exception MissingKeyError
     */
    std::string &at(const std::string &key);

    const std::string &at(const std::string &key) const {
        return const_cast<ConfileSection *>(this)->at(key);
    }
};


/**
 * Configuration file.
 * A configuration file is conformed by one or more sections,
 * each section contains key-value pairs.
 * File format:
 *      [section_name_1]
 *      key1 = value1
 *      key2 = value2
 *      # comment
 *      [section_name_2]
 *      key1 = value1
 *      key2 = value2
 *
 * Section names are case insensitive. Key names are case sensitive.
 *
 * @see ConfileSection
 */
class Confile {
public:
    Confile() {
    }

    /**
     * Load a configuration file.
     * If the configuration file does not exist as it is specified,
     * it is searched in the provided search paths.
     * @param fname  Name of the configuration file;
     * @param spaths Search paths.
     */
    Confile(const std::string &fname, const std::initializer_list<std::string> &spaths = {}) {
        parse(fname, spaths);
    }

    virtual ~Confile() {
    }

    /**
     * @return true if the configuration file is successfully loaded; false otherwise.
     */
    operator bool() const {
        return (!operator!());
    }

    /**
     * @return true if the configuration file is successfully loaded; false otherwise.
     */
    bool operator!() const {
        return _fullpath.empty();
    }

    /**
     * @return The full pathname of the parsed configuration file.
     */
    const std::string &fullpath() const {
        return _fullpath;
    }

    /**
     * Open and parse a configuration file.
     * Any previous values on this instance is lost.
     * @param fname  Name of the configuration file to open;
     * @param spaths Search paths.
     * @return true if the file is loaded and parsed; false otherwise.
     * @exception ConfileError if the specified file is not found or cannot be opened.
     */
    virtual void parse(const std::string &fname, const std::initializer_list<std::string> &spaths = {});

    /**
     * Get a configuration section.
     * If the specified section does not exit an empty one is created.
     * @param sname Name of the section.
     * @return The requested section.
     */
    ConfileSection &operator[](const std::string &sname) {
        return _sections[utils::tolow(sname)];
    }

    /**
     * Find a section.
     * @param secname Section name.
     * @return An iterator to the section; or end() if the section does not exists.
     */
    std::map<std::string, ConfileSection>::const_iterator find(const std::string &secname) const {
        return _sections.find(secname);
    }

    /**
     * @return An iterator to the last section plus one.
     */
    std::map<std::string, ConfileSection>::const_iterator end() const {
        return _sections.end();
    }

    /**
     * Get a configuration section.
     * @param sname Name of the section.
     * @return The required section.
     * @exception MissingSectionError
     */
    ConfileSection &at(const std::string &sname);

    const ConfileSection &at(const std::string &key) const {
        return const_cast<Confile *>(this)->at(key);
    }

    /**
     * @return A human readable representation of this configuration.
     */
    std::string to_string() const;

private:
    std::ifstream open(const std::string &fname, const std::initializer_list<std::string> &spaths = {});

    void load(std::ifstream &is);

    std::string                           _fullpath{};
    std::map<std::string, ConfileSection> _sections{};
};

}
