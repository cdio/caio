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

#ifdef __APPLE__
#include <mach-o/dyld.h>    /* _NSGetExecutablePath */
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <sstream>

#include "sha2.h"

#include "logger.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace caio {
namespace fs {

Path home()
{
    static const char* chome = std::getenv("HOME");
    return (chome == nullptr ? "" : chome);
}

Path fix_home(const Path& path)
{
    const auto& p = path.string();
    if (!p.empty() && p[0] == '~') {
        std::string newpath{p};
        newpath.replace(0, 1, home().string());
        return newpath;
    }

    return path;
}

Path exec_path()
{
    static Path path{};
    if (path.empty()) {
        char buf[PATH_MAX] = { "" };
#ifdef __APPLE__
        uint32_t bufsiz = sizeof(buf);
        _NSGetExecutablePath(buf, &bufsiz);
#elif defined __linux__
        [[maybe_unused]] auto _ = ::readlink("/proc/self/exe", buf, sizeof(buf));
#else
#error "Implement exec_path() under this OS"
#endif
        path = buf;
    }

    return path;
}

Path exec_directory()
{
    static Path path{};
    if (path.empty()) {
        path = exec_path().parent_path();
    }
    return path;
}

Path exec_filename()
{
    static Path path{};
    if (path.empty()) {
        path = exec_path().filename();
    }
    return path;
}

Path search(const Path& fname, const std::initializer_list<const Path>& spath)
{
    if (fname.empty()) {
        return {};
    }

    log.debug("Looking for file: {}: ", fname.string());

    auto name = basename(fname);
    if (name != fname) {
        /*
         * A directory is specified in fname.
         */
        const auto fullpath = fix_home(fname);
        if (fs::exists(fullpath)) {
            log.debug("Found: {}\n", fullpath.string());
            return fullpath;
        }
    }

    for (const auto& pth : spath) {
        auto fullpath{fix_home(pth)};
        fullpath /= name;
        log.debug("Trying {}... ", fullpath.string());
        if (fs::exists(fullpath)) {
            log.debug("Found\n");
            return fullpath;
        }
    }

    log.debug("Not found\n");
    return {};
}

void concat(const Path& dst, const Path& src)
{
    std::ifstream is{src, std::ios_base::in | std::ios_base::binary};
    if (!is) {
        throw IOError{"Can't open input file: {}: {}", src.string(), Error::to_string(errno)};
    }

    std::ofstream os{dst, std::ios_base::out | std::ios_base::app | std::ios_base::binary};
    if (!os) {
        throw IOError{"Can't open output file: {}: {}", dst.string(), Error::to_string(errno)};
    }

    try {
        is.unsetf(std::ios_base::skipws);
        std::copy(std::istream_iterator<uint8_t>(is), std::istream_iterator<uint8_t>(),
            std::ostream_iterator<uint8_t>(os));
    } catch (const std::exception& err) {
        throw IOError{err};
    }
}

void unlink(const Path& fname)
{
    try {
        std::filesystem::remove(fname);
    } catch (const std::exception& err) {
        throw IOError{err};
    }
}

bool match(const Path& path, const Path& pattern, bool icase)
{
    const char* cpath = path.c_str();
    const char* cpattern = pattern.c_str();
    return (!::fnmatch(cpattern, cpath, FNM_NOESCAPE | (icase == MATCH_CASE_INSENSITIVE ? FNM_CASEFOLD : 0)));
}

template<typename ITERATOR_TYPE>
bool _directory(const Path& path, const Path& pattern, bool icase,
    const std::function<bool(const Path&, uint64_t)>& callback)
{
    const auto& end = std::filesystem::end(ITERATOR_TYPE{});
    ITERATOR_TYPE it{path, std::filesystem::directory_options::skip_permission_denied};

    for (; it != end; ++it) {
        const auto& entry = it->path();
        if (!std::filesystem::is_directory(entry) && fs::match(entry, pattern, icase)) {
            auto size = std::filesystem::file_size(entry);
            if (callback(entry, size) == false) {
                return false;
            }
        }
    }

    return true;
}

bool directory(const Path& path, const Path& pattern, bool icase,
    const std::function<bool(const Path&, uint64_t)>& callback)
{
    return _directory<std::filesystem::recursive_directory_iterator>(path, pattern, icase, callback);
}

Dir directory(const Path& path, const Path& pattern, bool icase, size_t limit)
{
    Dir entries{};
    bool limited = (limit != 0);

    const auto filter = [&entries, &limit, limited](const Path& entry, uint64_t size) -> bool {
        if (limited) {
            if (limit == 0) {
                return false;
            }
            --limit;
        }
        entries.push_back({entry, size});
        return true;
    };

    directory(path, pattern, icase, filter);
    return entries;
}

Buffer load(const Path& fname, size_t maxsiz)
{
    if (maxsiz == 0) {
        maxsiz = LOAD_MAXSIZ;
    }

    std::ifstream is{fname, std::ios::in};
    if (!is) {
        throw IOError{"Can't load: {}: {}", fname.string(), Error::to_string()};
    }

    try {
        return load(is, maxsiz);
    } catch (const std::exception& err) {
        throw IOError{"Can't load: {}: {}", fname.string(), err.what()};
    }
}

Buffer load(std::istream& is, size_t maxsiz)
{
    Buffer buf{};
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

void save(const Path& fname, std::span<const uint8_t> buf, std::ios_base::openmode mode)
{
    std::ofstream os{fname, mode};
    if (!os) {
        throw IOError{"Can't save: {}: {}", fname.string(), Error::to_string()};
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

std::string sha256(const fs::Path& fname)
{
    std::ifstream is{fname};
    return sha256(is);
}

std::string sha256(std::ifstream& is)
{
    uint8_t buf[BUFSIZ];

    SHA2_CTX ctx{};
    uint8_t md[SHA256_DIGEST_LENGTH];
    SHA256Init(&ctx);

    while (is) {
        is.read(reinterpret_cast<char*>(buf), sizeof(buf));
        const auto size = is.gcount();
        if (size > 0) {
            SHA256Update(&ctx, buf, size);
        }
    }

    SHA256Final(md, &ctx);

    std::ostringstream os{};
    for (uint8_t value : md) {
        os << utils::to_string(value);
    }

    return os.str();
}

IDir::IDir(EntryType etype, const std::string& eempty, size_t elimit)
    : _etype{etype},
      _eempty{eempty},
      _elimit{elimit}
{
}

IDir::~IDir()
{
}

void IDir::filter(const FilterCb& efilter)
{
    _efilter = efilter;
    reset(_path);
}

inline Path IDir::filter(const Path& entry) const
{
    return (_efilter ? _efilter(entry) : entry);
}

void IDir::reset(const Path& path)
{
    std::error_code ec{};
    bool isdir{};
    const auto fpath = fix_home(path);
    auto canon = std::filesystem::canonical(fpath, ec);
    if (!ec) {
        isdir = (std::filesystem::status(canon, ec).type() == std::filesystem::file_type::directory);
    }

    if (ec) {
        /*
         * Error getting the canonical path, default to the root directory.
         */
        if (_path.empty()) {
            _path = "/";
        }
    } else {
        /*
         * Canonical path exists.
         */
        if (!isdir) {
            canon = canon.parent_path();
        }

        if (_path.empty()) {
            /*
             * First time reset is called: Set the specified directory.
             */
            _path = canon;
        } else {
            /*
             * This is not the first time reset is called.
             */
            if (path == "..") {
                /*
                 * Traverse the parent of the current path.
                 */
                _path = _path.parent_path();
            } else {
                /*
                 * Traverse a new directory.
                 */
                _path = canon;
            }
        }
    }

    _entries.clear();
    _nrefresh = 0;
    refresh();
}

bool IDir::refresh()
{
    if (!time_to_refresh()) {
        return false;
    }

    const bool want_dirs = (_etype & EntryType::Dir);
    const bool want_files = (_etype & EntryType::File);
    std::vector<Path> files{};

    const auto flt = [this, &files, want_dirs, want_files](const Path& entry) {
        std::error_code ec{};
        if (want_dirs && std::filesystem::is_directory(entry, ec)) {
            /* Add an ending slash to the entry name so the user knows it is a directory */
            auto direntry = entry;
            direntry /= "";
            _entries.push_back(direntry);

        } else if (!ec && want_files && std::filesystem::is_regular_file(entry, ec)) {
            const auto filtered = filter(entry);
            if (!filtered.empty()) {
                files.push_back(filtered);
            }
        }

        const size_t total = _entries.size() + files.size();
        return ((_elimit == 0) || (total < _elimit));
    };

    _entries.clear();
    const auto end = std::filesystem::end(std::filesystem::directory_iterator{});
    std::filesystem::directory_iterator it{_path, std::filesystem::directory_options::skip_permission_denied};

    for (; it != end; ++it) {
        const auto& entry = it->path();
        if (!flt(entry)) {
            break;
        }
    }

    std::sort(_entries.begin(), _entries.end());
    if (want_dirs && _entries.size() == 0) {
        _entries.push_back(_path);
    }

    std::sort(files.begin(), files.end());
    std::move(files.begin(), files.end(), std::back_inserter(_entries));
    _entries.insert(_entries.begin(), _eempty);
    _nrefresh = utils::now() + REFRESH_TIME;
    return true;
}

inline bool IDir::time_to_refresh() const
{
    return (utils::now() >= _nrefresh);
}

IDirNav::IDirNav(EntryType etype, const std::string& eempty, size_t elimit)
    : IDir{etype, eempty, elimit}
{
}

IDirNav::~IDirNav()
{
}

bool IDirNav::refresh()
{
    if (!IDir::refresh()) {
        return false;
    }
    _entries.insert(_entries.begin() + 1, ENTRY_BACK);
    return true;
}

}
}
