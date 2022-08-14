/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include <string>
#include <utility>
#include <vector>


namespace caio {
namespace fs {

/**
 * @return A string with the value of the HOME environment variable.
 */
std::string home();


/**
 * Set a the proper home path.
 * If the specified path starts with '~' it is replaced
 * with the value of the HOME environment variable.
 * @parma path Path to fix.
 * @return The fixed path.
 */
std::string fix_home(const std::string &path);


/**
 * Detect whether a file exists or not.
 * @param path Path to the file.
 * @return True if the specified file exists; false otherwise.
 */
bool exists(const std::string &path);


/**
 * std::filesystem::is_directory() wrapper.
 */
static inline bool is_directory(const std::string &path)
{
    return std::filesystem::is_directory(path);
}


/**
 * std::filesystem::file_size() wrapper.
 */
static inline std::uintmax_t file_size(const std::string &path)
{
    return std::filesystem::file_size(path);
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
std::string search(const std::string &fname, const std::initializer_list<std::string> &spath = {}, bool cwd = false);


/**
 * @return The base name given a full path name.
 */
std::string basename(const std::string &fullpath);


/**
 * Concatenate files.
 * If the destination file exists it is opened in append mode.
 * @param dst Destination file name;
 * @param src Source file name.
 * @exception IOError
 */
void concat(const std::string &dst, const std::string &src);


/**
 * Remove a file.
 * @param fname File remove.
 * @return True on success; false on error.
 */
bool unlink(const std::string &fname);


/**
 * Match a file.
 * @param fname   File name;
 * @param pattern fnmatch(3) style pattern.
 * @return True if the file name matches the specified pattern; false otherwise.
 * @see ::fnmatch(3)
 */
bool match(const std::string &path, const std::string &pattern);


/**
 * Get a directory listing.
 * @param dirpath  Directory;
 * @param pattern  Matching pattern (see fs::match());
 * @param callback User defined callback (return false to stop directory traversing).
 * @return False if the callback stopped the traversal; true otherwise.
 * @see fs::match()
 */
bool directory(const std::string &path, const std::string &pattern,
    const std::function<bool(const std::string &, uint64_t)> &callback);


/**
 * Get a directory listing.
 * @param dirpath Directory;
 * @param pattern Matching pattern.
 * @return The entries that match the specified pattern plus their size on disk.
 * @see directory(const std::string &, const std::string &, const std::function<void(const std::string &, uint64_t)> &)
 * @see fs::match()
 */
std::vector<std::pair<std::string, uint64_t>> directory(const std::string &path, const std::string &pattern);

}
}
