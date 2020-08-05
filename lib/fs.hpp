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
#pragma once

#include <initializer_list>
#include <string>


namespace cemu {
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

}
}
