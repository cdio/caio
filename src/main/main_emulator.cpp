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
#include "c64.hpp"
#include "nes.hpp"
#include "zx80.hpp"
#include "zxsp.hpp"

extern std::string progname;

using namespace caio;

template<class MACHINE, class CMDLINE>
int main_machine(int argc, const char** argv)
{
    uptr_t<MACHINE> machine{};

    try {
        CMDLINE cmdline{};
        auto [sec, pname] = config::parse(argc, argv, cmdline);

        caio::log.logfile(sec[config::KEY_LOGFILE]);
        caio::log.loglevel(sec[config::KEY_LOGLEVEL]);

        machine = std::make_unique<MACHINE>(sec);
        machine->run(pname);
        return 0;

    } catch (const std::exception& err) {
        std::cerr << progname << ": ";
        if (machine) {
            std::cerr << machine->label() << ": ";
        }
        std::cerr <<  "Error: " << err.what() << "\n";
    }

    return EXIT_FAILURE;
}

#define MACHINE_ENTRY(name, nm, type)         { name, main_machine<nm::type, nm::type ## Cmdline> }

static const std::map<std::string, std::function<int(int, const char**)>> machines = {
    MACHINE_ENTRY("c64",        commodore::c64,         C64),
    MACHINE_ENTRY("nes",        nintendo::nes,          NES),
    MACHINE_ENTRY("zx80",       sinclair::zx80,         ZX80),
    MACHINE_ENTRY("zxspectrum", sinclair::zxspectrum,   ZXSpectrum)
};

std::string emulators(std::string_view separator)
{
    std::stringstream ss{};

    for (const auto& entry : machines) {
        ss << entry.first << separator;
    }

    return ss.str();
}

static int usage()
{
    const auto& usage = std::format(
        "usage: {} <arch> [--help]\n"
        "where arch is one of:\n"
        "{}\n",
        progname,
        emulators("\n"));

    std::cerr << usage;
    return EXIT_FAILURE;
}

int main_emulator(int argc, const char** argv)
{
    const auto& name = utils::tolow(argv[1]);

    if (name != "help") {
        if (auto it = machines.find(name); it != machines.end()) {
            return it->second(argc - 1, argv + 1);
        }

        std::cerr << progname << ": Unknown emulator: " << name << "\n";
    }

    return usage();
}
