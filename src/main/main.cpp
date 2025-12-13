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
#include "file_dialog.hpp"

#include "c64.hpp"
#include "nes.hpp"
#include "zx80.hpp"
#include "zxsp.hpp"

#include "types.hpp"
#include "config.hpp"
#include "fs.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "version.hpp"

extern int main_dialog(int, const char**);
extern int main_emulator(int, const char**);
extern int main_gui(int, const char**);
extern std::string emulators(std::string_view);

std::string progname{};

[[noreturn]]
static void terminate()
{
#if 0   // Not very useful
    stacktrace(std::cerr);
#endif
    std::exit(EXIT_FAILURE);
}

static int usage()
{
    const auto& usage = std::format(
        "usage: {} [gui]\n"
        "       {} {}help\n"
        "       {} dialog [-h]\n"
        "       {} -v|--version\n"
        "       {} -h|--help\n\n",
        progname,
        progname, emulators(" | "),
        progname,
        progname,
        progname);

    std::cerr << usage;
    return EXIT_FAILURE;
}

int main(int argc, const char** argv)
{
    std::set_terminate(terminate);
    progname = std::filesystem::path{argv[0]}.filename().string();

    if (argc < 2) {
        /*
         * No machine is specified: Run the machine configurator.
         */
        return main_gui(argc, argv);
    }

    const std::string_view name = argv[1];

    if (name.empty() || name == "--help" || name == "-h") {
        return usage();
    }

    if (name == "-v" || name == "--version") {
        std::cerr << caio::full_version() << "\n";
        return EXIT_SUCCESS;
    }

    if (name == "gui") {
        return main_gui(argc, argv);
    }

    if (name == "dialog") {
        return main_dialog(argc, argv);
    }

    return main_emulator(argc, argv);
}
