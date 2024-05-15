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

std::string fix_home(std::string_view path)
{
    if (!path.empty() && path[0] == '~') {
        std::string newpath{path};
        newpath.replace(0, 1, home() + "/");
        return newpath;
    }

    return std::string{path};
}

std::string search(std::string_view fname, const std::initializer_list<std::string>& spath, bool cwd)
{
    if (fname.empty()) {
        return {};
    }

    log.debug("Looking for file: {}: ", fname);

    auto name = basename(fname);
    if (name != fname) {
        /*
         * A directory is specified in fname.
         */
        const auto fullpath = fix_home(fname);
        if (exists(fullpath)) {
            log.debug("Found: {}\n", fullpath);
            return fullpath;
        }
    }

    if (cwd) {
        /*
         * Directory not specified in fname.
         * Search on the current working directory.
         */
        log.debug("Looking for file: {}: ", name);
        if (exists(name)) {
            log.debug("Found: {}\n", name);
            return std::string{fname};
        }
    }

    for (const auto& path : spath) {
        std::string fullpath = fix_home(path) + "/" + std::string{name};
        log.debug("Trying {}...", fullpath);
        if (exists(fullpath)) {
            log.debug("Found\n");
            return fullpath;
        }
    }

    log.debug("Not found\n");
    return {};
}

std::string_view basename(std::string_view fullpath)
{
    auto pos = fullpath.find_last_of("/");
    return (pos == std::string::npos ? fullpath : fullpath.substr(pos + 1));
}

std::string_view dirname(std::string_view fullpath)
{
    auto pos = fullpath.find_last_of("/");
    return (pos == std::string::npos ? "./" : fullpath.substr(0, pos));
}

void concat(std::string_view dst, std::string_view src)
{
    std::ifstream is{src, std::ios_base::in | std::ios_base::binary};
    if (!is) {
        throw IOError{"Can't open input file: {}: {}", src, Error::to_string(errno)};
    }

    std::ofstream os{dst, std::ios_base::out | std::ios_base::app | std::ios_base::binary};
    if (!os) {
        throw IOError{"Can't open output file: {}: {}", dst, Error::to_string(errno)};
    }

    try {
        is.unsetf(std::ios_base::skipws);
        std::copy(std::istream_iterator<uint8_t>(is), std::istream_iterator<uint8_t>(),
            std::ostream_iterator<uint8_t>(os));
    } catch (const std::exception& err) {
        throw IOError{err};
    }
}

bool unlink(std::string_view fname)
{
    return (fname.empty() || !::unlink(fname.data()));
}

bool match(std::string_view path, std::string_view pattern, bool icase)
{
    const char* cpath = path.data();
    const char* cpattern = pattern.data();
    return (!::fnmatch(cpattern, cpath, FNM_NOESCAPE | (icase == MATCH_CASE_INSENSITIVE ? FNM_CASEFOLD : 0)));
}

bool directory(std::string_view path, std::string_view pattern, bool icase,
    const std::function<bool(std::string_view, uint64_t)>& callback)
{
    const auto& end = std::filesystem::end(std::filesystem::recursive_directory_iterator{});
    std::filesystem::recursive_directory_iterator it{path, std::filesystem::directory_options::skip_permission_denied};

    for (; it != end; ++it) {
        const auto& entry = it->path();
        if (!std::filesystem::is_directory(entry) && fs::match(entry.c_str(), pattern, icase)) {
            auto size = std::filesystem::file_size(entry);
            if (callback(entry.c_str(), size) == false) {
                return false;
            }
        }
    }

    return true;
}

dir_t directory(std::string_view path, std::string_view pattern, bool icase, size_t limit)
{
    dir_t entries{};
    bool limited = (limit != 0);

    directory(path, pattern, icase, [&entries, &limit, limited](const std::string_view entry, uint64_t size) -> bool {
        if (limited) {
            if (limit == 0) {
                return false;
            }
            --limit;
        }
        entries.push_back({std::string{entry}, size});
        return true;
    });

    return entries;
}

buffer_t load(std::string_view fname, size_t maxsiz)
{
    if (maxsiz == 0) {
        maxsiz = LOAD_MAXSIZ;
    }

    std::ifstream is{fname, std::ios::in};
    if (!is) {
        throw IOError{"Can't load: {}: {}", fname, Error::to_string()};
    }

    try {
        return load(is, maxsiz);
    } catch (const std::exception& err) {
        throw IOError{"Can't load: {}: {}", fname, err.what()};
    }
}

buffer_t load(std::istream& is, size_t maxsiz)
{
    buffer_t buf{};
    uint8_t c{};

    if (maxsiz == 0) {
        maxsiz = LOAD_MAXSIZ;
    }

    while (buf.size() < maxsiz) {
        if (!is.read(reinterpret_cast<char*>(&c), sizeof(c))) {
            if (!is.eof()) {
                throw IOError{"Can't read from stream: {}", Error::to_string()};
            }
            break;
        }

        buf.push_back(c);
    }

    return buf;
}

void save(std::string_view fname, std::span<const uint8_t> buf, std::ios_base::openmode mode)
{
    std::ofstream os{fname, mode};
    if (!os) {
        throw IOError{"Can't save: {}: {}", fname, Error::to_string()};
    }

    save(os, buf);
}

std::ostream& save(std::ostream& os, std::span<const uint8_t> buf)
{
    if (!os.write(reinterpret_cast<const char*>(buf.data()), buf.size())) {
        throw IOError{"Can't write: {}", Error::to_string()};
    }

    return os;
}

}
}
