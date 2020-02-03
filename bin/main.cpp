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
#include <iostream>
#include <cstdlib>

#include "fs.hpp"
#include "utils.hpp"

#include "c64_main.hpp"


using namespace cemu;


#ifndef D_DEBUG
[[noreturn]]
static void terminate()
{
    stacktrace(std::cerr);
    ::exit(EXIT_FAILURE);
}
#endif

int main(int argc, char * const *argv)
{
#ifndef D_DEBUG
    std::set_terminate(terminate);
#endif

    auto progname = *argv;
    auto name = utils::tolow(fs::basename(*argv));
    if (name == "cemu") {
        if (argc > 1) {
            name = fs::basename(argv[1]);
            --argc;
            ++argv;
        } else {
            name = {};
        }

        if (name == "" || name == "--help" | name == "-h" || name == "-?") {
            std::cerr << "usage: " << progname << " <arch> [--help]" << std::endl
                      << "where arch is one of: "                        << std::endl
                      << "c64"                                           << std::endl
                      << std::endl;

            return EXIT_FAILURE;
        }
    }

    if (name == "c64") {
        return c64::main(argc, argv);
    }

    std::cerr << "Unkown emulator: " << name << std::endl;

    return EXIT_FAILURE;
}
