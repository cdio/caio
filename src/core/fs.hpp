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
#pragma once

#include <unistd.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "types.hpp"

namespace caio {
namespace fs {

constexpr static const size_t LOAD_MAXSIZ       = 65536;
constexpr static size_t DIR_ENTRIES_LIMIT       = 256;
constexpr static bool MATCH_CASE_INSENSITIVE    = true;
constexpr static bool MATCH_CASE_SENSITIVE      = false;

using Path = std::filesystem::path;
using DirEntry = std::pair<Path, uint64_t>;
using Dir = std::vector<DirEntry>;

/**
 * Return the value of the HOME environment variable.
 * @return A string with the value of the HOME environment variable.
 */
Path home();

/**
 * Set a the proper home path.
 * If the specified path starts with '~' it is replaced
 * with the value of the HOME environment variable.
 * @param path Path to fix.
 * @return The fixed path.
 */
Path fix_home(const Path& path);

/**
 * Get the executable file full path.
 * @return The executable file full path.
 */
Path exec_path();

/**
 * Get the executable file directory.
 * @return The executable file directory.
 * @see exec_path()
 */
Path exec_directory();

/**
 * Get the executable file name.
 * @return The executable file name.
 * @see exec_path()
 */
Path exec_filename();

/**
 * std::filesystem::exists() wrapper.
 * @param path Pathname.
 * @return True if the specified file exists; false otherwise.
 */
static inline bool exists(const Path& path)
{
    std::error_code ec{};
    return std::filesystem::exists(path, ec);
}

/**
 * std::filesystem::is_directory() wrapper.
 * @param path Pathname.
 * @return True if the specified path is a directory; false otherwise.
 */
static inline bool is_directory(const Path& path)
{
    std::error_code ec{};
    return std::filesystem::is_directory(path, ec);
}

/**
 * std::filesystem::file_size() wrapper.
 * @param path File name.
 * @return The file size or -1 if the file does not exist or it is not a file.
 */
static inline std::uintmax_t file_size(const Path& path)
{
    std::error_code ec{};
    return std::filesystem::file_size(path, ec);
}

/**
 * Search for a file.
 * If the file name specifies a directory it is searched as it is, the search paths are ignored.
 * A staring '~' character in fname or spaths is replaced with the content of the HOME environment variable.
 * @param fname Name of the file to search;
 * @param spath Search paths (used only if the file name does not contain a directory);
 * @return The existing file name; an empty path if the file is not found.
 */
Path search(const Path& fname, const std::initializer_list<const Path>& spath = {});

/**
 * std::filesystem::path::filename() wrapper.
 * @param fullpath Full path name.
 * @return The basename.
 */
static inline Path basename(const Path& fullpath)
{
    return fullpath.filename();
}

/**
 * std::filesystem::path::parent_path() wrapper.
 * @param fullpath Full path name.
 * @return The parent directory.
 */
static inline Path dirname(const Path& fullpath)
{
    return fullpath.parent_path();
}

/**
 * Concatenate files.
 * If the destination file exists it is opened in append mode.
 * @param dst Destination file name;
 * @param src Source file name.
 * @exception IOError
 */
void concat(const Path& dst, const Path& src);

/**
 * std::filesystem::remove() wrapper.
 * @param fname File remove.
 * @return True on success; false on error.
 * @exception IOError
 */
void unlink(const Path& fname);

/**
 * Match a file.
 * @param fname   File name;
 * @param pattern fnmatch(3) style pattern;
 * @param icase   MATCH_CASE_INSENSITIVE or MATCH_CASE_SENSITIVE (default).
 * @return True if the file name matches the specified pattern; false otherwise.
 * @see MATCH_CASE_INSENSITIVE
 * @see MATCH_CASE_SENSITIVE
 * @see Unix manpage fnmatch(3)
 */
bool match(const Path& path, const Path& pattern, bool icase = MATCH_CASE_SENSITIVE);

/**
 * Get a (recursive) directory listing.
 * @param dirpath  Directory;
 * @param pattern  Matching pattern;
 * @param icase    MATCH_CASE_INSENSITIVE or MATCH_CASE_SENSITIVE;
 * @param callback User defined callback (return false to stop directory traversing).
 * @return False if the callback stopped the traversal; true otherwise.
 * @see match(const Path&, const Path&, bool)
 * @see MATCH_CASE_INSENSITIVE
 * @see MATCH_CASE_SENSITIVE
 */
bool directory(const Path& path, const Path& pattern, bool icase,
    const std::function<bool(const Path&, uint64_t)>& callback);

/**
 * Get a (recursive) directory listing.
 * @param dirpath Directory;
 * @param pattern Matching pattern;
 * @param icase   MATCH_CASE_INSENSITIVE or MATCH_CASE_SENSITIVE;
 * @param limit   Maximum number of entries (0 means no limits; default is DIR_ENTRIES_LIMIT).
 * @return The entries that match the specified pattern plus their size on disk.
 * @see directory(const Path&, const Path&, const std::function<void(const Path&, uint64_t)>&)
 * @see match(const Path&, const Path&, bool)
 * @see DIR_ENTRIES_LIMIT
 * @see MATCH_CASE_INSENSITIVE
 * @see MATCH_CASE_SENSITIVE
 */
Dir directory(const Path& path, const Path& pattern, bool icase, size_t limit = DIR_ENTRIES_LIMIT);

/**
 * Interactive directory traversing.
 */
class IDir {
public:
    constexpr static const uint64_t REFRESH_TIME = 1'000'000; /* us */
    constexpr static const char* ENTRY_BACK      = "..";
    constexpr static const size_t MAX_DIRS       = 2000;

