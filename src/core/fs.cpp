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
#include "fs.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <sstream>

#include "logger.hpp"
#include "types.hpp"


namespace caio {
namespace fs {

std::string home()
{
    static const char* chome = std::getenv("HOME");
    return (chome == nullptr ? "" : chome);
}

std::string fix_home(const std::string& path)
{
    if (!path.empty() && path[0] == '~') {
        std::string newpath{path};
        newpath.replace(0, 1, home() + "/");
        return newpath;
    }

    return path;
}

bool exists(const std::string& path)
{
    //TODO: use std::filesystem::exists() ?
    struct ::stat st{};
    return (::stat(path.c_str(), &st) == 0);
}

std::string search(const std::string& fname, const std::initializer_list<std::string>& spath, bool cwd)
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

std::string basename(const std::string& fullpath)
{
    auto pos = fullpath.find_last_of("/");
    return (pos == std::string::npos ? fullpath : fullpath.substr(pos + 1));
}

void concat(const std::string& dst, const std::string& src)
{
    std::ifstream is{src, std::ios_base::in | std::ios_base::binary};
    if (!is) {
        throw IOError{"Can't open input file: " + src + ": " + Error::to_string(errno)};
    }

    std::ofstream os{dst, std::ios_base::out | std::ios_base::app | std::ios_base::binary};
    if (!os) {
        throw IOError{"Can't open output file: " + dst + ": " + Error::to_string(errno)};
    }

    try {
        is.unsetf(std::ios_base::skipws);
        std::copy(std::istream_iterator<uint8_t>(is), std::istream_iterator<uint8_t>(),
            std::ostream_iterator<uint8_t>(os));
    } catch (const std::exception& err) {
        throw IOError{err};
    }
}

bool unlink(const std::string& fname)
{
    return (fname.empty() || !::unlink(fname.c_str()));
}

bool match(const std::string& path, const std::string& pattern)
{
    const char* cpath = path.c_str();
    const char* cpattern = pattern.c_str();
    return (!::fnmatch(cpattern, cpath, FNM_NOESCAPE));
}

bool directory(const std::string& path, const std::string& pattern,
    const std::function<bool(const std::string&, uint64_t)>& callback)
{
    const auto& end = std::filesystem::end(std::filesystem::recursive_directory_iterator{});
    std::filesystem::recursive_directory_iterator it{path, std::filesystem::directory_options::skip_permission_denied};

    for (; it != end; ++it) {
        const auto& entry = it->path();
        if (!std::filesystem::is_directory(entry) && fs::match(entry, pattern)) {
            auto size = std::filesystem::file_size(entry);
            if (callback(entry, size) == false) {
                return false;
            }
        }
    }

    return true;
}

dir_t directory(const std::string& path, const std::string& pattern, size_t limit)
{
    dir_t entries{};
    bool limited = (limit != 0);

    directory(path, pattern, [&entries, &limit, limited](const std::string& entry, uint64_t size) -> bool {
        if (limited) {
            if (limit == 0) {
                return false;
            }
            --limit;
        }
        entries.push_back({entry, size});
        return true;
    });

    return entries;
}

std::vector<uint8_t> load(const std::string& fname, size_t maxsiz)
{
    std::string errmsg{};

    if (maxsiz == 0) {
        maxsiz = LOAD_MAXSIZ;
    }

    try {
        std::ifstream is{fname, std::ios::in};
        if (is) {
            return load(is, maxsiz);
        }
    } catch (const std::exception& err) {
        errmsg = err.what();
    }

    throw IOError{"Can't load: " + fname + ": " + (errmsg.empty() ? Error::to_string() : errmsg)};
}

std::vector<uint8_t> load(std::istream& is, size_t maxsiz)
{
    std::vector<uint8_t> buf{};
    uint8_t c{};

    if (maxsiz == 0) {
        maxsiz = LOAD_MAXSIZ;
    }

    while (buf.size() < maxsiz) {
        if (!is.read(reinterpret_cast<char*>(&c), sizeof(c))) {
            if (!is.eof()) {
                throw IOError{"Can't read from stream: " + Error::to_string()};
            }
            break;
        }

        buf.push_back(c);
    }

    return buf;
}

void save(const std::string& fname, const gsl::span<uint8_t>& buf, std::ios_base::openmode mode)
{
    std::string errmsg{};

    try {
        std::ofstream os{fname, mode};
        if (os) {
            save(os, buf);
            return;
        }
    } catch (const std::exception& err) {
        errmsg = err.what();
    }

    throw IOError{"Can't save: " + fname + ": " + (errmsg.empty() ? Error::to_string() : errmsg)};
}

std::ostream& save(std::ostream& os, const gsl::span<uint8_t>& buf)
{
    if (!os.write(reinterpret_cast<char*>(buf.data()), buf.size())) {
        throw IOError{"Can't write: " + Error::to_string()};
    }

    return os;
}

}
}
