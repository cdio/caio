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
#include "z80_test.hpp"

#include <unistd.h>

#include <algorithm>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>

#include "logger.hpp"
#include "utils.hpp"
#include "version.hpp"

namespace caio {
namespace test {

Z80Test::Z80Test(const std::string &fname)
    : _clk{std::make_shared<Clock>("clk", CLOCK_FREQ, 0)},
      _ram{std::make_shared<RAM>("ram", 65536)},
      _rom{std::make_shared<ROM>("rom", fname, 0)},
      _cpu{std::make_shared<Z80>()},
      _mmap{std::make_shared<Z80TestASpace>(_cpu, _ram, _rom, std::cout)}
{
    std::copy(_rom->begin(), _rom->end(), _ram->begin() + 0x100);
    _cpu->init(_mmap);
    _clk->add(_cpu);
}

void Z80Test::run(bool autostart)
{
    log.loglevel(Logger::All);
    log.info("Starting {} - Z80 Test Suite\n", full_version());

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

        static const std::string start{"g 100\nquit\n"};
        if (::write(fds[1], start.c_str(), start.size()) != start.size()) {
            log.fatal("Can't write pipe: {}\n", Error::to_string(errno));
            /* NOTREACHED */
        }

        _cpu->init_monitor(fds[0], STDOUT_FILENO, {}, {});

    } else {
        _cpu->init_monitor(STDIN_FILENO, STDOUT_FILENO, {}, {});
    }
#endif

    _clk->run();

    log.info("\nTerminating Z80 Test Suite\n");
}

}
}

static caio::uptr_t<caio::test::Z80Test> test{};

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

    if (argc > 1 && argv[1] == std::string{"-b"}) {
        /*
         * Batch mode.
         */
        autostart = true;
        ++pos;
    }

    try {
        while (pos < argc) {
            std::cout << "==> Running test: " << argv[pos] << "\n";
            test = std::make_unique<caio::test::Z80Test>(argv[pos]);
            test->run(autostart);
            ++pos;
        }
    } catch (const std::exception &err) {
        std::cerr << "Error: " << err.what() << "\n";
        return -1;
    }
}