    using FilterCb = std::function<Path(const Path&)>;

    enum EntryType {
        Dir     = 0x01,     /**< Take directory entries     */
        File    = 0x02,     /**< Take regular file entries  */
        All     = 0x03      /**< Take all entries           */
    };

    /**
     * Initialise this interactive directory traverser.
     * @param etype  Entry type to consider;
     * @param eempty Empty entry;
     * @param elimit Directory entries limit (0 means not limit).
     * @see EntryType
     * @see MAX_DIRS
     */
    IDir(EntryType etype, const std::string& eempty, size_t limit = MAX_DIRS);

    virtual ~IDir();

    /**
     * Set the entry filter callback.
     * The filter decides whether a specific entry must
     * be taken and optionally manipulates its name.
     * @param efilter Filter callback.
     */
    void filter(const FilterCb& efilter);

    /**
     * Detect whether a filter callback is set.
     * @return True if a filter is set; false otherwise.
     * @see filter(const FilterCb&)
     */
    bool filter()
    {
        return static_cast<bool>(_efilter);
    }

    /**
     * Reset this interactive directory traverser.
     * @param path New path to traverse.
     */
    void reset(const fs::Path& path);

    /**
     * Refresh the internal state of this interactive directory traverser.
     * The internal state is refreshed (the directory re-traversed again)
     * when at least REFRESH_TIME microseconds have passed since the last
     * refresh.
     * @return True if a refresh was done; otherwise false.
     */
    virtual bool refresh();

    /**
     * Get the current traversed directory.
     * @return The current directory.
     */
    Path path() const
    {
        return _path;
    }

    /**
     * Detect whether there are entries in the current directory.
     * @return True if there are no entries in the current directory; false otherwise.
     */
    bool empty() const
    {
        return (size() == 0);
    }

    /**
     * Return the number of entries in the current directory.
     * @return The number of directory entries in the current directory.
     */
    size_t size() const
    {
        return _entries.size();
    }

    /**
     * Retrieve a directory entry.
     * @param index Entry index.
     * @return The specified entry.
     */
    Path operator[](size_t index) const
    {
        return _entries[index];
    }

    std::string_view empty_entry() const
    {
        return _eempty;
    }

protected:
    bool time_to_refresh() const;
    Path filter(const Path& entry) const;

    EntryType               _etype;
    std::string             _eempty;

    uint64_t                _nrefresh{};
    fs::Path                _path{};
    std::vector<fs::Path>   _entries{};
    FilterCb                _efilter{};
    size_t                  _elimit{};
};

/**
 * Interactive directory traversing that can navigate other directories.
 */
class IDirNav : public IDir {
public:
    /**
     * Initialise this interactive directory traverser.
     * @param etype  Entry type to consider;
     * @param eempty Empty entry;
     * @param elimit Directory entries limit (0 means not limit).
     * @see IDir
     * @see MAX_DIRS
     */
    IDirNav(EntryType etype, const std::string& eempty, size_t elimit = MAX_DIRS);

    virtual ~IDirNav();

    bool refresh() override;
};

/**
 * Load the contents of a file into memory.
 * @param fname  File name;
 * @param maxsiz Maximum number of bytes to read (0 means LOAD_MAXSIZ).
 * @return A buffer with the contents of the file.
 * @exception IOError
 * @see load(std::istream&, size_t)
 * @see save(const Path&, std::span<const uint8_t>, std::ios_base::openmode)
 * @see save(std::ostream&, std::span<const uint8_t>)
 * @see LOAD_MAXSIZ
 * @see buffer_t
 */
buffer_t load(const Path& fname, size_t maxsiz = 0);

/**
 * Read data from an input stream and create a memory buffer.
 * @param is     Input stream;
 * @param maxsiz Maximum number of bytes to read (0 means LOAD_MAXSIZ).
 * @return A buffer with the data read from the input stream.
 * @exception IOError
 * @see load(const Path&, size_t)
 * @see save(const Path&, std::span<const uint8_t>, std::ios_base::openmode)
 * @see save(std::ostream&, std::span<const uint8_t>&)
 */
buffer_t load(std::istream& is, size_t maxsiz = 0);

/**
 * Save a buffer to a file.
 * @param fname File name;
 * @param buf   Buffer to save;
 * @param mode  Open mode (by default, if the file exists it is truncated).
 * @exception IOError
 * @see load(const Path&, size_t);
 * @see load(std::istream&, size_t)
 * @see save(std::ostream&, std::span<const uint8_t>)
 */
void save(const Path& fname, std::span<const uint8_t> buf,
    std::ios_base::openmode mode = std::ios_base::out | std::ios_base::trunc);

/**
 * Send a buffer to an output stream.
 * @param os  Output stream;
 * @param buf Buffer.
 * @see load(const Path&, size_t)
 * @see load(std::istream&, size_t)
 * @see save(const Path&, std::span<const uint8_t>, std::ios_base::openmode)
 */
std::ostream& save(std::ostream& os, std::span<const uint8_t> buf);

/**
 * Calculate the SHA-256 signature of a file.
 * @param fname File name.
 * @return The hash value as a string.
 * @see sha256(std::ifstream&)
 */
std::string sha256(const fs::Path& fname);

/**
 * Calculate the SHA-256 of values coming from an input stream.
 * @param is Input stream.
 * @return The hash value as a string.
 * @see sha256(const fs::Path&)
 */
std::string sha256(std::ifstream& is);

}
}
