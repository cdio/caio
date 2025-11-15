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
#pragma once

#include <string>

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * SaveAs dialog.
 * @param title Dialog title;
 * @param dir   Default selection directory;
 * @param fname Default selected file name.
 * @return The chosen fullpath on success; an empty string if the
 * operation was cancelled by the user or some error occurred.
 */
std::string saveas_dialog(const std::string& title, const std::string& dir, const std::string& fname);

}
}
}
