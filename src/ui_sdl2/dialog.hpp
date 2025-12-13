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

#include <string>

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Spawn a new process and launch the dialog command.
 * @param args Dialog arguments.
 * @return The dialog output on success; an empty string if there is no output or on error.
 * @exception IOError
 * @see fs::shell(const std::string&)
 */
std::string dialog_exec(const std::string& args);

/**
 * SaveAs dialog.
 * @param msg   Dialog message;
 * @param dir   Initial browsing directory;
 * @param fname Current selected file name.
 * @param ext   File extension.
 * @return The chosen fullpath on success; an empty string if the
 * operation was cancelled by the user or some error occurred.
 */
std::string dialog_saveas(const std::string& msg, const std::string& dir, const std::string& fname,
    const std::string& ext = "");

/**
 * Pick File dialog.
 * @param msg   Dialog message;
 * @param dir   Initial browsing directory;
 * @param fname Default selected file name;
 * @param ext   File extension.
 * @return The chosen fullpath on success; an empty string if the
 * operation was cancelled by the user or some error occurred.
 */
std::string dialog_pick_file(const std::string& msg, const std::string& dir, const std::string& fname,
    const std::string& ext);

/**
 * Error Message dialog.
 * @param title  Dialog title;
 * @param errmsg Error message (can be multiline);
 */
void dialog_error(const std::string& reason, const std::string& errmsg);

}
}
}
