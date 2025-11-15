/*
 * Copyright (C) 2025 Claudio Castiglia
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
#include "dialog.hpp"

#include "fs.hpp"
#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

std::string saveas_dialog(const std::string& title, const std::string& dir, const std::string& fname)
{
#ifdef __APPLE__
    const auto script = std::format(
        "(/usr/bin/osascript 2>/dev/null <<EOF\n"
        "set fname to (choose file name with prompt \"{}\" default name \"{}\" default location \"{}\") as text\n"
        "EOF\n"
        ") | /usr/bin/sed -e 's,^Macintosh HD,,' -e 's,:,/,g'",
        title, fname, dir);
#else
    constexpr static const char* ZENITY = "zenity";
    const auto zenity_path = fs::search(ZENITY, {"/usr/bin", "/bin", "/usr/local/bin"});
    const auto script = std::format(
        "cd {}; {} --title '{}' --file-selection --save --filename='{}' 2>/dev/null",
        dir, (zenity_path.empty() ? ZENITY : zenity_path.string()), title, fname);
#endif

    const auto ss = fs::shell(script);
    return utils::trim(ss.str());
}

}
}
}
