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
#include <filesystem>
#include <functional>
#include <iostream>
#include <unordered_map>

#include "types.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "version.hpp"

#include "c64.hpp"
#include "nes.hpp"
#include "zx80.hpp"
#include "zxsp.hpp"

#include "configurator.hpp"

using namespace caio;

static const char* progname;

template<class MACHINE, class CMDLINE>
int machine_main(int argc, const char** argv)
{
    try {
        CMDLINE cmdline{};
        auto [sec, pname] = config::parse(argc, argv, cmdline);

        caio::log.logfile(sec[config::KEY_LOGFILE]);
        caio::log.loglevel(sec[config::KEY_LOGLEVEL]);

        MACHINE machine{sec};
        machine.run(pname);
        return 0;

    } catch (const std::exception& err) {
        std::cerr << MACHINE::name() << ": Error: " << err.what() << "\n";
    }

    return EXIT_FAILURE;
}

#define MACHINE_ENTRY(name, nm, type)         { name, machine_main<nm::type, nm::type ## Cmdline> }

static std::unordered_map<std::string, std::function<int(int, const char**)>> machines = {
    MACHINE_ENTRY("c64",        commodore::c64,         C64),
    MACHINE_ENTRY("nes",        nintendo::nes,          NES),
    MACHINE_ENTRY("zx80",       sinclair::zx80,         ZX80),
    MACHINE_ENTRY("zxspectrum", sinclair::zxspectrum,   ZXSpectrum)
};

[[noreturn]]
static void terminate()
{
    stacktrace(std::cerr);
    std::exit(EXIT_FAILURE);
}

static void usage()
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
    static const auto run_gui = []() -> int {
#ifdef GUI_COMBO_PATH_RELATIVE
        /*
         * chdir to the binary's directory so the GUI
         * looks for everything relative to that position.
         * This is used to run self-contained bundles.
         */
        std::error_code ec{};
        const auto bindir = fs::exec_directory();
        std::filesystem::current_path(bindir, ec);
        if (ec) {
            caio::log.fatal("Can't change current working directory: {}: {}\n", bindir.string(), ec.message());
        }
#endif
        auto& gui = ui::sdl2::ConfiguratorApp::instance();
        return gui.run();
    };

    progname = argv[0];
    std::set_terminate(terminate);

    if (argc == 1) {
        /*
         * No machine is specified: Run the selector GUI.
         */
        return run_gui();
    }

    std::string name{};
    if (argc > 1) {
        name = utils::tolow(argv[1]);
        --argc;
        ++argv;
    }

    if (name == "" || name == "--help" || name == "-h" || name == "-?") {
        usage();
        std::exit(EXIT_FAILURE);
    }

    if (name == "-v" || name == "--version") {
        std::cerr << full_version() << "\n";
        std::exit(EXIT_SUCCESS);
    }

    if (name == "gui") {
        return run_gui();
    }

    auto it = machines.find(name);
    if (it != machines.end()) {
        return it->second(argc, argv);
    }

    std::cerr << "Unknown emulator: " << name << "\n";
    return EXIT_FAILURE;
}
