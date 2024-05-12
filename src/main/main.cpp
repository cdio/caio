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
#include <cstdlib>
#include <exception>
#include <iostream>
#include <functional>
#include <map>

#include "types.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "version.hpp"

#include "c64.hpp"
#include "zx80.hpp"
#include "zxsp.hpp"

using namespace caio;

template<class MACHINE, class CMDLINE>
[[noreturn]] void machine_main(int argc, const char** argv)
{
    try {
        CMDLINE cmdline{};
        auto sec = config::parse(argc, argv, cmdline);

        caio::log.logfile(sec[config::KEY_LOGFILE]);
        caio::log.loglevel(sec[config::KEY_LOGLEVEL]);

        MACHINE machine{sec};
        machine.run();

        std::exit(EXIT_SUCCESS);

    } catch (const std::exception& err) {
        std::cerr << MACHINE::name() << ": Error: " << err.what() << "\n";
    }

    std::exit(EXIT_FAILURE);
}

#define MACHINE_ENTRY(name, nm, type)         { name, machine_main<nm::type, nm::type ## Cmdline> }

static std::map<std::string, std::function<void(int, const char**)>> machines = {
    MACHINE_ENTRY("c64",        commodore::c64,         C64),
    MACHINE_ENTRY("zx80",       sinclair::zx80,         ZX80),
    MACHINE_ENTRY("zxspectrum", sinclair::zxspectrum,   ZXSpectrum)
};

[[noreturn]]
static void terminate()
{
    stacktrace(std::cerr);
    std::exit(EXIT_FAILURE);
}

static void usage(const std::string& progname)
{
    std::cerr << "usage: " << progname << " <arch> [--help]\n"
                 "where arch is one of:\n";

    std::for_each(machines.begin(), machines.end(), [](const auto& entry) {
        std::cerr << entry.first << "\n";
    });

    std::cerr << "\n";
}

int main(int argc, const char** argv)
{
    std::set_terminate(terminate);
    std::string progname = *argv;

    std::string name{};
    if (argc > 1) {
        name = argv[1];
        --argc;
        ++argv;
    }

    if (name == "" || name == "--help" || name == "-h" || name == "-?") {
        usage(progname);
        std::exit(EXIT_FAILURE);
    }

    if (name == "-v" || name == "--version") {
        std::cerr << full_version() << "\n";
        std::exit(EXIT_SUCCESS);
    }

    auto it = machines.find(caio::tolow(name));
    if (it != machines.end()) {
        it->second(argc, argv);
        /* NOTREACHED */
    }

    std::cerr << "Unknown emulator: " << name << "\n";
    return EXIT_FAILURE;
}
