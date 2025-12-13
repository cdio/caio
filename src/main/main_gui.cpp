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
#include "configurator.hpp"

extern std::string progname;

int main_gui(int argc, const char** argv)
{
#ifdef GUI_COMBO_PATH_RELATIVE
    /*
     * chdir to the binary's directory so the gui
     * looks for everything relative to that position.
     * This is used to run self-contained bundles
     * (such as macos dmg packages).
     */
    std::error_code ec{};
    const auto& bindir = caio::fs::exec_directory();
    std::filesystem::current_path(bindir, ec);
    if (ec) {
        caio::log.fatal("{}: Can't change working directory: {}: {}\n",
            progname, bindir.string(), ec.message());
    }
#endif

    caio::ui::sdl2::ConfiguratorApp gui{};
    return gui.run();
}
