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
#include <iostream>
#include <cstdlib>

#include "fs.hpp"
#include "utils.hpp"

#include "c64_main.hpp"
#include "zx80_main.hpp"


using namespace caio;

static std::terminate_handler prev_terminate{};

[[noreturn]]
static void terminate()
{
    stacktrace(std::cerr);
    prev_terminate();
    ::exit(EXIT_FAILURE);
}

int main(int argc, char* const* argv)
{
    prev_terminate = std::get_terminate();
    std::set_terminate(terminate);

    auto progname = *argv;
    auto name = utils::tolow(fs::basename(*argv));
    if (name == "caio") {
        if (argc > 1) {
            name = fs::basename(argv[1]);
            --argc;
            ++argv;
        } else {
            name = {};
        }

        if (name == "" || name == "--help" || name == "-h" || name == "-?") {
            std::cerr << "usage: " << progname << " <arch> [--help]" << std::endl
                      << "where arch is one of: "                    << std::endl
                      << "c64"                                       << std::endl
                      << "zx80"                                      << std::endl
                      << std::endl;

            return EXIT_FAILURE;
        }
    }

    if (name == "c64") {
        c64::main(argc, argv);
    } else if (name == "zx80") {
        zx80::main(argc, argv);
    }

    std::cerr << "Unknown emulator: " << name << std::endl;

    return EXIT_FAILURE;
}
