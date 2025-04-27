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
#include "mos_6502_test.hpp"

#include <unistd.h>

#include <algorithm>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <memory>

#include "fs.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "version.hpp"

using namespace std::literals::string_literals;

namespace caio {
namespace test {

Mos6502Test::Mos6502Test(const fs::Path& fname)
    : _clk{std::make_shared<Clock>("clk", CLOCK_FREQ, 0)},
      _ram{std::make_shared<RAM>("ram", fname, 65536)},
      _cpu{std::make_shared<Mos6502>()},
      _io{STDIN_FILENO, STDOUT_FILENO},
      _mmap{std::make_shared<Mos6502TestASpace>(_cpu, _ram, _io)}
{
    _cpu->init(_mmap);
    _clk->add(_cpu);

    auto size = fs::file_size(fname);
    if (size > 0xFFFF - 0x400) {
        size = 0xFFFF - 0x400;
    }

    std::copy_n(_ram->begin(), size, _ram->begin() + 0x400);
}

void Mos6502Test::run(bool autostart)
{
    log.loglevel(Logger::All);
    log.info("Starting {} - MOS-6502 Test Suite\n", full_version());

    _cpu->loglevel("error|info|warning");

#if 0   /* C++26 */
    if (autostart) {
        std::istringstream in{"g 100\n"};
        _cpu->init_monitor(in, std::cout);
    } else {
        _cpu->init_monitor(std::cin, std::cout);
    }
#else
    if (autostart) {
        int fds[2];
        if (::pipe(fds) < 0) {
            log.fatal("Can't create pipe: {}\n", Error::to_string(errno));
            /* NOTREACHED */
        }

        static const auto start = "b 403\ng 400\nq\n"s;
        if (::write(fds[1], start.c_str(), start.size()) != start.size()) {
            log.fatal("Can't write pipe: {}\n", Error::to_string(errno));
            /* NOTREACHED */
        }

        _io.fds({fds[0], -1});
        _cpu->init_monitor(fds[0], STDOUT_FILENO, {}, {});

    } else {
        _cpu->init_monitor(STDIN_FILENO, STDOUT_FILENO, {}, {});
    }
#endif

    _clk->run();

    log.info("\nTerminating MOS-6502 Test Suite\n");
}

}
}

static caio::uptr_t<caio::test::Mos6502Test> test{};

void signal_handler(int signo)
{
    if (signo == SIGINT && test) {
        test->cpu()->ebreak();
    }
}

int main(int argc, char **argv)
{
    bool autostart = false;
    int pos = 1;

    std::signal(SIGINT, signal_handler);

    if (argc > 1 && argv[1] == "-b"s) {
        /*
         * Batch mode.
         */
        autostart = true;
        ++pos;
    }

    try {
        while (pos < argc) {
            std::cout << "==> Running test: " << argv[pos] << "\n";
            test = std::make_unique<caio::test::Mos6502Test>(argv[pos]);
            test->run(autostart);
            ++pos;
        }
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << "\n";
        return -1;
    }
}

