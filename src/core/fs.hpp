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

#include <cstdint>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <gsl/span>

#include "types.hpp"


namespace caio {
namespace fs {

constexpr static const size_t LOAD_MAXSIZ       = 65536;
constexpr static size_t DIR_ENTRIES_LIMIT       = 256;
constexpr static bool MATCH_CASE_INSENSITIVE    = true;
constexpr static bool MATCH_CASE_SENSITIVE      = false;

using dir_entry_t = std::pair<std::string, uint64_t>;
using dir_t = std::vector<dir_entry_t>;

/**
 * Return the value of the HOME environment variable.
 * @return A string with the value of the HOME environment variable.
 */
std::string home();

/**
 * Set a the proper home path.
 * If the specified path starts with '~' it is replaced
 * with the value of the HOME environment variable.
 * @param path Path to fix.
 * @return The fixed path.
 */
std::string fix_home(const std::string& path);

/**
 * std::filesystem::exists() wrapper
 * @param path Pathname.
 * @return True if the specified file exists; false otherwise.
 */
static inline bool exists(const std::string& path)
{
    std::error_code ec{};
    return std::filesystem::exists(path, ec);
}

/**
 * std::filesystem::is_directory() wrapper.
 * @param path Pathname.
 * @return True if the specified path is a directory; false otherwise.
 */
static inline bool is_directory(const std::string& path)
{
    std::error_code ec{};
    return std::filesystem::is_directory(path, ec);
}

/**
 * std::filesystem::file_size() wrapper.
 * @param path File name.
 * @return The file size or -1 if the file does not exist or it is not a file.
 */
static inline std::uintmax_t file_size(const std::string& path)
{
    std::error_code ec{};
    return std::filesystem::file_size(path, ec);
}

/**
 * Search for a file.
 * If the file name specifies a directory it is searched as it is, the search paths are ignored.
 * If the file name does not specify a directory and cwd is true the file is first searched on
 * the current working directory.
 * A staring '~' character in fname or spaths is replaced with
 * the content of the HOME environment variable.
 * @param fname Name of the file to search;
 * @param spath Search paths (used only if the file name does not contain a directory);
 * @param cwd   If true (and fname does not contain a directory) look for the file in the current working directory,
 *              if it is not found there then try the search paths.
 * @return The existing file name; an empty string if the file is not found.
 */
std::string search(const std::string& fname, const std::initializer_list<std::string>& spath = {}, bool cwd = false);

/**
 * Retrieve the basename of a full path.
 * @param fullpath Full path name.
 * @return The basename.
 * @see dirname(const std::string&)
 */
std::string basename(const std::string& fullpath);

/**
 * Retrieve the directory name of a full path.
 * @param fullpath Full path name.
 * @return The directory name.
 * @see basename(const std::string&)
 */
std::string dirname(const std::string& fullpath);

/**
 * Concatenate files.
 * If the destination file exists it is opened in append mode.
 * @param dst Destination file name;
 * @param src Source file name.
 * @exception IOError
 */
void concat(const std::string& dst, const std::string& src);

/**
 * Remove a file.
 * @param fname File remove.
 * @return True on success; false on error.
 */
bool unlink(const std::string& fname);

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
bool match(const std::string& path, const std::string& pattern, bool icase = MATCH_CASE_SENSITIVE);

/**
 * Get a directory listing.
 * @param dirpath  Directory;
 * @param pattern  Matching pattern;
 * @param icase    MATCH_CASE_INSENSITIVE or MATCH_CASE_SENSITIVE;
 * @param callback User defined callback (return false to stop directory traversing).
 * @return False if the callback stopped the traversal; true otherwise.
 * @see match(const std::string&, const std::string&)
 * @see MATCH_CASE_INSENSITIVE
 * @see MATCH_CASE_SENSITIVE
 */
bool directory(const std::string& path, const std::string& pattern, bool icase,
    const std::function<bool(const std::string&, uint64_t)>& callback);

/**
 * Get a directory listing.
 * @param dirpath Directory;
 * @param pattern Matching pattern;
 * @param icase   MATCH_CASE_INSENSITIVE or MATCH_CASE_SENSITIVE;
 * @param limit   Maximum number of entries (0 means no limits; default is DIR_ENTRIES_LIMIT).
 * @return The entries that match the specified pattern plus their size on disk.
 * @see directory(const std::string&, const std::string&, const std::function<void(const std::string&, uint64_t)>&)
 * @see match(const std::string&, const std::string&)
 * @see DIR_ENTRIES_LIMIT
 * @see MATCH_CASE_INSENSITIVE
 * @see MATCH_CASE_SENSITIVE
 */
dir_t directory(const std::string& path, const std::string& pattern, bool icase, size_t limit = DIR_ENTRIES_LIMIT);

/**
 * Load the contents of a file into memory.
 * @param fname  File name;
 * @param maxsiz Maximum number of bytes to read (0 means LOAD_MAXSIZ).
 * @return A buffer with the contents of the file.
 * @exception IOError
 * @see load(std::istream&)
 * @see save(const std::string&, const gsl::span<const uint8_t>&, std::ios_base::openmode)
 * @see save(std::ostream&, const gsl::span<const uint8_t>&)
 * @see LOAD_MAXSIZ
 * @see buffer_t
 */
buffer_t load(const std::string& fname, size_t maxsiz = 0);

/**
 * Read data from an input stream and create a memory buffer.
 * @param is     Input stream;
 * @param maxsiz Maximum number of bytes to read (0 means LOAD_MAXSIZ).
 * @return A buffer with the data read from the input stream.
 * @exception IOError
 * @see load(const std::string&)
 * @see save(const std::string&, const gsl::span<const uint8_t>&, std::ios_base::openmode)
 * @see save(std::ostream&, const gsl::span<const uint8_t>&)
 */
buffer_t load(std::istream& is, size_t maxsiz = 0);

/**
 * Save a buffer to a file.
 * @param fname File name;
 * @param buf   Buffer to save;
 * @param mode  Open mode (by default, if the file exists it is truncated).
 * @exception IOError
 * @see load(const std::string&)
 * @see load(std::istream&)
 * @see save(std::ostream&, const gsl::span<const uint8_t>&)
 */
void save(const std::string& fname, const gsl::span<const uint8_t>& buf,
    std::ios_base::openmode mode = std::ios_base::out | std::ios_base::trunc);

/**
 * Send a buffer to an output stream.
 * @param os  Output stream;
 * @param buf Buffer.
 * @see load(const std::string&)
 * @see load(std::istream&)
 * @see save(const std::string&, const gsl::span<const uint8_t>&, std::ios_base::openmode)
 */
std::ostream& save(std::ostream& os, const gsl::span<const uint8_t>& buf);

}
}
