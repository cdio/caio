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
#include "fs.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>

#include "logger.hpp"


namespace cemu {
namespace fs {

std::string home()
{
    static const char *chome = std::getenv("HOME");
    return (chome == nullptr ? "" : chome);
}

std::string fix_home(const std::string &path)
{
    if (!path.empty() && path[0] == '~') {
        std::string newpath{path};
        newpath.replace(0, 1, home() + "/");
        return newpath;
    }

    return path;
}

bool exists(const std::string &path)
{
    struct ::stat st{};
    return (::stat(path.c_str(), &st) == 0);
}

std::string search(const std::string &fname, const std::initializer_list<std::string> &spath, bool cwd)
{
    if (fname.empty()) {
        return {};
    }

    log.debug("Looking for file: " + fname + ": ");

    const auto name = basename(fname);
    if (name != fname) {
        /*
         * A directory is specified in fname.
         */
        const auto fullpath = fix_home(fname);
        if (exists(fullpath)) {
            log.debug("Found: " + fullpath + "\n");
            return fullpath;
        }
    }

    if (cwd) {
        /*
         * Directory not specified in fname.
         * Search on the current working directory.
         */
        log.debug("Looking for file: " + name + ": ");
        if (exists(name)) {
            log.debug("Found: " + name + "\n");
            return fname;
        }
    }

    for (const auto &path : spath) {
        std::string fullpath = fix_home(path) + "/" + name;
        log.debug("Trying " + fullpath + "... ");
        if (exists(fullpath)) {
            log.debug("Found\n");
            return fullpath;
        }
    }

    log.debug("Not found\n");
    return {};
}

std::string basename(const std::string &fullpath)
{
    auto pos = fullpath.find_last_of("/");
    return (pos == std::string::npos ? fullpath : fullpath.substr(pos + 1));
}

}
}
